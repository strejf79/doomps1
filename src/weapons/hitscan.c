/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Pistol / shotgun / chaingun hitscan.
 */
#include "engine/entities.h"

#include <psxgte.h>

#define MON_HIT_RAD 18

static i32 iabs32(i32 v) { return v < 0 ? -v : v; }

static i16 hitscan_ray(
	EntityWorld *ew,
	Player *player,
	i16 yaw,
	i16 damage,
	i16 range
)
{
	i32 ox = player->x >> 12;
	i32 oz = player->z >> 12;
	i32 dx = -isin(yaw);
	i32 dz = icos(yaw);
	i16 best = -1;
	i32 best_t = range;
	u16 i;

	for (i = 0; i < MAX_MONSTERS; i++) {
		Monster *m = &ew->monsters[i];
		i32 mx, mz, fx, fz, t, px, pz, dist;
		if (!m->active || m->state == MON_DEAD) continue;
		mx = m->x >> 12;
		mz = m->z >> 12;
		fx = mx - ox;
		fz = mz - oz;
		t = (fx * dx + fz * dz) >> 12;
		if (t < 8 || t > range) continue;
		px = ox + ((dx * t) >> 12);
		pz = oz + ((dz * t) >> 12);
		dist = iabs32(px - mx) + iabs32(pz - mz);
		if (dist > MON_HIT_RAD) continue;
		if (t < best_t) {
			best_t = t;
			best = (i16)i;
		}
	}
	if (best < 0) return 0;
	{
		Monster *m = &ew->monsters[best];
		m->health -= damage;
		if (m->health <= 0) {
			m->health = 0;
			m->state = MON_DEAD;
			m->state_t = 0;
		} else {
			m->state = MON_PAIN;
			m->state_t = 0;
		}
	}
	return damage;
}

static i16 fire_common(Player *player, u16 cooldown, i16 ammo_cost)
{
	if (player->ammo < ammo_cost || player->health <= 0) return -1;
	if (player->fire_cooldown > 0) return -1;
	player->ammo = (i16)(player->ammo - ammo_cost);
	player->fire_cooldown = cooldown;
	player->flash = 3;
	return 0;
}

i16 weapon_hitscan(EntityWorld *ew, const World *w, Player *player)
{
	(void)w;
	if (fire_common(player, 8, 1) < 0) return -1;
	return hitscan_ray(ew, player, player->yaw, 15, 500);
}

i16 weapon_shotgun(EntityWorld *ew, const World *w, Player *player)
{
	i16 total = 0;
	i16 pel;
	(void)w;
	if (fire_common(player, 22, 2) < 0) return -1;
	for (pel = -2; pel <= 2; pel++) {
		i16 yaw = (i16)((player->yaw + pel * 48) & 4095);
		total = (i16)(total + hitscan_ray(ew, player, yaw, 8, 280));
	}
	return total;
}

i16 weapon_chaingun(EntityWorld *ew, const World *w, Player *player)
{
	(void)w;
	if (fire_common(player, 3, 1) < 0) return -1;
	return hitscan_ray(ew, player, player->yaw, 10, 450);
}
