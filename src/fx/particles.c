/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Silent muzzle / particle FX.
 */
#include "engine/fx.h"

#include <psxgpu.h>
#include <string.h>

/* Draw uses render's OT via a tiny exported hook — keep particles screen-local. */
extern void r_draw_particle_tile(i16 x, i16 y, i16 w, i16 h, u8 r, u8 g, u8 b);

void fx_init(FxWorld *fx)
{
	memset(fx, 0, sizeof(*fx));
}

void fx_muzzle(FxWorld *fx, const Player *player)
{
	u16 i;
	u16 spawned = 0;
	(void)player;

	for (i = 0; i < MAX_PARTICLES && spawned < 8; i++) {
		Particle *p = &fx->particles[i];
		if (p->active) {
			continue;
		}
		p->active = true;
		p->sx = (i16)(SCREEN_W / 2 - 8 + (i16)((spawned * 3) & 15));
		p->sy = (i16)(SCREEN_H - 48 - (i16)(spawned * 2));
		p->vx = (i16)((spawned & 1) ? 2 : -2);
		p->vy = (i16)(-3 - (spawned & 3));
		p->life = (u8)(8 + (spawned & 7));
		p->r = 255;
		p->g = (u8)(180 - spawned * 10);
		p->b = 40;
		spawned++;
	}
}

void fx_tick(FxWorld *fx)
{
	u16 i;
	for (i = 0; i < MAX_PARTICLES; i++) {
		Particle *p = &fx->particles[i];
		if (!p->active) {
			continue;
		}
		p->sx = (i16)(p->sx + p->vx);
		p->sy = (i16)(p->sy + p->vy);
		if (p->life > 0) {
			p->life--;
		}
		if (p->life == 0) {
			p->active = false;
		}
	}
}

void fx_draw(const FxWorld *fx)
{
	u16 i;
	for (i = 0; i < MAX_PARTICLES; i++) {
		const Particle *p = &fx->particles[i];
		if (!p->active) {
			continue;
		}
		r_draw_particle_tile(p->sx, p->sy, 3, 3, p->r, p->g, p->b);
	}
}
