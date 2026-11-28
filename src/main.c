/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * M2: load cooked FreeDoom-derived .map.bin (E1M1 subset) and walk it.
 * Clean-room — no Doom / PSXDOOM-RE / psyq sources.
 */
#include "engine/audio.h"
#include "engine/mapbin.h"
#include "engine/pad.h"
#include "engine/platform.h"
#include "engine/player.h"
#include "engine/render.h"
#include "engine/time.h"
#include "engine/types.h"
#include "engine/world.h"

#include <stdio.h>

int main(int argc, const char **argv)
{
	MapRuntime map;
	Player player;
	const u8 *blob;
	u32 blob_size;
	(void)argc;
	(void)argv;

	printf("doomps1 M2: cooked map walk PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	r_init();
	pad_init();
	audio_init();
	time_init();

	blob = mapbin_embedded(&blob_size);
	if (mapbin_load(blob, blob_size, &map) != 0) {
		printf("mapbin load failed — abort\n");
		for (;;) {
			plat_vsync();
		}
	}

	printf("map: %u sectors, %u walls, spawn sec=%d\n",
		map.world.sector_count, map.world.wall_count, map.spawn_sector);

	player_init(&player, &map.world);
	map_apply_spawn(&map, &player);

	for (;;) {
		PadState pad;
		u32 ticks;
		u32 i;

		pad_poll(&pad);
		ticks = time_begin_frame();

		for (i = 0; i < ticks; i++) {
			player_tick(&player, &map.world, &pad);
		}

		r_begin_frame();
		r_draw_world(&map.world, &player);
		r_end_frame();

		audio_update();
	}

	return 0;
}
