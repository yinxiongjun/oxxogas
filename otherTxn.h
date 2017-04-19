/****************************************************************************
NAME
	otherTxn.h - 其它交易公共函数

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	pyming		2011.08.02		- add precompile process
****************************************************************************/

#ifndef _OTHER_TXN_H
#define _OTHER_TXN_H

#include "posapi.h"
#include "posappapi.h"
#include "emvapi.h"
#include "emvtranproc.h"

uint8_t TransElecSignature(void); 
void    AfterTransBatch(void);



#endif

// end of file


