#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <psxapi.h>
#include <psxcd.h>
#include <psxetc.h>
#include <psxpad.h>
#include <psxspu.h>
#include <libgpu.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define OT_LEN 16
#define PACKET_LEN 2048

typedef struct {
	DISPENV disp;
	DRAWENV draw;
	uint32_t ot[OT_LEN];
	uint8_t packets[PACKET_LEN];
} Frame;

static Frame frames[2];
static int active;
static uint8_t pad_buffer[2][34];
static uint32_t guards[4] = {
	0x13579bdf, 0x2468ace0, 0x55aa55aa, 0xaa55aa55
};
static uint32_t upload_pixels[2048];

static void fail(const char *name) {
	printf("SMOKE FAIL %s\n", name);
	for (;;)
		VSync(0);
}

static void check_guards(void) {
	if (
		guards[0] != 0x13579bdf ||
		guards[1] != 0x2468ace0 ||
		guards[2] != 0x55aa55aa ||
		guards[3] != 0xaa55aa55
	)
		fail("memory-guard");
}

static void init_video(void) {
	ResetGraph(0);

	SetDefDispEnv(&frames[0].disp, 0, 0, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&frames[0].draw, 0, SCREEN_H, SCREEN_W, SCREEN_H);
	SetDefDispEnv(&frames[1].disp, 0, SCREEN_H, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&frames[1].draw, 0, 0, SCREEN_W, SCREEN_H);

	for (int i = 0; i < 2; i++) {
		setRGB0(&frames[i].draw, i ? 24 : 0, 0, i ? 0 : 24);
		frames[i].draw.isbg = 1;
		frames[i].draw.dtd = 1;
		ClearOTagR(frames[i].ot, OT_LEN);
	}

	active = 0;
	PutDrawEnv(&frames[0].draw);
	PutDispEnv(&frames[0].disp);
	SetDispMask(1);
	printf("SMOKE PASS video-init\n");
}

static void test_odd_upload(void) {
	/* Nine 16-bit pixels require five DMA words. The unused high halfword of
	   the final word is intentional and catches truncating upload wrappers. */
	static uint32_t pixels[5] = {
		0x03e0001f, 0x7c007fff, 0x42100421, 0x56b52d6b, 0x00007fff
	};
	RECT rect = { 960, 240, 3, 3 };

	LoadImage(&rect, pixels);
	DrawSync(0);
	printf("SMOKE PASS gpu-odd-upload\n");
}

static void test_upload_matrix(void) {
	static const RECT shapes[] = {
		{ 640, 0, 32, 128 }, /* wall/sky: 2048 words, DMA */
		{ 672, 0, 32, 64 },  /* flat: 1024 words, DMA */
		{ 704, 0, 256, 1 },  /* palette: 128 words, DMA */
		{ 320, 368, 18, 56 } /* dynamic texture: 504 words, CPU fallback */
	};

	for (unsigned int i = 0; i < sizeof(upload_pixels) / sizeof(upload_pixels[0]); i++)
		upload_pixels[i] = 0x4210001f ^ i;

	for (unsigned int i = 0; i < sizeof(shapes) / sizeof(shapes[0]); i++) {
		LoadImage(&shapes[i], upload_pixels);
		DrawSync(0);
		check_guards();
	}
	printf("SMOKE PASS gpu-upload-matrix\n");
}

static void test_pad(void) {
	memset(pad_buffer, 0, sizeof(pad_buffer));
	InitPAD(pad_buffer[0], sizeof(pad_buffer[0]),
		pad_buffer[1], sizeof(pad_buffer[1]));
	StartPAD();
	ChangeClearPAD(0);
	printf("SMOKE PASS pad-init\n");
}

static void test_spu(void) {
	SpuInit();
	SpuSetCommonMasterVolume(0, 0);
	printf("SMOKE PASS spu-init\n");
}

static void test_cd(void) {
	CdlFILE file;
	CdlFILE audio;
	uint8_t mode = CdlModeDA | CdlModeAP;
	CdlLOCINFOP loc_start;
	CdlLOCINFOP loc_end;
	CdlLOC start_abs;
	CdlLOC end_abs;

	if (!CdInit())
		fail("cd-init");
	printf("SMOKE PASS cd-init\n");

	if (!CdSearchFile(&file, "\\PSXDOOM\\ABIN\\PSXDOOM.WAD;1"))
		fail("cd-file-search");
	if (file.size < 2048)
		fail("cd-file-size");

	printf("SMOKE PASS cd-file lba=%d size=%d\n",
		CdPosToInt(&file.pos), file.size);

	if (!CdSearchFile(&audio, "\\PSXDOOM\\CDAUDIO\\DOOMMAIN.RAW;1"))
		fail("cdda-file-search");
	if (!CdControlB(CdlSetmode, &mode, 0))
		fail("cdda-mode");
	if (!CdControlB(CdlSetloc, &audio.pos, 0))
		fail("cdda-location");
	if (!CdControl(CdlPlay, 0, 0))
		fail("cdda-play");
	if (!CdControlB(CdlGetlocP, 0, (uint8_t *)&loc_start))
		fail("cdda-loc-start");
	for (int i = 0; i < 120; i++)
		VSync(0);
	if (!CdControlB(CdlGetlocP, 0, (uint8_t *)&loc_end))
		fail("cdda-loc-end");
	start_abs = (CdlLOC){
		loc_start.minute, loc_start.second, loc_start.sector, 0
	};
	end_abs = (CdlLOC){
		loc_end.minute, loc_end.second, loc_end.sector, 0
	};
	if (CdPosToInt(&end_abs) <= CdPosToInt(&start_abs))
		fail("cdda-progress");
	CdControlB(CdlPause, 0, 0);
	printf("SMOKE PASS cdda-play lba=%d\n", CdPosToInt(&audio.pos));
}

static void draw_frame(uint32_t counter) {
	Frame *frame = &frames[active];
	TILE *tile = (TILE *)frames[active].packets;

	setTile(tile);
	setXY0(tile, (counter >> 1) % (SCREEN_W - 32), 80);
	setWH(tile, 32, 32);
	setRGB0(tile, 255, 255, 0);
	addPrim(&frame->ot[0], tile);

	DrawSync(0);
	VSync(0);
	PutDispEnv(&frame->disp);
	DrawOTagEnv(&frame->ot[OT_LEN - 1], &frame->draw);

	active ^= 1;
	ClearOTagR(frames[active].ot, OT_LEN);
}

int main(void) {
	uint32_t counter = 0;

	printf("SMOKE START\n");
	init_video();
	test_odd_upload();
	test_upload_matrix();
	test_pad();
	test_spu();
	test_cd();
	printf("SMOKE READY\n");

	for (;;) {
		check_guards();
		draw_frame(counter++);
		if (counter == 600)
			printf("SMOKE PASS sustained-frames\n");
	}
}
