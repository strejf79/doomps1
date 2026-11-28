/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Hand-authored two-room test map (axis-aligned). Not from Doom/WAD data.
 *
 * Layout (top-down, +Z forward from spawn facing +Z):
 *
 *   z=-200  +------------------+
 *           |     ROOM 0       |
 *           |   (start)        |
 *   z= 200  +-----[portal]-----+
 *           |     ROOM 1       |
 *           |                  |
 *   z= 600  +------------------+
 *         x=-200            x=200
 */
#include "engine/world.h"

/* Wall order per sector: used for drawing + collision. Portal walls listed too. */
static const Wall g_walls[] = {
	/* Sector 0 — start room */
	{ -200, -200,  200, -200, -1,  90,  90, 140 }, /* south */
	{  200, -200,  200,  200, -1, 100, 100, 150 }, /* east  */
	{  200,  200,   80,  200, -1,  90,  90, 140 }, /* north solid L */
	{   80,  200,  -80,  200,  1,   0,   0,   0 }, /* north portal → 1 */
	{  -80,  200, -200,  200, -1,  90,  90, 140 }, /* north solid R */
	{ -200,  200, -200, -200, -1,  80,  80, 130 }, /* west  */

	/* Sector 1 — second room */
	{  200,  200,  200,  600, -1,  80, 140,  90 }, /* east  */
	{  200,  600, -200,  600, -1,  70, 130,  80 }, /* north */
	{ -200,  600, -200,  200, -1,  80, 140,  90 }, /* west  */
	{ -200,  200,  -80,  200, -1,  70, 130,  80 }, /* south solid L */
	{  -80,  200,   80,  200,  0,   0,   0,   0 }, /* south portal → 0 */
	{   80,  200,  200,  200, -1,  70, 130,  80 }, /* south solid R */
};

static const Sector g_sectors[] = {
	{
		.floor_y = 0,
		.ceil_y = -180,
		.floor_r = 70, .floor_g = 70, .floor_b = 85,
		.ceil_r = 30, .ceil_g = 30, .ceil_b = 45,
		.min_x = -200, .max_x = 200,
		/* Exclusive of portal plane so z>=200 belongs to room 1. */
		.min_z = -200, .max_z = 199,
		.wall_begin = 0, .wall_count = 6
	},
	{
		.floor_y = 0,
		.ceil_y = -180,
		.floor_r = 90, .floor_g = 75, .floor_b = 55,
		.ceil_r = 35, .ceil_g = 40, .ceil_b = 30,
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
