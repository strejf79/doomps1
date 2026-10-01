#pragma once

#include <psxgte.h>
#include <inline_c.h>

#define SetRotMatrix(matrix) gte_SetRotMatrix(matrix)
#define SetTransMatrix(matrix) gte_SetTransMatrix(matrix)
#define SetGeomOffset(x, y) gte_SetGeomOffset((x), (y))
#define SetGeomScreen(distance) gte_SetGeomScreen(distance)
#define RotTrans(input, output, flag) do { \
	gte_ldv0(input); \
	gte_rt(); \
	gte_stlvnl(output); \
	gte_stflg(flag); \
} while (0)
