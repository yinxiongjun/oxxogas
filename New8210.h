#ifndef NEW8210_H
#define NEW8210_H

#ifdef _POS_TYPE_8210

#include <ped.h>
#include <ppp.h>
#include <wnet.h>
#include <iccard.h>
#include <iso14443.h>
#include "seos.h"
#include "postslib.h"
#include <epp.h>
#include "wccom.h"

#define     FILE_EXIST          1
#define     FILE_NOEXIST        2
#define     MEM_OVERFLOW        3
#define     TOO_MANY_FILES      4
#define     INVALID_HANDLE      5
#define     INVALID_MODE        6
#define     FILE_NOT_OPENED     8
#define     FILE_OPENED         9
#define     END_OVERFLOW        10
#define     TOP_OVERFLOW        11
#define     NO_PERMISSION       12
#define     FS_CORRUPT          13

#define     KB_REVDISP              0x00

#define     EMV_PIN_KEY_ID          31      // PIN KEY ID
#define     NET_ERR_USER_CANCEL     1043    //  User press key cancel

#ifndef  MAX
#define MAX(a, b)       ( (a)>=(b) ? (a) : (b) )
#endif

#ifndef  MIN
#define MIN(a, b)       ( (a)<=(b) ? (a) : (b) )
#endif

enum {
	NFC_ERR_NONE				= 0,
	NFC_ERR_TIMEOUT				= ETIME,		/* timeout */
	NFC_ERR_EIO					= EIO,			/* chip error */
	NFC_ERR_EPROTO				= EPROTO,		/* not ISO14443 */
	NFC_ERR_ECOMM				= ECOMM,		/* CRC error */
	NFC_ERR_EBADE				= EBADE,		/* len error*/
	NFC_ERR_EMULTIHOP			= EMULTIHOP,	/* multi card*/
	NFC_ERR_ENODATA 			= ENODATA,		/* not T=CL*/
};

typedef struct {
    char  SSID[100];	 /* WIFI用户名 */
    int   SecMode;	     /* 安全加密模式，参见enum UWLNNETSEC */
} WIFIINFO;

typedef struct {
    char  DHCP;	         /* 是否开启DHCP,0-否，1-是 */
    int   KeyType;	     /* 密码格式，参见enum UWLNKEYTYPE */
    int   SecMode;	     /* 安全加密模式，参见enum UWLNNETSEC */
    char  SSID[100];     /* WIFI用户名 */
    char  Pwd[100];	     /* WIFI密码 */
    char  IP[16];	     /* 本地IP地址 */
    char  Gateway[16];	 /* 本地网关 */
    char  Mask[16];	     /* 本地子网掩码 */
    char  Dns1[16];	     /* DNS */
    char  Dns2[16];	     /* DNS */
} WIFICONFIG;

//extern WIFICONFIG 			gstWificonfig;


#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))


#define  PARITY_NONE        0x00
#define  PED_RET_APPTSSAKERR     2009        //  No TSSAK for this APP in APP
#define  FORMAT_0           PIN_BLOCK_FORMAT_0
#define  TYPE_MACKEY        KEY_TYPE_MACK

INT32 PedWriteMasterKey(UINT32 Mode, UINT16  srcKeyID, UINT16 destKeyID, UINT16 KeyLen, UINT8 *KeyBuf);
INT32 PedWritePinKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 PinKeyID, UINT16 PinKeyLen, UINT8 *PinKey);
INT32 PedWriteMacKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 MacKeyID, UINT16 MacKeyLen, UINT8 *MacKey);
INT32 PedGetPin(UINT16 PinKeyID, UINT8 *ExpectPinLen, UINT8 *CardNo, INT32 Mode, UINT8 *PinBlock);
 int inter_pp_GetPin(char *Title, unsigned char Mode, unsigned char *PAN, unsigned char *PinData,
  unsigned char MinPINLen, unsigned char MaxPINLen, unsigned short TimeOut,
  unsigned short KeyIndex, unsigned short iDesType);
 
INT32 PedGetMac(UINT16 MacKeyID, UINT8 *DataIn, UINT16 InLen, INT32 Mode, UINT8 *MacOut);
#define  PedTDEA            ped_tdea
#define  PedDelKey          ped_delete_key

int32_t PedGetSensitiveTimer(int32_t SensitiveType);
INT32  PedSetPinInputTimeout(int iTimeOut);

void  lcdDrLogo(INT32 X, INT32 Y, UINT8 *pucBitMap);

extern struct _EPP_CONFIGURE_ *glbEpp;
extern int  g_epp_Port_Num;

  
#define KEY_TYPE_FIXED_PIN   0x11
int   EppLight(unsigned long dwMode, unsigned long dwTimeMs);
int   EppDisplayLogo(int X, int Y, const uint8_t *pbyBmpLogoIn);
int   EppDisplayString(int X, int Y, int iMode, const void *str, int iStrLen);
int   EppClearScreen(void);
int   EppSetPinInputTimeout(WORD wTimeout100Ms);
int   EppGetPin(uint32_t byKeyType, uint32_t byKeyIndex, uint32_t byDisplayMode,
            uint32_t byMode, const uint8_t *pbyCardInfo, const uint8_t *pbyLenInfo, uint8_t *pbyPinBlockOut);
int   EppRestoreDefaultIdleLogo(void);
int   EppGetMac(uint32_t byKeyType, uint32_t byKeyIndex, const uint8_t *pbyMacDataIn,
                        WORD wMacLen, uint32_t byMode, uint8_t *pbyMacOut);
int   EppDownloadAppKey(uint32_t byKeyType, uint32_t byMode, uint32_t byKeyIndex,
                        const uint8_t *pbyKeyDataIn, uint32_t byKeyLen, uint32_t byMasterKeyIndex);
int   EppClose(void);
int   EppBeep(unsigned long dwFrequency, unsigned long dwTimeMs);
int   EppClearAKey(uint32_t byKeyType, uint32_t byKeyIndex);
void  lcdDispScreen(INT32 X, INT32 Y, UINT8 *pucBitMap,UINT8 ucFirst,UINT8 *pDispBuf1,UINT8 *pDispBuf2,UINT8 ucMode);
int   GetCurWirelessSocket(void);
int   GetCurTcpSocket(void);
void CheckWireless(void);
uint32_t  DispFailComm(int iReturn, int ierror);
int TcpSendData(int iHandle, uint8_t *psTxdData, uint32_t uiDataLen);
uint32_t DispRecvTime(uint32_t uiTimeOut);
int WirelessTxdNormal(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
int WirelessRxdNormal(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
int TcpTxdNormal(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
int TcpRxdNormal(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
int SetWifiUI(void);
int SetCurComm(void);
void hex_debug(u8 *src, int len);
int  return_manager(void);
int  __attribute__((weak)) wifi_config_ui(int fontsize, int language);//兼容镜像没有wifi_config_ui这个定义
int  __attribute__((weak)) wifi_reassoc(void);

int  __attribute__((weak)) wifi_power_control_force(int is_open);//The definition of wifi_power_control_force compatible image








#endif

#endif
