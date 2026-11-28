/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Sector / portal world + collision + static light schema.
 */
#ifndef ENGINE_WORLD_H
#define ENGINE_WORLD_H

#include "engine/types.h"

typedef struct Rgb8 {
	u8 r, g, b;
} Rgb8;

typedef struct Wall {
	i16 x0, z0;
	i16 x1, z1;
	i16 portal_to; /* -1 = solid; else destination sector index */
	/* Albedo (modulated by sector/vertex light at submit). */
	u8  ar, ag, ab;
} Wall;

typedef struct Sector {
	i16 floor_y;
	i16 ceil_y;
	/* Static colored lighting (PSX Doom-style). */
	u8  color_id;
	u8  brightness; /* 0–255 */
	Rgb8 light;     /* resolved rgb * brightness (cooked / authored) */
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

i16 world_find_sector(const World *w, i32 x, i32 z);

u16 world_portal_flood(const World *w, i16 cam_sector, i16 *vis, u16 vis_cap);

void world_move_cylinder(
	const World *w,
	i16 *sector,
	i32 *x, i32 *z,
	i32 dx, i32 dz,
	i32 radius
);

/* Resolve vert light: sector light, or average with neighbor if on a portal edge. */
Rgb8 world_vert_light(const World *w, i16 sector, i16 x, i16 z);

/* Modulate albedo × light → packet RGB. */
Rgb8 world_shade(Rgb8 albedo, Rgb8 light);

#endif /* ENGINE_WORLD_H */
