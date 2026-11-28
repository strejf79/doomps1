/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Cooked .map.bin layout (little-endian). Host cooker + runtime loader.
 */
#ifndef ENGINE_MAPBIN_H
#define ENGINE_MAPBIN_H

#include "engine/types.h"
#include "engine/world.h"
#include "engine/player.h"

#define MAPBIN_MAGIC   0x314D5044u /* 'DPM1' LE */
#define MAPBIN_VERSION 1u

#pragma pack(push, 1)
typedef struct MapBinHeader {
	u32 magic;
	u16 version;
	u16 sector_count;
	u16 wall_count;
	i16 spawn_x, spawn_z;
	i16 spawn_yaw;
	i16 spawn_sector;
	u16 reserved;
} MapBinHeader;

typedef struct MapBinSector {
	i16 floor_y, ceil_y;
	u8  color_id, brightness;
	u8  light_r, light_g, light_b;
	u8  pad;
	i16 min_x, max_x, min_z, max_z;
	u16 wall_begin, wall_count;
} MapBinSector;

typedef struct MapBinWall {
	i16 x0, z0, x1, z1;
	i16 portal_to;
	u8  ar, ag, ab;
	u8  pad;
} MapBinWall;
#pragma pack(pop)

typedef struct MapRuntime {
	World world;
	Sector *sectors; /* owned heap from load — static arena for M2 */
	Wall   *walls;
	i16 spawn_x, spawn_z, spawn_yaw, spawn_sector;
} MapRuntime;

/* Load from memory blob into out (uses static arenas — one map at a time). */
int mapbin_load(const void *data, u32 size, MapRuntime *out);

/* Built-in cooked blob (generated / authored). */
const u8 *mapbin_embedded(u32 *out_size);

void map_apply_spawn(const MapRuntime *map, Player *player);

#endif /* ENGINE_MAPBIN_H */
