/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Thin GPU / OT wrapper for PAL 320×256 double-buffered frames.
 */
#include "engine/render.h"

#include <assert.h>
#include <psxgpu.h>

#define OT_LEN       16
#define PRIM_BUF_LEN 8192

typedef struct RenderBuffer {
	DISPENV disp;
	DRAWENV draw;
	u32 ot[OT_LEN];
	u8 prims[PRIM_BUF_LEN];
} RenderBuffer;

typedef struct RenderCtx {
	RenderBuffer buf[2];
	u8 *next;
	int active;
} RenderCtx;

static RenderCtx g_ctx;

static void *alloc_prim(int z, size_t size)
{
	RenderBuffer *b = &g_ctx.buf[g_ctx.active];
	u8 *p = g_ctx.next;

	addPrim(&b->ot[z], p);
	g_ctx.next += size;
	assert(g_ctx.next <= &b->prims[PRIM_BUF_LEN]);
	return p;
}

void r_init(void)
{
	/* Force PAL and set up 320×256 framebuffers stacked in VRAM. */
	SetVideoMode(MODE_PAL);

	SetDefDispEnv(&g_ctx.buf[0].disp, 0, 0, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&g_ctx.buf[0].draw, 0, 0, SCREEN_W, SCREEN_H);
	SetDefDispEnv(&g_ctx.buf[1].disp, 0, SCREEN_H, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&g_ctx.buf[1].draw, 0, SCREEN_H, SCREEN_W, SCREEN_H);

	/* Dark clear so the hello triangle reads clearly. */
	setRGB0(&g_ctx.buf[0].draw, 16, 16, 32);
	setRGB0(&g_ctx.buf[1].draw, 16, 16, 32);
	g_ctx.buf[0].draw.isbg = 1;
	g_ctx.buf[1].draw.isbg = 1;
	g_ctx.buf[0].draw.dtd = 1;
	g_ctx.buf[1].draw.dtd = 1;

	g_ctx.active = 0;
	g_ctx.next = g_ctx.buf[0].prims;
	ClearOTagR(g_ctx.buf[0].ot, OT_LEN);

	SetDispMask(1);
}

void r_begin_frame(void)
{
	/* OT already cleared after previous flip; nothing else for M0. */
}

void r_end_frame(void)
{
	DrawSync(0);
	VSync(0);

	RenderBuffer *drawn = &g_ctx.buf[g_ctx.active];
	RenderBuffer *show = &g_ctx.buf[g_ctx.active ^ 1];

	PutDispEnv(&show->disp);
	DrawOTagEnv(&drawn->ot[OT_LEN - 1], &drawn->draw);

	g_ctx.active ^= 1;
	g_ctx.next = g_ctx.buf[g_ctx.active].prims;
	ClearOTagR(g_ctx.buf[g_ctx.active].ot, OT_LEN);
}

void r_draw_tri(
	i16 x0, i16 y0, u8 r0, u8 g0, u8 b0,
	i16 x1, i16 y1, u8 r1, u8 g1, u8 b1,
	i16 x2, i16 y2, u8 r2, u8 g2, u8 b2
)
{
	POLY_G3 *poly = (POLY_G3 *)alloc_prim(1, sizeof(POLY_G3));

	setPolyG3(poly);
	setXY3(poly, x0, y0, x1, y1, x2, y2);
	setRGB0(poly, r0, g0, b0);
	setRGB1(poly, r1, g1, b1);
	setRGB2(poly, r2, g2, b2);
}
