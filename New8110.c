#include "glbvar.h"
#include "tranfunclist.h"

#ifdef _POS_TYPE_8110

#include "New8110.h"

int    icc_fd = ICC_USERCARD;
INT32 g_epp_Port_Num;
pthread_t PreDialTid=-1,InitCommTid=-1;

unsigned char Bmp_passive[] = "passive.png";

#define DYNAMIC_MEMORY_SIZE		(1024 * 512)

void CheckPinPad(void)
{
	PedSetPinInputTimeout(60000);
	g_epp_Port_Num = EPP_PORT; // PORT_COM1
	if(stPosParam.ucSupPinpad==PARAM_OPEN)
	{
		EppOpen(g_epp_Port_Num);
		EppLight(3,60*1000);
		EppSetPinInputTimeout(600);
	}
}

int BootInit(int argc, char *argv[])
{
	// Memory allocation
	static uint8_t theMemoryPool[DYNAMIC_MEMORY_SIZE] __attribute__((aligned(4)));	
	_HeapInit((ulong)theMemoryPool, sizeof(theMemoryPool));
	return 0;
	
	//CheckPedStatus();
}

/*void CheckPedStatus(void)
{
	tPedConfig  stPedInfo;
	int  iRet;
	
	memset((char*)&stPedInfo,0,sizeof(stPedInfo));
	
	iRet = PedGetConfig(&stPedInfo);
	if (iRet!= 0)
	{
		return;
	}
	
	if (stPedInfo.Status == PED_RET_LOCKED)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE, NULL, " UNLOCK PINPAD  ");
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, " CLEAR ALL KEY? ");
		iRet = kbGetKey();
		if( iRet==KEY_ENTER )
		{
			lcdClrLine(2,7);
			PedRebuild();
		}
	}
}*/

int iccCardExchangeAPDU(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdata)
{
	APDU_SEND apdu_s;
	APDU_RESP apdu_r;
	INT32 Ret=0;	
	if (sendlen <4)
		return -1;
	memset(&apdu_s, 0, sizeof(apdu_s));
	memcpy(&apdu_s.CLA, senddata, 4);
	if (sendlen == 5) //Has Le, no lc and data CLA INS P1 P2 LE
	{
		memcpy(&apdu_s.Le, senddata +4, 1);
		apdu_s.LeFlag = 1;	
	}
	else if (sendlen >5) // CLA INS P1 P2 LC Data (LE)
	{
		memcpy(&apdu_s.Lc, senddata + 4, 1);
		if (apdu_s.Lc)
		{
			memcpy(apdu_s.DataIn, senddata +5, apdu_s.Lc);
		}

		if (apdu_s.Lc + 5 < sendlen) //There maybe has le
		{
			memcpy(&apdu_s.Le, senddata + 5 + apdu_s.Lc, 1);
			apdu_s.LeFlag = 1;		
		}
	}
	
	Ret = iccExchangeAPDU(0,&apdu_s,&apdu_r);
	if(Ret){
		Ret = -1;
	}	
	else
	{
		*recvlen  = apdu_r.LenOut;
		memcpy(recvdata, apdu_r.DataOut, apdu_r.LenOut);
		
		memcpy(recvdata + *recvlen, &apdu_r.SWA, 1);
		*recvlen = *recvlen + 1;
		memcpy(recvdata + *recvlen, &apdu_r.SWB, 1);
		*recvlen = *recvlen + 1;
	}
	return Ret;
}

void PrnBigFontChnData(void)
{
	INT32 iLanguage, iWidth, iRet;
	UINT8 sFontName[40], sFontChar[40];
	
	prnGetCurFont(sFontName, sFontChar, &iLanguage, &iWidth, SET_PRIMARY_FONT);
	
	iRet = prnSetFont(sFontName, sFontChar, 0, 24, SET_PRIMARY_FONT);
	if( iRet==ERROR )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST ");
		lcdFlip();
		kbGetKey();
		return ;
	}
	
	prnGetCurFont(sFontName, sFontChar, &iLanguage, &iWidth, SET_SECONDARY_FONT);
	
	iRet = prnSetFont(FONT_SYSTEM, sFontChar, 0, 24, SET_SECONDARY_FONT);
	if( iRet==ERROR )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
		lcdFlip();
		kbGetKey();
		return ;
	}	
	return ;		
}

void PrnSmallFontData(void)
{
	INT32 iLanguage, iWidth, iRet;
	UINT8 sFontName[40], sFontChar[40];
	
	prnGetCurFont(sFontName, sFontChar, &iLanguage, &iWidth, SET_PRIMARY_FONT);
	
	iRet = prnSetFont(sFontName, sFontChar, 0, 16, SET_PRIMARY_FONT);
	if( iRet==ERROR )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
		lcdFlip();
		kbGetKey();
		return ;
	}
	
	prnGetCurFont(sFontName, sFontChar, &iLanguage, &iWidth, SET_SECONDARY_FONT);	
	iRet = prnSetFont(sFontName, sFontChar, 0, 16, SET_SECONDARY_FONT);
	if( iRet==ERROR )
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
	INT32 iLanguage, iWidth, iRet;
	UINT8 sFontName[40], sFontChar[40];
	
	prnGetCurFont(sFontName, sFontChar, &iLanguage, &iWidth, SET_SECONDARY_FONT);	
	iRet = prnSetFont(sFontName, sFontChar, 0, 16, SET_SECONDARY_FONT);
	if( iRet==ERROR )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
		lcdFlip();
		kbGetKey();
		return ;
	}
	
	return ;	
}

void PrnBlackEngData(void)
{
	INT32 iLanguage, iWidth, iRet;
	UINT8 sFontName[40], sFontChar[40];
	
	prnGetCurFont(sFontName, sFontChar, &iLanguage, &iWidth, SET_SECONDARY_FONT);
	iRet = prnSetFont(FONT_ARIAL, sFontChar, 0, 32, SET_SECONDARY_FONT);
	if( iRet==ERROR )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
		lcdFlip();
		kbGetKey();
		return ;
	}
	
	return ;	
}

void prnClose(void)
{
	return;
}

int iccGetSlotFd(int CardSlot)
{
	return ICC_USERCARD;
}


#define WIFI_COMM_ERR(a)		(ERR_COMM_WIFI_BASE|(a))
int   InitWiFiModule(uint8_t bInitModule)
{
	int       iRet, iRetry, iTemp;
    char      szlocalIP[32];
    WiFiStatus_t    stWiFiStatus;

	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}

	if (bInitModule)
	{
		iRet = WnetInit(20000);
		if( iRet!= NET_OK )
		{		
			if (iRet<0)
			{
				iRet = -iRet;
			}
			return WIFI_COMM_ERR(iRet);
		}
	}

	memset(szlocalIP, 0, sizeof(szlocalIP));
	iRetry = 0;
	while(1)
	{
		while(1)
		{
			iRetry++;
			lcdClrLine(2, 7);	
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PLS WAITING...");
			lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY|DISP_CLRLINE,"CONNECTING..(%d)",iRetry);
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			iRet = WifiGetCurConnect(&stWiFiStatus);
			iTemp = iRet;
			if (iRet != NET_OK || stWiFiStatus.Status != WIFI_STATUS_CONNECTED)
			{
				if (iTemp== NET_OK)
				{
					iTemp = WIFI_COMM_ERR(EWIFI_ONLINELOST);
				}
				if (iTemp<0)
				{
					iTemp = -iTemp;
				}
				lcdClrLine(2,7);	
				PubDispString("FAIL,PRESS KEY",2|DISP_LINE_CENTER);
				PubDispString("FUNC OR MENU",4|DISP_LINE_CENTER);
				PubDispString("SET WIFI CONFIG",6|DISP_LINE_CENTER);
				lcdFlip();
				iRet = PubWaitKey(6);
				if (iRet == KEY_FN || iRet == KEY_MENU)
				{
					iRet = SelectAndSetWIFIAP();
					if (iRetry>=3)
					{
						return  WIFI_COMM_ERR(iTemp);
					}
					sysDelayMs(2000);
					continue;
				}
				else
				{
					return  WIFI_COMM_ERR(iTemp);
				}
			}
			break;
		}

		iRetry = 0;
		while(1)
		{
			iRetry++;
			lcdClrLine(2,7);	
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PLS WAITING...");
			lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY|DISP_CLRLINE,"GET LOCAL IP..(%d)",iRetry);
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			memset(szlocalIP, 0, sizeof(szlocalIP));
			iRet = WifiGetLocalIP(szlocalIP);
			if(iRet != NET_OK)
			{
				iTemp = iRet;
				if (iTemp<0)
				{
					iTemp = -iTemp;
				}
				lcdClrLine(2,7);	
				PubDispString("FAIL,PRESS KEY",2|DISP_LINE_CENTER);
				PubDispString("FUNC OR MENU",4|DISP_LINE_CENTER);
				PubDispString("SET WIFI CONFIG",6|DISP_LINE_CENTER);
				lcdFlip();
				iRet = PubWaitKey(6);
				if (iRet == KEY_FN || iRet == KEY_MENU)
				{
					iRet = SelectAndSetWIFIAP();
					if (iRetry>=3)
					{
						return  WIFI_COMM_ERR(iTemp);
					}
					continue;
				}
				else
				{
					return  WIFI_COMM_ERR(iTemp);
				}
			}

			if (strcmp(szlocalIP, "0.0.0.0") == 0)
			{
				if (iRetry>=3)
				{
					return  WIFI_COMM_ERR(EWIFI_SOCKIPREG);
				}
				sysDelayMs(2000);
				continue;
			}
			else
			{
				break;
			}
		}

		if (strcmp(szlocalIP, "0.0.0.0") != 0)
		{
			strcpy((char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szLocalIP,szlocalIP);
			break;
		}
	}

	return 0;
}

int   GetWiFiStatus(void)
{
	int       iRet, iRetry, iTemp;
    char      szlocalIP[32];
    WiFiStatus_t    stWiFiStatus;

	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}

	memset(szlocalIP, 0, sizeof(szlocalIP));
	iRetry = 0;
	while(1)
	{
		while(1)
		{
			iRetry++;
			lcdClrLine(2,7);	
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PLS WAITING...");
			lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY|DISP_CLRLINE,"CONNECTING..(%d)",iRetry);
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			iRet = WifiGetCurConnect(&stWiFiStatus);
			iTemp = iRet;
			if (iRet != NET_OK || stWiFiStatus.Status != WIFI_STATUS_CONNECTED)
			{
				if (iTemp== NET_OK)
				{
					iTemp = WIFI_COMM_ERR(EWIFI_ONLINELOST);
				}

				if (iRetry>=3)
				{
					if (iTemp<0)
					{
						iTemp = -iTemp;
					}
					return  WIFI_COMM_ERR(iTemp);
				}
				sysDelayMs(2000);
				continue;	
			}

			break;
		}

		iRetry = 0;
		while(1)
		{
			iRetry++;
			lcdClrLine(2,7);	
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PLS WAITING...");
			lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY|DISP_CLRLINE,"GET LOCOAL IP..(%d)",iRetry);
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			memset(szlocalIP, 0, sizeof(szlocalIP));
			iRet = WifiGetLocalIP(szlocalIP);
			if(iRet != NET_OK)
			{
				if (iRetry>=3)
				{
					if (iRet<0)
					{
						iRet = -iRet;
					}
					return  WIFI_COMM_ERR(iRet);
				}
				continue;
			}

			if (strcmp(szlocalIP, "0.0.0.0") == 0)
			{
				if (iRetry>=3)
				{
					return  WIFI_COMM_ERR(EWIFI_SOCKIPREG);
				}
				sysDelayMs(2000);
				continue;
			}
			else
			{
				break;
			}
		}

		if (strcmp(szlocalIP, "0.0.0.0") != 0)
		{
			break;
		}
	}

	return 0;
}

int    SetWIFIParam(int iMode)
{
	int		iFlag, iTryCnt=0;
	WiFiStatus_t    stWiFiStatus;
	char    szlocalIP[20];
	int		iRet;
	
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "SETUP WIFI      ");
	DispMulLanguageString(0, 4, DISP_CFONT,NULL, " PROCESSING....");
	lcdFlip();
	
	WnetInit(20000);
	iFlag = 1;
	while(iFlag)
	{
		iRet = WifiGetCurConnect(&stWiFiStatus);
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "SETUP WIFI      ");
		lcdDisplay(0,2,DISP_ASCII|DISP_MEDIACY,"%16.16s",stWiFiStatus.SSID);
		if (stWiFiStatus.Status != WIFI_STATUS_CONNECTED)
			lcdDisplay(0, 3, DISP_ASCII, "STATUS: DISCONNECT");
		else
			lcdDisplay(0, 3, DISP_ASCII, "STATUS: CONNECTED");
		lcdDisplay(0, 4, DISP_ASCII, "SIG: %d%",stWiFiStatus.SigLevel);
		if (stWiFiStatus.LnkQual > 75)
		{
			lcdDisplay(0, 4, DISP_ASCII, "QUALITY: EXCELLED");
		}
		else if (stWiFiStatus.LnkQual > 50)
		{
			lcdDisplay(0, 4, DISP_ASCII, "QUALITY: GOOD");
		}
		else
		{
			lcdDisplay(0, 4, DISP_ASCII, "QUALITY: BAD");
		}
		
		while(iTryCnt++<3)
		{
			memset(szlocalIP, 0, sizeof(szlocalIP));
			iRet = WifiGetLocalIP(szlocalIP);
			if(iRet==OK && strcmp(szlocalIP,"0.0.0.0")!=0)
			{
				break;
			}
			else
			{
				sysDelayMs(1000);
			}
		}
		if (iRet !=0 || strcmp(szlocalIP,"0.0.0.0")==0 )
			lcdDisplay(0, 5, DISP_ASCII, "LOCAL IP: 0.0.0.0");
		else
			lcdDisplay(0, 5, DISP_ASCII, "LOCAL:%s",szlocalIP);	
		lcdDisplay(0,6,DISP_CFONT,"[FN/MENU] MODIFY");
		lcdFlip();
		iRet = PubWaitKey(10);
		switch (iRet)
		{
		case KEY_FN:
		case KEY_MENU:
			SelectAndSetWIFIAP();
			break;
		default:
			iFlag = 0;
			break;
		}
	}
	
	iRet = InitWiFiModule(TRUE);
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "SETUP WIFI      ");	
	iRet = SetTcpIpParam(iMode);
	if (iRet!=0)
	{
		return iRet;
	}
	
	return 0;
}

void SetSupportPinpad(void)
{
	UINT8 ucKey;
	
	ucKey = OptionsSelect(" SUPPORT PINPAD ", stPosParam.ucSupPinpad==PARAM_OPEN,
		"CURRENT:YES", "CURRENT:NO",	"[1].YES", "[2].NO", (stPosParam.ucOprtLimitTime));
	
	if( ucKey==KEY2 ) 
	{
		if(stPosParam.ucSupPinpad != PARAM_CLOSE)
		{
			stPosParam.ucSupPinpad = PARAM_CLOSE;
			stTransCtrl.ucLogonFlag=LOGGEDOFF;
		}
	}
	else if( ucKey==KEY1 ) 
	{
		
		if(stPosParam.ucSupPinpad != PARAM_OPEN)
		{
			stPosParam.ucSupPinpad = PARAM_OPEN;
			stTransCtrl.ucLogonFlag=LOGGEDOFF;
		}
		EppOpen(g_epp_Port_Num);
	}
	else if( ucKey==KEY_CANCEL )
		return;		
}

int CheckConnectPinPad(uint8_t ucFlag)
{
	if (ucFlag)
	{
		if (EppOpen(g_epp_Port_Num) != EPP_SUCCESS || EppLight(2,60*1000) != EPP_SUCCESS)
		{
			lcdClrLine(2, 7);    
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "CHECK PINPAD");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(3000);
			return NO_DISP;
		}
	}
	else
	{
		// 8110GPRS:PORT_COM1£»8110E:PORT_WNET
		if (EppOpen(PORT_WNET) != EPP_SUCCESS || EppLight(2,60*1000) != EPP_SUCCESS)
		{
			EppClose();
			EppOpen(PORT_PINPAD);
			EppLight(2,60*1000);
		}
	}
	return 0;
}

void IfSupportPinpad(void)
{
	if (PARAM_OPEN != stPosParam.ucSupPinpad && PARAM_CLOSE != stPosParam.ucSupPinpad)
	{
		stPosParam.ucSupPinpad = PARAM_CLOSE;
	}

	stPosParam.ucSupportElecSign = PARAM_CLOSE;
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

void lcdFlip(void)
{

}

void lcdClrRect(int x, int y, int width, int height)
{

}

void newprnLogo(int x, int y, int w, int h, uint8_t *filename)
{
	prnLogo(EM_BMP_ROTATE0,filename);
}

int start_timer(time_t sec, suseconds_t usec)
{
	return 0;
}

int stop_timer(void)
{
	return 0;
}

int fileMakeDir(const char *pathname)
{
	return 0;
}
uint8_t TransElecSignature(void)  
{
	return OK;
}
int jbg2png(const char *jbgfile, const char *pngfile)
{
	return -1;
}
uint8_t TrickleFeedElecSignTxn(uint8_t ucAllowBreak,uint8_t szSettleBatch)
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
void   ClearElecSignaturefile(void)
{
	
}

int     CheckWirelessAttached(uint32_t uiTimeOut)
{
	return 0;
}

int ExitApplication(void)
{
	return 0;
}

int wnet_power_down(void)
{
	return 0;
}

void   AdjustMenuContext(const char *pszInStr, char *pszOutStr)
{
}

void lcdGetSize(int *width, int *height)
{
	*width = 128;
	*height = 64;
}

void lcdDispScreen(INT32 X, INT32 Y, UINT8 *pucBitMap,UINT8 ucFirst,UINT8 *pDispBuf1,UINT8 *pDispBuf2,UINT8 ucMode)
{	
	if (ucFirst == 0)
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, pDispBuf1);
		if (strlen(pDispBuf2))
		{
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, pDispBuf2);
		}		
		lcdFlip();
	}
}

void _root_beep(unsigned int frequency, unsigned int keepms)
{
}

void AppMagOpen(void)
{
	magOpen();
}

void AppMagClose(void)
{
	magClose();
}

void GetBaseStationInfo(uint8_t *szStationInfo)
{
}


#endif
//end of line




