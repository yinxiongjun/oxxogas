/****************************************************************************
NAME
	Menu.h 

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	pyming		2011.08.01		- add precompile process
****************************************************************************/

#ifndef _TXN_MENU_H
#define _TXN_MENU_H

#include "posapi.h"
#include "posappapi.h"
#include "emvapi.h"
#include "emvtranproc.h"

uint8_t SelectEcFunc(void);         
uint8_t SelectInstalFunc(void);        
uint8_t TransEcSale(int iTransType);       
uint8_t TransEcTopup(int iTransType);      
uint8_t TransEcVoidTopup(int iTransType);  
uint8_t TransEcBalance(void);              
uint8_t TransEcViewRecord(void);           
uint8_t GetEcElementData(uint8_t *tag, uint8_t *len, uint8_t *val);  
uint8_t TransEcRefund(void);               

uint8_t TransInstallment(void);              
uint8_t TransVoidInstal(void);           
#endif

// end of file


