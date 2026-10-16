#include "Alchemy/AlchemyScreen.h"
#include "Filesystem/BackgroundLoader.h"
#include "GameState/GameState.h"
#include "Input/Input.h"
#include "Resource/Brightness.h"
#include "Resource/GameResources.h"
#include "System/BGBases.h"
#include "System/ColorEffects.h"
#include "System/Graphics.h"
#include "Util/Random.h"
#include "std_library_functions.h"

#define BG0OFSSUB (*(volatile unsigned int *) 0x04001010)

// Background index of the pot per item category (data_ov006_02160030)
struct CategoryBgTable {
    int v[9];
};

extern "C" {
GameResources *func_0200fb8c(GameState *state);
int func_020100a8(GameState *state);
void func_0209c3b4(void *sound, int a);
void func_0209c678(void *sound, int a);
void func_0209c830(void *sound, int id);
void func_0205eaa0(void *sound, int a, int b);
void G3X_SetFog(int, int, int, int);
void G3X_SetClearColor(int, int, int, int, int);
void *func_02094a00();
void func_02094b40(void *p);
void func_02094b34(void *p, int a, int b, int c, int d);
int func_02094b4c(void *p);
void func_020dfc40(TextArchive *archive);
void func_020dfec0(TextArchive *archive, SafeAllocator *alloc, void *file, unsigned int size);
const char *func_020e0434(TextArchive *archive, int id);
void func_02071be8(RecipeDatabase *db);
void func_02071c00(RecipeDatabase *db, SafeAllocator *alloc, void *file, unsigned int size);
void func_02071ffc(RecipeDatabase *db, int type, int subtype, int order, int a, short *out);
Recipe *func_02071da4(RecipeDatabase *db, short *ids, u8 *counts, u8 *outCount);
void func_0204af38(BgLayer *layer, int a, SafeAllocator *alloc);
void func_0204af64(BgLayer *layer);
void func_0204b0e8(BgLayer *layer, int a);
void func_0204b11c(BgLayer *layer, int a);
void func_0204b12c(BgLayer *layer, SafeAllocator *alloc);
void func_0204b174(BgLayer *layer, void *data, SafeAllocator *alloc, unsigned int size);
void func_0204b5b4(BgLayer *layer, int a);
void func_0204b5e8(BgLayer *layer, int a, int b);
void func_0204c684(TitleBg *bg);
void func_0204c7a8(TitleBg *bg, SafeAllocator *alloc, void *buffer, int size);
int func_02046900(void *archive);
void *func_020467f0(void *archive, int index, void *outUnk, void *outSize);
void func_ov023_021e20c0(ScreenLayout *layout);
void func_ov023_021e20f0(ScreenLayout *layout, SafeAllocator *alloc, void *file, unsigned int size);
void func_0205a444(SpriteBank *bank);
void func_0205a528(SpriteBank *bank, void *data, int size, SafeAllocator *alloc);
void func_020e526c(ItemResources *res);
void func_020e5604(ItemResources *res, SafeAllocator *alloc, void *file, unsigned int size);
void **func_020e5294(ItemResources *res, int id);
void func_020e4864(void *src, char *dst, int a, int b, int c, int d);
void func_0207f7f0(UiObject *ui, TitleBg *bgs, int count);
void func_0207f84c(UiObject *ui);
void func_0207f914(UiObject *ui, SafeAllocator *alloc, const char *gp2, const char *name);
int func_0207f9f4(UiObject *ui);
void func_0207fdcc(UiObject *ui, int id);
void func_0207fe44(UiObject *ui);
short func_02080468(UiObject *ui, int id);
void func_020804fc(UiObject *ui, int id);
void func_020805f4(UiObject *ui, int id);
void func_02080608(UiObject *ui, int id);
void func_0208065c(UiObject *ui, int id);
void func_020806b0(UiObject *ui, int id);
void func_020806c4(UiObject *ui, int id);
void func_020806d8(UiObject *ui, int a, int b, int c, int d);
void func_02080798(UiObject *ui, int id, int v);
void func_02080b2c(UiObject *ui, int id);
void func_02080b40(UiObject *ui, int id);
void func_02080b54(UiObject *ui, int id);
void func_02080bac(UiObject *ui, int id);
void func_02080c04(UiObject *ui, int id);
void func_02080c20(UiObject *ui, int id);
void func_02080c68(UiObject *ui, int id, int v);
void func_02080cc0(UiObject *ui, int id, int v);
void func_02080f8c(UiObject *ui, int id, const char *text);
void func_02080fa8(UiObject *ui, int id, int value);
void func_0208103c(UiObject *ui, int id, int v);
void func_0208108c(UiObject *ui, int id);
void func_020810a0(UiObject *ui, int a);
void func_02081130(UiObject *ui, int id, int v);
void func_02081164(UiObject *ui, int id, int v);
void func_020813ec(UiObject *ui, int id);
UiElement *func_02081da8(UiObject *ui, int id);
void func_02081ea4(UiObject *ui, int id, int v);
void func_0208203c(MenuInput *input);
void func_020e25e8(TouchCursor *cursor);
void *func_020421a0();
void func_02046608(void *p, int a, const char *text, char *buf, int b, int c, int d);
u32 func_020dd4c4(int a, ItemData *item);
int func_020de234(ItemData *item, int a);
void *func_020d6c00();
void func_020466e4(void *p, int a);
void func_020466f4(void *p, int a);
void func_020a9ea4(void *p);
void *func_0202ae18();
int func_0202c540(void *p);
int func_020aad1c(void *p, void *save, int n, int a);
void func_02075cdc(Sprite2D *sprite);
void func_02076080(Sprite2D *sprite, SafeAllocator *alloc, void *data, unsigned int size);
void *func_0203bd08();
char *func_0203be4c(void *p);

extern char data_02108760[];
extern char data_02109bf4[];
extern unsigned char data_0211e33c[];
extern const CategoryBgTable data_ov006_02160030;
extern char data_ov006_0216026c[];
extern char data_ov006_02160286[];
extern char data_ov006_02160297[];
extern char data_ov006_021602ab[];
extern char data_ov006_021602bb[];
extern char data_ov006_021602d2[];
extern char data_ov006_021602e7[];
extern char data_ov006_021602f8[];
extern char data_ov006_0216030a[];
extern char data_ov006_02160318[];
extern char data_ov006_0216032e[];
extern char data_ov006_02160340[];
extern char data_ov006_02160359[];
extern char data_ov006_02160360[];
}

void AlchemyScreen::UpdateLoad() {
    func_0200fb8c(GameState::GetInstance());
    BackgroundLoader *loader = BackgroundLoader::GetInstance();

    if (subStep_ == 0) {
        if (!(flags_ & 2)) {
            func_0209c3b4(data_02109bf4, 8);
        }
        G3X_SetFog(0, 0, 0, 0);
        G3X_SetClearColor(0, 0, 0x7fff, 0, 0);
        void *p = func_02094a00();
        func_02094b40(p);
        func_02094b34(p, 0x6c, 0x1fd, 0, 0);
        subStep_++;
    } else if (subStep_ == 1) {
        if (func_02094b4c(func_02094a00())) {
            subStep_++;
        }
    } else if (subStep_ == 2) {
        taskId_ = loader->QueueLoadFileInGP2(data_ov006_0216026c, data_ov006_02160286, NULL);
        subStep_++;
    } else if (subStep_ == 3) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data) {
            SafeAllocator *heaps = heaps_;
            heaps[1].Reset();
            func_020dfc40(&messages_);
            func_020dfec0(&messages_, &heaps[1], data, size);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 4) {
        taskId_ = loader->QueueLoadFileInGP2(data_ov006_02160297, data_ov006_021602ab, NULL);
        subStep_++;
    } else if (subStep_ == 5) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data) {
            SafeAllocator *heaps = heaps_;
            heaps[0].Reset();
            func_02071be8(&recipeDb_);
            func_02071c00(&recipeDb_, heaps, data, size);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 6) {
        BG0CNTSUB = (BG0CNTSUB & 0x43) | 0x1010;
        BG1CNTSUB = (BG1CNTSUB & 0x43) | 0x1110;
        BG2CNTSUB = (BG2CNTSUB & 0x43) | 0x1210;
        BG3CNTSUB = (BG3CNTSUB & 0x43) | 0x1300;
        memset((void *) GetSubBG0ScreenBase(), 0, 0x800);
        BG0OFSSUB = 0;
        heaps_[3].Reset();
        func_0204af64(&bgLayer_);
        func_0204b11c(&bgLayer_, 0);
        bgLayer_.unk_1c_0 = 1;
        bgLayer_.index    = 3;
        func_0204b5b4(&bgLayer_, 3);
        func_0204b5e8(&bgLayer_, 0, 0);
        func_0204b12c(&bgLayer_, &heaps_[3]);
        func_0204af38(&bgLayer_, 7, &heaps_[3]);
        taskId_ = loader->QueueLoadFile(data_ov006_021602bb, NULL);
        subStep_++;
    } else if (subStep_ == 7) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        int a;
        void *data;
        unsigned int size;
        int b;
        void *r;
        int n;
        int i;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        n = func_02046900(data);
        for (i = 0; i < n; i++) {
            r = func_020467f0(data, i, &a, &b);
            if (r) {
                if (i == 0) {
                    for (int j = 0; j < 2; j++) {
                        BgLayer *l = &layers_[j];
                        func_0204af64(l);
                        func_0204b11c(l, 0);
                        l->unk_1c_0 = 1;
                        l->index    = j + 1;
                        func_0204b5e8(l, 0, 0);
                        func_0204b5b4(l, j + 1);
                        func_0204b12c(l, &heaps_[3]);
                        func_0204b174(l, r, &heaps_[3], b);
                    }
                } else {
                    func_0204b174(&bgLayer_, r, &heaps_[3], b);
                }
            }
        }
        func_0204b0e8(&bgLayer_, 0);
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 8) {
        SafeAllocator *heaps = heaps_;
        heaps[4].Reset();
        for (u8 i = 0; i < 7; i++) {
            TitleBg *bg = &titleBgs_[i];
            func_0204c684(bg);
            func_0204c7a8(bg, &heaps[4], buffer_, 0x600);
            bg->layers = layers_;
        }
        taskId_ = loader->QueueLoadFileInGP2(data_ov006_021602d2, data_ov006_021602e7, NULL);
        subStep_++;
    } else if (subStep_ == 9) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data) {
            SafeAllocator *heaps = heaps_;
            heaps[6].Reset();
            func_ov023_021e20c0(&layout_);
            func_ov023_021e20f0(&layout_, &heaps[6], data, size);
            ScreenLayout::Element *e = FindLayoutElement(&layout_, 1);
            if (e) {
                e->flags &= ~1;
            }
            e = FindLayoutElement(&layout_, 0x23);
            if (e) {
                e->flags &= ~1;
            }
            e = FindLayoutElement(&layout_, 0x24);
            if (e) {
                e->flags &= ~1;
            }
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 10) {
        func_0205a444(&spriteBank_);
        spriteBank_.unk_50     = 1;
        spriteBank_.sprites    = sprites_;
        spriteBank_.numSprites = 0x1d;
        spriteBank_.unk_3c     = *(void **) &cursor_->unk_14[0xc];
        taskId_                = loader->QueueLoadFileInGP2(data_ov006_021602f8, data_ov006_0216030a, NULL);
        subStep_++;
    } else if (subStep_ == 11) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        int a;
        void *data;
        unsigned int size;
        int b;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        SafeAllocator *heaps;
        int n = func_02046900(data);
        heaps = heaps_;
        heaps[7].Reset();
        for (int i = 0; i < n; i++) {
            void *r = func_020467f0(data, i, &a, &b);
            if (r) {
                func_0205a528(&spriteBank_, r, b, &heaps[7]);
            }
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
        for (int i = 0; i < 8; i++) {
            sprites_[i + 0x12].unk_22 = 0x18;
        }
    } else if (subStep_ == 12) {
        taskId_ = loader->QueueLoadFileInGP2(data_ov006_02160318, data_ov006_0216032e, NULL);
        subStep_++;
    } else if (subStep_ == 13) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data) {
            SafeAllocator *heaps = heaps_;
            heaps[2].Reset();
            func_020e526c(&itemResources_);
            func_020e5604(&itemResources_, &heaps[2], data, size);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 14) {
        SafeAllocator *heaps = heaps_;
        heaps[5].Reset();
        func_0207f84c(ui_);
        func_0207f914(ui_, &heaps[5], data_ov006_02160340, data_ov006_02160359);
        subStep_++;
    } else if (subStep_ == 15) {
        int r = func_0207f9f4(ui_);
        if (r == 0) {
            subStep_++;
        }
        if (r < 0) {
            state_ = State_FadeOut;
        }
    } else if (subStep_ == 16) {
        func_020810a0(ui_, 1);
        UiObject *ui = ui_;
        ui->layers   = layers_;
        ui->unk_38   = 2;
        func_0207f7f0(ui_, titleBgs_, 7);
        ui_->unk_3a = 1;
        func_0208108c(ui_, 0x20);
        func_0208108c(ui_, 0x3e);
        BG0CNTSUB  = BG0CNTSUB & ~3;
        BG1CNTSUB  = (BG1CNTSUB & ~3) | 1;
        BG2CNTSUB  = (BG2CNTSUB & ~3) | 2;
        BG3CNTSUB  = (BG3CNTSUB & ~3) | 3;
        DISPCNTSUB = (DISPCNTSUB & ~0x1f00) | 0x1f00;
        ColorEffect_ConfigureAlphaBlend(0x4001050, 4, 0x18, 10, 6);
        BuildIngredientLists();
        state_   = State_FadeIn;
        subStep_ = 0;
        book_.ApplyUnlockFlags(&recipeDb_, recipeEntries_, 0x1d7);
        func_02071ffc(&recipeDb_, -1, -1, 0, 0, &unk_35c);
        short tmp = 0;
        func_02071ffc(&recipeDb_, filterType_, filterSubtype_, sortOrder_, 0x10, &tmp);
        pageStart_ = recipeList_ = recipeDb_.list;
        CountRecipePages();
        book_.SetEntries(recipeEntries_, 0x1d7);
        book_.SetDatabase(&recipeDb_);
        if (!(flags_ & 2)) {
            func_02081164(ui_, 0, 1);
            func_02081164(ui_, 1, 1);
            func_020813ec(ui_, 0);
            func_020813ec(ui_, 1);
            UiElement *e = func_02081da8(ui_, 1);
            if (e) {
                e->unk_b8 = 0xc;
                e->unk_ba = 0x10;
            }
            char buf[0x400] = {0};
            void *p         = func_020421a0();
            func_02046608(p, 0xc, func_020e0434(&messages_, 0), buf, 0xe3, 0, 1);
            func_02080f8c(ui_, 1, buf);
            func_020813ec(ui_, 1);
        }
    }
}

void AlchemyScreen::UpdateFadeIn() {
    GameResources *res = func_0200fb8c(GameState::GetInstance());
    if (subStep_ == 0) {
        if (pot_->flags_ & 0x1008) {
            subStep_++;
        }
    } else if (subStep_ == 1) {
        SetBrightness(res, 0, 8);
        subStep_++;
    } else if (subStep_ == 2) {
        if (!IsBrightnessTransitionActive(res)) {
            state_   = State_TopMenu;
            subStep_ = 0;
            if (flags_ & 2) {
                state_ = topBgIndex_ = State_BookFilter;
            }
        }
    }
}

// Recipe book only: choice of the recipe filter
void AlchemyScreen::UpdateBookFilter() {
    if (subStep_ == 0) {
        topBgIndex_ = 2;
        RefreshCompletionRate(true);
        activeElement_ = 8;
        if (filterCursor_ < 0) {
            filterCursor_ = 0x24;
        }
        ui_->cursor = filterCursor_;
        func_020813ec(ui_, activeElement_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 1) {
        selection_ = &filterCursor_;
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            bool done        = false;
            selection_       = 0;
            subFilterCursor_ = -1;
            recipeCursor_    = -1;
            filterType_      = -1;
            filterSubtype_   = -1;
            switch (filterCursor_) {
                case 0x24: done = true; break;
                case 0x25:
                    filterType_ = 0;
                    subStep_++;
                    break;
                case 0x26: subStep_++; break;
                case 0x27:
                    filterType_ = 7;
                    done        = true;
                    break;
                case 0x28:
                    filterType_ = 8;
                    done        = true;
                    break;
            }
            if (done) {
                state_   = State_BookList;
                subStep_ = 0;
                SetupBookListButtons(&layout_);
                flags_ |= 0x80;
            }
            short tmp = 0;
            func_02071ffc(&recipeDb_, filterType_, filterSubtype_, sortOrder_, 0x10, &tmp);
            pageStart_ = recipeList_ = recipeDb_.list;
            CountRecipePages();
            func_0207fdcc(ui_, 8);
        } else if (IsBackPressed() || g_keyManager.TestKeysPressed(KEY_Y)) {
            *selection_ = -1;
            selection_  = 0;
            state_      = State_FadeOut;
            subStep_    = 0;
        }
    } else if (subStep_ == 2) {
        topBgIndex_ = 2;
        RefreshCompletionRate(false);
        if (filterCursor_ == 0x25) {
            activeElement_ = 0xc;
            if (subFilterCursor_ < 0) {
                subFilterCursor_ = 0x48;
            }
        } else if (filterCursor_ == 0x26) {
            activeElement_ = 0xb;
            if (subFilterCursor_ < 0) {
                subFilterCursor_ = 0x40;
            }
        }
        ui_->cursor = subFilterCursor_;
        func_020813ec(ui_, activeElement_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 3) {
        selection_ = &subFilterCursor_;
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            state_        = State_BookList;
            subStep_      = 0;
            selection_    = 0;
            recipeCursor_ = -1;
            DecodeFilterButton(subFilterCursor_, &filterType_, &filterSubtype_);
            short tmp = 0;
            func_02071ffc(&recipeDb_, filterType_, filterSubtype_, sortOrder_, 0x10, &tmp);
            pageStart_ = recipeList_ = recipeDb_.list;
            CountRecipePages();
            func_0207fdcc(ui_, activeElement_);
            state_   = State_BookList;
            subStep_ = 0;
            SetupBookListButtons(&layout_);
            flags_ |= 0x80;
        } else {
            if (IsBackPressed()) {
                subFilterCursor_ = -1;
                func_0207fdcc(ui_, activeElement_);
                subStep_ = 0;
            }
            HandleCloseKey();
        }
    }
}

// Recipe book only: browsing the recipe list
void AlchemyScreen::UpdateBookList() {
    AlchemyPot *pot = pot_;
    if (subStep_ == 0) {
        topBgIndex_ = 1;
        RefreshCompletionRate(false);
        RefreshFilterIcon();
        RefreshSortButton();
        activeElement_ = 9;
        if (recipeCursor_ < 0) {
            recipeCursor_ = 0x29;
        }
        ui_->cursor = recipeCursor_;
        RefreshRecipePage();
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 1) {
        selection_   = &recipeCursor_;
        bool changed = false;
        if (scrollKeys_ & 0x10) {
            changed = true;
            page_++;
        } else if (scrollKeys_ & 0x20) {
            changed = true;
            page_--;
        }
        if (page_ == 0xff) {
            page_ = numPages_ - 1;
        }
        if (numPages_ <= page_) {
            page_ = 0;
        }
        Recipe *node        = GetPageRecipe(recipeCursor_ - 0x29);
        RecipeEntry *recipe = 0;
        if (node) {
            recipe = book_.FindEntry(node->id);
            book_.GetIngredientCounts(node->id, (signed char *) ownedCounts_);
        } else {
            pot->HidePanel();
        }
        pot->PreviewRecipe(node, recipe, ownedCounts_);
        if (changed) {
            RefreshRecipePage();
            return;
        }
        int action = PollBookAction();
        if (action == 1) {
            ToggleSortOrder();
            return;
        }
        if (action == 4) {
            if (node == 0 || recipe == 0) {
                return;
            }
            pot->SetPanelVisible(!(pot->panel_.flags & 0x800));
            func_0205eaa0(data_02108760, 1, 0);
            return;
        }
        if (!IsConfirmPressed() && IsBackPressed()) {
            flags_ &= ~0x80;
            func_0207fdcc(ui_, activeElement_);
            func_0207fdcc(ui_, 0xe);
            func_0207fdcc(ui_, 0xf);
            state_      = State_BookFilter;
            subStep_    = 0;
            topBgIndex_ = 2;
            pot->HidePanel();
            if (filterCursor_ == 0x25 || filterCursor_ == 0x26) {
                subStep_ = 2;
            }
        }
        HandleCloseKey();
    }
}

void AlchemyScreen::UpdateTopMenu() {
    UiObject *ui = ui_;
    if (subStep_ == 0) {
        pot_->SetPanelVisible(true);
        topBgIndex_    = 0;
        activeElement_ = 2;
        if (menuCursor_ < 0) {
            menuCursor_ = func_02080468(ui, activeElement_);
        }
        ui->cursor = menuCursor_;
        func_020813ec(ui, activeElement_);
        RefreshMenuLabels();
        func_02081ea4(ui, activeElement_, 0);
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
        if (menuCursor_ == 4) {
            func_02080c04(ui, 3);
        } else {
            func_02080c20(ui, 3);
        }
        return;
    }
    if (subStep_ != 1) {
        return;
    }
    selection_ = &menuCursor_;
    if (prevCursor_ != *selection_) {
        if (menuCursor_ == 4) {
            func_02080c04(ui, 3);
        } else {
            func_02080c20(ui, 3);
        }
        RefreshMenuLabels();
    }
    blinkTimer_ = 0;
    if (IsConfirmPressed()) {
        func_0205eaa0(data_02108760, 1, 0);
        func_0208203c(&menuInput_);
        selection_ = 0;
        switch (menuCursor_) {
            case 2: {
                filterCursor_ = -1;
                func_0207fe44(ui);
                state_   = State_RecipeFilter;
                subStep_ = 0;
                ScreenLayout::Element *e;
                e = FindLayoutElement(&layout_, 0x50);
                if (e) {
                    e->flags &= ~1;
                }
                e = FindLayoutElement(&layout_, 0x4f);
                if (e) {
                    e->flags |= 1;
                }
                e = FindLayoutElement(&layout_, 0xe);
                if (e) {
                    e->flags &= ~1;
                }
                e = FindLayoutElement(&layout_, 0xf);
                if (e) {
                    e->flags |= 1;
                }
                e = FindLayoutElement(&layout_, 0x15);
                if (e) {
                    e->flags &= ~1;
                }
                break;
            }
            case 3: {
                BuildIngredientLists();
                u16 *totals = itemTotals_;
                u8 i;
                bool any = false;
                if (totals) {
                    for (i = 0; i < 9; i++) {
                        if (totals[i]) {
                            any = true;
                            break;
                        }
                    }
                    if (totals[8]) {
                        any = true;
                    }
                }
                if (any) {
                    categoryCursor_ = -1;
                    itemCursor_     = -1;
                    func_0207fe44(ui);
                    state_   = State_FirstIngredient;
                    subStep_ = 0;
                    pot_->SetPanelVisible(true);
                    messageId_   = 0x24;
                    messageStep_ = 0;
                    flags_ |= 0x120;
                } else {
                    flags_ |= 0x20;
                    messageId_   = 0x35;
                    messageStep_ = 0;
                    func_02081ea4(ui, activeElement_, 1);
                    subStep_ = 0;
                    func_0208203c(&menuInput_);
                    selection_ = 0;
                    return;
                }
                break;
            }
            case 4:
                state_   = State_FadeOut;
                subStep_ = 0;
                break;
        }
        func_0207fdcc(ui, 3);
        func_0207fdcc(ui, activeElement_);
        return;
    }
    if (IsBackPressed() || g_keyManager.TestKeysPressed(KEY_Y)) {
        func_0207fe44(ui);
        state_   = State_FadeOut;
        subStep_ = 0;
    }
}

void AlchemyScreen::UpdateFirstIngredient() {
    UiObject *ui = ui_;
    if (subStep_ == 0) {
        func_02080c20(ui, 0x10);
        func_02080c20(ui, 0x11);
        func_02080c68(ui, 0x11, 1);
        pot_->flags_ &= ~AlchemyPot::Flag_Brewing;
        func_0207fdcc(ui, 0);
        func_0207fdcc(ui, 1);
        activeElement_ = 0x10;
        if (categoryCursor_ < 0) {
            categoryCursor_ = func_02080468(ui, activeElement_);
        }
        func_02081ea4(ui, activeElement_, 0);
        func_020813ec(ui, activeElement_);
        func_02080c68(ui, activeElement_, 0);
        selectedItemId_ = -1;
        quantity_       = 0;
        for (u8 i = 0; i < 3; i++) {
            pendingIds_[i]    = -1;
            pendingCounts_[i] = 0;
        }
        CountIngredientPages();
        func_02081ea4(ui, 0x11, 0);
        func_020804fc(ui, 0x11);
        RefreshIngredientList();
        flags_ |= 0x10;
        SetupIngredientListButtons(&layout_);
        pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        pot_->SetItem(-1, 0);
        subStep_++;
    } else if (subStep_ == 1) {
        if (HandleListButtons()) {
            return;
        }
        selection_ = &categoryCursor_;
        if (prevCursor_ != *selection_) {
            CountIngredientPages();
            RefreshIngredientList();
        }
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            if (!HasCategoryItems()) {
                return;
            }
            func_02080c68(ui, activeElement_, 1);
            func_02081ea4(ui, activeElement_, 1);
            func_02080c68(ui, 0x11, 0);
            subStep_++;
        } else if (IsBackPressed()) {
            showPanel_ = 1;
            func_0207fe44(ui);
            func_0208203c(&menuInput_);
            selection_  = 0;
            itemCursor_ = -1;
            state_      = State_TopMenu;
            subStep_    = 0;
            flags_ &= ~0x10;
            flags_ |= 0x20;
            messageId_   = 0;
            messageStep_ = 0;
        }
        HandleCloseKey();
    } else if (subStep_ == 2) {
        for (u8 i = 0; i < 3; i++) {
            pendingIds_[i]    = -1;
            pendingCounts_[i] = 0;
        }
        OpenIngredientList();
        pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
        subStep_++;
    } else if (subStep_ == 3) {
        if (UpdateIngredientCursor()) {
            return;
        }
        if (IsConfirmPressed() && prevCursor_ == *selection_) {
            short item;
            u8 bag;
            u8 ok;
            ok   = 0;
            bag  = 0;
            item = 0;
            GetSelectedIngredient(&bag, &item, &ok);
            if (ok) {
                func_0205eaa0(data_02108760, 1, 0);
                func_0208203c(&menuInput_);
                selection_ = 0;
                func_02081ea4(ui_, 0x11, 1);
                selectedCategory_ = bag;
                selectedItemId_   = item;
                quantity_         = 1;
                subStep_++;
                flags_ &= ~0x10;
                flags_ |= 0x20;
                messageId_   = 9;
                messageStep_ = 0;
                return;
            }
        } else if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_  = 0;
            itemCursor_ = -1;
            subStep_    = 0;
        }
        HandleCloseKey();
    } else if (subStep_ == 4) {
        OpenQuantitySelect();
        repeatTimer_ = 0;
        subStep_++;
    } else if (subStep_ == 5) {
        blinkTimer_ = 0;
        int r       = HandleQuantityInput();
        if (r == 1) {
            return;
        }
        if (KeyManager::GetInstance()->TestKeysPressed(KEY_A) || r == 2) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            flags_ &= ~8;
            func_0207fdcc(ui, 6);
            ReserveIngredient(selectedCategory_, selectedItemId_, quantity_);
            pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
            state_       = State_SecondIngredient;
            subStep_     = 0;
            brewCount_   = quantity_;
            messageId_   = 0x39;
            messageStep_ = 0;
            flags_ |= 0x20;
            flags_ |= 0x100;
        } else if (IsBackPressed() || r == 3) {
            func_0208203c(&menuInput_);
            selection_ = 0;
            flags_ &= ~8;
            func_0207fdcc(ui, 6);
            subStep_     = 0;
            messageId_   = 0x24;
            messageStep_ = 0;
            flags_ |= 0x20;
            flags_ |= 0x100;
        }
    }
}

void AlchemyScreen::UpdateSecondIngredient() {
    UiObject *ui = ui_;
    if (subStep_ == 0) {
        func_0207fdcc(ui, 0);
        func_0207fdcc(ui, 1);
        activeElement_ = 0x10;
        if (categoryCursor_ < 0) {
            categoryCursor_ = func_02080468(ui, activeElement_);
        }
        func_02081ea4(ui, activeElement_, 0);
        func_020813ec(ui, activeElement_);
        func_02080c68(ui, activeElement_, 0);
        selectedItemId_ = -1;
        quantity_       = 0;
        CountIngredientPages();
        func_02081ea4(ui, 0x11, 0);
        func_020804fc(ui, 0x11);
        RefreshIngredientList();
        flags_ |= 0x10;
        func_02080c68(ui, 0x11, 1);
        SetupIngredientListButtons(&layout_);
        pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        pot_->SetItem(-1, 0);
        subStep_++;
    } else if (subStep_ == 1) {
        if (HandleListButtons()) {
            return;
        }
        selection_ = &categoryCursor_;
        if (prevCursor_ != *selection_) {
            CountIngredientPages();
            RefreshIngredientList();
        }
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            if (!HasCategoryItems()) {
                return;
            }
            func_02080c68(ui, activeElement_, 1);
            func_02081ea4(ui, activeElement_, 1);
            func_02080c68(ui, 0x11, 0);
            subStep_++;
        } else if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_ = 0;
            state_     = State_FirstIngredient;
            subStep_   = 0;
            ReturnPendingIngredient();
            pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
            RefreshIngredientList();
        }
        HandleCloseKey();
    } else if (subStep_ == 2) {
        OpenIngredientList();
        pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
        subStep_++;
    } else if (subStep_ == 3) {
        if (UpdateIngredientCursor()) {
            return;
        }
        if (IsConfirmPressed() && prevCursor_ == *selection_) {
            short item;
            u8 bag;
            u8 ok;
            ok   = 0;
            bag  = 0;
            item = 0;
            GetSelectedIngredient(&bag, &item, &ok);
            if (ok) {
                func_0205eaa0(data_02108760, 1, 0);
                func_0208203c(&menuInput_);
                selection_ = 0;
                func_02081ea4(ui_, 0x11, 1);
                selectedCategory_ = bag;
                selectedItemId_   = item;
                quantity_         = 1;
                subStep_++;
                flags_ &= ~0x10;
                flags_ |= 0x20;
                messageId_   = 9;
                messageStep_ = 0;
                return;
            }
        } else if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_  = 0;
            itemCursor_ = -1;
            subStep_    = 0;
        }
        HandleCloseKey();
    } else if (subStep_ == 4) {
        OpenQuantitySelect();
        repeatTimer_ = 0;
        subStep_++;
    } else if (subStep_ == 5) {
        blinkTimer_ = 0;
        int r       = HandleQuantityInput();
        if (r == 1) {
            return;
        }
        if (g_keyManager.TestKeysPressed(KEY_A) || r == 2) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            flags_ &= ~8;
            func_0207fdcc(ui, 6);
            ReserveIngredient(selectedCategory_, selectedItemId_, quantity_);
            pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
            state_       = State_ThirdIngredient;
            subStep_     = 0;
            brewCount_   = quantity_;
            messageId_   = 0x3a;
            messageStep_ = 0;
            flags_ |= 0x20;
            flags_ |= 0x100;
        } else if (IsBackPressed() || r == 3) {
            func_0208203c(&menuInput_);
            selection_ = 0;
            flags_ &= ~8;
            func_0207fdcc(ui, 6);
            subStep_     = 0;
            messageId_   = 0x24;
            messageStep_ = 0;
            flags_ |= 0x20;
            flags_ |= 0x100;
        }
    }
}

void AlchemyScreen::UpdateThirdIngredient() {
    UiObject *ui = ui_;
    if (subStep_ == 0) {
        func_0207fdcc(ui, 0);
        func_0207fdcc(ui, 1);
        activeElement_ = 0x10;
        if (categoryCursor_ < 0) {
            categoryCursor_ = func_02080468(ui, activeElement_);
        }
        func_02081ea4(ui, activeElement_, 0);
        func_020813ec(ui, activeElement_);
        func_02080c68(ui, activeElement_, 0);
        selectedItemId_ = -1;
        quantity_       = 0;
        CountIngredientPages();
        func_02081ea4(ui, 0x11, 0);
        func_020804fc(ui, 0x11);
        RefreshIngredientList();
        flags_ |= 0x10;
        flags_ |= 0x20;
        func_02080c68(ui, 0x11, 1);
        SetupIngredientListButtons(&layout_);
        pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        pot_->SetItem(-1, 0);
        subStep_++;
    } else if (subStep_ == 1) {
        if (HandleListButtons()) {
            return;
        }
        selection_ = &categoryCursor_;
        if (prevCursor_ != *selection_) {
            CountIngredientPages();
            RefreshIngredientList();
        }
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            if (HasCategoryItems()) {
                func_02080c68(ui, activeElement_, 1);
                func_02081ea4(ui, activeElement_, 1);
                func_02080c68(ui, 0x11, 0);
                subStep_++;
            }
            return;
        }
        if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_ = 0;
            state_     = State_SecondIngredient;
            subStep_   = 0;
            ReturnPendingIngredient();
            pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
            RefreshIngredientList();
        }
        HandleCloseKey();
    } else if (subStep_ == 2) {
        OpenIngredientList();
        pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
        subStep_++;
    } else if (subStep_ == 3) {
        if (UpdateIngredientCursor()) {
            return;
        }
        if (IsConfirmPressed() && prevCursor_ == *selection_) {
            short item;
            u8 bag;
            u8 ok;
            ok   = 0;
            bag  = 0;
            item = 0;
            GetSelectedIngredient(&bag, &item, &ok);
            if (ok) {
                func_0205eaa0(data_02108760, 1, 0);
                func_0208203c(&menuInput_);
                selection_ = 0;
                func_02081ea4(ui_, 0x11, 1);
                selectedCategory_ = bag;
                selectedItemId_   = item;
                quantity_         = 1;
                subStep_++;
                flags_ &= ~0x10;
                flags_ |= 0x20;
                messageId_   = 9;
                messageStep_ = 0;
                return;
            }
        } else if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_  = 0;
            itemCursor_ = -1;
            subStep_    = 0;
        }
        HandleCloseKey();
    } else if (subStep_ == 4) {
        OpenQuantitySelect();
        repeatTimer_ = 0;
        subStep_++;
    } else if (subStep_ == 5) {
        blinkTimer_ = 0;
        int r       = HandleQuantityInput();
        if (r == 1) {
            return;
        }
        if (KeyManager::GetInstance()->TestKeysPressed(KEY_A) || r == 2) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            flags_ &= ~8;
            func_0207fdcc(ui, 6);
            ReserveIngredient(selectedCategory_, selectedItemId_, quantity_);
            pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
            subStep_++;
            flags_ |= 0x20;
            messageId_   = 10;
            messageStep_ = 0;
        } else if (IsBackPressed() || r == 3) {
            func_0208203c(&menuInput_);
            selection_ = 0;
            flags_ &= ~8;
            func_0207fdcc(ui, 6);
            subStep_     = 0;
            messageId_   = 0x24;
            messageStep_ = 0;
            flags_ |= 0x120;
        }
    } else if (subStep_ == 6) {
        pot_->SetPanelVisible(true);
        OpenConfirmDialog();
        subStep_++;
    } else if (subStep_ == 7) {
        int r = HandleConfirmDialog();
        if (r != -1) {
            if (r == 1) {
                state_            = State_FreeBrewing;
                showResultSprite_ = 0;
                subStep_          = 0;
            }
        } else {
            while (pendingIds_[0] > 0) {
                ReturnPendingIngredient();
            }
            pot_->SetIngredientIcons(pendingIds_, pendingCounts_);
            state_   = State_FirstIngredient;
            subStep_ = 0;
            flags_ |= 0x20;
            messageId_   = 0x24;
            messageStep_ = 0;
            flags_ |= 0x100;
        }
    }
}

// Brewing with three freely chosen ingredients
void AlchemyScreen::UpdateFreeBrewing() {
    AlchemyPot *pot = pot_;
    BackgroundLoader::GetInstance();

    if (subStep_ == 0) {
        showResultSprite_ = 0;
        brewCount_        = 0;
        recipe_           = FindMatchingRecipe(pendingIds_, pendingCounts_, &brewCount_);
        if (recipe_ != 0) {
            successRateV_ = book_.GetSuccessRate(recipe_->id);
            subStep_++;
        } else {
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            subStep_ = 0x78;
            flags_ |= 0x20;
            messageId_   = 0xb;
            messageStep_ = 0;
        }
    } else if (subStep_ == 1) {
        StartItemSpriteLoad(false);
        subStep_++;
    } else if (subStep_ == 2) {
        if (resultSpriteTask_ == -1) {
            if (FinishItemSpriteLoad(false)) {
                StartResultSpriteLoad(items_);
            }
        } else if (FinishResultSpriteLoad()) {
            u32 r    = func_020dd4c4((signed char) func_020100a8(GameState::GetInstance()), items_);
            subStep_ = 5;
            if (r & 0x10) {
                flags_ |= 0x20;
                messageId_   = unk_396 + 4;
                messageStep_ = 0;
                subStep_++;
            }
        }
    } else if (subStep_ == 3) {
        OpenConfirmDialog();
        subStep_++;
    } else if (subStep_ == 4) {
        int r = HandleConfirmDialog();
        if (r != -1) {
            if (r == 1) {
                subStep_++;
            }
        } else {
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            subStep_ = 0x78;
        }
    } else if (subStep_ == 5) {
        if (!pot->IsLoadingSlots()) {
            flags_ &= ~0x40;
            bool b     = false;
            Recipe *rc = recipe_;
            if (rc != 0) {
                if (rc->bonusRecipe > 0) {
                    b = true;
                }
            }
            if (b) {
                successRateV_ = book_.GetSuccessRate((*(Recipe *volatile *) &recipe_)->bonusRecipe);
                messageId_    = 0xd;
                messageStep_  = 0;
                flags_ |= 0x20;
                subStep_++;
                if ((u32) NextRandomMax(GetBTRandom(), 10000) / 100 < successRateV_) {
                    flags_ |= 0x40;
                }
                StartItemSpriteLoad(IsGreatSuccess());
            } else {
                func_02080c04(ui_, 0x10);
                func_02080c04(ui_, 0x11);
                func_02080c04(ui_, 1);
                func_02080c04(ui_, 0);
                pot_->flags_ |= AlchemyPot::Flag_Brewing;
                showPanel_   = 1;
                waitFrames_  = 0;
                nextSubStep_ = 0x64;
                subStep_     = 0x96;
            }
        }
    } else if (subStep_ == 6) {
        if (IsAnyKeyPressed() && resultSpriteTask_ == -1) {
            if (FinishItemSpriteLoad(IsGreatSuccess())) {
                StartResultSpriteLoad(items_);
            }
        } else if (resultSpriteTask_ != -1 && FinishResultSpriteLoad()) {
            flags_ |= 0x20;
            messageId_   = unk_396 + 0xe;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 7) {
        if (IsAnyKeyPressed()) {
            short m = 0x10;
            short r;
            if ((r = successRateV_) >= 3 && r < 10) {
                m = 0x11;
            } else if ((r = successRateV_) >= 10 && r <= 99) {
                m = 0x12;
            } else if (successRateV_ == 100) {
                m = unk_396 + 0x16;
            }
            flags_ |= 0x20;
            messageId_   = m;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 8) {
        if (!pot->IsLoadingSlots() && IsAnyKeyPressed()) {
            short m = 0x13;
            subStep_++;
            if (successRateV_ == 100) {
                subStep_ = 0xb;
                m        = 0x18;
            }
            flags_ |= 0x20;
            messageId_   = m;
            messageStep_ = 0;
        }
    } else if (subStep_ == 9) {
        if (IsAnyKeyPressed()) {
            flags_ |= 0x20;
            messageId_   = 0x14;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 10) {
        if (IsAnyKeyPressed()) {
            flags_ |= 0x20;
            messageId_   = 0x15;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 11) {
        OpenConfirmDialog();
        subStep_++;
    } else if (subStep_ == 12) {
        int r = HandleConfirmDialog();
        if (r != -1) {
            if (r == 1) {
                bool success = IsGreatSuccess();
                if (success) {
                    book_.RegisterCraft(recipe_->bonusRecipe, recipe_->id, &recipeDb_);
                } else {
                    book_.RegisterCraft(recipe_->id, -1, &recipeDb_);
                }
                CraftItem(items_, recipe_, brewCount_);
                BuildIngredientLists();
                itemCursor_ = -1;
                subStep_++;
            }
        } else {
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            flags_ |= 0x20;
            flags_ &= ~0x40;
            func_02080c20(ui_, 0);
            messageId_   = 0x19;
            messageStep_ = 0;
            flags_ |= 0x20;
            subStep_ = 0x5a;
        }
    } else if (subStep_ == 0x5a) {
        if (IsAnyKeyPressed()) {
            func_02080c20(ui_, 0);
            messageId_   = 0x24;
            messageStep_ = 0;
            flags_ |= 0x120;
            state_   = State_FirstIngredient;
            subStep_ = 0;
        }
    } else if (subStep_ == 0xd) {
        messageId_   = 0x36;
        messageStep_ = 0;
        func_02080c04(ui_, 0);
        subStep_++;
    } else if (subStep_ == 0xe) {
        flags_ |= 0x200;
        if (BackgroundLoader::GetInstance()->GetNumQueuedTasks() <= 0) {
            func_020466e4(func_020d6c00(), 0x80);
            func_020a9ea4(unk_3c);
            subStep_++;
        }
    } else if (subStep_ == 0xf) {
        BackgroundLoader::AddLockGlobal();
        BackgroundLoader::FreeAllocationsGlobal();
        int n = 8;
        if (func_0202c540(func_0202ae18())) {
            n = 0x14;
        }
        if (func_020aad1c(unk_3c, data_0211e33c, n, 0) == 1) {
            func_020466f4(func_020d6c00(), 0x80);
            subStep_++;
            flags_ &= ~0x200;
        }
        BackgroundLoader::RemoveLockGlobal();
    } else if (subStep_ == 0x10) {
        messageId_   = 0x37;
        messageStep_ = 0;
        subStep_++;
    } else if (subStep_ == 0x11) {
        if (IsAnyKeyPressed()) {
            messageId_   = 0x1a;
            messageStep_ = 0;
            flags_ |= 0x20;
            func_02080c20(ui_, 0);
            subStep_++;
        }
    } else if (subStep_ == 0x12) {
        blinkTimer_ = 0;
        if (IsAnyKeyPressed()) {
            func_02080c04(ui_, 0x10);
            func_02080c04(ui_, 0x11);
            func_02080c04(ui_, 1);
            func_02080c04(ui_, 0);
            pot_->flags_ |= AlchemyPot::Flag_Brewing;
            showPanel_   = 1;
            waitFrames_  = 0;
            nextSubStep_ = 0x13;
            subStep_     = 0x96;
        }
    } else if (subStep_ == 0x13) {
        pot->flags_ |= 0x100;
        subStep_++;
    } else if (subStep_ == 0x14) {
        pot->flags_ |= 0x200;
        subStep_++;
    } else if (subStep_ == 0x15) {
        showResultSprite_ = 1;
        void **m          = func_020e5294(&itemResources_, items_[0].id);
        if (m != 0) {
            memset(recipeNames_[0], 0, 0x80);
            func_020e4864(*m, recipeNames_[0], 1, 0, 0, 0);
            func_02080f8c(ui_, 0x7f, recipeNames_[0]);
        }
        func_020805f4(ui_, 0x7f);
        func_020813ec(ui_, 0x12);
        pot->flags_ |= 0x400;
        unk_42f      = 1;
        int sound    = 0x3e;
        bool success = IsGreatSuccess();
        if (success) {
            sound = 0x3f;
            pot_->StartEffectAnimation();
        }
        func_0209c830(data_02109bf4, sound);
        messageId_   = 0xc;
        messageStep_ = 0;
        subStep_++;
    } else if (subStep_ == 0x16) {
        if (IsAnyKeyPressed()) {
            messageId_   = 0x38;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 0x17) {
        if (IsAnyKeyPressed()) {
            func_0207fdcc(ui_, 0x12);
            showResultSprite_ = 0;
            pot->flags_ &= ~0x800;
            short m      = 0x1f;
            bool success = IsGreatSuccess();
            if (success) {
                pot_->StopEffectAnimation();
                m = unk_396 + 0x1b;
            }
            flags_ |= 0x20;
            messageId_   = m;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 0x18) {
        if (IsAnyKeyPressed()) {
            short m      = 0x20;
            bool success = IsGreatSuccess();
            if (success) {
                m = unk_396 + 0x1d;
            }
            flags_ |= 0x20;
            messageId_   = m;
            messageStep_ = 0;
            subStep_     = 0x78;
            flags_ &= ~0x40;
        }
    } else if (subStep_ == 0x64) {
        pot->flags_ |= 0x100;
        subStep_++;
    } else if (subStep_ == 0x65) {
        pot->flags_ |= 0x200;
        subStep_++;
    } else if (subStep_ == 0x66) {
        if (recipe_ != 0) {
            showResultSprite_ = 1;
            void **m          = func_020e5294(&itemResources_, items_[0].id);
            if (m != 0) {
                memset(recipeNames_[0], 0, 0x80);
                func_020e4864(*m, recipeNames_[0], 1, 0, 0, 0);
                func_02080f8c(ui_, 0x7f, recipeNames_[0]);
            }
            func_020805f4(ui_, 0x7f);
            func_020813ec(ui_, 0x12);
            book_.RegisterCraft(recipe_->id, -1, &recipeDb_);
            CraftItem(items_, recipe_, brewCount_);
            BuildIngredientLists();
            itemCursor_ = -1;
            unk_42f     = 1;
            func_0209c830(data_02109bf4, 0x3e);
            pot->flags_ |= 0x400;
            func_02080c04(ui_, 0);
            messageId_   = 0xc;
            messageStep_ = 0;
            subStep_++;
        } else {
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            ReturnPendingIngredient();
            flags_ |= 0x20;
            messageId_   = 0xb;
            messageStep_ = 0;
            subStep_     = 0x78;
        }
    } else if (subStep_ == 0x67) {
        if (IsAnyKeyPressed()) {
            messageId_   = 0x38;
            messageStep_ = 0;
            subStep_     = 0x78;
        }
    } else if (subStep_ == 0x78) {
        if (unk_42f) {
            blinkTimer_ = 0;
        } else if (IsAnyKeyPressed()) {
            func_0207fdcc(ui_, 0x12);
            pot->flags_ &= ~0x800;
            func_02080c20(ui_, 0);
            flags_ |= 0x20;
            messageId_   = 0x24;
            messageStep_ = 0;
            flags_ |= 0x100;
            state_   = State_FirstIngredient;
            subStep_ = 0;
        }
    } else if (subStep_ == 0x96) {
        pot->flags_ |= 0x2000;
        if (++waitFrames_ >= 10) {
            waitFrames_ = 0;
            pot->flags_ &= ~0x2000;
            pot->flags_ |= 0x80;
            subStep_ = nextSubStep_;
        }
    }
}

// Alchemy from the recipe book: choice of the recipe filter
void AlchemyScreen::UpdateRecipeFilter() {
    if (subStep_ == 0) {
        topBgIndex_ = 2;
        RefreshCompletionRate(true);
        activeElement_ = 8;
        if (filterCursor_ < 0) {
            filterCursor_ = 0x24;
        }
        ui_->cursor = filterCursor_;
        func_020813ec(ui_, activeElement_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 1) {
        selection_ = &filterCursor_;
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            bool done        = false;
            selection_       = 0;
            subFilterCursor_ = -1;
            recipeCursor_    = -1;
            filterType_      = -1;
            filterSubtype_   = -1;
            switch (filterCursor_) {
                case 0x24: done = true; break;
                case 0x25:
                    filterType_ = 0;
                    subStep_++;
                    break;
                case 0x26: subStep_++; break;
                case 0x27:
                    filterType_ = 7;
                    done        = true;
                    break;
                case 0x28:
                    filterType_ = 8;
                    done        = true;
                    break;
            }
            if (done) {
                state_   = State_RecipeList;
                subStep_ = 0;
                flags_ |= 0x80;
            }
            short tmp = 0;
            func_02071ffc(&recipeDb_, filterType_, filterSubtype_, sortOrder_, 0x10, &tmp);
            pageStart_ = recipeList_ = recipeDb_.list;
            CountRecipePages();
            func_0207fdcc(ui_, activeElement_);
            return;
        }
        if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_ = 0;
            func_0207fe44(ui_);
            state_      = State_TopMenu;
            subStep_    = 0;
            topBgIndex_ = 0;
            flags_ |= 0x20;
            messageId_   = 0;
            messageStep_ = 0;
            pot_->HidePanel();
        }
        HandleCloseKey();
    } else if (subStep_ == 2) {
        topBgIndex_ = 2;
        RefreshCompletionRate(false);
        if (filterCursor_ == 0x25) {
            activeElement_ = 0xc;
            if (subFilterCursor_ < 0) {
                subFilterCursor_ = 0x48;
            }
        } else if (filterCursor_ == 0x26) {
            activeElement_ = 0xb;
            if (subFilterCursor_ < 0) {
                subFilterCursor_ = 0x40;
            }
        }
        ui_->cursor = subFilterCursor_;
        func_020813ec(ui_, activeElement_);
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 3) {
        selection_ = &subFilterCursor_;
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_    = 0;
            recipeCursor_ = -1;
            DecodeFilterButton(subFilterCursor_, &filterType_, &filterSubtype_);
            short tmp = 0;
            func_02071ffc(&recipeDb_, filterType_, filterSubtype_, sortOrder_, 0x10, &tmp);
            pageStart_ = recipeList_ = recipeDb_.list;
            CountRecipePages();
            func_0207fdcc(ui_, activeElement_);
            state_   = State_RecipeList;
            subStep_ = 0;
            flags_ |= 0x80;
            return;
        }
        if (IsBackPressed()) {
            func_0208203c(&menuInput_);
            selection_       = 0;
            subFilterCursor_ = -1;
            func_0207fdcc(ui_, activeElement_);
            subStep_ = 0;
        }
        HandleCloseKey();
    }
}

// Alchemy from the recipe book: choice of the recipe and of the brew count
void AlchemyScreen::UpdateRecipeList() {
    if (subStep_ == 0) {
        topBgIndex_ = 1;
        func_0207fdcc(ui_, 0);
        func_0207fdcc(ui_, 1);
        RefreshCompletionRate(false);
        RefreshFilterIcon();
        RefreshSortButton();
        activeElement_ = 9;
        if (recipeCursor_ < 0) {
            recipeCursor_ = 0x29;
        }
        ui_->cursor = recipeCursor_;
        RefreshRecipePage();
        func_0208203c(&menuInput_);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 1) {
        selection_   = &recipeCursor_;
        bool changed = false;
        if (scrollKeys_ & 0x10) {
            changed = true;
            page_++;
        } else if (scrollKeys_ & 0x20) {
            changed = true;
            page_--;
        }
        if (page_ == 0xff) {
            page_ = numPages_ - 1;
        }
        if (numPages_ <= page_) {
            page_ = 0;
        }
        Recipe *node        = GetPageRecipe(recipeCursor_ - 0x29);
        RecipeEntry *recipe = 0;
        if (node) {
            recipe = book_.FindEntry(node->id);
            book_.GetIngredientCounts(node->id, (signed char *) ownedCounts_);
        }
        pot_->PreviewRecipe(node, recipe, ownedCounts_);
        if (changed) {
            RefreshRecipePage();
            return;
        }
        int action = PollBookAction();
        if (action == 1) {
            ToggleSortOrder();
            return;
        }
        if (action == 4) {
            if (node == 0 || recipe == 0) {
                return;
            }
            pot_->SetPanelVisible(!((pot_->panel_.flags & 0x800) ? 1 : 0));
            func_0205eaa0(data_02108760, 1, 0);
            return;
        }
        if (IsConfirmPressed() || action == 2) {
            if (node != 0 && recipe != 0) {
                if (!recipe->known) {
                    return;
                }
                recipe_     = node;
                ui_->cursor = *selection_;
                func_0205eaa0(data_02108760, 1, 0);
                func_0208203c(&menuInput_);
                selection_ = 0;
                if (book_.SelectRecipe(node->id)) {
                    func_02081ea4(ui_, 9, 1);
                    messageId_   = 3;
                    messageStep_ = 0;
                    flags_ |= 0x20;
                    quantity_  = 1;
                    brewCount_ = 1;
                    subStep_++;
                } else {
                    func_02081ea4(ui_, 9, 1);
                    messageId_   = 2;
                    messageStep_ = 0;
                    flags_ |= 0x20;
                    subStep_ = 0x64;
                }
                return;
            }
        } else if (IsBackPressed() || action == 3) {
            if (action == 3) {
                func_0205eaa0(data_02108760, 1, 0);
            }
            func_0207fdcc(ui_, activeElement_);
            func_0207fdcc(ui_, 0xe);
            func_0207fdcc(ui_, 0xf);
            state_   = State_RecipeFilter;
            subStep_ = 0;
            func_0208203c(&menuInput_);
            selection_ = 0;
            pot_->HidePanel();
            topBgIndex_ = 2;
            if (filterCursor_ == 0x25 || filterCursor_ == 0x26) {
                subStep_ = 2;
            }
            flags_ &= ~0x80;
        }
        HandleCloseKey();
    } else if (subStep_ == 2) {
        OpenConfirmDialog();
        subStep_++;
    } else if (subStep_ == 3) {
        int r = HandleConfirmDialog();
        if (r != -1) {
            if (r == 1) {
                StartItemSpriteLoad(false);
                subStep_++;
            }
        } else {
            subStep_ = 0;
        }
    } else if (subStep_ == 4) {
        blinkTimer_ = 0;
        if (resultSpriteTask_ == -1) {
            if (FinishItemSpriteLoad(false)) {
                StartResultSpriteLoad(items_);
            }
        } else if (FinishResultSpriteLoad()) {
            showResultSprite_ = 0;
            if (!(func_020dd4c4((signed char) func_020100a8(GameState::GetInstance()), items_) & 4)) {
                subStep_ = 7;
                return;
            }
            messageId_   = unk_396 + 4;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (subStep_ == 5) {
        OpenConfirmDialog();
        subStep_++;
    } else if (subStep_ == 6) {
        int r = HandleConfirmDialog();
        if (r != -1) {
            if (r == 1) {
                subStep_++;
            }
        } else {
            subStep_ = 0;
        }
    } else if (subStep_ == 7) {
        if (book_.HasIngredientsFor(2)) {
            messageId_   = 6;
            messageStep_ = 0;
            subStep_++;
        } else {
            messageId_   = 8;
            messageStep_ = 0;
            state_       = State_RecipeBrewing;
            pot_->HidePanel();
            subStep_ = 0;
        }
    } else if (subStep_ == 8) {
        func_02080c68(ui_, 9, 1);
        RefreshBrewCount();
        flags_ |= 8;
        repeatTimer_ = 0;
        subStep_++;
    } else if (subStep_ == 9) {
        blinkTimer_ = 0;
        int r       = HandleBrewCountInput();
        if (r == 1) {
            return;
        }
        if (g_keyManager.TestKeysPressed(KEY_A) || r == 2) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(&menuInput_);
            selection_ = 0;
            brewCount_ = quantity_;
            func_0207fdcc(ui_, 10);
            state_ = State_RecipeBrewing;
            pot_->HidePanel();
            subStep_     = 0;
            messageId_   = 7;
            messageStep_ = 0;
            flags_ &= ~8;
            return;
        }
        if (IsBackPressed() || r == 3) {
            func_0207fdcc(ui_, 10);
            subStep_ = 0;
            flags_ &= ~8;
            quantity_ = 1;
            ApplyQuantityToPanel();
        }
    } else if (subStep_ == 0x64) {
        if (IsAnyKeyPressed()) {
            messageId_   = 1;
            messageStep_ = 0;
            flags_ |= 0x20;
            subStep_++;
        }
    } else if (subStep_ == 0x65) {
        if (IsAnyKeyPressed()) {
            subStep_ = 0;
        }
    }
}

void AlchemyScreen::SetBookButtonsEnabled(UiObject *ui, bool enable, u8 *active) {
    if (ui == 0) {
        return;
    }
    if (enable) {
        flags_ |= 0x80;
        *active = 1;
        func_02080c20(ui, 0xd);
        func_02080c20(ui, 0xe);
        func_02080c20(ui, 0xf);
        func_02080c20(ui, 0x9);
    } else {
        flags_ &= ~0x80;
        *active = 0;
        func_02080c04(ui, 0xd);
        func_02080c04(ui, 0xe);
        func_02080c04(ui, 0xf);
        func_02080c04(ui, 0x9);
    }
}

// Brewing of the recipe chosen in the recipe book
void AlchemyScreen::UpdateRecipeBrewing() {
    AlchemyPot *pot = pot_;
    BackgroundLoader::GetInstance();
    u8 sub = subStep_;
    if (sub == 0) {
        showResultSprite_ = 0;
        if (!IsAnyKeyPressed()) {
            return;
        }
        func_0207fdcc(ui_, 0);
        func_0207fdcc(ui_, 1);
        func_02080c68(ui_, 9, 0);
        flags_ &= ~0x40;
        short res = recipe_->bonusRecipe;
        if (res >= 0) {
            successRateV_ = book_.GetSuccessRate(res);
            messageId_    = 0xd;
            messageStep_  = 0;
            flags_ |= 0x20;
            subStep_++;
            if ((u32) NextRandomMax(GetBTRandom(), 10000) / 100 < successRateV_) {
                flags_ |= 0x40;
            }
            StartItemSpriteLoad(IsGreatSuccess());
        } else {
            pot->flags_ &= ~0x40;
            waitFrames_  = 0;
            nextSubStep_ = 100;
            subStep_     = 0x96;
            SetBookButtonsEnabled(ui_, false, &topBgIndex_);
        }
    } else if (sub == 1) {
        if (IsAnyKeyPressed() && resultSpriteTask_ == -1) {
            if (FinishItemSpriteLoad(IsGreatSuccess())) {
                StartResultSpriteLoad(items_);
            }
        } else if (resultSpriteTask_ != -1 && FinishResultSpriteLoad()) {
            messageId_   = unk_396 + 0xe;
            messageStep_ = 0;
            subStep_++;
        }
    } else if (sub == 2) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        short msg = 0x10;
        short r;
        if ((r = successRateV_) >= 3 && r < 10) {
            msg = 0x11;
        } else if ((r = successRateV_) >= 10 && r <= 99) {
            msg = 0x12;
        } else if (successRateV_ == 100) {
            msg = unk_396 + 0x16;
        }
        messageId_   = msg;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 3) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        short msg = 0x13;
        subStep_++;
        if (successRateV_ == 100) {
            subStep_ = 6;
            msg      = 0x18;
        }
        messageId_   = msg;
        messageStep_ = 0;
    } else if (sub == 4) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        messageId_   = 0x14;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 5) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        messageId_   = 0x15;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 6) {
        OpenConfirmDialog();
        subStep_++;
    } else if (sub == 7) {
        int r = HandleConfirmDialog();
        if (r != -1) {
            if (r == 1) {
                bool ok = IsGreatSuccess();
                if (ok) {
                    book_.RegisterCraft(recipe_->bonusRecipe, recipe_->id, &recipeDb_);
                } else {
                    book_.RegisterCraft(recipe_->id, -1, &recipeDb_);
                }
                CraftItem(items_, recipe_, brewCount_);
                u8 page     = page_;
                u8 numPages = numPages_;
                BuildIngredientLists();
                page_     = page;
                numPages_ = numPages;
                subStep_++;
            }
        } else {
            messageId_   = 0x19;
            messageStep_ = 0;
            flags_ |= 0x20;
            flags_ &= ~0x40;
            subStep_ = 0x5a;
        }
    } else if (sub == 0x5a) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        func_02080c20(ui_, 0);
        messageId_   = 1;
        messageStep_ = 0;
        flags_ |= 0x20;
        subStep_ = 0x79;
    } else if (sub == 8) {
        if (BackgroundLoader::GetInstance()->GetNumQueuedTasks() <= 0) {
            subStep_++;
        }
    } else if (sub == 9) {
        messageId_   = 0x36;
        messageStep_ = 0;
        func_02080c04(ui_, 0);
        subStep_++;
    } else if (sub == 10) {
        flags_ |= 0x200;
        if (BackgroundLoader::GetInstance()->GetNumQueuedTasks() > 0) {
            return;
        }
        func_020466e4(func_020d6c00(), 0x80);
        func_020a9ea4(unk_3c);
        subStep_++;
    } else if (sub == 11) {
        BackgroundLoader::AddLockGlobal();
        BackgroundLoader::FreeAllocationsGlobal();
        int n = 8;
        if (func_0202c540(func_0202ae18())) {
            n = 0x14;
        }
        if (func_020aad1c(unk_3c, data_0211e33c, n, 0) == 1) {
            func_020466f4(func_020d6c00(), 0x80);
            subStep_++;
            flags_ &= ~0x200;
        }
        BackgroundLoader::RemoveLockGlobal();
    } else if (sub == 12) {
        messageId_   = 0x37;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 13) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        messageId_   = 0x1a;
        messageStep_ = 0;
        flags_ |= 0x20;
        subStep_++;
    } else if (sub == 14) {
        blinkTimer_ = 0;
        if (!IsAnyKeyPressed()) {
            return;
        }
        SetBookButtonsEnabled(ui_, false, &topBgIndex_);
        pot->flags_ &= ~0x40;
        waitFrames_  = 0;
        nextSubStep_ = 0xf;
        subStep_     = 0x96;
    } else if (sub == 15) {
        pot->flags_ |= 0x100;
        subStep_++;
    } else if (sub == 16) {
        pot->flags_ |= 0x200;
        subStep_++;
    } else if (sub == 17) {
        showResultSprite_ = 1;
        void **name       = func_020e5294(&itemResources_, items_[0].id);
        if (name) {
            memset(recipeNames_[0], 0, 0x80);
            func_020e4864(*name, recipeNames_[0], 1, 0, 0, 0);
            func_02080f8c(ui_, 0x7f, recipeNames_[0]);
        }
        func_020805f4(ui_, 0x7f);
        func_020813ec(ui_, 0x12);
        pot->flags_ |= 0x400;
        unk_42f   = 1;
        int sound = 0x3e;
        bool ok   = IsGreatSuccess();
        if (ok) {
            sound = 0x3f;
            pot_->StartEffectAnimation();
        }
        func_0209c830(data_02109bf4, sound);
        func_02080c04(ui_, 0);
        messageId_   = 0xc;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 0x12) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        bool ok = IsGreatSuccess();
        if (ok) {
            pot->StopEffectAnimation();
        }
        messageId_   = 0x38;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 0x13) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        func_0207fdcc(ui_, 0x12);
        showResultSprite_ = 0;
        pot->flags_ &= ~0x800;
        short msg = 0x1f;
        bool ok   = IsGreatSuccess();
        if (ok) {
            msg = unk_396 + 0x1b;
        }
        flags_ |= 0x20;
        messageId_   = msg;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 0x14) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        short msg = 0x20;
        bool ok   = IsGreatSuccess();
        if (ok) {
            msg = unk_396 + 0x1d;
        }
        messageId_   = msg;
        messageStep_ = 0;
        subStep_     = 0x78;
    } else if (sub == 0x64) {
        pot->flags_ |= 0x100;
        subStep_++;
    } else if (sub == 0x65) {
        pot->flags_ |= 0x200;
        subStep_++;
    } else if (sub == 0x66) {
        book_.RegisterCraft(recipe_->id, -1, &recipeDb_);
        CraftItem(items_, recipe_, brewCount_);
        u8 page     = page_;
        u8 numPages = numPages_;
        BuildIngredientLists();
        page_             = page;
        numPages_         = numPages;
        showResultSprite_ = 1;
        void **name       = func_020e5294(&itemResources_, items_[0].id);
        if (name) {
            memset(recipeNames_[0], 0, 0x80);
            func_020e4864(*name, recipeNames_[0], 1, 0, 0, 0);
            func_02080f8c(ui_, 0x7f, recipeNames_[0]);
        }
        func_020805f4(ui_, 0x7f);
        func_020813ec(ui_, 0x12);
        pot->flags_ |= 0x400;
        unk_42f = 1;
        func_0209c830(data_02109bf4, 0x3e);
        messageId_   = 0xc;
        messageStep_ = 0;
        subStep_++;
    } else if (sub == 0x67) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        messageId_   = 0x38;
        messageStep_ = 0;
        subStep_     = 0x78;
    } else if (sub == 0x78) {
        if (unk_42f != 0) {
            blinkTimer_ = 0;
            return;
        }
        PotAnimation *anim = &pot->anim_;
        if (anim != 0 && anim->busy != 0) {
            return;
        }
        if (!IsAnyKeyPressed()) {
            return;
        }
        func_0207fdcc(ui_, 0x12);
        showResultSprite_ = 0;
        pot->flags_ &= ~0x800;
        messageId_   = 1;
        messageStep_ = 0;
        flags_ |= 0x20;
        subStep_++;
    } else if (sub == 0x79) {
        if (!IsAnyKeyPressed()) {
            return;
        }
        SetBookButtonsEnabled(ui_, true, &topBgIndex_);
        func_0207fdcc(ui_, 0);
        func_0207fdcc(ui_, 1);
        state_   = State_RecipeList;
        subStep_ = 0;
    } else if (sub == 0x96) {
        pot->flags_ |= 0x2000;
        if (++waitFrames_ >= 10) {
            waitFrames_ = 0;
            pot->flags_ &= ~0x2000;
            pot->flags_ |= 0x80;
            subStep_ = nextSubStep_;
        }
    }
}

void AlchemyScreen::UpdateFadeOut() {
    GameResources *res = func_0200fb8c(GameState::GetInstance());
    if (subStep_ == 0) {
        if (cursor_ != 0) {
            func_020e25e8(cursor_);
        }
        SetBrightness(res, -16, 16);
        if (!(flags_ & 2)) {
            func_0209c678(data_02109bf4, 15);
        }
        subStep_++;
    } else if (subStep_ == 1) {
        if (!IsBrightnessTransitionActive(res)) {
            state_   = State_Exit;
            subStep_ = 0;
        }
    }
}

// Labels of the three top menu entries, depending on the highlighted one
void AlchemyScreen::RefreshMenuLabels() {
    short value = -1;
    switch (menuCursor_) {
        case 2: value = 4; break;
        case 3: value = 7; break;
        case 4: break;
    }
    short id = 5;
    for (u8 i = 0; i < 3; i++) {
        func_0208103c(ui_, id, value);
        id++;
        if (value >= 0) {
            value++;
        }
    }
    func_020813ec(ui_, 3);
}

// Page change of the ingredient list (L / R)
void AlchemyScreen::UpdateItemPage() {
    if (!(flags_ & 4)) {
        return;
    }
    bool changed = false;
    if (scrollKeys_ & 0x10) {
        changed = true;
        page_++;
    } else if (scrollKeys_ & 0x20) {
        changed = true;
        page_--;
    }
    if (page_ == 0xff) {
        page_ = numPages_ - 1;
    }
    if (numPages_ <= page_) {
        page_ = 0;
    }
    if (!changed) {
        return;
    }
    int count;
    short pos;
    count = GetCategoryTotal();
    pos   = itemCursor_ - func_02080468(ui_, 0x11);
    pos += page_ * 8;
    if (count <= pos) {
        pos = count - 1;
    }
    pos         = pos % 8;
    itemCursor_ = func_02080468(ui_, 0x11) + pos;
    RefreshIngredientList();
    PreviewSelectedIngredient();
}

void AlchemyScreen::RefreshIngredientList() {
    UiObject *ui = ui_;
    short cat    = categoryCursor_ - 0x5b;
    short *ids   = itemIds_[cat];
    u8 *counts   = itemCounts_[cat];
    u16 total    = itemTotals_[cat];
    int a        = 0x64;
    int b        = 0x6c;
    int c        = 0x74;
    u16 n        = page_ * 8;
    u8 i;
    func_02080b54(ui, 0x11);
    func_02080608(ui, 0x11);
    flags_ &= ~4;
    if (total > 8) {
        flags_ |= 4;
    }
    for (i = 0; i < 8; a = (short) (a + 1), b = (short) (b + 1), c = (short) (c + 1), n++, i++) {
        if (total > n && ids[n] > 0) {
            func_020806c4(ui, a);
            func_02080b40(ui, a);
            func_02080b40(ui, b);
            func_02080b40(ui, c);
            void **r = func_020e5294(&itemResources_, ids[n]);
            if (r) {
                memset(recipeNames_[i], 0, 4);
                func_020e4864(*r, recipeNames_[i], 1, 0, 0, 0);
                func_02080f8c(ui_, a, recipeNames_[i]);
            }
            func_020805f4(ui, a);
            u8 q = counts[n];
            if (selectedItemId_ == ids[n]) {
                q = q - quantity_;
            }
            func_02080fa8(ui, c, q);
        }
    }
    func_02080b40(ui, 0x7c);
    func_02080b40(ui, 0x7d);
    func_02080b40(ui, 0x7e);
    func_02080fa8(ui, 0x7c, page_ + 1);
    func_02080fa8(ui, 0x7d, numPages_);
    func_02081130(ui, 0x11, (flags_ & 4) != 0);
    func_020813ec(ui, 0x11);
}

// Shows the highlighted ingredient in the pot
void AlchemyScreen::PreviewSelectedIngredient() {
    short *list = GetCategoryIds();
    if (list != 0) {
        u16 idx           = (u16) (page_ << 3) + (itemCursor_ - 100);
        CategoryBgTable t = data_ov006_02160030;
        short k           = categoryCursor_ - 0x5b;
        if (k < 0) {
            k = 0;
        }
        if (k > 8) {
            k = 8;
        }
        pot_->SetItem(list[idx], (signed char) t.v[k]);
    }
}

void AlchemyScreen::RefreshRecipePage() {
    pageStart_ = GetPageStart(page_);
    if (pageStart_ == NULL) {
        return;
    }
    flags_ &= ~4;
    if (numPages_ > 1) {
        flags_ |= 4;
    }
    UiObject *ui = ui_;
    func_02080bac(ui, 9);
    func_0208065c(ui, 9);
    func_02080fa8(ui, 0x3a, page_ + 1);
    func_02080fa8(ui, 0x3b, numPages_);
    Recipe *node        = pageStart_;
    const char *unknown = func_020e0434(&messages_, 0x25);
    short id            = 0x29;
    for (u8 i = 0; i < 16; id++, i++) {
        if (node != NULL) {
            func_02080f8c(ui, id, unknown);
            func_02080cc0(ui, id, 3);
            RecipeEntry *e = book_.FindEntry(node->id);
            if (e != NULL) {
                const char *text = unknown;
                if (e->known) {
                    void **r = func_020e5294(&itemResources_, node->resultItem);
                    if (r != NULL) {
                        memset(recipeNames_[i], 0, 4);
                        func_020e4864(*r, recipeNames_[i], 1, 0, 0, 0);
                        text = recipeNames_[i];
                    }
                }
                func_02080f8c(ui, id, text);
                func_020805f4(ui, id);
                func_02080cc0(ui, id, 0xf);
            }
            node = node->next;
        } else {
            func_020806b0(ui, id);
            func_02080b2c(ui, id);
        }
    }
    func_02080c68(ui, 9, 0);
    func_02081ea4(ui, 9, 0);
    func_02081130(ui, 9, (flags_ & 4) ? 1 : 0);
    func_020813ec(ui, 9);
}

void AlchemyScreen::RefreshBrewCount() {
    func_02080fa8(ui_, 0x3d, quantity_);
    func_020813ec(ui_, 10);
    activeElement_ = 10;
}

// Header of the recipe book: filter title and completion rate
void AlchemyScreen::RefreshCompletionRate(bool show) {
    UiObject *ui = ui_;
    func_02080b54(ui, 0xd);
    func_02080b40(ui, 0x56);
    func_0208103c(ui_, 0x56, 0x2b);
    if (state_ != State_RecipeList && state_ != State_BookList) {
        int v = 0x2c;
        if (!show) {
            switch (filterCursor_) {
                case 0x25: v = 0x12; break;
                case 0x26: v = 0x13; break;
            }
        }
        func_0208103c(ui, 0x57, v);
        func_02080b40(ui, 0x57);
        func_020806d8(ui, 0xd, 0x57, 1, 4);
        if (show) {
            u16 n   = book_.CountCraftedRecipes();
            float f = n / 448.0f;
            f *= 10000.0f;
            int r   = (int) f;
            int pct = (r - r % 100) / 100;
            if (n != 0 && pct <= 0) {
                pct = 1;
            }
            if (pct > 100) {
                pct = 100;
            }
            func_02080fa8(ui, 0x58, pct);
            func_02080b40(ui, 0x58);
            func_0208103c(ui_, 0x56, 0x2e);
        }
    }
    func_02081164(ui, 0xd, 1);
    func_020813ec(ui, 0xd);
}

void AlchemyScreen::RefreshFilterIcon() {
    UiObject *ui = ui_;
    int v        = 0x2f;
    switch (filterCursor_) {
        case 0x27: v = 0x14; break;
        case 0x28: v = 0x15; break;
    }
    switch (subFilterCursor_) {
        case 0x48: v = 0x1d; break;
        case 0x49: v = 0x1e; break;
        case 0x4a: v = 0x1f; break;
        case 0x4b: v = 0x20; break;
        case 0x4c: v = 0x21; break;
        case 0x4d: v = 0x22; break;
        case 0x4e: v = 0x23; break;
        case 0x4f: v = 0x24; break;
        case 0x50: v = 0x25; break;
        case 0x51: v = 0x26; break;
        case 0x52: v = 0x27; break;
        case 0x53: v = 0x28; break;
        case 0x40: v = 0x17; break;
        case 0x41: v = 0x18; break;
        case 0x42: v = 0x19; break;
        case 0x43: v = 0x1a; break;
        case 0x44: v = 0x1b; break;
        case 0x45: v = 0x1c; break;
    }
    func_0208103c(ui, 0x59, v);
    func_0208108c(ui, 0x59);
    func_02081164(ui, 0xe, 1);
    func_020813ec(ui, 0xe);
    func_02080c68(ui, 0xe, 2);
    func_02080c20(ui, 0xe);
}

void AlchemyScreen::RefreshSortButton() {
    UiObject *ui = ui_;
    func_0208103c(ui, 0x5a, (short) (sortOrder_ + 0x29));
    func_02080798(ui, 0x5a, 1);
    func_02081164(ui, 0xf, 1);
    func_020813ec(ui, 0xf);
    func_02080c68(ui, 0xf, 2);
    func_02080c20(ui, 0xf);
}

// Merges the three ingredients (same item: counts added) and looks up the
// recipe they make
Recipe *AlchemyScreen::FindMatchingRecipe(short *inIds, u8 *inCounts, u8 *outCount) {
    short ids[3] = {0};
    u8 counts[3] = {0};
    for (u8 i = 0; i < 3; i++) {
        short id = inIds[i];
        if (id > 0) {
            for (u8 j = 0; j < 3; j++) {
                if (id == ids[j]) {
                    ids[j] = id;
                    counts[j] += inCounts[i];
                    break;
                } else if (ids[j] <= 0) {
                    ids[j]    = id;
                    counts[j] = inCounts[i];
                    break;
                }
            }
        }
    }
    return func_02071da4(&recipeDb_, ids, counts, outCount);
}

void AlchemyScreen::StartResultSpriteLoad(ItemData *item) {
    if (item != NULL) {
        BackgroundLoader *loader = BackgroundLoader::GetInstance();
        func_02075cdc(&resultSprite_);
        resultSprite_.unk_3c = 5;
        resultSprite_.unk_5e = 1;
        resultSprite_.vram   = func_0203be4c(func_0203bd08()) + 0x1b8;
        resultSprite_.unk_38 = 0x10000;
        resultSprite_.unk_40 = 0;
        int id               = func_020de234(item, 0);
        char path[0x40];
        sprintf(path, data_ov006_02160360, item->spriteGroup, id);
        resultSpriteTask_ = loader->QueueLoadFile(path, NULL);
    }
}

bool AlchemyScreen::FinishResultSpriteLoad() {
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(resultSpriteTask_) != 0) {
        SafeAllocator *alloc = &heaps_[9];
        alloc->Reset();
        void *data        = NULL;
        unsigned int size = 0;
        loader->GetLoadedFileByID(resultSpriteTask_, &data, &size);
        func_02076080(&resultSprite_, alloc, data, size);
        loader->RemoveTask(resultSpriteTask_);
        resultSpriteTask_ = -1;
        return true;
    }
    return false;
}
