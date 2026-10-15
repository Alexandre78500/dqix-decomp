#include "globaldefs.h"
#include "Filesystem/BackgroundLoader.h"
#include "Bestiary/BestiaryViewer.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct MonsterRef {
    u8 pad_0[0x8];
    char* name;      // 0x8
    short id;        // 0xc
    u8 pad_e[0x12 - 0xe];
    signed char unk_12;
    u8 pad_13[0x1a - 0x13];
    short no;        // 0x1a
};

struct RecordWord {
    u32 count : 10;
    u32 flag : 1;
    u32 f1 : 7;
    u32 f2 : 7;
    u32 f3 : 7;
};

struct UiEntry {
    u8 pad_0[0x16];
    u8 flags;
};

struct UiScreen {
    u8 pad_0[0x4];
    void* unk_4;
    u8 pad_8[0x12 - 0x8];
    short unk_12;
};

struct Pair9c {
    int value;
    short count;
    u16 extra;
    u16 GetExtra() { return extra; }
};

struct TableKey {
    u16 unk_0;
    u16 index : 12;
    u16 count : 4;
};

struct TableEntry {
    u16 index;
    u16 flag : 1;
    u16 rest : 15;
};

struct Builder3a0 {
    u8 pad_0[0x8];
    void* unk_8;
    u8 pad_c[0xa8 - 0xc];
    short unk_a8;
    short unk_aa;
    u8 pad_ac[0xb4 - 0xac];
    short unk_b4;
    short unk_b6;
    u8 pad_b8[0xe0 - 0xb8];
};

struct Packer {
    u8 pad_0[0x1c];
    u8 lo : 4;
    u8 hi : 4;
    u8 pad_1d[3];
};

struct Bestiary {
    void** buffers;   // 0x0
    u8 pad_4[0xc - 0x4];
    u8 unk_c[0x10];   // 0xc
    u8 slot[0x2c];    // 0x1c
    MonsterRef* cur;  // 0x48
    void* prev;       // 0x4c
    RecordWord* rec; // 0x50
    void* unk_54;
    ScreenLayout* ui; // 0x58
    void* text;       // 0x5c
    void* unk_60;
    void* unk_64;
    char* unk_68;
    int unk_6c;
    int unk_70;
    u8 pad_74[0x7e - 0x74];
    u8 unk_7e;
    u8 unk_7f;
    u8 unk_80;
    union {
        u8 flags81;
        struct {
            u8 f81_0 : 1;
            u8 f81_1 : 1;
            u8 f81_2 : 1;
            u8 f81_3 : 1;
            u8 f81_4 : 1;
            u8 f81_5 : 1;
            u8 f81_6 : 2;
        };
    };
    u8 pad_82[0x86 - 0x82];
    short unk_86[5];
    u8 pad_90[3];
    u8 unk_93;
    u8 unk_94;
    u8 pad_95[0x9c - 0x95];
    Pair9c unk_9c;
};

extern "C"
{
    int func_02046900(void* data);
    void* func_020467f0(void* data, int index, void** outPtr, u32* outSize);
    void func_0204c684(Builder3a0*);
    void* func_02097224(void*, int);
    void __clear(void*, u32);
    void func_0206819c(void*, char*, int);
    void* func_020421a0();
    void func_02046608(void*, int, const char*, char*, int, int, int);
    void func_ov023_021e24b0(ScreenLayout*, int, int, int, int, int, int, int, int);
    void func_ov023_021e23d0(ScreenLayout*, int, const char*, int, int);
    const char* func_020e0434(void*, int);
    void func_ov023_021e257c(ScreenLayout*);
    void func_0204af64(Packer*);
    void func_0204b11c(Packer*, int);
    void func_0204b5b4(Packer*, int);
    void func_0204b5e8(Packer*, int, int);
    void* memcpy(void* dst, const void* src, unsigned int length);
    void func_0204b4c0(Packer*, void*);
    void func_0204b2e0(Packer*, void*);
    void func_0204b3a0(Packer*, void*);
}

extern "C" void func_ov014_02185c90(Bestiary* self)
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if ((self->flags81 & 1) && self->unk_7f == 5 && loader->GetTaskStatus(self->unk_86[3])) {
        void* ptr;
        void* data;
        u32 size;
        u32 len;
        loader->GetLoadedFileByID(self->unk_86[3], &data, &size);
        int count = func_02046900(data);
        if (count != 0) {
            void* file = func_020467f0(data, 0, &ptr, &len);
            if (file != NULL) {
                Builder3a0 builder;
                func_0204c684(&builder);
                builder.unk_a8 = 0x20;
                builder.unk_aa = 0x18;
                builder.unk_b4 = 0xa;
                builder.unk_b6 = 0xb;
                builder.unk_8 = (char*)file + 0x10;
                func_02097224(self->unk_c, self->cur->id);
                char name1[0x48];
                char name2[0x48];
                char desc[0x200];
                char str1[0x48];
                char str2[0x48];
                __clear(name1, 0x48);
                __clear(name2, 0x48);
                __clear(desc, 0x200);
                __clear(str1, 0x48);
                __clear(str2, 0x48);
                char* p1 = str1;
                char* p2 = str2;
                char* p3 = desc;
                func_0206819c(self->unk_60, name1, 0);
                func_0206819c(self->unk_64, name2, 0);
                void* conv = func_020421a0();
                func_02046608(conv, 0xa, name1, str1, 0x400, 0, 0);
                func_02046608(conv, 0xa, name2, str2, 0x400, 0, 0);
                func_02046608(conv, 0xa, self->unk_68, desc, 0x8c, 0, 0);

                u8 v80;
                ScreenLayout* ui;
                void* text;
                RecordWord* rec;
                MonsterRef* ref;
                v80 = self->unk_80;
                ui = self->ui;
                rec = self->rec;
                text = self->text;
                ref = self->cur;
                if (ui != NULL && ref != NULL && text != NULL && rec != NULL && rec->count != 0) {
                    UiEntry* e;
                    func_ov023_021e24b0(ui, 2, ref->no, 8, 0xf, 1, 3, 0, 1);
                    func_ov023_021e23d0(ui, 3, ref->name, 0xa, 0xf);
                    func_ov023_021e23d0(ui, 4, func_020e0434(text, ref->unk_12), 0xa, 0xf);
                    e = (UiEntry*)ui->FindElement(4);
                    if (e) e->flags |= 0x10;
                    if (&self->unk_9c != NULL && self->unk_9c.count > 0) {
                        func_ov023_021e24b0(ui, 5, self->unk_9c.value, 8, 0xf, 1, 6, 0, 0);
                        func_ov023_021e24b0(ui, 6, self->unk_9c.GetExtra(), 8, 0xf, 1, 6, 0, 0);
                    }
                    func_ov023_021e24b0(ui, 7, rec->count, 8, 0xf, 1, 6, 0, 0);
                    e = (UiEntry*)ui->FindElement(8);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0xa);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0xc);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(9);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0xb);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0xd);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x16);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x14);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x1a);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x1b);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x17);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x18);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x19);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x15);
                    if (e) e->flags |= 1;

                    int idC;
                    int idB;
                    int idA;
                    int shown;
                    shown = 0;
                    idA = 8;
                    idB = 0xa;
                    idC = 0xc;
                    if (rec->flag || rec->count) {
                        shown = 1;
                        if (*p1 != 0) {
                            e = (UiEntry*)ui->FindElement(idA);
                            if (e) e->flags |= 1;
                            e = (UiEntry*)ui->FindElement(idB);
                            if (e) e->flags |= 1;
                            e = (UiEntry*)ui->FindElement(idC);
                            if (e) e->flags |= 1;
                            func_ov023_021e23d0(ui, idA, p1, 0xa, 0xf);
                            func_ov023_021e24b0(ui, idB, rec->f1, 8, 0xf, 1, 3, 0, 0);
                            func_ov023_021e23d0(ui, idC, func_020e0434(text, 0xf), 0xa, 0xf);
                            idA = 9;
                            idB = 0xb;
                            idC = 0xd;
                        }
                    }
                    if (rec->f2 || rec->flag) {
                        if (*p2 != 0) {
                            shown = 1;
                            e = (UiEntry*)ui->FindElement(idA);
                            if (e) e->flags |= 1;
                            e = (UiEntry*)ui->FindElement(idB);
                            if (e) e->flags |= 1;
                            e = (UiEntry*)ui->FindElement(idC);
                            if (e) e->flags |= 1;
                            func_ov023_021e23d0(ui, idA, p2, 0xa, 0xf);
                            func_ov023_021e24b0(ui, idB, rec->f2, 8, 0xf, 1, 3, 0, 0);
                            func_ov023_021e23d0(ui, idC, func_020e0434(text, 0xf), 0xa, 0xf);
                        }
                    }
                    if (!shown) {
                        e = (UiEntry*)ui->FindElement(0x16);
                        if (e) e->flags |= 1;
                        const char* none = func_020e0434(text, 0xe);
                        func_ov023_021e23d0(ui, 0x16, none, 0xa, 0xf);
                    }
                    func_ov023_021e23d0(ui, 0x13, p3, 0xa, 0xf);
                    if (rec->flag) {
                        e = (UiEntry*)ui->FindElement(0x14);
                        if (e) e->flags &= ~1;
                        e = (UiEntry*)ui->FindElement(0x17);
                        if (e) e->flags |= 1;
                        func_ov023_021e23d0(ui, 0x17, func_020e0434(text, 0x1c), 8, 0xf);
                        e = (UiEntry*)ui->FindElement(0x18);
                        if (e) e->flags |= 1;
                        func_ov023_021e24b0(ui, 0x18, v80 + 1, 8, 0xf, 0, 1, 0, 0);
                        e = (UiEntry*)ui->FindElement(0x19);
                        if (e) e->flags |= 1;
                        func_ov023_021e24b0(ui, 0x19, 2, 8, 0xf, 0, 1, 0, 0);
                    }
                    e = (UiEntry*)ui->FindElement(0xe);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0xf);
                    if (e) e->flags &= ~1;
                    e = (UiEntry*)ui->FindElement(0x10);
                    if (e) e->flags &= ~1;
                    int i;
                    int id;
                    PackedTable* table = (PackedTable*)self->slot;
                    PackedTable::Key* key = table->FindKey(-1);
                    if (key != NULL) {
                        id = 0xe;
                        for (i = 0; i < 2; i++) {
                            if (i < key->numGroups) {
                                e = (UiEntry*)ui->FindElement(id);
                                if (e) e->flags |= 1;
                                PackedTable::Group* te = table->GetGroup(i, key);
                                u32* str = table->GetGroupOffset(i, key);
                                if (te->enabled) {
                                    func_ov023_021e23d0(ui, id, (const char*)*str, 0xa, 0xf);
                                } else {
                                    func_ov023_021e23d0(ui, id, func_020e0434(text, 0x15), 0xa, 0xf);
                                }
                                id = (short)(id + 1);
                            }
                        }
                        if (key->numGroups >= 3) {
                            PackedTable::Group* te = table->GetGroup(2, key);
                            if (te->enabled) {
                                e = (UiEntry*)ui->FindElement(0x10);
                                if (e) e->flags |= 1;
                                func_ov023_021e23d0(ui, 0x10, func_020e0434(text, 0x10), 0xa, 0xf);
                            }
                        }
                    }
                }
                UiScreen* u = (UiScreen*)self->ui;
                u->unk_4 = &builder;
                u->unk_12 = 1;
                func_ov023_021e257c(self->ui);

                Packer packer;
                func_0204af64(&packer);
                func_0204b11c(&packer, 0);
                packer.lo = 0;
                packer.hi = 1;
                func_0204b5b4(&packer, 1);
                func_0204b5e8(&packer, 0, 0);
                for (int i = 0; i < count; i++) {
                    void* p = func_020467f0(data, i, &ptr, &len);
                    if (p != NULL) {
                        memcpy(self->buffers[i], p, len);
                        func_0204b4c0(&packer, self->buffers[i]);
                    }
                }
                self->f81_5 = 0;
            }
        }
        loader->RemoveTask(self->unk_86[3]);
        self->unk_86[3] = -1;
        self->unk_7f++;
    }

    if ((self->flags81 & 0x10) && self->unk_7e == 1 && loader->GetTaskStatus(self->unk_70)) {
        void* ptr;
        void* data;
        u32 size;
        u32 len;
        loader->GetLoadedFileByID(self->unk_70, &data, &size);
        int count = func_02046900(data);
        Packer packer;
        func_0204af64(&packer);
        func_0204b11c(&packer, 0);
        packer.lo = 0;
        packer.hi = 1;
        func_0204b5b4(&packer, 1);
        func_0204b5e8(&packer, 0, 0);
        for (int i = 0; i < count; i++) {
            void* p = func_020467f0(data, i, &ptr, &len);
            if (p != NULL) {
                func_0204b2e0(&packer, p);
                func_0204b3a0(&packer, p);
            }
        }
        loader->RemoveTask(self->unk_70);
        self->unk_70 = -1;
        self->unk_7e = 0;
        self->f81_4 = 0;
        self->unk_94 = self->unk_93;
        self->unk_93 = 0;
    }
}
