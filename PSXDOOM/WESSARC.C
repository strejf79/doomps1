// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

    /*------------------------------------------------------------------*/
    /*
                     The Williams Entertainment Sound System
                     Sony Architecture Specific Routines
                            by Scott Patterson
    */
    /*------------------------------------------------------------------*/

//#include "win_port.h"


//#define BLOCK_SOUND_WESS
//#ifndef BLOCK_SOUND_WESS


#include "kernel.h"
#include <libspu.h>
#include <string.h>
#include "wessarc.h"
#include "wessapi.h"
#include "wessseq.h"

/*extern long EnableEvent(unsigned long event);
extern long DisableEvent(unsigned long event);
extern long OpenEvent(unsigned long desc,long spec,long mode,long (*func)(void));
extern long CloseEvent(unsigned long event);
extern long SetRCnt(unsigned long spec,unsigned long target,long mode);
extern long StartRCnt(unsigned long spec);*/

//#include "psxspu.h"
#include "psxcd.h"

void (**CmdFuncArr[10])(track_status *) = {
    DrvFunctions,
    drv_cmds,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions
    };

short         GetIntsPerSec(void);
unsigned long CalcPartsPerInt(short ips,short ppq,short qpm);
static long WessInterruptHandler(void);
extern void PSX_ReapSfx(unsigned long age);
extern void SeqEngine(void);

/* Movido a WESSBASE.C
int SeqOn = 0;
unsigned long millicount = 0;
unsigned long millisum = 0;

int WessTimerActive = 0;

int                     T2counter = 0;
*/
extern volatile int SeqOn;
extern volatile long millisum;
extern volatile int WessTimerActive;
extern volatile int T2counter;

static unsigned long    EV2 = 0;                /* interrupt event var */

// module loader stuff

static Wess_File_IO_Struct module_fileref;//8007EE20
static unsigned char *module_memory;
static unsigned long module_memory_size;
static unsigned long module_memory_pos;

// data loader stuff
//static unsigned long addr;
//static unsigned long size;

//#define MALLOC_MAX 1
//char spu_malloc_rec [SPU_MALLOC_RECSIZ * (MALLOC_MAX + 1)];

static Wess_Data_IO_Struct data_fileref;//8007EE48

// sram transfer stuff

#define CHUNKSIZE 2048
extern char chunk[CHUNKSIZE];//80096394
extern char chunk2[CHUNKSIZE];//80096BA4

#define SRAM_TO_ALLOC 520192

// while I thought end of sram should be 524288, it seems to be 520192
//#define END_O_SRAM 520192
extern unsigned long end_of_sram;

void wess_low_level_init(void)//80043868
{
    psxspu_init();
}

void wess_low_level_exit(void)//80043888
{
    return;
}

void *wess_malloc(int size)//80043890
{
    return 0;
}

void wess_free(char *mem)//80043898
{
    return;
}

short GetIntsPerSec(void)//800438A0
{
    return(120);
}

unsigned long CalcPartsPerInt(short ips,short ppq,short qpm)//800438A8
{
    unsigned long ppi;
	ppi = ((((unsigned long)qpm*0x10000)+((unsigned long)ips*30)+30)/((unsigned long)ips*60))*(unsigned long)ppq;
	return(ppi);
}

static volatile int wess_irq_busy;

void WessRunEngineOnce(void)
{
	EnterCriticalSection();
	if (SeqOn && !wess_irq_busy) {
		wess_irq_busy = 1;
		SeqEngine();
		wess_irq_busy = 0;
	}
	ExitCriticalSection();
}

static long WessInterruptHandler(void)//800438F8
{
    unsigned int millitemp;

    //printf("millicount %d\n",millicount);
    //printf("SeqOn %d\n",SeqOn);
    //printf("WessTimerActive %d\n",WessTimerActive);

    if (wess_irq_busy)
        return (0);
    wess_irq_busy = 1;

    millitemp = millisum + 0x85555;
    T2counter += 1;
    millisum = millitemp & 0xffff;
    millicount += (millitemp >> 0x10);

    psxspu_fadeengine();
    PSX_ReapSfx(3000);
    if (SeqOn)
    {
        //printf("SeqOn %d\n",SeqOn);
        SeqEngine();
    }
    wess_irq_busy = 0;
    return (0);
}

void init_WessTimer(void)//8004398C
{
    //printf("init_WessTimer\n");

    SeqOn = 0;
	EnterCriticalSection();

	EV2 = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, WessInterruptHandler);
	EnableEvent(EV2);
	SetRCnt(RCntCNT2, 34722, RCntMdINTR);
	StartRCnt(RCntCNT2);
	/*
	 * Timer 2 clock bits are 8-9. Nocash sources 1 and 3 are system clock/8.
	 * DuckStation (this build) treats only bit 9 as clock/8, so source 1 runs
	 * at the full system clock and the sequencer is about 8x too fast.
	 * Source 3 (both bits) is clock/8 on both.
	 */
	*(volatile unsigned short *)0x1F801124 =
		(*(volatile unsigned short *)0x1F801124 & (unsigned short)~0x0300) | 0x0300;

	WessTimerActive = 1;

	ExitCriticalSection();
}

void exit_WessTimer(void)//80043A18
{
    psxspu_set_master_vol(psxspu_get_master_vol());
	psxspu_set_cd_vol(psxspu_get_cd_vol());

	EnterCriticalSection();

    WessTimerActive = 0;

    DisableEvent(EV2);
    CloseEvent(EV2);

    ExitCriticalSection();
}


//static SpuReverbAttr rev_attr;
int Wess_init_for_LoadFileData(char *filename)//80043A88
{
    /*SpuInitMalloc (MALLOC_MAX, spu_malloc_rec);
    rev_attr.mask = SPU_REV_MODE;
    rev_attr.mode = SPU_REV_MODE_OFF;
    SpuSetReverbModeParam (&rev_attr);
    SpuSetReverbVoice (SpuOff, SPU_ALLCH);
    SpuReserveReverbWorkArea(SpuOff);
    SpuSetReverb (SpuOff);
    SpuMalloc (520192);
    SpuSetTransMode (SpuTransferByDMA);*/
    return(1);
}

Wess_File_IO_Struct *module_open(char *filename)//80043A90
{
    Wess_File_IO_Struct *fp;

    if (module_memory)
    {
        module_memory_pos = 0;
        return(&module_fileref);
    }

    fp = psxcd_open(filename);

    module_fileref = *fp;

    return(&module_fileref);
}

int module_read(void *destptr,int readbytes,Wess_File_IO_Struct *fileptr)//80043B04
{
    if (module_memory)
    {
        if ((unsigned long)readbytes > module_memory_size - module_memory_pos)
            return(0);
        memcpy(destptr, module_memory + module_memory_pos, readbytes);
        module_memory_pos += readbytes;
        return(readbytes);
    }
    return(psxcd_read(destptr,readbytes,fileptr));
}

int module_seek(Wess_File_IO_Struct *fileptr,int seekpos,int seekmode)//80043B24
{
    long newpos;

    if (module_memory)
    {
        if (seekmode == PSXCD_SEEK_SET)
            newpos = seekpos;
        else if (seekmode == PSXCD_SEEK_CUR)
            newpos = module_memory_pos + seekpos;
        else
            newpos = module_memory_size + seekpos;
        if (newpos < 0 || (unsigned long)newpos > module_memory_size)
            return(-1);
        module_memory_pos = (unsigned long)newpos;
        return(0);
    }
    return(psxcd_seek(fileptr,seekpos,seekmode));
}

unsigned long module_tell(Wess_File_IO_Struct *fileptr)//80043B44
{
    if (module_memory)
        return(module_memory_pos);
    return(psxcd_tell(fileptr));
}

void module_close(Wess_File_IO_Struct *fileptr)//80043B64
{
    if (!module_memory)
        psxcd_close(fileptr);
}

void module_set_memory_source(void *data, unsigned long size)
{
    module_memory = data;
    module_memory_size = size;
    module_memory_pos = 0;
}

void module_clear_memory_source(void)
{
    module_memory = 0;
    module_memory_size = 0;
    module_memory_pos = 0;
}

int get_num_Wess_Sound_Drivers(int **settings_tag_lists)//80043B84
{
    return(1);
}

Wess_Data_IO_Struct *data_open(char *filename)
{
    Wess_File_IO_Struct *fp;

    fp = psxcd_open(filename);

    data_fileref = *fp;

    return(&data_fileref);
}

extern int ReadChunk1;//*L8007EE70
extern int ReadChunk2;//*L8007EE1C

void data_read_chunk(Wess_Data_IO_Struct *fileptr, int totaltogo, unsigned long spuptr)//80043C00
{
    if (ReadChunk1)//Read to chunk
	{
		psxcd_read(chunk, totaltogo, fileptr);
		SpuSetTransferStartAddr(spuptr);
		SpuWrite(chunk, totaltogo);
		ReadChunk2 = 1;
		ReadChunk1 = 0;
	}
	else
	{
		if (ReadChunk2)//Read to chunk2
		{
			psxcd_read(chunk2, totaltogo, fileptr);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
			SpuSetTransferStartAddr(spuptr);
			SpuWrite(chunk2, totaltogo);
			ReadChunk2 = 0;
		}
		else//Read to chunk
		{
			psxcd_read(chunk, totaltogo, fileptr);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
			SpuSetTransferStartAddr(spuptr);
			SpuWrite(chunk, totaltogo);
			ReadChunk2 = 1;
		}
	}
}

int data_read(Wess_Data_IO_Struct *fileptr,void *destptr,int readbytes,int filepos)//80043D1C
{
    int totaltogo;
	unsigned long spuptr;

	totaltogo = readbytes;
	spuptr = (unsigned long)destptr;

	if ((end_of_sram - spuptr) < readbytes)
	{
		return(0);
	}

	psxcd_seek(fileptr, filepos, PSXCD_SEEK_SET);

	ReadChunk1 = 1;
	while (CHUNKSIZE <= totaltogo)
	{
		data_read_chunk(fileptr, CHUNKSIZE, spuptr);

		totaltogo -= CHUNKSIZE;
		spuptr += CHUNKSIZE;
	}

	if (totaltogo)
	{
		data_read_chunk(fileptr, totaltogo, spuptr);
	}

	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

	return(readbytes);
}

void data_close(Wess_Data_IO_Struct *fileptr)//80043DE8
{
    psxcd_close(fileptr);
}
