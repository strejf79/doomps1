/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 */
#include "engine/player.h"
#include "engine/audio.h"

#include <psxgte.h>

#define EYE_HEIGHT    80
#define MOVE_SPEED    6
#define TURN_SPEED    32
#define PLAYER_RADIUS 12
#define FOOTSTEP_TICKS 12

static u16 s_foot_cd;

void player_init(Player *p, const World *w)
{
	(void)w;
	p->x = 0;
	p->y = (-EYE_HEIGHT) << 12;
	p->z = 0;
	p->yaw = 0;
	p->sector = 0;
	p->health = 100;
	p->ammo = 50;
	p->flash = 0;
	p->weapon = WPN_PISTOL;
	p->has_weapon[WPN_PISTOL] = 1;
	p->has_weapon[WPN_SHOTGUN] = 0;
	p->has_weapon[WPN_CHAINGUN] = 0;
	p->fire_cooldown = 0;
	s_foot_cd = 0;
}

void player_tick(Player *p, const World *w, const PadState *pad)
{
	i32 dx = 0, dz = 0, wx, wz;
	i16 yaw;
	u8 i;
	bool moving;

	if (p->flash > 0) p->flash--;
	if (p->fire_cooldown > 0) p->fire_cooldown--;
	if (s_foot_cd > 0) s_foot_cd--;
	if (p->health <= 0) return;

	if (pad->weapon_next) {
		for (i = 1; i <= WPN_COUNT; i++) {
			WeaponId next = (WeaponId)((p->weapon + i) % WPN_COUNT);
			if (p->has_weapon[next]) {
				p->weapon = next;
				break;
			}
		}
	}

	if (pad->turn < 0) p->yaw = (i16)((p->yaw + TURN_SPEED) & 4095);
	else if (pad->turn > 0) p->yaw = (i16)((p->yaw - TURN_SPEED) & 4095);

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

	moving = (dx != 0 || dz != 0);
	wx = p->x >> 12;
	wz = p->z >> 12;
	world_move_cylinder(w, &p->sector, &wx, &wz, dx, dz, PLAYER_RADIUS);
	p->x = wx << 12;
	p->z = wz << 12;
	if (p->sector >= 0) {
		p->y = (w->sectors[p->sector].floor_y - EYE_HEIGHT) << 12;
	}
	if (moving && s_foot_cd == 0) {
		audio_play_sfx(SFX_FOOTSTEP);
		s_foot_cd = FOOTSTEP_TICKS;
	}
}
