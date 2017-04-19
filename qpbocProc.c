
#include "MathLib.h"
#include "PosLib.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "EMVCallBack.h"
#ifdef CL_LED_SUPPORT
#include "dfb_led.h"
#endif


/********************** Internal macros declaration ************************/
#define FILE_QPBOC_PARAM    "qbpoc.log"
/********************** Internal structure declaration *********************/
// field EMV tag
static Def55EmvTag stIccFailTxnTagList[] =
{
	{0x9F26, DE55_OPT_SET},
	{0x9F27, DE55_OPT_SET},
	{0x9F10, DE55_OPT_SET},
	{0x9F37, DE55_OPT_SET},
	{0x9F36, DE55_OPT_SET},
	{0x95,   DE55_OPT_SET},
	{0x9A,   DE55_OPT_SET},
	{0x9C,   DE55_OPT_SET},
	{0x9F02, DE55_OPT_SET},
	{0x5F2A, DE55_OPT_SET},
	{0x82,   DE55_OPT_SET},
	{0x9F1A, DE55_OPT_SET},
	{0x9F03, DE55_OPT_SET},
	{0x9F33, DE55_OPT_SET},
	{0x9F34, DE55_OPT_SET},
	{0x9F35, DE55_OPT_SET},
	{0x9F1E, DE55_COND_SET},
	{0x84,   DE55_OPT_SET},
	{0x9F09, DE55_OPT_SET},
	{0x9F41, DE55_OPT_SET},
	{0x9F63, DE55_COND_SET},
	{0x9F74, DE55_COND_SET},
	{NULL_TAG_1},
};
/********************** Internal variables declaration *********************/
//struct qpboc_parameters  gstQpbocParamters;
/********************** Internal functions declaration *********************/
/********************** external reference declaration *********************/
extern int(*__ICCARD_EXCHANGE_APDU)(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdat);
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
void ledSetStatus(int status)
{
#ifdef CL_LED_SUPPORT
  switch (status) {
  	case LED_TRANS_OK:
      led_set_mode(LED_BLUE,   LED_ON);
      led_set_mode(LED_YELLOW, LED_ON);
      led_set_mode(LED_GREEN,  LED_ON);
      led_set_mode(LED_RED,    LED_OFF);
      break;
    case LED_IDLE:
      led_set_mode(LED_BLUE,   LED_BLINK);
      led_set_mode(LED_YELLOW, LED_OFF);
      led_set_mode(LED_GREEN,  LED_OFF);
      led_set_mode(LED_RED,    LED_OFF);
      break;
    case LED_ACTIV_CARD:
      led_set_mode(LED_BLUE,   LED_ON);
      led_set_mode(LED_YELLOW, LED_OFF);
      led_set_mode(LED_GREEN,  LED_OFF);
      led_set_mode(LED_RED,    LED_OFF);
      break;
    case LED_TRANS_HANDLING:
      led_set_mode(LED_BLUE,   LED_ON);
      led_set_mode(LED_YELLOW, LED_ON);
      led_set_mode(LED_GREEN,  LED_OFF);
      led_set_mode(LED_RED,    LED_OFF);
      break;
    /*case LED_REMOVE_CARD:
      led_set_mode(LED_BLUE, 	LED_ON);
      led_set_mode(LED_YELLOW, 	LED_ON);
      led_set_mode(LED_GREEN, 	LED_ON);
      led_set_mode(LED_RED,		LED_OFF);
      break;*/
    case LED_ONLINE_HANDLING:
      led_set_mode(LED_BLUE,   LED_ON);
      led_set_mode(LED_YELLOW, LED_ON);
      led_set_mode(LED_GREEN,  LED_BLINK);
      led_set_mode(LED_RED,    LED_OFF);
      break;
    case LED_TRANS_FAIL:
      led_set_mode(LED_BLUE,   LED_OFF);
      led_set_mode(LED_YELLOW, LED_OFF);
      led_set_mode(LED_GREEN,  LED_OFF);
      led_set_mode(LED_RED,    LED_ON);
      break;
    case LED_NOT_READY:
    default:
      led_set_mode(LED_BLUE,   LED_OFF);
      led_set_mode(LED_YELLOW, LED_OFF);
      led_set_mode(LED_GREEN,  LED_OFF);
      led_set_mode(LED_RED,		 LED_OFF);
      break;
  }
  //sysDelayMs(200);
#endif
}

void dispRspError(int start_line)
{
	int i;
	uint8_t tmpbuf[32], dispStr1[32];

	memset(dispStr1,0,sizeof(dispStr1));
	memset(tmpbuf,0,sizeof(tmpbuf));
	
	if( (memcmp(PosCom.szRespCode, "00", 2)==0) )
	{
		return;
	}
	
	for(i=0; strlen((char*)sRetErrInfo[i].ucRetCode)!=0; i++) 
	{
		if( memcmp(sRetErrInfo[i].ucRetCode, (char*)PosCom.szRespCode, 2)==0 )
			break;
	}

	if( strlen((char*)sRetErrInfo[i].ucRetCode)==0 )
	{
		strcpy((char *)dispStr1, (char *)"TRAN FAIL");
		memcpy(tmpbuf, "ERRCODE:", 7);
		sprintf((char *)(tmpbuf+7), "[%.2s]", PosCom.szRespCode);
		tmpbuf[11] = 0;
	}
	else
	{
		strcpy((char *)dispStr1, (char *)sRetErrInfo[i].szMessage);
		strcpy((char *)tmpbuf, (char *)sRetErrInfo[i].szOperate);
	}

	lcdDisplay(0, start_line, DISP_MEDIACY|DISP_CFONT, (char *)dispStr1);
	lcdDisplay(0, start_line+2, DISP_MEDIACY|DISP_CFONT, (char *)tmpbuf);
	return;
}

void   DisplayQpbocError(int iErrorCode)
{	
	uint8_t Buffer[255];
	uint8_t uiSize;
	char szTemp[255];

	if(iErrorCode != EMV_QPBOC_OK)
		ledSetStatus(LED_TRANS_FAIL);
	lcdClrLine(2,7);
	switch (iErrorCode)
	{
		case EMV_QPBOC_OK:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"SUCCESS");
			break;
 		case EMV_QPBOC_TRANSACTION_DECLIEN:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"REFUSE");
			break;
		case EMV_QPBOC_CARD_EXPIRED:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"REFUSE");
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"CARD EXPIRED");
			break;
		
		case EMV_QPBOC_APP_BLOCKED:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"APP LOCKED");
			break;
		case EMV_QPBOC_CONNECT_TRANSACTION_NEEDED:
			lcdDisplay(0,2, DISP_CFONT|DISP_MEDIACY, "TRANS CANCELED"); 
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"USE OTHERS");
			break;
			
		case EMV_QPBOC_NEED_PBOC_TRANSACTION:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"NFC PBOC TRANS");
			break;
		
		case EMV_QPBOC_NEED_RETRY:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"PLS RETRY");
			break;
		case EMV_QPBOC_CANCELED:
		case EMV_QPBOC_ICC_ERROR:
		case EMV_QPBOC_INVALID_DATA:
		case EMV_QPBOC_DATA_MISSING:
 		case EMV_QPBOC_DATA_DUPLICATE:
		case EMV_QPBOC_MEMORY_OVERFLOW:
		case EMV_QPBOC_MEMORY_NO_ENOUGH:
		case EMV_QPBOC_PROGRAMING_ERROR:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRANS CANCELED");
			break;
		case EMV_QPBOC_TRANSACTION_TERMINATED:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRANS REFUSE");
			break;
		default:
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRANS REFUSE");
			break;
	}

	memset(Buffer, 0, sizeof(Buffer));
	uiSize = sizeof(Buffer);
	memset(Buffer, 0, sizeof(Buffer));
	if (EMV_QPBOC_TRANSACTION_DECLIEN == iErrorCode)
	{
		if (0 == emv_get_data_element("\x9F\x5D", 2, &uiSize, Buffer))
		{
			memset(szTemp,0,sizeof(szTemp));
			PubBcd2Asc0(Buffer,uiSize,szTemp);
			PubAddHeadChars(szTemp,12,'0');
			memset(Buffer, 0, sizeof(Buffer));
			PubConvAmount(NULL,(uint8_t*)szTemp,2,Buffer,0);
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"BALANCE:%s", Buffer);
		}
	}
	else if (EMV_QPBOC_CARD_EXPIRED == iErrorCode || EMV_QPBOC_CONNECT_TRANSACTION_NEEDED == iErrorCode)
	{
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
	}
	else
	{
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
	}

	lcdFlip();
	QPBOCErrorBeep();
	PubWaitKey(1);
	if (EMV_QPBOC_TRANSACTION_DECLIEN != iErrorCode)
	{
		PubWaitKey(1);
		contactless_poweroff(imif_fd);		
	}
	else
	{
		PubWaitKey(3);
	}
	return;
}

uint8_t ProcQpbocTran(void)
{
	int	iRet;

	if( PosCom.bOnlineTxn && memcmp(PosCom.szRespCode, "00", 2)!=0 )
	{
		ledSetStatus(LED_TRANS_FAIL);
		return AfterCommProc();
	}
	
	if( PosCom.bOnlineTxn)
	{
		fileRemove(REVERSAL_FILE);
	}

	SaveEmvTranData();

	ledSetStatus(LED_TRANS_OK );
	sysDelayMs(750);
	iRet = AfterCommProc();
	return iRet;
}


uint8_t TransEcQuickSale(void)
{
	uint8_t	ucRet;
	
	while(1)
	{
		if (memcmp(PosCom.stTrans.sAmount,"\x00\x00\x00\x00\x00\x00",6) != 0)
		{
			break;
		}

		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "QPBOC");
		ucRet = AppGetAmount(9, TRAN_AMOUNT);
		if( ucRet!=OK )
		{
			return ucRet;
		}	
		break;
	}
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "QPBOC");
	ucRet = PosGetCard(CARD_PASSIVE);
	if( ucRet!=OK )
	{
		__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
		return ucRet;
	}

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");
	strcpy((char *)PosCom.stTrans.szCardUnit, "CUP");
	ucRet = OnlineCommProc(); 
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;
}

uint8_t GetCIDfromIAD(uint8_t *IAD)	//IAD:071E0103A02000010A010000000093C7F0F699
{
	if((IAD[4] & 0x30) == 0x10)		//TC
		return 0x40;
	else if((IAD[4] & 0x30) == 0x20)//ARQC
		return 0x80;
	else if((IAD[4] & 0xC0) == 0x40)//TC
		return 0x40;
	else
		return 0x00;
}

uint8_t  GetAndSetQpbocElement(void)
{
	int	iRet, iErrNo, i;
	uint8_t  uLen;
	char szBuffer[100];
	uint8_t tag9F10[60];


	iRet = emv_get_data_element("\x5F\x24", 2, &uLen, szBuffer);
	if( iRet==0 )
	{
		BcdToAsc0(PosCom.stTrans.szExpDate, (uint8_t*)szBuffer, 4);
	}

	emv_get_data_element("\x9F\x26", 2, &uLen, PosCom.stTrans.sAppCrypto);
	emv_get_data_element("\x8A", 1, &uLen, PosCom.stTrans.sAuthRspCode);
	emv_get_data_element("\x95", 1, &uLen, PosCom.stTrans.sTVR);
	emv_get_data_element("\x9B", 1, &uLen, PosCom.stTrans.sTSI);
	emv_get_data_element("\x9F\x36", 2, &uLen, PosCom.stTrans.sATC);
	PosCom.stTrans.bPanSeqNoOk = FALSE;
	iRet = emv_get_data_element("\x5F\x34", 2, &uLen, &PosCom.stTrans.ucPanSeqNo);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_RET_SUCCESS )
	{
		PosCom.stTrans.bPanSeqNoOk = TRUE;
	}
	emv_get_data_element("\x50", 1, &uLen, PosCom.stTrans.szAppLable);
	memset(PosCom.stTrans.sLoyaltAmount,0,sizeof(PosCom.stTrans.sLoyaltAmount));
	emv_get_data_element("\x9F\x5D", 2, &uLen, szBuffer);
	if (uLen <= 6)
		memcpy(PosCom.stTrans.sLoyaltAmount+(6-uLen),szBuffer,uLen);
	else
		memcpy(PosCom.stTrans.sLoyaltAmount,szBuffer,6);
	memset(szBuffer,0,sizeof(szBuffer));
	memset(PosCom.stTrans.szCardNo,0,sizeof(PosCom.stTrans.szCardNo));
	iErrNo = emv_get_data_element("\x5A", 1, &uLen, szBuffer);
	if (iErrNo == EMV_RET_SUCCESS)
	{
		PubBcd2Asc(szBuffer,uLen,(char*)PosCom.stTrans.szCardNo);
		PubTrimTailChars((char*)PosCom.stTrans.szCardNo,'F');
	}

	if (iErrNo != EMV_RET_SUCCESS)
	{
		memset(szBuffer,0,sizeof(szBuffer));
		iRet = emv_get_data_element("\x57", 1, &uLen, szBuffer);
		iErrNo = iRet?errno:0;
		if( iErrNo==EMV_RET_SUCCESS )
		{
			BcdToAsc0(PosCom.szTrack2, (uint8_t*)szBuffer, uLen*2);
			RemoveTailChars(PosCom.szTrack2, 'F');	// erase padded 'F' chars
			for(i=0; PosCom.szTrack2[i]!='\0'; i++)		// convert 'D' to '='
			{
				if( PosCom.szTrack2[i]=='D' )
				{
					PosCom.szTrack2[i] = '=';
					memcpy(PosCom.stTrans.szCardNo,PosCom.szTrack2,i);
					break;
				}
			}
		}
	}

	memset(szBuffer,0,sizeof(szBuffer));
	iRet = emv_get_data_element("\x4F", 1, &uLen, szBuffer);	// AID
	BcdToAsc0(PosCom.stTrans.szAID, (uint8_t*)szBuffer, uLen*2);
	RemoveTailChars(PosCom.stTrans.szAID, 'F');
	memcpy(PosCom.stTrans.szEntryMode, "072", 3);

	iRet = emv_get_data_element("\x9F\x10", 2, &uLen, tag9F10);
	iErrNo = iRet?errno:0;
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		memset(tag9F10,0,sizeof(tag9F10));
		emv_set_data_element("\x9F\x10", 2, 8, tag9F10);
	}

	iRet = emv_get_data_element("\x9F\x27", 2, &uLen, szBuffer);
	iErrNo = iRet?errno:0;
	if( iErrNo != EMV_RET_SUCCESS )	//qPBOC have not 9F27
	{
		memset(szBuffer,0,sizeof(szBuffer));
		szBuffer[0] = GetCIDfromIAD(tag9F10);
		emv_set_data_element("\x9F\x27", 2, 1, szBuffer);
	}
	
	iRet = emv_get_data_element("\x9F\x37", 2, &uLen, szBuffer);
	iErrNo = iRet?errno:0;
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		memset(szBuffer,0,sizeof(szBuffer));
		emv_set_data_element("\x9F\x37", 2, 4, szBuffer);
	}

	iRet = emv_get_data_element("\x95", 1, &uLen, szBuffer);
	iErrNo = iRet?errno:0;
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		memset(szBuffer,0,sizeof(szBuffer));
		emv_set_data_element("\x95", 1, 5, szBuffer);
	}

	return 0;
}

void     SaveIccFailureTranData(uint8_t ucIncStanFlag)
{
	int	    iCnt, iRet, iErrNo;
	uint8_t iTemp, iLength;
	uint8_t sBuff[100], sTemp[100];
		
	iCnt = 0;
	while(stIccFailTxnTagList[iCnt].uiTagNum != NULL_TAG_1)
	{
		memset(sBuff, 0, sizeof(sBuff));
		memset(sTemp, 0, sizeof(sTemp));
		if( stIccFailTxnTagList[iCnt].uiTagNum>0xFF )
		{
			sTemp[0] = (uint8_t)(stIccFailTxnTagList[iCnt].uiTagNum>>8);
			sTemp[1] = (uint8_t)(stIccFailTxnTagList[iCnt].uiTagNum);
			iTemp = 2;
		}
		else
		{
			sTemp[0] = (uint8_t)(stIccFailTxnTagList[iCnt].uiTagNum);
			iTemp = 1;
		}

		iRet = emv_get_data_element(sTemp, iTemp, &iLength, sBuff);
		iErrNo = iRet?errno:0;
		if( iErrNo!=EMV_RET_SUCCESS )
		{
			if (stIccFailTxnTagList[iCnt].uiTagNum == 0x9F74)
			{
				iCnt++;
				continue;
			}

			if (stIccFailTxnTagList[iCnt].uiTagNum == 0x9F63)
			{
				iCnt++;
				continue;
			}

			memset(sBuff,0,sizeof(sBuff));
			switch(stIccFailTxnTagList[iCnt].uiTagNum)
			{
			case 0x9F26:  iLength = 8; break;
			case 0x9F27:  iLength = 1; break;
			case 0x9F10:  iLength = 8; break;
			case 0x9F37:  iLength = 4; break;
			case 0x9F36:  iLength = 2; break;
			case 0x95:  iLength = 5; break;
			case 0x9A:  iLength = 6; break;
			case 0x9C:  iLength = 2; break;
			case 0x9F02:  iLength = 12; break;
			case 0x5F2A:  iLength = 3; break;
			case 0x82:  iLength = 2; break;
			case 0x9F1A:  iLength = 3; break;
			case 0x9F03:  iLength = 12; break;
			case 0x9F33:  iLength = 3; break;
			case 0x9F34:  iLength = 3; break;
			case 0x9F35:  iLength = 2; break;
			case 0x9F1E:  iLength = 8; break;
			case 0x84:  iLength = 8; break;
			case 0x9F09:  iLength = 2; break;
			case 0x9F41:  iLength = 2; break;
			case 0x9F63:  iLength = 16; break;
			case 0x9F74:  iLength = 6; break;
			default:
				break;
			}
		}

		emv_set_data_element(sTemp, iTemp, iLength, sBuff);
		iCnt++;
	}

	iCnt = 0;
	SetOnlineTranDE55(stIccFailTxnTagList, PosCom.stTrans.sIccData, &iCnt);
	PosCom.stTrans.iIccDataLen = (uint8_t)iCnt;
	strcpy((char*)PosCom.stTrans.szEntryMode,"072");
	if (PosCom.ucPinEntry)
	{
		strcpy((char*)PosCom.stTrans.szEntryMode,"071");
	}

	if (strlen((char*)PosCom.stTrans.szCardNo)==0)
	{
		strcpy((char*)PosCom.stTrans.szCardNo,"0000000000000000");
	}

	if (ucIncStanFlag)
	{
		PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
		IncreaseTraceNo();
	}
}

uint8_t SaveIccFailureFile(void)
{
	int		fd;
	int     ret;
	
	fd = fileOpen(ICC_FAILUR_TXN_FILE, O_RDWR|O_CREAT);
	if( fd<0 )
	{
		return E_MEM_ERR;
	}

	ret = fileSeek(fd, 0, SEEK_END);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_FILE_SEEK);
	}
	
	if( fileWrite(fd, (uint8_t *)&PosCom, COM_SIZE)!=COM_SIZE )
	{
		fileClose(fd);
		return E_MEM_ERR;
	}
	fileClose(fd);
	
	return OK;
}


uint8_t ReadIccFailureFile(uint32_t uiIndex)
{
	int		fd, ret;
	int     iFilesize = 0;
	
	iFilesize = fileSize(ICC_FAILUR_TXN_FILE);
	if (iFilesize < 0)
	{
		iFilesize = 0;
	}

	if (iFilesize <= uiIndex*COM_SIZE)
	{
		return E_MEM_ERR;
	}

	fd = fileOpen(ICC_FAILUR_TXN_FILE, O_RDWR);
	if( fd<0 )
	{
		return E_MEM_ERR;
	}

	ret = fileSeek(fd, (int)(uiIndex*COM_SIZE), SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
	if( fileRead(fd, (uint8_t *)&PosCom, COM_SIZE)!=COM_SIZE )
	{
		fileClose(fd);
		return E_MEM_ERR;
	}
	fileClose(fd);
	
	return OK;
}


uint8_t UpDateIccFailureFile(uint32_t uiIndex)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(ICC_FAILUR_TXN_FILE, O_RDWR);
	if( iLogFile<0 )
	{
		return E_MEM_ERR;
	}
	
	iRet = fileSeek(iLogFile, (int)(uiIndex * COM_SIZE), SEEK_SET);
	if( iRet<0 )
	{
		fileClose(iLogFile);
		return E_MEM_ERR;
	}
	
	iRet = fileWrite(iLogFile, (uint8_t *)&PosCom, COM_SIZE);
	fileClose(iLogFile);
	if( iRet!=COM_SIZE)
	{
		return E_MEM_ERR;
	}
	
	return OK;
}

uint8_t ViewIccFailureLog(void)
{
	int		iRet, iCurRec, iTotalRecNum, iAtcNum;
	int     iFilesize = 0;
	uint8_t ucRet;
	char    szBuffer[100];
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "QUERY FAIL TRANS");
	lcdFlip();
	iFilesize = fileSize(ICC_FAILUR_TXN_FILE);
	if (iFilesize <=0 )
	{
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"NO FAIL TRANS");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	iTotalRecNum = iFilesize/(sizeof(PosCom));
	iCurRec=0;
    while(iCurRec<iTotalRecNum)
	{
		memset(&PosCom, 0, sizeof(PosCom));
		ucRet = ReadIccFailureFile(iCurRec);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		lcdCls();
		if (PosCom.stTrans.ucSwipeFlag & CARD_INSERTED)
		{
			lcdDisplay(0,0,DISP_CFONT,(char*)"SALE");
		}
		else
		{
			lcdDisplay(0,0,DISP_CFONT,(char*)"QPBOC");
		}
		memset(szBuffer,0x00,sizeof(szBuffer));
		ConvBcdAmount(PosCom.stTrans.sAmount,(uint8_t*)szBuffer);
		lcdDisplay(0,2,DISP_CFONT,"AMOUNT:%s",szBuffer);
		memset(szBuffer,0x00,sizeof(szBuffer));		
		lcdDisplay(0,4,DISP_ASCII,(char*)PosCom.stTrans.szCardNo);
		iAtcNum = PubChar2Long(PosCom.stTrans.sATC,2,NULL);
		lcdDisplay(0,6,DISP_ASCII,"ATC:%d STAN:%d",iAtcNum,PosCom.stTrans.lTraceNo);
		lcdDisplay(0,7, DISP_ASCII, "%4.4s/%2.2s/%2.2s %2.2s:%2.2s:%2.2s", PosCom.stTrans.szDate,
			PosCom.stTrans.szDate+4,PosCom.stTrans.szDate+6,
			PosCom.stTrans.szTime,PosCom.stTrans.szTime+2,PosCom.stTrans.szTime+4);
		
		lcdFlip();
		iRet = PubWaitKey(60);
		if( iRet==KEY_CANCEL || iRet==KEY_INVALID )
		{
			break;
		}
		else if( iRet==KEY_DOWN || iRet== KEY_ENTER )
		{
			if (iCurRec < iTotalRecNum)
				iCurRec += 1;
			else
			{
				lcdClrLine(2,7);
				lcdDisplay(0,3,DISP_CFONT,"END OF RECORD");
				lcdFlip();
				PubBeepErr();
				PubWaitKey(5);
				return NO_DISP;
			}
		}
		else if( iRet==KEY_UP || iRet==KEY_CLEAR )
		{
			if (iCurRec > 1)
				iCurRec -= 1;
			else
				PubBeepErr();
		}
		else
		{
			PubBeepErr();
		}
	}
	
	return OK;
}

int contactless_poweredon(int ifd)
{
	static int iret_old = MIF_RET_NOCARD;
	int status = MIF_RET_NOCARD;
	int lasterror;

	emv_contactless_obtain_status(ifd, &status);
	if (NFC_OP_EXCHANGE_APDU == status)
	{
		iret_old = MIF_RET_NOCARD;
		return MIF_RET_SUCCESS;
	}
	else
	{
		emv_contactless_get_lasterror(ifd, &lasterror);
		if (NFC_ERR_ENODATA == lasterror){
			iret_old = MIF_RET_NOCARD;
			return MIF_RET_NOCARD;
		}else if (NFC_ERR_EMULTIHOP == lasterror){
			if (NFC_ERR_EMULTIHOP == iret_old){
				return MIF_RET_MULTICARD;
			}else{
				iret_old = MIF_RET_MULTICARD;
				return MIF_RET_NOCARD;
			}
		}else 
		{
			iret_old = MIF_RET_NOCARD;
			return MIF_RET_NOCARD;
			
		}
	}
	
}

int contactless_close(int icc_fd)
{
	int  iRet;
	
	if (icc_fd < 0)
	{
		return 0;
	}
	
	iRet = mif_close(icc_fd);
	return iRet;
}

int contactless_exchangeapdu(int icc_fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdata)
{
	int ret;
	ret = emv_contactless_exchange_apdu(icc_fd, sendlen, senddata, recvlen, recvdata);
#ifdef __DEBUG
	printf("cmd,len=%-3d,data: ",sendlen);
	for(icc_fd=0; icc_fd<sendlen;icc_fd++)
	  printf("%02x",*(uint8_t*)(senddata+icc_fd));
	
	printf("\nrsp,len=%-3d,data: ",*recvlen);
	for(icc_fd=0; icc_fd<*recvlen;icc_fd++)
	  printf("%02x",*(uint8_t*)(recvdata+icc_fd));
	printf("\n");
#endif
	return ret;
}

int contactless_open(void)
{
	return mif_open("/dev/nfc");
}

int contactless_poll(int icc_fd)
{
	return emv_contactless_active_picc(icc_fd); 
}

int contactless_poweroff(int fd)
{
	return emv_contactless_deactive_picc(fd); 
}

int TransQpbocQueAmt(void)
{
	int  iRet;
	uint32_t iSendLen, iRecvLen;
	uint8_t	szAmount[20], szBuffer[500];
	UINT8 szSendData[300], szRecvData[300];

	ledSetStatus(LED_TRANS_HANDLING);
	__ICCARD_EXCHANGE_APDU = contactless_exchangeapdu;
	
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "PLS WAITING...");
	lcdFlip();
	iRet = emv_qpboc_select_no_gpo(imif_fd);
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	if(iRet != 0)
	{
		ledSetStatus(LED_TRANS_FAIL);
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ ERR");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	iSendLen = iRecvLen = 0;  
    memset(szSendData, 0x00, sizeof(szSendData));
    memset(szRecvData, 0x00, sizeof(szRecvData));	
	memcpy(szSendData,"\x80\xCA\x9F\x79\x00",5);
    iSendLen = 5;	
	iRet = contactless_exchangeapdu(imif_fd,iSendLen,szSendData,&iRecvLen,szRecvData);
	if(iRet != ICC_SUCCESS || iRecvLen != 11
		|| szRecvData[iRecvLen-2] != 0x90
		|| szRecvData[iRecvLen-1] != 0x00)
	{
		ledSetStatus(LED_TRANS_FAIL);
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRANS FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x REED ERR",szRecvData[iRecvLen-2],szRecvData[iRecvLen-1]);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	if(iRecvLen != 11
		|| szRecvData[0] != 0x9F
		|| szRecvData[1] != 0x79
		|| szRecvData[2] != 0x06)
	{
		ledSetStatus(LED_TRANS_FAIL);
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,"TRANS FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",szRecvData[iRecvLen-2],szRecvData[iRecvLen-1]);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	PosCom.ucSwipedFlag = CARD_PASSIVE;
	memset(szAmount,0,sizeof(szAmount));
	memset(szBuffer,0,sizeof(szBuffer));
	PubBcd2Asc0(&szRecvData[3],6,(char *)szAmount);
	PubConvAmount(NULL,szAmount,2,szBuffer,0);

	ledSetStatus(LED_TRANS_OK );
	DispBalAmt(szBuffer,NULL);
	return 0;
}

#if 0
void ledSetStatus(int status)
{
	 switch(status){
	 case LED_IDLE:
		 led_set_mode(LED_BLUE, 	LED_ON);
		 led_set_mode(LED_YELLOW, 	LED_OFF);
		 led_set_mode(LED_GREEN, 	LED_OFF);
		 led_set_mode(LED_RED,		LED_OFF);
		break;
	 case LED_ACTIV_CARD:
		 led_set_mode(LED_BLUE, 	LED_ON);
		 led_set_mode(LED_YELLOW, 	LED_OFF);
		 led_set_mode(LED_GREEN, 	LED_OFF);
		 led_set_mode(LED_RED,		LED_OFF);
		break;
	 case LED_TRANS_HANDLING:
		 led_set_mode(LED_BLUE, 	LED_ON);
		 led_set_mode(LED_YELLOW, 	LED_ON);
		 led_set_mode(LED_GREEN, 	LED_OFF);
		 led_set_mode(LED_RED,		LED_OFF);
		break;
	 /*case LED_REMOVE_CARD:
		 led_set_mode(LED_BLUE, 	LED_ON);
		 led_set_mode(LED_YELLOW, 	LED_ON);
		 led_set_mode(LED_GREEN, 	LED_ON);
		 led_set_mode(LED_RED,		LED_OFF);
		break;*/
	case LED_ONLINE_HANDLING:
		 led_set_mode(LED_BLUE, 	LED_ON);
		 led_set_mode(LED_YELLOW, 	LED_ON);
		 led_set_mode(LED_GREEN, 	LED_BLINK);
		 led_set_mode(LED_RED,		LED_OFF);
		break;	
	 case LED_TRANS_OK:
		 led_set_mode(LED_BLUE, 	LED_ON);
		 led_set_mode(LED_YELLOW,	LED_ON);
		 led_set_mode(LED_GREEN, 	LED_ON);
		 led_set_mode(LED_RED,		LED_OFF);
		break;
	 case LED_TRANS_FAIL:
		 led_set_mode(LED_BLUE, 	LED_OFF);
		 led_set_mode(LED_YELLOW, 	LED_OFF);
		 led_set_mode(LED_GREEN, 	LED_OFF);
		 led_set_mode(LED_RED,		LED_ON);
		break;
	 case LED_NOT_READY:
	 default:
		 led_set_mode(LED_BLUE, 	LED_OFF);
		 led_set_mode(LED_YELLOW, 	LED_OFF);
		 led_set_mode(LED_GREEN, 	LED_OFF);
		 led_set_mode(LED_RED,		LED_OFF);
		break;
	 }
	 //sysDelayMs(200);
}
#endif

int qpboc_pre_process(int *iPreErrNo)
{
	int ierrorno = EMV_QPBOC_OK;
	struct emv_core_init Init;
	uint8_t TransType;
	uint32_t Amount, AmountOther;
	struct qpboc_parameters  gstQpbocParamters;

	PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
	PosCom.stTrans.ucQPBOCorPBOC = 0; //QPBOC
	
	TransType = 0x00;
	/*else if (PosCom.ucPBOCFlag==2 || CheckSupport(EC_QUICK_SALE)==FALSE ||stTemp.iTransNo==EC_TOPUP_CASH 
		|| stTemp.iTransNo==EC_TOPUP_NORAML || stTemp.iTransNo==EC_TOPUP_SPEC || stTemp.iTransNo==EC_VOID_TOPUP)
	{
		memcpy(gstQpbocParamters.m_TransactionProperty, "\x46\x00\x00\x80", 4);
	}
	else
	{*/
		memcpy(gstQpbocParamters.m_TransactionProperty, "\x26\x00\x00\x80", 4);//QPBOC
	//}
	gstQpbocParamters.m_StatusCheckSupported = 0;
	ierrorno = emv_qpboc_set_parameter(&gstQpbocParamters);
	if (EMV_QPBOC_OK != ierrorno)
	{
		//ledSetStatus(LED_TRANS_FAIL); 
		return E_SET_PARAM_FAIL;
	}	

	TransType = 0x00;
	emv_get_core_init_parameter(&Init);		
	switch(Init.transaction_type)
	{
	  case EMV_CASH:     TransType = 0x01; break;
	  case EMV_CASHBACK: TransType = 0x09; break;
	  case EMV_GOODS:    TransType = 0x00; break;
	  case EMV_SERVICE:  TransType = 0x00; break;
	  case EMV_INQUIRY:  TransType = 0x10; break;
	  case EMV_TRANSFER: TransType = 0x11; break;
	  case EMV_PAYMENT:  TransType = 0x12; break;
	  case EMV_ADMIN:    TransType = 0x13; break;
	}

	Amount = 0;
	AmountOther = 0;	
	if (memcmp(PosCom.stTrans.sAmount,"\x00\x00\x00\x00\x00\x00",6) == 0)
	{
		//  NEYPAY,EMV处理不处理金额
		if( PosCom.stTrans.iTransNo==POS_SALE )
		{
			;
		}
		else {
			if (0 != emv_get_amount(&Amount, NULL))
			return NO_DISP;
		}
//		if (0 != emv_get_amount(&Amount, NULL))
//			return NO_DISP;
	}
	else
	{
		Amount = PubBcd2Long(PosCom.stTrans.sAmount,6,NULL);
	}

	*iPreErrNo = emv_qpboc_pre_transaction(stPosParam.lNowTraceNo, TransType, Amount);
	if (*iPreErrNo!= EMV_QPBOC_OK)
	{
		//DisplayQpbocError(ierrorno);
		return NO_DISP;
	}
	__ICCARD_EXCHANGE_APDU = contactless_exchangeapdu;
	return 0;
}

void QPBOCErrorBeep(void)
{
	sysBeef(4,100);
	sysDelayMs(10);
	sysBeef(4, 100);
}



// end of file
