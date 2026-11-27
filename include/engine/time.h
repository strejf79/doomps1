/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Fixed 30 Hz simulation clock (stub for M0).
 */
#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

#include "engine/types.h"

void time_init(void);

/* Returns how many sim ticks to run this frame (0–2). M0 always returns 1. */
u32 time_begin_frame(void);

#endif /* ENGINE_TIME_H */
