/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Hitscan — ray in XZ vs monster cylinders.
 */
#include "engine/entities.h"

#include <psxgte.h>

#define HIT_DAMAGE   15
#define HIT_RANGE    500
#define MON_HIT_RAD  18

static i32 iabs32(i32 v)
{
	return v < 0 ? -v : v;
}

i16 weapon_hitscan(EntityWorld *ew, const World *w, Player *player)
{
	i32 ox, oz, dx, dz;
	i16 best = -1;
	i32 best_t = HIT_RANGE;
	u16 i;
	(void)w;

	if (player->ammo <= 0 || player->health <= 0) {
		return 0;
	}

	player->ammo--;
	player->flash = 3; /* muzzle feedback (silent) */

	ox = player->x >> 12;
	oz = player->z >> 12;
	/* Ray dir matching movement convention. */
	dx = -isin(player->yaw);
	dz = icos(player->yaw);

	for (i = 0; i < MAX_MONSTERS; i++) {
		Monster *m = &ew->monsters[i];
		i32 mx, mz, fx, fz, t, px, pz, dist;

		if (!m->active || m->state == MON_DEAD) {
			continue;
		}

		mx = m->x >> 12;
		mz = m->z >> 12;
		fx = mx - ox;
		fz = mz - oz;

		/* Project onto look dir (ONE-space). */
		t = (fx * dx + fz * dz) >> 12;
		if (t < 8 || t > HIT_RANGE) {
			continue;
		}

		px = ox + ((dx * t) >> 12);
		pz = oz + ((dz * t) >> 12);
		dist = iabs32(px - mx) + iabs32(pz - mz);
		if (dist > MON_HIT_RAD) {
			continue;
		}
		if (t < best_t) {
			best_t = t;
			best = (i16)i;
		}
	}

	if (best < 0) {
		return 0;
	}

	{
		Monster *m = &ew->monsters[best];
		m->health -= HIT_DAMAGE;
		if (m->health <= 0) {
			m->health = 0;
			m->state = MON_DEAD;
			m->state_t = 0;
		} else {
			m->state = MON_PAIN;
			m->state_t = 0;
		}
	}
	return HIT_DAMAGE;
}
