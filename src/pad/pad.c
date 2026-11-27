/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * BIOS pad poll stub with Doom-like tank + strafe mapping.
 */
#include "engine/pad.h"

#include <string.h>
#include <psxapi.h>
#include <psxpad.h>

static u8 g_pad_buf[2][34];

void pad_init(void)
{
	memset(g_pad_buf, 0xff, sizeof(g_pad_buf));

	InitPAD(g_pad_buf[0], 34, g_pad_buf[1], 34);
	StartPAD();
	ChangeClearPAD(0);
}

void pad_poll(PadState *out)
{
	PADTYPE *pad;

	out->forward = 0;
	out->turn = 0;
	out->strafe = 0;
	out->buttons = 0xffff;
	out->connected = false;

	pad = (PADTYPE *)g_pad_buf[0];
	if (pad->stat != 0) {
		return;
	}

	/* Digital, analog stick, DualShock. */
	if (pad->type != 0x4 && pad->type != 0x5 && pad->type != 0x7) {
		return;
	}

	out->connected = true;
	out->buttons = pad->btn;

	/* D-pad / face: tank turn + forward/back (active-low buttons). */
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

	/* Strafe on shoulders (L1/R1 preferred; L2/R2 also accepted). */
	if (!(pad->btn & PAD_L1) || !(pad->btn & PAD_L2)) {
		out->strafe = -1;
	} else if (!(pad->btn & PAD_R1) || !(pad->btn & PAD_R2)) {
		out->strafe = 1;
	}
}
