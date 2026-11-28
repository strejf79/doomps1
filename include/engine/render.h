/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Renderer — PAL 320×256, double OT, GTE transforms, hybrid 2.5D submit.
 */
#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include "engine/player.h"
#include "engine/types.h"
#include "engine/world.h"

void r_init(void);
void r_begin_frame(void);
void r_end_frame(void);

/* Draw visible sectors: flat floor/ceiling quads + linedef wall quads. */
void r_draw_world(const World *w, const Player *player);

#endif /* ENGINE_RENDER_H */
