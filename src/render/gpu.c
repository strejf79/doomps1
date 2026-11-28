/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Double-buffered OT + GTE hybrid 2.5D submit (flat floors, linedef quads).
 */
#include "engine/render.h"

#include <assert.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#define OT_LEN       256
#define PRIM_BUF_LEN 24576

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

void r_init(void)
{
	SetVideoMode(MODE_PAL);

	SetDefDispEnv(&g_ctx.buf[0].disp, 0, 0, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&g_ctx.buf[0].draw, 0, 0, SCREEN_W, SCREEN_H);
	SetDefDispEnv(&g_ctx.buf[1].disp, 0, SCREEN_H, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&g_ctx.buf[1].draw, 0, SCREEN_H, SCREEN_W, SCREEN_H);

	setRGB0(&g_ctx.buf[0].draw, 8, 8, 16);
	setRGB0(&g_ctx.buf[1].draw, 8, 8, 16);
	g_ctx.buf[0].draw.isbg = 1;
	g_ctx.buf[1].draw.isbg = 1;
	/* Dither off (locked perf lever). */
	g_ctx.buf[0].draw.dtd = 0;
	g_ctx.buf[1].draw.dtd = 0;

	g_ctx.active = 0;
	g_ctx.next = g_ctx.buf[0].prims;
	ClearOTagR(g_ctx.buf[0].ot, OT_LEN);
	ClearOTagR(g_ctx.buf[1].ot, OT_LEN);

	InitGeom();
	gte_SetGeomOffset(SCREEN_W / 2, SCREEN_H / 2);
	gte_SetGeomScreen(SCREEN_W / 2);

	FntLoad(960, 0);
	FntOpen(8, 8, 304, 64, 0, 128);

	SetDispMask(1);
}

void r_begin_frame(void)
{
}

void r_end_frame(void)
{
	DrawSync(0);
	VSync(0);

	{
		RenderBuffer *drawn = &g_ctx.buf[g_ctx.active];
		RenderBuffer *show = &g_ctx.buf[g_ctx.active ^ 1];

		PutDispEnv(&show->disp);
		DrawOTagEnv(&drawn->ot[OT_LEN - 1], &drawn->draw);
	}

	g_ctx.active ^= 1;
	g_ctx.next = g_ctx.buf[g_ctx.active].prims;
	ClearOTagR(g_ctx.buf[g_ctx.active].ot, OT_LEN);
}

/* Reject if all four screen points are clearly off one side. */
static int quad_offscreen(const POLY_F4 *q)
{
	i16 minx, maxx, miny, maxy;

	minx = q->x0;
	if (q->x1 < minx) minx = q->x1;
	if (q->x2 < minx) minx = q->x2;
	if (q->x3 < minx) minx = q->x3;

	maxx = q->x0;
	if (q->x1 > maxx) maxx = q->x1;
	if (q->x2 > maxx) maxx = q->x2;
	if (q->x3 > maxx) maxx = q->x3;

	miny = q->y0;
	if (q->y1 < miny) miny = q->y1;
	if (q->y2 < miny) miny = q->y2;
	if (q->y3 < miny) miny = q->y3;

	maxy = q->y0;
	if (q->y1 > maxy) maxy = q->y1;
	if (q->y2 > maxy) maxy = q->y2;
	if (q->y3 > maxy) maxy = q->y3;

	if (maxx < 0 || minx >= SCREEN_W || maxy < 0 || miny >= SCREEN_H) {
		return 1;
	}
	return 0;
}

static int submit_flat_quad(
	SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3,
	u8 r, u8 g, u8 b,
	int backface_cull
)
{
	int p;
	POLY_F4 *poly;

	gte_ldv3(v0, v1, v2);
	gte_rtpt();

	if (backface_cull) {
		gte_nclip();
		gte_stopz(&p);
		if (p <= 0) {
			return 0;
		}
	}

	gte_avsz3();
	gte_stotz(&p);
	if ((p >> 2) <= 0 || (p >> 2) >= OT_LEN) {
		return 0;
	}

	poly = (POLY_F4 *)g_ctx.next;
	setPolyF4(poly);
	gte_stsxy0(&poly->x0);
	gte_stsxy1(&poly->x1);
	gte_stsxy2(&poly->x2);

	gte_ldv0(v3);
	gte_rtps();
	gte_stsxy(&poly->x3);

	if (quad_offscreen(poly)) {
		return 0;
	}

	gte_avsz4();
	gte_stotz(&p);
	if ((p >> 2) <= 0 || (p >> 2) >= OT_LEN) {
		return 0;
	}

	setRGB0(poly, r, g, b);
	addPrim(&g_ctx.buf[g_ctx.active].ot[p >> 2], poly);
	g_ctx.next += sizeof(POLY_F4);
	assert(g_ctx.next <= &g_ctx.buf[g_ctx.active].prims[PRIM_BUF_LEN]);
	return 1;
}

static void draw_sector_flats(const Sector *s)
{
	SVECTOR v[4];

	/* Floor: Y = floor_y (one quad). Winding for upward-facing in Y-down space. */
	setVector(&v[0], s->min_x, s->floor_y, s->min_z);
	setVector(&v[1], s->max_x, s->floor_y, s->min_z);
	setVector(&v[2], s->min_x, s->floor_y, s->max_z);
	setVector(&v[3], s->max_x, s->floor_y, s->max_z);
	/* Order so nclip sees floor from above (camera looks down at floor). */
	submit_flat_quad(&v[0], &v[2], &v[1], &v[3], s->floor_r, s->floor_g, s->floor_b, 0);

	/* Ceiling */
	setVector(&v[0], s->min_x, s->ceil_y, s->min_z);
	setVector(&v[1], s->max_x, s->ceil_y, s->min_z);
	setVector(&v[2], s->min_x, s->ceil_y, s->max_z);
	setVector(&v[3], s->max_x, s->ceil_y, s->max_z);
	submit_flat_quad(&v[0], &v[1], &v[2], &v[3], s->ceil_r, s->ceil_g, s->ceil_b, 0);
}

static int draw_wall(const Wall *wall, i16 floor_y, i16 ceil_y)
{
	SVECTOR v[4];

	if (wall->portal_to >= 0) {
		return 0; /* opening — neighbouring sector draws its own solids */
	}

	/* Vertical linedef quad (strip/quad path — flat shaded for M1). */
	setVector(&v[0], wall->x0, floor_y, wall->z0);
	setVector(&v[1], wall->x1, floor_y, wall->z1);
	setVector(&v[2], wall->x0, ceil_y, wall->z0);
	setVector(&v[3], wall->x1, ceil_y, wall->z1);

	/* Try inward facing; if nclip rejects, flip winding. */
	if (submit_flat_quad(&v[0], &v[1], &v[2], &v[3], wall->r, wall->g, wall->b, 1)) {
		return 1;
	}
	return submit_flat_quad(&v[1], &v[0], &v[3], &v[2], wall->r, wall->g, wall->b, 1);
}

static void setup_camera(const Player *player)
{
	SVECTOR trot;
	VECTOR tpos;
	MATRIX mtx;

	trot.vx = 0;
	trot.vy = player->yaw;
	trot.vz = 0;

	RotMatrix(&trot, &mtx);

	tpos.vx = -(player->x >> 12);
	tpos.vy = -(player->y >> 12);
	tpos.vz = -(player->z >> 12);
	ApplyMatrixLV(&mtx, &tpos, &tpos);
	TransMatrix(&mtx, &tpos);

	gte_SetRotMatrix(&mtx);
	gte_SetTransMatrix(&mtx);
}

void r_draw_world(const World *w, const Player *player)
{
	i16 vis[MAX_VIS_SECTORS];
	u16 vis_n;
	u16 i, j;
	u16 walls_drawn = 0;

	setup_camera(player);
	vis_n = world_portal_flood(w, player->sector, vis, MAX_VIS_SECTORS);

	for (i = 0; i < vis_n; i++) {
		const Sector *s = &w->sectors[vis[i]];

		draw_sector_flats(s);

		for (j = 0; j < s->wall_count; j++) {
			const Wall *wall = &w->walls[s->wall_begin + j];
			walls_drawn += (u16)draw_wall(wall, s->floor_y, s->ceil_y);
		}
	}

	FntPrint(-1, "M1 two-room walk\n");
	FntPrint(-1, "sec=%d vis=%d walls~%d\n", player->sector, vis_n, walls_drawn);
	FntPrint(-1, "x=%d z=%d yaw=%d\n", player->x >> 12, player->z >> 12, player->yaw);
	FntFlush(-1);
}
