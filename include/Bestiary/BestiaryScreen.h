#pragma once

#include "BestiaryViewer.h"

// Touch-screen menu object (bm_sml.gp2), driven by the func_0207f... /
// func_02080... / func_02081... functions. sizeof == 0x40.
struct UiObject
{
    u8 unk_0[0x2c];
    void* layers;           // 0x2c
    u8 unk_30[0x6];
    short cursor;           // 0x36
    u8 unk_38;
    u8 unk_39;
    u8 unk_3a;
    u8 unk_3b[0x5];
};

// Element of a UiObject, as returned by func_02081da8. Sizes in tiles.
struct UiElement
{
    u8 unk_0[0xa8];
    short width;            // 0xa8
    short height;           // 0xaa
    short x;                // 0xac
    short y;                // 0xae
};

// Background layer (init func_0204af64). sizeof == 0x20.
struct BgLayer
{
    u8 unk_0[0x1c];
    u8 unk_1c_0 : 4;
    u8 index : 4;
    u8 unk_1d[0x3];
};

// Title background object (init func_0204c684). sizeof == 0xe0.
struct TitleBg
{
    u8 unk_0[0x4];
    BgLayer* layers;        // 0x4
    u8 unk_8[0xe0 - 0x8];
};

// Frame of the cursor animation (func_0205a3d0)
struct CursorFrame
{
    u8 unk_0[0x4];
    short x;                // 0x4
    short y;                // 0x6
    u8 unk_8[0xd];
    u8 flags;               // 0x15
};

// Cursor animation (init func_0205a234). sizeof == 0x8.
struct CursorAnim
{
    u8 unk_0[0x8];
};

// Bottom screen of the bestiary: monster list, menus and the 3D view of the
// selected monster (BestiaryViewer) on the top screen.
class BestiaryScreen : public BestiaryViewer
{
public:
    typedef void (BestiaryScreen::*StateHandler)();

    enum State
    {
        State_Load = 0,
        State_FadeIn = 1,
        State_MainMenu = 2,
        State_CategoryMenu = 3,
        State_List = 4,
        State_FadeOut = 5,
        State_Exit = 6,
    };

    u8 monsterList_[0x10];          // 0xa4: mon_list.gp2 (func_020972b0)
    SafeAllocator* screenHeaps_;    // 0xb4: 5 sub-heaps
    MonsterEntry* list_;            // 0xb8: listed monsters
    MonsterEntry* pageStart_;       // 0xbc: first monster of the displayed page
    UiObject* ui_;                  // 0xc0
    BgLayer* titleLayers_;          // 0xc4: 2 layers
    TitleBg* titleBgs_;             // 0xc8: 4 objects
    void* titleBuffer_;             // 0xcc
    short* selection_;              // 0xd0: cursor of the active element
    Sprite* cursorSprites_;         // 0xd4: 8 sprites
    CursorAnim* cursorAnim_;        // 0xd8
    MonsterRecord* records_;        // 0xdc: indexed by bestiary number - 1
    TextArchive messages_;          // 0xe0: str_sml.gp2
    SpriteBank cursorBank_;         // 0xf8
    u8 unk_14c[0xc];
    u16 unk_158;
    u8 unk_15a;
    u8 unk_15b;
    int unk_15c;
    int scrollKeys_;                // 0x160
    int frameTime_;                 // 0x164
    int taskId_;                    // 0x168
    int titleTaskId_;               // 0x16c
    union
    {
        short elements_[5];         // 0x170
        struct
        {
            short activeElement_;   // 0x170: UI element of the cursor
            short menuCursor_;      // 0x172
            short categoryCursor_;  // 0x174
            short listCursor_;      // 0x176
            short prevCursor_;      // 0x178
        };
    };
    u8 state_;                      // 0x17a
    u8 subStep_;                    // 0x17b
    u8 titleLoadStep_;              // 0x17c
    signed char category_;          // 0x17d: -1 = all monsters
    // 0x08: sort order, 0x10: title background to load, 0x40: reload on confirm
    u8 flags_;                      // 0x17e
    u8 exitRequested_;              // 0x17f
    short totalMonsters_;           // 0x180
    short modelIdBuffer_[8];        // 0x182

    void Allocate(SafeAllocator* alloc);
    void Initialize();
    void Shutdown();
    int Update(int frameTime);
    void DrawModel();
    void DrawMenu();
    void DrawInfo();
    void RequestExit();
    void DrawCursor();
    u8 IsConfirmPressed();
    int IsBackPressed();
    int UpdateSortToggle();
    void ResetPageTo(MonsterEntry* monster);
    void ScrollPage(int delta);
    MonsterEntry* GetPageStart(int page);
    MonsterEntry* GetSelectedMonster();
    MonsterRecord* GetRecord(MonsterEntry* monster);
    void MarkRecordedMonsters();
    short CountListedMonsters();
    short GetCurrentPage();
    void UpdateLoad();
    void UpdateFadeIn();
    void UpdateMainMenu();
    void UpdateCategoryMenu();
    void UpdateList();
    void UpdateFadeOut();
    void RefreshHeader();
    void RefreshCategoryTitle();
    void RefreshSortButton();
    void RefreshListPage();
    void UpdateTitleBackgroundLoad();
    void SetModelIdsFor(int number);
};
