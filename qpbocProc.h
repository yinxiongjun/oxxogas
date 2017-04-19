/****************************************************************************
NAME
	qpbocProc.h - QPboc

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	pyming		2011.08.01		- add precompile process
****************************************************************************/

#ifndef _QPBOC_PROC_H
#define _QPBOC_PROC_H

#include "posapi.h"
#include "posappapi.h"
#include "emvapi.h"
#include "emvtranproc.h"
#include "qpbocapi.h"
#ifdef CL_LED_SUPPORT
#include "dfb_led.h"
#endif

#if 1
#define	LED_NOT_READY  			     0x0001
#define	LED_IDLE	   			     0x0002
#define	LED_ACTIV_CARD 			     0x0004
#define	LED_TRANS_HANDLING    	     0x0008
//#define	LED_REMOVE_CARD   		     0x0010
#define	LED_ONLINE_HANDLING          0x0020
#define	LED_TRANS_OK                 0x0040
#define	LED_TRANS_FAIL               0x0080
#endif

void ledSetStatus(int status);
void   DisplayQpbocError(int iErrorCode);
void   DisplayQpbocBalance(void);

// int    LoadQpbocParam(void);
// void   LoadDefaultQpbocParam(void);
// int    SaveQpbocParam(void);

uint8_t ProcQpbocTran(void);

uint8_t TransEcQuickSale(void);      

uint8_t  GetAndSetQpbocElement(void); 
void     SaveIccFailureTranData(uint8_t ucIncStanFlag);

//extern struct qpboc_parameters  gstQpbocParamters;

uint8_t SaveIccFailureFile(void);
uint8_t ReadIccFailureFile(uint32_t uiIndex);
uint8_t UpDateIccFailureFile(uint32_t uiIndex);

uint8_t ViewIccFailureLog(void);

int  contactless_poweredon(int ifd);
int  contactless_close(int icc_fd);
int  contactless_exchangeapdu(int icc_fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdata);
int  contactless_open(void);
int  contactless_poll(int icc_fd);
int  contactless_poweroff(int fd);
//void ledSetStatus(int status);
int  qpboc_pre_process(int *iPreErrNo);
void QPBOCErrorBeep(void);




#endif

// end of file
