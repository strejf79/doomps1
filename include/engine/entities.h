/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Entity pools — M3: one monster class.
 */
#ifndef ENGINE_ENTITIES_H
#define ENGINE_ENTITIES_H

#include "engine/player.h"
#include "engine/types.h"
#include "engine/world.h"

#define MAX_MONSTERS 8

typedef enum MonsterState {
	MON_IDLE = 0,
	MON_CHASE,
	MON_ATTACK,
	MON_PAIN,
	MON_DEAD
} MonsterState;

typedef struct Monster {
	bool active;
	i32 x, z;      /* 12.12 */
	i16 sector;
	i16 health;
	MonsterState state;
	u16 state_t;   /* ticks in state */
	u16 cooldown;  /* attack cooldown */
	u8  anim;
} Monster;

typedef struct EntityWorld {
	Monster monsters[MAX_MONSTERS];
	u16 monster_count;
} EntityWorld;

void entities_init(EntityWorld *ew, const World *w, const Player *player);
void entities_tick(EntityWorld *ew, const World *w, Player *player);

/* Hitscan from player eye along yaw; returns damage dealt (0 if miss). */
i16 weapon_hitscan(EntityWorld *ew, const World *w, Player *player);

#endif /* ENGINE_ENTITIES_H */
