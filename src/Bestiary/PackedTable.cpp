#include "Bestiary/PackedTable.h"
#include "std_library_functions.h"

// Calls `visit` on every entry of the keys block
int PackedTable::ForEach(Visitor visit)
{
    u16 count;
    u32* e = entries_;
    if (e == 0 || (count = numKeys_) == 0 || visit == 0)
    {
        return 0;
    }
    for (int i = 0; i < count; i++, e++)
    {
        visit(this, e);
    }
    return 1;
}

// Uses the blocks of a file already in memory, without copying them. The
// file is visited only once: `alreadyLoaded` tells whether it was before.
int PackedTable::Attach(PackedTable* file, u8* alreadyLoaded, Visitor visit)
{
    *alreadyLoaded = 0;
    if (file == 0)
    {
        return 0;
    }
    memcpy(this, file, 0xc);
    entries_ = (u32*)((char*)file + 0xc);
    data_ = (char*)file + (GetEntriesSize() + 0xc);
    if (loaded_)
    {
        *alreadyLoaded = 1;
        return 1;
    }
    ForEach(visit);
    loaded_ = 1;
    file->loaded_ = 1;
    return 1;
}

// Binary search in the keys block, NULL if the key is missing
u32* PackedTable::BinarySearch(int key, KeyGetter getKey)
{
    u32* entries = entries_;
    if (entries == NULL || getKey == NULL)
    {
        return NULL;
    }
    u16 count = numKeys_;
    if (count == 0)
    {
        return NULL;
    }
    int lo;
    int hi;
    hi = count - 1;
    lo = 0;
    while (lo <= hi)
    {
        int mid = lo + ((hi - lo + 1) >> 1);
        u32* e = &entries[mid];
        int k = getKey(e);
        if (k == key)
        {
            return e;
        }
        if (k > key)
        {
            hi = mid - 1;
        }
        else
        {
            lo = mid + 1;
        }
    }
    return NULL;
}

// Size in bytes of the entries block
int PackedTable::GetEntriesSize()
{
    int s = numGroups_ * 4 + numIds_ * 2 + numKeys_ * 4;
    int a = (s + 3) & ~3;
    int t = numOffsets_ * 4 + (a - numKeys_ * 4);
    t += numKeys_ * 4;
    return t;
}

u16* PackedTable::GetIds()
{
    return (u16*)(entries_ + numKeys_ + numGroups_);
}

u32* PackedTable::GetOffsets()
{
    char* base = (char*)entries_;
    u16* p = GetIds();
    return (u32*)(base + (((char*)(p + numIds_) - base + 3) & ~3));
}

// Keys block if key < 0
PackedTable::Key* PackedTable::FindKey(int key)
{
    if (key < 0)
    {
        return (Key*)entries_;
    }
    return (Key*)BinarySearch(key, GetEntryKey);
}

PackedTable::Group* PackedTable::GetGroups(Key* key)
{
    if (key == NULL)
        return NULL;
    return (Group*)(entries_ + numKeys_ + key->firstGroup);
}

PackedTable::Group* PackedTable::GetGroup(int index, Key* key)
{
    if (key == NULL)
    {
        return NULL;
    }
    return GetGroups(key) + index;
}

u32* PackedTable::GetGroupOffset(int index, Key* key)
{
    if (!key)
        return NULL;
    Group* g = GetGroup(index, key);
    if (!g)
        return NULL;
    return GetOffsets() + g->offsetIndex;
}
