/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Common fixed-width aliases for the engine (C99).
 */
#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;

/* 16.16 fixed-point (reserved). Runtime uses GTE ONE=4096 (12-bit frac). */
typedef i32 fix16_t;

#define SCREEN_W 320
#define SCREEN_H 256

/* Hard vis budget for M1 (plan: ≤32–48 later). */
#define MAX_SECTORS     8
#define MAX_WALLS       32
#define MAX_VIS_SECTORS 8

#endif /* ENGINE_TYPES_H */
