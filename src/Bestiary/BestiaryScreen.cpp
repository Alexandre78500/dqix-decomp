#include "Bestiary/BestiaryScreen.h"
#include "Filesystem/BackgroundLoader.h"
#include "GameState/GameState.h"
#include "Input/Input.h"
#include "Resource/Brightness.h"
#include "Resource/GameResources.h"
#include "System/ColorEffects.h"
#include "std_library_functions.h"

struct TouchState {
    u8 unk_0[0x55];
    u8 touched;
};

// UI elements hidden by RefreshHeader, per mode, each list ending with -1
struct HiddenElements {
    short ids[11];
};

extern "C" {
    // Top screen texts of the viewer
void func_ov014_02185c90(BestiaryViewer *viewer);
int func_ov017_021959b4();

    // monster list (mon_list.gp2)
void func_020972b0(void *list);
void func_020972c8(void *list);
void func_020972e0(void *list, SafeAllocator *alloc, void *file, unsigned int size);
MonsterEntry *func_02097418(void *list);
int func_02097420(void *list);
MonsterEntry *func_020974b0(void *list, int category, int a, int sortOrder, int b, short *outCount);
void func_020abf60(void *list);
void func_020abf38(MonsterRecord *records);

    // UiObject
void func_0207f7f0(UiObject *ui, TitleBg *bgs, int count);
void func_0207f84c(UiObject *ui);
void func_0207f914(UiObject *ui, SafeAllocator *alloc, const char *gp2, const char *name);
int func_0207f9f4(UiObject *ui);
void func_0207fc6c(UiObject *ui, int frameTime);
void func_0207fcb8(UiObject *ui);
void func_0207fd00(UiObject *ui);
void func_0207fd44(UiObject *ui);
void func_0207fd88(UiObject *ui);
void func_0207fdcc(UiObject *ui, int id);
void func_0207fe80(UiObject *ui, int a, int b, int c);
int func_020800fc(UiObject *ui, short *cursor, short prev, u16 a, u8 b, int c);
int func_02080468(UiObject *ui, int id);
void func_020804fc(UiObject *ui, int id);
void func_0208065c(UiObject *ui, int id);
void func_020806b0(UiObject *ui, int id);
void func_020806d8(UiObject *ui, int a, int b, int c, int d);
void func_02080798(UiObject *ui, int id, int a);
void func_020809c4(UiObject *ui, int id, int cursor, short *x, short *y);
void func_02080b2c(UiObject *ui, int id);
void func_02080bac(UiObject *ui, int id);
void func_02080c68(UiObject *ui, int id, int a);
int func_02080d54(UiObject *ui, short id, short x, short y);
int func_02080dd4(UiObject *ui, short id, short x, short y, u8 *confirmed, int a);
void func_02080f8c(UiObject *ui, int id, const char *text);
void func_02080fa8(UiObject *ui, int id, short value);
void func_0208103c(UiObject *ui, int id, short value);
void func_0208108c(UiObject *ui, int id);
void func_02081130(UiObject *ui, int id, int a);
void func_02081164(UiObject *ui, int id, int a);
void func_020813ec(UiObject *ui, int id);
UiElement *func_02081da8(UiObject *ui, int id);
void func_02081ee4(void *p, void *q);
void func_02081f20(void *p, int frameTime);
void func_0208203c(void *p);

    // sprites
void func_0205a198(Sprite *sprite);
void func_0205a234(CursorAnim *anim);
void func_0205a330(CursorAnim *anim, int frameTime);
void func_0205a370(CursorAnim *anim, int a);
CursorFrame *func_0205a3d0(CursorAnim *anim, int a);
void func_0205a444(SpriteBank *bank);
void func_0205a528(SpriteBank *bank, void *data, int arg, SafeAllocator *alloc);
void func_0205ae8c(SpriteBank *bank);

    // background layers
void func_0204af38(BgLayer *layer, int a, SafeAllocator *alloc);
void func_0204af64(BgLayer *layer);
void func_0204b010(BgLayer *layer, int a);
void func_0204b0e8(BgLayer *layer, int a);
void func_0204b11c(BgLayer *layer, int a);
void func_0204b12c(BgLayer *layer, SafeAllocator *alloc);
void func_0204b174(BgLayer *layer, void *data, SafeAllocator *alloc, unsigned int size);
void func_0204b5b4(BgLayer *layer, int a);
void func_0204b5e8(BgLayer *layer, int a, int b);
void func_0204b8d0(BgLayer *layer, int a, int b, int c, int d, int e, int f, int g, int h);
void func_0204b988(BgLayer *layer, int a, int b, int c, int d);
void func_0204c684(TitleBg *bg);
void func_0204c7a8(TitleBg *bg, SafeAllocator *alloc, void *buffer, int size);
int func_0204c7e0();

void func_020dfc40(TextArchive *archive);
void func_020dfec0(TextArchive *archive, SafeAllocator *alloc, void *file, unsigned int size);
const char *func_020e0434(TextArchive *archive, int id);

int func_02046900(void *archive);
void *func_020467f0(void *archive, int index, void *outUnk, void *outArg);

GameResources *func_0200fb8c(GameState *state);
int func_0200fb08(GameState *state);
void *func_02094a00();
void func_02094ab0(void *p);
void func_02094b30(void *p, int a, int b);
void func_02094b40(void *p);
int func_02094b4c(void *p);
void func_0205eaa0(void *p, int a, int b);
void func_02012a84(TouchState *touch, int *x, int *y);
void __clear(void *dst, unsigned int size);
void func_0206819c(char *src, char *dst, int a);
void G3X_SetFog(int, int, int, int);
void G3X_SetClearColor(int, int, int, int, int);

extern TouchState data_02114e54;
extern char data_02108760[];
extern const u32 data_ov014_021894b8[];
extern const HiddenElements data_ov014_021894cc[];
extern unsigned int data_ov014_0218981c;
extern const BestiaryScreen::StateHandler data_020e6d5c;
extern BestiaryScreen::StateHandler data_ov014_021896d4[];

extern char data_ov014_0218970c[];
extern char data_ov014_02189722[];
extern char data_ov014_02189734[];
extern char data_ov014_0218974d[];
extern char data_ov014_02189754[];
extern char data_ov014_0218976a[];
extern char data_ov014_02189784[];
extern char data_ov014_02189795[];
extern char data_ov014_021897ac[];
}

// Same table as data_ov014_021896d4, seen as a distinct object by the static
// initialization of BestiaryScreen::Update
namespace StaticInit {
    extern "C" BestiaryScreen::StateHandler data_ov014_021896d4[];
}

#define REG_DB_DISPCNT (*(volatile unsigned int *) 0x4001000)
#define REG_DB_BG0CNT (*(volatile u16 *) 0x4001008)
#define REG_DB_BG1CNT (*(volatile u16 *) 0x400100a)
#define REG_DB_BG2CNT (*(volatile u16 *) 0x400100c)
#define REG_DB_BG3CNT (*(volatile u16 *) 0x400100e)

void BestiaryScreen::Allocate(SafeAllocator *alloc) {
    if (alloc == NULL) {
        return;
    }
    titleBuffer_   = alloc->Allocate(0x4000);
    cursorSprites_ = (Sprite *) alloc->Allocate(sizeof(Sprite) * 8);
    cursorAnim_    = (CursorAnim *) alloc->Allocate(sizeof(CursorAnim));
    screenHeaps_   = (SafeAllocator *) alloc->Allocate(sizeof(SafeAllocator) * 5);
    records_       = (MonsterRecord *) alloc->Allocate(sizeof(MonsterRecord) * 0x134);
    for (u8 i = 0; i < 8; i++) {
        func_0205a198(&cursorSprites_[i]);
    }
    func_0205a234(cursorAnim_);
    for (int k = 0; k < 5; k++) {
        screenHeaps_[k].CreateTypeA(alloc->Allocate(data_ov014_021894b8[k]), data_ov014_021894b8[k]);
        screenHeaps_[k].Reset();
    }
    for (u16 j = 0; j < 0x134; j++) {
        MonsterRecord *r = &records_[j];
        r->defeatCount   = 0;
        r->unk_10        = 0;
        r->unk_11        = 0;
        r->unk_18        = 0;
    }
    ui_          = (UiObject *) alloc->Allocate(sizeof(UiObject));
    titleLayers_ = (BgLayer *) alloc->Allocate(sizeof(BgLayer) * 2);
    titleBgs_    = (TitleBg *) alloc->Allocate(sizeof(TitleBg) * 4);
    func_0207f84c(ui_);
    for (u8 i = 0; i < 2; i++) {
        func_0204af64(&titleLayers_[i]);
    }
    for (u8 i = 0; i < 4; i++) {
        func_0204c684(&titleBgs_[i]);
    }
    BestiaryViewer::Allocate(alloc);
}

void BestiaryScreen::Initialize() {
    BestiaryViewer::Initialize();
    func_020972b0(monsterList_);
    screenHeaps_   = 0;
    list_          = 0;
    pageStart_     = 0;
    ui_            = 0;
    titleLayers_   = 0;
    titleBgs_      = 0;
    titleBuffer_   = 0;
    selection_     = 0;
    cursorSprites_ = 0;
    cursorAnim_    = 0;
    records_       = 0;
    func_020dfc40(&messages_);
    func_0205a444(&cursorBank_);
    func_02081ee4(unk_14c, &unk_158);
    unk_15c        = 0;
    scrollKeys_    = 0;
    frameTime_     = 0;
    taskId_        = -1;
    elements_[0]   = -1;
    elements_[1]   = -1;
    elements_[2]   = -1;
    elements_[3]   = -1;
    elements_[4]   = -1;
    state_         = 0;
    subStep_       = 0;
    titleLoadStep_ = 0;
    category_      = -1;
    flags_         = 0;
    exitRequested_ = 0;
    totalMonsters_ = 0;
    memset(modelIdBuffer_, 0, 0x10);
}

void BestiaryScreen::Shutdown() {
    if (taskId_ >= 0) {
        BackgroundLoader *l = BackgroundLoader::GetInstance();
        l->RemoveTask(taskId_);
        taskId_ = -1;
    }
    void *p = func_02094a00();
    func_02094b40(p);
    func_02094ab0(p);
    func_020972c8(monsterList_);
    BestiaryViewer::Shutdown();
    if (screenHeaps_ == NULL) return;
    for (int i = 0; i < 5; i++) {
        if (screenHeaps_[i].GetSignedAllocator() != NULL) screenHeaps_[i].Destroy();
    }
}

// Returns 1 once there is no handler left (exit state)
int BestiaryScreen::Update(int frameTime) {
    if (frameTime == 0) frameTime = 1;
    frameTime_ = frameTime;
    if (func_ov017_021959b4()) RequestExit();
    if (state_ != State_Load) {
        func_0207fc6c(ui_, frameTime_);
        if (selection_ != 0) {
            func_02081f20(unk_14c, frameTime_);
            prevCursor_ = *selection_;
            int more    = 0;
            if (state_ == State_List) more = CountListedMonsters() > 16;
            scrollKeys_ = func_020800fc(ui_, selection_, prevCursor_, unk_158, unk_15a, more);
            ui_->cursor = *selection_;
            if (scrollKeys_) func_020804fc(ui_, activeElement_);
            if (prevCursor_ != *selection_) func_020813ec(ui_, activeElement_);
        }
    }
    BestiaryViewer::Update();
    UpdateTitleBackgroundLoad();
    if (!(data_ov014_0218981c & 1)) {
        StaticInit::data_ov014_021896d4[6] = data_020e6d5c;
        data_ov014_0218981c |= 1;
    }
    if (data_ov014_021896d4[state_] == 0) return 1;
    (this->*data_ov014_021896d4[state_])();
    return 0;
}

void BestiaryScreen::DrawModel() {
    if (state_ == State_List) {
        Draw();
    }
}

void BestiaryScreen::DrawMenu() {
    if (state_ == State_Load) {
        return;
    }
    func_0207fcb8(ui_);
    func_0207fd00(ui_);
    func_0207fe80(ui_, 1, 0, 1);
    func_0207fd44(ui_);
    DrawCursor();
}

void BestiaryScreen::DrawInfo() {
    if (state_ != State_Load) {
        func_0207fd88(ui_);
    }
    func_ov014_02185c90(this);
}

void BestiaryScreen::RequestExit() {
    if (exitRequested_) return;
    exitRequested_ = 1;
    if (selection_) *selection_ = -1;
    selection_ = 0;
    state_     = State_FadeOut;
    subStep_   = 0;
}

void BestiaryScreen::DrawCursor() {
    if (selection_ == NULL) return;
    if (activeElement_ < 0) return;
    if (!func_02081da8(ui_, activeElement_)) return;
    if (!func_0204c7e0()) return;

    short x, y;
    func_020809c4(ui_, activeElement_, *selection_, &x, &y);
    x -= 0x10;
    y -= 3;
    func_0205a370(cursorAnim_, 0);
    CursorFrame *s = func_0205a3d0(cursorAnim_, 0);
    if (s) s->flags |= 8;
    func_0205a330(cursorAnim_, frameTime_);
    short sx;
    short sy;
    sy = y;
    sx = x;
    s  = func_0205a3d0(cursorAnim_, 0);
    if (s) {
        s->x = sx;
        s->y = sy;
    }
    func_0205ae8c(&cursorBank_);
}

// A/X, or a touch on an item of the active element
u8 BestiaryScreen::IsConfirmPressed() {
    u8 confirmed = 0;
    if (g_keyManager.TestKeysPressed(KEY_A | KEY_X)) {
        confirmed = 1;
    }
    if (data_02114e54.touched && selection_) {
        int x, y;
        func_02012a84(&data_02114e54, &x, &y);
        int sel = func_02080dd4(ui_, activeElement_, x, y, &confirmed, 1);
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

// B, or a touch outside of the active element
int BestiaryScreen::IsBackPressed() {
    int result = 0;
    if (g_keyManager.TestKeysPressed(KEY_B)) {
        result = 1;
    }
    if (data_02114e54.touched) {
        int x, y;
        func_02012a84(&data_02114e54, &x, &y);
        if (func_02080d54(ui_, activeElement_, x, y) == 0) {
            result = 1;
        }
    }
    return result;
}

// SELECT or a touch on UI element 4 reverses the sort order of the list
int BestiaryScreen::UpdateSortToggle() {
    short left, top, right, bottom;
    int x, y;
    int result   = 0;
    UiElement *e = func_02081da8(ui_, 4);
    if (e && func_0204c7e0()) {
        short ty = e->y;
        short th = e->height;
        ty--;
        th++;
        left   = e->x * 8;
        right  = left + (short) (e->width * 8);
        top    = ty * 8;
        bottom = top + (short) (th * 8);
        if (data_02114e54.touched) {
            func_02012a84(&data_02114e54, &x, &y);
            if (left <= x && x < right && top <= y && y < bottom) {
                result = 1;
            }
        }
    }
    if (g_keyManager.TestKeysPressed(KEY_SELECT)) {
        result = 1;
    }
    if (result) {
        MonsterEntry *node = GetSelectedMonster();
        if (!node) {
            return 0;
        }
        if (!node->seen) {
            listCursor_ = -1;
            prevCursor_ = -1;
            node        = 0;
        }
        if (flags_ & 8) {
            flags_ &= ~8;
        } else {
            flags_ |= 8;
        }
        short tmp = 0;
        list_     = func_020974b0(monsterList_, category_, -1, (flags_ & 8) ? 1 : 0, 0x10, &tmp);
        ResetPageTo(node);
        func_0205eaa0(data_02108760, 1, 0);
        func_020804fc(ui_, 5);
        RefreshSortButton();
        RefreshListPage();
    }
    return result;
}

// Shows the page of 16 monsters containing `monster`
void BestiaryScreen::ResetPageTo(MonsterEntry *monster) {
    pageStart_  = list_;
    listCursor_ = 0x1b;
    if (ui_ != NULL) {
        listCursor_ = func_02080468(ui_, 5);
    }
    if (monster == NULL) {
        return;
    }
    MonsterEntry *n;
    MonsterEntry *page;
    short i;
    MonsterEntry *head = list_;
    i                  = 0;
    n                  = head;
    page               = head;
    while (n != NULL) {
        if (i % 16 == 0) {
            page = n;
            i    = 0;
        }
        if (n == monster) {
            break;
        }
        i++;
        n = n->next;
    }
    if (n == NULL) {
        i    = 0;
        page = head;
    }
    listCursor_ += i;
    pageStart_ = page;
}

// Moves by `delta` pages, wrapping around
void BestiaryScreen::ScrollPage(int delta) {
    short pages = (short) ((CountListedMonsters() + 15) / 16);
    short page  = GetCurrentPage();
    page += delta;
    if (page < 0) {
        page = (short) (pages + page);
    }
    if (pages <= page) {
        page = (short) (page - pages);
    }
    pageStart_ = GetPageStart(page);
}

// First monster of page `page` (or of the last page)
MonsterEntry *BestiaryScreen::GetPageStart(int page) {
    MonsterEntry *node = list_;
    int i              = 0;
    MonsterEntry *ret  = node;
    short count        = 0;
    while (node != NULL) {
        if (i == 0) {
            ret = node;
            if (count == page) {
                break;
            }
            count++;
        }
        i    = (short) (i + 1) % 16;
        node = node->next;
    }
    return ret;
}

// Monster under the list cursor; also prepares its model ids
MonsterEntry *BestiaryScreen::GetSelectedMonster() {
    UiObject *o       = ui_;
    short n           = listCursor_ - func_02080468(o, 5);
    MonsterEntry *cur = pageStart_;
    short i           = 0;
    while (cur) {
        MonsterEntry *next;
        if (n == i || (next = cur->next) == 0) break;
        cur = next;
        i++;
    }
    if (!cur) {
        pageStart_ = list_;
        i          = 0;
    }
    listCursor_ = i + func_02080468(o, 5);
    if (cur) {
        SetModelIdsFor(cur->number);
        modelIds_ = modelIdBuffer_;
    }
    return cur;
}

MonsterRecord *BestiaryScreen::GetRecord(MonsterEntry *monster) {
    if (monster == NULL) return NULL;
    short idx = monster->number - 1;
    if (idx >= 0 && idx < 0x134) return &records_[idx];
    return NULL;
}

// Sets bit 1 of MonsterEntry::flags for every monster with a non-zero record
void BestiaryScreen::MarkRecordedMonsters() {
    MonsterEntry *entries = func_02097418(monsterList_);
    u16 n                 = func_02097420(monsterList_);
    for (u16 i = 0; i < n; i++) {
        MonsterEntry *entry = &entries[i];
        MonsterRecord *rec  = GetRecord(entry);
        if (rec != NULL && rec->defeatCount != 0) {
            entry->flags |= 2;
        }
    }
}

short BestiaryScreen::CountListedMonsters() {
    MonsterEntry *n = list_;
    short count     = 0;
    while (n != 0) {
        count++;
        n = n->next;
    }
    return count;
}

short BestiaryScreen::GetCurrentPage() {
    MonsterEntry *n = list_;
    short i         = 0;
    while (n != 0) {
        if (n == pageStart_) {
            return i / 16;
        }
        i++;
        n = n->next;
    }
    return -1;
}

// State 0: loads the monster list, menu, sprites and texts
void BestiaryScreen::UpdateLoad() {
    func_0200fb8c(GameState::GetInstance());
    BackgroundLoader *loader = BackgroundLoader::GetInstance();

    if (subStep_ == 0) {
        SetFlags04(GetGameResources(), 0x609fe);
        G3X_SetFog(0, 0, 0, 0);
        G3X_SetClearColor(0, 0, 0x7fff, 0, 0);
        void *p = func_02094a00();
        func_02094ab0(p);
        func_02094b40(p);
        func_02094b30(p, 0x6a, 0);
        subStep_++;
    } else if (subStep_ == 1) {
        if (func_02094b4c(func_02094a00())) {
            subStep_++;
        }
    } else if (subStep_ == 2) {
        taskId_ = loader->QueueLoadFileInGP2(data_ov014_0218970c, data_ov014_02189722, NULL);
        subStep_++;
    } else if (subStep_ == 3) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data) {
            screenHeaps_->Reset();
            func_020972e0(monsterList_, screenHeaps_, data, size);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 4) {
        int count       = func_02097420(monsterList_);
        MonsterEntry *e = func_02097418(monsterList_);
        char buf[0x80];
        for (int i = 0; i < count; i++, e++) {
            __clear(buf, sizeof(buf));
            func_0206819c(e->name, buf, 0);
            sprintf(e->name, buf);
        }
        REG_DB_BG0CNT = (REG_DB_BG0CNT & 0x43) | 0x1d00;
        REG_DB_BG1CNT = (REG_DB_BG1CNT & 0x43) | 0x1e00;
        REG_DB_BG2CNT = (REG_DB_BG2CNT & 0x43) | 0x1f08;
        flags_ |= 0x10;
        subStep_++;
    } else if (subStep_ == 5) {
        if (flags_ & 0x10) {
            return;
        }
        SafeAllocator *heaps = screenHeaps_;
        heaps[2].Reset();
        func_0207f914(ui_, &heaps[2], data_ov014_02189734, data_ov014_0218974d);
        subStep_++;
    } else if (subStep_ == 6) {
        int r = func_0207f9f4(ui_);
        if (r == 0) {
            subStep_++;
        }
        if (r < 0) {
            state_ = State_Exit;
        }
    } else if (subStep_ == 7) {
        func_02080798(ui_, 6, 1);
        cursorBank_.unk_50     = 1;
        cursorBank_.sprites    = cursorSprites_;
        cursorBank_.numSprites = 8;
        cursorBank_.unk_3c     = cursorAnim_;
        taskId_                = loader->QueueLoadFile(data_ov014_02189754, NULL);
        subStep_++;
    } else if (subStep_ == 8) {
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
        heaps = screenHeaps_;
        heaps[3].Reset();
        for (int i = 0; i < n; i++) {
            void *r = func_020467f0(data, i, &a, &b);
            if (r) {
                func_0205a528(&cursorBank_, r, b, &heaps[3]);
            }
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_++;
    } else if (subStep_ == 9) {
        func_020dfc40(&messages_);
        taskId_ = loader->QueueLoadFileInGP2(data_ov014_0218976a, data_ov014_02189784, NULL);
        subStep_++;
    } else if (subStep_ == 10) {
        if (loader->GetTaskStatus(taskId_) == 0) {
            return;
        }
        void *data;
        unsigned int size;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        if (data) {
            SafeAllocator *heaps = screenHeaps_;
            heaps[4].Reset();
            func_020dfec0(&messages_, &heaps[4], data, size);
        }
        loader->RemoveTask(taskId_);
        taskId_ = -1;
        subStep_ += 3;
    } else if (subStep_ == 11) {
        subStep_++;
    } else if (subStep_ == 12) {
        subStep_++;
    } else if (subStep_ == 13) {
        func_020abf60(monsterList_);
        func_020abf38(records_);
        MarkRecordedMonsters();
        REG_DB_BG0CNT  = REG_DB_BG0CNT & ~3;
        REG_DB_BG1CNT  = (REG_DB_BG1CNT & ~3) | 1;
        REG_DB_BG2CNT  = (REG_DB_BG2CNT & ~3) | 2;
        REG_DB_BG3CNT  = (REG_DB_BG3CNT & ~3) | 3;
        REG_DB_DISPCNT = (REG_DB_DISPCNT & ~0x1f00) | 0x1700;
        ColorEffect_ConfigureAlphaBlend(0x4001050, 2, 4, 10, 6);
        UiObject *ui = ui_;
        ui->layers   = titleLayers_;
        ui->unk_38   = 2;
        func_0207f7f0(ui_, titleBgs_, 4);
        ui_->unk_3a = 0;
        func_020974b0(monsterList_, -1, -1, 0, 0, &totalMonsters_);
        short tmp  = 0;
        pageStart_ = list_ = func_020974b0(monsterList_, category_, -1, (flags_ & 8) ? 1 : 0, 0x10, &tmp);
        state_             = State_FadeIn;
        subStep_           = 0;
    }
}

// State 1
void BestiaryScreen::UpdateFadeIn() {
    GameResources *res = func_0200fb8c(GameState::GetInstance());
    if (subStep_ == 0) {
        texts_ = &messages_;
        RefreshHeader();
        activeElement_ = 1;
        if (menuCursor_ < 0) {
            menuCursor_ = func_02080468(ui_, activeElement_);
        }
        ui_->cursor = menuCursor_;
        func_020813ec(ui_, activeElement_);
        subStep_++;
    } else if (subStep_ == 1) {
        SetBrightness(res, 0, 8);
        subStep_++;
    } else if (subStep_ == 2) {
        if (!IsBrightnessTransitionActive(res)) {
            state_   = State_MainMenu;
            subStep_ = 0;
        }
    }
}

// State 2: entry 8 opens the whole list, entry 9 the category menu
void BestiaryScreen::UpdateMainMenu() {
    if (subStep_ == 0) {
        if (flags_ & 0x10) {
            return;
        }
        short out  = 0;
        pageStart_ = list_ = func_020974b0(monsterList_, category_, -1, (flags_ & 8) ? 1 : 0, 0x10, &out);
        RefreshHeader();
        activeElement_ = 1;
        if (menuCursor_ < 0) {
            menuCursor_ = func_02080468(ui_, activeElement_);
        }
        ui_->cursor = menuCursor_;
        func_020813ec(ui_, activeElement_);
        func_0208203c(unk_14c);
        selection_ = 0;
        subStep_++;
    } else if (subStep_ == 1) {
        selection_ = &menuCursor_;
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(unk_14c);
            switch (*selection_) {
                case 8:
                    flags_ |= 0x10;
                    state_   = State_List;
                    subStep_ = 0;
                    break;
                case 9:
                    state_   = State_CategoryMenu;
                    subStep_ = 0;
                    break;
            }
            func_0207fdcc(ui_, activeElement_);
            selection_ = 0;
        } else if (IsBackPressed() || g_keyManager.TestKeysPressed(KEY_Y)) {
            state_   = State_FadeOut;
            subStep_ = 0;
        }
    }
}

// State 3: choice of the monster category
void BestiaryScreen::UpdateCategoryMenu() {
    if (subStep_ == 0) {
        if (flags_ & 0x10) {
            return;
        }
        activeElement_ = 2;
        if (categoryCursor_ < 0) {
            categoryCursor_ = func_02080468(ui_, activeElement_);
        }
        func_020813ec(ui_, activeElement_);
        category_  = categoryCursor_ - 0xb;
        short tmp  = 0;
        pageStart_ = list_ = func_020974b0(monsterList_, category_, -1, (flags_ & 8) ? 1 : 0, 0x10, &tmp);
        RefreshHeader();
        subStep_++;
    } else if (subStep_ == 1) {
        selection_ = &categoryCursor_;
        category_  = categoryCursor_ - 0xb;
        if (scrollKeys_ != 0 || prevCursor_ != *selection_) {
            short tmp  = 0;
            pageStart_ = list_ = func_020974b0(monsterList_, category_, -1, (flags_ & 8) ? 1 : 0, 0x10, &tmp);
            RefreshHeader();
        }
        if (IsConfirmPressed()) {
            func_0205eaa0(data_02108760, 1, 0);
            func_0208203c(unk_14c);
            if (CountListedMonsters() == 0) {
                return;
            }
            func_0207fdcc(ui_, activeElement_);
            selection_ = 0;
            state_     = State_List;
            subStep_   = 0;
            flags_ |= 0x10;
        } else if (IsBackPressed()) {
            func_0207fdcc(ui_, activeElement_);
            categoryCursor_ = -1;
            category_       = -1;
            state_          = State_MainMenu;
            subStep_        = 0;
        } else if (g_keyManager.TestKeysPressed(KEY_Y)) {
            func_0207fdcc(ui_, activeElement_);
            categoryCursor_ = -1;
            category_       = -1;
            state_          = State_FadeOut;
            subStep_        = 0;
        }
    }
}

// State 4: monster list, the selected monster is shown by the viewer
void BestiaryScreen::UpdateList() {
    MonsterEntry *node;
    if (subStep_ == 0) {
        if (flags_ & 0x10) {
            return;
        }
        pageStart_ = list_;
        RefreshHeader();
        RefreshCategoryTitle();
        RefreshSortButton();
        activeElement_ = 5;
        listCursor_    = func_02080468(ui_, activeElement_);
        RefreshListPage();
        func_0208203c(unk_14c);
        selection_ = 0;
        subStep_++;
        monster_     = 0;
        prevMonster_ = 0;
        node         = GetSelectedMonster();
        SetMonster(node, GetRecord(node), 1);
        flags82_ |= 8;
    } else if (subStep_ == 1) {
        selection_ = &listCursor_;
        if (UpdateSortToggle()) {
            if (prevCursor_ < 0) {
                node = GetSelectedMonster();
                if (node->seen) {
                    ChangeMonster(node, GetRecord(node));
                }
                prevCursor_ = *selection_;
            }
            return;
        }
        if (scrollKeys_ != 0 || prevCursor_ != *selection_) {
            if (scrollKeys_ & 0x10) {
                ScrollPage(1);
            } else if (scrollKeys_ & 0x20) {
                ScrollPage(-1);
            }
            RefreshListPage();
            node = GetSelectedMonster();
            ChangeMonster(node, GetRecord(node));
        }
        if (IsConfirmPressed()) {
            if (flags_ & 0x40) {
                node = GetSelectedMonster();
                ChangeMonster(node, GetRecord(node));
            } else {
                CycleAnimation();
            }
            return;
        }
        if (g_keyManager.TestKeysPressed(KEY_Y)) {
            ToggleInfoPage();
            return;
        }
        if (!IsBackPressed()) {
            return;
        }
        flags81_ &= ~4;
        if (category_ >= 0) {
            state_ = State_CategoryMenu;
        }
        if (category_ < 0) {
            state_ = State_MainMenu;
        }
        monster_     = 0;
        prevMonster_ = 0;
        flags_ |= 0x10;
        func_0207fdcc(ui_, activeElement_);
        subStep_   = 0;
        selection_ = 0;
        func_0207fdcc(ui_, 4);
        func_0207fdcc(ui_, 3);
        CancelMonsterLoad();
    }
}

// State 5
void BestiaryScreen::UpdateFadeOut() {
    GameResources *res = func_0200fb8c(GameState::GetInstance());
    if (subStep_ == 0) {
        SetBrightness(res, -16, 8);
        subStep_++;
    } else if (subStep_ == 1) {
        if (!IsBrightnessTransitionActive(res)) {
            state_   = State_Exit;
            subStep_ = 0;
        }
    }
}

// Category, number of seen monsters and completion percentage
void BestiaryScreen::RefreshHeader() {
    int mode = 0;
    if (state_ == State_List) mode = 1;
    if (state_ == State_CategoryMenu) mode = 2;
    func_020806d8(ui_, 0, 2, 1, 4);
    func_02080bac(ui_, 0);
    if (category_ >= 0) {
        func_0208103c(ui_, 0x2f, (short) (category_ + 0x1e));
    }
    MonsterEntry *node = list_;
    short count        = 0;
    if (node != 0) {
        for (; node != 0; node = node->next) {
            count = count + node->seen;
        }
    }
    short total = totalMonsters_;
    short pct   = 0;
    if (total != 0) {
        float f = (float) count / (float) total;
        f *= 10000.0f;
        if (f < 100.0f) f = 100.0f;
        if (count == 0) f = 0.0f;
        short v = f;
        pct     = v / 100;
    }
    func_02080fa8(ui_, 4, pct);
    func_02080fa8(ui_, 5, count);
    u8 i = 0;
    while (true) {
        short id = data_ov014_021894cc[mode].ids[i];
        if (id < 0) break;
        func_02080b2c(ui_, id);
        i++;
    }
    func_02081164(ui_, 0, 1);
    func_020813ec(ui_, 0);
}

void BestiaryScreen::RefreshCategoryTitle() {
    short v = menuCursor_ + 0x10;
    if (menuCursor_ == 9) {
        if (categoryCursor_ >= 0) {
            v = categoryCursor_ - 4;
        }
    }
    func_0208103c(ui_, 0x19, v);
    func_0208108c(ui_, 0x19);
    func_02081164(ui_, 3, 1);
    func_02080c68(ui_, 3, 1);
    func_020813ec(ui_, 3);
}

void BestiaryScreen::RefreshSortButton() {
    func_0208103c(ui_, 0x1a, (short) (((flags_ & 8) ? 1 : 0) + 0x1b));
    func_02080798(ui_, 0x1a, 1);
    func_02081164(ui_, 4, 1);
    func_02080c68(ui_, 4, 1);
    func_020813ec(ui_, 4);
}

// Names of the 16 monsters of the page (UI elements 0x1b-0x2a), page numbers
void BestiaryScreen::RefreshListPage() {
    UiObject *ui = ui_;
    ui->cursor   = listCursor_;
    func_02080bac(ui, 5);
    func_0208065c(ui, 5);
    MonsterEntry *node = pageStart_;
    short id           = 0x1b;
    for (short i = 0; i < 16; id++, i++) {
        if (node != 0) {
            func_02080f8c(ui, id, func_020e0434(&messages_, 0x15));
            if (node->seen) {
                func_02080f8c(ui_, id, node->name);
            }
            node = node->next;
        } else {
            func_020806b0(ui, id);
            func_02080b2c(ui, id);
        }
    }
    short count = CountListedMonsters();
    func_02080fa8(ui, 0x2b, (short) (GetCurrentPage() + 1));
    func_02080fa8(ui, 0x2d, (short) ((count + 15) / 16));
    func_02081130(ui, 5, count > 16);
    func_020813ec(ui, 5);
}

// Loads the title background (bg_smlttl.pac, or bg_smliti.pac in the list)
void BestiaryScreen::UpdateTitleBackgroundLoad() {
    SafeAllocator *heaps;
    GameState *gs = GameState::GetInstance();
    if (!(flags_ & 0x10)) return;
    BackgroundLoader *loader = BackgroundLoader::GetInstance();
    if (titleLoadStep_ == 0) {
        const char *name = data_ov014_02189795;
        if (state_ == State_List) name = data_ov014_021897ac;
        titleTaskId_ = loader->QueueLoadFile(name, NULL);
        titleLoadStep_++;
    } else if (titleLoadStep_ == 1) {
        if (loader->GetTaskStatus(titleTaskId_) == 0) return;
        int unk;
        void *data;
        unsigned int size;
        unsigned int sz;
        loader->GetLoadedFileByID(titleTaskId_, &data, &size);
        int count = func_02046900(data);
        heaps     = screenHeaps_;
        heaps[1].Reset();
        BgLayer layer;
        func_0204af64(&layer);
        func_0204b11c(&layer, 0);
        layer.unk_1c_0 = 1;
        layer.index    = 2;
        func_0204b5b4(&layer, 2);
        func_0204b12c(&layer, &heaps[1]);
        func_0204b5e8(&layer, 0, 0);
        func_0204af38(&layer, 5, &heaps[1]);
        for (int i = 1; i < count; i++) {
            void *r = func_020467f0(data, i, &unk, &sz);
            if (r) func_0204b174(&layer, r, &heaps[1], sz);
        }
        loader->RemoveTask(titleTaskId_);
        titleTaskId_ = -1;
        int region   = func_0200fb08(gs);
        int lang     = 0;
        switch (region) {
            case 2: lang = 1; break;
            case 4: lang = 2; break;
            case 3: lang = 3; break;
            case 5: lang = 4; break;
        }
        func_0204b010(&layer, 0);
        func_0204b988(&layer, 0, 0, 0, 0xffff);
        if (lang != 0) func_0204b8d0(&layer, lang, 0, 0, 0x16, 2, 9, 2, 0xffff);
        func_0204b0e8(&layer, 0);
        heaps[1].Reset();
        for (u8 i = 0; i < 2; i++) {
            BgLayer *l = &titleLayers_[i];
            func_0204af64(l);
            l->unk_1c_0 = 1;
            l->index    = i;
            func_0204b11c(l, 0);
            func_0204b5b4(l, i);
            func_0204b12c(l, &heaps[1]);
            func_0204b5e8(l, 0, 0);
        }
        void *r = func_020467f0(data, 0, &unk, &sz);
        if (r) func_0204b174(titleLayers_, r, &heaps[1], sz);
        for (u8 j = 0; j < 2; j++) {
            BgLayer *l = &titleLayers_[j];
            func_0204b010(l, 0);
            func_0204b0e8(l, 0);
        }
        for (u8 k = 0; k < 4; k++) {
            TitleBg *u = &titleBgs_[k];
            func_0204c684(u);
            func_0204c7a8(u, &heaps[1], titleBuffer_, 0x600);
            u->layers = titleLayers_;
        }
        titleLoadStep_ = 0;
        flags_ &= ~0x10;
    }
}

// Model ids of the monster with this bestiary number (0-terminated list)
void BestiaryScreen::SetModelIdsFor(int number) {
    MonsterEntry *e = func_02097418(monsterList_);
    if (e == NULL) return;
    int n = func_02097420(monsterList_);
    if (n == 0) return;
    memset(modelIdBuffer_, 0, 0x10);
    for (int i = 0; i < n; i++, e++) {
        if (e != NULL && e->seen) {
            if (e->number == number) {
                modelIdBuffer_[0] = e->modelId;
                return;
            }
        }
    }
}
