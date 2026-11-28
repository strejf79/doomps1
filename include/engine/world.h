/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Sector / portal world + collision queries.
 */
#ifndef ENGINE_WORLD_H
#define ENGINE_WORLD_H

#include "engine/types.h"

typedef struct Wall {
	i16 x0, z0;
	i16 x1, z1;
	i16 portal_to; /* -1 = solid; else destination sector index */
	u8  r, g, b;
} Wall;

typedef struct Sector {
	i16 floor_y;
	i16 ceil_y;
	u8  floor_r, floor_g, floor_b;
	u8  ceil_r, ceil_g, ceil_b;
	i16 min_x, max_x;
	i16 min_z, max_z;
	u16 wall_begin;
	u16 wall_count;
} Sector;

typedef struct World {
	const Sector *sectors;
	const Wall   *walls;
	u16 sector_count;
	u16 wall_count;
} World;

void world_init_tworoom(World *out);

/* Point-in-sector AABB (sufficient for axis-aligned M1 rooms). */
i16 world_find_sector(const World *w, i32 x, i32 z);

/*
 * Flood portals from camera sector into vis[] (capped).
 * M1: no frustum clip yet — both rooms via the shared portal when open.
 */
u16 world_portal_flood(const World *w, i16 cam_sector, i16 *vis, u16 vis_cap);

/*
 * Move a circle of radius `radius` from (x,z) by (dx,dz) inside the world.
 * Updates *sector on portal cross. Returns new x/z via out pointers.
 */
void world_move_cylinder(
	const World *w,
	i16 *sector,
	i32 *x, i32 *z,
	i32 dx, i32 dz,
	i32 radius
);

#endif /* ENGINE_WORLD_H */
