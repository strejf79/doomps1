/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Doom-like pad stub — tank turn + forward/back; strafe on shoulder.
 */
#ifndef ENGINE_PAD_H
#define ENGINE_PAD_H

#include "engine/types.h"

typedef struct PadState {
	i16 forward; /* -1, 0, +1 */
	i16 turn;    /* -1, 0, +1 (tank turn) */
	i16 strafe;  /* -1, 0, +1 (L1/R1 or L2/R2) */
	u16 buttons; /* raw mask for debug */
	bool connected;
} PadState;

void pad_init(void);
void pad_poll(PadState *out);

#endif /* ENGINE_PAD_H */
