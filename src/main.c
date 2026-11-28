/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * Plan M3 budgeted level: cooked FreeDoom subset, map spawns, exit,
 * silent muzzle FX, single-load under RAM budget.
 */
#include "engine/audio.h"
#include "engine/entities.h"
#include "engine/fx.h"
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
	FxWorld fx;
	const u8 *blob;
	u32 blob_size;
	(void)argc;
	(void)argv;

	printf("doomps1 plan-M3: budgeted level PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	r_init();
	pad_init();
	audio_init();
	time_init();
	fx_init(&fx);

	blob = mapbin_embedded(&blob_size);
	if (blob_size > MAP_BUDGET_BYTES) {
		printf("embedded map over byte budget\n");
	}
	if (mapbin_load(blob, blob_size, &map) != 0) {
		printf("mapbin load failed\n");
		for (;;) {
			plat_vsync();
		}
	}

	player_init(&player, &map.world);
	map_apply_spawn(&map, &player);
	entities_init_from_map(&entities, &map);

	for (;;) {
		PadState pad;
		u32 ticks;
		u32 i;

		pad_poll(&pad);
		ticks = time_begin_frame();

		for (i = 0; i < ticks; i++) {
			if (!map.exited && player.health > 0) {
				player_tick(&player, &map.world, &pad);
				if (pad.fire_pressed) {
					weapon_hitscan(&entities, &map.world, &player);
					fx_muzzle(&fx, &player);
				}
				entities_tick(&entities, &map.world, &player, true);
				map_check_exit(&map, &player);
			}
			fx_tick(&fx);
		}

		r_begin_frame();
		r_draw_world(&map.world, &player);
		r_draw_monsters(&map.world, &player, &entities);
		r_draw_flash(&player);
		fx_draw(&fx);
		r_draw_hud(&player, map.exited);
		r_end_frame();

		audio_update();
	}

	return 0;
}
