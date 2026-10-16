#include "Alchemy/AlchemyScreen.h"
#include "Filesystem/BackgroundLoader.h"
#include "GameState/GameState.h"
#include "Input/Input.h"
#include "std_library_functions.h"

// Player inventory (func_02010828)
struct Inventory {
    u8 unk_00[0xc];
    short items[0x98];       // 0x0c
    u8 itemCounts[0x98];     // 0x13c
    u8 bag[0xf78 - 0x1d4];   // 0x1d4
    u8 party[4];             // 0xf78
    u8 partyCount;           // 0xf7c
};

// Message window of the game (func_020421a0)
struct MsgWindow {
    void *unk_00;
    u8 unk_04[0x14];
    void *unk_18;
    u8 unk_1c[0x40];
    char *text;              // 0x5c
};

// Glyph of the font (func_0204254c)
struct GlyphInfo {
    u8 unk_00[5];
    signed char width : 6;
    signed char unk_5_6 : 2;
};

struct MessageSpeaker {
    u8 unk_00[4];
    short unk_04;
};

// Item ids of the four sprite slots
struct SlotItemIds {
    short ids[4];
};

extern "C" {
    // capacities of the 9 item categories
extern const u16 data_ov006_0215fffe[];
    // bag pocket of the categories 0..7
extern const int data_ov006_02160010[];
extern const short data_ov006_0215ffd2[];
extern const short data_ov006_0215ffea[];
extern const SlotItemIds data_ov006_0215ffda;
extern const char *data_020f2a38;
extern const char *data_020f2a30;
extern char data_02108760[];

Inventory *func_02010828(GameState *);
u8 *func_0200ff1c(GameState *, u8);
int func_0207c620(void *, int);
short *func_0207c5f8(void *, int);
u8 *func_0207c60c(void *, int);
void func_0205eaa0(void *, int, int);
void func_02080fa8(UiObject *ui, int id, int value);
void func_020813ec(UiObject *ui, int id);
void func_02080c20(UiObject *ui, int id);
void func_02080c04(UiObject *ui, int id);
short func_02080468(UiObject *ui, int id);
void func_020804fc(UiObject *ui, int id);
void func_0207fdcc(UiObject *ui, int id);
void func_02080f8c(UiObject *ui, int id, const char *text);
void func_02081ea4(UiObject *ui, int id, int v);
void func_0208203c(MenuInput *input);
u16 func_02081f20(MenuInput *input, int tick);
int func_ov023_021e2868(ScreenLayout *layout);
int func_ov023_021e29d0(ScreenLayout *layout);
void func_02071ffc(RecipeDatabase *db, int type, int subtype, int order, int a, short *out);
void func_020e280c(TouchCursor *cursor, int);
void func_020e2d24(void *, int, int);
void func_020e25e8(TouchCursor *cursor);
const char *func_020e0434(TextArchive *messages, int id);
void *func_020e5294(ItemResources *res, int id);
MsgWindow *func_020421a0(void);
void func_02046380(MsgWindow *w);
void func_020465c0(MsgWindow *w, int idx, int value);
void func_020e4bf4(void *out, int id);
void func_02046608(MsgWindow *w, int a, const char *msg, char *buf, int b, int c, int d);
GlyphInfo *func_0204254c(const char *s, int a);
void func_020836e4(void *dst, void *src);
Recipe *func_02071d60(RecipeDatabase *db, int id);
void func_020deef4(void *out, void *data, unsigned int size, short *ids, int count);
}

void AlchemyScreen::CountRecipePages() {
    Recipe *n = recipeList_;
    u16 count = 0;
    while (n != 0) {
        count++;
        n = n->next;
    }
    numPages_ = (count + 15) / 16;
    if (numPages_ == 0) numPages_ = 1;
    page_ = 0;
}

void AlchemyScreen::SetPageStartFor(Recipe *recipe) {
    Recipe *n     = recipeList_;
    Recipe *start = n;
    signed char i = 0;
    while (n != 0) {
        if (i % 16 == 0) {
            start = n;
            i     = 0;
        }
        if (n == recipe) break;
        i++;
        n = n->next;
    }
    pageStart_ = start;
}

Recipe *AlchemyScreen::GetPageStart(int page) {
    Recipe *n      = recipeList_;
    Recipe *result = n;
    short target   = page * 16;
    short i        = 0;
    while (n != 0) {
        if (i == target) {
            result = n;
            break;
        }
        i++;
        n = n->next;
    }
    return result;
}

// Recipe at line `index` of the page; clamps recipeCursor_ to the last one
Recipe *AlchemyScreen::GetPageRecipe(short index) {
    Recipe *n = pageStart_;
    short i   = 0;
    while (n != 0 && index != 0) {
        Recipe *next = n->next;
        if (next == 0) {
            recipeCursor_ = i + 0x29;
            break;
        }
        index--;
        i++;
        n = next;
    }
    return n;
}

// Gathers the items of the bag, the party equipment and the pockets
void AlchemyScreen::BuildIngredientLists() {
    u16 caps[9];
    memcpy(caps, data_ov006_0215fffe, sizeof(caps));
    for (u8 i = 0; i < 9; i++) {
        u16 cap = caps[i];
        memset(itemIds_[i], -1, cap * 2);
        memset(itemCounts_[i], 0, cap);
        itemTotals_[i] = 0;
    }

    GameState *gs  = GameState::GetInstance();
    Inventory *inv = func_02010828(gs);
    for (u16 i = 0; i < 0x98; i++) {
        AddIngredient(8, inv->items[i], inv->itemCounts[i]);
    }
    for (u8 i = 0; i < inv->partyCount; i++) {
        u8 *member = func_0200ff1c(gs, inv->party[i]);
        if (member != NULL) {
            short *equip = (short *) (*(u8 **) (member + 0x150) + 0x454);
            for (u8 j = 0; j < 8; j++) {
                AddIngredient(8, equip[j], 1);
            }
        }
    }
    for (u8 c = 0; c < 8; c++) {
        int pocket = data_ov006_02160010[c];
        int n      = func_0207c620(inv->bag, pocket);
        short *ids = func_0207c5f8(inv->bag, pocket);
        u8 *counts = func_0207c60c(inv->bag, pocket);
        for (short j = 0; j < n; j++) {
            AddIngredient(c, ids[j], counts[j]);
        }
    }
    book_.SetDatabase(&recipeDb_);
    book_.SetItems(itemIds_, itemCounts_, itemTotals_);
}

void AlchemyScreen::AddIngredient(u8 category, short id, u8 count) {
    if (id > 0 && count != 0) {
        short *ids = itemIds_[category];
        u8 *counts = itemCounts_[category];
        u16 *total = &itemTotals_[category];
        u16 caps[9];
        memcpy(caps, data_ov006_0215fffe, sizeof(caps));
        u16 max = caps[category];
        for (u16 i = 0; i < max; i++) {
            short cur = ids[i];
            if (cur <= 0) {
                ids[i]    = id;
                counts[i] = count;
                (*total)++;
                return;
            }
            if (cur == id) {
                counts[i] += count;
                return;
            }
        }
    }
}

// Takes the last ingredient back out of the pot
void AlchemyScreen::ReturnPendingIngredient() {
    for (signed char i = 2; i >= 0; i--) {
        if (pendingIds_[i] > 0) {
            AddIngredient(pendingCategories_[i], pendingIds_[i], pendingCounts_[i]);
            pendingCategories_[i] = 0;
            pendingIds_[i]        = -1;
            pendingCounts_[i]     = 0;
            return;
        }
    }
}

// Puts an ingredient in the pot
void AlchemyScreen::ReserveIngredient(u8 category, short id, u8 count) {
    short *ids = itemIds_[category];
    u8 *counts = itemCounts_[category];
    u16 total  = itemTotals_[category];
    for (u16 i = 0; i < total; i++) {
        if (id == ids[i]) {
            counts[i] -= count;
        }
    }
    for (u8 j = 0; j < 3; j++) {
        if (pendingIds_[j] <= 0) {
            pendingCategories_[j] = category;
            pendingIds_[j]        = id;
            pendingCounts_[j]     = count;
            return;
        }
    }
}

short *AlchemyScreen::GetCategoryIds() {
    short *result = 0;
    short cat     = categoryCursor_ - 0x5b;
    if (cat >= 0) {
        result = itemIds_[cat];
    }
    return result;
}

u8 *AlchemyScreen::GetCategoryCounts() {
    u8 *result = 0;
    short cat  = categoryCursor_ - 0x5b;
    if (cat >= 0) {
        result = itemCounts_[cat];
    }
    return result;
}

u16 AlchemyScreen::GetCategoryTotal() {
    u16 result = 0;
    short cat  = categoryCursor_ - 0x5b;
    if (cat >= 0) {
        result = itemTotals_[cat];
    }
    return result;
}

bool AlchemyScreen::HasCategoryItems() {
    short *ids  = GetCategoryIds();
    u16 n       = GetCategoryTotal();
    bool result = false;
    if (ids != 0) {
        for (u16 i = 0; i < n; i++) {
            if (ids[i] > 0) {
                result = true;
                break;
            }
        }
    }
    return result;
}

void AlchemyScreen::GetSelectedIngredient(u8 *category, short *id, u8 *count) {
    *category = 0;
    *id       = -1;
    *count    = 0;
    if (itemCursor_ < 0) {
        return;
    }
    short c    = categoryCursor_ - 0x5b;
    short *ids = GetCategoryIds();
    u8 *counts = GetCategoryCounts();
    int n      = GetCategoryTotal();
    if (ids == 0 || counts == 0 || n == 0) {
        return;
    }
    u16 idx   = (u16) (page_ << 3) + (itemCursor_ - 100);
    *category = c;
    *id       = ids[idx];
    *count    = counts[idx];
}

void AlchemyScreen::IncreaseQuantity() {
    short id;
    u8 cat;
    u8 count;
    GetSelectedIngredient(&cat, &id, &count);
    if (count > 9) {
        count = 9;
    }
    if (count != 0) {
        u8 old = quantity_;
        quantity_++;
        if (count < quantity_) {
            quantity_ = count;
        }
        if (old != quantity_) {
            quantityUp_ = 1;
        }
    }
    RefreshIngredientList();
    func_02080fa8(ui_, 0x1f, quantity_);
    func_020813ec(ui_, 6);
}

void AlchemyScreen::DecreaseQuantity() {
    short id;
    u8 cat;
    u8 count;
    GetSelectedIngredient(&cat, &id, &count);
    if (count > 9) {
        count = 9;
    }
    if (count != 0) {
        u8 old = quantity_;
        quantity_--;
        if (quantity_ == 0) {
            quantity_ = 1;
        }
        if (old != quantity_) {
            quantityDown_ = 1;
        }
    }
    RefreshIngredientList();
    func_02080fa8(ui_, 0x1f, quantity_);
    func_020813ec(ui_, 6);
}

void AlchemyScreen::OpenIngredientList() {
    func_02080c20(ui_, 0);
    flags_ |= 0x10;
    selectedItemId_ = -1;
    quantity_       = 0;
    SetupIngredientListButtons(&layout_);
    u16 n = GetCategoryTotal();
    flags_ &= ~4;
    if (n > 8) {
        flags_ |= 4;
    }
    activeElement_ = 0x11;
    itemCursor_    = -1;
    if (itemCursor_ < 0) {
        itemCursor_ = func_02080468(ui_, activeElement_);
    }
    ui_->cursor = itemCursor_;
    RefreshIngredientList();
    PreviewSelectedIngredient();
    func_0208203c(&menuInput_);
    selection_ = 0;
    func_02081ea4(ui_, activeElement_, 0);
}

void AlchemyScreen::CountIngredientPages() {
    page_     = 0;
    numPages_ = (itemTotals_[(short) (categoryCursor_ - 0x5b)] + 7) / 8;
    if (numPages_ == 0) {
        numPages_ = 1;
    }
}

bool AlchemyScreen::UpdateIngredientCursor() {
    selection_ = &itemCursor_;
    UpdateItemPage();
    if (prevCursor_ != *selection_) {
        PreviewSelectedIngredient();
        return false;
    }
    return HandleListButtons() != 0;
}

// Brew button (X) and L/R to toggle the ingredient panel
bool AlchemyScreen::HandleListButtons() {
    int id       = func_ov023_021e29d0(&layout_);
    bool confirm = id == 0x78;
    bool toggle  = id == 0x25;

    if (pendingCounts_[0] > 1 || pendingCounts_[1] >= 1) {
        confirm = confirm | g_keyManager.TestKeysPressed(KEY_X);
        if (confirm) {
            selection_ = 0;
            state_     = State_ThirdIngredient;
            subStep_   = 6;
            RefreshIngredientList();
            func_02081ea4(ui_, 0x10, 1);
            flags_ &= ~0x10;
            flags_ |= 0x20;
            messageId_   = 10;
            messageStep_ = 0;
            return true;
        }
    }
    if (confirm) {
        return true;
    }
    if (flags_ & 0x10) {
        if (g_keyManager.TestKeysPressed(KEY_L | KEY_R) || toggle) {
            pot_->SetPanelVisible(!(pot_->panel_.flags & 0x800));
            func_0205eaa0(data_02108760, 1, 0);
            return true;
        }
    }
    return false;
}

void AlchemyScreen::OpenQuantitySelect() {
    ScreenLayout::Element *e;
    e = FindLayoutElement(&layout_, 0x50);
    if (e != 0) {
        e->flags |= 1;
    }
    e = FindLayoutElement(&layout_, 0x4f);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0xe);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0xf);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0x15);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0x1f);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0x20);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0x21);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(&layout_, 0x22);
    if (e != 0) {
        e->flags &= ~1;
    }
    RefreshIngredientList();
    flags_ |= 8;
    activeElement_ = 6;
    func_02080fa8(ui_, 0x1f, quantity_);
    func_020813ec(ui_, 6);
}

// Returns 1 when the quantity changed, 2 / 3 for the buttons 0x11 / 0x12
int AlchemyScreen::HandleQuantityInput() {
    int dir;
    u8 step;
    bool b12;
    bool b11;

    b11 = false;
    b12 = b11;
    dir = b11;
    if (repeatTimer_ < 5) {
        repeatTimer_++;
    } else {
        int id = func_ov023_021e2868(&layout_);
        if (id == 0x76) dir = 1;
        if (id == 0x77) dir = -1;
        if (dir != 0) repeatTimer_ = 0;
    }
    step   = 0;
    int id = func_ov023_021e29d0(&layout_);
    if (id >= 0) {
        if (id == 0x76) dir = 1;
        if (id == 0x77) dir = -1;
        b11 = id == 0x11;
        b12 = id == 0x12;
    }
    if ((u16) (func_02081f20(&menuInput_, GameState::GetInstance()->GetTickCount()) + 0xffff) <= 1) {
        u16 v = menuInput_.GetKey();
        if (v == 0x40) dir = 1;
        if (v == 0x80) dir = -1;
        if (v == 0x20) step = 9;
        if (v == 0x10) step = 1;
    }
    if (step != 0) {
        int old   = quantity_;
        quantity_ = step;
        if (step == 9) {
            IncreaseQuantity();
        } else {
            DecreaseQuantity();
        }
        quantityUp_   = old < quantity_;
        quantityDown_ = quantity_ < old;
        return 1;
    }
    if (dir != 0) {
        if (dir > 0) {
            IncreaseQuantity();
        } else {
            DecreaseQuantity();
        }
        return 1;
    }
    if (b11) return 2;
    if (b12) return 3;
    return 0;
}

u16 MenuInput::GetKey() {
    if (key != 0) {
        return *key;
    }
    return 0;
}

// Returns 1 when the count changed, 2 / 3 for the buttons 0x4e / 0x14
int AlchemyScreen::HandleBrewCountInput() {
    bool b2   = false;
    bool b3   = false;
    int delta = 0;
    if (repeatTimer_ < 5) {
        repeatTimer_++;
    } else {
        int id = func_ov023_021e2868(&layout_);
        if (id == 0x49) delta = 1;
        if (id == 0x4a) delta = -1;
        if (delta != 0) repeatTimer_ = 0;
    }
    int set = 0;
    int id  = func_ov023_021e29d0(&layout_);
    if (id >= 0) {
        if (id == 0x49) delta = 1;
        if (id == 0x4a) delta = -1;
        b2 = id == 0x4e;
        b3 = id == 0x14;
    }
    u16 st = func_02081f20(&menuInput_, GameState::GetInstance()->GetTickCount());
    if (st == 1 || st == 2) {
        u16 k = menuInput_.GetKey();
        if (k == 0x40) delta = 1;
        if (k == 0x80) delta = -1;
        if (k == 0x20) set = 9;
        if (k == 0x10) set = 1;
    }
    if (set != 0) {
        int old   = quantity_;
        quantity_ = set;
        while (!book_.HasIngredientsFor(quantity_)) {
            quantity_--;
        }
        if (old < quantity_) quantityUp_ = 1;
        if (quantity_ < old) quantityDown_ = 1;
        RefreshBrewCount();
        ApplyQuantityToPanel();
        return 1;
    }
    if (delta != 0) {
        int old   = quantity_;
        quantity_ = old + delta;
        if (!book_.HasIngredientsFor(quantity_)) quantity_--;
        if (quantity_ == 0) quantity_ = 1;
        if (quantity_ > 9) quantity_ = 9;
        if (old < quantity_) quantityUp_ = 1;
        if (quantity_ < old) quantityDown_ = 1;
        RefreshBrewCount();
        ApplyQuantityToPanel();
        return 1;
    }
    if (b2) return 2;
    return b3 ? 3 : 0;
}

// Returns 1 back, 2 / 3 the buttons 0x41 / 0x42, 4 sort
int AlchemyScreen::PollBookAction() {
    bool select = false;
    bool decide = false;
    bool cancel = false;
    bool sort   = false;
    int hit     = func_ov023_021e29d0(&layout_);
    if (hit >= 0) {
        if (hit == 0x41) {
            select = true;
        }
        decide = hit == 0x42;
        cancel = hit == 0x13;
        sort   = hit == 0x26;
    } else {
        if (g_keyManager.TestKeysPressed(KEY_SELECT)) {
            cancel = true;
        }
        if (g_keyManager.TestKeysPressed(KEY_R | KEY_L)) {
            sort = true;
        }
    }
    if (cancel) {
        return 1;
    }
    if (select) {
        return 2;
    }
    if (decide) {
        return 3;
    }
    if (sort) {
        return 4;
    }
    return 0;
}

// Re-sorts the recipe book and keeps the cursor on the same recipe
void AlchemyScreen::ToggleSortOrder() {
    short r        = recipeCursor_ - 0x29;
    Recipe *recipe = GetPageRecipe(r);
    if (recipe == 0) return;
    if (!recipe->unlocked) {
        recipeCursor_ = -1;
        prevCursor_   = -1;
        recipe        = 0;
    }
    sortOrder_ = sortOrder_ == 0 ? 1 : 0;
    short tmp  = 0;
    func_02071ffc(&recipeDb_, filterType_, filterSubtype_, sortOrder_, 0x10, &tmp);
    pageStart_ = recipeList_ = recipeDb_.list;
    if (recipe == 0) recipe = pageStart_;
    CountRecipePages();
    func_0205eaa0(data_02108760, 1, 0);
    short i = 0;
    for (Recipe *n = pageStart_; n != 0; n = n->next) {
        if (n == recipe) break;
        i++;
    }
    r     = i % 16;
    page_ = i / 16;
    SetPageStartFor(recipe);
    recipeCursor_ = r + 0x29;
    func_020804fc(ui_, 9);
    RefreshRecipePage();
    RefreshSortButton();
}

void AlchemyScreen::OpenConfirmDialog() {
    func_0205eaa0(data_02108760, 5, 0);
    activeElement_ = 7;
    prevCursor_ = confirmCursor_ = 0x22;
    ui_->cursor                  = confirmCursor_;
    func_020813ec(ui_, activeElement_);
    func_0208203c(&menuInput_);
    selection_ = 0;
    if (cursor_ == 0) {
        return;
    }
    func_02080c04(ui_, activeElement_);
    func_020e280c(cursor_, -1);
    func_020e2d24(cursor_->data->unk_28, 0x18, 1);
}

// Returns 1 yes, -1 no / back, 0 still open
int AlchemyScreen::HandleConfirmDialog() {
    int result  = 0;
    blinkTimer_ = 0;
    selection_  = &confirmCursor_;
    if (IsConfirmPressed() || g_keyManager.TestKeysPressed(KEY_L)) {
        func_0205eaa0(data_02108760, 1, 0);
        result = -1;
        if (confirmCursor_ == 0x22) {
            result = 1;
        }
    } else if (IsBackPressed()) {
        result = -1;
    }
    if (result != 0) {
        func_0207fdcc(ui_, activeElement_);
        func_0208203c(&menuInput_);
        selection_ = 0;
    }
    if (cursor_ && result != 0) {
        func_020e25e8(cursor_);
    }
    return result;
}

void AlchemyScreen::PlayPotSound() {
    if (soundPlayed_ != 0) return;
    if (soundEnabled_ == 0) return;
    switch (soundMode_) {
        case 0: func_0205eaa0(data_02108760, 10, 0); break;
        case 1: func_0205eaa0(data_02108760, 12, 0); break;
        case 2: func_0205eaa0(data_02108760, 11, 0); break;
    }
    soundPlayed_ = 1;
}

void AlchemyScreen::ResetPotSound() {
    if (soundPlayed_ != 0 && soundEnabled_ != 0) {
        soundPlayed_ = 0;
    }
}

void AlchemyScreen::UpdatePotSound() {
    if (soundDelay_ < 0) {
        soundTimer_ = 0;
        ResetPotSound();
        return;
    }
    if (soundTimer_ > 60) {
        soundTimer_ = 0;
        ResetPotSound();
        PlayPotSound();
    }
    soundTimer_ += GameState::GetInstance()->GetEffectiveDeltaTime();
}

// Scrolls the text of messageId_ in the message window
void AlchemyScreen::UpdateMessageWindow() {
    if (messageStep_ == 0) {
        if (flags_ & 0x20) {
            func_02080c20(ui_, 0);
            func_020813ec(ui_, 0);
        }
        flags_ &= ~0x20;
        textPos_ = 0;
        if (func_020e0434(&messages_, messageId_) == 0) {
            messageId_ = -1;
        }
        soundDelay_ = -1;
        messageStep_++;
    } else if (messageStep_ == 1) {
        MsgWindow *w = func_020421a0();
        char *buf    = w->text;
        memset(buf, 0, 0x960);
        func_02046380(w);
        u16 v = 0;
        if (recipe_ != NULL) {
            void *r = func_020e5294(&itemResources_, items_[0].id);
            if (r != NULL) {
                w->unk_18 = r;
            }
            v = successRate_ / 10 * 10;
        }
        func_020465c0(w, 0, brewCount_);
        func_020465c0(w, 1, v);
        u8 tmp[0xc];
        MessageSpeaker *o = (MessageSpeaker *) GameState::GetInstance()->GetUnknownGameObject();
        if (o != NULL) {
            func_020e4bf4(tmp, o->unk_04);
            w->unk_00 = tmp;
        }
        int k = 0;
        for (const short *q = data_ov006_0215ffd2;; q++, k++) {
            short id = *q;
            if (id < 0) {
                break;
            }
            if (id == messageId_) {
                void *r = func_020e5294(&itemResources_, pendingIds_[k]);
                if (r != NULL) {
                    w->unk_18 = r;
                }
                break;
            }
        }
        const char *m = func_020e0434(&messages_, messageId_);
        func_02046608(w, 0xc, m, buf, 0xe3, 0, 1);

        char *s;
        u16 pos;
        int n;
        pos      = textPos_;
        s        = buf + pos;
        int prev = textScroll_ >> 4;
        textScroll_ += frameTime_ << 4;
        int cur = textScroll_ >> 4;
        n       = cur - prev;
        if (textScroll_ >= 0xffffff) {
            textScroll_ = 0;
        }
        for (int i = 0; i < n; i++) {
            if (*s == 0) {
                break;
            }
            int step = 1;
            if (*s == '\\' && s[1] == 'n') {
                s += 2;
                pos += 2;
            } else {
                GlyphInfo *g = func_0204254c(s, 1);
                if (g != NULL) {
                    step = g->width;
                }
                s += step;
                pos += step;
            }
        }
        if (textPos_ >= pos) {
            messageId_ = -1;
        }
        textPos_         = pos;
        char text[0x400] = {0};
        memcpy(text, buf, pos);
        func_02080f8c(ui_, 1, text);
        func_020813ec(ui_, 1);
        func_02080c20(ui_, 1);
        soundDelay_ = 1;
        for (const short *p = data_ov006_0215ffea;; p++) {
            short id = *p;
            if (id < 0) {
                break;
            }
            if (id == messageId_) {
                soundDelay_ = -1;
                break;
            }
        }
        UpdatePotSound();
    }
}

void AlchemyScreen::StartItemSpriteLoad(bool unused) {
    pot_->ResetSlots();
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    loader->AddFence();
    taskId_ = loader->QueueLoadFileInGP2(data_020f2a38, data_020f2a30, NULL);
}

// Sets the item sprites of the pot (result and ingredients) once loaded
bool AlchemyScreen::FinishItemSpriteLoad(bool useBonus) {
    u8 sprites[4][0x20];
    void *data;
    unsigned int size;
    SlotItemIds ids;

    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(taskId_)) {
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            heaps_[8].Reset();
            ids = data_ov006_0215ffda;
            for (u8 i = 0; i < 4; i++) {
                items_[i].Initialize();
                func_020836e4(&sprites[i], &items_[i]);
            }
            if (recipe_ != NULL) {
                Recipe *r = recipe_;
                if (useBonus) {
                    r = func_02071d60(&recipeDb_, r->bonusRecipe);
                }
                ids.ids[0] = items_[0].id = r->resultItem;
            }
            ids.ids[1] = items_[1].id = pendingIds_[0];
            ids.ids[2] = items_[2].id = pendingIds_[1];
            ids.ids[3] = items_[3].id = pendingIds_[2];
            if (recipe_ != NULL) {
                if (ids.ids[1] < 0) ids.ids[1] = items_[1].id = recipe_->ingredients[0];
                if (ids.ids[2] < 0) ids.ids[2] = items_[2].id = recipe_->ingredients[1];
                if (ids.ids[3] < 0) ids.ids[3] = items_[3].id = recipe_->ingredients[2];
            }
            func_020deef4(sprites, data, size, ids.ids, 4);
            if (ids.ids[1] == ids.ids[2] && ids.ids[2] > 0) func_020836e4(&sprites[2], &sprites[1]);
            if (ids.ids[1] == ids.ids[3] && ids.ids[3] > 0) func_020836e4(&sprites[3], &sprites[1]);
            if (ids.ids[2] == ids.ids[3] && ids.ids[3] > 0) func_020836e4(&sprites[3], &sprites[2]);
            for (u8 i = 0; i < 4; i++) {
                func_020836e4(&items_[i], &sprites[i]);
            }
            pot_->LoadSlotSprites(items_);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    return taskId_ == -1;
}

void AlchemyScreen::ApplyQuantityToPanel() {
    pot_->SetPanelQuantity(quantity_);
    pot_->RefreshPanel();
}
