/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Silent particle / muzzle FX (no SPU).
 */
#ifndef ENGINE_FX_H
#define ENGINE_FX_H

#include "engine/player.h"
#include "engine/types.h"

#define MAX_PARTICLES 64

typedef struct Particle {
	bool active;
	i16 sx, sy;   /* screen-space for muzzle; or world for later */
	i16 vx, vy;
	u8  life;
	u8  r, g, b;
} Particle;

typedef struct FxWorld {
	Particle particles[MAX_PARTICLES];
} FxWorld;

void fx_init(FxWorld *fx);
void fx_muzzle(FxWorld *fx, const Player *player);
void fx_tick(FxWorld *fx);
void fx_draw(const FxWorld *fx);

#endif /* ENGINE_FX_H */
