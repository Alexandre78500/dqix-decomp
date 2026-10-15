#pragma once

void InvalidateDataCache();
void CleanDataCache();
void CleanInvalidateDataCache();
extern "C" void DC_InvalidateRange(const void* where, unsigned int len);
extern "C" void DC_StoreRange(const void* where, unsigned int len);
extern "C" void DC_FlushRange(const void* where, unsigned int len);
extern "C" void DC_WaitWriteBufferEmpty();
void InvalidateInstructionCache();
void InvalidateInstructionCacheRange(const void* where, unsigned int len);