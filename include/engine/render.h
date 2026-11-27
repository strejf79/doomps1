/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Renderer — PAL 320×256 double-buffer, OT submit, affine primitives.
 */
#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include "engine/types.h"

void r_init(void);
void r_begin_frame(void);
void r_end_frame(void);

/* Flat/gouraud colored triangle via Ordering Table (affine GPU path). */
void r_draw_tri(
	i16 x0, i16 y0, u8 r0, u8 g0, u8 b0,
	i16 x1, i16 y1, u8 r1, u8 g1, u8 b1,
	i16 x2, i16 y2, u8 r2, u8 g2, u8 b2
);

#endif /* ENGINE_RENDER_H */
