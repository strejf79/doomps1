/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Fixed-tick stub — always one sim step per frame in M0.
 */
#include "engine/time.h"

void time_init(void)
{
}

u32 time_begin_frame(void)
{
	return 1u;
}
