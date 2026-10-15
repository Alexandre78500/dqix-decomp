#include "Alchemy/AlchemyScreen.h"
#include "Filesystem/BackgroundLoader.h"
#include "GameState/GameState.h"
#include "Graphics/Model3D.h"
#include "Graphics/NSBXX/GeometryFifo.h"
#include "Graphics/NSBXX/RenderCommands.h"
#include "Graphics/NSBXX/RenderConfig.h"
#include "Resource/Brightness.h"
#include "Resource/GameResources.h"
#include "System/ColorEffects.h"
#include "System/Graphics.h"
#include "System/Matrix.h"
#include "std_library_functions.h"

typedef signed char s8;

// Pot state table (data_ov006_0215ff94): UpdateLoad, UpdateReady, null
struct PotStateTable {
    AlchemyPot::StateHandler fn[3];
};

// Ingredient slots shown for each value of numIngredientSlots_
struct SlotPositions {
    u8 v[4][4];
};

struct BgPalettes {
    u8 v[3];
};

// Letters of the pot model variants, indexed by the pot level
struct PotLetters {
    char c[8];
};

struct Unk02012fe4Sub {
    u8 pad[0xb3c];
    int unk_b3c;
};

struct Unk02012fe4 {
    u8 pad[0x1840];
    Unk02012fe4Sub sub;

    Unk02012fe4Sub *GetSub() {
        return &sub;
    }
};

// Equipment slot of a party member
struct EquipSlot {
    u8 pad_00[8];
    u32 kind : 4;
    u32 rest : 28;
    u8 pad_0c[0x18 - 0xc];
    short id;
    u8 pad_1a[0x20 - 0x1a];
};

struct PartyMember {
    u8 pad_00[0x150];
    u8 *data;
};

struct PartyInventory {
    u8 pad_00[0xf78];
    u8 party[4];
    u8 partyCount;
};

struct CraftTemp {
    u8 data[0x24];
};

extern "C" {
    // ingredient panel (overlay 23)
void func_ov023_021dbfd0(IngredientPanel *panel, SafeAllocator *alloc);
void func_ov023_021dbfa8(IngredientPanel *panel, int arg);
void func_ov023_021dc134(IngredientPanel *panel, int a, int b);
void func_ov023_021dc354(IngredientPanel *panel);
void func_ov023_021dc488(IngredientPanel *panel);
void func_ov023_021dfad8(IngredientPanel *panel);
void func_ov023_021dcd78(IngredientPanel *panel);
void func_ov023_021dcd90(IngredientPanel *panel, int a);
void func_ov023_021dcae0(IngredientPanel *panel, int id);
void func_ov023_021dceac(IngredientPanel *panel, u8 i, ItemIcon *icon, u8 qty, u8 count, u8 owned);
void func_ov023_021dced4(IngredientPanel *panel, int a);
void func_ov023_021dd344(IngredientPanel *panel);
void func_ov023_021dd404(IngredientPanel *panel);
void func_ov023_021dd4cc(IngredientPanel *panel, int mode);
void func_ov023_021e20c0(ScreenLayout *layout);
void func_ov023_021e20f0(ScreenLayout *layout, SafeAllocator *alloc, void *data, unsigned int size);

    // camera
void func_020a2010(void *camera);
void func_0202e5c8(void *camera, int x, int y, int z);
void func_0202e5d8(void *camera, int x, int y, int z);
void func_0202e0a4(void *camera);
void func_020a27a0(void *camera);
void func_0202ec84(void *camera, Vector3fix *pos, int *x, int *y);

    // backgrounds and UI
void func_0204af64(BgLayer *layer);
void func_0204b010(BgLayer *layer, int a);
void func_0204b11c(BgLayer *layer, int a);
void func_0204b12c(BgLayer *layer, SafeAllocator *alloc);
void func_0204b174(BgLayer *layer, void *data, SafeAllocator *alloc, unsigned int size);
void func_0204b2e0(BgLayer *layer, void *data);
void func_0204b3a0(BgLayer *layer, void *data);
void func_0204b5b4(BgLayer *layer, int a);
void func_0204b5e8(BgLayer *layer, int a, int b);
void func_0204c684(TitleBg *bg);
void func_0204c7a8(TitleBg *bg, SafeAllocator *alloc, void *buffer, int size);
int func_02046900(void *archive);
void *func_020467f0(void *archive, int index, void *outUnk, void *outArg);
void func_0207f7f0(UiObject *ui, TitleBg *bgs, int n);
void func_0207f84c(UiObject *ui);
void func_0207f914(UiObject *ui, SafeAllocator *alloc, const char *a, const char *b);
int func_0207f9f4(UiObject *ui);
void func_0207fc6c(UiObject *ui, int frameTime);
void func_0207fcb8(UiObject *ui);
void func_0207fd44(UiObject *ui);
void func_0207fd88(UiObject *ui);
void func_0205a198(Sprite *sprite);
void func_0205a444(SpriteBank *bank);
void func_0205a528(SpriteBank *bank, void *data, int arg, SafeAllocator *alloc);

    // Sprite2D
void func_02075cdc(Sprite2D *sprite);
void func_02076080(Sprite2D *sprite, SafeAllocator *alloc, void *data, unsigned int size);
void func_02076988(Sprite2D *sprite, int sx, int sy);
void func_02075db0(Sprite2D *sprite, int x, int y);
void *func_0203bd08();
char *func_0203be40(void *p);

    // items
void **func_020e5294(void *resources, int id);
void func_020e4864(void *src, ItemIcon *dst, int a, int b, int c, int d);
int func_020de234(ItemData *item, int arg);
void func_020de824(CraftTemp *tmp);
void func_020de848(void *bg);
void func_020de980(void *bg, SafeAllocator *alloc, void *data, unsigned int size, int id);
int func_020dedd0(void *bg, int id);
const char *func_020e0434(TextArchive *archive, short id);
void func_0207cbe8(SafeAllocator *alloc);
void func_0207d300(SafeAllocator *alloc, int id, signed char count, int a);
void func_0207cc0c(SafeAllocator *alloc);
void func_02086d88(void *inventory, int id);

    // PotAnimation
void func_020dbd9c(PotAnimation *anim);
void func_020dbdc0(PotAnimation *anim);
void func_020dbebc(PotAnimation *anim);
void func_020dbf18(PotAnimation *anim, const char *name, SafeAllocator *alloc, void *p, int a);
void func_020dbfa4(PotAnimation *anim, AnimParams *params);
void func_02078484(AnimParams *params);

    // game state
GameResources *func_0200fb8c(GameState *state);
void *func_020100bc(GameState *state);
void func_020100c4(GameState *state, void *scene);
void *func_020100f8(GameState *state);
PartyInventory *func_02010828(GameState *state);
PartyMember *func_0200ff1c(GameState *state, u8 index);
Unk02012fe4 *func_02012fe4();
void func_0207df50(void *p);
void func_0207df90(void *p);
void func_0207dfac(void *p);
void func_020407b4(Object3D *obj, int x, int y, int z);
void func_02059f38(Object3D *obj, Vector3fix *pos);
void func_0205ebc0(void *p, int a, int b);
void func_0205ebfc(void *p, int a, int b);
void func_0205ebec(void *p);
void G3X_ResetMtxStack();

extern const PotStateTable data_ov006_0215ff94;
extern const AlchemyPot::StateHandler data_020e6d5c;
extern const u8 data_ov006_0215ff5c[];
extern const SlotPositions data_ov006_0215ff6c;
extern const unsigned int data_ov006_0215ff7c[6];
extern const BgPalettes data_ov006_0215ff54;
extern const PotLetters data_ov006_0215ff64;
extern int data_ov006_02160380[3];
extern Matrix4x3 *data_ov006_0216038c[3];
extern int data_0210a00c;
extern char data_02108760[];
extern char data_ov006_02160080[];
extern char data_ov006_02160087[];
extern char data_ov006_02160089[];
extern char data_ov006_0216008b[];
extern char data_ov006_02160092[];
extern char data_ov006_02160094[];
extern char data_ov006_02160096[];
extern char data_ov006_0216009e[];
extern char data_ov006_021600a0[];
extern char data_ov006_021600a9[];
extern char data_ov006_021600af[];
extern char data_ov006_021600c4[];
extern char data_ov006_021600d7[];
extern char data_ov006_021600f0[];
extern char data_ov006_021600f7[];
extern char data_ov006_0216010b[];
extern char data_ov006_02160125[];
extern char data_ov006_0216013d[];
extern char data_ov006_02160156[];
extern char data_ov006_02160172[];
extern char data_ov006_0216018b[];
extern char data_ov006_02160190[];
extern char data_ov006_02160195[];
extern char data_ov006_0216019a[];
extern char data_ov006_021601b6[];
extern char data_ov006_021601cc[];
extern char data_ov006_021601e5[];
}

namespace StaticInit {
    extern "C" int data_ov006_02160380[3];
}

void CaptureIngredientBones(RenderCommandHandler *handler) {
    if (!(handler->flags_ & 0x10)) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        if (data_ov006_02160380[i] == ((handler->flags_ & 0x10) ? handler->currentBoneMatrix_ : -1) &&
            data_ov006_0216038c[i] != NULL)
        {
            GetCurrentPositionAndDirectionMatrices(data_ov006_0216038c[i], NULL);
            MTX_Concat43(data_ov006_0216038c[i], RenderConfig::GetInverseViewMatrix(), data_ov006_0216038c[i]);
        }
    }
}

void SetupPotCamera(void *camera) {
    if (camera == NULL) {
        return;
    }
    func_020a2010(camera);
    func_0202e5c8(camera, 0, -0x1e8f, 0);
    func_0202e5d8(camera, 0, 0x2eb8, 0xa000);
    func_0202e0a4(camera);
    func_020a27a0(camera);
}

void AlchemyPot::Allocate(SafeAllocator *alloc, int arg) {
    heaps_      = (SafeAllocator *) alloc->Allocate(0x78);
    bgHeap_     = (SafeAllocator *) alloc->Allocate(0x14);
    nextBgHeap_ = (SafeAllocator *) alloc->Allocate(0x14);
    unk_1c4     = (u8 *) alloc->Allocate(0x80);
    layout_     = (ScreenLayout *) alloc->Allocate(0x4c);
    layers_     = (BgLayer *) alloc->Allocate(0x40);
    titleBg_    = (TitleBg *) alloc->Allocate(0xe0);
    ui_         = (UiObject *) alloc->Allocate(0x40);
    sprites_    = (Sprite *) alloc->Allocate(0x2a8);
    bgHeap_->CreateTypeA(alloc->Allocate(0x258), 0x258);
    nextBgHeap_->CreateTypeA(alloc->Allocate(0x258), 0x258);
    bgHeap_->Reset();
    nextBgHeap_->Reset();
    panelHeap_ = (SafeAllocator *) alloc->Allocate(0x14);
    unk_1284   = (SafeAllocator *) alloc->Allocate(0x14);
    panelHeap_->CreateTypeA(alloc->Allocate(0x1e00), 0x1e00);
    panelHeap_->Reset();
    unk_1284->CreateTypeA(alloc->Allocate(0x80), 0x80);
    unk_1284->Reset();
    func_ov023_021dbfd0(&panel_, panelHeap_);
    func_ov023_021dbfa8(&panel_, arg);

    for (u8 i = 0; i < 6; i++) {
        unsigned int size = data_ov006_0215ff7c[i];
        heaps_[i].CreateTypeA(alloc->Allocate(size), size);
        heaps_[i].Reset();
    }
    for (u8 i = 0; i < 4; i++) {
        slotHeaps_[i].CreateTypeA(alloc->Allocate(0x280), 0x280);
        slotHeaps_[i].Reset();
    }
    unk_1c4[0] = 0;
    func_ov023_021e20c0(layout_);
    for (u8 i = 0; i < 2; i++)
        func_0204af64(&layers_[i]);
    for (u8 i = 0; i < 1; i++)
        func_0204c684(&titleBg_[i]);
    func_0207f84c(ui_);
    for (u8 i = 0; i < 17; i++)
        func_0205a198(&sprites_[i]);
}

void AlchemyPot::Initialize() {
    memset(ingredientIcons_, 0, sizeof(ingredientIcons_));
    heaps_         = NULL;
    bgHeap_        = NULL;
    nextBgHeap_    = NULL;
    savedScene_    = NULL;
    ownedCounts_   = NULL;
    itemResources_ = NULL;
    messages_      = NULL;
    shownRecipe_   = NULL;
    recipe_        = NULL;
    unk_1bc        = 0;
    recipeEntry_   = NULL;
    unk_1c4        = NULL;
    layout_        = NULL;
    layers_        = NULL;
    titleBg_       = NULL;
    ui_            = NULL;
    unk_1d8        = 0;
    unk_1dc        = 0;
    sprites_       = NULL;
    unk_1284       = NULL;
    panelHeap_     = NULL;
    func_ov023_021dc134(&panel_, -1, 1);
    panel_.flags |= 0x210;
    func_0205a444(&spriteBank_);
    models_[0].Initialize();
    models_[1].Initialize();
    models_[2].Initialize();
    models_[3].Initialize();
    models_[4].Initialize();
    memset(unk_594, 0, sizeof(unk_594));
    heap_.ResetAllocatorPointer();
    for (u8 i = 0; i < 4; i++) {
        slots_[i].Reset();
        slotHeaps_[i].ResetAllocatorPointer();
    }
    taskId_             = -1;
    bgTaskId_           = -1;
    state_              = 0;
    loadStep_           = 0;
    recipeLoadStep_     = 0;
    bgLoadStep_         = 0;
    unk_ae0             = 0;
    flags_              = 0;
    numIngredientSlots_ = 0;
    itemId_             = 0;
    itemBgIndex_        = 0;
    animState_          = 0;
    func_020dbd9c(&anim_);
    restoreBg0_ = 0;
}

void ItemSpriteSlot::Reset() {
    func_02075cdc(&sprite);
    taskId = -1;
    itemId = -1;
    x = y   = 0;
    loading = 0;
}

void AlchemyPot::Destroy() {
    func_ov023_021dc354(&panel_);
    ResetSlots();
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (taskId_ >= 0) {
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    if (bgTaskId_ >= 0) {
        loader->RemoveTask(bgTaskId_);
        bgTaskId_ = -1;
    }
    if (heap_.GetSignedAllocator()) heap_.Destroy();

    SafeAllocator *list[7] = {bgHeap_, nextBgHeap_, bgHeap_, nextBgHeap_, panelHeap_, unk_1284};
    for (u8 i = 0; list[i] != NULL; i++) {
        if (list[i]->GetSignedAllocator()) list[i]->Destroy();
    }
    for (u8 i = 0; i < 6; i++) {
        if (heaps_[i].GetSignedAllocator()) heaps_[i].Destroy();
    }
    for (u8 i = 0; i < 4; i++) {
        if (slotHeaps_[i].GetSignedAllocator()) slotHeaps_[i].Destroy();
    }
    if (savedScene_) {
        GameState *gs = GameState::GetInstance();
        func_020100c4(gs, savedScene_);
        savedScene_ = NULL;
    }
    if (!(flags_ & Flag_Frozen)) {
        models_[0].Destroy();
        models_[1].Destroy();
        models_[2].Destroy();
        models_[3].Destroy();
        models_[4].Destroy();
    }
    func_020dbebc(&anim_);
    Initialize();
}

void AlchemyPot::Update(int frameTime) {
    UpdateBackgroundLoad();
    if (ui_ != 0) {
        func_0207fc6c(ui_, frameTime);
    }
    UpdateSlotLoads();
    UpdateEffectAnimation();
    PotStateTable t = data_ov006_0215ff94;
    t.fn[2]         = data_020e6d5c;
    if (t.fn[state_] != 0) {
        (this->*t.fn[state_])();
        UpdateModelAnimations();
    }
}

// Places the item sprites on the bones of the pot model and draws them
void AlchemyPot::DrawSlotSprites() {
    Matrix4x3 mtx[3];
    Vector3fix pos[3];
    int sx[3];
    int sy[3];
    SlotPositions tbl;

    if (panel_.hidden) return;
    if (state_ == 0) return;
    UiObject *obj = ui_;
    func_0207fcb8(obj);
    func_0207fd44(obj);
    if (flags_ & Flag_Frozen) return;

    for (int i = 0; i < 3; i++)
        data_ov006_0216038c[i] = &mtx[i];
    G3X_ResetMtxStack();
    RenderConfig::SubmitToFifo();
    models_[4].MaybeUpdateBonePositions();
    for (int i = 0; i < 3; i++)
        data_ov006_0216038c[i] = NULL;

    pos[0].x = mtx[0].translation.x;
    pos[0].y = mtx[0].translation.y;
    pos[0].z = mtx[0].translation.z;
    pos[1].x = mtx[1].translation.x;
    pos[1].y = mtx[1].translation.y;
    pos[1].z = mtx[1].translation.z;
    pos[2].x = mtx[2].translation.x;
    pos[2].y = mtx[2].translation.y;
    pos[2].z = mtx[2].translation.z;
    sx[0]    = mtx[0].entries[0];
    sx[1]    = mtx[1].entries[0];
    sx[2]    = mtx[2].entries[0];
    sy[0]    = mtx[0].entries[4];
    sy[1]    = mtx[1].entries[4];
    sy[2]    = mtx[2].entries[4];
    for (int i = 0; i < 3; i++) {
        if (sx[i] < 0x28) sx[i] = 0x28;
        if (sy[i] < 0x28) sy[i] = 0x28;
    }

    u8 layout = numIngredientSlots_;
    tbl       = data_ov006_0215ff6c;
    void *cam = func_020100bc(GameState::GetInstance());
    if (cam != NULL) {
        ItemSpriteSlot *slot = slots_;
        for (u8 j = 0; j < 4; j++, slot++) {
            Vector3fix v = pos[tbl.v[layout][j]];
            {
                fix32_t s = models_[1].GetScale().x;
                v.x       = FIX32_MULTIPLY(v.x, s);
            }
            {
                fix32_t s = models_[1].GetScale().y;
                v.y       = FIX32_MULTIPLY(v.y, s);
            }
            {
                fix32_t s = models_[1].GetScale().z;
                v.z       = FIX32_MULTIPLY(v.z, s);
            }
            Vector3fix out;
            Vector3fix p = models_[1].position_;
            VEC_Add(&v, &p, &out);
            int x, y;
            func_0202ec84(cam, &out, &x, &y);
            slot->x = x - 0x12;
            slot->y = y - 0xc;
        }
    }

    char *base;
    ItemSpriteSlot *slot;
    u8 j;
    int anim;
    base = func_0203be40(func_0203bd08());
    anim = models_[1].normalizedAnimationTime_;
    if ((flags_ & 0x2000) || (flags_ & 0x80)) {
        if (!models_[4].HasAnimationStopped()) {
            for (j = 1, slot = &slots_[1]; j < 4; slot++, j++) {
                u8 idx              = tbl.v[layout][j];
                slot->sprite.unk_4c = 0x300;
                slot->sprite.vram   = base + (j * 4 + 0x20) * 8;
                slot->sprite.unk_44 = (j * 4 + 0x20) / 4;
                func_02076988(&slot->sprite, sx[idx], sy[idx]);
                func_02075db0(&slot->sprite, slot->x, slot->y);
            }
        }
    }

    if (flags_ & 0x200) {
        ItemSpriteSlot *slot = &slots_[0];
        data_0210a00c        = 0;
        if (anim > 0x3d7 && slot->y < 0xc0) {
            slot->sprite.unk_4c = 0x300;
            slot->sprite.vram   = base + 0x120;
            slot->sprite.unk_44 = 9;
            func_02076988(&slot->sprite, sx[0], sy[0]);
            func_02075db0(&slot->sprite, slot->x, slot->y);
        }
    }
}

void AlchemyPot::Draw3D() {
    u16 f = flags_;
    if (!(f & Flag_Visible)) {
        return;
    }
    if (f & 0x48) {
        if ((f & Flag_Frozen) || (panel_.flags & 4)) {
            if (panel_.mode != 2) {
                return;
            }
            if (!panel_.hidden) {
                return;
            }
        }
    }
    if (panel_.mode == 2 && panel_.hidden && (panel_.flags & 4)) {
        data_0210a010.viewportArg = 0x46501313;
        BG0CNT                    = (BG0CNT & ~3);
        BG1CNT                    = (BG1CNT & ~3) | 1;
        BG2CNT                    = (BG2CNT & ~3) | 2;
        if (camera_ != NULL) {
            func_020a2010(camera_);
            func_0202e5c8(camera_, 0, -0x1e8f, 0);
            func_0202e5d8(camera_, 0, 0x2bae, 0x9199);
            func_0202e0a4(camera_);
            func_020a27a0(camera_);
        }
        restoreBg0_ = 0;
    } else {
        SetupPotCamera(camera_);
    }
    models_[0].Draw(true);
    models_[1].Draw(true);
    models_[2].Draw(true);
    models_[3].Draw(true);
}

void AlchemyPot::Draw() {
    if (state_ == 0) {
        return;
    }
    if (restoreBg0_ != 0) {
        unsigned int planes = (DISPCNT & 0x1f00) >> 8;
        DISPCNT             = (DISPCNT & ~0x1f00) | ((planes | 1) << 8);
        restoreBg0_         = 0;
    }
    func_ov023_021dfad8(&panel_);
    if (panel_.hidden) {
        return;
    }
    if (ui_ == 0) {
        return;
    }
    func_0207fd88(ui_);
}

void AlchemyPot::UpdateModelAnimations() {
    GameResources *res = func_0200fb8c(GameState::GetInstance());
    if (flags_ & 0x2000) {
        SetFlags04(res, 0x2000);
        models_[4].MaybeSetRegularAnimation(data_ov006_02160080, 9);
        models_[4].EnableFlag(0x1000);
        return;
    }
    if (flags_ & 0x80) {
        if (animStep_ == 0) {
            func_0205ebc0(data_02108760, 0x7a, 0x7a);
            func_0205ebfc(data_02108760, 0, 0);
            models_[1].MaybeSetRegularAnimation(data_ov006_02160080, 1);
            models_[3].MaybeSetRegularAnimation(data_ov006_02160087, 1);
            models_[4].MaybeSetRegularAnimation(data_ov006_02160080, 9);
            models_[4].DisableFlag(0x1000);
        } else if (animStep_ >= 1) {
            if (models_[1].HasAnimationStopped()) {
                animStep_ = 0;
                flags_ &= ~0x80;
                models_[3].MaybeSetRegularAnimation(data_ov006_02160089, 1);
                models_[1].MaybeSetRegularAnimation(data_ov006_0216008b, 0);
                models_[4].StopCurrentAnimation();
                return;
            }
        }
        animStep_++;
        return;
    }
    if (flags_ & 0x100) {
        if (!models_[1].HasAnimationReachedEnd()) {
            return;
        }
        if (animStep_ == 0) {
            models_[3].MaybeSetRegularAnimation(data_ov006_02160092, 0);
        } else if (animStep_ == 1) {
            models_[3].MaybeSetRegularAnimation(data_ov006_02160094, 1);
        } else if (animStep_ == 2) {
            animStep_ = 0;
            models_[1].StopCurrentAnimation();
            models_[1].MaybeSetRegularAnimation(data_ov006_02160096, 1);
            models_[3].MaybeSetRegularAnimation(data_ov006_0216009e, 0);
            models_[4].MaybeSetRegularAnimation(data_ov006_02160096, 1);
            flags_ &= ~0x100;
            return;
        }
        animStep_++;
        return;
    }
    if (flags_ & 0x200) {
        flags_ |= 0x800;
        if (!models_[1].HasAnimationStopped()) {
            animStep_++;
            return;
        }
        ClearFlags04(res, 0x2000);
        func_0205ebec(data_02108760);
        animStep_ = 0;
        models_[1].MaybeSetRegularAnimation(data_ov006_021600a0, 1);
        models_[3].StopCurrentAnimation();
        flags_ &= ~0x200;
        return;
    }
    if (flags_ & 0x400) {
        if (models_[1].HasAnimationStopped()) {
            animStep_ = 0;
            models_[1].MaybeSetRegularAnimation(data_ov006_021600a9, 0);
            models_[3].StopCurrentAnimation();
            flags_ &= ~0x400;
            return;
        }
        models_[1].MaybeSetRegularAnimation(data_ov006_021600a0, 1);
        return;
    }
    animStep_ = 0;
    models_[1].MaybeSetRegularAnimation(data_ov006_021600a9, 0);
    models_[3].StopCurrentAnimation();
}

void AlchemyPot::Freeze() {
    flags_ |= Flag_Frozen;
}

void AlchemyPot::SetIngredientIcons(short *ids, u8 *counts) {
    ItemIcon *icons[3] = {0};
    for (u8 i = 0; i < 3; i++) {
        if (itemResources_ == 0) continue;
        void **r = func_020e5294(itemResources_, ids[i]);
        if (r != 0) {
            func_020e4864(*r, (ItemIcon *) ((u8 *) this + (i << 7)), 1, 0, 0, 0);
            icons[i] = &ingredientIcons_[i];
        }
        u8 c = counts[i];
        func_ov023_021dceac(&panel_, i, icons[i], c, c, 0);
    }
    func_ov023_021dced4(&panel_, 1);
    RefreshPanel();
}

void AlchemyPot::EnterPotView() {
    restoreBg0_               = 1;
    data_0210a010.viewportArg = 0xbfff0000;
    BG0CNT                    = (u16) ((BG0CNT & ~3) | 2);
    BG1CNT                    = (u16) (BG1CNT & ~3);
    BG2CNT                    = (u16) ((BG2CNT & ~3) | 1);
    flags_ &= ~Flag_RecipeShown;
    func_ov023_021dcd90(&panel_, 0);
    ClearRecipe();
}

void AlchemyPot::ClearRecipe() {
    if (flags_ & Flag_LoadBackground) {
        return;
    }
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (taskId_ >= 0) {
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    taskId_         = -1;
    recipeLoadStep_ = 0;
    flags_ &= ~Flag_LoadRecipe;
    if (bgTaskId_ >= 0) {
        loader->RemoveTask(bgTaskId_);
        bgTaskId_ = -1;
    }
    bgTaskId_    = -1;
    bgLoadStep_  = 0;
    shownRecipe_ = 0;
    recipe_      = 0;
    recipeEntry_ = 0;
    flags_ |= Flag_LoadBackground;
    flags_ &= ~4;
}

// Shows `recipe` unless it is already shown or being loaded
void AlchemyPot::PreviewRecipe(Recipe *recipe, RecipeEntry *entry, u8 *counts) {
    if (recipe == 0 || entry == 0) {
        if (recipeEntry_ == entry) {
            return;
        }
        ClearRecipe();
        return;
    }
    if (entry != 0 && !entry->known) {
        ClearRecipe();
        return;
    }
    if (shownRecipe_ != 0 && shownRecipe_->id == recipe->id) {
        return;
    }
    if (recipe_ != 0 && recipe_->id == recipe->id) {
        return;
    }
    SetRecipe(recipe, entry, counts);
}

void AlchemyPot::SetRecipe(Recipe *recipe, RecipeEntry *entry, u8 *counts) {
    if (recipe == 0 || entry == 0) {
        ClearRecipe();
        return;
    }
    if (entry != 0 && !entry->known) {
        ClearRecipe();
        return;
    }
    func_ov023_021dd4cc(&panel_, 1);
    itemId_                  = 0;
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (taskId_ >= 0) {
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    taskId_         = -1;
    recipeLoadStep_ = 0;
    recipe_         = recipe;
    recipeEntry_    = entry;
    ownedCounts_    = counts;
    flags_ |= Flag_LoadRecipe;
    flags_ &= ~Flag_LoadBackground;
    if (bgTaskId_ >= 0) {
        loader->RemoveTask(bgTaskId_);
        bgTaskId_ = -1;
    }
    bgTaskId_   = -1;
    bgLoadStep_ = 0;
}

// Shows a single item instead of a recipe (-1 = nothing)
void AlchemyPot::SetItem(short itemId, signed char bgIndex) {
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (taskId_ >= 0) {
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    taskId_         = -1;
    recipeLoadStep_ = 0;
    flags_ |= Flag_LoadRecipe;
    func_ov023_021dd4cc(&panel_, 2);
    itemId_      = itemId;
    itemBgIndex_ = bgIndex;
    flags_ &= ~Flag_LoadBackground;
    if (bgTaskId_ >= 0) {
        loader->RemoveTask(bgTaskId_);
        bgTaskId_ = -1;
    }
    bgTaskId_   = -1;
    bgLoadStep_ = 0;
}

// Loads the result background of the item or recipe, then the ingredient
// icons and the counts of equipped ingredients
void AlchemyPot::UpdateRecipeLoad() {
    if (!(flags_ & Flag_LoadRecipe) || panel_.ready == 0) return;

    BackgroundLoader *loader = BackgroundLoader::GetInstance();

    if (itemId_ > 0) {
        if (recipeLoadStep_ == 0) {
            short a           = itemBgIndex_ + 0x26;
            short b           = itemBgIndex_ + 0x64;
            const char *inner = func_020e0434(messages_, a);
            const char *gp2   = func_020e0434(messages_, b);
            taskId_           = loader->QueueLoadFileInGP2(gp2, inner, NULL);
            recipeLoadStep_++;
        }
        if (recipeLoadStep_ == 1) {
            if (!loader->GetTaskStatus(taskId_)) return;
            void *data;
            unsigned int size;
            loader->GetLoadedFileByID(taskId_, &data, &size);
            nextBgHeap_->Reset();
            short id = itemId_;
            func_020de848(resultBg_);
            func_020de980(resultBg_, nextBgHeap_, data, size, id);
            panel_.flags &= ~0x1000;
            panel_.resultBg = resultBg_;
            func_ov023_021dcd90(&panel_, 1);
            loader->RemoveTask(taskId_);
            unk_1dc = func_020dedd0(resultBg_, id);
            flags_ &= ~Flag_LoadRecipe;
            flags_ |= Flag_RecipeShown;
            func_ov023_021dcae0(&panel_, itemId_);
            return;
        }
    } else if (itemId_ == -1) {
        flags_ &= ~Flag_LoadRecipe;
        flags_ |= Flag_RecipeShown;
        panel_.flags &= ~0x1000;
        func_ov023_021dcd90(&panel_, 1);
        func_ov023_021dcae0(&panel_, itemId_);
        return;
    }

    if (recipeLoadStep_ == 0) {
        short b = 0x64;
        short a = 0x26;
        b += recipe_->bgIndex;
        a += recipe_->bgIndex;
        const char *inner = func_020e0434(messages_, a);
        const char *gp2   = func_020e0434(messages_, b);
        taskId_           = loader->QueueLoadFileInGP2(gp2, inner, NULL);
        recipeLoadStep_++;
    } else if (recipeLoadStep_ == 1) {
        if (!loader->GetTaskStatus(taskId_)) return;
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            nextBgHeap_->Reset();
            func_020de848(resultBg_);
            func_020de980(resultBg_, nextBgHeap_, data, size, recipe_->resultItem);
            unk_1dc = func_020dedd0(resultBg_, recipe_->resultItem);
        }
        loader->RemoveTask(taskId_);
        taskId_         = -1;
        recipeLoadStep_ = 3;
    } else if (recipeLoadStep_ == 3) {
        shownRecipe_       = recipe_;
        SafeAllocator *tmp = bgHeap_;
        bgHeap_            = nextBgHeap_;
        nextBgHeap_        = tmp;
        panel_.resultBg    = resultBg_;
        func_ov023_021dcd90(&panel_, 1);

        ItemIcon *icons[3] = {0};
        if (itemResources_ != NULL) {
            memset(this, 0, 0x180);
            void **r = func_020e5294(itemResources_, recipe_->ingredients[0]);
            if (r != NULL) {
                func_020e4864(*r, &ingredientIcons_[0], 1, 0, 0, 0);
                icons[0] = &ingredientIcons_[0];
            }
            r = func_020e5294(itemResources_, recipe_->ingredients[1]);
            if (r != NULL) {
                func_020e4864(*r, &ingredientIcons_[1], 1, 0, 0, 0);
                icons[1] = &ingredientIcons_[1];
            }
            r = func_020e5294(itemResources_, recipe_->ingredients[2]);
            if (r != NULL) {
                func_020e4864(*r, &ingredientIcons_[2], 1, 0, 0, 0);
                icons[2] = &ingredientIcons_[2];
            }
        }

        // ingredients currently equipped by the party
        u8 owned[3]         = {0};
        GameState *gs       = GameState::GetInstance();
        PartyInventory *inv = func_02010828(gs);
        for (int i = 0; i < inv->partyCount; i++) {
            PartyMember *m = func_0200ff1c(gs, inv->party[i]);
            if (m == NULL) continue;
            for (int j = 0; j < 8; j++) {
                EquipSlot *e = &((EquipSlot *) (m->data + 0x194))[data_ov006_0215ff5c[j]];
                if (e == NULL) continue;
                bool valid = e->kind <= 7 ? 1 : 0;
                if (!valid) continue;
                if (recipe_->ingredients[0] == e->id) owned[0]++;
                if (recipe_->ingredients[1] == e->id) owned[1]++;
                if (recipe_->ingredients[2] == e->id) owned[2]++;
            }
        }

        func_ov023_021dceac(&panel_, 0, icons[0], recipe_->quantity0, ownedCounts_[0], owned[0]);
        func_ov023_021dceac(&panel_, 1, icons[1], recipe_->quantity1, ownedCounts_[1], owned[1]);
        func_ov023_021dceac(&panel_, 2, icons[2], recipe_->quantity2, ownedCounts_[2], owned[2]);
        func_ov023_021dced4(&panel_, 1);

        bool on = false;
        if (recipeEntry_ != NULL && recipeEntry_->crafted) on = true;
        if (on)
            panel_.flags &= ~0x1000;
        else
            panel_.flags |= 0x1000;
        func_ov023_021dcae0(&panel_, recipe_->resultItem);
        unk_1d8         = unk_1dc;
        unk_1dc         = 0;
        recipeLoadStep_ = 0;
        flags_ &= ~Flag_LoadRecipe;
        flags_ |= 0x44;
    }
}

// State 0: loads the sprites, backgrounds, UI, layout and the 3D models
void AlchemyPot::UpdateLoad() {
    SafeAllocator *heaps;
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    SafeAllocator *alloc2;
    SafeAllocator *alloc;
    GameResources *res;

    if (loadStep_ == 0) {
        func_0205a444(&spriteBank_);
        spriteBank_.unk_50     = 0;
        spriteBank_.sprites    = sprites_;
        spriteBank_.numSprites = 0x11;
        taskId_                = loader->QueueLoadFile(data_ov006_021600af, NULL);
        loadStep_++;
    } else if (loadStep_ == 1) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        int a;
        void *data;
        unsigned int size;
        int b;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        int n                = func_02046900(data);
        SafeAllocator *heaps = heaps_;
        heaps->Reset();
        for (int i = 0; i < n; i++) {
            void *r = func_020467f0(data, i, &a, &b);
            if (r) {
                func_0205a528(&spriteBank_, r, b, heaps);
            }
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
    } else if (loadStep_ == 2) {
        BG1CNT = (BG1CNT & 0x43) | 0x1d00;
        BG2CNT = (BG2CNT & 0x43) | 0x1e00;
        BG3CNT = (BG3CNT & 0x43) | 0x1f08;
        heaps  = heaps_;
        heaps[2].Reset();
        BgPalettes tbl = data_ov006_0215ff54;
        for (u8 i = 0; i < 2; i++) {
            BgLayer *l = &layers_[i];
            func_0204af64(l);
            l->unk_1c_0 = 0;
            l->index    = i + 1;
            func_0204b11c(l, 0);
            func_0204b5b4(l, tbl.v[i]);
            func_0204b12c(l, &heaps[2]);
            func_0204b5e8(l, 0, 0);
        }
        func_0204b010(&layers_[0], 0);
        func_0204b010(&layers_[1], 0);
        SafeAllocator *heaps2 = heaps_;
        heaps2[3].Reset();
        for (u8 k = 0; k < 1; k++) {
            TitleBg *u = &titleBg_[k];
            func_0204c684(u);
            func_0204c7a8(u, &heaps2[3], bgBuffer_, 0x1c0);
            u->layers = layers_;
        }
        taskId_ = loader->QueueLoadFile(data_ov006_021600c4, NULL);
        loadStep_++;
    } else if (loadStep_ == 3) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        int a;
        void *data;
        unsigned int size;
        int b;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        int n = func_02046900(data);
        heaps = heaps_;
        for (int i = 0; i < n; i++) {
            void *r = func_020467f0(data, i, &a, &b);
            if (r) {
                func_0204b174(layers_, r, &heaps[2], b);
            }
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
        flags_ |= Flag_LoadBackground;
    } else if (loadStep_ == 4) {
        BG0CNT               = (BG0CNT & ~3) | 2;
        BG1CNT               = (BG1CNT & ~3);
        BG2CNT               = (BG2CNT & ~3) | 1;
        BG3CNT               = (BG3CNT & ~3) | 3;
        SafeAllocator *heaps = heaps_;
        heaps[4].Reset();
        func_0207f84c(ui_);
        func_0207f914(ui_, &heaps[4], data_ov006_021600d7, data_ov006_021600f0);
        loadStep_++;
    } else if (loadStep_ == 5) {
        int r = func_0207f9f4(ui_);
        if (r == 0) {
            loadStep_++;
        }
        if (r < 0) {
            state_ = 2;
        }
    } else if (loadStep_ == 6) {
        taskId_ = loader->QueueLoadFile(data_ov006_021600f7, NULL);
        loadStep_++;
    } else if (loadStep_ == 7) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            SafeAllocator *heaps = heaps_;
            heaps[1].Reset();
            func_ov023_021e20c0(layout_);
            func_ov023_021e20f0(layout_, &heaps[1], data, size);
            // FindLayoutElement(layout_, 1), inlined
            ScreenLayout *l = layout_;
            ScreenLayout::Element *e;
            ScreenLayout::Element *elems = l->elements_;
            if (elems == NULL) {
                e = NULL;
            } else {
                u16 count = l->numElements_;
                if (count == 0) {
                    e = NULL;
                } else {
                    for (u16 i = 0; i < count; i++) {
                        e = &elems[i];
                        if (e->id == 1) goto found;
                    }
                    e = NULL;
                }
            }
        found:
            if (e != NULL) e->visible = 0;
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
        if (flags_ & Flag_Frozen) {
            loadStep_ = 0x12;
        }
    } else if (loadStep_ == 8) {
        int v           = (signed char) func_02012fe4()->GetSub()->unk_b3c;
        char name[0x20] = {0};
        if (v < 1) v = 1;
        if (v > 6) v = 6;
        PotLetters letters = data_ov006_0215ff64;
        sprintf(name, data_ov006_0216010b, letters.c[v]);
        taskId_ = loader->QueueLoadFile(name, NULL);
        loadStep_++;
    } else if (loadStep_ == 9) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            res   = func_0200fb8c(GameState::GetInstance());
            alloc = &res->allocator_array_38[0];
            alloc->Reset();
            func_0207df50(res->unk_2cc);
            func_0207df90(res->unk_2cc);
            ObjectArchiveLoadInfo info;
            info.fileData  = data;
            info.unk_8     = size;
            info.allocator = alloc;
            info.unk_10    = 1;
            models_[0].LoadFromCHRArchive(&info);
            models_[0].MaybeSetBCFGAnimation(0, 0);
            func_0207dfac(res->unk_2cc);
            func_020407b4(&models_[0], -0x3800, -0xb3b, 0x60a3);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
    } else if (loadStep_ == 10) {
        taskId_ = loader->QueueLoadFile(data_ov006_02160125, NULL);
        loadStep_++;
    } else if (loadStep_ == 11) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            res   = func_0200fb8c(GameState::GetInstance());
            alloc = &res->allocator_array_38[0];
            func_0207df90(res->unk_2cc);
            ObjectArchiveLoadInfo info;
            info.fileData  = data;
            info.unk_8     = size;
            info.allocator = alloc;
            info.unk_10    = 1;
            models_[1].LoadFromCCHROrCMOTArchive(&info, NULL);
            models_[1].MaybeSetBCFGAnimation(0, 0);
            func_0207dfac(res->unk_2cc);
            models_[1].SetScale(0x10a, 0x10a, 0x10a);
            func_020407b4(&models_[1], -0x41, -0xaac, 0x61f3);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
    } else if (loadStep_ == 12) {
        taskId_ = loader->QueueLoadFile(data_ov006_0216013d, NULL);
        loadStep_++;
    } else if (loadStep_ == 13) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            res   = func_0200fb8c(GameState::GetInstance());
            alloc = &res->allocator_array_38[0];
            func_0207df90(res->unk_2cc);
            ObjectArchiveLoadInfo info;
            info.fileData  = data;
            info.unk_8     = size;
            info.allocator = alloc;
            info.unk_10    = 1;
            models_[2].LoadFromCCHROrCMOTArchive(&info, NULL);
            models_[2].MaybeSetBCFGAnimation(0, 0);
            func_0207dfac(res->unk_2cc);
            models_[2].SetScale(0x10a, 0x10a, 0x10a);
            func_02059f38(&models_[2], &models_[1].position_);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
    } else if (loadStep_ == 14) {
        taskId_ = loader->QueueLoadFile(data_ov006_02160156, NULL);
        loadStep_++;
    } else if (loadStep_ == 15) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        res = GetGameResources();
        heap_.CreateTypeA(res->allocator_array_38[0].Allocate(0x8000), 0x8000);
        heap_.Reset();
        func_0207df90(res->unk_2cc);
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL && heap_.GetSignedAllocator() != NULL) {
            models_[3].Initialize();
            ObjectArchiveLoadInfo info;
            info.fileData  = data;
            info.unk_8     = size;
            alloc2         = &heap_;
            info.allocator = alloc2;
            info.unk_10    = 1;
            models_[3].LoadFromCHRArchive(&info);
            models_[3].SetScale(&models_[1].GetScale());
            func_02059f38(&models_[3], &models_[1].position_);
            models_[3].StopCurrentAnimation();
        }
        func_0207dfac(res->unk_2cc);
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
    } else if (loadStep_ == 16) {
        taskId_ = loader->QueueLoadFile(data_ov006_02160172, NULL);
        loadStep_++;
    } else if (loadStep_ == 17) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data != NULL) {
            res    = func_0200fb8c(GameState::GetInstance());
            alloc2 = &res->allocator_array_38[0];
            func_0207df90(res->unk_2cc);
            ObjectArchiveLoadInfo info;
            Object3D *obj  = &models_[4];
            info.fileData  = data;
            info.unk_8     = size;
            info.allocator = alloc2;
            info.unk_10    = 1;
            obj->LoadFromCCHROrCMOTArchive(&info, NULL);
            obj->MaybeSetBCFGAnimation(0, 0);
            func_0207dfac(res->unk_2cc);
            obj->SetScale(0x10a, 0x10a, 0x10a);
            func_02059f38(obj, &models_[1].position_);
            obj->StopCurrentAnimation();
            obj->MaybeSetRegularAnimation(data_ov006_02160080, 9);
            obj->DisableFlag(0x1000);
            for (int i = 0; i < 3; i++) {
                StaticInit::data_ov006_02160380[i] = -1;
            }
            // hook the 3 ingredient bones (CaptureIngredientBones)
            Model3D *m = models_[4].pModel_;
            if (m != NULL) {
                data_ov006_02160380[0]  = m->GetBoneIndex(data_ov006_0216018b);
                data_ov006_02160380[1]  = m->GetBoneIndex(data_ov006_02160190);
                data_ov006_02160380[2]  = m->GetBoneIndex(data_ov006_02160195);
                ModelRenderContext *ctx = m->unknown_flags_a8_0_ ? &m->renderContext_ : NULL;
                if (ctx != NULL) {
                    SetModelRenderContextRenderCommandHook(ctx, CaptureIngredientBones, 0, 6, 3);
                }
            }
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        loadStep_++;
    } else if (loadStep_ == 18) {
        models_[1].StopCurrentAnimation();
        models_[2].StopCurrentAnimation();
        models_[1].MaybeSetBCFGAnimation(0, 0);
        models_[2].MaybeSetBCFGAnimation(0, 0);
        GameState *gs = GameState::GetInstance();
        savedScene_   = func_020100f8(gs);
        SetupPotCamera(camera_);
        func_020100c4(gs, camera_);
        UiObject *ui = ui_;
        ui->layers   = layers_;
        ui->unk_38   = 2;
        func_0207f7f0(ui_, titleBg_, 1);
        ui_->unk_3a = 1;
        DISPCNT     = (DISPCNT & ~0x1f00) | 0x1f00;
        ColorEffect_ConfigureAlphaBlend(0x4000050, 4, 1, 10, 6);
        flags_ |= Flag_Visible;
        flags_ &= ~Flag_RecipeShown;
        func_020dbf18(&anim_, data_ov006_0216019a, &heaps_[5], func_0200fb8c(gs)->unk_2cc, 0x18);
        loadStep_ = loadStep_ + 1;
    } else if (loadStep_ == 19) {
        if (anim_.state == 2) {
            state_    = 1;
            loadStep_ = 0;
        }
    }
}

// State 1
void AlchemyPot::UpdateReady() {
    if (!(flags_ & Flag_Frozen)) {
        models_[0].AdvanceEffects();
        models_[1].AdvanceEffects();
        models_[2].AdvanceEffects();
        models_[3].SetNormalizedAnimationTime(models_[1].normalizedAnimationTime_);
    }
    UpdateRecipeLoad();
    func_ov023_021dc488(&panel_);
}

void AlchemyPot::UpdateBackgroundLoad() {
    if (!(flags_ & Flag_LoadBackground)) return;
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (bgLoadStep_ == 0) {
        if (flags_ & Flag_Frozen)
            bgTaskId_ = loader->QueueLoadFile(data_ov006_021601b6, NULL);
        else
            bgTaskId_ = loader->QueueLoadFile(data_ov006_021601cc, NULL);
        func_ov023_021dcd90(&panel_, 0);
        bgLoadStep_++;
    } else if (bgLoadStep_ == 1) {
        if (loader->GetTaskStatus(bgTaskId_) == 0) return;
        BgLayer layer;
        int unk;
        void *data;
        unsigned int size;
        unsigned int sz;
        loader->GetLoadedFileByID(bgTaskId_, &data, &size);
        int count = func_02046900(data);
        func_0204af64(&layer);
        func_0204b11c(&layer, 0);
        layer.unk_1c_0 = 0;
        layer.index    = 3;
        func_0204b5b4(&layer, 3);
        func_0204b5e8(&layer, 0, 0);
        for (int i = 0; i < count; i++) {
            void *r = func_020467f0(data, i, &unk, &sz);
            if (r) {
                func_0204b2e0(&layer, r);
                func_0204b3a0(&layer, r);
            }
        }
        loader->RemoveTask(bgTaskId_);
        bgTaskId_   = -1;
        bgLoadStep_ = 0;
        flags_ &= ~Flag_LoadBackground;
        flags_ &= ~4;
    }
}

void AlchemyPot::UpdateEffectAnimation() {
    func_020dbdc0(&anim_);
    if (anim_.busy == 0 && animState_ != 0) {
        AnimParams p;
        func_02078484(&p);
        p.flags &= ~4;
        p.unk_12 = 0;
        if (animState_ == 1 || animState_ == 2) {
            strcpy(p.name, data_ov006_02160089);
            animState_ = 2;
        } else {
            strcpy(p.name, data_ov006_02160092);
            animState_ = 0;
        }
        p.scaleX = 0x10a;
        p.scaleY = 0x10a;
        p.scaleZ = 0x10a;
        p.x      = -0x41;
        p.y      = -0xaac;
        p.z      = 0x4526;
        func_020dbfa4(&anim_, &p);
    }
}

void AlchemyPot::ResetSlots() {
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    ItemSpriteSlot *slot     = slots_;
    for (u8 i = 0; i < 4; i++) {
        if (slot->taskId > 0) {
            loader->RemoveTask(slot->taskId);
        }
        slot->taskId = -1;
        slot->Reset();
        slot->sprite.unk_5e = 0;
        slot->sprite.vram   = func_0203be40(func_0203bd08()) + i * 0x40 + 0x100;
        slot->sprite.unk_38 = i * 0x120 + 0x240;
        slot->sprite.unk_3c = (i + 10) & 0xf;
        slot->sprite.unk_40 = 0;
        slot->x             = i * 0x20 + 0x20;
        slot->y             = 0x80;
        slot++;
    }
}

// Starts loading "d_%c%03d.spr" for the result and the 3 ingredients
void AlchemyPot::LoadSlotSprites(ItemData *items) {
    ResetSlots();
    ItemSpriteSlot *slot     = slots_;
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    numIngredientSlots_      = 0;
    for (u8 i = 0; i < 4; items++, slot++, i++) {
        if ((slot->itemId = items->id) > 0) {
            slot->loading   = 1;
            int id          = func_020de234(items, 0);
            char path[0x40] = {0};
            sprintf(path, data_ov006_021601e5, items->spriteGroup, id);
            slot->taskId = loader->QueueLoadFile(path, NULL);
            if (i != 0) {
                numIngredientSlots_++;
            }
        }
    }
}

void AlchemyPot::UpdateSlotLoads() {
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    ItemSpriteSlot *slot     = slots_;
    for (u8 i = 0; i < 4; slot++, i++) {
        if (slot->loading) {
            if (loader->GetTaskStatus(slot->taskId) != 0) {
                void *data;
                unsigned int size;
                loader->GetLoadedFileByID(slot->taskId, &data, &size);
                if (data != NULL) {
                    slotHeaps_[i].Reset();
                    func_02076080(&slot->sprite, &slotHeaps_[i], data, size);
                }
                loader->RemoveTask(slot->taskId);
                slot->taskId  = -1;
                slot->loading = 0;
            }
            slot->x = 0x6f;
            slot->y = -26;
        }
    }
}

bool AlchemyPot::IsLoadingSlots() {
    bool any             = false;
    ItemSpriteSlot *slot = slots_;
    for (u8 i = 0; i < 4; slot++, i++) {
        any = any | slot->loading;
    }
    return any;
}

// Returns the previous visibility
bool AlchemyPot::SetPanelVisible(bool visible) {
    bool was = (panel_.flags & 0x800) != 0;
    if (visible) {
        panel_.flags |= 0x800;
    } else {
        panel_.flags &= ~0x800;
    }
    if (!was && visible) {
        func_ov023_021dd404(&panel_);
        func_ov023_021dd344(&panel_);
    }
    if (was && !visible) {
        func_ov023_021dcd78(&panel_);
    }
    return was;
}

void AlchemyPot::HidePanel() {
    flags_ &= ~Flag_RecipeShown;
    func_ov023_021dcd90(&panel_, 0);
    ClearRecipe();
}

void AlchemyPot::SetPanelQuantity(u8 quantity) {
    func_ov023_021dced4(&panel_, quantity);
}

void AlchemyPot::RefreshPanel() {
    if (IsPanelVisible()) {
        func_ov023_021dd344(&panel_);
    }
}

void AlchemyPot::StartEffectAnimation() {
    AnimParams p;
    func_02078484(&p);
    p.flags &= ~4;
    p.unk_12 = 0;
    strcpy(p.name, data_ov006_02160087);
    p.scaleX = 0x10a;
    p.scaleY = 0x10a;
    p.scaleZ = 0x10a;
    p.x      = -0x41;
    p.y      = -0xaac;
    p.z      = 0x4526;
    func_020dbfa4(&anim_, &p);
    animState_ = 1;
}

void AlchemyPot::StopEffectAnimation() {
    animState_ = 3;
}

void SetupBookListButtons(ScreenLayout *layout) {
    ScreenLayout::Element *e;
    e = FindLayoutElement(layout, 0x50);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0x4f);
    if (e != 0) {
        e->flags |= 1;
    }
    e = FindLayoutElement(layout, 0xe);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0xf);
    if (e != 0) {
        e->flags |= 1;
    }
    e = FindLayoutElement(layout, 0x15);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0x41);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0x42);
    if (e != 0) {
        e->flags &= ~1;
    }
}

ScreenLayout::Element *FindLayoutElement(ScreenLayout *layout, int id) {
    ScreenLayout::Element *elements = layout->elements_;
    if (elements == 0) {
        return 0;
    }
    u16 count = layout->numElements_;
    if (count == 0) {
        return 0;
    }
    for (u16 i = 0; i < count; i++) {
        ScreenLayout::Element *e = &elements[i];
        if (e->id == id) {
            return e;
        }
    }
    return 0;
}

void SetupIngredientListButtons(ScreenLayout *layout) {
    ScreenLayout::Element *e;
    e = FindLayoutElement(layout, 0x50);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0x4f);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0xe);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0xf);
    if (e != 0) {
        e->flags &= ~1;
    }
    e = FindLayoutElement(layout, 0x15);
    if (e != 0) {
        e->flags |= 1;
    }
}

// Filter buttons of the recipe book: 0x41..0x45 select an item type,
// 0x48..0x53 and 0x40 a subtype
void DecodeFilterButton(int buttonId, signed char *type, signed char *subtype) {
    *subtype = -1;
    switch (buttonId) {
        case 0x48: *subtype = 0; break;
        case 0x49: *subtype = 1; break;
        case 0x4a: *subtype = 2; break;
        case 0x4b: *subtype = 3; break;
        case 0x4c: *subtype = 4; break;
        case 0x4d: *subtype = 5; break;
        case 0x4e: *subtype = 6; break;
        case 0x4f: *subtype = 7; break;
        case 0x50: *subtype = 8; break;
        case 0x51: *subtype = 9; break;
        case 0x52: *subtype = 10; break;
        case 0x53: *subtype = 11; break;
        case 0x40: *subtype = 12; break;
        case 0x41: *type = 4; break;
        case 0x42: *type = 2; break;
        case 0x43: *type = 5; break;
        case 0x44: *type = 3; break;
        case 0x45: *type = 6; break;
    }
}

void CraftItem(ItemData *result, Recipe *recipe, u8 count) {
    void *inv = func_02010828(GameState::GetInstance());
    CraftTemp tmp;
    SafeAllocator alloc;
    alloc.ResetAllocatorPointer();
    func_020de824(&tmp);
    func_0207cbe8(&alloc);
    func_0207cbe8(&alloc);
    func_0207d300(&alloc, result->unk_18, (signed char) count, 0);
    func_0207cc0c(&alloc);
    for (u8 n = 0; n < count; n++) {
        for (u8 i = 0; i < recipe->quantity0; i++)
            func_02086d88(inv, recipe->ingredients[0]);
        for (u8 j = 0; j < recipe->quantity1; j++)
            func_02086d88(inv, recipe->ingredients[1]);
        for (u8 k = 0; k < recipe->quantity2; k++)
            func_02086d88(inv, recipe->ingredients[2]);
    }
}
