#include "Alchemy/AlchemyScreen.h"
#include "Filesystem/BackgroundLoader.h"
#include "GameState/GameState.h"
#include "Input/Input.h"
#include "System/BGBases.h"
#include "std_library_functions.h"

// Bit field of the protagonist data read by Initialize
struct ProtagonistFlags {
    u8 unk_0[0x49c];
    u8 unk_49c_0 : 1;
};

// Button ids of the quantity buttons, ending with -1
struct QuantityButtonIds {
    short ids[5];
};

// Cursor data read by Allocate, at +0xc of the TouchCursor (TouchCursor::GetData reads +0x10)
inline TouchCursorData *GetCursorData(TouchCursor *cursor) {
    return *(TouchCursorData **) &cursor->unk_0[0xc];
}

extern "C" {
    // UiObject
void func_0207f84c(UiObject *ui);
void func_0207fc6c(UiObject *ui, int frameTime);
void func_0207fcb8(UiObject *ui);
void func_0207fd00(UiObject *ui);
void func_0207fd44(UiObject *ui);
void func_0207fd88(UiObject *ui);
void func_0207fdcc(UiObject *ui, int id);
void func_0207fe80(UiObject *ui, int a, int b, int c);
int func_020800fc(UiObject *ui, short *cursor, short prev, u16 a, u8 b, int c);
void func_020804fc(UiObject *ui, int id);
void func_020807c4(UiObject *ui, int id, short *x, short *y);
void func_020809c4(UiObject *ui, int id, int cursor, short *x, short *y);
int func_02080c3c(UiObject *ui, int id);
int func_02080d54(UiObject *ui, short id, short x, short y);
int func_02080dd4(UiObject *ui, short id, short x, short y, u8 *confirmed, int a);
void func_020813ec(UiObject *ui, int id);
UiElement *func_02081da8(UiObject *ui, int id);
void func_02081ee4(MenuInput *input, void *p);
void func_02081f20(MenuInput *input, int frameTime);

    // touch cursor
void func_020e2490(TouchCursor *cursor, int a, int b, CursorAnim *anim, SafeAllocator *alloc, int c, int d);
void func_020e263c(TouchCursor *cursor, int frameTime);
void func_020e2794(TouchCursor *cursor, SpriteBank *bank);
void func_020e2834(TouchCursor *cursor);
int func_020e28dc(TouchCursor *cursor);
void func_020e28f0(TouchCursor *cursor, int x, int y);
int func_020e2918(TouchCursor *cursor);
int func_020e2984(TouchCursor *cursor);
void func_020e2cc4(void *p, int a);

    // background layers
void func_0204af64(BgLayer *layer);
void *func_0204af14(BgLayer *layer, int a);
void func_0204b010(BgLayer *layer, int a);
void func_0204b04c(BgLayer *layer, int a);
void func_0204b088(BgLayer *layer, int a);
void func_0204b5e8(BgLayer *layer, int a, int b);
void func_0204b8d0(BgLayer *layer, int a, int b, int c, int d, int e, int f, int g, int h);
void func_0204b938(BgLayer *layer, void *data, int x, int y, int mask);
void func_0204c684(TitleBg *bg);
int func_0204c7e0(void);

    // sprites
void func_0205a198(Sprite *sprite);
void func_0205a330(CursorAnim *anim, int frameTime);
void func_0205a370(CursorAnim *anim, int a);
CursorFrame *func_0205a3d0(CursorAnim *anim, int a);
void func_0205a444(SpriteBank *bank);
void func_0205ac40(SpriteBank *bank, Sprite *sprite);
void func_0205ae8c(SpriteBank *bank);
void func_02075db0(Sprite2D *sprite, int x, int y);
void func_02076988(Sprite2D *sprite, int a, int b);
void *func_0203bd08(void);
int func_0203be4c(void *p);

    // touch buttons (overlay 23)
void func_ov023_021e20c0(ScreenLayout *layout);
void func_ov023_021e2bdc(ScreenLayout *layout, int id, short *x, short *y);
int func_ov023_021e2c38(ScreenLayout *layout, int id);

    // misc
char *func_0205ec34();
int func_0206dfb0(char *a, char *b, int c);
void func_0206df6c(char *a, char *b, int c, int d);
void func_02071be8(RecipeDatabase *db);
void func_020dfc40(TextArchive *archive);
void func_020de1d4(ItemData *item);
void func_020ac234(RecipeEntry *entries);
char *func_020421a0(void);
int func_0200fb08(GameState *gs);
int func_0209ca2c(void *p);
int func_ov017_021959b4();
void func_02012a84(TouchState *touch, int *x, int *y);

extern TouchState data_02114e54;
extern char data_02109bf4[];
extern const u16 data_ov006_0215fffe[];
extern const int data_ov006_02160054[];
extern const QuantityButtonIds data_ov006_0215fff4;
extern const short data_ov006_0215ffe2[];
extern const u8 data_ov006_0215ffc0[];
extern const short data_ov006_0215ffc4[];
extern const u8 data_ov006_0215ffb4[];
extern const short data_ov006_0215ffca[];
extern unsigned int data_ov006_02160398;
extern const AlchemyScreen::StateHandler data_020e6d5c;
extern AlchemyScreen::StateHandler data_ov006_021601fc[];
}

void AlchemyScreen::Allocate(SafeAllocator *alloc, int arg) {
    u16 caps[9];
    memcpy(caps, data_ov006_0215fffe, sizeof(caps));
    itemIds_    = (short **) alloc->Allocate(0x24);
    itemCounts_ = (u8 **) alloc->Allocate(0x24);
    itemTotals_ = (u16 *) alloc->Allocate(0x12);
    for (u8 i = 0; i < 9; i++) {
        u16 n          = caps[i];
        itemIds_[i]    = (short *) alloc->Allocate(n * 2);
        itemCounts_[i] = (u8 *) alloc->Allocate(n);
        memset(itemIds_[i], -1, n * 2);
        memset(itemCounts_[i], 0, n);
        itemTotals_[i] = 0;
    }
    buffer_        = alloc->Allocate(0x4000);
    heaps_         = (SafeAllocator *) alloc->Allocate(sizeof(SafeAllocator) * 10);
    ui_            = (UiObject *) alloc->Allocate(0x40);
    layers_        = (BgLayer *) alloc->Allocate(sizeof(BgLayer) * 2);
    titleBgs_      = (TitleBg *) alloc->Allocate(sizeof(TitleBg) * 7);
    sprites_       = (Sprite *) alloc->Allocate(sizeof(Sprite) * 29);
    cursorAnim_    = (CursorAnim *) alloc->Allocate(8);
    pot_           = (AlchemyPot *) alloc->Allocate(0x12c0);
    recipeEntries_ = (RecipeEntry *) alloc->Allocate(sizeof(RecipeEntry) * 0x1d7);
    cursor_        = (TouchCursor *) alloc->Allocate(0x24);
    func_020e2490(cursor_, 1, 1, cursorAnim_, alloc, 4, 0x40);
    GetCursorData(cursor_)->visible = 1;
    for (u8 i = 0; i < 10; i++) {
        heaps_[i].ResetAllocatorPointer();
        int size = data_ov006_02160054[i];
        heaps_[i].CreateTypeA(alloc->Allocate(size), size);
        heaps_[i].Reset();
    }
    func_0207f84c(ui_);
    for (u8 i = 0; i < 2; i++) {
        func_0204af64(&layers_[i]);
    }
    for (u8 i = 0; i < 7; i++) {
        func_0204c684(&titleBgs_[i]);
    }
    for (u8 i = 0; i < 29; i++) {
        func_0205a198(&sprites_[i]);
    }
    u16 i = 0;
    while (i < 0x1d7) {
        RecipeEntry *e = &recipeEntries_[i];
        e->id          = -1;
        e->crafted     = 0;
        e->known       = 0;
        e->unk_2_2     = 0;
        i++;
    }
    pot_->Initialize();
    pot_->Allocate(alloc, arg);
    pot_->bgBuffer_      = buffer_;
    pot_->itemResources_ = &itemResources_;
    pot_->messages_      = &messages_;
    if (flags_ & 2) {
        pot_->Freeze();
    }
    func_020ac234(recipeEntries_);
    recipeNames_ = (char **) alloc->Allocate(0x40);
    for (int i = 0; i < 16; i++) {
        recipeNames_[i] = (char *) alloc->Allocate(0x80);
        memset(recipeNames_[i], 0, 0x80);
    }
}

void AlchemyScreen::Initialize() {
    char *s = func_0205ec34();
    if (!func_0206dfb0(s, s + 0x8c, 0x777)) {
        func_0206df6c(s, s + 0x8c, 0x777, 1);
    }
    textScroll_       = 0;
    showResultSprite_ = 0;
    resultSpriteTask_ = -1;
    waitFrames_       = 0;
    nextSubStep_      = 0;
    buffer_           = 0;
    heaps_            = 0;
    pot_              = 0;
    ui_               = 0;
    cursor_           = 0;
    layers_           = 0;
    titleBgs_         = 0;
    sprites_          = 0;
    cursorAnim_       = 0;
    recipeList_       = 0;
    pageStart_        = 0;
    recipe_           = 0;
    recipeEntries_    = 0;
    selection_        = 0;
    itemIds_          = 0;
    itemCounts_       = 0;
    itemTotals_       = 0;
    func_0204af64(&bgLayer_);
    book_.Initialize();
    func_02081ee4(&menuInput_, &unk_a8);
    func_0205a444(&spriteBank_);
    func_ov023_021e20c0(&layout_);
    func_02071be8(&recipeDb_);
    func_020dfc40(&messages_);
    for (u8 i = 0; i < 4; i++) {
        items_[i].Initialize();
    }
    scrollKeys_           = 0;
    taskId_               = -1;
    unk_358               = -1;
    unk_35c               = 0;
    prevCursor_           = -1;
    menuCursor_           = -1;
    categoryCursor_       = -1;
    itemCursor_           = -1;
    filterCursor_         = -1;
    subFilterCursor_      = -1;
    recipeCursor_         = -1;
    confirmCursor_        = -1;
    activeElement_        = -1;
    selectedItemId_       = -1;
    pendingCategories_[0] = pendingCategories_[1] = pendingCategories_[2] = 0;
    pendingIds_[0] = pendingIds_[1] = pendingIds_[2] = -1;
    messageId_                                       = -1;
    textPos_                                         = 0;
    successRate_                                     = 0;
    filterType_                                      = -1;
    filterSubtype_                                   = -1;
    sortOrder_                                       = 0;
    brewCount_                                       = 0;
    page_                                            = 0;
    numPages_                                        = 0;
    selectedCategory_                                = 0;
    quantity_                                        = 0;
    pendingCounts_[0] = pendingCounts_[1] = pendingCounts_[2] = 0;
    state_                                                    = 0;
    subStep_                                                  = 0;
    unk_391                                                   = 0;
    messageStep_                                              = 0;
    repeatTimer_                                              = 0;
    flags_                                                    = 0;
    unk_396                                                   = 0;
    topBgIndex_                                               = 0;
    ProtagonistFlags *p = *(ProtagonistFlags **) ((char *) GameState::GetInstance()->GetProtagonist() + 0x150);
    unk_396             = p->unk_49c_0;
    soundMode_          = 0;
    soundEnabled_       = 1;
    soundTimer_         = 0;
    soundPlayed_        = 0;
    soundDelay_         = -1;
    unk_42f             = 0;
    quantityUp_ = quantityDown_ = 0;
    showPanel_                  = 0;
    exitRequested_              = 0;
    exitLatched_                = 0;
    blinkTimer_                 = 0;
}

void ItemData::Initialize() {
    func_020de1d4(this);
    memset(buffer20, 0, sizeof(buffer20));
    memset(buffer40, 0, sizeof(buffer40));
    unk_0 = buffer20;
    unk_4 = buffer40;
    id    = -1;
}

void AlchemyScreen::Destroy() {
    if (pot_ != 0) {
        pot_->Destroy();
        pot_ = 0;
    }
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (taskId_ >= 0) {
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    if (unk_358 >= 0) {
        loader->RemoveTask(unk_358);
        unk_358 = -1;
    }
    book_.Reset();
    *(int *) (func_020421a0() + 0x2d8) = 0;
    if (heaps_ == 0) {
        return;
    }
    for (u8 i = 0; i < 10; i++) {
        if (heaps_[i].GetSignedAllocator() != 0) {
            heaps_[i].Destroy();
        }
    }
}

// Runs one frame: pot, message window, menu input, then the current state.
// Returns 1 once the screen is closed.
int AlchemyScreen::Update(int frameTime) {
    frameTime_ = frameTime;
    if (func_ov017_021959b4()) RequestExit();
    if (!(flags_ & 0x200) && exitRequested_) {
        if (selection_) *selection_ = -1;
        selection_     = 0;
        state_         = State_FadeOut;
        subStep_       = 0;
        exitRequested_ = 0;
    }
    if ((state_ <= 4) <= state_) {
        blinkTimer_ = blinkTimer_ + frameTime;
        blinkTimer_ = blinkTimer_ & 0x3f;
    }
    if (func_0209ca2c(data_02109bf4) == 0) unk_42f = 0;
    pot_->Update(frameTime);
    UiObject *ui = ui_;
    if (ui) func_0207fc6c(ui, frameTime);
    if (messageId_ >= 0) {
        blinkTimer_ = 0;
        UpdateMessageWindow();
        return 0;
    }
    if (flags_ & 0x100) {
        if (IsAnyKeyPressed()) {
            func_0207fdcc(ui, 0);
            func_0207fdcc(ui, 1);
            flags_ &= ~0x100;
        }
        return 0;
    }
    if (pot_->flags_ & 0x380) {
        blinkTimer_ = 0;
        return 0;
    }
    if (selection_) {
        func_02081f20(&menuInput_, frameTime);
        prevCursor_ = *selection_;
        scrollKeys_ = func_020800fc(ui, selection_, prevCursor_, unk_a8, unk_aa, (flags_ & 4) ? 1 : 0);
        ui->cursor  = *selection_;
        if (scrollKeys_) func_020804fc(ui, activeElement_);
        if (prevCursor_ != *selection_) func_020813ec(ui, activeElement_);
    }
    if (cursor_ && func_020e28dc(cursor_)) func_020e263c(cursor_, frameTime_);
    if (!(data_ov006_02160398 & 1)) {
        data_ov006_021601fc[State_Exit] = data_020e6d5c;
        data_ov006_02160398 |= 1;
    }
    if (data_ov006_021601fc[state_] == 0) return 1;
    (this->*data_ov006_021601fc[state_])();
    return 0;
}

void AlchemyScreen::DrawModel() {
    if (state_ == 0) {
        return;
    }
    pot_->Draw3D();
}

void DrawSpriteAt(SpriteBank *bank, Sprite *sprite, int x, int y, u8 a, u8 b) {
    sprite->x      = x << 12;
    sprite->y      = y << 12;
    sprite->unk_22 = a;
    sprite->unk_26 = b;
    func_0205ac40(bank, sprite);
}

bool IsUiElementShown(UiObject *ui, int id) {
    if (func_02081da8(ui, id) != 0 && func_02080c3c(ui, id) != 0) {
        return true;
    }
    return false;
}

// Bottom screen: menu sprites, backgrounds, UI, cursor and the result sprite
void AlchemyScreen::DrawMenu() {
    if (state_ != 0) {
        if (IsUiElementShown(ui_, 0)) {
            DrawSpriteAt(&spriteBank_, &sprites_[26], 2, 0x61, 100, 1);
        }
        if (IsUiElementShown(ui_, 1)) {
            DrawSpriteAt(&spriteBank_, &sprites_[27], 2, 0x74, 0x66, 1);
            if (blinkTimer_ > 0x1f) {
                DrawSpriteAt(&spriteBank_, &sprites_[28], 0x78, 0xb6, 0x63, 1);
            }
        } else {
            blinkTimer_ = 0;
        }
        pot_->DrawSlotSprites();
        func_0204b010(&bgLayer_, 0);
        void *data = func_0204af14(&bgLayer_, topBgIndex_);
        if (data) {
            func_0204b938(&bgLayer_, data, 0, 0, 0xffff);
        }
        if (topBgIndex_ == 1) {
            int region        = func_0200fb08(GameState::GetInstance());
            unsigned int lang = 1;
            switch (region) {
                case 2: lang = 3; break;
                case 4: lang = 4; break;
                case 3: lang = 5; break;
                case 5: lang = 6; break;
            }
            if (lang > 1) {
                func_0204b8d0(&bgLayer_, lang, 0, 0, 0x16, 2, 9, 2, 0xffff);
            }
        }
        UiObject *ui = ui_;
        func_0207fcb8(ui);
        func_0207fd00(ui);
        func_0207fe80(ui, 2, 1, 1);
        func_0204b04c(&bgLayer_, 0);
        func_0207fd44(ui);
        if (flags_ & 0x200) {
            blinkTimer_ = 0;
            func_0205a370(cursorAnim_, 1);
            CursorFrame *f = func_0205a3d0(cursorAnim_, 0);
            if (f) f->flags &= ~8;
            f = func_0205a3d0(cursorAnim_, 1);
            if (f) f->flags |= 8;
            func_0205a330(cursorAnim_, frameTime_);
            f = func_0205a3d0(cursorAnim_, 1);
            if (f) {
                f->x = 0xd7;
                f->y = 0x96;
            }
            func_0205ae8c(&spriteBank_);
        } else {
            CursorFrame *f = func_0205a3d0(cursorAnim_, 1);
            if (f) f->flags &= ~8;
        }
        if (messageId_ < 0) {
            if (cursor_ && func_020e28dc(cursor_)) {
                func_020e2794(cursor_, &spriteBank_);
            } else {
                DrawCursor();
            }
        }
        DrawListButtons();
        DrawQuantityButtons();
        DrawBookButtons();
        quantityDown_ = 0;
        quantityUp_   = 0;
    }
    if (showResultSprite_ != 0 && (state_ == State_FreeBrewing || state_ == State_RecipeBrewing)) {
        short x, y;
        func_020807c4(ui_, 0x12, &x, &y);
        Sprite *s = &sprites_[17];
        if (s) {
            s->x      = (x + 4) << 12;
            s->y      = (y + 4) << 12;
            s->unk_22 = 0x32;
            s->unk_26 = 1;
            func_0205ac40(&spriteBank_, s);
        }
        int vram                     = func_0203be4c(func_0203bd08()) + 0x1b8;
        resultSprite_.unk_4c         = 0;
        *(int *) &resultSprite_.vram = vram;
        resultSprite_.unk_44         = 0x37;
        func_02076988(&resultSprite_, 0x1000, 0x1000);
        func_02075db0(&resultSprite_, x + 8, y + 8);
    }
}

// Top screen: element 1 of the UI, the pot panel and the touch cursor
void AlchemyScreen::DrawInfo() {
    if (state_ == 0) return;
    u32 base = GetSubBG0ScreenBase();
    memset((void *) base, 0, 0x800);
    if (IsUiElementShown(ui_, 1)) {
        short x;
        short y;
        void *data;
        UiElement *e = func_02081da8(ui_, 1);
        if (e == NULL) return;
        data = e->tiles;
        if (data == NULL) return;
        x = e->x;
        y = e->y;
        BgLayer layer;
        func_0204af64(&layer);
        layer.unk_1c_0   = 1;
        layer.index      = 0;
        layer.screenBase = base;
        func_0204b5e8(&layer, 0, 0);
        func_0204b938(&layer, data, x, y, 0xffff);
    }
    pot_->Draw();
    if (showPanel_) {
        pot_->EnterPotView();
        pot_->SetPanelVisible(true);
        showPanel_ = 0;
    }
    func_0204b088(&bgLayer_, 0);
    if (cursor_) {
        u8 *p = (u8 *) cursor_->GetData();
        func_020e2cc4(p + 0x28, func_020e28dc(cursor_));
        func_020e2834(cursor_);
    }
    if (ui_) func_0207fd88(ui_);
}

void AlchemyScreen::RequestExit() {
    if (exitLatched_ == 0) {
        exitLatched_   = 1;
        exitRequested_ = 1;
    }
}

void AlchemyScreen::DrawCursor() {
    if (state_ == 0 || selection_ == NULL) return;
    if (activeElement_ < 0) return;
    if (!func_02081da8(ui_, activeElement_)) return;
    if (func_0204c7e0() == 0 || cursor_ == NULL) return;
    if (func_020e28dc(cursor_) != 0) return;
    short x, y;
    func_020809c4(ui_, activeElement_, *selection_, &x, &y);
    func_020e263c(cursor_, frameTime_);
    x -= 0x10;
    y -= 3;
    func_020e28f0(cursor_, x, y);
    func_0205ae8c(&spriteBank_);
}

// Buttons of the ingredient list: brew, back and panel toggle
void AlchemyScreen::DrawListButtons() {
    if (!(flags_ & 0x10)) return;
    short x, y;
    Sprite *s;

    func_ov023_021e2bdc(&layout_, 0x78, &x, &y);
    s         = &sprites_[10];
    s->x      = x << 12;
    s->y      = y << 12;
    s->unk_22 = 12;
    s->unk_25 = 9;
    s->unk_26 = 2;
    if (pendingCounts_[0] > 1 || pendingCounts_[1] >= 1) {
        s->unk_25 = 8;
    }
    func_0205ac40(&spriteBank_, s);

    func_ov023_021e2bdc(&layout_, 0x10, &x, &y);
    s         = &sprites_[9];
    s->x      = x << 12;
    s->y      = y << 12;
    s->unk_22 = 14;
    s->unk_26 = 2;
    func_0205ac40(&spriteBank_, s);

    func_ov023_021e2bdc(&layout_, 0x25, &x, &y);
    s = &sprites_[15];
    if (!(pot_->panel_.flags & 0x800)) {
        s = &sprites_[16];
    }
    s->x      = x << 12;
    s->y      = y << 12;
    s->unk_22 = 16;
    s->unk_26 = 2;
    func_0205ac40(&spriteBank_, s);
}

// Quantity buttons (+/-, 1, 9, OK), drawn pressed for one frame on change
void AlchemyScreen::DrawQuantityButtons() {
    if (!(flags_ & 8)) return;
    short ids[5];
    short x, y;
    memcpy(ids, &data_ov006_0215fff4, sizeof ids);
    if (state_ == State_RecipeList) {
        ids[0] = 0x1f;
        ids[1] = 0x20;
        ids[2] = 0x21;
        ids[3] = 0x22;
    }
    for (u8 i = 0; ids[i] >= 0; i++) {
        func_ov023_021e2bdc(&layout_, ids[i], &x, &y);
        short idx = data_ov006_0215ffe2[i];
        Sprite *s = &sprites_[idx];
        if (idx == 11 && quantityUp_) {
            x++;
            y++;
        }
        if (idx == 12 && quantityDown_) {
            x++;
            y++;
        }
        s->x      = x << 12;
        s->y      = y << 12;
        s->unk_22 = data_ov006_0215ffc0[i];
        func_0205ac40(&spriteBank_, s);
    }
}

void AlchemyScreen::DrawBookButtons() {
    if (!(flags_ & 0x80)) return;
    if (func_02081da8(ui_, 1) != 0) return;
    for (u8 i = 0; data_ov006_0215ffca[i] >= 0; i++) {
        short id = data_ov006_0215ffca[i];
        if (func_ov023_021e2c38(&layout_, id) != 0) {
            short x, y;
            func_ov023_021e2bdc(&layout_, id, &x, &y);
            Sprite *s;
            short idx = data_ov006_0215ffc4[i];
            s         = &sprites_[idx];
            if (i == 2 && !(pot_->panel_.flags & 0x800)) {
                s = &sprites_[idx + 1];
            }
            s->x      = x << 12;
            s->y      = y << 12;
            s->unk_22 = data_ov006_0215ffb4[i];
            s->unk_26 = 2;
            if (idx == 8) {
                s->unk_25 = 11;
            } else if (idx == 9) {
                s->unk_25 = 1;
            }
            func_0205ac40(&spriteBank_, s);
        }
    }
}

u8 AlchemyScreen::IsConfirmPressed() {
    u8 confirmed = 0;
    if (g_keyManager.TestKeysPressed(KEY_A)) {
        confirmed = 1;
    }
    if (flags_ & 2) {
        if (g_keyManager.TestKeysPressed(KEY_X)) {
            confirmed = 1;
        }
    }
    if (cursor_ && func_020e28dc(cursor_)) {
        int idx = func_020e2918(cursor_);
        if (idx >= 0) {
            confirmed   = 1;
            *selection_ = idx + 0x22;
        }
    } else if (data_02114e54.touched && selection_) {
        int x, y;
        func_02012a84(&data_02114e54, &x, &y);
        prevCursor_ = *selection_;
        int sel     = func_02080dd4(ui_, activeElement_, x, y, &confirmed, 1);
        if (sel < 0) {
            return 0;
        }
        *selection_ = sel;
        if (prevCursor_ != *selection_) {
            ui_->cursor = *selection_;
            func_020813ec(ui_, activeElement_);
        }
    }
    return confirmed;
}

int AlchemyScreen::IsBackPressed() {
    int result = 0;
    if (g_keyManager.TestKeysPressed(KEY_B)) {
        result = 1;
    }
    if (cursor_ && func_020e28dc(cursor_)) {
        if (func_020e2984(cursor_)) {
            result = 1;
        }
    } else if (data_02114e54.touched) {
        int x, y;
        func_02012a84(&data_02114e54, &x, &y);
        if (func_02080d54(ui_, activeElement_, x, y) == 0) {
            result = 1;
        }
    }
    return result;
}

// Y closes the screen
void AlchemyScreen::HandleCloseKey() {
    if (g_keyManager.TestKeysPressed(KEY_Y)) {
        if (selection_) {
            *selection_ = -1;
        }
        selection_ = 0;
        state_     = State_FadeOut;
        subStep_   = 0;
    }
}

int AlchemyScreen::IsAnyKeyPressed() {
    int result;
    u8 touched;
    touched = data_02114e54.touched;
    result  = 0;
    if (g_keyManager.TestKeysPressed(KEY_A | KEY_B | KEY_RIGHT | KEY_LEFT | KEY_UP | KEY_DOWN | KEY_R | KEY_L | KEY_X) ||
        touched)
    {
        result = 1;
    }
    return result;
}
