#include "glbvar.h"
#include "tranfunclist.h"
#include <sys/stat.h>
#include <pthread.h>

#ifdef _POS_TYPE_7110

#include "New7110.h"


int    icc_fd;
struct _EPP_CONFIGURE_ *glbEpp;

unsigned char Bmp_Display[] = "logo.png";
unsigned char Bmp_Prn[] = "logo_p.png";
unsigned char Bmp_passive[] = "passive.png";

pthread_t PreDialTid=-1,InitCommTid=-1;

void CheckPinPad(void)
{
	int       IfRedraw, uiLen=0, iRet;
	char      szBuffer[20];

	IfRedraw = 1;
	while(IfRedraw)
	{
		glbEpp = epp_open("/dev/ttyS4", O_RDWR);
		iRet = epp_get_rand(glbEpp,szBuffer,uiLen);
		if( iRet != 0 )
		{
			lcdClrLine(2,7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
			lcdFlip();
			kbFlush();
			if (KEY_CANCEL == kbGetKey())
			{
				IfRedraw = 0;
				break;
			}
			continue;
		}
		
		IfRedraw = 0;
	}
	epp_light(glbEpp, 0);
	epp_set_pin_input_timeout(glbEpp,60000);
}

int BootInit(int argc, char *argv[])
{
	struct kbdset stdbCfg;
	application_init(&argc, &argv);	
	dfb_create_font("/usr/share/fonts/arialnb.ttf", 32, 0);
	sys_kbd_get(&stdbCfg);
	stdbCfg.beep_enable = 1;
	stdbCfg.beep_freq = 2800;
	stdbCfg.beep_time = 20;
	stdbCfg.backlight = 60;
	stdbCfg.feed_enable = 1;
	sys_kbd_set(&stdbCfg);
	return 0;
}

INT32 PedWriteMasterKey(UINT32 Mode, UINT16  srcKeyID, UINT16 destKeyID, UINT16 KeyLen, UINT8 *KeyBuf)
{
	/*EppAppKey_t  AppTempKey;
	int          iRet;
	
	AppTempKey.KeyType = KEY_TYPE_MASTER;
	AppTempKey.MasterKeyIndex = srcKeyID;
	AppTempKey.KeyIndex = destKeyID;
	AppTempKey.Mode = Mode;
	AppTempKey.KeyLen = KeyLen;
	memcpy(AppTempKey.KeyData, KeyBuf, KeyLen);
	iRet = epp_download_appkey(glbEpp, &AppTempKey);
	return iRet;*/
	return 0;
}

INT32  PedWritePinKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 PinKeyID, UINT16 PinKeyLen, UINT8 *PinKey)
{
	EppAppKey_t  AppTempKey;
	int          iRet;
	
	AppTempKey.KeyType = KEY_TYPE_PIN;
	AppTempKey.MasterKeyIndex = MasterKeyID;
	AppTempKey.KeyIndex = PinKeyID;
	AppTempKey.Mode = Mode;
	AppTempKey.KeyLen = PinKeyLen;
	memcpy(AppTempKey.KeyData, PinKey, PinKeyLen);
	iRet = epp_download_appkey(glbEpp, &AppTempKey);
	return iRet;
}

INT32  PedWriteMacKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 MacKeyID, UINT16 MacKeyLen, UINT8 *MacKey)
{
	EppAppKey_t  AppTempKey;
	int          iRet;
	
	AppTempKey.KeyType = KEY_TYPE_MAC;
	AppTempKey.MasterKeyIndex = MasterKeyID;
	AppTempKey.KeyIndex = MacKeyID;
	AppTempKey.Mode = Mode;
	AppTempKey.KeyLen = MacKeyLen;
	memcpy(AppTempKey.KeyData, MacKey, MacKeyLen);
	iRet = epp_download_appkey(glbEpp, &AppTempKey);		
	return iRet;
}

INT32  PedTDEA(UINT32 Mode, INT32 KeyType, UINT16  KeyID, UINT32 DataLen, UINT8 *InBuf, UINT8 *OutBuf)
{
	int          iRet;
	
	iRet = epp_get_tdea(glbEpp,Mode,KeyType,KeyID,InBuf,DataLen,OutBuf);
	return iRet;
}

INT32  PedGetMac(UINT16 MacKeyID, UINT8 *DataIn, UINT16 InLen, INT32 Mode, UINT8 *MacOut)
{
	int iRet;
	
	iRet = epp_get_mac(glbEpp, KEY_TYPE_MAC, MacKeyID, Mode, DataIn, InLen, MacOut); 
	return iRet;
}


INT32  PedGetPin(UINT16 PinKeyID, UINT8 *ExpectPinLen, UINT8 *CardNo, INT32 Mode, UINT8 *PinBlock)
{
	return 0;
}

int32_t PedGetSensitiveTimer(int32_t SensitiveType)
{
	return 0;
}

INT32  PedDelKey(INT32 KeyType, UINT16  KeyID)
{	
	return  epp_clear_key(glbEpp, KeyType,KeyID);		
}

int iccCardExchangeAPDU(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdata)
{
	return iccard_exchange_apdu(fd, sendlen, senddata, recvlen, recvdata);
}

void PrnBigFontChnData(void)
{
	int  iRet;
	
	iRet = prnSetFont("/usr/share/fonts/PosSong-24.ttf", "GB18030", 0, 24, SET_PRIMARY_FONT);
	if ( iRet!=OK )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST ");
		lcdFlip();
		kbGetKey();
		return ;
	}	
	return ;	
}

void PrnBlackEngData(void)
{
	int  iRet;
	
	iRet = prnSetFont("/usr/share/fonts/arialnb.ttf", FONT_CHARSET_ASCII, 0, 32, SET_PRIMARY_FONT);
	if ( iRet!=OK )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST ");
		lcdFlip();
		kbGetKey();
		return ;
	}	
	return ;	
}

void PrnSmallFontData(void)
{
	int  iRet;
	
	iRet = prnSetFont("/usr/share/fonts/PosSong-16.ttf", "GB18030", 0, 16, SET_PRIMARY_FONT);
	if ( iRet!=OK )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
		lcdFlip();
		kbGetKey();
		return ;
	}	
	return ;	
}

void PrnSmallEngData(void)
{
	PrnSmallFontData();
}

void lcdDrLogo(INT32 X, INT32 Y, UINT8 *pucBitMap)
{
	lcdDrawPicture(X,Y,48,32, (char *)pucBitMap);
}

void lcdDispScreen(INT32 X, INT32 Y, UINT8 *pucBitMap,UINT8 ucFirst,UINT8 *pDispBuf1,UINT8 *pDispBuf2,UINT8 ucMode)
{	
	if (ucFirst == 0)
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf1);
		if (strlen((char *)pDispBuf2))
		{
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf2);
		}		
		lcdFlip();
	}
}


int EppLight(DWORD dwMode, DWORD dwTimeMs)
{
	if (dwMode==2)
	{
		return epp_light(glbEpp,-1);//Backlight always on		
	}
	else if (dwMode==3)
	{
		return epp_light(glbEpp,0);//Backlight always off		
	}
	else
	{
		return epp_light(glbEpp,dwTimeMs);
	}
}

int EppDisplayLogo(int X, int Y, const uint8_t *pbyBmpLogoIn)
{
	return epp_display_logo(glbEpp,X,Y,pbyBmpLogoIn);
}

int EppDisplayString(int X, int Y, int iMode, const void *str, int iStrLen)
{
	return epp_display_string(glbEpp,X,Y,iMode,str,iStrLen);
}

int EppClearScreen(void)
{
	return epp_clear_screen(glbEpp);
}

int EppSetPinInputTimeout(WORD wTimeout100Ms)
{
	return epp_set_pin_input_timeout(glbEpp,wTimeout100Ms);
}

int EppGetPin(uint32_t byKeyType, uint32_t byKeyIndex, uint32_t byDisplayMode,
            uint32_t byMode, const uint8_t *pbyCardInfo, const uint8_t *pbyLenInfo, uint8_t *pbyPinBlockOut)
{
	return epp_get_pin(glbEpp,byKeyType,byKeyIndex,byDisplayMode,byMode,pbyCardInfo,pbyLenInfo,pbyPinBlockOut);
}

int EppRestoreDefaultIdleLogo(void)
{
	return epp_resume_default_idle_logo(glbEpp);
}

int EppGetMac(uint32_t byKeyType, uint32_t byKeyIndex, const uint8_t *pbyMacDataIn,
                        WORD wMacLen, uint32_t byMode, uint8_t *pbyMacOut)
{
	return epp_get_mac(glbEpp,byKeyType,byKeyIndex,byMode,pbyMacDataIn,wMacLen,pbyMacOut);
}

int EppDownloadAppKey(uint32_t byKeyType, uint32_t byMode, uint32_t byKeyIndex,
    const uint8_t *pbyKeyDataIn, uint32_t byKeyLen, uint32_t byMasterKeyIndex)
{
	EppAppKey_t  AppTempKey;
	
	AppTempKey.KeyType = byKeyType;
	AppTempKey.MasterKeyIndex = byMasterKeyIndex;
	AppTempKey.KeyIndex = byKeyIndex;
	AppTempKey.Mode = byMode;
	AppTempKey.KeyLen = byKeyLen;
	memcpy(AppTempKey.KeyData, pbyKeyDataIn, byKeyLen);
	return epp_download_appkey(glbEpp,&AppTempKey);
}

int EppClose(void)
{
	return epp_close(glbEpp);
}

int EppBeep(DWORD dwFrequency, DWORD dwTimeMs)
{
	return epp_beep(glbEpp,dwFrequency,dwTimeMs);
}

int EppClearAKey(uint32_t byKeyType, uint32_t byKeyIndex)
{
	return epp_clear_key(glbEpp,byKeyType,byKeyIndex);
}


int   InitWiFiModule(uint8_t bInitModule)
{
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}
	return 0;
}

int   GetWiFiStatus(void)
{
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}

	return 0;
}

int    SetWIFIParam(int iMode)
{
	int		iRet;
	
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "SETUP WIFI");	
	iRet = SetTcpIpParam(iMode);
	if (iRet!=0)
	{
		return iRet;
	}
	
	return 0;
}

int CheckConnectPinPad(uint8_t ucFlag)
{
	return 0;
}

void IfSupportPinpad(void)
{
	stPosParam.ucSupPinpad = PARAM_OPEN;
}

void   DrawRect(uint32_t uiBegRow, uint32_t uiBegCol, uint32_t uiEndRow, uint32_t uiEndCol)
{
	UINT32	uiPointx, uiPointy; 
	
	if (uiBegRow>=127)
	{
		uiEndRow = 127;
	}
	if (uiEndRow>=127)
	{
		uiEndRow = 127;
	}
	if (uiEndRow<=uiBegRow)
	{
		uiEndRow = uiBegRow;
	}
	
	if (uiBegCol>=63)
	{
		uiBegCol = 63;
	}
	if (uiEndCol>=63)
	{
		uiEndCol = 63;
	}
	if (uiEndCol<=uiBegCol)
	{
		uiEndCol = uiBegCol;
	}
	
	for (uiPointx=uiBegRow; uiPointx<=uiEndRow;uiPointx++)
	{
		lcdStipple(uiPointx, uiBegCol, ON);
		lcdStipple(uiPointx, uiEndCol, ON);
	}
	
	for (uiPointy=uiBegCol; uiPointy<=uiEndCol; uiPointy++)
	{
		lcdStipple(uiBegRow, uiPointy, ON);
		lcdStipple(uiEndRow, uiPointy, ON);
	}
	
	return;
}

void lcdClrRect(int x, int y, int width, int height)
{
	
}

void newprnLogo(int x, int y, int w, int h, uint8_t *filename)
{
	pntDrawPictureExt(x, y, w, h, (char *)filename);
}

int start_timer(time_t sec, suseconds_t usec)
{
	return 0;
}
int stop_timer(void)
{
	return 0;
}

int sysSetbacklighttime(int keep)
{
	return 0;
}
int sysGetbacklighttime(int *keep)
{
	return 0;
}

int     CheckWirelessAttached(uint32_t uiTimeOut)
{
	return 0;
}

int ExitApplication(void)
{
	int iRet;

	if( CheckParamPass() != 1)
	{
		return NO_DISP;
	}
	lcdClrLine(2,7);
	lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"EXIT APP?");
	lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY, "1.YES   0.NO");
	lcdFlip();
	iRet = kbGetKeyMs(30000);
	if( iRet==KEY1 )
	{
		exit(0);
	}
	return 0;
}

int wnet_power_down(void)
{
	return 0;
}

void   AdjustMenuContext(const char *pszInStr, char *pszOutStr)
{
}

void SetSupportPinpad(void)
{
}

void AppMagOpen(void)
{
}

void AppMagClose(void)
{
}

void GetBaseStationInfo(uint8_t *szStationInfo)
{
}


#endif
//end of line







