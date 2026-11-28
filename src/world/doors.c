/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Doors (toggle portal) + simple elevators (floor bob).
 */
#include "engine/doors.h"

#include <stdio.h>
#include <string.h>

void movers_init(MoverWorld *mw, World *world)
{
	u16 i;
	u16 d = 0;
	u16 e = 0;

	memset(mw, 0, sizeof(*mw));
	for (i = 0; i < 512; i++) {
		mw->wall_portal_override[i] = -2;
	}

	for (i = 0; i < world->wall_count && d < MAX_DOORS; i++) {
		Wall *w = (Wall *)&world->walls[i];
		if (!(w->flags & WALL_FLAG_DOOR) || w->portal_to < 0) {
			continue;
		}
		mw->doors[d].active = true;
		mw->doors[d].open = false;
		mw->doors[d].wall_index = i;
		mw->doors[d].sector_a = -1;
		mw->doors[d].sector_b = w->portal_to;
		mw->doors[d].timer = 0;
		/* Start closed: block portal. */
		mw->doors[d].sector_b = w->portal_to;
		w->portal_to = -1;
		d++;
	}
	mw->door_count = d;

	/* Elevators: every 3rd sector gets a mild lift for demo feel. */
	for (i = 0; i < world->sector_count && e < MAX_ELEVATORS; i++) {
		Sector *s = (Sector *)&world->sectors[i];
		if ((i % 3) != 2) {
			continue;
		}
		mw->elevators[e].active = true;
		mw->elevators[e].sector = (i16)i;
		mw->elevators[e].floor_base = s->floor_y;
		mw->elevators[e].floor_high = (i16)(s->floor_y - 24);
		mw->elevators[e].floor_cur = s->floor_y;
		mw->elevators[e].dir = 0;
		mw->elevators[e].wait = 0;
		e++;
	}
	mw->elev_count = e;
	printf("movers: %u doors, %u elevators\n", d, e);
}

static int near_wall(const Player *player, const Wall *w)
{
	i32 px = player->x >> 12;
	i32 pz = player->z >> 12;
	i32 mx = (w->x0 + w->x1) / 2;
	i32 mz = (w->z0 + w->z1) / 2;
	i32 dx = px - mx;
	i32 dz = pz - mz;
	if (dx < 0) dx = -dx;
	if (dz < 0) dz = -dz;
	return (dx + dz) < 48;
}

void movers_try_use(MoverWorld *mw, World *world, const Player *player)
{
	u16 i;
	for (i = 0; i < mw->door_count; i++) {
		Door *d = &mw->doors[i];
		Wall *w;
		if (!d->active || d->timer > 0) {
			continue;
		}
		w = (Wall *)&world->walls[d->wall_index];
		if (!near_wall(player, w)) {
			continue;
		}
		d->open = !d->open;
		if (d->open) {
			w->portal_to = d->sector_b;
		} else {
			w->portal_to = -1;
		}
		d->timer = 20;
		printf("door %u %s\n", i, d->open ? "OPEN" : "CLOSED");
		return;
	}

	/* Use near elevator: start motion */
	for (i = 0; i < mw->elev_count; i++) {
		Elevator *el = &mw->elevators[i];
		const Sector *s;
		i32 px, pz;
		if (!el->active || el->dir != 0) {
			continue;
		}
		s = &world->sectors[el->sector];
		px = player->x >> 12;
		pz = player->z >> 12;
		if (px >= s->min_x && px <= s->max_x && pz >= s->min_z && pz <= s->max_z) {
			el->dir = (el->floor_cur > el->floor_high) ? 1 : -1;
			return;
		}
	}
}

void movers_tick(MoverWorld *mw, World *world)
{
	u16 i;
	for (i = 0; i < mw->door_count; i++) {
		if (mw->doors[i].timer > 0) {
			mw->doors[i].timer--;
		}
	}
	for (i = 0; i < mw->elev_count; i++) {
		Elevator *el = &mw->elevators[i];
		Sector *s;
		if (!el->active || el->dir == 0) {
			continue;
		}
		s = (Sector *)&world->sectors[el->sector];
		el->floor_cur = (i16)(el->floor_cur - el->dir); /* dir+ → more negative = rise in Y-down */
		s->floor_y = el->floor_cur;
		if (el->dir > 0 && el->floor_cur <= el->floor_high) {
			el->floor_cur = el->floor_high;
			s->floor_y = el->floor_cur;
			el->dir = 0;
			el->wait = 60;
		} else if (el->dir < 0 && el->floor_cur >= el->floor_base) {
			el->floor_cur = el->floor_base;
			s->floor_y = el->floor_cur;
			el->dir = 0;
		}
		if (el->wait > 0) {
			el->wait--;
			if (el->wait == 0 && el->floor_cur <= el->floor_high) {
				el->dir = -1; /* return down after wait */
			}
		}
	}
}

i16 movers_effective_portal(const MoverWorld *mw, u16 wall_index, i16 map_portal)
{
	(void)mw;
	(void)wall_index;
	return map_portal;
}
