#ifndef NEW7110_H
#define NEW7110_H

#ifdef _POS_TYPE_7110

#include <seos.h>
#include <iccard.h>
#include <epp.h>
#include <iso14443.h>
#include  "postslib.h"

#define PPP_ALG_PAP					0x01
#define PPP_ALG_CHAP				0x02

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

//  return value
#define     PED_RET_OK              0x00        //  PED OK
#define     PED_RET_LOCKED          0x01        //  PED Locked
#define     PED_RET_ERROR           0x02        //  The others error
#define     PED_RET_COMMERR         0x03        //  Communicate with PED failed
#define     PED_RET_NEEDAUTH        0x04        //  Need auth before use sensitive service or expired
#define     PED_RET_AUTHERR         0x05        //  PED auth failed
#define     PED_RET_APPTSSAKERR     0x06        //  No TSSAK for this APP in APP
#define     PED_RET_PEDTSSAKERR     0x07        //  No TSSAK for this APP in PED
#define     PED_RET_KEYINDEXERR     0x08        //  The index of key incorrect
#define     PED_RET_NOKEY           0x09        //  No designated key in PED
#define     PED_RET_KEYFULL         0x0A        //  Key space is full
#define     PED_RET_OTHERAPPKEY     0x0B        //  The designated key is not belong to this APP
#define     PED_RET_KEYLENERR       0x0C        //  The key length error
#define     PED_RET_NOPIN           0x0D        //  Card holder press ENTER directly when enter PIN(no PIN)
#define     PED_RET_CANCEL          0x0E        //  Card holder press CANCEL to quit enter PIN
#define     PED_RET_TIMEOUT         0x0F        //  Timeout
#define     PED_RET_NEEDWAIT        0x10        //  Two frequent between 2 sensitive API
#define     PED_RET_KEYOVERFLOW     0x11        //  DUKPT KEY overflow
#define     PED_RET_NOCARD          0x12        //  No ICC
#define     PED_RET_ICCNOTPWRUP     0x13        //  ICC no power up
#define     PED_RET_PARITYERR       0x14        //  The parity incorrect
#define     PED_RET_UNSUPPORTED     0xFF        //  can not support

#define     MAC_MODE_1              0x00        //  MAC method 1, TDES-TDES...TDES
#define     MAC_MODE_2              0x01        //  MAC method 2, XOR...XOR...TDES
#define     MAC_MODE_EMV            0x02        //  MAC for EMV EMV, DES-DES...TDES
#define     MAC_MODE_CUP            0x03        //  MAC for CUP, XOR-XOR...TDES(left)-XOR-TDES...
#define     MAC_MODE_DUKPT          0x04        //  MAC for DUKPT,Expand, XOR-XOR...TDES(left)-XOR-TDES...

#define     PARITY_NONE             0x00        //  No parity
#define     PARITY_ODD              0x10        //  Odd parity
#define     PARITY_EVEN             0x20        //  Even parity

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

#define     TYPE_MACKEY        KEY_TYPE_MAC
#define     TYPE_PIN_INTERVAL       1           //  TYPE: The interval time between 2 times PIN input
#define     FORMAT_0                0x00        //  PIN BLOCK FORMAT 0


extern struct _EPP_CONFIGURE_ *glbEpp;

INT32 PedWriteMasterKey(UINT32 Mode, UINT16  srcKeyID, UINT16 destKeyID, UINT16 KeyLen, UINT8 *KeyBuf);
INT32 PedWritePinKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 PinKeyID, UINT16 PinKeyLen, UINT8 *PinKey);
INT32 PedWriteMacKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 MacKeyID, UINT16 MacKeyLen, UINT8 *MacKey);
INT32 PedTDEA(UINT32 Mode, INT32 KeyType, UINT16  KeyID, UINT32 DataLen, UINT8 *InBuf, UINT8 *OutBuf);
INT32 PedGetMac(UINT16 MacKeyID, UINT8 *DataIn, UINT16 InLen, INT32 Mode, UINT8 *MacOut);
INT32  PedGetPin(UINT16 PinKeyID, UINT8 *ExpectPinLen, UINT8 *CardNo, INT32 Mode, UINT8 *PinBlock);
int32_t PedGetSensitiveTimer(int32_t SensitiveType);
INT32  PedDelKey(INT32 KeyType, UINT16  KeyID);

void  lcdDrLogo(INT32 X, INT32 Y, UINT8 *pucBitMap);

int   EppLight(DWORD dwMode, DWORD dwTimeMs);
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
int   EppBeep(DWORD dwFrequency, DWORD dwTimeMs);
int   EppClearAKey(uint32_t byKeyType, uint32_t byKeyIndex);
void  lcdClrRect(int x, int y, int width, int height);
int   wnet_power_down(void);
void  lcdDispScreen(INT32 X, INT32 Y, UINT8 *pucBitMap,UINT8 ucFirst,UINT8 *pDispBuf1,UINT8 *pDispBuf2,UINT8 ucMode);

#endif

#endif
