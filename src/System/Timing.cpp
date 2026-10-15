#include "System/Timing.h"
#include "System/Interrupts.h"
#include "System/DTCM.h"
#include <globaldefs.h>
#include <asmhacks.h>

#define TIMER_N_COUNTER(n) (*(volatile unsigned short*)(0x04000100 + 4 * (n)))
#define TIMER_N_CONTROL(n) (*(volatile unsigned short*)(0x04000102 + 4 * (n)))

#define TIMER_CONTROL_FLAGS_START (1 << 7)
#define TIMER_CONTROL_FLAGS_ENABLE_IRQ_ON_OVERFLOW (1 << 6)
#define TIMER_CONTROL_FLAGS_COUNT_UP (1 << 2)
#define TIMER_CONTROL_FLAGS_PRESCALE_1X (0 << 0)
#define TIMER_CONTROL_FLAGS_PRESCALE_64X (1 << 0)
#define TIMER_CONTROL_FLAGS_PRESCALE_256X (2 << 0)
#define TIMER_CONTROL_FLAGS_PRESCALE_1024X (3 << 0)

#define INTERRUPT_REQUEST_FLAGS (*(volatile unsigned int*)0x04000214)

#pragma dont_inline on

#if defined(jpn)
#define OS_Terminate func_020cb6ac
#endif

extern "C"
{
    // something like abort() (never returns)
    void OS_Terminate();
}

void MarkAlarmInitializationFlagBit(int bit)
{
    data_02111634 |= (1 << bit);
}

void OS_InitTick()
{
    if (data_02111638.isInitialized)
        return;

    data_02111638.isInitialized = true;
    MarkAlarmInitializationFlagBit(0);
    data_02111638.numTimerOverflows = 0;
    TIMER_N_CONTROL(0) = 0;
    TIMER_N_COUNTER(0) = 0;
    TIMER_N_CONTROL(0) = TIMER_CONTROL_FLAGS_START | TIMER_CONTROL_FLAGS_ENABLE_IRQ_ON_OVERFLOW | TIMER_CONTROL_FLAGS_PRESCALE_64X;

    SetInterruptHandler(IRQ_MASK_TIMER_0_OVERFLOW, &On16BitTimerOverflow);
    EnableSpecificInterrupts(IRQ_MASK_TIMER_0_OVERFLOW);
    data_02111638.reloadTimerOnNextInterrupt = false;
}

int OS_IsTickAvailable()
{
    return data_02111638.isInitialized;
}

void On16BitTimerOverflow(int)
{
    data_02111638.numTimerOverflows++;
    if (data_02111638.reloadTimerOnNextInterrupt)
    {
        TIMER_N_CONTROL(0) = 0;
        TIMER_N_COUNTER(0) = 0;
        TIMER_N_CONTROL(0) = TIMER_CONTROL_FLAGS_START | TIMER_CONTROL_FLAGS_ENABLE_IRQ_ON_OVERFLOW | TIMER_CONTROL_FLAGS_PRESCALE_64X;
        
        data_02111638.reloadTimerOnNextInterrupt = false;
    }
    SetTimerOverflowCallback(0, &On16BitTimerOverflow, 0);
}

uint64_t OS_GetTick()
{
    int priorState = OS_DisableInterrupts();
    volatile unsigned short lowBits = TIMER_N_COUNTER(0);

    volatile uint64_t highBits = data_02111638.numTimerOverflows & 0xffffffffffff;
    if ((INTERRUPT_REQUEST_FLAGS & IRQ_MASK_TIMER_0_OVERFLOW) && !(lowBits & 0x8000))
        highBits++;

    OS_RestoreInterrupts(priorState);
    return (highBits << 16) | lowBits;
}

unsigned short OS_GetTickLo()
{
    return TIMER_N_COUNTER(0);
}

void MarkNextAlarmToSound(Alarm *timing)
{
    uint64_t now = OS_GetTick();
    TIMER_N_CONTROL(1) = 0;
    int64_t timeRemaining = timing->alarmTime - now;
    SetTimerOverflowCallback(1, &Timer1OverflowInterruptRoutine, 0);
    unsigned short counterValue = 0;
    if (timeRemaining < 0)
    {
        counterValue = 0xfffe;
    }
    else if (timeRemaining < 0x10000)
    {
        DECLARE_ASM_NOP();
        counterValue = ~timeRemaining;
    }
    TIMER_N_COUNTER(1) = counterValue;
    TIMER_N_CONTROL(1) = TIMER_CONTROL_FLAGS_START | TIMER_CONTROL_FLAGS_ENABLE_IRQ_ON_OVERFLOW | TIMER_CONTROL_FLAGS_PRESCALE_64X;
    EnableSpecificInterrupts(IRQ_MASK_TIMER_1_OVERFLOW);
}

void OS_InitAlarm()
{
    if (data_02111648.isInitialized)
        return;

    data_02111648.isInitialized = true;
    MarkAlarmInitializationFlagBit(1);
    data_02111648.pFirstAlarm = NULL;
    data_02111648.pLastAlarm = NULL;
    DisableSpecificInterrupts(IRQ_MASK_TIMER_1_OVERFLOW);
}

int OS_IsAlarmAvailable()
{
    return data_02111648.isInitialized;
}

void OS_CreateAlarm(Alarm* alarm)
{
    alarm->completionProc = NULL;
    alarm->unknown_8 = 0;
}

void RegisterAlarm(Alarm* alarm, uint64_t ringTime)
{
    if (alarm->alarmIntervalLength != 0)
    {
        uint64_t now = OS_GetTick();
        ringTime = alarm->alarmIntervalResidue;
        if (alarm->alarmIntervalResidue < now)
        {
            uint64_t blockCount = (now - alarm->alarmIntervalResidue) / alarm->alarmIntervalLength;
            ringTime = alarm->alarmIntervalResidue + alarm->alarmIntervalLength * (blockCount + 1);
        }
    }
    alarm->alarmTime = ringTime;

    Alarm* loopAlarm = data_02111648.pFirstAlarm;
    if (loopAlarm != NULL)
    {
        do
        {
            if ((int64_t)(ringTime - loopAlarm->alarmTime) < 0)
            {
                alarm->pPrev = loopAlarm->pPrev;
                loopAlarm->pPrev = alarm;
                alarm->pNext = loopAlarm;

                if (alarm->pPrev != NULL)
                {
                    alarm->pPrev->pNext = alarm;
                }
                else
                {
                    data_02111648.pFirstAlarm = alarm;
                    MarkNextAlarmToSound(alarm);
                }
                return;
            }
            loopAlarm = loopAlarm->pNext;
        } while (loopAlarm != NULL);
    }

    // If we get here, this alarm is to go at the end of the list
    alarm->pNext = NULL;
    Alarm* prevAlarm = data_02111648.pLastAlarm;
    data_02111648.pLastAlarm = alarm;
    alarm->pPrev = prevAlarm;
    if (prevAlarm != NULL)
    {
        prevAlarm->pNext = alarm;
    }
    else
    {
        data_02111648.pLastAlarm = alarm;
        data_02111648.pFirstAlarm = alarm;
        MarkNextAlarmToSound(alarm);
    }
}

void OS_SetAlarm(Alarm* alarm, uint64_t numTicks,
    Alarm::PFNCompletion callback, ProcessorContext** ppContext)
{
    if (alarm == NULL || alarm->completionProc != NULL)
        OS_Terminate();

    int priorState = OS_DisableInterrupts();
    alarm->alarmIntervalLength = 0;
    alarm->completionProc = callback;
    alarm->ppContext = ppContext;

    uint64_t now = OS_GetTick();
    RegisterAlarm(alarm, numTicks + now);
    OS_RestoreInterrupts(priorState);
}

void SetInterval(Alarm *alarm, uint64_t residue, uint64_t interval,
    Alarm::PFNCompletion callback, ProcessorContext **ppContext)
{
    if (alarm == NULL || alarm->completionProc != NULL)
        OS_Terminate();

    int priorState = OS_DisableInterrupts();
    alarm->alarmIntervalLength = interval;
    alarm->alarmIntervalResidue = residue;
    alarm->completionProc = callback;
    alarm->ppContext = ppContext;

    RegisterAlarm(alarm, 0);
    OS_RestoreInterrupts(priorState);
}

void OS_CancelAlarm(Alarm *alarm)
{
    int priorState = OS_DisableInterrupts();

    if (alarm->completionProc == NULL)
    {
        OS_RestoreInterrupts(priorState);
        return;
    }

    Alarm* nextAlarm = alarm->pNext;
    if (alarm->pNext != NULL)
        alarm->pNext->pPrev = alarm->pPrev;
    else
    {
        DECLARE_ASM_NOP();
        data_02111648.pLastAlarm = alarm->pPrev;
    }

    if (alarm->pPrev != NULL)
        alarm->pPrev->pNext = nextAlarm;
    else // this is the first alarm
    {
        data_02111648.pFirstAlarm = nextAlarm;
        if (nextAlarm != NULL)
            MarkNextAlarmToSound(nextAlarm);
    }

    alarm->completionProc = NULL;
    alarm->alarmIntervalLength = 0;
    OS_RestoreInterrupts(priorState);
}

#ifdef __MWERKS__
asm void Timer1OverflowInterruptRoutine(int)
{
    stmdb sp!, {r0, lr}
    bl HandleTimer1Overflow
    ldmia sp!, {r0, lr}
    bx lr
}
#else
void Timer1OverflowInterruptRoutine(int)
{
    HandleTimer1Overflow();
}
#endif

void HandleTimer1Overflow()
{
    TIMER_N_CONTROL(1) = 0;
    DisableSpecificInterrupts(IRQ_MASK_TIMER_1_OVERFLOW);
    DTCM_DATA_INTERRUPTS_FIRED |= IRQ_MASK_TIMER_1_OVERFLOW;
    uint64_t now = OS_GetTick();

    Alarm* firstAlarm = data_02111648.pFirstAlarm;
    if (firstAlarm == NULL)
        return;

    if (now < firstAlarm->alarmTime)
    {
        MarkNextAlarmToSound(firstAlarm);
    }
    else
    {
        Alarm* next = firstAlarm->pNext;
        data_02111648.pFirstAlarm = next;
        if (next == NULL)
            data_02111648.pLastAlarm = NULL;
        else
            next->pPrev = NULL;

        Alarm::PFNCompletion callback = firstAlarm->completionProc;
        if (firstAlarm->alarmIntervalLength == 0)
            firstAlarm->completionProc = NULL;
        if (callback != NULL)
            callback(firstAlarm->ppContext);
        
        if (firstAlarm->alarmIntervalLength != 0)
        {
            firstAlarm->completionProc = callback; // why is this necessary?
            RegisterAlarm(firstAlarm, 0);
        }

        if (data_02111648.pFirstAlarm != NULL)
            MarkNextAlarmToSound(data_02111648.pFirstAlarm);
    }
}