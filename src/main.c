/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * Post-M6: multi FreeDoom subsets on disc; Select / EXIT advances map.
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

#define MAP_SLOT_COUNT 3

static const char *const g_map_paths[MAP_SLOT_COUNT] = {
	"\\DATA\\MAP.BIN;1",
	"\\DATA\\MAP2.BIN;1",
	"\\DATA\\MAP3.BIN;1",
};

static const char *const g_map_names[MAP_SLOT_COUNT] = {
	"E1M1",
	"E1M2",
	"E1M3",
};

static u8 g_disc_map[MAP_BUDGET_BYTES];

static int load_map_slot(u16 slot, MapRuntime *map)
{
	u32 disc_sz = 0;
	const u8 *blob;
	u32 blob_size;

	if (slot >= MAP_SLOT_COUNT) {
		slot = 0;
	}

	if (disc_seek_read(g_map_paths[slot], g_disc_map, sizeof(g_disc_map), &disc_sz) == DISC_OK
		&& disc_sz > 0 && mapbin_load(g_disc_map, disc_sz, map) == 0) {
		printf("map: disc %s (%u bytes)\n", g_map_names[slot], disc_sz);
		return 0;
	}

	/* Slot 0 falls back to embedded; other slots retry slot 0 embedded. */
	blob = mapbin_embedded(&blob_size);
	if (mapbin_load(blob, blob_size, map) != 0) {
		return -1;
	}
	printf("map: embedded fallback for %s (%u bytes)\n", g_map_names[0], blob_size);
	return 0;
}

static void start_map(
	u16 slot,
	MapRuntime *map,
	Player *player,
	EntityWorld *entities,
	ItemWorld *items,
	MoverWorld *movers,
	FxWorld *fx)
{
	if (load_map_slot(slot, map) != 0) {
		for (;;) {
			plat_vsync();
		}
	}
	player_init(player, &map->world);
	map_apply_spawn(map, player);
	entities_init_from_map(entities, map);
	items_init_from_map(items, map);
	movers_init(movers, &map->world);
	fx_init(fx);
	disc_prefetch(g_map_paths[(slot + 1) % MAP_SLOT_COUNT], 1);
	disc_prefetch("\\DATA\\SFX.BIN;1", 2);
}

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
	u16 map_slot = 0;
	u16 exit_hold = 0;
	(void)argc;
	(void)argv;

	printf("doomps1 textured walls PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	disc_init();
	r_init();
	pad_init();
	audio_init();
	time_init();

	start_map(map_slot, &map, &player, &entities, &items, &movers, &fx);

	for (;;) {
		PadState pad;
		u32 ticks, i;
		bool advance = false;

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

		if (pad.select_pressed) {
			advance = true;
		}
		if (map.exited) {
			exit_hold++;
			if (exit_hold > 90) {
				advance = true;
			}
		} else {
			exit_hold = 0;
		}
		if (advance) {
			map_slot = (u16)((map_slot + 1) % MAP_SLOT_COUNT);
			exit_hold = 0;
			printf("advance → %s\n", g_map_names[map_slot]);
			start_map(map_slot, &map, &player, &entities, &items, &movers, &fx);
		}

		r_begin_frame();
		r_draw_world(&map.world, &player);
		r_draw_monsters(&map.world, &player, &entities);
		r_draw_items(&map.world, &items);
		r_draw_flash(&player);
		fx_draw(&fx);
		r_draw_hud(&player, map.exited, g_map_names[map_slot]);
		r_end_frame();

		disc_update();
		audio_update();
	}
	return 0;
}
