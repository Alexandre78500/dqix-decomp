#include "System/ProcessorContext.h"
#include "System/Mutex.h"
#include "System/Timing.h"
#include "System/Interrupts.h"
#include <globaldefs.h>
#include <asmhacks.h>

#pragma dont_inline on
#pragma optimize_for_size off

#if defined(jpn)
#define OS_Halt func_020cb6bc
#define OS_Terminate func_020cb6ac
#define MIi_CpuClear32 func_020cbeb8
#endif

extern "C"
{
    // wait for an interrupt
    void OS_Halt();

    // something like abort()
    void OS_Terminate();

    // memset with silly signature and assuming alignment
    void MIi_CpuClear32(int value, void* dst, unsigned len);
}

void OS_CreateThread(ProcessorContext *context, unsigned int startAddress,
    unsigned int userdata, unsigned int stackBottom,
    unsigned int stackSize, unsigned int priority)
{
    int priorState = OS_DisableInterrupts();
    int uniqueID = GenerateUniqueContextID();
    context->priority = priority;
    context->uniqueID = uniqueID;
    context->unknown_74 = context->blockState = 0; // CONTEXT_STATE_BLOCKED

    InsertContextIntoGlobalList(context);

    context->stackBottom = stackBottom;
    context->stackTop = stackBottom - stackSize;
    context->stackUnknownTopSubspaceSize = 0;
    *(int*)(context->stackBottom - 4) = STACK_BOTTOM_MAGIC;
    *(int*)(context->stackTop) = STACK_TOP_MAGIC;
    context->contextsAwaitingThisCompletion.first = context->contextsAwaitingThisCompletion.last = NULL;

    InitializeContextRegisters(context, startAddress, stackBottom - 4);
    context->userModeRegisters[0] = userdata;
    // when the function at startAddress returns, go here
    context->userModeRegisters[14] = (unsigned int)&OS_ExitThread;

    MIi_CpuClear32(0, (void*)(stackBottom - stackSize + 4), stackSize - 8);
    context->blockingMutex = NULL;
    context->lockedMutexes.pFirst = NULL;
    context->lockedMutexes.pLast = NULL;
    SetContextEndProc(context, NULL);

    context->containerBlockedQueue = NULL;
    context->pNextBlocked = NULL;
    context->pPrevBlocked = NULL;
    MIi_CpuClear32(0, &context->unknown_A4, 12);
    context->sleepAlarm = NULL;

    OS_RestoreInterrupts(priorState);
}

void OS_ExitThread()
{
    OS_DisableInterrupts();
    ExitContext(data_021112e0.substruct_24.activeContext, 0);
}

void ExitContext(ProcessorContext *context, int exitCode)
{
    if (data_021112e0.unknown_1C != 0)
    {
        // context will resume execution with pc at ExitCurrentContext invocation
        // and with IRQ interrupts disabled.
        InitializeContextRegisters(context, (unsigned int)&ExitCurrentContext, data_021112e0.unknown_1C);
        context->userModeRegisters[0] = exitCode;
        context->programStatusRegister |= (1 << 7); // disable IRQ interrupts here
        context->blockState = 1;
        OS_LoadContext(context);
    }
    else
    {
        ExitCurrentContext(exitCode);
    }
}

void ExitCurrentContext(int code)
{
    ProcessorContext* context = *data_021112e0.ppActiveContext;
    ProcessorContext::ExitRoutine proc = context->exitProc;
    if (proc != NULL)
    {
        context->exitProc = NULL;
        proc(code);
        OS_DisableInterrupts();
    }
    ShutdownCurrentContext();
}

void ShutdownCurrentContext()
{
    ProcessorContext* context = *data_021112e0.ppActiveContext;
    OS_DisableScheduler();
    UnlockAllMutexesLockedByContext(context);
    if (context->containerBlockedQueue != NULL)
        context->containerBlockedQueue->Remove(context);

    RemoveContextFromGlobalList(context);
    context->blockState = CONTEXT_STATE_INVALID;
    OS_WakeupThread(&context->contextsAwaitingThisCompletion);
    OS_EnableScheduler();
    OS_RescheduleThread();
    OS_Terminate();
}

void OS_DestroyThread(ProcessorContext* context)
{
    int priorState = OS_DisableInterrupts();

    if (data_021112e0.substruct_24.activeContext == context)
        ShutdownCurrentContext(); // function call will not return

    OS_DisableScheduler();
    UnlockAllMutexesLockedByContext(context);
    CancelContextSleepAlarm(context);
    if (context->containerBlockedQueue != NULL)
        context->containerBlockedQueue->Remove(context);

    RemoveContextFromGlobalList(context);
    context->blockState = CONTEXT_STATE_INVALID;
    OS_WakeupThread(&context->contextsAwaitingThisCompletion);
    OS_EnableScheduler();
    OS_RestoreInterrupts(priorState);

    OS_RescheduleThread();
}

void CancelContextSleepAlarm(ProcessorContext *context)
{
    if (context->sleepAlarm == NULL)
        return;

    OS_CancelAlarm(context->sleepAlarm);
}

void OS_JoinThread(ProcessorContext *context) 
{
    int priorState = OS_DisableInterrupts();
    if (context->blockState != CONTEXT_STATE_INVALID)
        OS_SleepThread(&context->contextsAwaitingThisCompletion);
    OS_RestoreInterrupts(priorState);
}

extern "C" bool OS_IsThreadTerminated(ProcessorContext* context)
{
    return context->blockState == CONTEXT_STATE_INVALID;
}

void OS_SleepThread(BlockedContextList* blockQueue)
{
    int priorState = OS_DisableInterrupts();

    ProcessorContext* current = *data_021112e0.ppActiveContext;
    if (blockQueue != NULL)
    {
        current->containerBlockedQueue = blockQueue;
        blockQueue->Insert(current);
    }
    current->blockState = CONTEXT_STATE_BLOCKED;
    SwitchContext();
    OS_RestoreInterrupts(priorState);
}

void OS_WakeupThread(BlockedContextList* blockQueue)
{
    int priorState = OS_DisableInterrupts();
    if (blockQueue->first != NULL)
    {
        // wtf?
        if (blockQueue->first != NULL)
        {
            do
            {
                ProcessorContext* context = blockQueue->PopFront();
                context->blockState = CONTEXT_STATE_READY;
                context->containerBlockedQueue = NULL;
                context->pNextBlocked = NULL;
                context->pPrevBlocked = NULL;
            } while (blockQueue->first != NULL);
        }
        blockQueue->last = NULL;
        blockQueue->first = NULL;
        SwitchContext();
    }
    OS_RestoreInterrupts(priorState);
}

void OS_WakeupThreadDirect(ProcessorContext* context)
{
    int priorState = OS_DisableInterrupts();
    context->blockState = CONTEXT_STATE_READY;
    SwitchContext();
    OS_RestoreInterrupts(priorState);
}

ProcessorContext* GetFirstReadyContext()
{
    ProcessorContext* context = data_021112e0.substruct_24.firstContext;
    while (context != NULL && context->blockState != CONTEXT_STATE_READY)
    {
        context = context->pNext;
    }
    return context;
}

void OS_RescheduleThread()
{
    int priorState = OS_DisableInterrupts();
    SwitchContext();
    OS_RestoreInterrupts(priorState);
}

void OS_YieldThread()
{
    ProcessorContext* contextBeforeActive = NULL;
    ProcessorContext* lastEqualPriorityContext = NULL;
    int numEqualPriorityContexts = 0;

    int priorState;

    ProcessorContext* active = data_021112e0.substruct_24.activeContext;
    priorState = OS_DisableInterrupts();
    

    ProcessorContext* loopPrevContext = NULL;    
    ProcessorContext* loopContext = data_021112e0.substruct_24.firstContext;
    if (data_021112e0.substruct_24.firstContext != NULL)
    {
        unsigned int targetPriority = active->priority;
        do {
            if (loopContext == active)
                contextBeforeActive = loopPrevContext;

            if (targetPriority == loopContext->priority)
            {
                lastEqualPriorityContext = loopContext;
                numEqualPriorityContexts++;
            }

            loopPrevContext = loopContext;
            loopContext = loopContext->pNext;
        } while (loopContext != NULL);
    }

    if (numEqualPriorityContexts <= 1 || lastEqualPriorityContext == active)
    {
        OS_RestoreInterrupts(priorState);
        return;
    }
    
    if (contextBeforeActive != NULL)
    {
        contextBeforeActive->pNext = active->pNext;
    }
    else
    {
        DECLARE_ASM_NOP();
        data_021112e0.substruct_24.firstContext = active->pNext;
    }

    active->pNext = lastEqualPriorityContext->pNext;
    lastEqualPriorityContext->pNext = active;
    SwitchContext();
    OS_RestoreInterrupts(priorState);
    return;
}

void MarkContextStackTopUnknownSubspace(ProcessorContext *context, unsigned int size)
{
    context->stackUnknownTopSubspaceSize = size;
    if (size != 0)
    {
        *(int*)(context->stackTop + size) = STACK_UNKNOWN_SECTION_MAGIC;
    }
}

bool OS_SetThreadPriority(ProcessorContext* context, unsigned int newPriority)
{
    ProcessorContext* loopContext = data_021112e0.substruct_24.firstContext;
    ProcessorContext* prevContext = NULL;
    int priorState = OS_DisableInterrupts();

    for (; loopContext != NULL && loopContext != context; loopContext = loopContext->pNext)
    {
        prevContext = loopContext;
    }

    if (loopContext == NULL || loopContext == &data_021112e0.contextA)
    {
        OS_RestoreInterrupts(priorState);
        return false;
    }

    if (loopContext->priority != newPriority)
    {
        // Remove context from the list
        if (prevContext != NULL)
            prevContext->pNext = context->pNext;
        else
        {
            DECLARE_ASM_NOP();
            data_021112e0.substruct_24.firstContext = context->pNext;
        }
        
        context->priority = newPriority;
        InsertContextIntoGlobalList(context);
        SwitchContext();
    }
    OS_RestoreInterrupts(priorState);
    return true;
}

unsigned int OS_GetThreadPriority(ProcessorContext* context)
{
    return context->priority;
}

void OS_Sleep(unsigned int milliseconds)
{
    Alarm timing;
    OS_CreateAlarm(&timing);
    ProcessorContext* context = *data_021112e0.ppActiveContext;

    int priorState = OS_DisableInterrupts();

    uint64_t numTicks = (uint64_t)milliseconds * TIMER_TICKS_PER_MILLISECOND >> 6;
    context->sleepAlarm = &timing;
    OS_SetAlarm(&timing, numTicks, &SleepCompletionProc, &context);

    if (context != NULL)
    {
        do {
            OS_SleepThread(NULL);
        } while (context != NULL);
    }

    OS_RestoreInterrupts(priorState);
}

void SleepCompletionProc(ProcessorContext **ppContext)
{
    ProcessorContext* context = *ppContext;
    *ppContext = NULL;
    context->sleepAlarm = NULL;
    OS_WakeupThreadDirect(context);
}

PFNSwitchContextProc SetSwitchContextProcB(PFNSwitchContextProc proc)
{
    int priorState = OS_DisableInterrupts();
    PFNSwitchContextProc oldProc = data_021112e0.substruct_24.switchContextProcB;
    data_021112e0.substruct_24.switchContextProcB = proc;
    OS_RestoreInterrupts(priorState);
    return oldProc;
}

void InterruptWaitLoopFunction(void* unusedUserdata)
{
    EnableIRQInterrupts();
    while (true)
        OS_Halt();
}

unsigned int OS_DisableScheduler()
{
    int priorState = OS_DisableInterrupts();

    unsigned int oldCount;
    if (data_021112e0.contextSwitchLock < (unsigned int)-1)
    {
        oldCount = data_021112e0.contextSwitchLock;
        data_021112e0.contextSwitchLock++;
    }

    OS_RestoreInterrupts(priorState);
    return oldCount;
}

unsigned int OS_EnableScheduler()
{
    int priorState = OS_DisableInterrupts();

    unsigned int oldCount = 0;
    if (data_021112e0.contextSwitchLock > 0)
    {
        oldCount = data_021112e0.contextSwitchLock;
        data_021112e0.contextSwitchLock--;
    }

    OS_RestoreInterrupts(priorState);
    return oldCount;
}

void SetContextEndProc(ProcessorContext* context, ProcessorContext::ExitRoutine proc)
{
    context->exitProc = proc;
}

void InitializeContextRegisters(ProcessorContext* context, unsigned int startAddress, unsigned int stackBottom)
{
    unsigned int resumeAddress = startAddress + 4;
    context->resumeAddress = resumeAddress;
    context->supervisorStackPointer = stackBottom;
    stackBottom -= 0x40;
    
    __asm("tst stackBottom, 4");
    __asm("bne adjustStackPtr");
    __asm("b skipAdjustStackPtr");


__asm("adjustStackPtr:");
    stackBottom -= 4;
__asm("skipAdjustStackPtr:");
    context->userModeRegisters[13] = stackBottom;

    unsigned int status;
    __asm("ands status, resumeAddress, 1");
    __asm("bne thumb_mode");
    __asm("b skip_thumb_mode_assignment");
__asm("thumb_mode:");
    status = 0x1f | (1 << 5);
__asm("skip_thumb_mode_assignment:");
    __asm("beq arm_mode");
    __asm("b skip_arm_mode_assignment");
__asm("arm_mode:");
    status = 0x1f;
__asm("skip_arm_mode_assignment:");


    context->programStatusRegister = status;
    context->userModeRegisters[0] = 0;
    context->userModeRegisters[1] = 0;
    context->userModeRegisters[2] = 0;
    context->userModeRegisters[3] = 0;
    context->userModeRegisters[4] = 0;
    context->userModeRegisters[5] = 0;
    context->userModeRegisters[6] = 0;
    context->userModeRegisters[7] = 0;
    context->userModeRegisters[8] = 0;
    context->userModeRegisters[9] = 0;
    context->userModeRegisters[10] = 0;
    context->userModeRegisters[11] = 0;
    context->userModeRegisters[12] = 0;
    context->userModeRegisters[14] = 0;
}