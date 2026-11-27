/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Platform boot: GPU reset and VSync helper.
 */
#include "engine/platform.h"

#include <psxgpu.h>

void plat_init(void)
{
	ResetGraph(0);
}

void plat_vsync(void)
{
	VSync(0);
}
