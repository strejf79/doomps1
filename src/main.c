/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * M1: two connected rooms, Doom-like tank+strafe walk at PAL 320×256.
 * Clean-room — no Doom / PSXDOOM-RE / psyq sources.
 */
#include "engine/audio.h"
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
	World world;
	Player player;
	(void)argc;
	(void)argv;

	printf("doomps1 M1: two-room walk PAL %dx%d\n", SCREEN_W, SCREEN_H);

	plat_init();
	r_init();
	pad_init();
	audio_init();
	time_init();

	world_init_tworoom(&world);
	player_init(&player, &world);

	for (;;) {
		PadState pad;
		u32 ticks;
		u32 i;

		pad_poll(&pad);
		ticks = time_begin_frame();

		for (i = 0; i < ticks; i++) {
			player_tick(&player, &world, &pad);
		}

		r_begin_frame();
		r_draw_world(&world, &player);
		r_end_frame();

		audio_update();
	}

	return 0;
}
