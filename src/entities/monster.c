/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Monsters from cooked map things; optional staggered AI.
 */
#include "engine/entities.h"
#include "engine/audio.h"

#include <stdio.h>
#include <string.h>

#define MON_RADIUS   14
#define MON_SPEED    3
#define ATK_RANGE   90
#define CHASE_RANGE  400
#define ATK_COOLDOWN 40

static i32 iabs32(i32 v) { return v < 0 ? -v : v; }

static i32 dist_approx(i32 ax, i32 az, i32 bx, i32 bz)
{
	i32 dx = iabs32((ax >> 12) - (bx >> 12));
	i32 dz = iabs32((az >> 12) - (bz >> 12));
	return dx > dz ? dx + (dz >> 1) : dz + (dx >> 1);
}

void entities_init_from_map(EntityWorld *ew, const MapRuntime *map)
{
	u16 i;
	u16 n = 0;

	memset(ew, 0, sizeof(*ew));

	for (i = 0; i < map->thing_count && n < MAX_MONSTERS; i++) {
		const MapThing *t = &map->things[i];
		Monster *m;
		u8 class_id;
		i16 hp;

		if (t->type == THING_ZOMBIE || t->type == 9) {
			class_id = 0;
			hp = 30;
		} else if (t->type == THING_IMP || t->type == 3001) {
			class_id = 1;
			hp = 40;
		} else {
			continue;
		}

		m = &ew->monsters[n];
		m->active = true;
		m->x = ((i32)t->x) << 12;
		m->z = ((i32)t->z) << 12;
		m->sector = t->sector;
		m->health = hp;
		m->state = MON_IDLE;
		m->state_t = 0;
		m->cooldown = 0;
		m->anim = 0;
		m->class_id = class_id;
		m->think_slot = (u8)(n & 7);
		n++;
	}

	/* Fallback: one grunt ahead of spawn if map has no enemies. */
	if (n == 0) {
		Monster *m = &ew->monsters[0];
		i32 mx = map->spawn_x;
		i32 mz = map->spawn_z + 60;
		m->active = true;
		m->x = mx << 12;
		m->z = mz << 12;
		m->sector = map->spawn_sector;
		m->health = 40;
		m->state = MON_IDLE;
		m->class_id = 0;
		m->think_slot = 0;
		n = 1;
		printf("no map enemies — spawned fallback grunt\n");
	}

	ew->monster_count = n;
	ew->think_cursor = 0;
	printf("entities: %u monsters from map\n", n);
}

static void mon_set_state(Monster *m, MonsterState st)
{
	m->state = st;
	m->state_t = 0;
}

void entities_tick(EntityWorld *ew, const World *w, Player *player, bool staggered)
{
	u16 i;
	u16 slot = (u16)(ew->think_cursor & 7);

	ew->think_cursor++;

	for (i = 0; i < MAX_MONSTERS; i++) {
		Monster *m = &ew->monsters[i];
		i32 d, wx, wz, dx, dz, step_x, step_z;
		i16 atk_dmg;

		if (!m->active || m->state == MON_DEAD) {
			continue;
		}

		/* Always age timers; only heavy chase AI is staggered. */
		m->state_t++;
		m->anim++;
		if (m->cooldown > 0) {
			m->cooldown--;
		}

		if (staggered && m->state == MON_CHASE && m->think_slot != slot) {
			continue;
		}

		d = dist_approx(m->x, m->z, player->x, player->z);
		atk_dmg = (m->class_id == 1) ? 8 : 5;

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
				player->health -= atk_dmg;
				if (player->health < 0) {
					player->health = 0;
				}
				player->flash = 8;
				m->cooldown = ATK_COOLDOWN;
				audio_play_sfx(SFX_HURT);
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
