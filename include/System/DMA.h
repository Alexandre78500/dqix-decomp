#pragma once

typedef void(*DMACompletionCallback)(int userdata);

struct DMARegisterSet
{
    unsigned int sourceAddr;
    unsigned int destAddr;
    union {
        unsigned int u32;
        struct {
            unsigned short transferSize;
            unsigned short control;
        };
    } controlAndSize;
};

#if defined(jpn)
#define data_020f2270 data_020f23dc
#endif

// DMA channel for general use
extern int data_020f2270;

// ITCM functions. The atomic operations are wrapped with OS_DisableInterrupts()
// and OS_RestoreInterrupts(), 
extern "C" void MIi_DmaSetParams(int channel, unsigned src, unsigned dst, unsigned ctrl);
void ConfigureDMATransferSafeAtomic(int channel, unsigned src, unsigned dst, unsigned ctrl);
void ConfigureDMATransfer(int channel, unsigned src, unsigned dst, unsigned ctrl);
void ConfigureDMATransferSafe(int channel, unsigned src, unsigned dst, unsigned ctrl);

void DMAMemsetSynchronous(int channel, unsigned int dst, unsigned int value, unsigned int len);
void DMAMemcpySynchronous(int channel, unsigned int src, unsigned int dst, unsigned int len);
extern "C" void MI_DmaCopy16(int channel, unsigned int src, unsigned int dst, unsigned int len);

// Because immediate mode DMA transfers block the CPU until done, this isn't
// actually async, just set up to look like it
void DMAMemsetAsync(int channel, unsigned int dst, unsigned int value, unsigned int len,
    DMACompletionCallback onCompletion, int callbackUserdata);
void DMAMemcpyAsync(int channel, unsigned int src, unsigned int dst, unsigned int len,
    DMACompletionCallback onCompletion, int callbackUserdata);

void AwaitDMACompletion(int channel);
extern "C" void MI_StopDma(int channel);
void ResetAllDMAChannels();

extern "C" void MIi_CheckAnotherAutoDMA(int channel, unsigned int newTimingFlags);
extern "C" void MIi_CheckDma0SourceAddress(int channel, unsigned int source, unsigned int length, unsigned int sourceCtrlFlags);
