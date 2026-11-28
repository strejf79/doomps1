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
#define MAPBIN_VERSION 2u

/* Thing types in cooked maps (Doom-compatible where noted). */
#define THING_PLAYER   1
#define THING_EXIT     999
#define THING_ZOMBIE   3004
#define THING_IMP      3001
#define THING_SHOTGUN  2001
#define THING_AMMO     2007
#define THING_HEALTH   2014
#define THING_CHAINGUN 2002

#define MAP_BUDGET_SECTORS 24
#define MAP_BUDGET_WALLS   384
#define MAP_BUDGET_THINGS  32
#define MAP_BUDGET_BYTES   12288

#pragma pack(push, 1)
typedef struct MapBinHeader {
	u32 magic;
	u16 version;
	u16 sector_count;
	u16 wall_count;
	u16 thing_count;
	i16 spawn_x, spawn_z;
	i16 spawn_yaw;
	i16 spawn_sector;
	i16 exit_x, exit_z;
	i16 exit_sector; /* -1 if none */
	u16 flags;
	u16 reserved;
} MapBinHeader;

typedef struct MapBinSector {
	i16 floor_y, ceil_y;
	u8  color_id, brightness;
	u8  light_r, light_g, light_b;
	u8  lod; /* 0 = full, 1 = far LOD (flat only) */
	i16 min_x, max_x, min_z, max_z;
	u16 wall_begin, wall_count;
} MapBinSector;

typedef struct MapBinWall {
	i16 x0, z0, x1, z1;
	i16 portal_to;
	u8  ar, ag, ab;
	u8  flags; /* bit0 = door candidate (M4) */
} MapBinWall;

typedef struct MapBinThing {
	u16 type;
	i16 x, z;
	i16 angle;
	i16 sector;
	u16 flags;
} MapBinThing;
#pragma pack(pop)

typedef struct MapThing {
	u16 type;
	i16 x, z;
	i16 angle;
	i16 sector;
	u16 flags;
} MapThing;

typedef struct MapRuntime {
	World world;
	Sector *sectors;
	Wall   *walls;
	MapThing *things;
	u16 thing_count;
	i16 spawn_x, spawn_z, spawn_yaw, spawn_sector;
	i16 exit_x, exit_z, exit_sector;
	u16 flags;
	bool exited;
} MapRuntime;

int mapbin_load(const void *data, u32 size, MapRuntime *out);
const u8 *mapbin_embedded(u32 *out_size);
void map_apply_spawn(const MapRuntime *map, Player *player);
void map_check_exit(MapRuntime *map, const Player *player);

#endif /* ENGINE_MAPBIN_H */
