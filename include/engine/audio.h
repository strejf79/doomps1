/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Audio — SPU ADPCM SFX bank + sequenced/stream music stub (plan M6).
 */
#ifndef ENGINE_AUDIO_H
#define ENGINE_AUDIO_H

#include "engine/types.h"

typedef enum SfxId {
	SFX_PISTOL = 0,
	SFX_SHOTGUN,
	SFX_CHAINGUN,
	SFX_HURT,
	SFX_FOOTSTEP,
	SFX_COUNT
} SfxId;

typedef enum MusicId {
	MUSIC_NONE = 0,
	MUSIC_SILENCE, /* looping silence — proves stream/seq path */
	MUSIC_TEST,    /* tiny looping tone */
	MUSIC_COUNT
} MusicId;

void audio_init(void);
void audio_update(void);

void audio_play_sfx(SfxId id);
void audio_music_play(MusicId id);
void audio_music_stop(void);

#endif /* ENGINE_AUDIO_H */
