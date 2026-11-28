/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
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
	bool fire;
	bool fire_pressed;
	bool use_pressed;   /* Triangle — doors */
	bool weapon_next;   /* Circle — cycle weapons */
	bool select_pressed; /* Select — next map */
} PadState;

void pad_init(void);
void pad_poll(PadState *out);

#endif /* ENGINE_PAD_H */
