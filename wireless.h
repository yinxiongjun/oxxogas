/****************************************************************************
CDMA/GPRS
****************************************************************************/
#ifndef __NEWPOS_WIRELESS_H
#define __NEWPOS_WIRELESS_H

#include "posapi.h"
#include "libsbTms.h"

#define WIRELESS_TIMER_ID 4

#define WIRELESS_TIME_OUT 1	

#define  TRANS_COMMTYPE        0x01
#define  DOWNLAOD_COMMTYPE     0x02

#ifdef __cplusplus
extern "C" {
#endif 

uint8_t SetModemPPPParam(POS_PARAM_STRC_MAIN *pSaveParam);
uint8_t SetWirelessParam(int iMode, uint8_t ucCommType, POS_PARAM_STRC_MAIN *pSaveParam);
int    PreDial(void);
int    CheckPreDial(int iPreDialRet);
int    SetCurCommParam(int iMode);
int    SelectAndSetWIFIAP(void);
int    SetWIFIParam(int iMode);
int    SetWiFiLoaclIPCfg(void);
int    SetTcpIpParam(int iMode);
int    SetRS232Param(void);
int    SetAcqTransTelNo(int iMode);
int    SetModemParam(void);
//int    SetDownModem(void);

void   NacTxd(uint8_t *psTxdData, uint32_t uiDataLen);
int    SetWnetMode(void);
int SetManageTcpIpParam(POS_PARAM_STRC_MAIN *pSaveParam);

#ifdef __cplusplus
}
#endif 

#endif 


