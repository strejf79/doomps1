/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * Plan M5 disc scaffolding: CdInit, fail-soft CdSearchFile/CdRead,
 * and a non-blocking prefetch queue. Real hardware seek timing is
 * documented for DuckStation-as-proxy — we never hitch the sim tick.
 */
#include "engine/disc.h"

#include <psxcd.h>
#include <psxgpu.h>
#include <stdio.h>
#include <string.h>

static bool s_ready;
static DiscStatus s_last;
static DiscPrefetchSlot s_slots[DISC_PREFETCH_MAX];
static u8 s_slot_count;
static u8 s_cursor;

const char *disc_status_str(DiscStatus s)
{
	switch (s) {
	case DISC_OK: return "ok";
	case DISC_NO_DRIVE: return "no_drive";
	case DISC_SEEK_FAIL: return "seek_fail";
	case DISC_READ_FAIL: return "read_fail";
	case DISC_NOT_FOUND: return "not_found";
	default: return "?";
	}
}

DiscStatus disc_last_error(void)
{
	return s_last;
}

bool disc_ready(void)
{
	return s_ready;
}

void disc_init(void)
{
	memset(s_slots, 0, sizeof(s_slots));
	s_slot_count = 0;
	s_cursor = 0;
	s_last = DISC_OK;

	/* CdInit after ResetGraph (plat_init). Soft-fail if no drive. */
	if (!CdInit()) {
		s_ready = false;
		s_last = DISC_NO_DRIVE;
		printf("disc: CdInit failed — embedded assets only (fail-soft)\n");
		return;
	}

	s_ready = true;
	s_last = DISC_OK;
	printf("disc: CdInit ok (DuckStation CD image or hardware)\n");

	/* Warm the ISOFS cache with a nop so lid/status stays fresh. */
	CdControl(CdlNop, 0, 0);
}

void disc_prefetch(const char *iso_path, u8 priority)
{
	u8 i;
	if (!iso_path || !iso_path[0]) return;

	for (i = 0; i < s_slot_count; i++) {
		if (strncmp(s_slots[i].path, iso_path, sizeof(s_slots[i].path) - 1) == 0) {
			if (priority < s_slots[i].priority) {
				s_slots[i].priority = priority;
			}
			s_slots[i].queued = 1;
			return;
		}
	}
	if (s_slot_count >= DISC_PREFETCH_MAX) return;

	{
		DiscPrefetchSlot *s = &s_slots[s_slot_count++];
		memset(s, 0, sizeof(*s));
		strncpy(s->path, iso_path, sizeof(s->path) - 1);
		s->priority = priority;
		s->queued = 1;
	}
}

DiscStatus disc_seek_read(const char *iso_path, void *buf, u32 buf_cap, u32 *out_size)
{
	CdlFILE file;
	int sectors;
	u8 result[8];

	if (out_size) *out_size = 0;
	if (!buf || buf_cap == 0 || !iso_path) {
		s_last = DISC_READ_FAIL;
		return s_last;
	}
	if (!s_ready) {
		s_last = DISC_NO_DRIVE;
		return s_last;
	}

	memset(&file, 0, sizeof(file));
	if (!CdSearchFile(&file, (char *)iso_path)) {
		s_last = DISC_NOT_FOUND;
		printf("disc: miss %s (fail-soft)\n", iso_path);
		return s_last;
	}

	sectors = (int)((file.size + 2047) / 2048);
	if ((u32)file.size > buf_cap) {
		/* Truncate rather than stall — combat working set must stay resident. */
		sectors = (int)(buf_cap / 2048);
		if (sectors < 1) {
			s_last = DISC_READ_FAIL;
			return s_last;
		}
	}

	CdControl(CdlSetloc, (u8 *)&file.pos, 0);
	if (CdRead(sectors, (u32 *)buf, CdlModeSpeed) == 0) {
		s_last = DISC_SEEK_FAIL;
		printf("disc: seek/read start fail %s (fail-soft)\n", iso_path);
		return s_last;
	}

	/* Bounded wait — never hang the boot forever. */
	{
		int spins = 0;
		while (CdReadSync(1, result) > 0) {
			if (++spins > 60 * 10) { /* ~10s @ 60Hz poll budget */
				CdControl(CdlPause, 0, 0);
				s_last = DISC_SEEK_FAIL;
				printf("disc: slow seek timeout %s (fail-soft)\n", iso_path);
				return s_last;
			}
			VSync(0);
		}
	}

	if (CdIsoError() != CdlIsoOkay) {
		s_last = DISC_READ_FAIL;
		printf("disc: read error %s (fail-soft)\n", iso_path);
		return s_last;
	}

	if (out_size) {
		u32 n = (u32)file.size;
		if (n > buf_cap) n = buf_cap;
		*out_size = n;
	}
	s_last = DISC_OK;
	return DISC_OK;
}

void disc_update(void)
{
	DiscPrefetchSlot *best;
	u8 i;
	u8 best_i;
	static u8 scratch[2048];
	u32 got;

	if (!s_ready || s_slot_count == 0) return;

	/* One cheap nop per frame keeps lid/status current without seeking. */
	if ((s_cursor++ & 31) == 0) {
		CdControlF(CdlNop, 0);
	}

	/* Pick highest-priority queued slot; attempt a 1-sector warm seek. */
	best = 0;
	best_i = 0;
	for (i = 0; i < s_slot_count; i++) {
		if (!s_slots[i].queued || s_slots[i].resident || s_slots[i].failed) {
			continue;
		}
		if (!best || s_slots[i].priority < best->priority) {
			best = &s_slots[i];
			best_i = i;
		}
	}
	if (!best) return;

	/* Fail-soft: mark resident on success, failed on miss — gameplay continues. */
	if (disc_seek_read(best->path, scratch, sizeof(scratch), &got) == DISC_OK) {
		best->resident = 1;
		best->queued = 0;
		printf("disc: prefetch ok [%u] %s (%u bytes warm)\n", best_i, best->path, got);
	} else {
		best->failed = 1;
		best->queued = 0;
		printf("disc: prefetch soft-fail [%u] %s\n", best_i, best->path);
	}
}
