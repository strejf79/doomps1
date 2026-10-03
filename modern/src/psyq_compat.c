#include <libcomb.h>
#include <libapi.h>
#include <libcd.h>
#include <libetc.h>
#include <libgpu.h>
#include <libspu.h>

static unsigned char pad_buffers[2][34];
static unsigned char cd_result_buffer[8];
static volatile int cd_ready_status;
static CdlCB cd_ready_chain;
static volatile unsigned long vsync_counter;
static int vsync_initialized;
static PsyqProfileStats profile_stats;

unsigned long _ramsize = 0x00200000;
unsigned long _stacksize = 0x00008000;

int PsyqResetCallback(void) {
	return 0;
}

#undef CdControl
#undef CdGetSector
#undef DrawOTag
#undef LoadImage
#undef VSync

static void count_vsync(void) {
	vsync_counter++;
}

int PsyqVSync(int mode) {
	unsigned long before;

	if (!vsync_initialized) {
		VSyncCallback(count_vsync);
		vsync_initialized = 1;
	}
	if (mode < 0)
		return (int)vsync_counter;
	before = vsync_counter;
	VSync(mode);
	return (int)(vsync_counter - before);
}

int PsyqDrawOTag(const unsigned long *tag) {
	profile_stats.ot_submissions++;
	DrawSync(0);
	return DrawOTag((const uint32_t *)tag);
}

int PsyqLoadImage(const RECT *rect, const unsigned long *data) {
	unsigned int bytes = (unsigned int)rect->w * (unsigned int)rect->h * 2;
	unsigned int words = (bytes + 3) / 4;
	unsigned int i;
	profile_stats.upload_calls++;
	profile_stats.upload_bytes += bytes;
	profile_stats.upload_words += words;
	if (words > profile_stats.upload_max_words)
		profile_stats.upload_max_words = words;
	/*
	 * PSn00bSDK 0.24 rounds non-multiple-of-16 DMA transfers up incorrectly,
	 * leaving the GPU in image-transfer mode (18x56 = 504 words exposed it).
	 * Multiples of 16 exactly fit its DMA block shape and are safe.
	 */
	DrawSync(0);
	if (words >= 16 && !(words & 15)) {
		profile_stats.upload_dma_calls++;
		profile_stats.upload_dma_words += words;
		LoadImage2(rect, (const uint32_t *)data);
		DrawSync(0);
		return 0;
	}
	GPU_GP0 = 0xa0000000;
	GPU_GP0 = ((unsigned int)(unsigned short)rect->y << 16)
		| (unsigned short)rect->x;
	GPU_GP0 = ((unsigned int)(unsigned short)rect->h << 16)
		| (unsigned short)rect->w;
	for (i = 0; i < words; i++) {
		while (!(GPU_GP1 & (1UL << 28)))
			;
		GPU_GP0 = data[i];
	}
	return 0;
}

void PsyqGetProfileStats(PsyqProfileStats *stats) {
	*stats = profile_stats;
}

static void cd_ready_callback(CdlIntrResult status, unsigned char *result) {
	cd_ready_status = status;
	if (cd_ready_chain)
		cd_ready_chain(status, result);
}

int PsyqCdControl(CdlCommand cmd, const void *param, unsigned char *result) {
	if (cmd == CdlReadN || cmd == CdlReadS) {
		CdlCB previous;
		cd_ready_status = CdlNoIntr;
		previous = CdReadyCallback(cd_ready_callback);
		if (previous != cd_ready_callback)
			cd_ready_chain = previous;
	}
	return CdControl(cmd, param, result ? result : cd_result_buffer);
}

int PsyqCdGetSector(void *address, int words) {
	int result = CdGetSector(address, words);
	cd_ready_status = CdlNoIntr;
	return result;
}

int CdReady(int mode, unsigned char *result) {
	(void)mode;
	if (result)
		result[0] = (unsigned char)CdStatus();
	return cd_ready_status;
}

void CdFlush(void) {
	cd_ready_status = CdlNoIntr;
}

static unsigned long encode_texture_window(const RECT *tw) {
	unsigned long mask_x;
	unsigned long mask_y;

	if (!tw)
		return 0xe2000000u;

	/*
	 * PsyQ's SetTexWindow takes a rectangle in texels. PSn00bSDK's
	 * setTexWindow macro instead takes the GPU's pre-encoded 8-texel
	 * mask and offset fields, so perform the PsyQ conversion here.
	 */
	mask_x = ((unsigned long)~(tw->w - 1) >> 3) & 0x1f;
	mask_y = ((unsigned long)~(tw->h - 1) >> 3) & 0x1f;
	return 0xe2000000u
		| mask_x
		| (mask_y << 5)
		| (((unsigned long)tw->x >> 3 & 0x1f) << 10)
		| (((unsigned long)tw->y >> 3 & 0x1f) << 15);
}

DR_TWIN *PsyqSetTexWindow(DR_TWIN *packet, const RECT *tw) {
	packet->code[0] = encode_texture_window(tw);
	setlen(packet, 1);
	return packet;
}

DR_MODE *SetDrawMode(DR_MODE *packet, int dfe, int dtd, int tpage, RECT *tw) {
	packet->code[0] = 0xe1000000u
		| (unsigned long)tpage
		| ((unsigned long)(dtd & 1) << 9)
		| ((unsigned long)(dfe & 1) << 10);
	packet->code[1] = encode_texture_window(tw);
	setlen(packet, 2);
	return packet;
}

void CombSetBPS(int baud) {
	(void)baud;
}

int CombSioStatus(void) {
	return 0;
}

int CombCTS(void) {
	return 0;
}

void CombCancelRead(void) {
}

void CombResetError(void) {
}

void AddCOMB(void) {
}

void SetDumpFnt(int id) {
	(void)id;
}

void PadInit(int mode) {
	(void)mode;
	InitPAD(pad_buffers[0], sizeof(pad_buffers[0]),
		pad_buffers[1], sizeof(pad_buffers[1]));
	StartPAD();
}

unsigned long PadRead(int port) {
	PADTYPE *pad;

	if ((unsigned int)port > 1)
		return 0;
	pad = (PADTYPE *)pad_buffers[port];
	if (pad->stat != 0 || pad->type == PAD_ID_NONE)
		return 0;
	return (unsigned short)~pad->btn;
}

void SpuInitMalloc(int max, char *table) {
	(void)max;
	(void)table;
}

int SpuSetTransMode(int mode) {
	return (int)SpuSetTransferMode((SPU_TransferMode)mode);
}

/* Last value written to SPU_CTRL. Reading that register back during a
   transfer can return 0, and storing that clears unmute and SPU enable. */
static unsigned short spu_ctrl_shadow = 0xC001;
static unsigned long spu_key_mask;

static void spu_ctrl_write(unsigned short value) {
	unsigned short hardware = SPU_CTRL;
	unsigned short transfer = hardware & 0x0030;

	/* A transfer in progress can read back as 0. Keep the mode we last set. */
	if (transfer == 0)
		transfer = spu_ctrl_shadow & 0x0030;
	spu_ctrl_shadow = (value & (unsigned short)~0x0030) | 0xC000 | transfer;
	SPU_CTRL = spu_ctrl_shadow;
}

void PsyqSpuOutputCd(int volume) {
	if (volume < 0)
		volume = 0;
	if (volume > 0x3fff)
		volume = 0x3fff;
	SPU_CD_VOL_L = (unsigned short)volume;
	SPU_CD_VOL_R = (unsigned short)volume;
	/* Rewriting SPU_CTRL every tick forces the transfer mode to stopped and
	   aborts sample DMA. The enable bit only needs to be set once. */
	if (volume > 0 && !(spu_ctrl_shadow & 1))
		spu_ctrl_write(spu_ctrl_shadow | 1);
}

void SpuSetCommonAttr(SpuCommonAttr *attr) {
	if (attr->mask & SPU_COMMON_MVOLL)
		SPU_MASTER_VOL_L = attr->mvol.left;
	if (attr->mask & SPU_COMMON_MVOLR)
		SPU_MASTER_VOL_R = attr->mvol.right;
	if (attr->mask & SPU_COMMON_CDVOLL)
		SPU_CD_VOL_L = attr->cd.volume.left;
	if (attr->mask & SPU_COMMON_CDVOLR)
		SPU_CD_VOL_R = attr->cd.volume.right;
	if (attr->mask & SPU_COMMON_CDREV) {
		if (attr->cd.reverb)
			spu_ctrl_write(spu_ctrl_shadow | 0x0004);
		else
			spu_ctrl_write(spu_ctrl_shadow & (unsigned short)~0x0004);
	}
	if (attr->mask & SPU_COMMON_CDMIX) {
		if (attr->cd.mix)
			spu_ctrl_write(spu_ctrl_shadow | 1);
		else
			spu_ctrl_write(spu_ctrl_shadow & (unsigned short)~1);
	}
}

void SpuSetReverbModeParam(SpuReverbAttr *attr) {
	(void)attr;
}

void SpuSetReverbDepth(SpuReverbAttr *attr) {
	SPU_REVERB_VOL_L = attr->depth.left;
	SPU_REVERB_VOL_R = attr->depth.right;
}

void SpuSetReverb(int on) {
	if (on)
		spu_ctrl_write(spu_ctrl_shadow | 0x0080);
	else
		spu_ctrl_write(spu_ctrl_shadow & (unsigned short)~0x0080);
}

void SpuSetReverbVoice(int on, unsigned long voices) {
	if (on) {
		SPU_REVERB_ON1 |= (unsigned short)voices;
		SPU_REVERB_ON2 |= (unsigned short)(voices >> 16);
	} else {
		SPU_REVERB_ON1 &= (unsigned short)~voices;
		SPU_REVERB_ON2 &= (unsigned short)~(voices >> 16);
	}
}

static unsigned short note_to_pitch(unsigned short note, unsigned short base) {
	static const unsigned short scale[12] = {
		4096, 4339, 4598, 4871, 5161, 5468,
		5793, 6137, 6502, 6889, 7298, 7732
	};
	int semitones = ((int)note - (int)base) >> 8;
	int octave = semitones / 12;
	int step = semitones % 12;
	unsigned long pitch;

	if (step < 0) {
		step += 12;
		octave--;
	}
	pitch = scale[step];
	if (octave >= 0)
		pitch <<= octave;
	else
		pitch >>= -octave;
	if (pitch > 0x3fff)
		pitch = 0x3fff;
	if (pitch == 0)
		pitch = 1;
	return (unsigned short)pitch;
}

void SpuSetVoiceAttr(SpuVoiceAttr *attr) {
	int channel;

	for (channel = 0; channel < 24; channel++) {
		if (!(attr->voice & (1UL << channel)))
			continue;
		if (attr->mask & SPU_VOICE_VOLL)
			SPU_CH_VOL_L(channel) = attr->volume.left;
		if (attr->mask & SPU_VOICE_VOLR)
			SPU_CH_VOL_R(channel) = attr->volume.right;
		if (attr->mask & SPU_VOICE_PITCH)
			SPU_CH_FREQ(channel) = attr->pitch;
		if (attr->mask & (SPU_VOICE_NOTE | SPU_VOICE_SAMPLE_NOTE))
			SPU_CH_FREQ(channel) = note_to_pitch(attr->note, attr->sample_note);
		if (attr->mask & SPU_VOICE_WDSA)
			SPU_CH_ADDR(channel) = getSPUAddr(attr->addr);
		if (attr->mask & SPU_VOICE_ADSR_ADSR1)
			SPU_CH_ADSR1(channel) = attr->adsr1;
		if (attr->mask & SPU_VOICE_ADSR_ADSR2)
			SPU_CH_ADSR2(channel) = attr->adsr2;
		if (attr->mask & SPU_VOICE_ADSR_RR)
			SPU_CH_ADSR2(channel) =
				(SPU_CH_ADSR2(channel) & 0xffc0) | (attr->rr & 0x3f);
	}
}

void PsyqSpuSetKey(int enable, unsigned long voice_bit) {
	voice_bit &= 0x00ffffffUL;
	/* Same split as psxspu.h: key-on and key-off are separate writes.
	   Setting both latches together makes the new note die on key-off. */
	if (enable) {
		spu_key_mask |= voice_bit;
		SPU_KEY_ON1 = (unsigned short)voice_bit;
		SPU_KEY_ON2 = (unsigned short)(voice_bit >> 16);
	} else {
		spu_key_mask &= ~voice_bit;
		SPU_KEY_OFF1 = (unsigned short)voice_bit;
		SPU_KEY_OFF2 = (unsigned short)(voice_bit >> 16);
	}
}

void SpuSetKeyOnWithAttr(SpuVoiceAttr *attr) {
	/* sound/vagsample: stop this channel, program it, then start it.
	   A key-on is ignored while the voice is still on. */
	SpuSetKey(SPU_OFF, attr->voice);
	SpuSetVoiceAttr(attr);
	SpuSetKey(SPU_ON, attr->voice);
}

void SpuGetAllKeysStatus(char *status) {
	int channel;
	for (channel = 0; channel < 24; channel++) {
		if (!(spu_key_mask & (1UL << channel)))
			status[channel] = SPU_OFF;
		else if (SPU_CH_ADSR_VOL(channel) == 0)
			status[channel] = 2;
		else
			status[channel] = SPU_ON;
	}
}

void SpuQuit(void) {
	SpuSetKey(SPU_OFF, SPU_ALLCH);
}
