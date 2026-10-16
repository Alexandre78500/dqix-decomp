#pragma once

#include "../Bestiary/BestiaryScreen.h"
#include "AlchemyBook.h"

struct RenderCommandHandler;

// 2D sprite built from a .spr file (init func_02075cdc, build func_02076080,
// scale func_02076988, draw func_02075db0). sizeof == 0x70.
struct Sprite2D {
    u8 unk_0[0x14];
    char *vram;                 // 0x14
    u8 unk_18[0x20];
    int unk_38;
    int unk_3c;
    int unk_40;
    int unk_44;
    u8 unk_48[0x4];
    int unk_4c;
    u8 unk_50[0xe];
    u8 unk_5e;
    u8 unk_5f[0x11];
};

// Item sprite shown over the pot (result and the three ingredients),
// loaded from "data/ani/d_%c%03d.spr". sizeof == 0x7c.
struct ItemSpriteSlot {
    Sprite2D sprite;            // 0x0
    int taskId;                 // 0x70: BackgroundLoader task, -1 = none
    short itemId;               // 0x74
    short x;                    // 0x76
    short y;                    // 0x78
    u8 loading;                 // 0x7a
    u8 unk_7b;

    void Reset();
};

// Item description used to load its sprite (init func_020de1d4, sprite number
// func_020de234, sprite data func_020deef4). sizeof == 0x74.
struct ItemData {
    void *unk_0;                // 0x0: = buffer20
    void *unk_4;                // 0x4: = buffer40
    u8 unk_8[0x8];
    u32 unk_10_0 : 20;          // 0x10
    u32 spriteGroup : 8;        // letter of "d_%c%03d.spr"
    u32 unk_10_28 : 4;
    u8 unk_14[0x4];
    short unk_18;               // item added to the bag by CraftItem
    u8 unk_1a[0x6];
    u8 buffer20[0x20];          // 0x20
    u8 buffer40[0x30];          // 0x40
    short id;                   // 0x70: item id, -1 = none
    u8 unk_72[0x2];

    void Initialize();
};

// Icon of an item (func_020e4864 copies it out of the item resources).
// sizeof == 0x80.
struct ItemIcon {
    u8 data[0x80];
};

// Parameters of PotAnimation::Play (init func_02078484, on the stack).
// sizeof == 0x50.
struct AnimParams {
    char name[0x11];            // 0x0
    u8 flags;                   // 0x11
    short unk_12;
    u8 unk_14[0x18];
    int x;                      // 0x2c
    int y;
    int z;
    u8 unk_38[0xc];
    int scaleX;                 // 0x44
    int scaleY;
    int scaleZ;
};

// Animated effect object (init func_020dbd9c, update func_020dbdc0, play
// func_020dbfa4, destroy func_020dbebc, load func_020dbf18). sizeof == 0x18.
struct PotAnimation {
    u8 unk_0[0x10];
    int state;                  // 0x10: 2 = finished
    u8 busy;                    // 0x14
    u8 unk_15[0x3];
};

// Ingredient panel on the top screen, overlay 23 code (init
// func_ov023_021dc134, update func_ov023_021dc488, draw func_ov023_021dfad8,
// set ingredient func_ov023_021dceac). sizeof == 0x79c.
struct IngredientPanel {
    u8 unk_0[0x48];
    void *resultBg;             // 0x48
    u8 unk_4c[0x728];
    // 0x4: special view, 0x800: panel shown, 0x1000: no result background
    u16 flags;                  // 0x774
    u8 ready;                   // 0x776
    u8 unk_777[0x6];
    signed char mode;           // 0x77d: 2 = special view
    u8 unk_77e[0x1d];
    u8 unk_79b_0 : 1;           // 0x79b
    u8 hidden : 1;
    u8 unk_79b_2 : 6;
};

// The alchemy pot: 3D models, item sprites, top screen panel and the loading
// of the recipe backgrounds. Owned by AlchemyScreen. sizeof == 0x12c0.
class AlchemyPot {
public:
    typedef void (AlchemyPot::*StateHandler)();

    enum Flags {
        Flag_LoadBackground = 0x1,
        Flag_LoadRecipe     = 0x2,
        Flag_Frozen         = 0x8,      // models are not updated nor destroyed
        Flag_Brewing        = 0x10,
        Flag_RecipeShown    = 0x40,
        Flag_Visible        = 0x1000,
    };

    ItemIcon ingredientIcons_[3];       // 0x0
    SafeAllocator *heaps_;              // 0x180: 6 heaps, sizes in data_ov006_0215ff7c
    SafeAllocator *bgHeap_;             // 0x184: swapped with nextBgHeap_
    SafeAllocator *nextBgHeap_;         // 0x188
    SafeAllocator heap_;                // 0x18c
    void *savedScene_;                  // 0x1a0: freed with func_020100c4
    void *bgBuffer_;                    // 0x1a4: AlchemyScreen::buffer_
    u8 *ownedCounts_;                   // 0x1a8: 3 counts
    void *itemResources_;               // 0x1ac: AlchemyScreen::itemResources_
    TextArchive *messages_;             // 0x1b0: AlchemyScreen::messages_
    Recipe *shownRecipe_;               // 0x1b4
    Recipe *recipe_;                    // 0x1b8: being loaded
    int unk_1bc;
    RecipeEntry *recipeEntry_;          // 0x1c0
    u8 *unk_1c4;                        // 0x1c4: 0x80 bytes
    ScreenLayout *layout_;              // 0x1c8
    BgLayer *layers_;                   // 0x1cc: 2 layers
    TitleBg *titleBg_;                  // 0x1d0
    UiObject *ui_;                      // 0x1d4
    int unk_1d8;
    int unk_1dc;
    Sprite *sprites_;                   // 0x1e0: 17 sprites
    SpriteBank spriteBank_;             // 0x1e4
    Object3D models_[5];                // 0x238
    u8 unk_594[0x38];
    u8 camera_[0x2c8];                  // 0x5cc
    ItemSpriteSlot slots_[4];           // 0x894: result, then the 3 ingredients
    SafeAllocator slotHeaps_[4];        // 0xa84
    int taskId_;                        // 0xad4
    int bgTaskId_;                      // 0xad8
    u8 state_;                          // 0xadc: index in the state table
    u8 loadStep_;                       // 0xadd
    u8 recipeLoadStep_;                 // 0xade
    u8 bgLoadStep_;                     // 0xadf
    u8 unk_ae0;
    u8 animStep_;                       // 0xae1
    u16 flags_;                         // 0xae2: see Flags
    IngredientPanel panel_;             // 0xae4
    SafeAllocator *panelHeap_;          // 0x1280
    SafeAllocator *unk_1284;            // 0x1284
    u8 resultBg_[0x18];                 // 0x1288: init func_020de848
    PotAnimation anim_;                 // 0x12a0
    short itemId_;                      // 0x12b8: item shown instead of a recipe
    signed char itemBgIndex_;           // 0x12ba
    u8 numIngredientSlots_;             // 0x12bb
    u8 animState_;                      // 0x12bc: 0 idle, 1 started, 2 looping, 3 stopping
    u8 restoreBg0_;                     // 0x12bd
    u8 unk_12be[0x2];

    bool IsPanelVisible() {
        return (panel_.flags & 0x800) ? 1 : 0;
    }

    void Allocate(SafeAllocator *alloc, int arg);
    void Initialize();
    void Destroy();
    void Update(int frameTime);
    void DrawSlotSprites();
    void Draw3D();
    void Draw();
    void UpdateModelAnimations();
    void Freeze();
    void SetIngredientIcons(short *ids, u8 *counts);
    void EnterPotView();
    void ClearRecipe();
    void PreviewRecipe(Recipe *recipe, RecipeEntry *entry, u8 *counts);
    void SetRecipe(Recipe *recipe, RecipeEntry *entry, u8 *counts);
    void SetItem(short itemId, signed char bgIndex);
    void UpdateRecipeLoad();
    void UpdateLoad();
    void UpdateReady();
    void UpdateBackgroundLoad();
    void UpdateEffectAnimation();
    void ResetSlots();
    void LoadSlotSprites(ItemData *items);
    void UpdateSlotLoads();
    bool IsLoadingSlots();
    bool SetPanelVisible(bool visible);
    void HidePanel();
    void SetPanelQuantity(u8 quantity);
    void RefreshPanel();
    void StartEffectAnimation();
    void StopEffectAnimation();
};

// Render hook on the pot model: captures the matrices of the 3 ingredient
// bones (data_ov006_02160380 / data_ov006_0216038c)
void CaptureIngredientBones(RenderCommandHandler *handler);
void SetupPotCamera(void *camera);
void SetupBookListButtons(ScreenLayout *layout);
// Copy of ScreenLayout::FindElement (overlay 14)
ScreenLayout::Element *FindLayoutElement(ScreenLayout *layout, int id);
void SetupIngredientListButtons(ScreenLayout *layout);
void DecodeFilterButton(int buttonId, signed char *type, signed char *subtype);
// Removes the ingredients of `count` brews from the bag and adds the result
void CraftItem(ItemData *result, Recipe *recipe, u8 count);
