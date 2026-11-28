/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Hand-authored two-room test map with distinct static tints.
 *
 * Room 0: warm red   Room 1: cool cyan
 */
#include "engine/world.h"

/* Neutral mid albedo so sector light dominates the mood. */
#define WALL_ALBEDO 180, 180, 180

static const Wall g_walls[] = {
	/* Sector 0 — warm room */
	{ -200, -200,  200, -200, -1, WALL_ALBEDO },
	{  200, -200,  200,  200, -1, WALL_ALBEDO },
	{  200,  200,   80,  200, -1, WALL_ALBEDO },
	{   80,  200,  -80,  200,  1, 0, 0, 0 },
	{  -80,  200, -200,  200, -1, WALL_ALBEDO },
	{ -200,  200, -200, -200, -1, WALL_ALBEDO },

	/* Sector 1 — cool room */
	{  200,  200,  200,  600, -1, WALL_ALBEDO },
	{  200,  600, -200,  600, -1, WALL_ALBEDO },
	{ -200,  600, -200,  200, -1, WALL_ALBEDO },
	{ -200,  200,  -80,  200, -1, WALL_ALBEDO },
	{  -80,  200,   80,  200,  0, 0, 0, 0 },
	{   80,  200,  200,  200, -1, WALL_ALBEDO },
};

/*
 * light = table_rgb * brightness / 255 (pre-resolved for the fixture).
 * color_id reserved for cooker light tables.
 */
static const Sector g_sectors[] = {
	{
		.floor_y = 0,
		.ceil_y = -180,
		.color_id = 1,
		.brightness = 220,
		.light = { 255, 72, 48 }, /* warm red */
		.min_x = -200, .max_x = 200,
		.min_z = -200, .max_z = 199,
		.wall_begin = 0, .wall_count = 6
	},
	{
		.floor_y = 0,
		.ceil_y = -180,
		.color_id = 2,
		.brightness = 220,
		.light = { 48, 160, 255 }, /* cool cyan */
		.min_x = -200, .max_x = 200,
		.min_z = 200, .max_z = 600,
		.wall_begin = 6, .wall_count = 6
	}
};

void world_init_tworoom(World *out)
{
	out->sectors = g_sectors;
	out->walls = g_walls;
	out->sector_count = (u16)(sizeof(g_sectors) / sizeof(g_sectors[0]));
	out->wall_count = (u16)(sizeof(g_walls) / sizeof(g_walls[0]));
}
