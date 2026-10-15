#pragma once

#include "../Memory/SafeAllocator.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Table packed in a single file (e.g. enchab_<LG>.nat). The file starts with
// the first 0xc bytes of this class, followed by the entries block
// (GetEntriesSize() bytes) and the data block (dataSize_ bytes).
//
// Entries block:
//   Key   keys[numKeys_]        sorted by key
//   Group groups[numGroups_]
//   u16   ids[numIds_]
//   (aligned to 4)
//   u32   offsets[numOffsets_]  offsets in the data block, relocated to
//                               pointers by Relocate()
// sizeof == 0x14.
class PackedTable
{
public:
    // sizeof == 0x4
    struct Key
    {
        u16 key;
        u16 firstGroup : 12;    // index in groups
        u16 numGroups : 4;
    };

    // sizeof == 0x4
    struct Group
    {
        u16 offsetIndex;        // index in offsets
        u16 enabled : 1;
        u16 firstId : 11;       // index in ids
        u16 numIds : 4;
    };

    typedef int (*KeyGetter)(void* entry);
    typedef void (*Visitor)(PackedTable* table, u32* entry);

    u16 numKeys_;           // 0x0
    u16 numGroups_;         // 0x2
    u16 numIds_;            // 0x4
    u16 numOffsets_;        // 0x6
    u32 dataSize_ : 31;     // 0x8
    u32 loaded_ : 1;        // set once the offsets have been visited
    u32* entries_;          // 0xc
    char* data_;            // 0x10

    static int GetEntryKey(void* entry);
    int Relocate(void* arg);
    char* ResolveOffset(char* offset, char* def);

    int ForEach(Visitor visit);
    int Attach(PackedTable* file, u8* alreadyLoaded, Visitor visit);
    u32* BinarySearch(int key, KeyGetter getKey);
    int GetEntriesSize();
    u16* GetIds();
    u32* GetOffsets();
    Key* FindKey(int key);
    Group* GetGroups(Key* key);
    Group* GetGroup(int index, Key* key);
    u32* GetGroupOffset(int index, Key* key);
};

// Loads a packed table from data/prm/enchab.gp2 in the background, keeping
// either the whole table or only the entries of one key. sizeof == 0x20.
class PackedTableLoader
{
public:
    PackedTable table_;         // 0x0
    short key_;                 // 0x14: -1 = keep the whole table
    short step_;                // 0x16: 0 = idle
    int taskId_;                // 0x18
    SafeAllocator* allocator_;  // 0x1c

    void Init();
    void Start(SafeAllocator* alloc, short key);
    int Update();
    int Cancel();
};
