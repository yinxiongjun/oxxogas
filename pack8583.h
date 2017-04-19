
/****************************************************************************
NAME
    pack8583.h
*****************************************************************************/

#ifndef _NEWPOS_PACK8583_H
#define _NEWPOS_PACK8583_H

#include "posapi.h"

#define  ERR_UNPACK_LEN     -1000         

#define Attr_a			0
#define Attr_b			1
#define Attr_n			2
#define Attr_z			3
#define Attr_UnUsed		4
#define Attr_Over		5

#define Attr_var1		0
#define Attr_var2		1
#define Attr_fix		2

typedef struct _tagFIELD_ATTR{
   uint8_t	eElementAttr;
   uint8_t	eLengthAttr;
   uint16_t	uiLength;
}FIELD_ATTR;

#ifdef __cplusplus
extern "C" {
#endif 

int iPack8583(FIELD_ATTR *pMsgAttr, FIELD_ATTR *pDataAttr, void *pSt8583, uint8_t *pusOut, uint16_t *puiOutLen);

int iUnPack8583(FIELD_ATTR *pMsgAttr, FIELD_ATTR *pDataAttr, uint8_t *pusIn, uint16_t uiInLen, void *pSt8583);

#ifdef __cplusplus
}
#endif  

#endif 

// end of file
