/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Renderer — world + sprites + HUD.
 */
#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include "engine/entities.h"
#include "engine/player.h"
#include "engine/types.h"
#include "engine/world.h"

void r_init(void);
void r_begin_frame(void);
void r_end_frame(void);

void r_draw_world(const World *w, const Player *player);
void r_draw_monsters(const World *w, const Player *player, const EntityWorld *ew);
void r_draw_hud(const Player *player);
void r_draw_flash(const Player *player);

#endif /* ENGINE_RENDER_H */
