#ifndef NEW8110_H
#define NEW8110_H

#ifdef _POS_TYPE_8110

#include <alloc.h>
#include "font.h"
#include "Epp.h"

#define     PRN_NOBATTERY       	0x10 


#define     TMS_ERROR_NOTASK        E_NOTASK

#define     DISP_FLIP               0
#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef INT32
#define INT32  int
#endif

#ifndef INT8
#define INT8 char
#endif


//void CheckPedStatus(void);
void prnClose(void);
int iccGetSlotFd(int CardSlot);
void lcdFlip(void);
void lcdClrRect(int x, int y, int width, int height);
int jbg2png(const char *jbgfile, const char *pngfile);
int   fileMakeDir(const char *pathname);
int   wnet_power_down(void);
void lcdGetSize(int *width, int *height);
void  lcdDispScreen(INT32 X, INT32 Y, UINT8 *pucBitMap,UINT8 ucFirst,UINT8 *pDispBuf1,UINT8 *pDispBuf2,UINT8 ucMode);



#endif

#endif
