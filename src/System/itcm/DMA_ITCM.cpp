#include "System/DMA.h"
#include "System/Interrupts.h"
#include <asmhacks.h>

// configuration is such that
// - DMA channel is enabled (bit 31)
// - source address is fixed (bits 24 & 23 are 10)
// - dest address is fixed (bits 22 & 21 are 10)
// - one 16-bit value to be transferred
#define DMA_CHANNEL_0_DEFAULT_CONTROL 0x81400001

inline volatile DMARegisterSet& DMARegisters(int n)
{
    return *(volatile DMARegisterSet*)(12 * n + 0x040000b0);
}

void MIi_DmaSetParams(int channel, unsigned src, unsigned dst, unsigned ctrl)
{
    int priorState = OS_DisableInterrupts();

    volatile DMARegisterSet& regs = DMARegisters(channel);
    regs.sourceAddr = src;
    regs.destAddr = dst;
    regs.controlAndSize.u32 = ctrl;

    OS_RestoreInterrupts(priorState);
}

void ConfigureDMATransferSafeAtomic(int channel, unsigned src, unsigned dst, unsigned ctrl)
{
    int priorState = OS_DisableInterrupts();

    volatile DMARegisterSet& regs = DMARegisters(channel);
    regs.sourceAddr = src;
    regs.destAddr = dst;
    regs.controlAndSize.u32 = ctrl;

    // wastes 2 cycles, probably for timing purposes
    (void)DMARegisters(0).sourceAddr;
    (void)DMARegisters(0).sourceAddr;

    if (channel != 0)
    {
        DECLARE_ASM_NOP();
    }
    else
    {
        regs.sourceAddr = 0;
        regs.destAddr = 0;
        regs.controlAndSize.u32 = DMA_CHANNEL_0_DEFAULT_CONTROL;
    }

    OS_RestoreInterrupts(priorState);
}

void ConfigureDMATransfer(int channel, unsigned src, unsigned dst, unsigned ctrl)
{
    volatile DMARegisterSet& regs = DMARegisters(channel);
    regs.sourceAddr = src;
    regs.destAddr = dst;
    regs.controlAndSize.u32 = ctrl;
}

void ConfigureDMATransferSafe(int channel, unsigned src, unsigned dst, unsigned ctrl)
{
    volatile DMARegisterSet& regs = DMARegisters(channel);
    regs.sourceAddr = src;
    regs.destAddr = dst;
    regs.controlAndSize.u32 = ctrl;

    // wastes 2 cycles, probably for timing purposes
    (void)DMARegisters(0).sourceAddr;
    (void)DMARegisters(0).sourceAddr;

    if (channel != 0)
    {
        DECLARE_ASM_NOP();
    }
    else
    {
        regs.sourceAddr = 0;
        regs.destAddr = 0;
        regs.controlAndSize.u32 = DMA_CHANNEL_0_DEFAULT_CONTROL;
    }

    (void)DMARegisters(0).sourceAddr;
    (void)DMARegisters(0).sourceAddr;
}