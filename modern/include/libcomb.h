#pragma once

#define COMB_DSR 0x100
#define COMB_CTS 0x080

void CombSetBPS(int baud);
int CombSioStatus(void);
int CombCTS(void);
void CombCancelRead(void);
void CombResetError(void);
