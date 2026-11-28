/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Doom-like tank controls + vitals.
 */
#include "engine/player.h"

#include <psxgte.h>

#define EYE_HEIGHT    80
#define MOVE_SPEED    6
#define TURN_SPEED    32
#define PLAYER_RADIUS 12

void player_init(Player *p, const World *w)
{
	(void)w;
	p->x = 0 << 12;
	p->y = (-EYE_HEIGHT) << 12;
	p->z = 0 << 12;
	p->yaw = 0;
	p->sector = 0;
	p->health = 100;
	p->ammo = 50;
	p->flash = 0;
}

void player_tick(Player *p, const World *w, const PadState *pad)
{
	i32 dx = 0;
	i32 dz = 0;
	i32 wx, wz;
	i16 yaw;

	if (p->flash > 0) {
		p->flash--;
	}

	if (p->health <= 0) {
		return;
	}

	if (pad->turn < 0) {
		p->yaw = (i16)((p->yaw + TURN_SPEED) & 4095);
	} else if (pad->turn > 0) {
		p->yaw = (i16)((p->yaw - TURN_SPEED) & 4095);
	}

	yaw = p->yaw;

	if (pad->forward > 0) {
		dx -= (isin(yaw) * MOVE_SPEED) >> 12;
		dz += (icos(yaw) * MOVE_SPEED) >> 12;
	} else if (pad->forward < 0) {
		dx += (isin(yaw) * MOVE_SPEED) >> 12;
		dz -= (icos(yaw) * MOVE_SPEED) >> 12;
	}

	if (pad->strafe > 0) {
		dx += (icos(yaw) * MOVE_SPEED) >> 12;
		dz += (isin(yaw) * MOVE_SPEED) >> 12;
	} else if (pad->strafe < 0) {
		dx -= (icos(yaw) * MOVE_SPEED) >> 12;
		dz -= (isin(yaw) * MOVE_SPEED) >> 12;
	}

	wx = p->x >> 12;
	wz = p->z >> 12;
	world_move_cylinder(w, &p->sector, &wx, &wz, dx, dz, PLAYER_RADIUS);
	p->x = wx << 12;
	p->z = wz << 12;

	if (p->sector >= 0) {
		p->y = (w->sectors[p->sector].floor_y - EYE_HEIGHT) << 12;
	}
}
