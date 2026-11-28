/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Hand-authored two-room fixture (regression / JSON cook source sibling).
 */
#include "engine/world.h"

#define WALL_ALBEDO 180, 180, 180, 0

static const Wall g_walls[] = {
	{ -200, -200,  200, -200, -1, WALL_ALBEDO },
	{  200, -200,  200,  200, -1, WALL_ALBEDO },
	{  200,  200,   80,  200, -1, WALL_ALBEDO },
	{   80,  200,  -80,  200,  1, 0, 0, 0, 0 },
	{  -80,  200, -200,  200, -1, WALL_ALBEDO },
	{ -200,  200, -200, -200, -1, WALL_ALBEDO },
	{  200,  200,  200,  600, -1, WALL_ALBEDO },
	{  200,  600, -200,  600, -1, WALL_ALBEDO },
	{ -200,  600, -200,  200, -1, WALL_ALBEDO },
	{ -200,  200,  -80,  200, -1, WALL_ALBEDO },
	{  -80,  200,   80,  200,  0, 0, 0, 0, 0 },
	{   80,  200,  200,  200, -1, WALL_ALBEDO },
};

static const Sector g_sectors[] = {
	{
		.floor_y = 0, .ceil_y = -180,
		.color_id = 1, .brightness = 220,
		.light = { 255, 72, 48 }, .lod = 0,
		.min_x = -200, .max_x = 200, .min_z = -200, .max_z = 199,
		.wall_begin = 0, .wall_count = 6
	},
	{
		.floor_y = 0, .ceil_y = -180,
		.color_id = 2, .brightness = 220,
		.light = { 48, 160, 255 }, .lod = 0,
		.min_x = -200, .max_x = 200, .min_z = 200, .max_z = 600,
		.wall_begin = 6, .wall_count = 6
	}
};

void world_init_tworoom(World *out)
{
	out->sectors = g_sectors;
	out->walls = g_walls;
	out->sector_count = 2;
	out->wall_count = 12;
}
