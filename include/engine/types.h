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

/* 16.16 fixed-point (reserved for later GTE/world math). */
typedef i32 fix16_t;

#define SCREEN_W 320
#define SCREEN_H 256

#endif /* ENGINE_TYPES_H */
