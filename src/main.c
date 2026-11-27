/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * M0 bring-up: PAL 320×256 clear + colored gouraud triangle via OT,
 * VSync loop, Doom-like pad poll stub, silent audio stub.
 *
 * Clean-room from-scratch engine — no Doom / PSXDOOM-RE / psyq sources.
 */
#include "engine/audio.h"
#include "engine/pad.h"
#include "engine/platform.h"
#include "engine/render.h"
#include "engine/time.h"
#include "engine/types.h"

#include <stdio.h>

/* Triangle drifts slightly with pad input so the stub is visible in DuckStation. */
static i16 g_ox;
static i16 g_oy;

static void tick_player(const PadState *pad)
{
	g_ox += (i16)(pad->strafe * 2 + pad->turn);
	g_oy += (i16)(-pad->forward * 2);

	if (g_ox < -80) {
		g_ox = -80;
	} else if (g_ox > 80) {
		g_ox = 80;
	}

	if (g_oy < -60) {
		g_oy = -60;
	} else if (g_oy > 60) {
		g_oy = 60;
	}
}

static void draw_hello_tri(void)
{
	const i16 cx = (i16)(SCREEN_W / 2 + g_ox);
	const i16 cy = (i16)(SCREEN_H / 2 + g_oy);

	r_draw_tri(
		(i16)(cx),      (i16)(cy - 56), 255,  64,  64,
		(i16)(cx - 72), (i16)(cy + 48),  64, 255,  64,
		(i16)(cx + 72), (i16)(cy + 48),  64,  64, 255
	);
}

int main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;

	printf("doomps1 M0: PAL %dx%d hello triangle\n", SCREEN_W, SCREEN_H);

	plat_init();
	r_init();
	pad_init();
	audio_init();
	time_init();

	g_ox = 0;
	g_oy = 0;

	for (;;) {
		PadState pad;
		u32 ticks;
		u32 i;

		pad_poll(&pad);
		ticks = time_begin_frame();

		for (i = 0; i < ticks; i++) {
			tick_player(&pad);
		}

		r_begin_frame();
		draw_hello_tri();
		r_end_frame();

		audio_update();
	}

	return 0;
}
