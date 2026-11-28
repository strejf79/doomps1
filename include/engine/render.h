/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Renderer — world + sprites + HUD + particles.
 */
#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include "engine/entities.h"
#include "engine/items.h"
#include "engine/player.h"
#include "engine/types.h"
#include "engine/world.h"

#include <stdbool.h>

void r_init(void);
void r_begin_frame(void);
void r_end_frame(void);

void r_draw_world(const World *w, const Player *player);
void r_draw_monsters(const World *w, const Player *player, const EntityWorld *ew);
void r_draw_items(const World *w, const ItemWorld *items);
void r_draw_hud(const Player *player, bool exited);
void r_draw_flash(const Player *player);
void r_draw_particle_tile(i16 x, i16 y, i16 w, i16 h, u8 r, u8 g, u8 b);

#endif /* ENGINE_RENDER_H */
