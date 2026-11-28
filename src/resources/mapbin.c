/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Load cooked .map.bin into runtime World.
 */
#include "engine/mapbin.h"

#include <stdio.h>
#include <string.h>

#define MAX_LOAD_SECTORS 48
#define MAX_LOAD_WALLS   512

static Sector g_sec_arena[MAX_LOAD_SECTORS];
static Wall   g_wall_arena[MAX_LOAD_WALLS];

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

	if (size < sizeof(MapBinHeader)) {
		printf("mapbin: too small\n");
		return -1;
	}

	hdr = (const MapBinHeader *)p;
	if (hdr->magic != MAPBIN_MAGIC || hdr->version != MAPBIN_VERSION) {
		printf("mapbin: bad magic/version\n");
		return -1;
	}
	if (hdr->sector_count == 0 || hdr->sector_count > MAX_LOAD_SECTORS
		|| hdr->wall_count > MAX_LOAD_WALLS) {
		printf("mapbin: counts out of range\n");
		return -1;
	}

	need = (u32)sizeof(MapBinHeader)
		+ (u32)hdr->sector_count * (u32)sizeof(MapBinSector)
		+ (u32)hdr->wall_count * (u32)sizeof(MapBinWall);
	if (size < need) {
		printf("mapbin: truncated (%u < %u)\n", size, need);
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
		p += sizeof(MapBinWall);
	}

	out->sectors = g_sec_arena;
	out->walls = g_wall_arena;
	out->world.sectors = g_sec_arena;
	out->world.walls = g_wall_arena;
	out->world.sector_count = hdr->sector_count;
	out->world.wall_count = hdr->wall_count;
	out->spawn_x = hdr->spawn_x;
	out->spawn_z = hdr->spawn_z;
	out->spawn_yaw = hdr->spawn_yaw;
	out->spawn_sector = hdr->spawn_sector;
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
