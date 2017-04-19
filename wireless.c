
#define  _NEWPOS_WIRELESS_C

#include "posapi.h"
#include "terstruc.h"
#include "glbvar.h"
#include "wireless.h"
#include "arpa/inet.h"
#include "posappapi.h"
#include "tranfunclist.h"
#include "libsbTms.h"


int CheckIpValid(char *ip)
{
	int i;
	
	i = PubAsc2Long(ip, 3, NULL);
	if(i > 255)
		return 0;
	else
		return 1;
}

int SetIpAddr(uint8_t x, uint8_t y, char *ipAddr)
{
	int pos = 0;
	int ch;
	char tmp_buf[20];
	
	strcpy(tmp_buf, "___.___.___.___");
	lcdDisplay(x, y, DISP_CFONT, (char*)tmp_buf);
	
	while(1)
	{
		lcdDisplay(x, y, DISP_CFONT | DISP_CLRLINE, tmp_buf);
		lcdFlip();
		ch = kbGetKey();
		if( ch==KEY_CANCEL )
			return -1;
		
		if( (ch>=KEY0) && (ch<=KEY9) && (pos<15) )
		{		
			tmp_buf[pos++] = ch;
			
			if( (pos==3) || (pos==7) || (pos==11) || (pos==15))
			{
				if(CheckIpValid(&tmp_buf[pos-3]))
				{
					if(pos <15)
						pos++;
				}
				else
				{
					pos -= 3;
					memcpy(&tmp_buf[pos], "___", 3);
				}
			}
		}
		else if(ch==KEY_ENTER)
		{
			if (pos==0)
			{
				return 1;
			}
			else if (pos==15)
			{
				strcpy((char*)ipAddr, (char*)tmp_buf);
				return 0;
			}
			else
			{
 				switch (pos%4)
				{
					case 1:
						tmp_buf[pos+1] = tmp_buf[pos-1];
						tmp_buf[pos] = tmp_buf[pos-1] = '0';
						pos += 2;
						break;
					case 2:
						tmp_buf[pos] = tmp_buf[pos-1];
						tmp_buf[pos-1] = tmp_buf[pos-2];
						tmp_buf[pos-2] = '0';
						pos ++;
						break;
					default:
						break;
				}
				if ( (pos==3) || (pos==7) || (pos==11) )
					pos++;
				else if (pos==15)
				{
					strcpy((char*)ipAddr, (char*)tmp_buf);
					return 0;
				}
			}
		}
		else if( (ch==KEY_BACKSPACE) && (pos>0) )
		{
			pos--;
			if( (pos==3) || (pos==7) || (pos==11) )
				pos--;
			tmp_buf[pos] = '_';
		}
	}
}


uint8_t SetModemPPPParam(POS_PARAM_STRC_MAIN *pSaveParam)//设置主控的modem ppp参数
{
	uint8_t tmpbuf[32];
	int iRet;

	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "TCP SETUP", "TCP SETUP");
	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, "SETUP PPP APN", "SETUP PPP APN");
	strcpy((char*)tmpbuf, (char*)pSaveParam->PPPNetTel);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)pSaveParam->PPPNetTel,(char*)tmpbuf))
	{
		strcpy((char*)pSaveParam->PPPNetTel,(char*)tmpbuf);
	}

	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, "SETUP USER NAME", "SETUP USER NAME");
	strcpy((char *)tmpbuf, (char *)pSaveParam->PPPUser);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,40,tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)pSaveParam->PPPUser,(char*)tmpbuf))
	{
		strcpy((char*)pSaveParam->PPPUser,(char *)tmpbuf);
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, "SETUP USER PWD", "SETUP USER PWD");
	strcpy((char*)tmpbuf, (char*)pSaveParam->PPPPwd);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)pSaveParam->PPPPwd,(char*)tmpbuf))
	{
		strcpy((char*)pSaveParam->PPPPwd,(char*)tmpbuf);
	}

	memset(tmpbuf,0,sizeof(tmpbuf));
	lcdCls();
	DispMulLanguageString(0, 2, DISP_CFONT, "SETUP HOST IP", "SETUP HOST IP");
	lcdDisplay(0, 4, DISP_CFONT, (char *)pSaveParam->szPOSRemoteIP);
	if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
	{
		strcpy((char*)pSaveParam->szPOSRemoteIP, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteWifiIp, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteGprsIp, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteCdmaIp, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemotePPPIp, (char*)tmpbuf);
	}
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, "SETUP HOST IP", "SETUP HOST IP");
	lcdDisplay(0, 4, DISP_CFONT, (char *)pSaveParam->szPOSRemotePort);
	lcdFlip();
	lcdGoto(80, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
		(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return KEY_CANCEL;
	}
	if( iRet>0 )
	{
		strcpy((char*)pSaveParam->szPOSRemotePort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteWifiPort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteGprsPort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteCdmaPort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemotePPPPort, (char*)tmpbuf);
	}

	return OK;

}

#if 0
uint8_t SetModemPPPParam(void)
{
	uint8_t tmpbuf[32];
	int iRet;

	PubDisplayTitle(TRUE,"SETUP PPP PARA");
	memset(tmpbuf,0,sizeof(tmpbuf));
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP HOST IP");
	lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.stDownParamCommCfg.stWirlessPara.szRemoteIP));
	if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
	{
		strcpy((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szRemoteIP, (char*)tmpbuf);
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP HOST PORT");
	lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.stDownParamCommCfg.stWirlessPara.szRemotePort));
	lcdFlip();
	lcdGoto(72, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return KEY_CANCEL;
	}
	else if( iRet>0 )
	{
		strcpy((char*)(stPosParam.stDownParamCommCfg.stWirlessPara.szRemotePort), (char*)tmpbuf);
		stPosParam.stDownParamCommCfg.stWirlessPara.szRemotePort[iRet] = 0;
	}

	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP PPP APN");
	strcpy((char*)tmpbuf, (char*)stPosParam.stDownParamCommCfg.stWirlessPara.szAPN);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szAPN,(char*)tmpbuf))
	{
		strcpy((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szAPN, (char*)tmpbuf);
	}

	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP USER NAME");
	strcpy((char *)tmpbuf, (char *)stPosParam.stDownParamCommCfg.stWirlessPara.szUID);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,40,tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szUID,(char*)tmpbuf))
	{
		strcpy((char *)stPosParam.stDownParamCommCfg.stWirlessPara.szUID, (char *)tmpbuf);
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP USER PWD");
	strcpy((char*)tmpbuf, (char*)(stPosParam.stDownParamCommCfg.stWirlessPara.szPwd));
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szPwd,(char*)tmpbuf))
	{
		strcpy((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szPwd, (char*)tmpbuf);
	}

	return OK;

}
#endif
/*
int SetDownModem(void)
{
	int iRet;


	if (stPosParam.stDownParamCommCfg.ucCommType == CT_MODEMPPP)
	{
		iRet = SetModemPPPParam();
		if (iRet != 0)
		{
			return iRet;
		}
	}
	else
	{
		iRet = SetModemAsyncParam();
		if (iRet != 0)
		{
			return iRet;
		}
	}
	return 0;
}
*/
#if 0
uint8_t SetWirelessParam(int iMode, uint8_t ucCommType)
{
	char   tmpbuf[100],buf2[2];
	int    iRet;

	lcdCls();	
	if( ucCommType==CT_GPRS )
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "GPRS SETUP");
	else
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CDMA SETUP");

	lcdDisplay(0, 2, DISP_CFONT, "LONG CONNECT");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucLongConnection;
	iRet = SelectSetTwo((uint8_t *)buf2);
	if( iRet==0 )
	{
		stPosParam.ucLongConnection = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucLongConnection = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return KEY_CANCEL;
	}

	if( ucCommType==CT_GPRS )	
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "DIAL NUMBER:");
		strcpy(tmpbuf, (char*)stPosParam.szDialNum);
		lcdDisplay(8, 4, DISP_CFONT, tmpbuf);
		iRet = PubGetString(ALPHA_IN|ECHO_IN,0,32,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
		if( iRet!=0 )
		{
			return KEY_CANCEL;
		}
		else if( strcmp((char*)stPosParam.szDialNum,tmpbuf) != 0)
		{
			strcpy((char*)&stPosParam.szDialNum, tmpbuf);
		}
	}

	lcdClrLine(2, 7);
	if( ucCommType==CT_GPRS )
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP APN:");
	else
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "DIAL NUMBER:");
	strcpy(tmpbuf, (char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN);	
	lcdDisplay(8, 4, DISP_CFONT, tmpbuf);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN,tmpbuf))
	{
		strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN, tmpbuf);
	}
	
	iRet = SetTcpIpParam(iMode);
	if (iRet!=0)
	{
		return iRet;
	}

	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, "USER NAME");
	lcdDisplay(0, 4, DISP_CFONT, "0.OFF  1.ON");
	lcdFlip();
	iRet = kbGetKey();
	if( iRet==KEY0 || iRet==KEY_CANCEL )
	{
		return OK;
	}
	
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP USER NAME");
	strcpy(tmpbuf, (char *)stPosParam.stTxnCommCfg.stWirlessPara.szUID);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,40,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szUID,tmpbuf))
	{
		strcpy((char *)stPosParam.stTxnCommCfg.stWirlessPara.szUID, tmpbuf);
	}
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP USER PWD");
	strcpy(tmpbuf, (char*)(stPosParam.stTxnCommCfg.stWirlessPara.szPwd));
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szPwd,tmpbuf))
	{
		strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szPwd, tmpbuf);
	}
	
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "SETUP SIM PWD");
	strcpy(tmpbuf, (char*)(stPosParam.stTxnCommCfg.stWirlessPara.szSimPin));
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szSimPin,tmpbuf))
	{
		strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szSimPin, tmpbuf);
	}

	if(stPosParam.stTxnCommCfg.ucCommType==CT_GPRS || stPosParam.stTxnCommCfg.ucCommType==CT_CDMA)
		CommInitModule(&stPosParam.stTxnCommCfg);
	

	return OK;
}
#endif

uint8_t SetWirelessParam(int iMode, uint8_t ucCommType, POS_PARAM_STRC_MAIN *pSaveParam)	//zyl
{
	char   tmpbuf[100],buf2[2];
	int    iRet;
//	char	dial_num[41];	//zyl

	lcdCls();	
	if( ucCommType==CT_GPRS )
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "GPRS参数设置", "GPRS SETUP");
	else
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "CDMA参数设置", "CDMA SETUP");

	lcdDisplay(0, 2, DISP_CFONT, "LONG CONNECT");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucLongConnection;
	iRet = SelectSetTwo((uint8_t *)buf2);
	if( iRet==0 )
	{
		stPosParam.ucLongConnection = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucLongConnection = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return KEY_CANCEL;
	}

	if( ucCommType==CT_GPRS )	//zyl
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, "DIAL NUMBER:", "DIAL NUMBER:");
		strcpy(tmpbuf, (char*)stPosParam.szDialNum);
		lcdDisplay(8, 4, DISP_CFONT, tmpbuf);
		iRet = PubGetString(ALPHA_IN|ECHO_IN,0,32,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
		if( iRet!=0 )
		{
			return KEY_CANCEL;
		}
		else if( strcmp((char*)stPosParam.szDialNum,tmpbuf) != 0)
		{
			strcpy((char*)&stPosParam.szDialNum, tmpbuf);
		}
	}

	lcdClrLine(2, 7);
	if( ucCommType==CT_GPRS )	//zyl
		DispMulLanguageString(0, 2, DISP_CFONT, "SETUP APN:", "SETUP APN:");
	else
		DispMulLanguageString(0, 2, DISP_CFONT, "DIAL NUMBER:", "DIAL NUMBER:");
	strcpy(tmpbuf, (char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN);	
	lcdDisplay(8, 4, DISP_CFONT, tmpbuf);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN,tmpbuf))
	{
		strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN, tmpbuf);
		strcpy((char*)pSaveParam->szPOSGprsApn, tmpbuf);
		strcpy((char*)pSaveParam->szPOSCdmaApn, tmpbuf);
	}
	
	iRet = SetTcpIpParam(iMode);
	if (iRet!=0)
	{
		return iRet;
	}

	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, "USER NAME");
	lcdDisplay(0, 4, DISP_CFONT, "0.OFF  1.ON");
	lcdFlip();
	iRet = kbGetKey();
	if( iRet==KEY0 || iRet==KEY_CANCEL )
	{
		return OK;
	}
	
	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, "SETUP USER NAME");
	strcpy(tmpbuf, (char *)stPosParam.stTxnCommCfg.stWirlessPara.szUID);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,40,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szUID,tmpbuf))
	{
		strcpy((char *)stPosParam.stTxnCommCfg.stWirlessPara.szUID, tmpbuf);
		strcpy((char*)pSaveParam->szPOSGprsUID, tmpbuf);
		strcpy((char*)pSaveParam->szPOSCdmaUID, tmpbuf);
	}

	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, "SETUP SIM PWD", "SETUP USER PWD");
	strcpy(tmpbuf, (char*)(stPosParam.stTxnCommCfg.stWirlessPara.szPwd));
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szPwd,tmpbuf))
	{
		strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szPwd, tmpbuf);
		strcpy((char*)pSaveParam->szPOSGprsPwd, tmpbuf);
		strcpy((char*)pSaveParam->szPOSCdmaPwd, tmpbuf);
	}
	
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, "SETUP SIM PWD", "SETUP SIM PWD");
	strcpy(tmpbuf, (char*)(stPosParam.stTxnCommCfg.stWirlessPara.szSimPin));
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return KEY_CANCEL;
	}
	else if( strcmp((char*)stPosParam.stTxnCommCfg.stWirlessPara.szSimPin,tmpbuf))
	{
		strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szSimPin, tmpbuf);
	}

	if(stPosParam.stTxnCommCfg.ucCommType==CT_GPRS || stPosParam.stTxnCommCfg.ucCommType==CT_CDMA)
		CommInitModule(&stPosParam.stTxnCommCfg);
	

	return OK;
}


static int s_WifiSetAp(uint8_t	*ssid_in, uint8_t *ssid_out)
{
	int iRet;
	int iMenuNo,iSsidIndex;
	int iApNum;
	uint8_t szBuffer[64];
    WiFiAPInfo_t    stApInfoList[64];
	MenuItem        stSelectMenu[64+1];

	if(ssid_out==NULL
		|| ssid_in==NULL)
	{
		return -1;
	}
	
	lcdClrLine(2, 7);
	DispMulLanguageString(0,0,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL,"   SETUP WIFI   ");
    DispMulLanguageString(0, 2, DISP_CFONT, NULL,"1.SCAN AP");
    DispMulLanguageString(0, 4, DISP_CFONT, NULL,"2.MANUAL SETUP");
	lcdFlip();
    iRet = kbGetKeyMs(60*1000);
	if (iRet != KEY1 && iRet != KEY2)
	{
		return NO_DISP;
	}
    if(iRet == KEY2)
    {
        lcdClrLine(2, 7);	
        DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL,"INPUT SSID:");
        memset(szBuffer,0,sizeof(szBuffer));
		strcpy((char*)szBuffer, (char*)ssid_in);
		lcdGoto(0, 6);
		iRet = PubGetString(ALPHA_IN|ECHO_IN, 1, 24, szBuffer, stPosParam.ucOprtLimitTime,0);
		if(iRet!=0)
		{
			strcpy((char *)ssid_out, (char *)szBuffer);
		}
    }
    else
    {
        lcdClrLine(2, 7);	
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"PLS WAIT...");
		DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"SCAN...");
		DrawRect(0, 17, 127, 63);
		lcdFlip();
        iApNum = WifiScanAP(64, stApInfoList);
        if(iApNum < 0)
        {
            lcdClrLine(2, 7);	
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"CAN'T SACN AP");
			DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"PLS TRY IT AGAIN");
			DrawRect(0, 17, 127, 63);
			lcdDisplay(96, 7, DISP_ASCII, "%d", iApNum);
			lcdFlip();
			kbGetKeyMs(3000);
			return NO_DISP;
        }

        if(iApNum == 0)
        {
            lcdClrLine(2, 7);	
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"CAN'T SACN AP");
			DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"PLS TRY IT AGAIN");
			DrawRect(0, 17, 127, 63);
			lcdDisplay(96, 7, DISP_ASCII, "%d", iApNum);
			lcdFlip();
			kbGetKeyMs(3000);
			return NO_DISP;
        }

		for(iMenuNo=0, iSsidIndex=0; iSsidIndex < iApNum; ++iSsidIndex)
		{
			if(strlen(stApInfoList[iSsidIndex].SSID) > 0)
			{
				stSelectMenu[iMenuNo].bVisible = TRUE;
				stSelectMenu[iMenuNo].pfMenuFunc = NULL;
				strcpy(stSelectMenu[iMenuNo].szMenuName,stApInfoList[iSsidIndex].SSID);
				++iMenuNo;
			}
		}

		stSelectMenu[iMenuNo].bVisible = FALSE;
		stSelectMenu[iMenuNo].pfMenuFunc = NULL;
		memset(stSelectMenu[iMenuNo].szMenuName,0,sizeof(stSelectMenu[iMenuNo].szMenuName));

        iMenuNo = PubGetMenu((uint8_t *)"SELECT WIFI", stSelectMenu, MENU_AUTOSNO, 60);
        if(iMenuNo < 0 || iMenuNo>= iApNum)
        {
            return NO_DISP;
        }
		strcpy((char*)ssid_out, (char*)stSelectMenu[iMenuNo].szMenuName);

    }

	return OK;
}

static int s_WifiSetChannel(void)
{
	int iRet;
	uint8_t szBuffer[5];
	
	while(1)
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL,"CHANNEL(0-13):");
		memset(szBuffer,0,sizeof(szBuffer));
		sprintf((char*)szBuffer, "%d", stPosParam.stWIFIAPInfo.iChannel);
		iRet = PubGetString(NUM_IN|ECHO_IN,1,2,(uint8_t *)szBuffer,60,0);
		if(iRet != 0)
		{
			return 0;
		}

		iRet = atoi((char*)szBuffer);
		if (iRet>=13)
		{
            lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL,"INVALID VALUE");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL,"RANGE:0--13");
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			kbGetKeyMs(3000);	
			continue;
		}

		break;
	}
	
	return iRet;
}

static int s_WifiSetPwd(uint8_t *pwd_in, uint8_t *pwd_out)
{
	char szBuffer[33];
	int iRet;

	if(pwd_in==NULL || pwd_out==NULL)
	{
		return -1;
	}
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0,2,DISP_CFONT, NULL,"SET PASSWORD");
	memset(szBuffer, 0, sizeof(szBuffer));
	sprintf(szBuffer, "%s", pwd_in);
	iRet = PubGetString(ALPHA_IN|ECHO_IN,0,32,(uint8_t *)szBuffer,60,0);
	if(iRet != 0)
	{
		return NO_DISP;
	}
	strcpy((char*)pwd_out, (char*)szBuffer);
	return OK;
}

static int s_WifiSwitchEncryptMode(int mode)
{
    switch(mode)
    {
    case WIFI_SECURITY_NONE:
        return WIFI_SECURITY_WPATKIP;
		break;
		
    case WIFI_SECURITY_WPATKIP:
        return WIFI_SECURITY_WPA2AES;
		break;
		
    case WIFI_SECURITY_WPA2AES:
        return WIFI_SECURITY_WEP64;
		break;
		
    case WIFI_SECURITY_WEP64:
        return WIFI_SECURITY_WEP128;
        break;
		
    case WIFI_SECURITY_WEP128:
        return WIFI_SECURITY_WPATKIP_E;
        break;
		
    case WIFI_SECURITY_WPATKIP_E:
        return WIFI_SECURITY_WPA2AES_E;
        break;
		
    case WIFI_SECURITY_WPA2AES_E:
        return WIFI_SECURITY_NONE;
        break;

    default:
		return WIFI_SECURITY_WPATKIP;
        break;
    }

	return WIFI_SECURITY_WPATKIP;
}

int    SelectAndSetWIFIAP(void)
{
	int   iRet=0;
	int iSwitchCnt=0, iGetCurConnectCnt=0;
    WiFiConfig_t    stWIFIConfig;
    WiFiAPx_t       stApxInfo;    
    WiFiDefAP_t     stDefApInfo;
	WiFiStatus_t	stWiFiStatus;

	memset((char*)&stWIFIConfig,0,sizeof(stWIFIConfig));
	memset((char*)&stApxInfo,0,sizeof(stApxInfo));
	memset((char*)&stDefApInfo,0,sizeof(stDefApInfo));
	
	lcdCls();
	DispMulLanguageString(0,0,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL,"   SETUP WIFI   ");
	
	s_WifiSetAp((uint8_t*)stPosParam.stWIFIAPInfo.szSSID, (uint8_t*)stPosParam.stWIFIAPInfo.szSSID);
	DispMulLanguageString(0,0,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL,"   SETUP WIFI   ");
	stPosParam.stWIFIAPInfo.iChannel = s_WifiSetChannel();
	s_WifiSetPwd((uint8_t*)stPosParam.stWIFIAPInfo.szWPAPSK, (uint8_t*)stPosParam.stWIFIAPInfo.szWPAPSK);

	stPosParam.stWIFIAPInfo.iWEPKeyIdx = 1;
	sprintf(stPosParam.stWIFIAPInfo.szListWEPKey[0], "%.31s", stPosParam.stWIFIAPInfo.szWPAPSK);
	sprintf(stPosParam.stWIFIAPInfo.szListWEPKey[1], "%.31s", stPosParam.stWIFIAPInfo.szWPAPSK);
	sprintf(stPosParam.stWIFIAPInfo.szListWEPKey[2], "%.31s", stPosParam.stWIFIAPInfo.szWPAPSK);
	sprintf(stPosParam.stWIFIAPInfo.szListWEPKey[3], "%.31s", stPosParam.stWIFIAPInfo.szWPAPSK);

	stDefApInfo.WEPKeyIdx = 1;
	memcpy(stDefApInfo.WEPKey, stPosParam.stWIFIAPInfo.szListWEPKey, 
		sizeof(stDefApInfo.WEPKey));
	stDefApInfo.WEPMode = stPosParam.stWIFIAPInfo.iSecurityType;
	strcpy(stDefApInfo.SSID, stPosParam.stWIFIAPInfo.szSSID);
	
	stApxInfo.SecurityType = stPosParam.stWIFIAPInfo.iSecurityType;
	strcpy(stApxInfo.SSID, stPosParam.stWIFIAPInfo.szSSID);
	memcpy(stApxInfo.WPAPSK, stPosParam.stWIFIAPInfo.szWPAPSK, 
		sizeof(stApxInfo.WPAPSK));

    stWIFIConfig.IchipPowerSave = 5;
    stWIFIConfig.WLANPowerSave = 5;
    stWIFIConfig.RoamingMode = 0;
    stWIFIConfig.PeriodicScanInt = 5;
    stWIFIConfig.RoamingLowSNR = 10;
    stWIFIConfig.RoamingHighSNR = 30;
	stWIFIConfig.Channel = stPosParam.stWIFIAPInfo.iChannel;

	if(stPosParam.stWIFIAPInfo.iSecurityType<WIFI_SECURITY_NONE
		|| stPosParam.stWIFIAPInfo.iSecurityType>WIFI_SECURITY_WPA2AES_E)
	{
		stApxInfo.SecurityType =
			stDefApInfo.WEPMode =
			stPosParam.stWIFIAPInfo.iSecurityType = WIFI_SECURITY_NONE;
	}

TRY_EACH_ENCRYPT_MODE:	
	while(iSwitchCnt++<7)
	{
		lcdCls();	
		DispMulLanguageString(0,0,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL,"   SETUP WIFI   ");
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"PLS WAIT...");
		DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"ACTIVE SETTING..");
		DrawRect(0, 17, 127, 63);
		lcdFlip();
		if(stApxInfo.SecurityType == WIFI_SECURITY_WEP64 || stApxInfo.SecurityType == WIFI_SECURITY_WEP128)
	    {
			iRet = WifiSetDefAP(&stDefApInfo);
	    }
	    else
	    {
	 	    iRet = WifiSetAP(0, &stApxInfo);
	    }
	    if(iRet != NET_OK)
	    {
			stApxInfo.SecurityType =
				stDefApInfo.WEPMode =
				stPosParam.stWIFIAPInfo.iSecurityType = 
				s_WifiSwitchEncryptMode(stPosParam.stWIFIAPInfo.iSecurityType);
	    }
		else
		{
			break;
		}
	}

	if(iRet != OK)
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"Set AP FAILURE");
		DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"TRY IT AGAIN");
		DrawRect(0, 17, 127, 63);
		lcdFlip();
		kbGetKeyMs(5000);
		SaveAppParam();
		return NO_DISP;
	}
	
    iRet = WifiSetParms(&stWIFIConfig);
    if(iRet != NET_OK)
    {
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"Set AP FAILURE");
		DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"TRY IT AGAIN");
		DrawRect(0, 17, 127, 63);
		lcdDisplay(96, 7, DISP_ASCII, "%d", iRet);
		lcdFlip();
		kbGetKeyMs(5000);
	    return NO_DISP;
    }

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY,NULL,"PLS WAIT...");
	DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"RESET WI-FI");
	DrawRect(0, 17, 127, 63);
	lcdFlip();
    iRet = WifiSoftReset(0);
    if(iRet != NET_OK)
    {
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"Set WIFI FAILURE");
		DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"TRY IT AGAIN");
		DrawRect(0, 17, 127, 63);
		lcdDisplay(96, 7, DISP_ASCII, "%d", iRet);
		lcdFlip();
		kbGetKeyMs(5000);
	    return NO_DISP;
    }

	iGetCurConnectCnt = 0;
	while(iGetCurConnectCnt++<3)
	{
		iRet = WifiGetCurConnect(&stWiFiStatus);
		if (iRet != NET_OK || stWiFiStatus.Status != WIFI_STATUS_CONNECTED)
		{
			if(iGetCurConnectCnt>=3 && iSwitchCnt<7)
			{
				stApxInfo.SecurityType =
					stDefApInfo.WEPMode =
					stPosParam.stWIFIAPInfo.iSecurityType = 
					s_WifiSwitchEncryptMode(stPosParam.stWIFIAPInfo.iSecurityType);
				++iSwitchCnt;
				goto TRY_EACH_ENCRYPT_MODE;
			}
		}
		else
		{
			break;
		}
	}
	
	SetWiFiLoaclIPCfg();

	SaveAppParam();
	return 0;
}

int    SetWiFiLoaclIPCfg(void)
{
    int         iRet;
    tcpipcfg_t  stdevcfg;
    char        szIpaddr[64];
	#define	GET_LOACLIP_DELAY_MS 3500

    PubDisplayTitle(TRUE,"SETUP LOACL IP");
    memset(&stdevcfg, 0, sizeof(stdevcfg));

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL,"GET LOCAL IP");
	DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL,"PLS WAIT...");
	DrawRect(0, 17, 127, 63);
	lcdFlip();
	sysDelayMs(GET_LOACLIP_DELAY_MS);
    iRet = NetGetTCPIPCfg(OPT_DEVWIFI, &stdevcfg);
    if(iRet != NET_OK)
    {
        lcdDisplay(0, 2, DISP_CFONT|DISP_CLRLINE, "Get CFG error");
        lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
		lcdFlip();
        PubWaitKey(60);
        return(iRet);
    }

	lcdCls();
    	lcdDisplay(0, 0, DISP_ASCII|DISP_CLRLINE, "DHCP: %s", stdevcfg.dhcp==NO ? "DISABLE" : "ENABLE");
	lcdFlip();
    	memset(szIpaddr, 0, sizeof(szIpaddr));
	iRet = WifiGetLocalIP(szIpaddr);
	if(iRet != OK)
	{
		strcpy(szIpaddr, "0.0.0.0");
	}
	inet_pton(AF_INET, (char *)szIpaddr, &stdevcfg.localip);
	lcdDisplay(0, 1, DISP_ASCII|DISP_CLRLINE, "IP  : %s", szIpaddr);

    	memset(szIpaddr, 0, sizeof(szIpaddr));
    	inet_ntop(AF_INET, &(stdevcfg.subnet), szIpaddr, sizeof(szIpaddr));
	lcdDisplay(0, 2, DISP_ASCII|DISP_CLRLINE, "MASK: %s", szIpaddr);

    	memset(szIpaddr, 0, sizeof(szIpaddr));
    	inet_ntop(AF_INET, &(stdevcfg.gateway), szIpaddr, sizeof(szIpaddr));
	lcdDisplay(0, 3, DISP_ASCII|DISP_CLRLINE, "GATE: %s", szIpaddr);

    	memset(szIpaddr, 0, sizeof(szIpaddr));
    	inet_ntop(AF_INET, &(stdevcfg.dns1), szIpaddr, sizeof(szIpaddr));
	lcdDisplay(0, 4, DISP_ASCII|DISP_CLRLINE, "DNS1: %s", szIpaddr);

    	memset(szIpaddr, 0, sizeof(szIpaddr));
    	inet_ntop(AF_INET, &(stdevcfg.dns2), szIpaddr, sizeof(szIpaddr));
	lcdDisplay(0, 5, DISP_ASCII|DISP_CLRLINE, "DNS2: %s", szIpaddr);

    	lcdDisplay(0, 6, DISP_CFONT, "[0]--Modify");
	lcdFlip();
    	kbFlush();
    	iRet = kbGetKey();
    	if(iRet == KEY0)
    	{
		PubDisplayTitle(TRUE,"SETUP LOACL IP");
        	lcdDisplay(0, 2, DISP_CFONT, "SET DHCP:");
	        if(stdevcfg.dhcp == NO)
	        {
	            lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "DISABLE");
	        }
	        else
	        {
	            lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "ENABLE");
	        }
		lcdDisplay(0, 6, DISP_CFONT, "1:ENABLE*  0:DIS");
        	kbFlush();
		lcdFlip();
        	iRet = PubWaitKey(60);
        	if(iRet == KEY0)
        	{
            		stdevcfg.dhcp = NO;
        	}
        	else if(iRet == KEY1)
        	{
            		stdevcfg.dhcp = YES;
        	}
        	else if(iRet != KEY_ENTER)
        	{
            		return(KB_CANCEL);
        	}

        if(stdevcfg.dhcp == NO)
        {
            		lcdClrLine(2, 7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_LEFT, NULL,"LOCAL IP:");
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "%s", inet_ntoa(stdevcfg.localip));
			iRet = SetIpAddr(0, 6, (char*)stPosParam.stTxnCommCfg.stWIFIIpPara.szLocalIP);
			if( iRet==0 )
			{
				inet_pton(AF_INET, (char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szLocalIP, &stdevcfg.localip);
			}

            		lcdClrLine(2, 7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_LEFT, NULL,"Subnet Mask");
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "%s", inet_ntoa(stdevcfg.subnet));
			iRet = SetIpAddr(0, 6, (char*)stPosParam.stTxnCommCfg.stWIFIIpPara.szNetMask);
			if( iRet==0 )
			{
				inet_pton(AF_INET, (char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szNetMask, &stdevcfg.subnet);
			}

            		lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_LEFT, NULL,"Default Gateway:");
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "%s", inet_ntoa(stdevcfg.gateway));
			iRet = SetIpAddr(0, 6, (char*)stPosParam.stTxnCommCfg.stWIFIIpPara.szGatewayIP);
			if( iRet==0 )
			{
				inet_pton(AF_INET, (char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szGatewayIP, &stdevcfg.gateway);
			}

            		lcdClrLine(2, 7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_LEFT, NULL,"DNS SERVER:");
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "%s", inet_ntoa(stdevcfg.dns1));
			iRet = SetIpAddr(0, 6, (char*)stPosParam.stTxnCommCfg.stWIFIIpPara.szDNS);
			if( iRet==0 )
			{
				inet_pton(AF_INET, (char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szDNS, &stdevcfg.dns1);
			}
        }
        lcdClrLine(2, 7);
        lcdDisplay(0, 3, DISP_CFONT, "SAVE CFG...");
        iRet = NetSetTCPIPCfg(OPT_DEVWIFI, &stdevcfg);
        if(iRet != NET_OK)
        {
            lcdDisplay(0, 3, DISP_CFONT|DISP_CLRLINE, NULL, "Save CFG error");
            lcdDisplay(0, 5, DISP_CFONT|DISP_CLRLINE, "RET = %d", iRet);
	     lcdFlip();
            PubWaitKey(60);
            return(iRet);
        }
    }
    return(OK);
}

void SetCommHostFlag(void)
{
	int key1;

	key1 = OptionsSelect("SERVER ACCESS", stPosParam.ucCommHostFlag==PARAM_OPEN,
		"CURRENT:DNS", "CURRENT:IP", "[1].DNS", "[2].IP", stPosParam.ucOprtLimitTime);

	if( key1==KEY1 )
	{
		stPosParam.ucCommHostFlag = PARAM_OPEN;
	}
	else if( key1==KEY2 )
	{
		stPosParam.ucCommHostFlag = PARAM_CLOSE;
	}
}

int SetWnetMode(void)
{
	int     iRet;
	uint8_t buf2[2];
	
	lcdClrLine(2, 7);
	lcdDisplay(0, 2, DISP_CFONT, "0.MANY INIT");
	lcdDisplay(0, 4, DISP_CFONT, "1.TWO OPEN PPP");
    	memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucWnetMode;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucWnetMode = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucWnetMode = PARAM_OPEN;
	}
	return iRet;
}

// 设置主控的TCP/IP参数
int SetManageTcpIpParam(POS_PARAM_STRC_MAIN *pSaveParam)
{
	uint8_t tmpbuf[32];
	int iRet;
	
	memset(tmpbuf,0,sizeof(tmpbuf));
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "远程下载IP设置", "TCP SETUP");
	DispMulLanguageString(0, 2, DISP_CFONT, "主机1IP地址", "SETUP HOST IP1");
	lcdDisplay(0, 4, DISP_CFONT, (char *)pSaveParam->szPOSRemoteIP);
	if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
	{
		strcpy((char*)pSaveParam->szPOSRemoteIP, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteWifiIp, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteGprsIp, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteCdmaIp, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemotePPPIp, (char*)tmpbuf);
	}
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, "主机1端口号    ", "SETUP HOST PORT1");
	lcdDisplay(0, 4, DISP_CFONT, (char *)pSaveParam->szPOSRemotePort);
	lcdFlip();
	lcdGoto(80, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
		(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return KEY_CANCEL;
	}
	if( iRet>0 )
	{
		strcpy((char*)pSaveParam->szPOSRemotePort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteWifiPort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteGprsPort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteCdmaPort, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemotePPPPort, (char*)tmpbuf);
	}
	
	memset(tmpbuf,0,sizeof(tmpbuf));
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, "主机2IP地址", "SETUP HOST IP2");
	lcdDisplay(0, 4, DISP_CFONT, (char *)pSaveParam->szPOSRemoteIP2);
	if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
	{
		strcpy((char*)pSaveParam->szPOSRemoteIP2, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteWifiIp2, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteGprsIp2, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteCdmaIp2, (char*)tmpbuf);
	}
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, "主机2端口号    ", "SETUP HOST PORT2");
	lcdDisplay(0, 4, DISP_CFONT, (char *)pSaveParam->szPOSRemotePort2);
	lcdFlip();
	lcdGoto(80, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
		(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return KEY_CANCEL;
	}
	if( iRet>0 )
	{
		strcpy((char*)pSaveParam->szPOSRemotePort2, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteWifiPort2, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteGprsPort2, (char*)tmpbuf);
		strcpy((char*)pSaveParam->szPOSRemoteCdmaPort2, (char*)tmpbuf);
	}
	return OK;
}

// ??TCP/IP??
int SetTcpIpParam(int iMode)
{
	uint8_t tmpbuf[32];
	int iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "TCP SETUP");
	if (iMode == DOWNLAOD_COMMTYPE)
	{
		memset(tmpbuf,0,sizeof(tmpbuf));
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST IP");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.stTxnCommCfg.stWirlessPara.szRemoteIP));
		if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
		{
			strcpy((char*)stPosParam.stTxnCommCfg.stWirlessPara.szRemoteIP, (char*)tmpbuf);
		}

		sprintf((char *)stPosParam.stTxnCommCfg.stTcpIpPara.szRemoteIP,"%s",stPosParam.stTxnCommCfg.stWirlessPara.szRemoteIP);
		sprintf((char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szRemoteIP,"%s",stPosParam.stTxnCommCfg.stWirlessPara.szRemoteIP);
	
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST PORT");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.stTxnCommCfg.stWirlessPara.szRemotePort));
		lcdFlip();
		lcdGoto(80, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
			(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return KEY_CANCEL;
		}
		if( iRet>0 )
		{
			strcpy((char*)(stPosParam.stTxnCommCfg.stWirlessPara.szRemotePort), (char*)tmpbuf);
		}

		
		sprintf((char *)stPosParam.stTxnCommCfg.stTcpIpPara.szRemotePort,"%s",stPosParam.stTxnCommCfg.stWirlessPara.szRemotePort);		
		sprintf((char *)stPosParam.stTxnCommCfg.stWIFIIpPara.szRemotePort,"%s",stPosParam.stTxnCommCfg.stWirlessPara.szRemotePort);
		return 0;
	}

	SetCommHostFlag();
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "TCP SETUP");
	if( stPosParam.ucCommHostFlag==PARAM_CLOSE)
	{
		memset(tmpbuf,0,sizeof(tmpbuf));
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST IP1");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.szPOSRemoteIP));
		if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
		{
			strcpy((char*)stPosParam.szPOSRemoteIP, (char*)tmpbuf);
		}
		
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST PORT1");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.szPOSRemotePort));
		lcdFlip();
		lcdGoto(80, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
			(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return KEY_CANCEL;
		}
		if( iRet>0 )
		{
			strcpy((char*)(stPosParam.szPOSRemotePort), (char*)tmpbuf);
		}
		
		memset(tmpbuf,0,sizeof(tmpbuf));
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST IP2");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.szPOSRemoteIP2));
		if( SetIpAddr(0, 6, (char*)tmpbuf)==0 ) 
		{
			strcpy((char*)stPosParam.szPOSRemoteIP2, (char*)tmpbuf);
		}
		
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST PORT2");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.szPOSRemotePort2));
		lcdFlip();
		lcdGoto(80, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
			(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return KEY_CANCEL;
		}
		if( iRet>0 )
		{
			strcpy((char*)(stPosParam.szPOSRemotePort2), (char*)tmpbuf);
		}
	}
	else
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT,NULL, " SETUP HOST DNS ");
		strcpy((char*)tmpbuf, (char*)stPosParam.stTxnCommCfg.stTcpIpPara.szDNS);
		iRet = PubGetString(ALPHA_IN|ECHO_IN,0,30,(uint8_t *)tmpbuf,stPosParam.ucOprtLimitTime,0);
		if( iRet!=0 )
			return KEY_CANCEL;
		else
			strcpy((char*)stPosParam.stTxnCommCfg.stTcpIpPara.szDNS, (char*)tmpbuf);		

		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SETUP HOST PORT");
		lcdDisplay(0, 4, DISP_CFONT, (char *)(stPosParam.szPOSRemotePort));
		lcdFlip();
		lcdGoto(80, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 5,
			(stPosParam.ucOprtLimitTime*1000), (char *)tmpbuf);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return KEY_CANCEL;
		}
		if( iRet>0 )
		{
			strcpy((char*)(stPosParam.szPOSRemotePort), (char*)tmpbuf);
		}
	}

	return OK;
}


int    SetRS232Param(void)
{
	int iKey ,ucFlag ;
	uint8_t ucCommType, szBuffer[30];
	
	ucCommType = stPosParam.stTxnCommCfg.stRS232Para.ucSendMode;
	ucFlag = 1;
	while(ucFlag)
	{
		lcdCls();	
		lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "SERIAL: ");
		if( ucCommType==CM_RAW )
		{
			lcdDisplay(0, 2, DISP_CFONT, "%.18s",  "=>Org. Msg");
			lcdDisplay(0, 4, DISP_CFONT, "%.18s",  "  Pack Msg");
		}
		else
		{
			lcdDisplay(0, 2, DISP_CFONT, "%.18s",  "  Org. Msg");
			lcdDisplay(0, 4, DISP_CFONT, "%.18s",  "=>Pack Msg");
		}
		lcdFlip();
		iKey = kbGetKeyMs(30000);
		switch (iKey)
		{
		case KEY_CANCEL:			
		case KEY_ENTER:	
			ucFlag = 0;  
			break;
		case KEY1 :			       
			ucCommType = CM_RAW;
			ucFlag = 0;
			break;
		case KEY2 :			       
			ucCommType = CM_SYNC;
			ucFlag = 0;
			break;			       			       
		case KEY_UP :
			if(ucCommType != CM_RAW)
			{
				ucCommType = CM_RAW;
			}
			else
			{
				ucCommType = CM_SYNC;
			}
			break;	
		case KEY_DOWN :
			if(ucCommType != CM_RAW)
			{
				ucCommType = CM_RAW;
			}
			else
			{
				ucCommType = CM_SYNC;
			}
			break;
		default:
			break;
		}
	}
	stPosParam.stTxnCommCfg.stRS232Para.ucSendMode = ucCommType;
	
	lcdCls();
	lcdDisplay(0,0,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,"SETUP SERIES");
	lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY, (char *)stPosParam.stTxnCommCfg.stRS232Para.szAttr);
	lcdFlip();
	lcdGoto(0, 4);
	memset(szBuffer, 0, sizeof(szBuffer));
	iKey = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CHAR+KB_EN_REVDISP+KB_EN_SHIFTLEFT+KB_EN_NEWLINE, 0, 24, 
		(stPosParam.ucOprtLimitTime*1000), (char *)szBuffer);
	if( iKey >0 && strcmp((char *)stPosParam.stTxnCommCfg.stRS232Para.szAttr, (char *)szBuffer))
	{
		strcpy((char *)stPosParam.stTxnCommCfg.stRS232Para.szAttr, (char *)szBuffer);
	}
	
	stPosParam.stTxnCommCfg.stRS232Para.ucPortNo = gstPosCapability.uiPortType;
	
	return 0;		
}

int SetAcqTransTelNo(int iMode)
{
	uint8_t buf[50];
	int     iRet;
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "COMM SETUP");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT PABX:");
	strcpy((char *)buf, (char *)stPosParam.szExtNum);
	iRet = PubGetString( ECHO_IN + ALPHA_IN, 0, 9,buf,stPosParam.ucOprtLimitTime,0 );
	if(iRet == 0x00)
	{
		strcpy((char *)stPosParam.szExtNum, (char *)buf);
		SaveCtrlParam();
	}

	if (iMode == DOWNLAOD_COMMTYPE)
	{
		lcdCls();	
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "REMOTE DOWNLOAD TEL");
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "DOWNLOAD TEL:");
		strcpy((char *)buf, (char *)stPosParam.szDownloadTel);
		lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
		lcdFlip();
		lcdGoto(0, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CHAR+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 14, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet>0 && strcmp((char *)stPosParam.szDownloadTel, (char *)buf))
		{
			strcpy((char *)stPosParam.szDownloadTel, (char *)buf);
		}
		
		strcpy((char *)stPosParam.stDownParamCommCfg.stPSTNPara.szTxnTelNo, (char *)stPosParam.szDownloadTel);
		strcpy((char *)stPosParam.stTxnCommCfg.stPSTNPara.szTxnTelNo, (char *)stPosParam.stDownParamCommCfg.stPSTNPara.szTxnTelNo);
		return 0;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "COMM SETUP");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NUM1:");
	strcpy((char *)buf, (char *)stPosParam.szPhoneNum1);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CHAR+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 14, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet>0 && strcmp((char *)stPosParam.szPhoneNum1, (char *)buf))
	{
		strcpy((char *)stPosParam.szPhoneNum1, (char *)buf);
	}
	
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NUM2:");
	strcpy((char *)buf, (char *)stPosParam.szPhoneNum2);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CHAR+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 14, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet>0 && strcmp((char *)stPosParam.szPhoneNum2, (char *)buf))
	{
		strcpy((char *)stPosParam.szPhoneNum2, (char *)buf);						  
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NUM3:");
	strcpy((char *)buf, (char *)stPosParam.szPhoneNum3);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CHAR+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 14, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet>0 && strcmp((char *)stPosParam.szPhoneNum3, (char *)buf))
	{
		strcpy((char *)stPosParam.szPhoneNum3, (char *)buf);						  
	}
	
	return 0;
}

int SetModemParam(void)
{
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "COMM SETUP");

	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_mode = MODEM_COMM_SYNC;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_speed = MODEM_CONNECT_1200BPS;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_mode = MODEM_DAIL_DTMF;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_pause = 1;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_timeo = 30;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.idle_timeo = 60;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.extension_chk = 1;
	return 0;
}

// pre-connect to host
int PreDial(void)
{
	int		iRet;
	
#ifdef _OFFLINE_TEST_VERSION
	return 0;
#endif	

	if( stPosParam.bPreDial==PARAM_CLOSE )
	{
		return 0;
	}

#ifndef _POS_TYPE_8210
	if(stPosParam.stTxnCommCfg.ucCommType!=CT_MODEM)
		return 0;
	SetPreDialDisplayFlag(1);
#else
	if(stPosParam.stTxnCommCfg.ucCommType==CT_MODEM)
		SetPreDialDisplayFlag(1);
#endif
	
	if (stPosParam.stTxnCommCfg.ucCommType == CT_MODEM)
		PosCom.ucConnectCnt = stPosParam.ucTelIndex;
	else
		PosCom.ucConnectCnt = stPosParam.ucTCPIndex;
	
	iRet = AdjustCommParam(0);
	if( iRet!=0 )
	{
		return iRet;
	}
	
	if (gstCurCommCfg.ucCommType ==CT_WIFI )
	{
		iRet = InitWiFiModule(FALSE);
		if (iRet != 0)
		{
			return iRet;
		}
	}
	
	iRet = CommDial(PREDIAL_MODE);
	if (CheckPreDialDisplayFlag())
	{
		iRet = CheckPreDial(iRet);
		if(iRet !=0)
		{
			return NO_DISP;
		}
	}

	return iRet;
}


int    CheckPreDial(int iPreDialRet)
{
	int iRet;
	
	iRet = iPreDialRet;
	iPreDialRet = iPreDialRet & MASK_ERR_CODE;
	
	if (iRet == ERR_COMM_INV_TYPE)
	{
		DispCommErrMsg(iRet);
		return NO_DISP; 
	}
	
	switch( gstCurCommCfg.ucCommType )
	{
	case CT_RS232:		
	case CT_MODEM:		
	case CT_TCPIP:
	case CT_WIFI:
		if (iPreDialRet != 0)
		{
			DispCommErrMsg(iRet);
			return NO_DISP;
		}
		break;
		
	case CT_CDMA:
	case CT_GPRS:
		if (iPreDialRet== NET_ERR_NOSIM ||
			iPreDialRet == NET_ERR_SIMDESTROY)
		{
			DispCommErrMsg(iRet);
			return NO_DISP;
		}
		break;
		
	default:
		DispCommErrMsg(ERR_COMM_INV_TYPE);
		return NO_DISP;
		break;
	}
	
	return 0;
}

void NacTxd(uint8_t *psTxdData, uint32_t uiDataLen)
{
	uint8_t	*psTemp, sWorkBuf[1024*50+10];
	
	if( uiDataLen>1024*50 )
	{
		Display2StringInRect(NULL, "INVALID PACK");
		lcdFlip();
		PubWaitKey(2);
		return;
	}
	
	sWorkBuf[0] = STX;
	sWorkBuf[1] = (uiDataLen/1000)<<4    | (uiDataLen/100)%10;	// convert to BCD
	sWorkBuf[2] = ((uiDataLen/10)%10)<<4 | uiDataLen%10;
	memcpy(&sWorkBuf[3], psTxdData, uiDataLen);
	sWorkBuf[3+uiDataLen]   = ETX;
	sWorkBuf[3+uiDataLen+1] = PubCalcLRC((char *)psTxdData, uiDataLen, (uint8_t)(sWorkBuf[1] ^ sWorkBuf[2] ^ ETX));
	uiDataLen += 5; 
	
	portClose(gstPosCapability.uiPortType);
	portOpen(gstPosCapability.uiPortType, (void *)"9600,8,n,1");
	psTemp = sWorkBuf;
	while( uiDataLen-->0 )
	{
		if( portSends(gstPosCapability.uiPortType, psTemp,1)!=0 )
		{
			psTemp++;
			break;
		}
		psTemp++;
	}
	portClose(gstPosCapability.uiPortType);
}



