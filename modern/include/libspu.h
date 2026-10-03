#pragma once

#include <psxspu.h>

typedef struct {
	short left;
	short right;
} SpuVolume;

typedef struct {
	unsigned long voice;
	unsigned long mask;
	SpuVolume volume;
	SpuVolume volmode;
	SpuVolume volumex;
	unsigned short pitch;
	unsigned short note;
	unsigned short sample_note;
	short envx;
	unsigned long addr;
	unsigned long loop_addr;
	int a_mode;
	int s_mode;
	int r_mode;
	unsigned short ar;
	unsigned short dr;
	unsigned short sr;
	unsigned short rr;
	unsigned short sl;
	unsigned short adsr1;
	unsigned short adsr2;
} SpuVoiceAttr;

typedef struct {
	SpuVolume volume;
	int reverb;
	int mix;
} SpuExtAttr;

typedef struct {
	unsigned long mask;
	SpuVolume mvol;
	SpuVolume mvolmode;
	SpuVolume mvolx;
	SpuExtAttr cd;
	SpuExtAttr ext;
} SpuCommonAttr;

typedef struct {
	unsigned long mask;
	long mode;
	SpuVolume depth;
	long delay;
	long feedback;
} SpuReverbAttr;

#define SPU_MALLOC_RECSIZ 8
#define SPU_ON 1
#define SPU_OFF 0
#define SPU_ALLCH 0x00ffffffUL
#define SpuTransferByDMA SPU_TRANSFER_BY_DMA

#define SPU_COMMON_MVOLL  (1UL << 0)
#define SPU_COMMON_MVOLR  (1UL << 1)
#define SPU_COMMON_CDVOLL (1UL << 8)
#define SPU_COMMON_CDVOLR (1UL << 9)
#define SPU_COMMON_CDREV  (1UL << 10)
#define SPU_COMMON_CDMIX  (1UL << 11)

#define SPU_VOICE_VOLL       (1UL << 0)
#define SPU_VOICE_VOLR       (1UL << 1)
#define SPU_VOICE_PITCH      (1UL << 4)
#define SPU_VOICE_NOTE       (1UL << 5)
#define SPU_VOICE_SAMPLE_NOTE (1UL << 6)
#define SPU_VOICE_WDSA       (1UL << 7)
#define SPU_VOICE_ADSR_RMODE (1UL << 10)
#define SPU_VOICE_ADSR_RR    (1UL << 14)
#define SPU_VOICE_ADSR_ADSR1 (1UL << 17)
#define SPU_VOICE_ADSR_ADSR2 (1UL << 18)
#define SPU_VOICE_EXPDec 1

#define SPU_REV_MODE      (1UL << 0)
#define SPU_REV_DEPTHL    (1UL << 1)
#define SPU_REV_DEPTHR    (1UL << 2)
#define SPU_REV_DELAYTIME (1UL << 3)
#define SPU_REV_FEEDBACK  (1UL << 4)
#define SPU_REV_MODE_CLEAR_WA (1L << 8)

void SpuInitMalloc(int max, char *table);
int SpuSetTransMode(int mode);
void SpuSetCommonAttr(SpuCommonAttr *attr);
void SpuSetReverbModeParam(SpuReverbAttr *attr);
void SpuSetReverbDepth(SpuReverbAttr *attr);
void SpuSetReverb(int on);
void SpuSetReverbVoice(int on, unsigned long voices);
void SpuSetVoiceAttr(SpuVoiceAttr *attr);
void PsyqSpuSetKey(int enable, unsigned long voice_bit);
void PsyqSpuOutputCd(int volume);
void SpuSetKeyOnWithAttr(SpuVoiceAttr *attr);
void SpuGetAllKeysStatus(char *status);
void SpuQuit(void);

/* The SDK macro writes the key registers and nothing else. Sound effects need
   to know which voices were keyed on, because the envelope volume stays 0 for
   the first tick after key-on. */
#undef SpuSetKey
#define SpuSetKey(enable, voice_bit) PsyqSpuSetKey((int)(enable), (unsigned long)(voice_bit))
