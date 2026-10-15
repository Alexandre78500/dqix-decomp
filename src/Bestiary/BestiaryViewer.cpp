#include "Bestiary/BestiaryViewer.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include "GameState/GameState.h"
#include "Graphics/LightingManager.h"
#include "Input/Input.h"
#include "Resource/Brightness.h"
#include "Resource/GameResources.h"
#include "std_library_functions.h"

// Temporary objects used while reading the texts of a monster
struct TextList
{
    u8 unk_0[0xc];
};

struct HeapSizes
{
    u32 size[6];
};

struct ScreenOffset
{
    short x;
    short y;
};

struct VariantInfo
{
    u32 unk_0;
    u8 unk_4[0x2];
    u16 unk_6;
};

struct Ov014Globals
{
    u32 unk_0;
    // guard of the static handler table of BestiaryViewer::Update
    u32 handlerTableGuard;
    u32 unk_8;
    void* savedScene;
};

extern "C"
{
    void func_02096fb0(void* monsterData);
    void func_02096fc4(void* monsterData);
    void func_02097054(void* monsterData, SafeAllocator* alloc, void* file, u32 size, int modelId);
    MonsterModelInfo* func_02097224(void* monsterData, int modelId);
    VariantInfo* func_02097238(void* monsterData);

    void func_0205a198(Sprite* sprite);
    void func_0205a444(SpriteBank* bank);
    void func_0205a528(SpriteBank* bank, void* data, int arg, SafeAllocator* alloc);
    void func_0205ac40(SpriteBank* bank, Sprite* sprite);

    void func_ov023_021e20c0(ScreenLayout* layout);
    void func_ov023_021e20f0(ScreenLayout* layout, SafeAllocator* alloc, void* file, unsigned int size);
    void func_ov023_021e2bdc(ScreenLayout* layout, int id, short* x, short* y);

    void func_020e526c(TextList* list);
    void func_020e56fc(TextList* list, void* file, u32 size);
    void** func_020e5294(TextList* list, int id);
    void func_020e4864(void* text, u8* dst, int a, int b, int c, int d);
    void func_020dfc2c(TextArchive* archive);
    void func_020dfc40(TextArchive* archive);
    void func_020e046c(u8* dst, void* file, u32 size, int number);

    int func_02046900(void* archive);
    void* func_020467f0(void* archive, int index, void* outUnk, void* outArg);

    void func_0207df50(void* p);
    void func_0207df90(void* p);
    void func_0207dfac(void* p);
    void func_0203dafc(ObjectArchiveLoadInfo* info);

    void* func_020100f8(GameState* state);
    void func_020100c4(GameState* state, void* scene);
    void func_020a2010(void* scene);
    void func_0202e5c8(void* scene, int x, int y, int z);
    void func_0202e5d8(void* scene, int x, int y, int z);
    void func_0202e0a4(void* scene);
    void func_020a27a0(void* scene);
    void G3X_SetFog(int, int, int, int);
    void G3X_SetClearColor(int, int, int, int, int);

    extern const u32 data_ov014_02189480[];
    extern const HeapSizes data_ov014_02189498;
    extern const short data_ov014_02189478[];
    extern const ScreenOffset data_ov014_0218948c[];
    extern const ScreenOffset data_ov014_0218948e[];
    extern Ov014Globals data_ov014_02189800;

    extern const BestiaryViewer::Handler data_020e6d5c;
    extern BestiaryViewer::Handler data_ov014_021895a0[];
    extern BestiaryViewer::Handler data_ov014_021895b0[];

    extern char data_ov014_021895c8[]; // "stand"
    extern char data_ov014_021895ce[]; // "run"
    extern char data_ov014_021895d5[];
    extern char data_ov014_021895ed[];
    extern char data_ov014_02189603[];
    extern char data_ov014_02189615[];
    extern char data_ov014_0218962c[];
    extern char data_ov014_0218963f[];
    extern char data_ov014_02189652[];
    extern char data_ov014_02189661[];
    extern char data_ov014_02189668[];
    extern char data_ov014_02189680[];
    extern char data_ov014_02189686[];
    extern char data_ov014_0218968c[];
    extern char data_ov014_02189692[];
    extern char data_ov014_021896a7[];
    extern char data_ov014_021896bc[];
}

// Vector3i's implicit copy assignment (main, 0x02013b54). Called under its
// symbol name so that this file does not emit a weak copy of it.

#define REG_DISPCNT (*(volatile unsigned int*)0x4000000)
#define REG_BG0CNT (*(volatile u16*)0x4000008)
#define REG_BG1CNT (*(volatile u16*)0x400000a)
#define REG_BG2CNT (*(volatile u16*)0x400000c)
#define REG_BG3CNT (*(volatile u16*)0x400000e)

ScreenLayout::Element* ScreenLayout::FindElement(int id)
{
    Element* elements = elements_;
    if (elements == 0)
    {
        return 0;
    }
    u16 count = numElements_;
    if (count == 0)
    {
        return 0;
    }
    for (u16 i = 0; i < count; i++)
    {
        Element* e = &elements[i];
        if (e->id == id)
        {
            return e;
        }
    }
    return 0;
}

void BestiaryViewer::Allocate(SafeAllocator* alloc)
{
    if (alloc == NULL)
    {
        return;
    }
    HeapSizes sizes = data_ov014_02189498;

    heaps_ = (SafeAllocator*)alloc->Allocate(sizeof(SafeAllocator) * 6);
    buffers_ = (void**)alloc->Allocate(sizeof(void*) * 3);
    for (int i = 0; i < 3; i++)
    {
        buffers_[i] = alloc->Allocate(data_ov014_02189480[i]);
    }
    model_ = (Object3D*)alloc->Allocate(0xac);
    scene_ = alloc->Allocate(0x2c8);
    layout_ = (ScreenLayout*)alloc->Allocate(0x4c);
    nameText_ = (u8*)alloc->Allocate(0x48);
    infoText_ = (u8*)alloc->Allocate(0x48);
    pageText_ = (u8*)alloc->Allocate(0x200);
    spriteBank_ = (SpriteBank*)alloc->Allocate(0x54);
    sprites_ = (Sprite*)alloc->Allocate(sizeof(Sprite) * 3);
    for (u8 j = 0; j < 3; j++)
    {
        func_0205a198(&sprites_[j]);
    }
    func_0205a444(spriteBank_);
    spriteBank_->unk_50 = 0;
    SpriteBank* bank = spriteBank_;
    bank->sprites = sprites_;
    bank->numSprites = 3;
    for (int k = 0; k < 6; k++)
    {
        heaps_[k].CreateTypeA(alloc->Allocate(sizes.size[k]), sizes.size[k]);
        heaps_[k].Reset();
    }
    model_->Initialize();
    model_->EnableFlag(4);
    flags82_ |= 4;
}

void BestiaryViewer::Initialize()
{
    data_ov014_02189800.savedScene = 0;
    buffers_ = 0;
    spriteBank_ = 0;
    sprites_ = 0;
    func_02096fb0(monsterData_);
    tableLoader_.Init();
    heaps_ = 0;
    model_ = 0;
    scene_ = 0;
    monster_ = 0;
    prevMonster_ = 0;
    record_ = 0;
    prevRecord_ = 0;
    layout_ = 0;
    texts_ = 0;
    nameText_ = 0;
    infoText_ = 0;
    pageText_ = 0;
    layoutTaskId_ = -1;
    topBgTaskId_ = -1;
    unk_78 = -1;
    animIndex_ = 0;
    mode_ = 0;
    loadStep_ = 0;
    topBgLoadStep_ = 0;
    monsterLoadStep_ = 0;
    page_ = 0;
    flags81_ = 0;
    autoRotating_ = 0;
    flags82_1_ = 0;
    loadModel_ = 0;
    flags82_4_ = 0;
    flags82_3_ = 0;
    for (int i = 0; i < 5; i++)
    {
        tasks_[i] = -1;
    }
    modelIds_ = 0;
    variant_.Reset();
    memset(spriteOffset_, 0, 3);
    nextShowSprites_ = 0;
    showSprites_ = 0;
}

void MonsterVariant::Reset()
{
    unk_0 = 0;
    modelId = -1;
    unk_6 = 0;
}

void BestiaryViewer::Shutdown()
{
    if (layoutTaskId_ >= 0)
    {
        BackgroundLoader* l = BackgroundLoader::GetInstance();
        l->RemoveTask(layoutTaskId_);
        layoutTaskId_ = -1;
    }
    if (topBgTaskId_ >= 0)
    {
        BackgroundLoader* l = BackgroundLoader::GetInstance();
        l->RemoveTask(topBgTaskId_);
        topBgTaskId_ = -1;
    }
    for (int i = 0; i < 5; i++)
    {
        if (tasks_[i] >= 0)
        {
            BackgroundLoader::GetInstance()->RemoveTask(tasks_[i]);
            tasks_[i] = -1;
        }
    }
    tableLoader_.Cancel();
    func_02096fc4(monsterData_);
    tableLoader_.Init();
    func_020100c4(GameState::GetInstance(), data_ov014_02189800.savedScene);
    data_ov014_02189800.savedScene = 0;
    LightingManager::GetInstance()->fogEnabled_ = true;
    if (heaps_ == NULL)
        return;
    for (int i = 0; i < 6; i++)
    {
        if (heaps_[i].GetSignedAllocator() != NULL)
            heaps_[i].Destroy();
    }
}

// Rotation with L/R (both: turn back to the default angle), idle animation,
// then the handler of the current mode
void BestiaryViewer::Update()
{
    UpdateTopBackgroundLoad();
    if (model_ != NULL)
    {
        model_->AdvanceEffects();
        int base = 0;
        if (monster_ != NULL)
        {
            MonsterModelInfo* p = func_02097224(monsterData_, monster_->modelId);
            if (p != NULL)
                base = (int)(4096.0f * p->rotationY);
        }
        Object3D* m;
        u8 rotating;
        int speed;
        u8 keep;
        u8 manual;
        rotating = autoRotating_;
        m = model_;
        if (m == NULL)
        {
            keep = 0;
        }
        else
        {
            manual = 0;
            rotatingLeft_ = 0;
            keep = rotating;
            speed = 0;
            rotatingRight_ = 0;
            if (g_keyManager.TestKeysHeld(KEY_L) && g_keyManager.TestKeysHeld(KEY_R))
            {
                if (!rotating)
                {
                    keep = 1;
                    rotatingLeft_ = keep;
                    rotatingRight_ = keep;
                }
            }
            else if (g_keyManager.TestKeysHeld(KEY_L) && !g_keyManager.TestKeysReleased(KEY_R))
            {
                manual = 1;
                rotatingLeft_ = manual;
                speed = 0xcc;
            }
            else if (g_keyManager.TestKeysHeld(KEY_R) && !g_keyManager.TestKeysReleased(KEY_L))
            {
                manual = 1;
                rotatingRight_ = manual;
                speed = -0xcc;
            }
            Vector3i rot2;
            Vector3i rot;
            if (rotating)
            {
                rot = m->rotation_;
                int d = base - rot.y;
                if (rot.y >= base + 0x3243)
                    d = base + 0x6487 - rot.y;
                speed = d / 5;
                if (!g_keyManager.TestKeysHeld(KEY_R) && !g_keyManager.TestKeysHeld(KEY_L) && rot.y == base)
                    keep = 0;
            }
            if (manual || rotating)
            {
                speed = speed * GameState::GetInstance()->GetTickCount();
                rot2 = m->rotation_;
                rot2.y += speed;
                if (rotating)
                {
                    if (rot2.y < base + 0xcc || rot2.y > base + 0x63bb)
                        rot2.y = base;
                    if (!rotatingLeft_ || !rotatingRight_ || rot2.y == base)
                    {
                        rotatingLeft_ = 0;
                        rotatingRight_ = 0;
                    }
                }
                if (rot2.y < 0)
                    rot2.y += 0x6487;
                if (rot2.y >= 0x6487)
                    rot2.y -= 0x6487;
                m->rotation_ = rot2;
            }
        }
        autoRotating_ = keep;
        if (model_->HasAnimationStopped() || model_->HasAnimationReachedEnd())
        {
            m = model_;
            BCFG::AnimationRecord* rec = m->activeAnimationRecord_;
            if (rec != NULL && strcmp(data_ov014_021895c8, rec->name) != 0 && strcmp(data_ov014_021895ce, rec->name) != 0)
                m->MaybeSetRegularAnimation(data_ov014_021895c8, 0);
        }
    }
    unk_92 = 0;
    if (!(data_ov014_02189800.handlerTableGuard & 1))
    {
        data_ov014_021895a0[4] = data_020e6d5c;
        data_ov014_02189800.handlerTableGuard |= 1;
    }
    if (data_ov014_021895b0[mode_] != 0)
        (this->*data_ov014_021895b0[mode_])();
}

void BestiaryViewer::Draw()
{
    if (!mode_)
        return;
    MonsterEntry* cur = monster_;
    if (cur == 0 || record_ == 0)
        return;
    if (!cur->seen)
        return;

    int show0 = 0;
    int show1 = 0;
    if (flags81_ & 0x20)
    {
        if (prevMonster_ && prevMonster_->seen)
            show0 = 1;
        if (prevRecord_ && prevRecord_->unk_10)
            show1 = 1;
    }
    else
    {
        if (cur->seen)
            show0 = 1;
        if (record_->unk_10)
            show1 = 1;
    }

    if (show0 || show1)
    {
        for (int i = 0; i < 3; i++)
        {
            short x = 0;
            short y = 0;
            if (i == 2 ? show1 : show0)
            {
                func_ov023_021e2bdc(layout_, data_ov014_02189478[i], &x, &y);
                if (spriteOffset_[i])
                {
                    x += data_ov014_0218948c[i].x;
                    y += data_ov014_0218948e[i].x;
                }
                Sprite* s = &sprites_[i];
                s->x = x << 12;
                s->y = y << 12;
                if (showSprites_)
                    func_0205ac40(spriteBank_, &sprites_[i]);
            }
        }
    }

    if ((flags81_ & 4) && model_)
        model_->Draw(true);
}

// Places the model as described by the monster data
void BestiaryViewer::ApplyPlacement()
{
    MonsterEntry* ref = monster_;
    if (ref == NULL)
        return;
    MonsterModelInfo* p = func_02097224(monsterData_, ref->modelId);
    if (p == NULL)
        return;

    Vector3i pos;
    Vector3i rot;
    Vector3i scale;
    pos.x = (int)(4096.0f * p->position[0]);
    pos.y = (int)(4096.0f * p->position[1]);
    pos.z = (int)(4096.0f * p->position[2]);
    rot.x = 0;
    rot.y = (int)(4096.0f * p->rotationY);
    rot.z = 0;
    scale.x = (int)(4096.0f * p->scale[0]);
    scale.y = (int)(4096.0f * p->scale[1]);
    scale.z = (int)(4096.0f * p->scale[2]);
    model_->position_ = pos;
    model_->rotation_ = rot;
    model_->SetScale(&scale);
}

void BestiaryViewer::CancelMonsterLoad()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (flags81_ & 1)
    {
        flags81_ &= ~1;
        if (layoutTaskId_ >= 0)
        {
            loader->RemoveTask(layoutTaskId_);
            layoutTaskId_ = -1;
        }
        for (int i = 0; i < 5; i++)
        {
            if (tasks_[i] >= 0)
            {
                BackgroundLoader::GetInstance()->RemoveTask(tasks_[i]);
                tasks_[i] = -1;
            }
        }
        tableLoader_.Cancel();
        monsterLoadStep_ = 0;
    }
    if (flags81_ & 0x10)
        return;
    flags81_ |= 0x10;
    if (topBgTaskId_ >= 0)
    {
        loader->RemoveTask(topBgTaskId_);
        topBgTaskId_ = -1;
    }
    topBgLoadStep_ = 0;
}

// Y button: switches between the two info pages (mon_trv1 / mon_trv2)
void BestiaryViewer::ToggleInfoPage()
{
    if (flags81_ & 1)
        return;
    if (flags81_ & 8)
        return;
    if (record_ == NULL)
        return;
    if (!record_->unk_10)
        return;
    u8 cur = page_;
    SetMonster(monster_, record_, 0);
    page_ = cur + 1;
    page_ &= 1;
    unk_92 = 1;
    flags81_ |= 8;
}

// Plays the next animation of the monster once the idle one is running
void BestiaryViewer::CycleAnimation()
{
    if (model_ == NULL || model_->loadedAnimationPackageList_ == NULL || monster_ == NULL)
        return;
    MonsterModelInfo* info = func_02097224(monsterData_, monster_->modelId);
    if (info == NULL)
        return;
    BCFG::AnimationRecord* rec = model_->activeAnimationRecord_;
    if (rec != NULL && strcmp(data_ov014_021895c8, rec->name) != 0 && strcmp(data_ov014_021895ce, rec->name) != 0)
        return;
    animIndex_++;
    if (info->animCount <= animIndex_)
    {
        animIndex_ = 0;
        if (info->animCount > 1)
            animIndex_ = 1;
    }
    char* name = info->animNames[animIndex_];
    if (name == NULL || name[0] == 0)
        return;
    model_->MaybeSetRegularAnimation(name, 0);
}

// Keeps the previous monster (for the sprites) and loads a new one
void BestiaryViewer::ChangeMonster(MonsterEntry* monster, MonsterRecord* record)
{
    if (!mode_)
        return;
    if (prevMonster_ != monster_)
    {
        prevMonster_ = monster_;
        prevRecord_ = record_;
        flags81_ |= 0x20;
    }
    if (monster == 0 || record == 0)
    {
        monster_ = monster;
        return;
    }
    if (monster_ == monster || record_ == record)
        return;
    SetMonster(monster, record, 1);
}

void BestiaryViewer::SetMonster(MonsterEntry* monster, MonsterRecord* record, u8 loadModel)
{
    if (!mode_)
        return;
    if (monster == 0 || record == 0)
    {
        monster_ = monster;
        return;
    }
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (layoutTaskId_ >= 0)
    {
        loader->RemoveTask(layoutTaskId_);
        layoutTaskId_ = -1;
    }
    for (int i = 0; i < 5; i++)
    {
        if (tasks_[i] >= 0)
        {
            BackgroundLoader::GetInstance()->RemoveTask(tasks_[i]);
            tasks_[i] = -1;
        }
    }
    tableLoader_.Cancel();
    monsterLoadStep_ = 0;
    page_ = 0;
    monster_ = monster;
    record_ = record;
    *nameText_ = 0;
    *infoText_ = 0;
    *pageText_ = 0;
    flags81_ |= 1;
    pageToggled_ = 0;
    loadModel_ = loadModel;
    if (!monster_->seen)
    {
        loadingMonster_ = 0;
        showTopBg_ = 1;
        modelReady_ = 0;
        nextShowSprites_ = 0;
        return;
    }
    showTopBg_ = 0;
    if (topBgTaskId_ >= 0)
    {
        loader->RemoveTask(topBgTaskId_);
        topBgTaskId_ = -1;
    }
    topBgLoadStep_ = 0;
    nextShowSprites_ = 1;
}

// Loads the data, texts and model of the monster set by SetMonster
void BestiaryViewer::UpdateMonsterLoad()
{
    if (!(flags81_ & 1))
        return;
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (monsterLoadStep_ == 0)
    {
        char gp2[0x40] = {0};
        char inner[0x40] = {0};
        if (loadModel_)
            tasks_[0] = loader->QueueLoadFile(data_ov014_021895d5, NULL);
        heaps_[1].Reset();
        tableLoader_.Start(&heaps_[1], monster_->number);
        tasks_[1] = loader->QueueLoadFileInGP2(data_ov014_021895ed, data_ov014_02189603, NULL);
        sprintf(gp2, data_ov014_02189615, page_ + 1);
        sprintf(inner, data_ov014_0218962c, page_ + 1);
        tasks_[2] = loader->QueueLoadFileInGP2(gp2, inner, NULL);
        tasks_[3] = loader->QueueLoadFileInGP2(data_ov014_0218963f, data_ov014_02189652, NULL);
        if (loadModel_ && monster_ && record_ && monster_->seen && monster_->fileName)
        {
            sprintf(inner, data_ov014_02189661, monster_->fileName);
            tasks_[4] = loader->QueueLoadFileInGP2(data_ov014_02189668, inner, NULL);
        }
        monsterLoadStep_++;
    }
    else if (monsterLoadStep_ == 1)
    {
        if (loadModel_)
        {
            if (loader->GetTaskStatus(tasks_[0]))
            {
                void* data;
                u32 size;
                loader->GetLoadedFileByID(tasks_[0], &data, &size);
                if (data)
                {
                    short* p = modelIds_;
                    variant_.Reset();
                    SafeAllocator* heap = heaps_;
                    for (; *p != 0; p++)
                    {
                        heap->Reset();
                        short id = *p;
                        func_02096fb0(monsterData_);
                        func_02097054(monsterData_, heap, data, size, id);
                        VariantInfo* v = func_02097238(monsterData_);
                        if (variant_.modelId < 0 || (v && (v->unk_0 > variant_.unk_0 || v->unk_6 > variant_.unk_6)))
                        {
                            variant_.modelId = id;
                            variant_.unk_0 = v->unk_0;
                            variant_.unk_6 = v->unk_6;
                        }
                    }
                    heap->Reset();
                    short id = monster_->modelId;
                    func_02096fb0(monsterData_);
                    func_02097054(monsterData_, heap, data, size, id);
                    func_02097238(monsterData_);
                }
                loader->RemoveTask(tasks_[0]);
                tasks_[0] = -1;
                monsterLoadStep_++;
            }
        }
        else
        {
            monsterLoadStep_++;
        }
    }

    if (monsterLoadStep_ == 2)
    {
        if (tableLoader_.Update())
            monsterLoadStep_++;
    }

    if (monsterLoadStep_ == 3)
    {
        if (loader->GetTaskStatus(tasks_[1]))
        {
            void* data;
            u32 size;
            loader->GetLoadedFileByID(tasks_[1], &data, &size);
            if (data)
            {
                MonsterModelInfo* info = func_02097224(monsterData_, monster_->modelId);
                if (info)
                {
                    TextList o;
                    func_020e526c(&o);
                    func_020e56fc(&o, data, size);
                    void** e = func_020e5294(&o, info->nameTextId);
                    if (e)
                    {
                        memset(nameText_, 0, 0x48);
                        func_020e4864(*e, nameText_, 1, 0, 0, 0);
                    }
                    e = func_020e5294(&o, info->infoTextId);
                    if (e)
                    {
                        memset(infoText_, 0, 0x48);
                        func_020e4864(*e, infoText_, 1, 0, 0, 0);
                    }
                }
            }
            loader->RemoveTask(tasks_[1]);
            tasks_[1] = -1;
            monsterLoadStep_++;
        }
    }

    if (monsterLoadStep_ == 4)
    {
        if (loader->GetTaskStatus(tasks_[2]))
        {
            void* data;
            u32 size;
            loader->GetLoadedFileByID(tasks_[2], &data, &size);
            if (data)
            {
                TextArchive o;
                func_020dfc2c(&o);
                func_020dfc40(&o);
                func_020e046c(pageText_, data, size, monster_->number);
            }
            loader->RemoveTask(tasks_[2]);
            tasks_[2] = -1;
            monsterLoadStep_++;
        }
    }

    if (monsterLoadStep_ == 6)
    {
        if (!(flags81_ & 8) && tasks_[4] == -1)
        {
            monsterLoadStep_ = 0;
            flags81_ &= ~9;
        }
        monsterLoadStep_++;
    }

    if (monsterLoadStep_ == 7)
    {
        if (loadModel_)
        {
            if (loader->GetTaskStatus(tasks_[4]))
            {
                flags81_ &= ~4;
                monsterLoadStep_++;
            }
        }
        else
        {
            monsterLoadStep_++;
        }
    }
    else if (monsterLoadStep_ == 8)
    {
        if (loadModel_)
        {
            SafeAllocator* heap = &heaps_[2];
            heap->Reset();
            void* data;
            u32 size;
            loader->GetLoadedFileByID(tasks_[4], &data, &size);
            if (data)
            {
                GameResources* res = GetGameResources();
                const void* fp;
                u32 fs;
                if (FindFilesInNarcBySubstring(data, data_ov014_02189680, &fp, &fs, 1))
                {
                    u32 cchrSize;
                    void* cchr = DecompressLZ77FileIntoAllocatedSpace(*heap, fp, cchrSize);
                    if (FindFilesInNarcBySubstring(data, data_ov014_02189686, &fp, &fs, 1))
                    {
                        u32 cmotSize;
                        void* cmot = DecompressLZ77FileIntoAllocatedSpace(*heap, fp, cmotSize);
                        const void* bp;
                        u32 bs;
                        if (FindFilesInNarcBySubstring(data, data_ov014_0218968c, &bp, &bs, 1))
                        {
                            model_->Initialize();
                            model_->unknown_2_ = monster_->modelId;
                            model_->EnableFlag(4);
                            func_0207df50(res->unk_2cc);
                            func_0207df90(res->unk_2cc);
                            ObjectArchiveLoadInfo info;
                            func_0203dafc(&info);
                            info.fileData = cchr;
                            info.allocator = heap;
                            info.unk_8 = cchrSize;
                            model_->LoadFromCCHROrCMOTArchive(&info, NULL);
                            func_0207dfac(res->unk_2cc);
                            func_0203dafc(&info);
                            info.allocator = heap;
                            info.fileData = cmot;
                            info.unk_8 = cmotSize;
                            model_->LoadFromCCHROrCMOTArchive(&info, NULL);
                            ApplyPlacement();
                            animIndex_ = 0;
                            MonsterModelInfo* in = func_02097224(monsterData_, monster_->modelId);
                            if (in)
                                model_->MaybeSetRegularAnimation(in->animNames[0], 0);
                            flags81_ |= 4;
                        }
                    }
                }
            }
            loader->RemoveTask(tasks_[4]);
            tasks_[4] = -1;
        }
        monsterLoadStep_ = 0;
        flags81_ &= ~9;
        if (flags82_3_)
            flags82_3_ = 0;
        showSprites_ = nextShowSprites_;
        nextShowSprites_ = 0;
    }
}

// Handler of mode 0: layout, sprites, 3D scene and display registers
void BestiaryViewer::LoadResources()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();

    if (loadStep_ == 0)
    {
        REG_BG1CNT = (u16)((REG_BG1CNT & 0x43) | 0x1f00);
        flags81_ |= 0x10;
        loadStep_++;
    }

    if (loadStep_ == 1)
    {
        func_ov023_021e20c0(layout_);
        layoutTaskId_ = loader->QueueLoadFile(data_ov014_02189692, NULL);
        loadStep_++;
    }

    if (loadStep_ == 2 && loader->GetTaskStatus(layoutTaskId_) != 0)
    {
        void* data;
        unsigned int size;
        loader->GetLoadedFileByID(layoutTaskId_, &data, &size);
        if (data != NULL)
        {
            SafeAllocator* heaps = heaps_;
            heaps[3].Reset();
            func_ov023_021e20f0(layout_, &heaps[3], data, size);
            ScreenLayout::Element* e = layout_->FindElement(1);
            if (e != NULL)
                e->visible = 0;
        }
        loader->RemoveTask(layoutTaskId_);
        layoutTaskId_ = -1;
        layoutTaskId_ = loader->QueueLoadFile(data_ov014_021896a7, NULL);
        loadStep_++;
    }

    if (loadStep_ == 3 && loader->GetTaskStatus(layoutTaskId_) != 0)
    {
        int unk;
        void* data;
        unsigned int size;
        int arg;
        loader->GetLoadedFileByID(layoutTaskId_, &data, &size);
        int count = func_02046900(data);
        heaps_[5].Reset();
        for (int i = 0; i < count; i++)
        {
            void* r = func_020467f0(data, i, &unk, &arg);
            if (r != NULL)
                func_0205a528(spriteBank_, r, arg, &heaps_[5]);
        }

        short x = 0;
        short y = 0;
        ScreenLayout::Element* e;
        Sprite* s;

        e = layout_->FindElement(0x1a);
        if (e != NULL)
            e->visible = 0;
        func_ov023_021e2bdc(layout_, 0x1a, &x, &y);
        s = &sprites_[0];
        s->x = x << 12;
        s->y = y << 12;
        s->unk_22 = 0x21;
        s->unk_26 = 0;

        e = layout_->FindElement(0x1b);
        if (e != NULL)
            e->visible = 0;
        func_ov023_021e2bdc(layout_, 0x1b, &x, &y);
        s = &sprites_[1];
        s->x = x << 12;
        s->y = y << 12;
        s->unk_22 = 0x22;
        s->unk_26 = 0;

        func_ov023_021e2bdc(layout_, 0x14, &x, &y);
        s = &sprites_[2];
        s->x = x << 12;
        s->y = y << 12;
        s->unk_22 = 0x20;
        s->unk_26 = 0;

        loader->RemoveTask(layoutTaskId_);
        layoutTaskId_ = -1;
        loadStep_++;
    }

    if (loadStep_ == 4)
    {
        REG_BG0CNT = (u16)(REG_BG0CNT & ~3);
        REG_BG1CNT = (u16)((REG_BG1CNT & ~3) | 1);
        REG_BG2CNT = (u16)((REG_BG2CNT & ~3) | 2);
        REG_BG3CNT = (u16)((REG_BG3CNT & ~3) | 3);
        REG_DISPCNT = (REG_DISPCNT & ~0x1f00) | 0x1300;

        GameState* gs = GameState::GetInstance();
        data_ov014_02189800.savedScene = func_020100f8(gs);
        func_020a2010(scene_);
        func_0202e5c8(scene_, 0, 0, 0);
        func_0202e5d8(scene_, 0, 0, 0xa000);
        func_0202e0a4(scene_);
        func_020a27a0(scene_);
        func_020100c4(gs, scene_);

        GameResources* res = GetGameResources();
        SetFlags04(res, 0x6093e);
        ClearFlags04(res, 0x20);
        G3X_SetFog(0, 0, 0, 0);
        G3X_SetClearColor(0, 0, 0x7fff, 0, 0);
        LightingManager::GetInstance()->fogEnabled_ = false;
        mode_ = 1;
        loadStep_ = 0;
    }
}

// Handler of mode 1
void BestiaryViewer::UpdateReady()
{
    UpdateMonsterLoad();
}

// Loads bg_smitop.pac (top background shown when no monster is displayed)
void BestiaryViewer::UpdateTopBackgroundLoad()
{
    if (flags81_ & 0x10)
    {
        BackgroundLoader* loader = BackgroundLoader::GetInstance();
        if (topBgLoadStep_ == 0)
        {
            topBgTaskId_ = loader->QueueLoadFile(data_ov014_021896bc, 0);
            topBgLoadStep_++;
        }
    }
}
