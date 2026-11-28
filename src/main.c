/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * M3: hitscan + one monster class on cooked map. Audio stub.
 */
#include "engine/audio.h"
#include "engine/entities.h"
#include "engine/mapbin.h"
#include "engine/pad.h"
#include "engine/platform.h"
#include "engine/player.h"
#include "engine/render.h"
#include "engine/time.h"
#include "engine/types.h"

#include <stdio.h>

int main(int argc, const char **argv)
{
	MapRuntime map;
	Player player;
	EntityWorld entities;
	const u8 *blob;
	u32 blob_size;
	(void)argc;
	(void)argv;

	printf("doomps1 M3: combat loop PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	r_init();
	pad_init();
	audio_init();
	time_init();

	blob = mapbin_embedded(&blob_size);
	if (mapbin_load(blob, blob_size, &map) != 0) {
		printf("mapbin load failed\n");
		for (;;) {
			plat_vsync();
		}
	}

	player_init(&player, &map.world);
	map_apply_spawn(&map, &player);
	entities_init(&entities, &map.world, &player);

	for (;;) {
		PadState pad;
		u32 ticks;
		u32 i;

		pad_poll(&pad);
		ticks = time_begin_frame();

		for (i = 0; i < ticks; i++) {
			player_tick(&player, &map.world, &pad);
			if (pad.fire_pressed) {
				weapon_hitscan(&entities, &map.world, &player);
			}
			entities_tick(&entities, &map.world, &player);
		}

		r_begin_frame();
		r_draw_world(&map.world, &player);
		r_draw_monsters(&map.world, &player, &entities);
		r_draw_flash(&player);
		r_draw_hud(&player);
		r_end_frame();

		audio_update();
	}

	return 0;
}
