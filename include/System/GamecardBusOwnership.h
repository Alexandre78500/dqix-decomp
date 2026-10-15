#pragma once

struct GamecardBusLock
{
    volatile unsigned int atomic;
    unsigned short ownerID;
    unsigned short unknown_6;
};

//void OS_InitLock();

extern "C" int OS_UnLockCartridge(unsigned short owner);
extern "C" int OS_TryLockCartridge(unsigned short owner);

int AcquireNDSBus(unsigned short owner);
int ReleaseNDSBus(unsigned short owner);

extern "C" unsigned int OS_GetLockID();
// parameter might be unsigned int
void ReleaseLockOwnerID(unsigned short id);