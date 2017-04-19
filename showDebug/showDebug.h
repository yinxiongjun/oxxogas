/*******************************************************************************************
*  version: V1.0.2
*******************************************************************************************/

#ifndef _SHOWDEBUG_H
#define _SHOWDEBUG_H

//fortest
//#define __DEBUG
#define  _SendToComm
//#define _ShowOnScreen
//#define _Debug_Save_Log
//#define _Debug_Prn
//#define __DEBUG_SN
//#define __DEBUG_REVERSE
//#define __DEBUG_KEY

#define _DEBUG_STR 0x01
#define _DEBUG_HEX 0x02

//≤‚ ‘ π”√S/N∫≈
#define SNTESTNAME    "82016182"
//#define SNTESTNAME    "82072571"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __DEBUG
#define showDebugMessage(a,b,c,...) subShowDebugMessage(__FILE__, __LINE__, a, b, c, ##__VA_ARGS__)
#else
#define showDebugMessage(a,b,c,...)
#endif

void PrintMsg(uint8_t *szData, uint32_t iLen, uint8_t ucFiled, uint8_t ucFlag);
void subShowDebugMessage(char *szFileName, uint32_t uiLine, uint32_t ulHexLen, const uint8_t *pHexHead, char *FmtBuf, ...);
void ListenComm(int iPort);
void SaveToDebugLog(char *pszIn);
#ifdef __cplusplus
}
#endif

#endif
//end of file
