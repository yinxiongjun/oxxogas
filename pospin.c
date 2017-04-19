#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "posappapi.h"

extern uint8_t g_InputPinLogoBmp[190];

int EnterPIN(uint8_t ucFlag)
{
	uint8_t	 szTmpBuff[25], sPan[17], szAmount[15];
	uint8_t	 ucKeyId, ucMode,ucDispFlag=0;
	int      iRet;
	uint32_t uiAmt=0;
	UINT8    szBuf[32];

	PosCom.ucPinEntry = FALSE;
	return OK;
	uiAmt=0;

	if ((PosCom.ucSwipedFlag==CARD_PASSIVE) && (PosCom.stTrans.ucQPBOCorPBOC==0))
	{
		if (PosCom.stTrans.iTransNo != QPBOC_ONLINE_SALE)
		{
			return OK;
		}
	}

	if( (ucFlag & 0x80)==0x00 )
	{
		if( !NeedPIN(stTemp.iTransNo) )
		{
			return OK;
		}
	}

	if( (ucFlag & 0x80)!=0x00 )
	{
		if( !ICNeedPIN(stTemp.iTransNo) )
		{
			return OK;
		}
	}
	lcdClrLine(2, 7);	
	memset(szAmount,0,sizeof(szAmount));
	ConvBcdAmount(PosCom.stTrans.sAmount, szAmount);
	do{
		if (stPosParam.ucSupPinpad != PARAM_OPEN)
		{
			iRet = PedGetSensitiveTimer(TYPE_PIN_INTERVAL);
			if( iRet>0 )
			{
				if (ucDispFlag == 0)
				{
					sprintf((char *)szBuf, "WAITING...");				
					lcdDisplay(0, 2, DISP_CFONT|DISP_CLRLINE, (char *)szBuf);
					lcdFlip();
					ucDispFlag = 1;
				}
				sysDelayMs(500); // Avoid CPU resources occupation
				continue;
			}			
		}
		lcdClrLine(2, 7);
		if( ucFlag==0 || ucFlag==0x80 )
		{
			if( stTemp.iTransNo!=POS_QUE )
			{
				sprintf((char *)szTmpBuff, "%s", szAmount);
				lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, (char *)szTmpBuff);
			}
		}
		else
		{
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "[55]PWD ERR,RETRY");
		}
		
		if (stTemp.iTransNo == EC_TOPUP_NORAML )
		{
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INPUT PIN:");
		}
		else
		{
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INPUT PIN:");
		}
		if (gstPosCapability.uiScreenType)
		{
			lcdDisplay(0, 8, DISP_CFONT|DISP_MEDIACY, "ENTER TO GO ON");
		}
		lcdFlip();
		break;		
	}while(1);
	if (PosCom.stTrans.iTransNo == EC_TOPUP_NORAML)
	{
		iRet = ExtractPAN(PosCom.stTrans.szTranferCardNo, sPan);
	}
	else
	{
		iRet = ExtractPAN(PosCom.stTrans.szCardNo, sPan);
	}
	if( iRet!=OK )
	{
		return iRet;
	}
	if( stPosParam.ucKeyMode==KEY_MODE_DES )
	{
		ucKeyId = PIN_KEY_ID;
		ucMode  = 0x01;
	}
	else
	{
		ucKeyId = DOUBLE_PIN_KEY_ID;
		ucMode  = 0x31;
	}
	if(stPosParam.ucSupPinpad==PARAM_OPEN)
	{
		iRet=EppLight(2,0);
	    if(iRet==EPP_RECV_TIMEOUT)
		{
           lcdClrLine(2, 7);	 
		   lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
		   lcdFlip();
		   ErrorBeep();
		   kbGetKeyMs(2000); 
		   EppLight(3,0);
		   return NO_DISP;
		}
		EppClearScreen();
		iRet= EppDisplayLogo(0,0,Bmp_Show_in_PIN);
		
		if( ucFlag==0 || ucFlag==0x80 )
		{
			if( stTemp.iTransNo!=POS_QUE )
			{
			    EppClearScreen();
				EppDisplayLogo(0, 0, Bmp_Show_AmtName);
				EppDisplayString(32, 0, 16, szAmount, strlen((char *)szAmount));
				EppDisplayLogo((strlen((char *)szAmount)*8+32), 0, Bmp_Show_CurrName);
				EppDisplayLogo( 0, 16, Bmp_Show_in_PIN);
			}
		}

		
		EppSetPinInputTimeout(stPosParam.ucOprtLimitTime*10);

		uiAmt = BcdToLong(PosCom.stTrans.sAmount, 6);
		while(1)
		{	
			kbFlush();
			iRet = EppGetPin(KEY_TYPE_PIN, ucKeyId, 0, 0x00, sPan, (uint8_t *)"0456789abc\0\0\0", PosCom.sPIN);
			iRet = ABS(iRet);
			switch( iRet ){
			case EPP_SUCCESS:
				PosCom.ucPinEntry = TRUE;
				EppLight(3,0);
				return OK;
			case EPP_NO_PIN:
				EppLight(3,0);
				if( (ucFlag&0x80)!=0x00 )
				{
					return PED_RET_NOPIN;
				}
				else
				{
					return OK;
				}	
			case RET_USER_PRESS_CANCEL_EXIT:
			case RET_USER_INPUT_TIME_OUT:
				EppLight(3,0);
				return E_TRANS_CANCEL;
			case RET_KEY_NOT_EXIST:
				EppLight(3,0);
				return PED_RET_NOKEY;
			default:
				break;
			}
			if (iRet!=EPP_USER_PRESS_CANCEL)
			{
				EppLight(3,0);
				return E_PINPAD_KEY;
			}
		}
	}
	else
	{		
		lcdGoto(32, 6);
		iRet = PedGetPin(ucKeyId, (uint8_t *)"0,4,5,6,7,8,9,10,11,12", sPan, FORMAT_0, PosCom.sPIN);
		return OK;       //netpay ≤ª–Ë“™ ‰√‹¬Î
		switch( iRet ){
		case PED_RET_OK:
			PosCom.ucPinEntry = TRUE;
			return OK;
		case PED_RET_NOPIN:
			if( (ucFlag&0x80)!=0x00 )
			{
				return PED_RET_NOPIN;
			}
			else
			{
				return OK;
			}
		case PED_RET_CANCEL:
			return E_TRANS_CANCEL;
		case PED_RET_TIMEOUT:
			return E_TRANS_CANCEL;
		case PED_RET_NOKEY:
			return PED_RET_NOKEY;
		default:
			return E_PINPAD_KEY;
		}
	}
}

void DispEppPedErrMsg(int rej)
{
	if (rej == NO_DISP){
		return;
	}

	lcdClrLine(2, 7);
	switch(rej) 
	{
	case PED_RET_NOPIN:	
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PLS INPUT PIN");
		break;
	case PED_RET_NOKEY:		
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PLS LOGON");
		break;
	case E_TRANS_CANCEL:		
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "USER CANCEL");
		break;
	case E_ERR_SWIPE:		
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "SWIPE ERR");
		break;
	default:
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "ERROR:%d",rej);
		break;
	}
	lcdFlip();
	PubBeepErr();
	PubWaitKey(5);
}

uint8_t DispBalance(void)
{
	UINT8	sBalance[6], szBuff[20]; 
	
	AscToBcd(sBalance, &glRecvPack.szExtAmount[8], 12);
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, " QUERY SUCCESS  ");
	
	memset(szBuff, 0, sizeof(szBuff));
	szBuff[0] = (glRecvPack.szExtAmount[7]!='C') ? '-' : ' ';
	ConvBcdAmount(sBalance, &szBuff[1]);

	DispBalAmt(szBuff,NULL);	
	return (OK);
}

uint8_t CalMAC(uint8_t * inbuf, int len, uint8_t * outbuf)
{
	uint8_t buf[17], tmpbuf[17];
	int     i, l, k, ret;
	uint8_t KeyId;

	if( stPosParam.ucKeyMode==KEY_MODE_DES )
		KeyId = MAC_KEY_ID;
	else
		KeyId = DOUBLE_MAC_KEY_ID;

	memset(buf, 0, 8);
	memset(inbuf+len, 0, 8);

	if( len%8 ) 
		l = len/8 + 1;		
	else 
		l = len/8;
	
	for(i=0; i<l; i++) 
	{
		for(k=0; k<8; k++) 
		{
			buf[k] = buf[k] ^ inbuf[i*8+k];
		}
		buf[8] = '\0';
	}
	
	BcdToAsc(tmpbuf, buf, 16);
	tmpbuf[16] = 0;
	
	memcpy(buf, tmpbuf, 8);
	buf[8] = 0;
	
	if( stPosParam.ucSupPinpad==PARAM_OPEN)
	{
		ret = EppGetMac(KEY_TYPE_MAC, KeyId, buf, 8, MAC_MODE_1, buf);
		if( ret!=EPP_SUCCESS )
			return(1);
	}
	else
	{
		ret = PedGetMac(KeyId, buf, 8, MAC_MODE_1, buf);
		if( ret!=PED_RET_OK )
			return(1);	
	}
	
	for(k=0; k<8; k++) 
	{
		buf[k] = buf[k] ^ tmpbuf[8+k];
	}

	if( stPosParam.ucSupPinpad==PARAM_OPEN)
	{
		ret = EppGetMac(KEY_TYPE_MAC, KeyId, buf, 8, MAC_MODE_1, buf);
		if( ret!=EPP_SUCCESS )
			return(1);
	}
	else
	{
		ret = PedGetMac(KeyId, buf, 8, MAC_MODE_1, buf);
		if( ret!=PED_RET_OK )
			return(1);	
	}
	
	BcdToAsc(tmpbuf, buf, 16);
	
	memcpy(outbuf, tmpbuf, 8);
	//outbuf[8] = '\0';
	return (OK);	
}


uint8_t ExtractPAN(uint8_t* cardno, uint8_t* pan)
{
	int16_t	len;
	
	len = strlen((char*)cardno);
	if( len<13 || len>19 ) 
		return E_ERR_SWIPE;
	
	memset(pan, '0', 16);
	
	memcpy(pan+4, cardno+len-13, 12);
	pan[16] = 0;
	
	return OK;
}

uint8_t LogonDivKey(uint8_t *pKeyData,uint8_t ucDispErrInFO)
{
	uint8_t ucRet;

	if (PARAM_OPEN == stPosParam.ucSupPinpad)
	{
		ucRet = LogonDivKey_EPP(pKeyData,ucDispErrInFO);
	}
	else
	{
		ucRet = LogonDivKey_PED(pKeyData,ucDispErrInFO);
	}

	return ucRet;
}

uint8_t LogonDivKey_EPP(uint8_t *pKeyData,uint8_t ucDispErrInFO)
{
	if( stPosParam.ucKeyMode==KEY_MODE_DES )
	{
		uint8_t sTempBuff[17], sTemp[17];
		int     ret, ucKeyId;
		
		memcpy(sTempBuff, pKeyData, 8);
		memcpy(sTempBuff+8, pKeyData, 8);
		ucKeyId = stPosParam.ucKeyIndex + 1;
		ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, TEST_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}

		memset(sTemp, 0x00, 8);
		ret = EppGetMac(KEY_TYPE_MAC, TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
		if( memcmp(sTemp, pKeyData+8, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "KEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}

		memcpy(sTempBuff, pKeyData+12, 8);
		memcpy(sTempBuff+8, pKeyData+12, 8);
		ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, TEST_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}

		memset(sTemp, 0, 8);
		ret = EppGetMac(KEY_TYPE_MAC, TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
		if( memcmp(sTemp, pKeyData+20, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "KEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}
		
		memcpy(sTempBuff, pKeyData, 8);
		memcpy(sTempBuff+8, pKeyData, 8);
		ret= EppDownloadAppKey(KEY_TYPE_PIN, PARITY_NONE|TDEA_DECRYPT, PIN_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
		
		memcpy(sTempBuff, pKeyData+12, 8);
		memcpy(sTempBuff+8, pKeyData+12, 8);
		ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, MAC_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
	}
	else
	{
				
		uint8_t sTempBuff[32+1], sTemp[32+1];
		int  ret, ucKeyId;
				
		memcpy(sTempBuff, pKeyData, 16);
		ucKeyId = stPosParam.ucKeyIndex + 1;
		ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, TEST_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
		
		memset(sTemp, 0, 8);
		ret = EppGetMac(KEY_TYPE_MAC, TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
		if (memcmp(sTemp, pKeyData+16, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PKEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}		

		memcpy(sTempBuff, pKeyData+20, 8);
		memcpy(sTempBuff+8, pKeyData+20, 8);
		ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, TEST_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}	

		memset(sTemp, 0, 8);
		ret = EppGetMac(KEY_TYPE_MAC, TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}
		if( memcmp(sTemp, pKeyData+36, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "MKEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}

		// TDK
		if (stPosParam.ucTrackEncrypt == PARAM_OPEN)
		{
			memcpy(sTempBuff, pKeyData+40, 16);
			ucKeyId = stPosParam.ucKeyIndex + 1;
			ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, TEST_KEY_ID, sTempBuff, 16, ucKeyId);
			if(ret!=EPP_SUCCESS)
			{  
				if(ret==EPP_RECV_TIMEOUT)
				{
					lcdClrLine(2, 7);	
					lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000); 
					return NO_DISP;
				}
				else
				{
					return (E_PINPAD_KEY); 
				}
			}	
		
			memset(sTemp, 0, 8);
			ret = EppGetMac(KEY_TYPE_MAC, TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
			if(ret!=EPP_SUCCESS)
			{  
				if(ret==EPP_RECV_TIMEOUT)
				{
					lcdClrLine(2, 7);	
					lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000); 
					return NO_DISP;
				}
				else
				{
					return (E_PINPAD_KEY); 
				}
			}
			if (memcmp(sTemp, pKeyData+56, 4)!=0 )
			{
				if (ucDispErrInFO!=2)
				{
					lcdClrLine(2, 7);	
					DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PKEY VERIFY ERR");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000);
				}
				return (0xff);
			}
		}

		memcpy(sTempBuff, pKeyData, 16);
		ret= EppDownloadAppKey(KEY_TYPE_PIN, PARITY_NONE|TDEA_DECRYPT, DOUBLE_PIN_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}			

		memcpy(sTempBuff, pKeyData+20, 8);
		memcpy(sTempBuff+8, pKeyData+20, 8);
		ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, DOUBLE_MAC_KEY_ID, sTempBuff, 16, ucKeyId);
		if(ret!=EPP_SUCCESS)
		{  
			if(ret==EPP_RECV_TIMEOUT)
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000); 
				return NO_DISP;
			}
			else
			{
				return (E_PINPAD_KEY); 
			}
		}	
		
		if (stPosParam.ucTrackEncrypt == PARAM_OPEN)
		{
			memcpy(sTempBuff, pKeyData+40, 16);			
			ret= EppDownloadAppKey(KEY_TYPE_MAC, PARITY_NONE|TDEA_DECRYPT, DOUBLE_TDK_KEY_ID, sTempBuff, 16, ucKeyId);
			if(ret!=EPP_SUCCESS)
			{  
				if(ret==EPP_RECV_TIMEOUT)
				{
					lcdClrLine(2, 7);
					lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000); 
					return NO_DISP;
				}
				else
				{
					return (E_PINPAD_KEY); 
				}
			}
		}	
	}

	return OK;
}

uint8_t LogonDivKey_PED(uint8_t *pKeyData,uint8_t ucDispErrInFO)
{
	if( stPosParam.ucKeyMode==KEY_MODE_DES )
	{
		uint8_t sTempBuff[17], sTemp[17];
		int     ret, ucKeyId;

		memcpy(sTempBuff, pKeyData, 8);
		memcpy(sTempBuff+8, pKeyData, 8);
		ucKeyId = stPosParam.ucKeyIndex + 1;
		ret = PedWriteMacKey(PARITY_NONE, ucKeyId, TEST_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK )
		{
			return (E_PINPAD_KEY);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication


		memset(sTemp, 0x00, 8);
		ret = PedGetMac(TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if( ret!=PED_RET_OK ) 
		{
			return(0xff);
		}
		if( memcmp(sTemp, pKeyData+8, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "KEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication

		memcpy(sTempBuff, pKeyData+12, 8);
		memcpy(sTempBuff+8, pKeyData+12, 8);
		ret = PedWriteMacKey(PARITY_NONE, ucKeyId, TEST_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK )
		{
			return (E_PINPAD_KEY);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication

		memset(sTemp, 0, 8);
		ret = PedGetMac(TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if( ret!=PED_RET_OK ) 
		{
			return(0xff);
		}

		if( memcmp(sTemp, pKeyData+20, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "KEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication
		
		memcpy(sTempBuff, pKeyData, 8);
		memcpy(sTempBuff+8, pKeyData, 8);
		ret = PedWritePinKey(PARITY_NONE, ucKeyId, PIN_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK ) 
		{
			return (E_PINPAD_KEY);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication
		
		memcpy(sTempBuff, pKeyData+12, 8);
		memcpy(sTempBuff+8, pKeyData+12, 8);
		ret = PedWriteMacKey(PARITY_NONE, ucKeyId, MAC_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK ) 
		{
			return (E_PINPAD_KEY);
		}
	}
	else
	{
				
		uint8_t sTempBuff[32+1], sTemp[32+1];
		int  ret, ucKeyId;
				
		memcpy(sTempBuff, pKeyData, 16);
		ucKeyId = stPosParam.ucKeyIndex + 1;
		ret = PedWriteMacKey(PARITY_NONE, ucKeyId, TEST_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK )
		{
			return (E_PINPAD_KEY);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication
		
		memset(sTemp, 0, 8);
		ret = PedGetMac(TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if( ret!=PED_RET_OK ) 
			return(0xff);
		if (memcmp(sTemp, pKeyData+16, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PKEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication

		memcpy(sTempBuff, pKeyData+20, 8);
		memcpy(sTempBuff+8, pKeyData+20, 8);
		ret = PedWriteMacKey(PARITY_NONE, ucKeyId, TEST_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK )
			return (E_PINPAD_KEY);
		sysDelayMs(50);//PED frequent operation will affect the communication

		memset(sTemp, 0, 8);
		ret = PedGetMac(TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
		if( ret!=PED_RET_OK ) 
			return(0xff);
		if( memcmp(sTemp, pKeyData+36, 4)!=0 )
		{
			if (ucDispErrInFO!=2)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "MKEY VERIFY ERR");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
			}
			return (0xff);
		}
		sysDelayMs(50);//PED frequent operation will affect the communication

		//TDK
		if(stPosParam.ucTrackEncrypt == PARAM_OPEN)
		{
			memcpy(sTempBuff, pKeyData+40, 16);
			ucKeyId = stPosParam.ucKeyIndex + 1;
			ret = PedWriteMacKey(PARITY_NONE, ucKeyId, TEST_KEY_ID, 16, sTempBuff);
			if( ret!=PED_RET_OK )
				return (E_PINPAD_KEY);
			sysDelayMs(50);//PED frequent operation will affect the communication
		
			memset(sTemp, 0, 8);
			ret = PedGetMac(TEST_KEY_ID, sTemp, 8, MAC_MODE_1, sTemp);
			if( ret!=PED_RET_OK ) 
				return(0xff);
			if (memcmp(sTemp, pKeyData+56, 4)!=0 )
			{
				if(ucDispErrInFO!=2)
				{
					lcdClrLine(2, 7);	
					DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PKEY VERIFY ERR");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000);
				}
				return (0xff);
			}
			sysDelayMs(50);//PED frequent operation will affect the communication
		}

		PedDelKey(TYPE_MACKEY, TEST_KEY_ID); 

		sysDelayMs(50);//PED frequent operation will affect the communication
		memcpy(sTempBuff, pKeyData, 16);
		ret = PedWritePinKey(PARITY_NONE, ucKeyId, DOUBLE_PIN_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK ) 
				return (E_PINPAD_KEY);
		sysDelayMs(50);//PED frequent operation will affect the communication

		memcpy(sTempBuff, pKeyData+20, 8);
		memcpy(sTempBuff+8, pKeyData+20, 8);
		ret = PedWriteMacKey(PARITY_NONE, ucKeyId, DOUBLE_MAC_KEY_ID, 16, sTempBuff);
		if( ret!=PED_RET_OK ) 
			return (E_PINPAD_KEY);
		sysDelayMs(50);//PED frequent operation will affect the communication
		
		if(stPosParam.ucTrackEncrypt == PARAM_OPEN)
		{
			memcpy(sTempBuff, pKeyData+40, 16);			
			ret = PedWriteMacKey(PARITY_NONE, ucKeyId, DOUBLE_TDK_KEY_ID, 16, sTempBuff);
			if( ret!=PED_RET_OK ) 
				return (E_PINPAD_KEY);
		}
	}

	return OK;
}


uint8_t DisplayLoyaltBalance(void)
{
	uint8_t	sBalance[6], szBuff[20]; 
	uint8_t szBuf[17];
	int     iLen;
	uint32_t uiBalance;
	
	AscToBcd(sBalance, &glRecvPack.szExtAmount[8], 12);
	memset(szBuff, 0, sizeof(szBuff));
	szBuff[0] = (glRecvPack.szExtAmount[7]!='C') ? '-' : ' ';
	uiBalance = PubBcd2Long(sBalance,6,NULL);
	sprintf((char*)&szBuff[1],"%d",uiBalance);
	
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT,NULL, " QUERY SUCCESS  ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "BALANCE:");
	memset(szBuf, 0x20, 16);
	iLen = strlen((char *)szBuff);
	memcpy(&szBuf[16-iLen-1], szBuff, iLen);
	szBuf[15] = 0;
	lcdDisplay(0, 6, DISP_CFONT, (char *)szBuf);
	lcdFlip();
	PubBeepErr();
	
	kbGetKeyMs(10000);

	return (OK);
}

// end of file 
