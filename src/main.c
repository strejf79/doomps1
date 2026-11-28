/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * Plan M6: SPU ADPCM SFX + music stub on M5 disc scaffolding.
 */
#include "engine/audio.h"
#include "engine/disc.h"
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
	SfxId sfx = SFX_PISTOL;
	switch (p->weapon) {
	case WPN_SHOTGUN:
		hit = weapon_shotgun(ew, w, p);
		sfx = SFX_SHOTGUN;
		break;
	case WPN_CHAINGUN:
		hit = weapon_chaingun(ew, w, p);
		sfx = SFX_CHAINGUN;
		break;
	default:
		hit = weapon_hitscan(ew, w, p);
		sfx = SFX_PISTOL;
		break;
	}
	if (hit >= 0) {
		fx_muzzle(fx, p);
		audio_play_sfx(sfx);
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

	printf("doomps1 plan-M6: SPU audio PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	disc_init();
	r_init();
	pad_init();
	audio_init();
	time_init();
	fx_init(&fx);

	/* Fail-soft: prefer disc MAP.BIN when present; else embedded. */
	{
		static u8 disc_map[12288];
		u32 disc_sz = 0;
		if (disc_seek_read("\\DATA\\MAP.BIN;1", disc_map, sizeof(disc_map), &disc_sz) == DISC_OK
			&& disc_sz > 0 && mapbin_load(disc_map, disc_sz, &map) == 0) {
			printf("map: loaded from disc (%u bytes)\n", disc_sz);
		} else {
			blob = mapbin_embedded(&blob_size);
			if (mapbin_load(blob, blob_size, &map) != 0) {
				for (;;) plat_vsync();
			}
			printf("map: embedded fallback (%u bytes)\n", blob_size);
		}
	}

	/* Queue adjacent / bank paths — warmed in disc_update, never blocks tick. */
	disc_prefetch("\\DATA\\SFX.BIN;1", 1);
	disc_prefetch("\\DATA\\PREFETC.TOC;1", 2);

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

		disc_update();
		audio_update();
	}
	return 0;
}
