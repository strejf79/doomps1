/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Entity pools — monsters from cooked map things.
 */
#ifndef ENGINE_ENTITIES_H
#define ENGINE_ENTITIES_H

#include "engine/mapbin.h"
#include "engine/player.h"
#include "engine/types.h"
#include "engine/world.h"

#define MAX_MONSTERS 24

typedef enum MonsterState {
	MON_IDLE = 0,
	MON_CHASE,
	MON_ATTACK,
	MON_PAIN,
	MON_DEAD
} MonsterState;

typedef struct Monster {
	bool active;
	i32 x, z;
	i16 sector;
	i16 health;
	MonsterState state;
	u16 state_t;
	u16 cooldown;
	u8  anim;
	u8  class_id; /* 0 zombie, 1 imp */
	u8  think_slot; /* staggered AI slot 0..7 */
} Monster;

typedef struct EntityWorld {
	Monster monsters[MAX_MONSTERS];
	u16 monster_count;
	u16 think_cursor;
} EntityWorld;

void entities_init_from_map(EntityWorld *ew, const MapRuntime *map);
void entities_tick(EntityWorld *ew, const World *w, Player *player, bool staggered);

i16 weapon_hitscan(EntityWorld *ew, const World *w, Player *player);
i16 weapon_shotgun(EntityWorld *ew, const World *w, Player *player);
i16 weapon_chaingun(EntityWorld *ew, const World *w, Player *player);

#endif /* ENGINE_ENTITIES_H */
