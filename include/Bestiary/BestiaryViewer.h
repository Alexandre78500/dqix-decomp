#pragma once

#include "../Memory/SafeAllocator.h"
#include "PackedTable.h"
#include "../World/Object3D.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Screen layout (lay_sml.lia), loaded and queried by overlay 23 code
// (func_ov023_021e20c0 / 021e20f0 / 021e2bdc). sizeof == 0x4c.
class ScreenLayout
{
public:
    // sizeof == 0x18
    struct Element
    {
        short id;
        u8 unk_2[0x14];
        u8 visible : 1; // 0x16
        u8 unk_16_1 : 7;
        u8 unk_17;
    };

    u8 unk_0[0x8];
    Element* elements_;     // 0x8
    u8 unk_c[0x8];
    u16 numElements_;       // 0x14
    u8 unk_16[0x4c - 0x16];

    // Also used by overlays 3, 17 and 21
    Element* FindElement(int id);
};

// One monster of the list loaded from mon_list.gp2 (see BestiaryScreen).
// The list is chained through `next` once built. sizeof == 0x20.
struct MonsterEntry
{
    MonsterEntry* next;     // 0x0
    char* fileName;         // 0x4: "<fileName>.mon" in data/pack_lv5/enemy.gp2
    char* name;             // 0x8: displayed name
    short modelId;          // 0xc
    u8 unk_e[0x4];
    signed char unk_12;
    union
    {
        u8 flags;           // 0x13
        struct
        {
            u8 unk_13_0 : 1;
            u8 seen : 1;
            u8 unk_13_2 : 6;
        };
    };
    u8 unk_14[0x6];
    short number;           // 0x1a: bestiary number, starting at 1
    u8 unk_1c[0x4];
};

// Per-monster record word (one per bestiary number, see BestiaryScreen::records_)
struct MonsterRecord
{
    u32 defeatCount : 10;
    // gates the second info page and its sprite
    u32 unk_10 : 1;
    u32 unk_11 : 7;
    u32 unk_18 : 7;
    u32 unk_25 : 7;
};

// Entry returned by func_02097224 (monster data from mons_info2.nat)
struct MonsterModelInfo
{
    u8 unk_0[0x8];
    short nameTextId;       // 0x8
    short infoTextId;       // 0xa
    u8 animCount;           // 0xc
    u8 unk_d[0x3];
    float position[3];      // 0x10
    float rotationY;        // 0x1c
    float scale[3];         // 0x20
    char** animNames;       // 0x2c
};

// Best model variant found while scanning mons_info2.nat. sizeof == 0x8.
struct MonsterVariant
{
    u32 unk_0;
    short modelId;
    u16 unk_6;

    void Reset();
};

// 2D sprite of a SpriteBank. sizeof == 0x28.
struct Sprite
{
    u8 unk_0[0x14];
    int x;                  // 0x14, fixed point
    int y;                  // 0x18, fixed point
    u8 unk_1c[0x6];
    u8 unk_22;
    u8 unk_23[0x3];
    u8 unk_26;
    u8 unk_27;
};

// Set of sprites drawn together (init func_0205a444, draw func_0205ac40).
// sizeof == 0x54.
struct SpriteBank
{
    u8 unk_0[0x3c];
    void* unk_3c;
    Sprite* sprites;        // 0x40
    u8 unk_44[0x8];
    u16 numSprites;         // 0x4c
    u8 unk_4e[0x2];
    u8 unk_50;
    u8 unk_51[0x3];
};

// Message archive (init func_020dfc40, string lookup func_020e0434). sizeof == 0x18.
struct TextArchive
{
    u8 unk_0[0x18];
};

// Top screen of the bestiary: 3D view of the selected monster, its texts
// and sprites. BestiaryScreen extends it.
class BestiaryViewer
{
public:
    typedef void (BestiaryViewer::*Handler)();

    void** buffers_;                // 0x0: 3 buffers, sizes in data_ov014_02189480
    SpriteBank* spriteBank_;        // 0x4
    Sprite* sprites_;               // 0x8: 3 sprites
    u8 monsterData_[0x10];          // 0xc: mons_info2.nat (func_02096fb0)
    PackedTableLoader tableLoader_; // 0x1c: enchab table, entries of the monster
    SafeAllocator* heaps_;          // 0x3c: 6 sub-heaps
    Object3D* model_;               // 0x40
    void* scene_;                   // 0x44
    MonsterEntry* monster_;         // 0x48
    MonsterEntry* prevMonster_;     // 0x4c
    MonsterRecord* record_;         // 0x50
    MonsterRecord* prevRecord_;     // 0x54
    ScreenLayout* layout_;          // 0x58
    TextArchive* texts_;            // 0x5c
    u8* nameText_;                  // 0x60
    u8* infoText_;                  // 0x64
    u8* pageText_;                  // 0x68: mon_trv%d.gp2
    int layoutTaskId_;              // 0x6c
    int topBgTaskId_;               // 0x70
    int animIndex_;                 // 0x74
    int unk_78;
    // index in the handler table: 0 = loading resources, 1 = ready
    u8 mode_;                       // 0x7c
    u8 loadStep_;                   // 0x7d
    u8 topBgLoadStep_;              // 0x7e
    u8 monsterLoadStep_;            // 0x7f
    u8 page_;                       // 0x80
    union
    {
        u8 flags81_;
        struct
        {
            u8 loadingMonster_ : 1;
            u8 flags81_1_ : 1;
            u8 modelReady_ : 1;
            u8 pageToggled_ : 1;
            u8 showTopBg_ : 1;
            u8 flags81_5_ : 3;
        };
    };
    union
    {
        u8 flags82_;
        struct
        {
            u8 autoRotating_ : 1;
            u8 flags82_1_ : 1;
            u8 loadModel_ : 1;
            u8 flags82_3_ : 1;
            u8 flags82_4_ : 4;
        };
    };
    u8 unk_83;
    short unk_84;
    short tasks_[5];                // 0x86
    union
    {
        u8 spriteOffset_[3];        // 0x90
        struct
        {
            u8 rotatingLeft_;
            u8 rotatingRight_;
            u8 unk_92;
        };
    };
    u8 nextShowSprites_;            // 0x93
    u8 showSprites_;                // 0x94
    u8 unk_95[0x3];
    short* modelIds_;               // 0x98: 0-terminated
    MonsterVariant variant_;        // 0x9c

    void Allocate(SafeAllocator* alloc);
    void Initialize();
    void Shutdown();
    void Update();
    void Draw();
    void ApplyPlacement();
    void CancelMonsterLoad();
    void ToggleInfoPage();
    void CycleAnimation();
    void ChangeMonster(MonsterEntry* monster, MonsterRecord* record);
    void SetMonster(MonsterEntry* monster, MonsterRecord* record, u8 loadModel);
    void UpdateMonsterLoad();
    void LoadResources();
    void UpdateReady();
    void UpdateTopBackgroundLoad();
};
