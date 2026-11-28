/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * Plan M4 vertical slice: doors/elevators, pickups, 3 weapons, staggered AI.
 * Audio remains stub.
 */
#include "engine/audio.h"
#include "engine/doors.h"
#include "engine/entities.h"
#include "engine/fx.h"
#include "engine/items.h"
#include "engine/mapbin.h"
#include "engine/pad.h"
#include "engine/platform.h"
#include "engine/player.h"
#include "engine/render.h"
#include "engine/time.h"
#include "engine/types.h"

#include <stdio.h>

static void fire_weapon(EntityWorld *ew, const World *w, Player *p, FxWorld *fx)
{
	i16 hit = 0;
	switch (p->weapon) {
	case WPN_SHOTGUN:
		hit = weapon_shotgun(ew, w, p);
		break;
	case WPN_CHAINGUN:
		hit = weapon_chaingun(ew, w, p);
		break;
	default:
		hit = weapon_hitscan(ew, w, p);
		break;
	}
	if (hit >= 0) {
		fx_muzzle(fx, p);
	}
}

int main(int argc, const char **argv)
{
	MapRuntime map;
	Player player;
	EntityWorld entities;
	ItemWorld items;
	MoverWorld movers;
	FxWorld fx;
	const u8 *blob;
	u32 blob_size;
	(void)argc;
	(void)argv;

	printf("doomps1 plan-M4: vertical slice PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	r_init();
	pad_init();
	audio_init();
	time_init();
	fx_init(&fx);

	blob = mapbin_embedded(&blob_size);
	if (mapbin_load(blob, blob_size, &map) != 0) {
		for (;;) plat_vsync();
	}

	player_init(&player, &map.world);
	map_apply_spawn(&map, &player);
	entities_init_from_map(&entities, &map);
	items_init_from_map(&items, &map);
	movers_init(&movers, &map.world);

	for (;;) {
		PadState pad;
		u32 ticks, i;

		pad_poll(&pad);
		ticks = time_begin_frame();

		for (i = 0; i < ticks; i++) {
			if (!map.exited && player.health > 0) {
				player_tick(&player, &map.world, &pad);
				if (pad.use_pressed) {
					movers_try_use(&movers, &map.world, &player);
				}
				if (pad.fire_pressed || (pad.fire && player.weapon == WPN_CHAINGUN)) {
					fire_weapon(&entities, &map.world, &player, &fx);
				}
				items_tick(&items, &player);
				entities_tick(&entities, &map.world, &player, true);
				movers_tick(&movers, &map.world);
				map_check_exit(&map, &player);
			}
			fx_tick(&fx);
		}

		r_begin_frame();
		r_draw_world(&map.world, &player);
		r_draw_monsters(&map.world, &player, &entities);
		r_draw_items(&map.world, &items);
		r_draw_flash(&player);
		fx_draw(&fx);
		r_draw_hud(&player, map.exited);
		r_end_frame();

		audio_update();
	}
	return 0;
}
