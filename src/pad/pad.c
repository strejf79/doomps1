/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 */
#include "engine/pad.h"

#include <string.h>
#include <psxapi.h>
#include <psxpad.h>

static u8 g_pad_buf[2][34];
static bool g_fire_prev;
static bool g_use_prev;
static bool g_wpn_prev;

void pad_init(void)
{
	memset(g_pad_buf, 0xff, sizeof(g_pad_buf));
	g_fire_prev = g_use_prev = g_wpn_prev = false;
	InitPAD(g_pad_buf[0], 34, g_pad_buf[1], 34);
	StartPAD();
	ChangeClearPAD(0);
}

void pad_poll(PadState *out)
{
	PADTYPE *pad;
	bool fire_now, use_now, wpn_now;

	memset(out, 0, sizeof(*out));
	out->buttons = 0xffff;

	pad = (PADTYPE *)g_pad_buf[0];
	if (pad->stat != 0 || (pad->type != 0x4 && pad->type != 0x5 && pad->type != 0x7)) {
		g_fire_prev = g_use_prev = g_wpn_prev = false;
		return;
	}

	out->connected = true;
	out->buttons = pad->btn;

	if (!(pad->btn & PAD_UP)) out->forward = 1;
	else if (!(pad->btn & PAD_DOWN)) out->forward = -1;
	if (!(pad->btn & PAD_LEFT)) out->turn = -1;
	else if (!(pad->btn & PAD_RIGHT)) out->turn = 1;
	if (!(pad->btn & PAD_L1) || !(pad->btn & PAD_L2)) out->strafe = -1;
	else if (!(pad->btn & PAD_R2)) out->strafe = 1;

	fire_now = (!(pad->btn & PAD_CROSS) || !(pad->btn & PAD_SQUARE));
	use_now = !(pad->btn & PAD_TRIANGLE);
	wpn_now = !(pad->btn & PAD_CIRCLE);

	out->fire = fire_now;
	out->fire_pressed = fire_now && !g_fire_prev;
	out->use_pressed = use_now && !g_use_prev;
	out->weapon_next = wpn_now && !g_wpn_prev;
	g_fire_prev = fire_now;
	g_use_prev = use_now;
	g_wpn_prev = wpn_now;
}
