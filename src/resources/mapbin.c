/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Load cooked DPM1 v2 .map.bin into runtime World.
 */
#include "engine/mapbin.h"

#include <stdio.h>
#include <string.h>

static Sector g_sec_arena[MAP_BUDGET_SECTORS];
static Wall g_wall_arena[MAP_BUDGET_WALLS];
static MapThing g_thing_arena[MAP_BUDGET_THINGS];

extern const u8 g_mapbin_blob[];
extern const u32 g_mapbin_blob_size;

const u8 *mapbin_embedded(u32 *out_size)
{
	*out_size = g_mapbin_blob_size;
	return g_mapbin_blob;
}

int mapbin_load(const void *data, u32 size, MapRuntime *out)
{
	const u8 *p = (const u8 *)data;
	const MapBinHeader *hdr;
	u32 need;
	u16 i;

	memset(out, 0, sizeof(*out));
	out->exit_sector = -1;

	if (size < sizeof(MapBinHeader)) {
		printf("mapbin: too small\n");
		return -1;
	}

	hdr = (const MapBinHeader *)p;
	if (hdr->magic != MAPBIN_MAGIC) {
		printf("mapbin: bad magic\n");
		return -1;
	}
	if (hdr->version != MAPBIN_VERSION) {
		printf("mapbin: unsupported version %u (need %u)\n", hdr->version, MAPBIN_VERSION);
		return -1;
	}
	if (hdr->sector_count == 0 || hdr->sector_count > MAP_BUDGET_SECTORS
		|| hdr->wall_count > MAP_BUDGET_WALLS
		|| hdr->thing_count > MAP_BUDGET_THINGS) {
		printf("mapbin: over budget counts\n");
		return -1;
	}

	need = (u32)sizeof(MapBinHeader)
		+ (u32)hdr->sector_count * (u32)sizeof(MapBinSector)
		+ (u32)hdr->wall_count * (u32)sizeof(MapBinWall)
		+ (u32)hdr->thing_count * (u32)sizeof(MapBinThing);
	if (size < need || need > MAP_BUDGET_BYTES) {
		printf("mapbin: size/budget fail (%u need, budget %u)\n", need, MAP_BUDGET_BYTES);
		return -1;
	}

	p += sizeof(MapBinHeader);
	for (i = 0; i < hdr->sector_count; i++) {
		const MapBinSector *ms = (const MapBinSector *)p;
		Sector *s = &g_sec_arena[i];
		s->floor_y = ms->floor_y;
		s->ceil_y = ms->ceil_y;
		s->color_id = ms->color_id;
		s->brightness = ms->brightness;
		s->light.r = ms->light_r;
		s->light.g = ms->light_g;
		s->light.b = ms->light_b;
		s->lod = ms->lod;
		s->min_x = ms->min_x;
		s->max_x = ms->max_x;
		s->min_z = ms->min_z;
		s->max_z = ms->max_z;
		s->wall_begin = ms->wall_begin;
		s->wall_count = ms->wall_count;
		p += sizeof(MapBinSector);
	}
	for (i = 0; i < hdr->wall_count; i++) {
		const MapBinWall *mw = (const MapBinWall *)p;
		Wall *w = &g_wall_arena[i];
		w->x0 = mw->x0;
		w->z0 = mw->z0;
		w->x1 = mw->x1;
		w->z1 = mw->z1;
		w->portal_to = mw->portal_to;
		w->ar = mw->ar;
		w->ag = mw->ag;
		w->ab = mw->ab;
		w->flags = mw->flags;
		p += sizeof(MapBinWall);
	}
	for (i = 0; i < hdr->thing_count; i++) {
		const MapBinThing *mt = (const MapBinThing *)p;
		MapThing *t = &g_thing_arena[i];
		t->type = mt->type;
		t->x = mt->x;
		t->z = mt->z;
		t->angle = mt->angle;
		t->sector = mt->sector;
		t->flags = mt->flags;
		p += sizeof(MapBinThing);
	}

	out->sectors = g_sec_arena;
	out->walls = g_wall_arena;
	out->things = g_thing_arena;
	out->thing_count = hdr->thing_count;
	out->world.sectors = g_sec_arena;
	out->world.walls = g_wall_arena;
	out->world.sector_count = hdr->sector_count;
	out->world.wall_count = hdr->wall_count;
	out->spawn_x = hdr->spawn_x;
	out->spawn_z = hdr->spawn_z;
	out->spawn_yaw = hdr->spawn_yaw;
	out->spawn_sector = hdr->spawn_sector;
	out->exit_x = hdr->exit_x;
	out->exit_z = hdr->exit_z;
	out->exit_sector = hdr->exit_sector;
	out->flags = hdr->flags;
	out->exited = false;

	printf("mapbin v2: %u secs %u walls %u things exit=%d bytes~%u\n",
		hdr->sector_count, hdr->wall_count, hdr->thing_count,
		hdr->exit_sector, need);
	return 0;
}

void map_apply_spawn(const MapRuntime *map, Player *player)
{
	player->x = ((i32)map->spawn_x) << 12;
	player->z = ((i32)map->spawn_z) << 12;
	player->yaw = map->spawn_yaw & 4095;
	player->sector = map->spawn_sector;
	if (player->sector >= 0 && (u16)player->sector < map->world.sector_count) {
		player->y = (map->world.sectors[player->sector].floor_y - 80) << 12;
	} else {
		player->y = (-80) << 12;
	}
}

void map_check_exit(MapRuntime *map, const Player *player)
{
	i32 dx, dz;

	if (map->exited || map->exit_sector < 0) {
		return;
	}
	dx = (player->x >> 12) - map->exit_x;
	dz = (player->z >> 12) - map->exit_z;
	if (dx < 0) dx = -dx;
	if (dz < 0) dz = -dz;
	if (dx + dz < 28 || player->sector == map->exit_sector) {
		if (dx + dz < 40) {
			map->exited = true;
			printf("EXIT reached\n");
		}
	}
}
