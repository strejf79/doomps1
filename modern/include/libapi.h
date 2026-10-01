#pragma once

#include <psxapi.h>

/* PSn00bSDK installs its IRQ/event handlers before main(). The Psy-Q startup
   call would remove those handlers and make VSync, CD and DMA waits hang. */
int PsyqResetCallback(void);
#define ResetCallback PsyqResetCallback
