#include "Filesystem/CardReadManager.h"
#include "System/BiosData.h"
#include "System/Interrupts.h"
#include "System/Cache.h"
#include "System/IPC.h"
#include "System/Memory.h"
#include "Filesystem/FSInnerDefs.h"
#include <asmhacks.h>

#pragma optimize_for_size off

#if defined(jpn)
#define MIi_CpuClearFast func_020cbf24
#define OS_Terminate func_020cb6ac

#define data_02111860 data_02111500
#endif

extern "C"
{
    // memset but tries to do 32 bytes at a time
    void MIi_CpuClearFast(int value, void* dst, unsigned len);

    // abort() or fatal error or similar
    void OS_Terminate();
}
// is card read manager initialized
extern int data_02111860;

void MarkCardReadManagerInitialized(int to);

void SendTaskToReadContext(CardReadManager::ReadProc task)
{
    CardReadManager* readManager = (CardReadManager*)&cardi_common;
    ChangeContextPriority(&readManager->cartridgeReadContext, cardi_common.contextPriority_108);
    readManager->currentTaskExecutionContext = &readManager->cartridgeReadContext;
    readManager->cartridgeReadProc = task;
    readManager->flags |= (1 << READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING);
    MarkContextReadyAndSwitch(&readManager->cartridgeReadContext);
}

void LockCardReadManager(unsigned short ownerID, int taskType)
{
    CardReadManager* manager = &cardi_common;
    volatile CardReadManager* volMan = manager;
    // here we access owner & multiplicity via volatile, but 
    // owner & taskType works too.
    int priorState = DisableIRQInterrupts();
    if (volMan->lock.owner == ownerID)
    {
        if (manager->lock.taskType != taskType)
            OS_Terminate();
    }
    else
    {
        while (volMan->lock.owner != -3)
            BlockCurrentContext(&manager->lock.waitingContexts);
        volMan->lock.owner = ownerID;
        manager->lock.taskType = taskType;
    }
    
    volMan->lock.multiplicity++;
    manager->pSharedData->unknown_0 = 0;
    SetIRQInterruptState(priorState);
}

void UnlockCardReadManager(unsigned short ownerID, int taskType)
{
    CardReadManager* manager = &cardi_common;
    volatile CardReadManager* volMan = manager;
    int priorState = DisableIRQInterrupts();
    if (volMan->lock.owner != ownerID || volMan->lock.multiplicity == 0)
    {
        OS_Terminate();
    }
    else
    {
        if (manager->lock.taskType != taskType)
            OS_Terminate();
        int multiplicity = volMan->lock.multiplicity;
        multiplicity--;
        volMan->lock.multiplicity = multiplicity;
        if (multiplicity == 0)
        {
            volMan->lock.owner = -3;
            manager->lock.taskType = 0;
            UnblockContexts(&manager->lock.waitingContexts);
        }
    }
    
    manager->pSharedData->unknown_0 = 0;
    SetIRQInterruptState(priorState);
}

void InitializeCardReadManager()
{
    CardReadManager* manager = &cardi_common;
    volatile CardReadManager* volatileManager = manager;

    // two of these need to be volatile and definitely owner
    volatileManager->lock.owner = -3;
    volatileManager->lock.multiplicity = 0;
    manager->lock.taskType = 0;
    manager->pSharedData = &data_02111880;
    
    MIi_CpuClearFast(0, &data_02111880, sizeof(Arm7CardReadData));
    CleanInvalidateCacheRange(&data_02111880, sizeof(Arm7CardReadData));
    manager->instructionCacheCleanThreshold = 0xffffffff;
    manager->dataCacheCleanThreshold = 0xffffffff;
    
    if (!IsDownloadPlay())
        MI_CpuCopy8((void*)BIOS_ADDR_CARTRIDGE_HEADER, (void*)0x027ffa80, 0x160);
    
    manager->contextPriority_108 = 4;
    manager->lock.waitingContexts.last = 0;
    manager->lock.waitingContexts.first = 0;
    manager->ongoingReadBlock.last = 0;
    manager->ongoingReadBlock.first = 0;

    PopulateContext(&manager->cartridgeReadContext, (unsigned int)&CartridgeReadContextLoop, 0, (unsigned int)(&cardi_common + 1), 0x400, manager->contextPriority_108);
    MarkContextReadyAndSwitch(&manager->cartridgeReadContext);
    SetArm9IPCCommandHandler(11, &IPCCommand11Proc);

    if (IsDownloadPlay())
    {
        DECLARE_ASM_NOP();
        return;
    }
    MarkCardReadManagerInitialized(true);    
}

int IsCardReadManagerInitialized()
{
    return data_02111860;
}

void VerifyCardReadManagerInitialized()
{
    if (!IsCardReadManagerInitialized())
        OS_Terminate();
}

void MarkCardReadManagerInitialized(int to)
{
    data_02111860 = to;
}

bool AwaitCardReadManagerIdle()
{
    CardReadManager* manager = &cardi_common;
    int priorState = DisableIRQInterrupts();
    while (manager->flags & (1 << READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS))
        BlockCurrentContext(&manager->ongoingReadBlock);
    SetIRQInterruptState(priorState);
    return manager->pSharedData->unknown_0 == 0;
}

bool IsCardReadManagerIdle()
{
    CardReadManager* manager = &cardi_common;
    return !(manager->flags & (1 << READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS));
}

int GetCardReadManagerSharedStatus()
{
    CardReadManager* manager = &cardi_common;
    return manager->pSharedData->unknown_0;
}