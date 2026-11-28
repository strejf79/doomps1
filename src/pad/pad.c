/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * BIOS pad poll — tank + strafe + fire (Cross / R1).
 */
#include "engine/pad.h"

#include <string.h>
#include <psxapi.h>
#include <psxpad.h>

static u8 g_pad_buf[2][34];
static bool g_fire_prev;

void pad_init(void)
{
	memset(g_pad_buf, 0xff, sizeof(g_pad_buf));
	g_fire_prev = false;

	InitPAD(g_pad_buf[0], 34, g_pad_buf[1], 34);
	StartPAD();
	ChangeClearPAD(0);
}

void pad_poll(PadState *out)
{
	PADTYPE *pad;
	bool fire_now;

	out->forward = 0;
	out->turn = 0;
	out->strafe = 0;
	out->buttons = 0xffff;
	out->connected = false;
	out->fire = false;
	out->fire_pressed = false;

	pad = (PADTYPE *)g_pad_buf[0];
	if (pad->stat != 0) {
		g_fire_prev = false;
		return;
	}

	if (pad->type != 0x4 && pad->type != 0x5 && pad->type != 0x7) {
		g_fire_prev = false;
		return;
	}

	out->connected = true;
	out->buttons = pad->btn;

	if (!(pad->btn & PAD_UP)) {
		out->forward = 1;
	} else if (!(pad->btn & PAD_DOWN)) {
		out->forward = -1;
	}

	if (!(pad->btn & PAD_LEFT)) {
		out->turn = -1;
	} else if (!(pad->btn & PAD_RIGHT)) {
		out->turn = 1;
	}

	if (!(pad->btn & PAD_L1) || !(pad->btn & PAD_L2)) {
		out->strafe = -1;
	} else if (!(pad->btn & PAD_R2)) {
		out->strafe = 1;
	}

	/* Fire: Cross or Square (R1 left for strafe). */
	fire_now = (!(pad->btn & PAD_CROSS) || !(pad->btn & PAD_SQUARE));
	out->fire = fire_now;
	out->fire_pressed = fire_now && !g_fire_prev;
	g_fire_prev = fire_now;
}
