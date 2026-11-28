/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Boot / platform — GPU reset, VSync, thin PSn00bSDK wrappers.
 */
#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

#include "engine/types.h"

void plat_init(void);
void plat_vsync(void);

#endif /* ENGINE_PLATFORM_H */
