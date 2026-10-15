#include "Alchemy/AlchemyScreen.h"
#include "Filesystem/BackgroundLoader.h"
#include "GameState/GameState.h"
#include "std_library_functions.h"

// Save data block read by func_020ac4c0 and written back by func_020ac494.
struct AlchemySaveBlock {
    u8 unk_00[0x10];
    u32 unk_10 : 23;
    u32 numCraftedRecipes : 9; // 0x10
    u8 unk_14[0x54];
    u8 unk_68[0x48];
};

// Stats of the protagonist read by AlchemyBook::GetSuccessRate.
struct HeroStats {
    u32 stat1 : 10;
    u32 stat2 : 10;
    u32 stat3 : 10;
    u32 unk_0_30 : 2;
    u32 stat4 : 10;
    u32 stat5 : 10;
    u32 stat6 : 10;
    u32 unk_4_30 : 2;
    u32 stat7 : 10;
    u32 unk_8_10 : 22;
};

struct HeroStats2 {
    u8 unk_00[0x30];
    u16 stat8; // 0x30
    u16 stat9; // 0x32
};

struct Hero {
    u8 unk_000[0x134];
    HeroStats2 *stats2; // 0x134
    u8 unk_138[0x18];
    HeroStats *stats; // 0x150
};

extern "C" {
    // recipe lookup by id
Recipe *func_02071d60(RecipeDatabase *db, int id);
    // read / write the saved state of recipes
int func_020ac2d4(int, RecipeEntry *key, RecipeEntry *out, int);
void func_020ac104(void *, RecipeEntry *entries, int count);
    // read / write the total brew count
int func_020ac0b4(u32 *count);
void func_020ac08c(u32 *count);
void func_020ac4c0(AlchemySaveBlock *save);
void func_020ac494(AlchemySaveBlock *save);
void func_020a0228(void *, int);
extern unsigned char data_0211e33c[];
}

void AlchemyBook::Initialize() {
    db_         = 0;
    itemIds_    = 0;
    itemCounts_ = 0;
    itemTotals_ = 0;
    recipe_     = 0;
    memset(ingredientIndices_, -1, 6);
    memset(ingredientCategories_, -1, 3);
    entries_    = 0;
    numEntries_ = 0;
}

void AlchemyBook::Reset() {
    Initialize();
}

void AlchemyBook::SetDatabase(RecipeDatabase *db) {
    db_ = db;
}

// Marks the recipes made or known in the save as shown in the recipe book.
void AlchemyBook::ApplyUnlockFlags(RecipeDatabase *db, RecipeEntry *entries, int count) {
    for (int i = 0; i < count; i++, entries++) {
        Recipe *recipe = func_02071d60(db, entries->id);
        if (recipe != 0) {
            recipe->unlocked = (entries->crafted || entries->known) ? 1 : 0;
        }
    }
}

void AlchemyBook::SetItems(short **ids, u8 **counts, u16 *totals) {
    itemIds_    = ids;
    itemCounts_ = counts;
    itemTotals_ = totals;
}

// Selects a recipe and finds its ingredients in the player's items; false if
// one is missing or not owned in the needed quantity.
bool AlchemyBook::SelectRecipe(int id) {
    if (itemTotals_ == 0) {
        return false;
    }
    recipe_ = func_02071d60(db_, id);
    if (recipe_ == 0) {
        return false;
    }
    if (recipe_->ingredients[0] > 0 && recipe_->quantity0 != 0) {
        ingredientIndices_[0]    = -1;
        ingredientCategories_[0] = -1;
        for (signed char cat = 0; cat < 9; cat++) {
            short *ids = itemIds_[cat];
            u16 n      = itemTotals_[cat];
            for (u16 i = 0; i < n; i++) {
                if (recipe_->ingredients[0] == ids[i]) {
                    ingredientIndices_[0]    = i;
                    ingredientCategories_[0] = cat;
                    break;
                }
            }
        }
        if (ingredientIndices_[0] < 0) {
            return false;
        }
        if (recipe_->quantity0 > itemCounts_[ingredientCategories_[0]][ingredientIndices_[0]]) {
            return false;
        }
    }
    if (recipe_->ingredients[1] > 0 && recipe_->quantity1 != 0) {
        ingredientIndices_[1]    = -1;
        ingredientCategories_[1] = -1;
        for (signed char cat = 0; cat < 9; cat++) {
            short *ids = itemIds_[cat];
            u16 n      = itemTotals_[cat];
            for (u16 i = 0; i < n; i++) {
                if (recipe_->ingredients[1] == ids[i]) {
                    ingredientIndices_[1]    = i;
                    ingredientCategories_[1] = cat;
                    break;
                }
            }
        }
        if (ingredientIndices_[1] < 0) {
            return false;
        }
        if (recipe_->quantity1 > itemCounts_[ingredientCategories_[1]][ingredientIndices_[1]]) {
            return false;
        }
    }
    if (recipe_->ingredients[2] > 0 && recipe_->quantity2 != 0) {
        ingredientIndices_[2]    = -1;
        ingredientCategories_[2] = -1;
        for (signed char cat = 0; cat < 9; cat++) {
            short *ids = itemIds_[cat];
            u16 n      = itemTotals_[cat];
            for (u16 i = 0; i < n; i++) {
                if (recipe_->ingredients[2] == ids[i]) {
                    ingredientIndices_[2]    = i;
                    ingredientCategories_[2] = cat;
                    break;
                }
            }
        }
        if (ingredientIndices_[2] < 0) {
            return false;
        }
        if (recipe_->quantity2 > itemCounts_[ingredientCategories_[2]][ingredientIndices_[2]]) {
            return false;
        }
    }
    return true;
}

// True if the selected recipe can be brewed `quantity` times.
bool AlchemyBook::HasIngredientsFor(int quantity) {
    Recipe *r = recipe_;
    if (r == 0) {
        return false;
    }
    if (r->ingredients[0] > 0 && r->quantity0 != 0) {
        int n       = r->quantity0;
        short index = ingredientIndices_[0];
        if (index < 0) {
            return false;
        }
        signed char cat = ingredientCategories_[0];
        if (cat < 0) {
            return false;
        }
        if (n * quantity > itemCounts_[cat][index]) {
            return false;
        }
    }
    if (r->ingredients[1] > 0 && r->quantity1 != 0) {
        int n       = r->quantity1;
        short index = ingredientIndices_[1];
        if (index < 0) {
            return false;
        }
        signed char cat = ingredientCategories_[1];
        if (cat < 0) {
            return false;
        }
        if (n * quantity > itemCounts_[cat][index]) {
            return false;
        }
    }
    if (r->ingredients[2] > 0 && r->quantity2 != 0) {
        int n       = r->quantity2;
        short index = ingredientIndices_[2];
        if (index < 0) {
            return false;
        }
        signed char cat = ingredientCategories_[2];
        if (cat < 0) {
            return false;
        }
        if (n * quantity > itemCounts_[cat][index]) {
            return false;
        }
    }
    return true;
}

// Owned count of each ingredient of a recipe.
void AlchemyBook::GetIngredientCounts(int recipeId, signed char *out) {
    memset(out, 0, 3);
    Recipe *r = func_02071d60(db_, recipeId);
    if (r == 0) {
        return;
    }
    for (int i = 0; i < 9; i++) {
        short *ids = itemIds_[i];
        u8 *counts = itemCounts_[i];
        short n    = ((short *) itemTotals_)[i];
        for (short j = 0; j < n; j++) {
            short item = ids[j];
            if (item > 0) {
                if (item == r->ingredients[0]) {
                    out[0] = counts[j];
                } else if (item == r->ingredients[1]) {
                    out[1] = counts[j];
                } else if (item == r->ingredients[2]) {
                    out[2] = counts[j];
                }
            }
        }
    }
}

void AlchemyBook::SetEntries(RecipeEntry *entries, u16 count) {
    entries_    = entries;
    numEntries_ = count;
}

RecipeEntry *AlchemyBook::FindEntry(int id) {
    if (id < 0) {
        return 0;
    }
    u16 count = numEntries_;
    for (u16 i = 0; i < count; i++) {
        RecipeEntry *e = &entries_[i];
        if (e->id == id) {
            return e;
        }
    }
    return 0;
}

// Saves a brewed recipe (made and known) and the bonus recipe it unlocks, then
// increments the total brew count.
void AlchemyBook::RegisterCraft(short crafted, short bonus, RecipeDatabase *db) {
    BackgroundLoader::AddLockGlobal();
    BackgroundLoader::FreeAllocationsGlobal();
    AlchemySaveBlock save;
    RecipeEntry e;
    e.Clear();
    if (func_020ac2d4(0, (RecipeEntry *) &crafted, &e, 1)) {
        e.crafted = 1;
        e.known   = 1;
        func_020ac104(data_0211e33c, &e, 1);
        UpdateEntry(&e);
        if (db != 0) {
            Recipe *recipe = func_02071d60(db, ((RecipeEntry *) &crafted)->id);
            if (recipe != 0) {
                recipe->flags10 |= 0x400000;
            }
        }
    }
    if (((RecipeEntry *) &bonus)->id > 0) {
        e.Clear();
        if (func_020ac2d4(0, (RecipeEntry *) &bonus, &e, 1)) {
            e.id    = ((RecipeEntry *) &bonus)->id;
            e.known = 1;
            func_020ac104(data_0211e33c, &e, 1);
            UpdateEntry(&e);
            if (db != 0) {
                Recipe *recipe = func_02071d60(db, ((RecipeEntry *) &bonus)->id);
                if (recipe != 0) {
                    recipe->flags10 |= 0x400000;
                }
            }
            CountCraftedRecipes();
        }
    }
    u32 count = 0;
    if (func_020ac0b4(&count)) {
        count++;
        if (count > 99999) {
            count = 99999;
        }
        func_020ac08c(&count);
        func_020ac4c0(&save);
        func_020a0228(save.unk_68, 1);
        func_020ac494(&save);
    }
    BackgroundLoader::RemoveLockGlobal();
}

void RecipeEntry::Clear() {
    id      = -1;
    crafted = 0;
    known   = 0;
    unk_2_2 = 0;
}

static inline void CopyEntry(RecipeEntry *dst, const RecipeEntry *src) {
    dst->id    = src->id;
    dst->flags = src->flags;
}

// Stores an entry over the one with the same id, else in the first free slot.
void AlchemyBook::UpdateEntry(RecipeEntry *entry) {
    short id  = entry->id;
    u16 count = numEntries_;
    u16 i;
    for (i = 0; i < count; i++) {
        RecipeEntry *e = &entries_[i];
        if (e->id == id) {
            CopyEntry(e, entry);
            return;
        }
    }
    for (i = 0; i < count; i++) {
        RecipeEntry *e = &entries_[i];
        if (e->id <= 0) {
            CopyEntry(e, entry);
            return;
        }
    }
}

// Success rate (0..100) of a recipe: the protagonist's stat mapped from
// [statMin, statMax] to [rateMin, rateMax].
short AlchemyBook::GetSuccessRate(int recipeId) {
    Recipe *recipe = func_02071d60(db_, recipeId);
    if (recipe == NULL) {
        return 0;
    }
    short result = 100;
    if (recipe->statType != 0) {
        Hero *hero = (Hero *) GameState::GetInstance()->GetProtagonist();
        if (hero != NULL) {
            u16 stat = 0;
            switch (recipe->statType) {
                case 1: stat = hero->stats->stat1; break;
                case 2: stat = hero->stats->stat2; break;
                case 3: stat = hero->stats->stat3; break;
                case 4: stat = hero->stats->stat4; break;
                case 5: stat = hero->stats->stat5; break;
                case 6: stat = hero->stats->stat6; break;
                case 7: stat = hero->stats->stat7; break;
                case 8: stat = hero->stats2->stat8; break;
                case 9: stat = hero->stats2->stat9; break;
            }
            float rateRange = recipe->rateMax - recipe->rateMin;
            float statRange = recipe->statMax - recipe->statMin;
            result          = (int) ((stat - (float) recipe->statMin) * (rateRange / statRange)) + recipe->rateMin;
            if (result > 100) {
                result = 100;
            } else if (result < 0) {
                result = 0;
            }
            if (result <= recipe->rateMin) {
                result = recipe->rateMin;
            }
            if (recipe->rateMax <= result) {
                result = recipe->rateMax;
            }
        }
    }
    return result;
}

// Counts the recipes both made and found in the database with unk_16 < 0,
// and saves the count.
short AlchemyBook::CountCraftedRecipes() {
    if (db_ == 0) {
        return 0;
    }
    u16 n                    = 0;
    signed char flags[0x1d8] = {0};
    AlchemySaveBlock save;
    u16 i;
    for (i = 0; i < numEntries_; i++) {
        flags[entries_[i].id] |= entries_[i].crafted;
    }
    for (u16 j = 0; j < db_->numRecipes; j++) {
        Recipe *r = &db_->recipes[(short) j];
        if (r != 0 && r->unk_16 < 0) {
            flags[r->id] |= 2;
        }
    }
    for (u16 k = 1; k < 0x1d8; k++) {
        if (flags[k] == 3) {
            n++;
        }
    }
    func_020ac4c0(&save);
    save.numCraftedRecipes = n;
    func_020ac494(&save);
    return n;
}
