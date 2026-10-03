/*
 * Native PSn00bSDK STR playback for the PSX Doom intro.
 * Derived from PSn00bSDK's MPL-licensed mdec/strvideo example.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <psxapi.h>
#include <psxcd.h>
#include <psxetc.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxpress.h>
#include <psxspu.h>

#define INTRO_XRES 320
#define INTRO_YRES 240
#define INTRO_SLICE_WIDTH 16

typedef struct {
	DISPENV disp;
	DRAWENV draw;
} IntroFramebuffer;

typedef struct {
	IntroFramebuffer buffers[2];
	int active;
} IntroRenderContext;

typedef struct {
	uint16_t magic;
	uint16_t type;
	uint16_t sector_id;
	uint16_t sector_count;
	uint32_t frame_id;
	uint32_t bs_length;
	uint16_t width;
	uint16_t height;
	uint8_t bs_header[8];
	uint32_t reserved;
} IntroSectorHeader;

typedef struct {
	uint16_t width;
	uint16_t height;
	uint32_t bs_data[0x2000];
	uint32_t mdec_data[0x8000];
} IntroFrame;

typedef struct {
	IntroFrame frames[2];
	uint32_t slices[2][INTRO_SLICE_WIDTH * INTRO_YRES / 2];
	int frame_id;
	int sector_count;
	int video_sectors_read;
	int video_sector_limit;
	RECT slice_pos;
	int frame_width;
	volatile int8_t sector_pending;
	volatile int8_t frame_ready;
	volatile int8_t final_frame;
	volatile int8_t current_frame;
	volatile int8_t current_slice;
} IntroStreamContext;

typedef struct {
	IntroRenderContext render;
	IntroStreamContext stream;
	IntroSectorHeader sector;
} IntroWorkspace;

static IntroWorkspace *intro_workspace;
#define intro_render (intro_workspace->render)
#define intro_stream (intro_workspace->stream)
#define intro_sector (intro_workspace->sector)

static void intro_init_display(void)
{
	IntroFramebuffer *buffer;

	ResetGraph(0);
	intro_render.active = 0;

	buffer = &intro_render.buffers[0];
	SetDefDispEnv(&buffer->disp, 0, 0, INTRO_XRES, INTRO_YRES);
	SetDefDrawEnv(&buffer->draw, 0, INTRO_YRES, INTRO_XRES, INTRO_YRES);
	setRGB0(&buffer->draw, 0, 0, 0);
	buffer->draw.isbg = 1;
	buffer->draw.dtd = 1;

	buffer = &intro_render.buffers[1];
	SetDefDispEnv(&buffer->disp, 0, INTRO_YRES, INTRO_XRES, INTRO_YRES);
	SetDefDrawEnv(&buffer->draw, 0, 0, INTRO_XRES, INTRO_YRES);
	setRGB0(&buffer->draw, 0, 0, 0);
	buffer->draw.isbg = 1;
	buffer->draw.dtd = 1;

	PutDrawEnv(&buffer->draw);
}

static void intro_display(void)
{
	IntroFramebuffer *buffer;

	intro_render.active ^= 1;
	DrawSync(0);
	buffer = &intro_render.buffers[intro_render.active];
	PutDrawEnv(&buffer->draw);
	PutDispEnv(&buffer->disp);
	SetDispMask(1);
}

static void intro_read_sector(void)
{
	IntroFrame *frame = &intro_stream.frames[intro_stream.current_frame];

	CdGetSector(&intro_sector, sizeof(intro_sector) / 4);
	if (intro_sector.magic != 0x0160) {
		intro_stream.frame_ready = -1;
		return;
	}
	if (intro_sector.type != 0x8001)
		return;
	if ((int) intro_sector.frame_id < intro_stream.frame_id) {
		intro_stream.frame_ready = -1;
		return;
	}

	if ((int) intro_sector.frame_id > intro_stream.frame_id) {
		if (!intro_stream.sector_count)
			intro_stream.frame_ready = 1;

		intro_stream.frame_id = intro_sector.frame_id;
		intro_stream.sector_count = intro_sector.sector_count;
		intro_stream.current_frame ^= 1;
		frame = &intro_stream.frames[intro_stream.current_frame];
		frame->width = (intro_sector.width + 15) & 0xfff0;
		frame->height = (intro_sector.height + 15) & 0xfff0;
	}

	intro_stream.sector_count--;
	CdGetSector(
		&frame->bs_data[(2016 / 4) * intro_sector.sector_id],
		2016 / 4
	);
	intro_stream.video_sectors_read++;
	if (
		intro_stream.video_sectors_read >= intro_stream.video_sector_limit &&
		!intro_stream.sector_count
	) {
		intro_stream.final_frame = 1;
		intro_stream.frame_ready = 1;
	}
}

static void intro_mdec_dma_handler(void)
{
	if (intro_stream.sector_pending) {
		intro_read_sector();
		intro_stream.sector_pending = 0;
	}

	LoadImage(
		&intro_stream.slice_pos,
		intro_stream.slices[intro_stream.current_slice]
	);
	intro_stream.current_slice ^= 1;
	intro_stream.slice_pos.x += INTRO_SLICE_WIDTH;

	if (intro_stream.slice_pos.x < intro_stream.frame_width) {
		DecDCTout(
			intro_stream.slices[intro_stream.current_slice],
			INTRO_SLICE_WIDTH * intro_stream.slice_pos.h / 2
		);
	}
}

static void intro_cd_handler(CdlIntrResult event, uint8_t *payload)
{
	(void) payload;
	if (event != CdlDataReady)
		return;
	if (DecDCTinSync(1))
		intro_stream.sector_pending = 1;
	else
		intro_read_sector();
}

static IntroFrame *intro_next_frame(void)
{
	if (intro_stream.final_frame == 2)
		return 0;
	while (!intro_stream.frame_ready)
		__asm__ volatile("");
	if (intro_stream.frame_ready < 0)
		return 0;
	intro_stream.frame_ready = 0;
	if (intro_stream.final_frame == 1) {
		intro_stream.final_frame = 2;
		return &intro_stream.frames[intro_stream.current_frame];
	}
	return &intro_stream.frames[intro_stream.current_frame ^ 1];
}

static void intro_start_stream(CdlFILE *file)
{
	uint8_t mode = CdlModeRT | CdlModeSpeed;

	intro_stream.frame_id = -1;
	intro_stream.sector_count = 0;
	intro_stream.video_sectors_read = 0;
	/*
	 * MOVIE.STR is interleaved as seven video sectors followed by one XA
	 * audio sector. XA sectors are consumed by the drive and do not trigger
	 * CdlDataReady, so derive the number of delivered video sectors from the
	 * raw 2336-byte sector count.
	 */
	intro_stream.video_sector_limit =
		(file->size / 2336) - ((file->size / 2336) / 8);
	intro_stream.sector_pending = 0;
	intro_stream.frame_ready = 0;
	intro_stream.final_frame = 0;
	CdSync(0, 0);
	CdControl(CdlSetmode, &mode, 0);
	CdControl(CdlReadS, &file->pos, 0);
	intro_next_frame();
}

static void intro_stop_stream(void)
{
	uint8_t mode = CdlModeSpeed;

	CdControlB(CdlPause, 0, 0);
	DecDCTinSync(0);
	DecDCToutSync(0);
	EnterCriticalSection();
	CdReadyCallback(0);
	DMACallback(1, 0);
	ExitCriticalSection();
	/* Leave XA streaming mode or the following CD-DA tracks stay silent. */
	CdControlB(CdlSetmode, &mode, 0);
	CdControlB(CdlDemute, 0, 0);
	DecDCTReset(1);
	SetDispMask(0);
}

void PlayIntroFMV(void)
{
	CdlFILE file;
	IntroFrame *frame;

	intro_workspace = malloc(sizeof(*intro_workspace));
	if (!intro_workspace)
		return;
	memset(intro_workspace, 0, sizeof(*intro_workspace));

	intro_init_display();
	SpuInit();
	CdInit();
	InitGeom();
	DecDCTReset(0);

	if (!CdSearchFile(&file, "\\PSXDOOM\\ABIN\\MOVIE.STR")) {
		free(intro_workspace);
		intro_workspace = 0;
		return;
	}

	memset(&intro_stream, 0, sizeof(intro_stream));
	EnterCriticalSection();
	DMACallback(1, intro_mdec_dma_handler);
	CdReadyCallback(intro_cd_handler);
	ExitCriticalSection();
	DecDCTvlcCopyTableV3((VLC_TableV3 *) 0x1f800000);
	intro_start_stream(&file);

	intro_display();
	intro_render.buffers[0].draw.isbg = 0;
	intro_render.buffers[1].draw.isbg = 0;

	for (;;) {
		VLC_Context vlc;
		RECT *clip;
		int x_offset;
		int y_offset;

		frame = intro_next_frame();
		if (!frame)
			break;
		if (DecDCTvlcStart(
			&vlc,
			frame->mdec_data,
			sizeof(frame->mdec_data) / 4,
			frame->bs_data
		))
			continue;

		VSync(0);
		DecDCTinSync(0);
		DecDCToutSync(0);
		intro_display();
		DecDCTin(frame->mdec_data, DECDCT_MODE_16BPP);

		clip = &intro_render.buffers[intro_render.active].draw.clip;
		x_offset = (clip->w - frame->width) / 2;
		y_offset = (clip->h - frame->height) / 2;
		intro_stream.slice_pos.x = clip->x + x_offset;
		intro_stream.slice_pos.y = clip->y + y_offset;
		intro_stream.slice_pos.w = INTRO_SLICE_WIDTH;
		intro_stream.slice_pos.h = frame->height;
		intro_stream.frame_width = frame->width;
		DecDCTout(
			intro_stream.slices[intro_stream.current_slice],
			INTRO_SLICE_WIDTH * intro_stream.slice_pos.h / 2
		);
	}

	intro_stop_stream();
	free(intro_workspace);
	intro_workspace = 0;
}
