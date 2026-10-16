#pragma once

#include "../Memory/SafeAllocator.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Alchemy recipe, as stored in the recipe database (func_02071d60 looks one up
// by id). sizeof == 0x20.
struct Recipe {
    short id;                   // 0x0
    short resultItem;           // 0x2: item made, also its background/model id
    short ingredients[3];       // 0x4: item ids, <= 0 = unused
    u16 quantity0 : 4;          // 0xa: needed count of each ingredient
    u16 quantity1 : 4;
    u16 quantity2 : 4;
    // hero stat driving the success rate (0 = always 100 %, 1..7 = 10-bit
    // stats of the protagonist, 8/9 = u16 stats)
    u16 statType : 4;
    u32 rateMin : 10;           // 0xc: success rate range
    u32 rateMax : 10;
    u32 bgIndex : 8;            // background file index (str_ren 0x26 + n / 0x64 + n)
    u32 unk_c_28 : 4;
    union {
        u32 flags10;            // 0x10
        struct {
            u32 statMin : 10;   // stat range mapped to [rateMin, rateMax]
            u32 statMax : 10;
            u32 unk_10_20 : 2;
            u32 unlocked : 1;   // 0x400000: shown in the recipe book
            u32 unk_10_23 : 9;
        };
    };
    short bonusRecipe;          // 0x14: recipe made on a great success, < 0 = none
    short unk_16;               // < 0: counted by AlchemyBook::CountCraftedRecipes
    u8 unk_18[0x4];
    Recipe *next;               // 0x1c: sorted list built by func_02071ffc
};

// Recipe database (init func_02071be8, load func_02071c00, lookup
// func_02071d60, sorted list func_02071ffc, match func_02071da4). sizeof == 0xc.
struct RecipeDatabase {
    Recipe *list;               // 0x0: head of the sorted list
    Recipe *recipes;            // 0x4
    u8 unk_8[0x2];
    u16 numRecipes;             // 0xa
};

// Saved state of one recipe (the save keeps 0x1d7 of them). sizeof == 0x4.
struct RecipeEntry {
    short id;                   // 0x0: recipe id, -1 = free slot
    union {
        u16 flags;              // 0x2
        struct {
            u16 crafted : 1;    // made at least once
            u16 known : 1;      // recipe known: can be brewed from the book
            u16 unk_2_2 : 14;
        };
    };

    void Clear();
};

// Recipe being brewed and the ingredients it takes from the player's items
// (AlchemyScreen::book_). sizeof == 0x28.
class AlchemyBook {
public:
    RecipeDatabase *db_;                // 0x0
    short **itemIds_;                   // 0x4: per category (9), see AlchemyScreen
    u8 **itemCounts_;                   // 0x8
    u16 *itemTotals_;                   // 0xc
    Recipe *recipe_;                    // 0x10
    short ingredientIndices_[3];        // 0x14: index in the category, -1 = not found
    signed char ingredientCategories_[3]; // 0x1a: -1 = not found
    u8 unk_1d[0x3];
    RecipeEntry *entries_;              // 0x20
    u16 numEntries_;                    // 0x24
    u8 unk_26[0x2];

    void Initialize();
    void Reset();
    void SetDatabase(RecipeDatabase *db);
    void ApplyUnlockFlags(RecipeDatabase *db, RecipeEntry *entries, int count);
    void SetItems(short **ids, u8 **counts, u16 *totals);
    bool SelectRecipe(int id);
    bool HasIngredientsFor(int quantity);
    void GetIngredientCounts(int recipeId, signed char *out);
    void SetEntries(RecipeEntry *entries, u16 count);
    RecipeEntry *FindEntry(int id);
    void RegisterCraft(short crafted, short bonus, RecipeDatabase *db);
    void UpdateEntry(RecipeEntry *entry);
    short GetSuccessRate(int recipeId);
    short CountCraftedRecipes();
};
