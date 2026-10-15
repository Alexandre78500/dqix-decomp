#include <nitro.h>

// The code below is compiled as ARM (32-bit) code.
#include <nitro/card/card_spi_wait_internal.h>
#include <nitro/code32.h>

// Card SPI waits: the wait settings and the wait between a transfer's command and its data.

extern CardSpiWait gCardSpiWait;

#pragma force_active on


void CARD_SetSpiWriteWaitCycles(u32 cycles) {
    gCardSpiWait.writeWait = cycles;
}
