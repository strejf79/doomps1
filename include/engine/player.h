/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Player — movement + combat vitals.
 */
#ifndef ENGINE_PLAYER_H
#define ENGINE_PLAYER_H

#include "engine/pad.h"
#include "engine/types.h"
#include "engine/world.h"

typedef struct Player {
	i32 x, y, z;
	i16 yaw;
	i16 sector;
	i16 health;
	i16 ammo;
	u8  flash; /* screen flash timer */
} Player;

void player_init(Player *p, const World *w);
void player_tick(Player *p, const World *w, const PadState *pad);

#endif /* ENGINE_PLAYER_H */
