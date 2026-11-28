/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * CD / disc layout helpers — prefetch stubs + fail-soft seeks (plan M5).
 */
#ifndef ENGINE_DISC_H
#define ENGINE_DISC_H

#include "engine/types.h"

typedef enum DiscStatus {
	DISC_OK = 0,
	DISC_NO_DRIVE,
	DISC_SEEK_FAIL,
	DISC_READ_FAIL,
	DISC_NOT_FOUND
} DiscStatus;

typedef struct DiscPrefetchSlot {
	char path[32];
	u8   priority; /* 0 = highest */
	u8   queued;
	u8   resident;
	u8   failed;   /* fail-soft: stay playable without this chunk */
} DiscPrefetchSlot;

#define DISC_PREFETCH_MAX 8

void disc_init(void);
void disc_update(void);

/* Non-blocking: mark a path for background prefetch. Never stalls combat. */
void disc_prefetch(const char *iso_path, u8 priority);

/* Fail-soft seek/read: returns DISC_OK and fills buf, or soft-fails. */
DiscStatus disc_seek_read(const char *iso_path, void *buf, u32 buf_cap, u32 *out_size);

DiscStatus disc_last_error(void);
const char *disc_status_str(DiscStatus s);

/* True when CdInit succeeded (DuckStation CD image or hardware). */
bool disc_ready(void);

#endif /* ENGINE_DISC_H */
