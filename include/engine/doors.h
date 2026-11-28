/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Simple door / elevator movers.
 */
#ifndef ENGINE_DOORS_H
#define ENGINE_DOORS_H

#include "engine/player.h"
#include "engine/types.h"
#include "engine/world.h"

#include <stdbool.h>

#define MAX_DOORS 16
#define MAX_ELEVATORS 8

typedef struct Door {
	bool active;
	bool open;
	u16 wall_index; /* in world.walls */
	i16 sector_a, sector_b;
	u16 timer;
} Door;

typedef struct Elevator {
	bool active;
	i16 sector;
	i16 floor_base;
	i16 floor_high;
	i16 floor_cur;
	i8  dir; /* 1 up (more negative Y), -1 down, 0 idle */
	u16 wait;
} Elevator;

typedef struct MoverWorld {
	Door doors[MAX_DOORS];
	Elevator elevators[MAX_ELEVATORS];
	u16 door_count;
	u16 elev_count;
	/* Mutable wall portal overrides: -2 = use map, else forced portal_to */
	i16 wall_portal_override[512];
} MoverWorld;

void movers_init(MoverWorld *mw, World *world);
void movers_try_use(MoverWorld *mw, World *world, const Player *player);
void movers_tick(MoverWorld *mw, World *world);
i16 movers_effective_portal(const MoverWorld *mw, u16 wall_index, i16 map_portal);

#endif /* ENGINE_DOORS_H */
