#include <nitro.h>
#include <nnsys/g2d_renderer_internal.h>

#pragma force_active on

static inline BOOL IsAffineLevelSet(void) {
    return !gRendererWork.state.translateOnly && gRendererContext.affineLevel != RENDERER_NO_AFFINE_LEVEL;
}

// a scale or rotation at the current level: the level needs a new matrix cache
static inline void MarkCurrentLevelAffine(void) {
    G2dMtxCacheRecord *pRecord;
    if (IsAffineLevelSet() == FALSE) {
        gRendererContext.affineLevel = (u16)gRendererWork.state.stackLevel;
    }
    pRecord = &gRendererWork.mtxCacheRecords[(u16)gRendererWork.state.stackLevel];
    switch (pRecord->state) {
    case 0:
    case 2:
    case 3:
        pRecord->groupId = gRendererWork.state.nextMtxGroupId++;
        pRecord->state = 1;
        break;
    case 1:
        break;
    }
}

#pragma force_active off

const NNSG2dPaletteSwapTable *NNS_G2dGetRendererPaletteTbl(const NNSG2dRendererInstance *pRend) {
    return pRend->pPaletteSwapTbl;
}

#pragma force_active on

