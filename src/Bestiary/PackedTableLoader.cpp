#include "Bestiary/PackedTable.h"
#include "Filesystem/BackgroundLoader.h"
#include "std_library_functions.h"

// File loaded by PackedTableLoader
struct Gp2FileRef
{
    const char* innerFile;  // "enchab_<LG>.nat"
    const char* gp2;        // "data/prm/enchab.gp2"
};

extern "C"
{
    // Copies a loaded packed table file into `table` (whole table if key < 0,
    // else only the groups of that key)
    int func_ov014_02188d48(PackedTable* table, SafeAllocator* alloc, void* file, int size, short key);

    extern Gp2FileRef data_ov014_021897c4;
}

// Key getter passed to BinarySearch: the key is the first u16 of an entry
int PackedTable::GetEntryKey(void* entry)
{
    return *(u16*)entry;
}

// Turns every offset of the offsets block into a pointer in the data block
int PackedTable::Relocate(void* arg)
{
    if (this == 0 || arg != 0)
    {
        return 0;
    }
    char* b = (char*)entries_;
    u32* p = (u32*)b + numKeys_ + numGroups_;
    u32* e = (u32*)(b + (((char*)((u16*)p + numIds_) - b + 3) & ~3));
    u16 count = numOffsets_;
    int i = 0;
    while (i < count)
    {
        *e = (u32)ResolveOffset((char*)*e, 0);
        e++;
        i++;
    }
    return 1;
}

// Offset in the data block to pointer, `def` if the offset is -1 or there is no data
char* PackedTable::ResolveOffset(char* offset, char* def)
{
    bool invalid = true;
    int o = offset - (char*)0;
    if (o != -1 && data_ != 0)
    {
        invalid = false;
    }
    if (!invalid)
    {
        def = data_ + o;
    }
    return def;
}

void PackedTableLoader::Init()
{
    memset(&table_, 0, 0x14);
    key_ = -1;
    taskId_ = -1;
    Cancel();
}

void PackedTableLoader::Start(SafeAllocator* alloc, short key)
{
    Cancel();
    Init();
    key_ = key;
    allocator_ = alloc;
    step_ = 1;
    Update();
}

// Returns 0 while the file is loading
int PackedTableLoader::Update()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    switch (step_)
    {
    case 1:
        taskId_ = loader->QueueLoadFileInGP2(data_ov014_021897c4.gp2, data_ov014_021897c4.innerFile, 0);
        step_ = 2;
        return Update();
    case 2:
        if (loader->GetTaskStatus(taskId_) == 0)
            return 0;
        step_ = 3;
        return Update();
    case 3:
    {
        void* data = 0;
        unsigned int size = 0;
        loader->GetLoadedFileByID(taskId_, &data, &size);
        func_ov014_02188d48(&table_, allocator_, data, size, key_);
        Cancel();
        return 1;
    }
    }
    return 1;
}

int PackedTableLoader::Cancel()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (taskId_ >= 0)
    {
        loader->RemoveTask(taskId_);
        taskId_ = -1;
    }
    step_ = 0;
    allocator_ = 0;
    return 1;
}
