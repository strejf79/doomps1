#pragma once

#include <psxcd.h>

int PsyqCdControl(CdlCommand cmd, const void *param, unsigned char *result);
int PsyqCdGetSector(void *address, int words);
int CdReady(int mode, unsigned char *result);
void CdFlush(void);

#define CdControl PsyqCdControl
#define CdGetSector PsyqCdGetSector
