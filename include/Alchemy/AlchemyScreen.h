#pragma once

#include "AlchemyPot.h"

// Touch state of the stylus (data_02114e54)
struct TouchState {
    u8 unk_0[0x55];
    u8 touched;                 // 0x55
};

// Data of a TouchCursor
struct TouchCursorData {
    u8 unk_0[0x28];
    u8 unk_28[0x16];            // 0x28: func_020e2cc4 / func_020e2d24
    u8 visible;                 // 0x3e
};

// Cursor driven by the touch screen and the D-pad (init func_020e2490, update
// func_020e263c, active func_020e28dc). sizeof == 0x24.
struct TouchCursor {
    u8 unk_0[0x10];
    TouchCursorData *data;      // 0x10
    u8 unk_14[0x10];

    TouchCursorData *GetData() {
        return data;
    }
};

// Held-button input of a menu (init func_02081ee4, update func_02081f20,
// reset func_0208203c). sizeof == 0xc.
struct MenuInput {
    u16 *key;                   // 0x0: key of the current entry
    u8 unk_4[0x8];

    u16 GetKey();
};

// Item names and icons (itemname, init func_020e526c, load func_020e5604,
// lookup func_020e5294). sizeof == 0xc.
struct ItemResources {
    u8 unk_0[0xc];
};

// Bottom screen of the alchemy pot: menus, recipe book, ingredient lists
// and the brewing sequence. Called by overlay 17. The pot itself (models and
// top screen) is AlchemyPot.
class AlchemyScreen {
public:
    typedef void (AlchemyScreen::*StateHandler)();

    enum State {
        State_Load             = 0,
        State_FadeIn           = 1,
        State_BookFilter       = 2,    // recipe book only (flags_ & 2)
        State_BookList         = 3,
        State_TopMenu          = 4,
        State_FirstIngredient  = 5,    // free alchemy: 3 ingredients
        State_SecondIngredient = 6,
        State_ThirdIngredient  = 7,
        State_FreeBrewing      = 8,
        State_RecipeFilter     = 9,    // alchemy from the recipe book
        State_RecipeList       = 10,
        State_RecipeBrewing    = 11,
        State_FadeOut          = 12,
        State_Exit             = 13,
    };

    int textScroll_;                    // 0x0: message window, fixed point << 4
    char **recipeNames_;                // 0x4: 16 lines of 0x80 bytes
    void *buffer_;                      // 0x8: 0x4000 bytes, AlchemyPot::bgBuffer_
    SafeAllocator *heaps_;              // 0xc: 10 heaps, sizes in data_ov006_02160054
    AlchemyPot *pot_;                   // 0x10
    UiObject *ui_;                      // 0x14
    TouchCursor *cursor_;               // 0x18
    BgLayer *layers_;                   // 0x1c: 2 layers
    TitleBg *titleBgs_;                 // 0x20: 7 objects
    Sprite *sprites_;                   // 0x24: 29 sprites
    CursorAnim *cursorAnim_;            // 0x28
    Recipe *recipeList_;                // 0x2c: sorted list of the recipe book
    Recipe *pageStart_;                 // 0x30
    Recipe *recipe_;                    // 0x34: recipe being brewed
    RecipeEntry *recipeEntries_;        // 0x38: 0x1d7 entries
    u8 unk_3c[0x8];
    short *selection_;                  // 0x44: cursor of the active element
    // Player's items per category (9 categories, capacities in
    // data_ov006_0215fffe): ids (<= 0 = free), counts and number of ids
    short **itemIds_;                   // 0x48
    u8 **itemCounts_;                   // 0x4c
    u16 *itemTotals_;                   // 0x50
    BgLayer bgLayer_;                   // 0x54
    AlchemyBook book_;                  // 0x74
    MenuInput menuInput_;               // 0x9c
    u16 unk_a8;
    u8 unk_aa;
    u8 unk_ab;
    SpriteBank spriteBank_;             // 0xac
    ScreenLayout layout_;               // 0x100: lay_rrb buttons
    RecipeDatabase recipeDb_;           // 0x14c
    TextArchive messages_;              // 0x158: str_ren
    ItemResources itemResources_;       // 0x170
    ItemData items_[4];                 // 0x17c: result, then the 3 ingredients
    int frameTime_;                     // 0x34c
    int scrollKeys_;                    // 0x350
    int taskId_;                        // 0x354
    int unk_358;                        // 0x358: BackgroundLoader task
    short unk_35c;
    short prevCursor_;                  // 0x35e
    short menuCursor_;                  // 0x360: top menu, 2 recipes, 3 alchemy, 4 quit
    short categoryCursor_;              // 0x362: 0x5b + category
    short itemCursor_;                  // 0x364: 100 + line
    short filterCursor_;                // 0x366: 0x24..0x28
    short subFilterCursor_;             // 0x368: button id, see DecodeFilterButton
    short recipeCursor_;                // 0x36a: 0x29 + line
    short confirmCursor_;               // 0x36c: 0x22 = yes
    short activeElement_;               // 0x36e: UI element of the cursor
    short selectedItemId_;              // 0x370
    u16 pendingCategories_[3];          // 0x372: ingredients put in the pot
    short pendingIds_[3];               // 0x378
    short messageId_;                   // 0x37e: -1 = no message
    u16 textPos_;                       // 0x380
    union {
        short successRate_;             // 0x382: percent
        volatile short successRateV_;   // re-read at each test by UpdateFreeBrewing/UpdateRecipeBrewing
    };
    signed char filterType_;            // 0x384
    signed char filterSubtype_;         // 0x385
    u8 sortOrder_;                      // 0x386
    u8 brewCount_;                      // 0x387
    u8 page_;                           // 0x388
    u8 numPages_;                       // 0x389
    u8 selectedCategory_;               // 0x38a
    u8 quantity_;                       // 0x38b
    u8 pendingCounts_[3];               // 0x38c
    u8 state_;                          // 0x38f: see State
    u8 subStep_;                        // 0x390
    u8 unk_391;
    u8 messageStep_;                    // 0x392
    u8 repeatTimer_;                    // 0x393
    // 0x2: recipe book only, 0x4: several pages, 0x8: quantity buttons,
    // 0x10: ingredient list, 0x20: message, 0x40: great success,
    // 0x80: book buttons, 0x100: wait for a key, 0x200: pause
    u16 flags_;                         // 0x394
    u8 unk_396;                         // 0x396: from the protagonist data
    u8 topBgIndex_;                     // 0x397
    u8 ownedCounts_[4];                 // 0x398: AlchemyPot::ownedCounts_
    int soundMode_;                     // 0x39c
    int unk_3a0;
    int soundEnabled_;                  // 0x3a4
    u32 soundTimer_;                    // 0x3a8
    int soundPlayed_;                   // 0x3ac
    int soundDelay_;                    // 0x3b0: < 0 = silent
    int showResultSprite_;              // 0x3b4
    Sprite2D resultSprite_;             // 0x3b8
    int resultSpriteTask_;              // 0x428
    signed char blinkTimer_;            // 0x42c
    u8 waitFrames_;                     // 0x42d
    u8 nextSubStep_;                    // 0x42e
    u8 unk_42f;
    u8 quantityUp_;                     // 0x430
    u8 quantityDown_;                   // 0x431
    u8 showPanel_;                      // 0x432
    u8 exitRequested_;                  // 0x433
    u8 exitLatched_;                    // 0x434

    bool IsGreatSuccess() {
        return (flags_ & 0x40) != 0;
    }

    void Allocate(SafeAllocator *alloc, int arg);
    void Initialize();
    void Destroy();
    int Update(int frameTime);
    void DrawModel();
    void DrawMenu();
    void DrawInfo();
    void RequestExit();
    void DrawCursor();
    void DrawListButtons();
    void DrawQuantityButtons();
    void DrawBookButtons();
    u8 IsConfirmPressed();
    int IsBackPressed();
    void HandleCloseKey();
    int IsAnyKeyPressed();
    void CountRecipePages();
    void SetPageStartFor(Recipe *recipe);
    Recipe *GetPageStart(int page);
    Recipe *GetPageRecipe(short index);
    void BuildIngredientLists();
    void AddIngredient(u8 category, short id, u8 count);
    void ReturnPendingIngredient();
    void ReserveIngredient(u8 category, short id, u8 count);
    short *GetCategoryIds();
    u8 *GetCategoryCounts();
    u16 GetCategoryTotal();
    bool HasCategoryItems();
    void GetSelectedIngredient(u8 *category, short *id, u8 *count);
    void IncreaseQuantity();
    void DecreaseQuantity();
    void OpenIngredientList();
    void CountIngredientPages();
    bool UpdateIngredientCursor();
    bool HandleListButtons();
    void OpenQuantitySelect();
    int HandleQuantityInput();
    int HandleBrewCountInput();
    int PollBookAction();
    void ToggleSortOrder();
    void OpenConfirmDialog();
    int HandleConfirmDialog();
    void PlayPotSound();
    void ResetPotSound();
    void UpdatePotSound();
    void UpdateMessageWindow();
    void StartItemSpriteLoad(bool unused);
    bool FinishItemSpriteLoad(bool useBonus);
    void ApplyQuantityToPanel();
    void UpdateLoad();
    void UpdateFadeIn();
    void UpdateBookFilter();
    void UpdateBookList();
    void UpdateTopMenu();
    void UpdateFirstIngredient();
    void UpdateSecondIngredient();
    void UpdateThirdIngredient();
    void UpdateFreeBrewing();
    void UpdateRecipeFilter();
    void UpdateRecipeList();
    void SetBookButtonsEnabled(UiObject *ui, bool enable, u8 *active);
    void UpdateRecipeBrewing();
    void UpdateFadeOut();
    void RefreshMenuLabels();
    void UpdateItemPage();
    void RefreshIngredientList();
    void PreviewSelectedIngredient();
    void RefreshRecipePage();
    void RefreshBrewCount();
    void RefreshCompletionRate(bool show);
    void RefreshFilterIcon();
    void RefreshSortButton();
    Recipe *FindMatchingRecipe(short *ids, u8 *counts, u8 *outCount);
    void StartResultSpriteLoad(ItemData *item);
    bool FinishResultSpriteLoad();
};

void DrawSpriteAt(SpriteBank *bank, Sprite *sprite, int x, int y, u8 a, u8 b);
bool IsUiElementShown(UiObject *ui, int id);
