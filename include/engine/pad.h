/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Doom-like pad — tank turn + forward/back; strafe; fire.
 */
#ifndef ENGINE_PAD_H
#define ENGINE_PAD_H

#include "engine/types.h"

typedef struct PadState {
	i16 forward;
	i16 turn;
	i16 strafe;
	u16 buttons;
	bool connected;
	bool fire;         /* held */
	bool fire_pressed; /* edge */
} PadState;

void pad_init(void);
void pad_poll(PadState *out);

#endif /* ENGINE_PAD_H */
