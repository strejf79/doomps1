/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * One monster class — Idle → Chase → Attack → Pain → Dead.
 */
#include "engine/entities.h"

#include <stdio.h>
#include <string.h>

#define MON_RADIUS   14
#define MON_SPEED    3
#define MON_HP       40
#define ATK_DAMAGE  5
#define ATK_RANGE   90
#define CHASE_RANGE  400
#define ATK_COOLDOWN 40

static i32 iabs32(i32 v)
{
	return v < 0 ? -v : v;
}

static i32 dist_approx(i32 ax, i32 az, i32 bx, i32 bz)
{
	i32 dx = iabs32((ax >> 12) - (bx >> 12));
	i32 dz = iabs32((az >> 12) - (bz >> 12));
	return dx > dz ? dx + (dz >> 1) : dz + (dx >> 1);
}

void entities_init(EntityWorld *ew, const World *w, const Player *player)
{
	Monster *m;
	i16 sec;
	i32 mx, mz;
	u16 i;

	memset(ew, 0, sizeof(*ew));

	sec = player->sector;
	if (sec < 0) {
		sec = 0;
	}
	mx = player->x >> 12;
	mz = (player->z >> 12) + 80;
	if ((u16)sec < w->sector_count) {
		const Sector *s = &w->sectors[sec];
		if (mx < s->min_x + 20) mx = s->min_x + 20;
		if (mx > s->max_x - 20) mx = s->max_x - 20;
		if (mz < s->min_z + 20) mz = s->min_z + 20;
		if (mz > s->max_z - 20) mz = s->max_z - 20;
	}

	m = &ew->monsters[0];
	m->active = true;
	m->x = mx << 12;
	m->z = mz << 12;
	m->sector = sec;
	m->health = MON_HP;
	m->state = MON_IDLE;
	m->state_t = 0;
	m->cooldown = 0;
	m->anim = 0;
	ew->monster_count = 1;

	for (i = 1; i < MAX_MONSTERS; i++) {
		ew->monsters[i].active = false;
	}

	printf("spawned monster at %d,%d sec=%d\n", mx, mz, sec);
}

static void mon_set_state(Monster *m, MonsterState st)
{
	m->state = st;
	m->state_t = 0;
}

void entities_tick(EntityWorld *ew, const World *w, Player *player)
{
	u16 i;

	for (i = 0; i < MAX_MONSTERS; i++) {
		Monster *m = &ew->monsters[i];
		i32 d;
		i32 wx, wz;
		i32 dx, dz;
		i32 step_x, step_z;

		if (!m->active || m->state == MON_DEAD) {
			continue;
		}

		m->state_t++;
		m->anim++;
		if (m->cooldown > 0) {
			m->cooldown--;
		}

		d = dist_approx(m->x, m->z, player->x, player->z);

		switch (m->state) {
		case MON_IDLE:
			if (d < CHASE_RANGE && player->health > 0) {
				mon_set_state(m, MON_CHASE);
			}
			break;

		case MON_CHASE:
			if (player->health <= 0) {
				mon_set_state(m, MON_IDLE);
				break;
			}
			if (d < ATK_RANGE) {
				mon_set_state(m, MON_ATTACK);
				break;
			}
			if (d > CHASE_RANGE + 80) {
				mon_set_state(m, MON_IDLE);
				break;
			}
			dx = (player->x >> 12) - (m->x >> 12);
			dz = (player->z >> 12) - (m->z >> 12);
			step_x = 0;
			step_z = 0;
			if (iabs32(dx) > iabs32(dz)) {
				step_x = (dx > 0) ? MON_SPEED : -MON_SPEED;
			} else if (dz != 0) {
				step_z = (dz > 0) ? MON_SPEED : -MON_SPEED;
			}
			wx = m->x >> 12;
			wz = m->z >> 12;
			world_move_cylinder(w, &m->sector, &wx, &wz, step_x, step_z, MON_RADIUS);
			m->x = wx << 12;
			m->z = wz << 12;
			break;

		case MON_ATTACK:
			if (d > ATK_RANGE + 20) {
				mon_set_state(m, MON_CHASE);
				break;
			}
			if (m->cooldown == 0 && player->health > 0) {
				player->health -= ATK_DAMAGE;
				if (player->health < 0) {
					player->health = 0;
				}
				player->flash = 8;
				m->cooldown = ATK_COOLDOWN;
			}
			if (m->state_t > 20) {
				mon_set_state(m, MON_CHASE);
			}
			break;

		case MON_PAIN:
			if (m->state_t > 12) {
				mon_set_state(m, (d < ATK_RANGE) ? MON_ATTACK : MON_CHASE);
			}
			break;

		default:
			break;
		}
	}
}
