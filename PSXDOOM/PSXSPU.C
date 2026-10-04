// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include <libspu.h>
#include <hwregs_c.h>
#include <psxetc.h>
#include <stdio.h>
#include "psxspu.h"
#include "wessapi.h"

extern void SpuSetCommonAttr (SpuCommonAttr *attr);

#define MALLOC_MAX 1
extern char spu_malloc_rec[SPU_MALLOC_RECSIZ * (MALLOC_MAX + 1)];

extern unsigned int psxspu_initialized;
extern unsigned int psxspu_status;
extern unsigned long end_of_sram;

extern int psxspu_master_vol;
extern int psxspu_master_fadecount;
extern int psxspu_master_vol_fixed;
extern int psxspu_master_destvol;
extern int psxspu_master_changeval;

extern int psxspu_cd_vol;
extern int psxspu_cd_fadecount;
extern int psxspu_cd_vol_fixed;
extern int psxspu_cd_destvol;
extern int psxspu_cd_changeval;

extern SpuReverbAttr rev_attr;//8007EEA4

unsigned long SpuGetReverbOffsetAddr[10] = // equivale a la función SpuGetReverbOffsetAddr()
{
    520192, 514368, 516288, 505792, 495648,
    479776, 461120, 425920, 425920, 508928
};

#define END_O_SRAM 520192
#define SPU_RAM_END 524288

/* Published SPU reverb presets, one row per mode (room through pipe).
   SpuSetReverbModeParam is a stub, so a BIOS boot otherwise keeps the
   logo's coefficients and an uninitialised delay line. */
static const unsigned short rev_preset[9][32] = {
	{ 0x007D, 0x005B, 0x6D80, 0x54B8, 0xBED0, 0x0000, 0x0000, 0xBA80,
	  0x5800, 0x5300, 0x04D6, 0x0333, 0x03F0, 0x0227, 0x0374, 0x01EF,
	  0x0334, 0x01B5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	  0x0000, 0x0000, 0x01B4, 0x0136, 0x00B8, 0x005C, 0x8000, 0x8000 },
	{ 0x0033, 0x0025, 0x70F0, 0x4FA8, 0xBCE0, 0x4410, 0xC0F0, 0x9C00,
	  0x5280, 0x4EC0, 0x03E4, 0x031B, 0x03A4, 0x02AF, 0x0372, 0x0266,
	  0x031C, 0x025D, 0x025C, 0x018E, 0x022F, 0x0135, 0x01D2, 0x00B7,
	  0x018F, 0x00B5, 0x00B4, 0x0080, 0x004C, 0x0026, 0x8000, 0x8000 },
	{ 0x00B1, 0x007F, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xB4C0,
	  0x5280, 0x4EC0, 0x0904, 0x076B, 0x0824, 0x065F, 0x07A2, 0x0616,
	  0x076C, 0x05ED, 0x05EC, 0x042E, 0x050F, 0x0305, 0x0462, 0x02B7,
	  0x042F, 0x0265, 0x0264, 0x01B2, 0x0100, 0x0080, 0x8000, 0x8000 },
	{ 0x00E3, 0x00A9, 0x6F60, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xA680,
	  0x5680, 0x52C0, 0x0DFB, 0x0B58, 0x0D09, 0x0A3C, 0x0BD9, 0x0973,
	  0x0B59, 0x08DA, 0x08D9, 0x05E9, 0x07EC, 0x04B0, 0x06EF, 0x03D2,
	  0x05EA, 0x031D, 0x031C, 0x0238, 0x0154, 0x00AA, 0x8000, 0x8000 },
	{ 0x01A5, 0x0139, 0x6000, 0x5000, 0x4C00, 0xB800, 0xBC00, 0xC000,
	  0x6000, 0x5C00, 0x15BA, 0x11BB, 0x14C2, 0x10BD, 0x11BC, 0x0DC1,
	  0x11C0, 0x0DC3, 0x0DC0, 0x09C1, 0x0BC4, 0x07C1, 0x0A00, 0x06CD,
	  0x09C2, 0x05C1, 0x05C0, 0x041A, 0x0274, 0x013A, 0x8000, 0x8000 },
	{ 0x033D, 0x0231, 0x7E00, 0x5000, 0xB400, 0xB000, 0x4C00, 0xB000,
	  0x6000, 0x5400, 0x1ED6, 0x1A31, 0x1D14, 0x183B, 0x1BC2, 0x16B2,
	  0x1A32, 0x15EF, 0x15EE, 0x1055, 0x1334, 0x0F2D, 0x11F6, 0x0C5D,
	  0x1056, 0x0AE1, 0x0AE0, 0x07A2, 0x0464, 0x0232, 0x8000, 0x8000 },
	{ 0x0001, 0x0001, 0x7FFF, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x8100,
	  0x0000, 0x0000, 0x1FFF, 0x0FFF, 0x1005, 0x0005, 0x0000, 0x0000,
	  0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	  0x0000, 0x0000, 0x1004, 0x1002, 0x0004, 0x0002, 0x8000, 0x8000 },
	{ 0x0001, 0x0001, 0x7FFF, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000,
	  0x0000, 0x0000, 0x1FFF, 0x0FFF, 0x1005, 0x0005, 0x0000, 0x0000,
	  0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	  0x0000, 0x0000, 0x1004, 0x1002, 0x0004, 0x0002, 0x8000, 0x8000 },
	{ 0x0017, 0x0013, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0x8500,
	  0x5F80, 0x54C0, 0x0371, 0x02AF, 0x02E5, 0x01DF, 0x02B0, 0x01D7,
	  0x0358, 0x026A, 0x01D6, 0x011E, 0x012D, 0x00B1, 0x011F, 0x0059,
	  0x01A0, 0x00E3, 0x0058, 0x0040, 0x0028, 0x0014, 0x8000, 0x8000 }
};

static unsigned int spu_zero_chunk[256];

static void psxspu_load_reverb_preset(int mode)
{
	const unsigned short *preset;
	int i;

	preset = rev_preset[mode - 1];
	for (i = 0; i < 32; i++)
		_MMIO16(IOBASE | (0x1dc0 + (i * 2))) = preset[i];
}

static void psxspu_clear_sram(unsigned long start, unsigned long end)
{
	SPU_TransferMode prev;
	unsigned long pos;

	prev = SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	for (pos = start; pos < end; )
	{
		unsigned long n = end - pos;

		if (n > sizeof(spu_zero_chunk))
			n = sizeof(spu_zero_chunk);
		n &= ~15UL;
		if (n == 0)
			break;
		SpuSetTransferStartAddr((uint32_t)pos);
		SpuWrite(spu_zero_chunk, (size_t)n);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
		pos += n;
	}
	SpuSetTransferMode(prev);
}

void psxspu_init_reverb(long  rev_mode, short depthleft, short depthright, long  delay, long  feedback)//80045098
{
	unsigned long rev_addr;

	(void)delay;
	(void)feedback;
	psxspu_status = 0;

	rev_attr.mask = (SPU_REV_MODE|SPU_REV_DEPTHL|SPU_REV_DEPTHR|SPU_REV_DELAYTIME|SPU_REV_FEEDBACK);
	rev_attr.mode = rev_mode | SPU_REV_MODE_CLEAR_WA;
	rev_attr.depth.left = 0;
	rev_attr.depth.right = 0;
	rev_attr.delay = delay;
	rev_attr.feedback = feedback;

	/* Depth stays 0 until the delay line is zero. A BIOS boot leaves the
	   logo in that memory, and enabling reverb plays it back as a screech. */
	SpuSetReverb(SPU_OFF);
	SpuSetReverbDepth(&rev_attr);
	SpuSetReverbVoice(SPU_OFF, SPU_ALLCH);

	if (rev_mode == PSXSPU_REV_MODE_OFF || rev_mode > PSXSPU_REV_MODE_PIPE)
	{
		end_of_sram = END_O_SRAM;
		psxspu_status = 1;
		return;
	}

	rev_addr = SpuGetReverbOffsetAddr[rev_mode];
	end_of_sram = rev_addr;
	SpuSetReverbAddr(rev_addr);
	psxspu_load_reverb_preset((int)rev_mode);
	psxspu_clear_sram(rev_addr, SPU_RAM_END);
	printf("PORT reverb-clear mode=%ld\n", rev_mode);

	SpuSetReverb(SPU_ON);
	VSync(2);
	rev_attr.depth.left = depthleft;
	rev_attr.depth.right = depthright;
	SpuSetReverbDepth(&rev_attr);
	SpuSetReverbVoice(SPU_ON, SPU_ALLCH);
	psxspu_status = 1;
}

void psxspu_set_reverb_depth(short depthleft,short depthright)//80045178
{
    psxspu_status = 0;
    rev_attr.depth.left = depthleft;
	rev_attr.depth.right = depthright;
	SpuSetReverbDepth(&rev_attr);
	psxspu_status = 1;

}

/* 16 silent ADPCM bytes that loop on themselves. SpuInit's dummy block
   does not loop, so a voice left running walks into whatever a BIOS boot
   stored after it. */
static unsigned char spu_silent_loop[16] = {
	0x00, 0x03, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void psxspu_plant_silence(void)
{
	SPU_TransferMode prev;
	int ch;

	prev = SpuSetTransferMode(SPU_TRANSFER_BY_IO);
	SpuSetTransferStartAddr(0x1000);
	SpuWrite((const uint32_t *)spu_silent_loop, 16);
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
	SpuSetTransferMode(prev);

	for (ch = 0; ch < 24; ch++)
	{
		SPU_CH_VOL_L(ch) = 0;
		SPU_CH_VOL_R(ch) = 0;
		SPU_CH_ADDR(ch) = 0x200;
		SPU_CH_LOOP_ADDR(ch) = 0x200;
	}
	/* SpuInit keys every voice on. Key-off in that same slice is ignored,
	   so wait a frame and key off again. */
	SpuSetKey(SPU_OFF, SPU_ALLCH);
	VSync(0);
	SpuSetKey(SPU_OFF, SPU_ALLCH);
}

void psxspu_halt_for_upload(void)
{
	psxspu_set_cd_vol(0);
	psxspu_setcdmixoff();
	SPU_CD_VOL_L = 0;
	SPU_CD_VOL_R = 0;
	psxspu_plant_silence();
	printf("PORT spu-halt\n");
}

void psxspu_init(void)//800451c0
{
    SpuCommonAttr com_attr;

    if (psxspu_initialized == 0)
	{
	    //printf("psxspu_init\n");
		psxspu_status = 0;
		psxspu_initialized = 1;

		SpuInit();
		psxspu_plant_silence();
		SpuInitMalloc(MALLOC_MAX, spu_malloc_rec);
		SpuSetTransMode(SpuTransferByDMA);

		psxspu_init_reverb(0, 0, 0, 0, 0);

		com_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDVOLR| SPU_COMMON_CDREV | SPU_COMMON_CDMIX);
		com_attr.mvol.left = MAX_MASTER_VOL;
		com_attr.mvol.right = MAX_MASTER_VOL;
		com_attr.cd.volume.left = MAX_CD_VOL;
		com_attr.cd.volume.right = MAX_CD_VOL;
		com_attr.cd.reverb = 0;
		com_attr.cd.mix = 1;
		SpuSetCommonAttr(&com_attr);
		psxspu_status = 1;
	}
}

void psxspu_set_master_volume(int vol)//8004526C
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
    com_attr.mvol.left = vol;
	com_attr.mvol.right = vol;
	com_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_set_cd_volume(int vol)//800452B0
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
    com_attr.cd.volume.left = vol;
	com_attr.cd.volume.right = vol;
	com_attr.mask = (SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR);
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_setcdmixon(void)//800452F4
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
	com_attr.mask = (SPU_COMMON_CDMIX);
	com_attr.cd.mix = 1;
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_setcdmixoff(void)//8004533C
{
    SpuCommonAttr com_attr;

    psxspu_status = 0;
	com_attr.mask = (SPU_COMMON_CDMIX);
	com_attr.cd.mix = 0;
	SpuSetCommonAttr(&com_attr);
	psxspu_status = 1;
}

void psxspu_fadeengine(void)//8004537C
{
    if (psxspu_status != 0)
	{
		if (psxspu_cd_fadecount > 0)
		{
			psxspu_cd_fadecount -= 1;
			psxspu_cd_vol_fixed += psxspu_cd_changeval;

			if (psxspu_cd_fadecount == 0)
				psxspu_cd_vol_fixed = psxspu_cd_destvol;

			psxspu_cd_vol = psxspu_cd_vol_fixed >> 16;

			psxspu_set_cd_volume(psxspu_cd_vol);
		}

		if (psxspu_master_fadecount > 0)
		{
			psxspu_master_fadecount -= 1;
			psxspu_master_vol_fixed += psxspu_master_changeval;

			if (psxspu_master_fadecount == 0)
				psxspu_master_vol_fixed = psxspu_master_destvol;

			psxspu_master_vol = psxspu_master_vol_fixed >> 16;
			psxspu_set_master_volume(psxspu_master_vol);
		}

		/* CD-DA is already being read off the disc. Keep the SPU input open
		   for as long as the game wants that music audible. */
		if (psxspu_cd_vol > 0)
			PsyqSpuOutputCd(psxspu_cd_vol);
	}
}

void psxspu_set_cd_vol(int vol)//80045490
{
    psxspu_status = 0;
    psxspu_cd_vol = vol;
	psxspu_cd_vol_fixed = psxspu_cd_vol << 16;
	psxspu_cd_fadecount = 0;
	psxspu_set_cd_volume(psxspu_cd_vol);
	psxspu_status = 1;
}

int  psxspu_get_cd_vol(void)//800454EC
{
    return psxspu_cd_vol;
}

void psxspu_start_cd_fade(int msec, int destvol)//800454FC
{
    psxspu_status = 0;
    if (WessTimerActive == 0)
    {
        psxspu_cd_fadecount = 0;
    }
    else
    {
        psxspu_cd_destvol = destvol * 0x10000;
        psxspu_cd_fadecount = (msec * 0x78) / 1000 + 1;
        psxspu_cd_changeval = (psxspu_cd_destvol - psxspu_cd_vol_fixed) / psxspu_cd_fadecount;
    }
    psxspu_status = 1;
}

void psxspu_stop_cd_fade(void)//800455B4
{
    psxspu_status = 0;
	psxspu_cd_fadecount = 0;
	psxspu_status = 1;
}

int  psxspu_get_cd_fade_status(void)//800455D8
{
    return (psxspu_cd_fadecount < 2) ^ 1;
}

void psxspu_set_master_vol(int vol)//800455F0
{
    psxspu_status = 0;
    psxspu_master_vol = vol;
	psxspu_master_vol_fixed = psxspu_master_vol << 16;
	psxspu_master_fadecount = 0;
	psxspu_set_master_volume(psxspu_master_vol);
	psxspu_status = 1;
}

int  psxspu_get_master_vol(void)//8004564C
{
    return psxspu_master_vol;
}

void psxspu_start_master_fade(int msec, int destvol)//8004565C
{
    psxspu_status = 0;
	if (WessTimerActive == 0)
    {
        psxspu_master_fadecount = 0;
    }
    else
    {
        psxspu_master_destvol = destvol * 0x10000;
        psxspu_master_fadecount = (msec * 0x78) / 1000 + 1;
        psxspu_master_changeval = (psxspu_master_destvol - psxspu_master_vol_fixed) / psxspu_master_fadecount;
    }
    psxspu_status = 1;
}

void psxspu_stop_master_fade(void)//80045714
{
    psxspu_status = 0;
	psxspu_master_fadecount = 0;
	psxspu_status = 1;
}

int  psxspu_get_master_fade_status(void)//80045738
{
    return (psxspu_master_fadecount < 2) ^ 1;
}
