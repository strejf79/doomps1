#pragma once

#include <psxgpu.h>

#define SetGraphDebug(level) ((void)(level))
#define SetSprt setSprt
#define SetSemiTrans setSemiTrans
#define SetShadeTex setShadeTex
#define SetTexWindow PsyqSetTexWindow
#define GetTPage getTPage
#define GetClut getClut

typedef struct {
	unsigned long tag;
	unsigned long code[2];
} DR_MODE;

DR_MODE *SetDrawMode(DR_MODE *packet, int dfe, int dtd, int tpage, RECT *tw);
DR_TWIN *PsyqSetTexWindow(DR_TWIN *packet, const RECT *tw);
int PsyqLoadImage(const RECT *rect, const unsigned long *data);
int PsyqVSync(int mode);
int PsyqDrawOTag(const unsigned long *tag);

#define LoadImage PsyqLoadImage
#define VSync PsyqVSync
#define DrawOTag PsyqDrawOTag
