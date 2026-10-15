#include "cps/cps_internal.h"

#pragma force_active on

// Gives the other threads a chance to run (CpsConfig.yieldSleep: sleep instead of yielding).
void OS_YieldThread__(void) {
    if (gCpsState.yieldSleep == 0) {
        OS_YieldThread();
    } else {
        OS_Sleep(gCpsState.yieldSleep);
    }
}
