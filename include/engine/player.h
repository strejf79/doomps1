/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Player — Doom-like tank turn + forward/back; strafe on shoulders.
 */
#ifndef ENGINE_PLAYER_H
#define ENGINE_PLAYER_H

#include "engine/pad.h"
#include "engine/types.h"
#include "engine/world.h"

typedef struct Player {
	/* World position in 12.12 fixed (ONE = 4096). Y is eye height above floor. */
	i32 x, y, z;
	/* Yaw in GTE angle units (0..4095); pitch reserved. */
	i16 yaw;
	i16 sector;
} Player;

void player_init(Player *p, const World *w);
void player_tick(Player *p, const World *w, const PadState *pad);

#endif /* ENGINE_PLAYER_H */
