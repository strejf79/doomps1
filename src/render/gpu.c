/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Double-buffered OT + GTE hybrid 2.5D with colored static Gouraud lighting.
 *
 * Walls/floors use POLY_G4 (untextured gouraud). POLY_GT* lands with atlases.
 */
#include "engine/render.h"

#include <assert.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

/* entities.h pulled via render.h */

#define OT_LEN       256
#define PRIM_BUF_LEN 32768

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

	setRGB0(&g_ctx.buf[0].draw, 4, 4, 8);
	setRGB0(&g_ctx.buf[1].draw, 4, 4, 8);
	g_ctx.buf[0].draw.isbg = 1;
	g_ctx.buf[1].draw.isbg = 1;
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
	FntOpen(8, 8, 304, 72, 0, 160);

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

static int gquad_offscreen(const POLY_G4 *q)
{
	i16 minx = q->x0, maxx = q->x0, miny = q->y0, maxy = q->y0;

	if (q->x1 < minx) minx = q->x1;
	if (q->x2 < minx) minx = q->x2;
	if (q->x3 < minx) minx = q->x3;
	if (q->x1 > maxx) maxx = q->x1;
	if (q->x2 > maxx) maxx = q->x2;
	if (q->x3 > maxx) maxx = q->x3;
	if (q->y1 < miny) miny = q->y1;
	if (q->y2 < miny) miny = q->y2;
	if (q->y3 < miny) miny = q->y3;
	if (q->y1 > maxy) maxy = q->y1;
	if (q->y2 > maxy) maxy = q->y2;
	if (q->y3 > maxy) maxy = q->y3;

	return (maxx < 0 || minx >= SCREEN_W || maxy < 0 || miny >= SCREEN_H);
}

static int submit_gquad(
	SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3,
	Rgb8 c0, Rgb8 c1, Rgb8 c2, Rgb8 c3,
	int backface_cull
)
{
	int p;
	POLY_G4 *poly;

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

	poly = (POLY_G4 *)g_ctx.next;
	setPolyG4(poly);
	gte_stsxy0(&poly->x0);
	gte_stsxy1(&poly->x1);
	gte_stsxy2(&poly->x2);

	gte_ldv0(v3);
	gte_rtps();
	gte_stsxy(&poly->x3);

	if (gquad_offscreen(poly)) {
		return 0;
	}

	gte_avsz4();
	gte_stotz(&p);
	if ((p >> 2) <= 0 || (p >> 2) >= OT_LEN) {
		return 0;
	}

	setRGB0(poly, c0.r, c0.g, c0.b);
	setRGB1(poly, c1.r, c1.g, c1.b);
	setRGB2(poly, c2.r, c2.g, c2.b);
	setRGB3(poly, c3.r, c3.g, c3.b);

	addPrim(&g_ctx.buf[g_ctx.active].ot[p >> 2], poly);
	g_ctx.next += sizeof(POLY_G4);
	assert(g_ctx.next <= &g_ctx.buf[g_ctx.active].prims[PRIM_BUF_LEN]);
	return 1;
}

static void draw_sector_flats(const World *w, i16 sec_i)
{
	const Sector *s = &w->sectors[sec_i];
	SVECTOR v[4];
	Rgb8 c[4];
	Rgb8 albedo_floor = { 200, 200, 200 };
	Rgb8 albedo_ceil = { 120, 120, 120 };

	/* Corner lights: blend on portal-adjacent corners. */
	c[0] = world_shade(albedo_floor, world_vert_light(w, sec_i, s->min_x, s->min_z));
	c[1] = world_shade(albedo_floor, world_vert_light(w, sec_i, s->max_x, s->min_z));
	c[2] = world_shade(albedo_floor, world_vert_light(w, sec_i, s->min_x, s->max_z));
	c[3] = world_shade(albedo_floor, world_vert_light(w, sec_i, s->max_x, s->max_z));

	setVector(&v[0], s->min_x, s->floor_y, s->min_z);
	setVector(&v[1], s->max_x, s->floor_y, s->min_z);
	setVector(&v[2], s->min_x, s->floor_y, s->max_z);
	setVector(&v[3], s->max_x, s->floor_y, s->max_z);
	submit_gquad(&v[0], &v[2], &v[1], &v[3], c[0], c[2], c[1], c[3], 0);

	c[0] = world_shade(albedo_ceil, world_vert_light(w, sec_i, s->min_x, s->min_z));
	c[1] = world_shade(albedo_ceil, world_vert_light(w, sec_i, s->max_x, s->min_z));
	c[2] = world_shade(albedo_ceil, world_vert_light(w, sec_i, s->min_x, s->max_z));
	c[3] = world_shade(albedo_ceil, world_vert_light(w, sec_i, s->max_x, s->max_z));

	setVector(&v[0], s->min_x, s->ceil_y, s->min_z);
	setVector(&v[1], s->max_x, s->ceil_y, s->min_z);
	setVector(&v[2], s->min_x, s->ceil_y, s->max_z);
	setVector(&v[3], s->max_x, s->ceil_y, s->max_z);
	submit_gquad(&v[0], &v[1], &v[2], &v[3], c[0], c[1], c[2], c[3], 0);
}

static int draw_wall(const World *w, i16 sec_i, const Wall *wall)
{
	const Sector *s = &w->sectors[sec_i];
	SVECTOR v[4];
	Rgb8 albedo = { wall->ar, wall->ag, wall->ab };
	Rgb8 c0, c1, c2, c3;

	if (wall->portal_to >= 0) {
		return 0;
	}

	c0 = world_shade(albedo, world_vert_light(w, sec_i, wall->x0, wall->z0));
	c1 = world_shade(albedo, world_vert_light(w, sec_i, wall->x1, wall->z1));
	c2 = c0;
	c3 = c1;

	setVector(&v[0], wall->x0, s->floor_y, wall->z0);
	setVector(&v[1], wall->x1, s->floor_y, wall->z1);
	setVector(&v[2], wall->x0, s->ceil_y, wall->z0);
	setVector(&v[3], wall->x1, s->ceil_y, wall->z1);

	if (submit_gquad(&v[0], &v[1], &v[2], &v[3], c0, c1, c2, c3, 1)) {
		return 1;
	}
	return submit_gquad(&v[1], &v[0], &v[3], &v[2], c1, c0, c3, c2, 1);
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
		i16 sec = vis[i];
		const Sector *s = &w->sectors[sec];

		draw_sector_flats(w, sec);

		/* Far LOD: skip detailed walls (flat floor/ceil only). */
		if (s->lod != 0) {
			continue;
		}

		for (j = 0; j < s->wall_count; j++) {
			const Wall *wall = &w->walls[s->wall_begin + j];
			walls_drawn += (u16)draw_wall(w, sec, wall);
		}
	}

	(void)walls_drawn;
}

void r_draw_monsters(const World *w, const Player *player, const EntityWorld *ew)
{
	u16 i;
	(void)player;

	for (i = 0; i < MAX_MONSTERS; i++) {
		const Monster *m = &ew->monsters[i];
		SVECTOR v[4];
		Rgb8 c0, c1, c2, c3;
		i16 floor_y;
		i32 mx, mz;
		i16 half = 18;
		i16 tall = 56;

		if (!m->active) {
			continue;
		}

		mx = m->x >> 12;
		mz = m->z >> 12;
		floor_y = 0;
		if (m->sector >= 0 && (u16)m->sector < w->sector_count) {
			floor_y = w->sectors[m->sector].floor_y;
			{
				Rgb8 light = w->sectors[m->sector].light;
				Rgb8 albedo = { 220, 60, 60 };
				if (m->state == MON_DEAD) {
					albedo.r = 80; albedo.g = 40; albedo.b = 40;
					tall = 16;
				} else if (m->state == MON_PAIN) {
					albedo.r = 255; albedo.g = 220; albedo.b = 220;
				} else if (m->state == MON_ATTACK) {
					albedo.r = 255; albedo.g = 120; albedo.b = 40;
				}
				c0 = world_shade(albedo, light);
				c1 = c0;
				c2 = c0;
				c3 = c0;
			}
			} else {
			c0.r = 200; c0.g = 40; c0.b = 40;
			c1 = c0;
			c2 = c0;
			c3 = c0;
		}

		/* Axis-aligned billboard quad in X (cheap sprite stand-in). */
		setVector(&v[0], (i16)(mx - half), floor_y, (i16)mz);
		setVector(&v[1], (i16)(mx + half), floor_y, (i16)mz);
		setVector(&v[2], (i16)(mx - half), (i16)(floor_y - tall), (i16)mz);
		setVector(&v[3], (i16)(mx + half), (i16)(floor_y - tall), (i16)mz);

		if (!submit_gquad(&v[0], &v[1], &v[2], &v[3], c0, c1, c2, c3, 1)) {
			submit_gquad(&v[1], &v[0], &v[3], &v[2], c1, c0, c3, c2, 1);
		}
	}
}

void r_draw_flash(const Player *player)
{
	TILE *tile;
	u8 a;

	if (player->flash == 0) {
		return;
	}

	/* Top strip muzzle / pain flash (opaque; avoids semi-trans setup). */
	a = (u8)(40 + player->flash * 20);
	tile = (TILE *)g_ctx.next;
	setTile(tile);
	setXY0(tile, 0, 0);
	setWH(tile, SCREEN_W, 24);
	setRGB0(tile, a, a / 4, a / 5);
	addPrim(&g_ctx.buf[g_ctx.active].ot[0], tile);
	g_ctx.next += sizeof(TILE);
}

void r_draw_particle_tile(i16 x, i16 y, i16 w, i16 h, u8 r, u8 g, u8 b)
{
	TILE *tile = (TILE *)g_ctx.next;
	setTile(tile);
	setXY0(tile, x, y);
	setWH(tile, w, h);
	setRGB0(tile, r, g, b);
	addPrim(&g_ctx.buf[g_ctx.active].ot[0], tile);
	g_ctx.next += sizeof(TILE);
	assert(g_ctx.next <= &g_ctx.buf[g_ctx.active].prims[PRIM_BUF_LEN]);
}

void r_draw_items(const World *w, const ItemWorld *items)
{
	u16 i;
	(void)w;
	for (i = 0; i < items->count; i++) {
		const Item *it = &items->items[i];
		SVECTOR v[4];
		Rgb8 c = { 80, 220, 80 };
		i16 x, z, y0;
		if (!it->active) continue;
		if (it->kind == ITEM_AMMO) { c.r = 220; c.g = 200; c.b = 40; }
		else if (it->kind == ITEM_SHOTGUN) { c.r = 40; c.g = 120; c.b = 255; }
		else if (it->kind == ITEM_CHAINGUN) { c.r = 255; c.g = 120; c.b = 40; }
		x = it->x;
		z = it->z;
		y0 = 0;
		if (it->sector >= 0 && (u16)it->sector < w->sector_count) {
			y0 = w->sectors[it->sector].floor_y;
			c = world_shade(c, w->sectors[it->sector].light);
		}
		setVector(&v[0], (i16)(x - 8), y0, z);
		setVector(&v[1], (i16)(x + 8), y0, z);
		setVector(&v[2], (i16)(x - 8), (i16)(y0 - 16), z);
		setVector(&v[3], (i16)(x + 8), (i16)(y0 - 16), z);
		if (!submit_gquad(&v[0], &v[1], &v[2], &v[3], c, c, c, c, 1)) {
			submit_gquad(&v[1], &v[0], &v[3], &v[2], c, c, c, c, 1);
		}
	}
}

void r_draw_hud(const Player *player, bool exited, const char *map_name)
{
	const char *wpn = "PISTOL";
	if (player->weapon == WPN_SHOTGUN) wpn = "SHOTGUN";
	else if (player->weapon == WPN_CHAINGUN) wpn = "CHAINGUN";
	FntPrint(-1, "%s HP:%d AM:%d %s\n",
		map_name ? map_name : "MAP", player->health, player->ammo, wpn);
	FntPrint(-1, "X fire O wpn /\\ use Sel next\n");
	if (player->health <= 0) FntPrint(-1, "YOU ARE DEAD\n");
	else if (exited) FntPrint(-1, "EXIT! next...\n");
	FntFlush(-1);
}
