#pragma once

#include <stdint.h>
#include <psxapi.h>

#ifndef getScratchAddr
#define getScratchAddr(offset) \
	((unsigned long *)(0x1f800000u + ((unsigned long)(offset) << 2)))
#endif
