/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 *
 * Plan M6: upload placeholder .VAG ADPCM slabs to SPU RAM and play on
 * rotating voices. Music uses a reserved loop voice (silence or tiny test).
 */
#include "engine/audio.h"

#include <psxspu.h>
#include <hwregs_c.h>
#include <stdio.h>
#include <string.h>

/* Embedded by psn00bsdk_target_incbin */
extern const u8 sfx_pistol[];
extern const u8 sfx_shotgun[];
extern const u8 sfx_chaingun[];
extern const u8 sfx_hurt[];
extern const u8 sfx_footstep[];
extern const u8 sfx_music_loop[];
extern const u8 sfx_silence[];

typedef struct VagHeader {
	u32 magic;
	u32 version;
	u32 interleave;
	u32 size;        /* big-endian */
	u32 sample_rate; /* big-endian */
	u16 reserved[5];
	u16 channels;
	char name[16];
} VagHeader;

typedef struct SfxSlot {
	u32 spu_addr;
	u32 sample_rate;
	u32 size;
} SfxSlot;

#define ALLOC_START_ADDR 0x1010
#define MUSIC_VOICE      23
#define SFX_VOICES       16

#define SWAP32(x) ( \
	(((u32)(x) & 0x000000ffu) << 24) | \
	(((u32)(x) & 0x0000ff00u) <<  8) | \
	(((u32)(x) & 0x00ff0000u) >>  8) | \
	(((u32)(x) & 0xff000000u) >> 24))

static SfxSlot s_sfx[SFX_COUNT];
static SfxSlot s_music_test;
static SfxSlot s_music_silence;
static u32 s_next_addr = ALLOC_START_ADDR;
static u8 s_next_voice;
static MusicId s_music;
static bool s_ready;

static u32 upload_vag(const void *vag_blob, SfxSlot *out)
{
	const VagHeader *h = (const VagHeader *)vag_blob;
	u32 size = SWAP32(h->size);
	u32 rate = SWAP32(h->sample_rate);
	u32 addr = s_next_addr;
	u32 rounded = (size + 63u) & ~63u;
	const u8 *data = (const u8 *)vag_blob + sizeof(VagHeader);

	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(addr);
	SpuWrite((const u32 *)data, rounded);
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

	s_next_addr = addr + rounded;
	out->spu_addr = addr;
	out->sample_rate = rate ? rate : 22050;
	out->size = size;
	return addr;
}

static void voice_play(u8 ch, u32 spu_addr, u32 sample_rate, u16 vol)
{
	SpuSetKey(0, 1u << ch);
	SPU_CH_FREQ(ch) = getSPUSampleRate((int)sample_rate);
	SPU_CH_ADDR(ch) = getSPUAddr(spu_addr);
	SPU_CH_VOL_L(ch) = vol;
	SPU_CH_VOL_R(ch) = vol;
	SPU_CH_ADSR1(ch) = 0x00ff;
	SPU_CH_ADSR2(ch) = 0x0000;
	SpuSetKey(1, 1u << ch);
}

void audio_init(void)
{
	memset(s_sfx, 0, sizeof(s_sfx));
	s_next_addr = ALLOC_START_ADDR;
	s_next_voice = 0;
	s_music = MUSIC_NONE;
	s_ready = false;

	SpuInit();
	SpuSetCommonMasterVolume(0x3fff, 0x3fff);

	upload_vag(sfx_pistol, &s_sfx[SFX_PISTOL]);
	upload_vag(sfx_shotgun, &s_sfx[SFX_SHOTGUN]);
	upload_vag(sfx_chaingun, &s_sfx[SFX_CHAINGUN]);
	upload_vag(sfx_hurt, &s_sfx[SFX_HURT]);
	upload_vag(sfx_footstep, &s_sfx[SFX_FOOTSTEP]);
	upload_vag(sfx_music_loop, &s_music_test);
	upload_vag(sfx_silence, &s_music_silence);

	s_ready = true;
	printf("audio: SPU bank ready (%u bytes SPU RAM used from 0x%x)\n",
		(unsigned)(s_next_addr - ALLOC_START_ADDR), ALLOC_START_ADDR);

	/* Start sequenced/stream stub on silence so the path is live. */
	audio_music_play(MUSIC_SILENCE);
}

void audio_play_sfx(SfxId id)
{
	u8 ch;
	SfxSlot *s;
	if (!s_ready || (u32)id >= (u32)SFX_COUNT) return;
	s = &s_sfx[id];
	if (!s->spu_addr) return;

	ch = s_next_voice % SFX_VOICES;
	s_next_voice = (u8)((ch + 1) % SFX_VOICES);
	voice_play(ch, s->spu_addr, s->sample_rate, 0x2fff);
}

void audio_music_stop(void)
{
	if (!s_ready) return;
	SpuSetKey(0, 1u << MUSIC_VOICE);
	s_music = MUSIC_NONE;
}

void audio_music_play(MusicId id)
{
	SfxSlot *s;
	if (!s_ready) return;
	audio_music_stop();
	if (id == MUSIC_NONE) return;

	if (id == MUSIC_TEST) {
		s = &s_music_test;
	} else {
		s = &s_music_silence;
		id = MUSIC_SILENCE;
	}
	/* Lower volume — background bed / silence loop. */
	voice_play(MUSIC_VOICE, s->spu_addr, s->sample_rate, 0x0fff);
	s_music = id;
}

void audio_update(void)
{
	/* Sequenced music stub: keep reserved voice keyed for active beds.
	 * Future: pattern ticks, XA stream refill, priority ducking. */
	if (!s_ready) return;
	if (s_music == MUSIC_NONE) return;
	/* Re-assert key periodically in case of voice steal (we reserve ch 23). */
	(void)s_music;
}
