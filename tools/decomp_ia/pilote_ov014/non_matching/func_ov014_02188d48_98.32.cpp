#include "Bestiary/PackedTable.h"
#include "globaldefs.h"
#include "std_library_functions.h"

struct KeyEntry {
    u16 key;
    u16 index : 12;
    u16 count : 4;
};

struct SubEntry {
    u16 word;
    u16 flag : 1;
    u16 index : 11;
    u16 count : 4;
};

extern "C" char *func_0205ec34();
extern "C" int func_0206dfb0(void *, void *, int);
extern "C" void *func_02012fe4();
extern "C" u32 func_020da150(SafeAllocator *alloc, u32 p);
extern "C" bool func_0201b588(u16 id);
extern "C" bool func_0201bb78(void *zone, u16 id);

extern "C" int func_ov014_02188d48(PackedTable *self, SafeAllocator *alloc, void *src, int size, short id) {
    int entSize;
    u32 dataSize;
    SubEntry *e;
    if (alloc == 0 || src == 0 || size == 0) {
        return 0;
    }
    if (id < 0) {
        if (alloc != 0 && src != 0) {
            memcpy(self, src, 0xc);
            entSize  = self->GetEntriesSize();
            dataSize = self->dataSize_;
            if (entSize != 0) {
                self->entries_ = (u32 *) alloc->Allocate(entSize);
            } else {
                self->entries_ = 0;
            }
            if (dataSize != 0) {
                self->data_ = (char *) alloc->Allocate(dataSize);
            } else {
                self->data_ = 0;
            }
            if (self->entries_ != 0) {
                memcpy(self->entries_, (char *) src + 0xc, entSize);
            }
            if (self->data_ != 0) {
                memcpy(self->data_, (char *) src + (self->GetEntriesSize() + 0xc), dataSize);
            }
            self->loaded_ = 1;
        }
        self->Relocate(0);
    } else {
        int c;
        u16 *p;
        u32 *outWords;
        PackedTable tmp;
        memset(&tmp, 0, 0x14);
        u8 already;
        tmp.Attach((PackedTable *) src, &already, 0);
        KeyEntry *key = (KeyEntry *) tmp.BinarySearch(id, PackedTable::GetEntryKey);
        if (key != 0) {
            char *fm = func_0205ec34();
            int flag;
            if (func_0206dfb0(fm, fm + 0x8c, 0x79d)) {
                flag = 1;
            } else {
                flag = 0;
            }
            void *zone = func_02012fe4();

            u32 *keys  = tmp.entries_ + tmp.numKeys_;
            u16 *ids   = (u16 *) (keys + tmp.numGroups_);
            e          = (SubEntry *) (keys + key->index);
            entSize    = key->count;
            u32 *words = (u32 *) ((char *) tmp.entries_ + (((char *) (ids + tmp.numIds_) - (char *) tmp.entries_ + 3) & ~3));

#define cnt entSize
            int total   = 0;
            SubEntry *s = e;
            for (int i = 0; i < cnt; i++, s++) {
                total += s->count;
            }

            PackedTable out;
            out.numKeys_    = 1;
            u16 n           = cnt;
            out.numGroups_  = n;
            out.numIds_     = total;
            out.numOffsets_ = n;
            out.dataSize_   = 0;
            u32 *buf        = (u32 *) alloc->Allocate(out.GetEntriesSize());
            out.entries_    = buf;
            out.data_       = 0;
            if (buf != 0) {
                SubEntry *outSubs = (SubEntry *) (buf + 1);
                u16 *outIds       = (u16 *) (outSubs + cnt);
                outWords          = (u32 *) ((char *) buf + (((char *) (outIds + total) - (char *) buf + 3) & ~3));
                memcpy(buf, key, 4);
                memcpy(outSubs, e, cnt * 4);
                ((KeyEntry *) buf)->index = 0;
                int off                   = 0;
                int j                     = 0;
                e                         = outSubs;
                for (int i = 0; i < cnt; i++) {
                    c = e->count;
                    memcpy(outIds, ids + e->index, c * 2);
                    memcpy(outWords, words + e->word, 4);
                    e->word   = j;
                    e->index  = (u16) off;
                    *outWords = func_020da150(alloc, (u32) tmp.ResolveOffset((char *) *outWords, 0));
                    bool ok   = (c == 0);
                    p         = outIds;
                    for (int k = 0; k < c; k++, p++) {
                        ok = (func_0201b588(*p) && flag) || func_0201bb78(zone, *p);
                        if (ok) {
                            break;
                        }
                    }
                    e->flag = ok;
                    outIds += c;
                    off += c;
                    outWords++;
                    e++;
                    j++;
                }
                int swapped;
                do {
                    swapped = 0;
                    e       = outSubs;
                    for (int i = 0; i < cnt - 1; i++, e++) {
                        if (!e[0].flag && e[1].flag) {
                            SubEntry t;
                            memcpy(&t, e, 4);
                            memcpy(e, e + 1, 4);
                            memcpy(e + 1, &t, 4);
                            swapped = 1;
                        }
                    }
                } while (swapped);
            } else {
                out.numKeys_    = 0;
                out.numGroups_  = 0;
                out.numIds_     = 0;
                out.numOffsets_ = 0;
            }
            memcpy(self, &out, 0x14);
        }
    }
    return 1;
}
