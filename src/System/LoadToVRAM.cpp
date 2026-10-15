#include "System/LoadToVRAM.h"
#include "System/DMA.h"
#include "System/BGBases.h"
#include "System/VRAM.h"
#include <globaldefs.h>
#include <asmhacks.h>

#pragma optimize_for_size off

#if defined(jpn)
#define MIi_CpuCopy16 func_020cbe84
#define MIi_CpuCopy32 func_020cbed4

#define data_020ed658 data_020ed764
#define data_020ed668 data_020ed774
#define data_02111240 data_02110ee0
#define data_0211125c data_02110efc
#endif

#define ADDR_MAIN_BG_STANDARD_PALETTE 0x05000000
#define ADDR_MAIN_OBJ_STANDARD_PALETTE 0x05000200
#define ADDR_SUB_BG_STANDARD_PALETTE 0x05000400
#define ADDR_SUB_OBJ_STANDARD_PALETTE 0x05000600

struct ExtPaletteMappingData
{
    // sub BG: can only use {H} or none, so no need for map address
    int subBGExtPaletteBanks_240_;
    // main obj: can use either {F} or {G}, each 2x large enough
    unsigned int mainObjExtPaletteMapAddress_244_;
    int mainObjExtPaletteBanks_248_;
    // main BG: can use subsets of {E, F, G}, max 32k usage.
    // if only {G} is mapped, we want to treat this as the 'second half'
    // (presumably so we can add/remove {F} seamlessly) so the start of G
    // is counted as offset 16384 - this is the first variable below
    unsigned int mainBGExtPaletteInitialOffset_24c_;
    unsigned int mainBGExtPaletteMapAddress_250_;
    int mainBGExtPaletteBanks_254_;
    // sub OBJ: can only use {I} or none, so no need for map address
    int subObjExtPaletteBanks_258_;
} extern data_02111240;

struct TextureMappingData
{
    int clearTextureBanks_25c_;
    // as texture image can use any of banks {A, B, C, D}
    // it's possible to end up spread over two different blocks
    unsigned int textureImageFirstMapAddress_260_;
    unsigned int texturePaletteMapAddress_264_;
    int texturePaletteBanks_268_;
    unsigned int clearTextureMapAddress_26c_;
    int textureImageBanks_270_;
    unsigned int textureImageSecondMapAddress_274_;
    unsigned int textureImageFirstMapRegionSize_278_;
} extern data_0211125c;

// Address lookup table for texture palette memory mapping using
// banks E, F, G. The bitmasks for these are 0, 0x10, 0x20, ..., 0x70
// so index into the table with (banks >> 4)
extern const unsigned short data_020ed658[8];

// Holds triples (first block start, second block start, first block size)
// for texture image memory mapping using banks A-D.
// All quantities are right shifted by 12 so that they fit into u16s
extern const unsigned short data_020ed668[48];

extern "C"
{
    // inverted memcpy via u16s
    void MIi_CpuCopy16(const void* src, void* dst, unsigned len);
    // inverted memcpy via u32s
    void MIi_CpuCopy32(const void* src, void* dst, unsigned len);
}

void GX_LoadBGPltt(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, 0x05000000 + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(0x05000000 + offset), length);
}

void GXS_LoadBGPltt(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, 0x05000400 + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(0x05000400 + offset), length);
}

void GX_LoadOBJPltt(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, 0x05000200 + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(0x05000200 + offset), length);
}

void GXS_LoadOBJPltt(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, 0x05000600 + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(0x05000600 + offset), length);
}

void GX_LoadOAM(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, 0x07000000 + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(0x07000000 + offset), length);
}

void GXS_LoadOAM(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, 0x07000400 + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(0x07000400 + offset), length);
}

// not quite a match
void GX_LoadOBJ(const void* data, unsigned int offset, unsigned int length)
{
    int writeAddr = 0x06400000;
    if (GXi_DmaId != -1 && length > 48)
    {
        writeAddr += offset;
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, writeAddr, length);
    }
    else
    {
        writeAddr += offset;
        MIi_CpuCopy32(data, (void*)writeAddr, length);
    }
}

// not quite a match
void GXS_LoadOBJ(const void* data, unsigned int offset, unsigned int length)
{
    int writeAddr = 0x06600000;
    if (GXi_DmaId != -1 && length > 48)
    {
        writeAddr += offset;
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, writeAddr, length);
    }
    else
    {
        writeAddr += offset;
        MIi_CpuCopy32(data, (void*)writeAddr, length);
    }
}

void GX_LoadBG0Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG0ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GXS_LoadBG0Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG0ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GX_LoadBG1Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG1ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GXS_LoadBG1Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG1ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GX_LoadBG2Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG2ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GXS_LoadBG2Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG2ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GX_LoadBG3Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG3ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GXS_LoadBG3Scr(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG3ScreenBase();
    if (GXi_DmaId != -1 && length > 28)
        MI_DmaCopy16(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy16(data, (void*)(base + offset), length);
}

void GX_LoadBG0Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG0CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GXS_LoadBG0Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG0CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GX_LoadBG1Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG1CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GXS_LoadBG1Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG1CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GX_LoadBG2Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG2CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GXS_LoadBG2Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG2CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GX_LoadBG3Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetMainBG3CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GXS_LoadBG3Char(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int base = GetSubBG3CharacterBase();
    if (GXi_DmaId != -1 && length > 48)
        DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, base + offset, length);
    else
        MIi_CpuCopy32(data, (void*)(base + offset), length);
}

void GX_BeginLoadBGExtPltt()
{
    data_02111240.mainBGExtPaletteBanks_254_ = ReleaseMainBGExtendedPaletteVRAMBanks();
    switch (data_02111240.mainBGExtPaletteBanks_254_)
    {
    case VRAM_BANK_E:
        data_02111240.mainBGExtPaletteMapAddress_250_ = 0x06880000;
        data_02111240.mainBGExtPaletteInitialOffset_24c_ = 0;
        break;
    case VRAM_BANK_G:
        data_02111240.mainBGExtPaletteMapAddress_250_ = 0x06894000;
        data_02111240.mainBGExtPaletteInitialOffset_24c_ = 16 * 1024;
        break;
    case VRAM_BANK_F | VRAM_BANK_G:
    case VRAM_BANK_F:
        data_02111240.mainBGExtPaletteMapAddress_250_ = 0x06890000;
        data_02111240.mainBGExtPaletteInitialOffset_24c_ = 0;
        break;
    case 0:
        break;
    }
}

void GX_LoadBGExtPltt(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int dest = data_02111240.mainBGExtPaletteMapAddress_250_ + offset - data_02111240.mainBGExtPaletteInitialOffset_24c_;
    if (GXi_DmaId != -1)
        DMAMemcpyAsync(GXi_DmaId, (unsigned int)data, dest, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)dest, length);
}

void GX_EndLoadBGExtPltt()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);

    MapVRAMBanksToMainBGExtendedPalette(data_02111240.mainBGExtPaletteBanks_254_);
    data_02111240.mainBGExtPaletteBanks_254_ = 0;
    data_02111240.mainBGExtPaletteMapAddress_250_ = 0;
    data_02111240.mainBGExtPaletteInitialOffset_24c_ = 0;
}

void GX_BeginLoadOBJExtPltt()
{
    data_02111240.mainObjExtPaletteBanks_248_ = ReleaseMainObjExtendedPaletteVRAMBanks();
    switch (data_02111240.mainObjExtPaletteBanks_248_)
    {
    case 0:
        break;
    case VRAM_BANK_F:
        data_02111240.mainObjExtPaletteMapAddress_244_ = 0x06890000;
        break;
    case VRAM_BANK_G:
        data_02111240.mainObjExtPaletteMapAddress_244_ = 0x06894000;
        break;
    }
}

void GX_LoadOBJExtPltt(const void* data, unsigned int offset, unsigned int length)
{
    int dmaChannel = GXi_DmaId;
    unsigned int dest = data_02111240.mainObjExtPaletteMapAddress_244_ + offset;
    if (dmaChannel != -1)
        DMAMemcpyAsync(dmaChannel, (unsigned int)data, dest, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)dest, length);
}

void GX_EndLoadOBJExtPltt()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);

    MapVRAMBanksToMainObjExtendedPalette(data_02111240.mainObjExtPaletteBanks_248_);
    data_02111240.mainObjExtPaletteBanks_248_ = 0;
    data_02111240.mainObjExtPaletteMapAddress_244_ = 0;
}

void GXS_BeginLoadBGExtPltt()
{
    data_02111240.subBGExtPaletteBanks_240_ = ReleaseSubBGExtendedPaletteVRAMBanks();
}

void GXS_LoadBGExtPltt(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1)
        DMAMemcpyAsync(GXi_DmaId, (unsigned int)data, 0x06898000 + offset, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)(0x06898000 + offset), length);
}

void GXS_EndLoadBGExtPltt()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);
    
    MapVRAMBanksToSubBGExtendedPalette(data_02111240.subBGExtPaletteBanks_240_);
    data_02111240.subBGExtPaletteBanks_240_ = 0;
}

void GXS_BeginLoadOBJExtPltt()
{
    data_02111240.subObjExtPaletteBanks_258_ = ReleaseSubObjExtendedPaletteVRAMBanks();
}

void GXS_LoadOBJExtPltt(const void* data, unsigned int offset, unsigned int length)
{
    if (GXi_DmaId != -1)
        DMAMemcpyAsync(GXi_DmaId, (unsigned int)data, 0x068a0000 + offset, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)(0x068a0000 + offset), length);
}

void GXS_EndLoadOBJExtPltt()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);

    MapVRAMBanksToSubObjExtendedPalette(data_02111240.subObjExtPaletteBanks_258_);
    data_02111240.subObjExtPaletteBanks_258_ = 0;
}

void GX_BeginLoadTex()
{
    int banks = ReleaseTextureImageVRAMBanks();
    data_0211125c.textureImageBanks_270_ = banks;

    struct LookupEntry {
        unsigned short entry;
        unsigned short pad[2];
    };

    const LookupEntry* firstBlockLookup =  (const LookupEntry*)&data_020ed668[0];
    const LookupEntry* secondBlockLookup = (const LookupEntry*)&data_020ed668[1];
    const LookupEntry* thirdBlockLookup =  (const LookupEntry*)&data_020ed668[2];

    data_0211125c.textureImageFirstMapAddress_260_ = firstBlockLookup[banks].entry << 12;
    data_0211125c.textureImageSecondMapAddress_274_ = secondBlockLookup[banks].entry << 12;
    data_0211125c.textureImageFirstMapRegionSize_278_ = thirdBlockLookup[banks].entry << 12;
}

void GX_LoadTex(const void* data, unsigned int offset, unsigned int length)
{
    unsigned int secondBlockStart = data_0211125c.textureImageSecondMapAddress_274_;
    unsigned int firstBlockWriteAddr;
    
    // only one region is memory mapped, so only do one write
    if (secondBlockStart == 0)
        firstBlockWriteAddr = data_0211125c.textureImageFirstMapAddress_260_ + offset;
    // the desired write fits entirely into the first mapped block
    else if (offset + length < data_0211125c.textureImageFirstMapRegionSize_278_)
        firstBlockWriteAddr = data_0211125c.textureImageFirstMapAddress_260_ + offset;
    // desired write fits entirely into second mapped block
    else if (offset >= data_0211125c.textureImageFirstMapRegionSize_278_)
        firstBlockWriteAddr = secondBlockStart + offset - data_0211125c.textureImageFirstMapRegionSize_278_;
    else // desired write spans both mapped blocks
    {
        firstBlockWriteAddr = data_0211125c.textureImageFirstMapAddress_260_ + offset;
        unsigned int firstWriteSize = data_0211125c.textureImageFirstMapRegionSize_278_ - offset;
        
        if (GXi_DmaId != -1 && firstWriteSize > 48)
            DMAMemcpySynchronous(GXi_DmaId, (unsigned int)data, firstBlockWriteAddr, firstWriteSize);
        else
            MIi_CpuCopy32(data, (void*)firstBlockWriteAddr, firstWriteSize);

        if (GXi_DmaId != -1)
            DMAMemcpyAsync(GXi_DmaId, (unsigned int)data + firstWriteSize, secondBlockStart, length - firstWriteSize, NULL, 0);
        else
            MIi_CpuCopy32((const void*)((unsigned int)data + firstWriteSize), (void*)secondBlockStart, length - firstWriteSize);
        return;
    }

    if (GXi_DmaId != -1)
        DMAMemcpyAsync(GXi_DmaId, (unsigned int)data, firstBlockWriteAddr, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)firstBlockWriteAddr, length);
}

void GX_EndLoadTex()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);
    MapVRAMBanksToTextureImage(data_0211125c.textureImageBanks_270_);
    data_0211125c.textureImageFirstMapRegionSize_278_ = 0;
    data_0211125c.textureImageSecondMapAddress_274_ = 0;
    data_0211125c.textureImageFirstMapAddress_260_ = 0;
    data_0211125c.textureImageBanks_270_ = 0;
}

void GX_BeginLoadTexPltt()
{
    int banks = ReleaseTexturePaletteVRAMBanks();
    data_0211125c.texturePaletteBanks_268_ = banks;
    data_0211125c.texturePaletteMapAddress_264_ = data_020ed658[banks >> 4] << 12;
}

void GX_LoadTexPltt(const void* data, unsigned int offset, unsigned int length)
{
    int dmaChannel = GXi_DmaId;
    unsigned int writeAddr = data_0211125c.texturePaletteMapAddress_264_ + offset;
    if (dmaChannel != -1)
        DMAMemcpyAsync(dmaChannel, (unsigned int)data, writeAddr, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)writeAddr, length);
}

void GX_EndLoadTexPltt()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);
    MapVRAMBanksToTexturePalette(data_0211125c.texturePaletteBanks_268_);
    data_0211125c.texturePaletteBanks_268_ = 0;
    data_0211125c.texturePaletteMapAddress_264_ = 0;
}

void GX_BeginLoadClearImage()
{
    int banks = ReleaseClearTextureVRAMBanks();
    data_0211125c.clearTextureBanks_25c_ = banks;
    switch (banks)
    {
    case VRAM_BANK_B:
    case VRAM_BANK_A | VRAM_BANK_B:
        data_0211125c.clearTextureMapAddress_26c_ = 0x06800000;
        break;
    case VRAM_BANK_D:
    case VRAM_BANK_C | VRAM_BANK_D:
        data_0211125c.clearTextureMapAddress_26c_ = 0x06840000;
        break;
    case VRAM_BANK_A:
        data_0211125c.clearTextureMapAddress_26c_ = 0x067e0000;
        break;
    case VRAM_BANK_C:
        data_0211125c.clearTextureMapAddress_26c_ = 0x06820000;
        break;
    }
}

void GX_LoadClearImageColor(const void* data, unsigned int length)
{
    int dmaChannel = GXi_DmaId;
    unsigned int writeAddr = data_0211125c.clearTextureMapAddress_26c_;
    if (dmaChannel != -1)
        DMAMemcpyAsync(dmaChannel, (unsigned int)data, writeAddr, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)writeAddr, length);
}

void GX_LoadClearImageDepth(const void* data, unsigned int length)
{
    unsigned int writeAddr = data_0211125c.clearTextureMapAddress_26c_ + 0x20000;
    if (GXi_DmaId != -1)
        DMAMemcpyAsync(GXi_DmaId, (unsigned int)data, writeAddr, length, NULL, 0);
    else
        MIi_CpuCopy32(data, (void*)writeAddr, length);
}

void GX_EndLoadClearImage()
{
    if (GXi_DmaId != -1)
        AwaitDMACompletion(GXi_DmaId);

    MapVRAMBanksToClearTextures(data_0211125c.clearTextureBanks_25c_);
    data_0211125c.clearTextureBanks_25c_ = 0;
    data_0211125c.clearTextureMapAddress_26c_ = 0;
}