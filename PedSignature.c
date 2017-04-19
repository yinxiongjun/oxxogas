
#include "glbvar.h"
#include "tranfunclist.h"
#include "posparams.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#ifndef	_POS_TYPE_8110
typedef struct _tagDefield55ElecSigTag
{
	uint16_t	uiTagNum;
	uint8_t     *ini_ptr;
	uint8_t     len;
	uint8_t     type;
}Def55ElecSignTag;

void SetElecSignReqField(int iTransNo)
{
	int      iLength,iRet;
	uint8_t	*psICCData, *psTemp,ucTicket;
	uint32_t ulICCDataLen;
	uint8_t  szTransNameBuf[40];
	uint8_t  szTransDate[15];
	uint8_t  szReserve[13];
	uint8_t  szBatch[7];
	void GetElecSignTransName(int tCode, uint8_t *szName);
	uint8_t SetOnlineElecSignDE55(Def55ElecSignTag *pstList, uint8_t *psTLVData, int *piDataLen, int iTransNo);
	
	Def55ElecSignTag stOnlineElecSignTagList[] =
	{
		{0xFF00, stPosParam.szEngName, sizeof(stPosParam.szEngName), STRING_ITEM},
		{0xFF01, szTransNameBuf, sizeof(szTransNameBuf), STRING_ITEM},
		{0xFF02, stTransCtrl.szNowTellerNo, sizeof(int), BIN_ITEM},
		{0xFF03, PosCom.stTrans.szRecvBankId, 11, ASSCII_ITEM},
		{0xFF04, PosCom.stTrans.szIssuerBankId, 11, ASSCII_ITEM},
		{0xFF05, PosCom.stTrans.szExpDate, 2, BCD_ITEM},
		{0xFF06, szTransDate, 7, BCD_ITEM},
		{0xFF07, PosCom.stTrans.szAuthNo, 6, ASSCII_ITEM},
		{0xFF08, PosCom.stTrans.sTipAmount, 6, ASSCII_ITEM},
		{0xFF09, PosCom.stTrans.szCardUnit, 3, ASSCII_ITEM},
		{0xFF0A, PosCom.stTrans.szCurrencyCode, 3, ASSCII_ITEM},
		{0xFF0B, PosCom.stTrans.szMobilePhone, 6, BCD_ITEM},
		{0xFF22, PosCom.stTrans.szAID , 16, ASSCII_ITEM},
		{0xFF23, gstIccPrintData.sARQC, 8, ASSCII_ITEM},
		{0xFF24, PosCom.stTrans.sAfterBalanceAmt, 6, BCD_ITEM},
		{0xFF25, PosCom.stTrans.szTranferCardNo, 10, BCD_ITEM},
		{0xFF26, gstIccPrintData.sTerminalRand, 4, ASSCII_ITEM},
		{0xFF27, gstIccPrintData.sAip, 2, ASSCII_ITEM},
		{0xFF28, PosCom.stTrans.sTVR, 5, ASSCII_ITEM},
		{0xFF29, PosCom.stTrans.sTSI, 2, ASSCII_ITEM},
		{0xFF30, PosCom.stTrans.szAppLable, 16, ASSCII_ITEM},
		{0xFF31, PosCom.stTrans.szAppLable, 16, ASSCII_ITEM},
		{0xFF2A, PosCom.stTrans.sATC, 2, ASSCII_ITEM},
		{0xFF2B, gstIccPrintData.sIssureAppData, 33, ASSCII_ITEM},	
		{0xFF40, PosCom.stTrans.szIssuerResp,60,ASSCII_ITEM},
		{0xFF41, &PosCom.stTrans.ucInstallmentNum,1,HEX_ITEM},
		{0xFF42, PosCom.stTrans.sTipAmount, 6, ASSCII_ITEM},
		{0xFF43, PosCom.stTrans.szInstalCurrCode, 2, BCD_ITEM},
		{0xFF44, PosCom.stTrans.sPreAddAmount, 6, ASSCII_ITEM},
		{0xFF45, PosCom.stTrans.szInstalCode, sizeof(PosCom.stTrans.szInstalCode), STRING_ITEM},
		{0xFF46, &PosCom.stTrans.sTipAmount[1], 5, ASSCII_ITEM},
		{0xFF48, PosCom.stTrans.sPreAddAmount, 6, ASSCII_ITEM},
		{0xFF49, PosCom.stTrans.sAmount, 6, ASSCII_ITEM},
		{0xFF4A, PosCom.stTrans.sExtAmount, 6, ASSCII_ITEM}, 
		{0xFF4B, PosCom.stTrans.szInstalCode, 11, ASSCII_ITEM},
		{0xFF57, PosCom.stTrans.sLoyaltAmount, 6, ASSCII_ITEM},
		{0xFF60, PosCom.stTrans.szPrtOldPosSer, 3, BCD_ITEM},
		{0xFF61, szBatch, 3,BCD_ITEM},
		{0xFF62, PosCom.stTrans.szPrtOldHostSer, 6, BCD_ITEM},
		{0xFF63, PosCom.stTrans.szOldTxnDate, 2, BCD_ITEM},
		{0xFF64, PosCom.stTrans.szPrtOldAuthNo, 6, ASSCII_ITEM},
		{0xFF65, PosCom.stTrans.szInstalCode, 8, ASSCII_ITEM},
		{0xFF70, &ucTicket, 1, HEX_ITEM},
		{NULL_TAG_1},
	};

	ucTicket = stPosParam.ucTicketNum-'0';
	memset(szTransNameBuf,0,sizeof(szTransNameBuf));

	memset(glSendPack.szProcCode,0,sizeof(glSendPack.szProcCode)); 
	if(strlen((char*)PosCom.stTrans.szSettDate))
		sprintf((char *)glSendPack.szSettleDate, "%s", PosCom.stTrans.szSettDate);	
	sprintf((char *)glSendPack.szRRN, "%s", PosCom.stTrans.szSysReferNo);
	GetElecSignTransName(iTransNo,szTransNameBuf); 
	memset(szTransDate,0,sizeof(szTransDate));
	memcpy(szTransDate,PosCom.stTrans.szDate,8); 
	memcpy(szTransDate+8,PosCom.stTrans.szTime,6);
	
	memset((char*)&gstIccPrintData,0,sizeof(gstIccPrintData));
	psICCData = PosCom.stTrans.sIccData;
	ulICCDataLen = PosCom.stTrans.iIccDataLen;
	if (ulICCDataLen > 0)
	{
		for(psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
		{
			iRet = GetTlvValue(&psTemp, psICCData+ulICCDataLen-psTemp, SavePrintICCData, TRUE);
			if( iRet<0 )
			{	
				break;
			}
		}
	}

	memset(szReserve,0,sizeof(szReserve));
	memset(szBatch,0,sizeof(szBatch));
	sprintf((char *)szBatch,"%06d",PosCom.stTrans.lOldBatchNumber);	
	SetOnlineElecSignDE55(stOnlineElecSignTagList, &glSendPack.sICCData[2], &iLength, iTransNo);
	ShortToByte(glSendPack.sICCData, (uint16_t)iLength);
}

int InputMobiePhone(void)
{
	int iRet;
	uint8_t szMobilePhone[13];

	memset(szMobilePhone,0,sizeof(szMobilePhone));
	while(1)
	{
		lcdClrLine(2,7);
				
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT MOBILE NO:");
		iRet = PubGetString(NUM_IN|CARRY_IN,0,11,(uint8_t*)szMobilePhone,stPosParam.ucOprtLimitTime,0);
		if( iRet != 0 ) 
			return (E_TRANS_CANCEL);
		if (strlen((char*)szMobilePhone)==0 || strlen((char*)szMobilePhone) == 11)
		{
			if(strlen((char*)szMobilePhone) == 11)
			{
				PosCom.stTrans.szMobilePhone[0] = '0';
				sprintf((char*)&PosCom.stTrans.szMobilePhone[1],"%s",szMobilePhone);
			}
			break;
		}
	
		lcdClrLine(2,7);
		DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "ERROR LENGTH");
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "TRY AGAIN");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(3);
	}
	return 0;

}

uint8_t TransElecSignature(void)          
{	
	char     szBuffer[1024],szTmp[30];
	int      iRet;
	ST_TS_ELEC_SIGNATURE  stElecSignaInfo;

RE_PRINT:
	PosCom.stTrans.ucElecSignatureflag=0;
	memset(PosCom.stTrans.szElecfilename,0,sizeof(PosCom.stTrans.szElecfilename));	
	memset(&stElecSignaInfo,0,sizeof(stElecSignaInfo));
	if (gstPosCapability.uiPrnType == 'S')
		stElecSignaInfo.ullineTimes = 4; //Draw the line thickness		
	else
		stElecSignaInfo.ullineTimes = 3; //Draw the line thickness	
	stElecSignaInfo.ulTimeout = stPosParam.iElecSignTimeOut*1000;
	if (stElecSignaInfo.ulTimeout ==0){
		stElecSignaInfo.ulTimeout = 40000;
	}
	stElecSignaInfo.ucLanguage = LANGUAGE_ENG;          
	stElecSignaInfo.ucImageType = IMAGE_JBG;         
//	strcpy(stElecSignaInfo.szPrompt,"signature:");
	strcpy(stElecSignaInfo.szPrompt,"Firma digital:");
	memset(szBuffer,0,sizeof(szBuffer));
	memset(szBuffer,'0',16);
	if (PosCom.stTrans.iTransNo==ICC_OFFSALE || PosCom.stTrans.iTransNo==OFF_SALE || PosCom.stTrans.iTransNo==OFF_ADJUST      
		|| PosCom.stTrans.iTransNo==ADJUST_TIP)
	{
		sprintf(szBuffer,"%06d%06d0000",PosCom.stTrans.lBatchNumber,PosCom.stTrans.lTraceNo);
	}
	else
	{
		if (strlen((char*)PosCom.stTrans.szSettDate)){
			sprintf(szBuffer,"%4.4s%12.12s",PosCom.stTrans.szSettDate,PosCom.stTrans.szSysReferNo);
		}
		else{
			sprintf(szBuffer,"0000%12.12s",PosCom.stTrans.szSysReferNo);
		}
	}
	sprintf(stElecSignaInfo.szImagefileName,"%s/%s.jbg",STR_ELECSIGNATURE_FILE_PATH,szBuffer);
	PubAsc2Bcd(szBuffer,16,szTmp);
	PubXor(szTmp,szTmp+4,4);
	PubBcd2Asc0((uint8_t *)szTmp,4,stElecSignaInfo.szTxnInfoCode);
	kbFlush();
	if (gstPosCapability.uiScreenType)
	{
		iRet = GetModeElecSignature(&stElecSignaInfo,0); //internal
	}
	else
	{
		iRet = SetElecSignTimeOut(stPosParam.iElecSignTimeOut*1000);
		iRet = GetModeElecSignature(&stElecSignaInfo,1); //External
	}
	
	lcdCls();
	if (iRet != 0)
	{
//		PrtTranTicket(0);
		NetpayPrtTranTicket(0);
		return iRet;
	}

	// NETPAY
	if (fileSize(stElecSignaInfo.szImagefileName)>=1000)
	{
		lcdClrLine(2, 7);	
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "Firma digital demasiado larga");
		lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "Intente nuevamente");
		lcdFlip();
		FailBeep();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		kbGetKeyMs(2000);
		goto RE_PRINT;
//		PrtTranTicket(0);
//		NetpayPrtTranTicket(0);
//		return NO_DISP;
	}

//	iRet = InputMobiePhone();
	PosCom.stTrans.ucElecSignatureflag = TS_ELECSIGN_HAVE;
	strcpy((char *)PosCom.stTrans.szElecfilename,stElecSignaInfo.szImagefileName);
	if( PosCom.stTrans.iTransNo==CHECK_IN || PosCom.stTrans.iTransNo==CHECK_OUT)
	{
		UpdateCheckTranLog(PosCom.stTrans.uiTxnRecIndex,&PosCom.stTrans);
	}
	else
	{
		UpdateTranLog(PosCom.stTrans.uiTxnRecIndex,&PosCom.stTrans);
	}
		
	if (stTransCtrl.sTxnRecIndexList[PosCom.stTrans.uiTxnRecIndex] == INV_TXN_INDEX)
	{
		stTransCtrl.sTxnRecIndexList[PosCom.stTrans.uiTxnRecIndex] = TS_ELECSIGN_TXN;//need upload
	}
	else
	{
		stTransCtrl.sTxnRecIndexList[PosCom.stTrans.uiTxnRecIndex] |= TS_ELECSIGN_TXN; //need upload
	}
	SaveCtrlParam();

	PosCom.stTrans.ucElecSignatureflag |= TS_ELECSIGN_UP; // need upload
	if( PosCom.stTrans.iTransNo==CHECK_IN || PosCom.stTrans.iTransNo==CHECK_OUT)
	{
		UpdateCheckTranLog(PosCom.stTrans.uiTxnRecIndex,&PosCom.stTrans);
	}
	else
	{
		UpdateTranLog(PosCom.stTrans.uiTxnRecIndex,&PosCom.stTrans);
	}

	
//	PrtTranTicket(0);
	NetpayPrtTranTicket(0);
	if (PosCom.stTrans.iTransNo==ICC_OFFSALE || PosCom.stTrans.iTransNo==OFF_SALE    
		|| PosCom.stTrans.iTransNo==OFF_ADJUST  || PosCom.stTrans.iTransNo==ADJUST_TIP)
	{		
		return 0;
	}
	
	return OK;
}

void GetElecSignTransName(int tCode, uint8_t *szName)
{	
	switch(tCode) 
	{        
	case POS_SALE:
	case ICC_OFFSALE:
	case QPBOC_ONLINE_SALE:
		strcpy((char *)szName, "SALE");
		break;
	case POS_PREAUTH:
		strcpy((char *)szName, "PREAUTH");
		break;
	case POS_AUTH_CM:
		strcpy((char *)szName, "PREAUTH COMP");
		break;
	case POS_OFF_CONFIRM:
		strcpy((char *)szName, "PREAUTH OFFL");
		break;			
	case POS_PREAUTH_VOID:
		strcpy((char *)szName, "VOID PREAUTH");
		break;
	case POS_AUTH_VOID:
		strcpy((char *)szName, "VOID PRE-COMP");
		break;
	case POS_SALE_VOID:
		strcpy((char *)szName, "VOID SALE");
		break;
	case POS_PREAUTH_ADD:
		strcpy((char *)szName, "PREAUTH ADD");
		break;
	case POS_REFUND:
		strcpy((char *)szName, "CANCELACION");
		break;
	case OFF_SALE:
		strcpy((char *)szName, "OFF-SETTLE");
		break;
	case ADJUST_TIP:
	case OFF_ADJUST:
		strcpy((char *)szName, "OFF-ADJUST");
		break;	

	case EC_TOPUP_CASH:
		strcpy((char *)szName, "CASH LOAD");
		break;

	case EC_TOPUP_SPEC:
		strcpy((char *)szName, "ACCT LOAD");
		break;

	case EC_TOPUP_NORAML:
		strcpy((char *)szName, "NON-ACCT LOAD");
		break;

	case EC_VOID_TOPUP:
		strcpy((char *)szName, "VOID CASH");
		break;

	case EC_REFUND:
		strcpy((char *)szName, "EC REFUND");
		break;
		
	case POS_INSTALLMENT:
		strcpy((char *)szName, "INSTALLMENT");
		break;
		
	case POS_VOID_INSTAL:
		strcpy((char *)szName, "VOID INSTAL");
		break;

	default	:
		strcpy((char *)szName, "    ");
		break;
	}	
	return;
}

uint8_t SetElecSignDate(uint16_t iNum, int iTransNo)
{
	if (iNum==0xFF07)
	{
		if (strlen((char *)PosCom.stTrans.szAuthNo)==0)
		{
			return 1;
		}
		return 0;
	}
	else if (iNum==0xFF08)
	{
		if(iTransNo == ADJUST_TIP)
			return 0;
	}
	else if( iNum==0xFF0B)
	{
		if(strlen((char*)PosCom.stTrans.szMobilePhone))
			return 0;
		else
			return 1;
	}
	else if (iNum==0xFF09)
	{
		if (iTransNo == OFF_SALE || iTransNo == OFF_ADJUST || iTransNo == ADJUST_TIP 
			|| iTransNo == POS_OFF_CONFIRM || iTransNo == ICC_OFFSALE || iTransNo == POS_REFUND
			|| iTransNo == EC_REFUND)
			return 0;
	}
	else if (iNum>=0xFF20 && iNum<=0xFF22)
	{
		if( memcmp(PosCom.stTrans.szEntryMode, "05", 2)==0 )
		{
			return 0;
		}		
		if( iTransNo== QPBOC_ONLINE_SALE || (iTransNo==POS_SALE && PosCom.stTrans.ucQPBOCorPBOC==1))
		{
			return 0;
		}
		if( iTransNo==EC_TOPUP_CASH || iTransNo==EC_TOPUP_NORAML || iTransNo==EC_TOPUP_SPEC  || iTransNo == EC_VOID_TOPUP)
		{
			return 0;
		}
	}
	else if (iNum==0xFF23)
	{
		if( memcmp(PosCom.stTrans.szEntryMode, "05", 2)==0 && (iTransNo == POS_SALE || iTransNo == POS_PREAUTH 
			|| iTransNo == POS_INSTALLMENT))
		{
			return 0;
		}
		if( iTransNo== QPBOC_ONLINE_SALE || (iTransNo==POS_SALE && PosCom.stTrans.ucQPBOCorPBOC==1))
		{
			return 0;
		}
		if( iTransNo==EC_TOPUP_CASH || iTransNo==EC_TOPUP_NORAML || iTransNo==EC_TOPUP_SPEC || iTransNo==EC_VOID_TOPUP)
		{
			return 0;
		}
	}
	else if (iNum==0xFF24)
	{
		if( iTransNo==EC_TOPUP_CASH || iTransNo==EC_TOPUP_NORAML || iTransNo==EC_TOPUP_SPEC )
		{
			return 0;
		}
	}
	else if (iNum==0xFF25)
	{
		if(iTransNo==EC_TOPUP_NORAML)
			return 0;
	}
	else if (iNum>=0xFF26 && iNum<=0xFF2B)
	{
		if(iTransNo==ICC_OFFSALE )
		{
			return 0;			
		}
	}
	else if (iNum>=0xFF41 && iNum<=0xFF44)
	{
		if (iTransNo == POS_INSTALLMENT || iTransNo == POS_VOID_INSTAL)
			return 0;
	}
	else if (iNum==0xFF49)
	{
		if (iTransNo==POS_SALE && PosCom.stTrans.ucInstalPayMode == 0x03)
			return 0;
	}
	else if (iNum==0xFF4A)
	{
		if (memcmp(PosCom.stTrans.sExtAmount,"\x00\x00\x00\x00\x00\x00",6)==0)
			return 1;
		else
			return 0;
	}
	else if (iNum==0xFF60)
	{
		if (iTransNo == POS_AUTH_VOID || iTransNo == POS_SALE_VOID || iTransNo == EC_VOID_TOPUP 
			|| iTransNo == POS_VOID_INSTAL || iTransNo == POS_PREAUTH_VOID
			|| iTransNo == EC_REFUND )	
			return 0;
	}
	else if (iNum==0xFF61)
	{
		if(iTransNo == EC_REFUND)	
			return 0;
	}
	else if (iNum==0xFF62)
	{
		if (iTransNo == POS_REFUND )
			return 0;
	}
	else if (iNum==0xFF63)
	{
		if (iTransNo == POS_REFUND ||iTransNo == EC_REFUND)
			return 0;
	}
	else if (iNum==0xFF64)
	{
		if (iTransNo == POS_PREAUTH_VOID || iTransNo == POS_AUTH_VOID || iTransNo == POS_AUTH_CM)
			return 0;
	}
	else if (iNum==0xFF65)
	{
		if (iTransNo == EC_REFUND)
			return 0;
	}
	else
	{
		return 0;
	}
	return 1;

}

uint8_t SetOnlineElecSignDE55(Def55ElecSignTag *pstList, uint8_t *psTLVData, int *piDataLen, int iTransNo)
{
	int	i, iAmount;
	uint8_t	*psTemp, sBuff[128], iLength=0, ucRet;
	uint8_t  szIssuerBuffer[61];
	char    szAmount[13];

	memset(szAmount,0,sizeof(szAmount));
	memset(szIssuerBuffer,0,sizeof(szIssuerBuffer));
	
	*piDataLen = 0;
	psTemp     = psTLVData;
	
	for(i=0; pstList[i].uiTagNum!=NULL_TAG_1; i++)
	{		
		memset(sBuff, 0, sizeof(sBuff));
		if (pstList[i].uiTagNum==0xFF24)
		{
			iAmount = PubChar2Long((uint8_t *)pstList[i].ini_ptr,4,NULL);
			sprintf((char *)szAmount,"%012d",iAmount);
		}
		else if (pstList[i].uiTagNum==0xFF40)
		{
			memcpy(szIssuerBuffer,PosCom.stTrans.szIssuerResp,20);
			memcpy(szIssuerBuffer+20,PosCom.stTrans.szCenterResp,20);
			strcpy((char *)szIssuerBuffer+40,(char *)PosCom.stTrans.szRecvBankResp);
			if (strlen((char *)PosCom.stTrans.szRecvBankResp))
			{
				pstList[i].len = 40 + strlen((char *)PosCom.stTrans.szRecvBankResp);
			}
			else if (strlen((char *)PosCom.stTrans.szCenterResp))
			{
				pstList[i].len = 20 + strlen((char *)PosCom.stTrans.szCenterResp);
			}
			else
			{
				pstList[i].len = strlen((char *)PosCom.stTrans.szIssuerResp);
			}
		}
		ucRet = SetElecSignDate(pstList[i].uiTagNum,iTransNo);
		if (ucRet != 0)
		{
			iLength = 0;
			continue;		
		}

		switch(pstList[i].type)
		{
		case STRING_ITEM:
			str_convert(sBuff, (uint8_t *)pstList[i].ini_ptr, pstList[i].len);
			iLength = strlen((char *)sBuff);
			break;
		case ASSCII_ITEM:
			if (pstList[i].uiTagNum==0xFF2B)
			{
				memcpy(sBuff, (uint8_t *)pstList[i].ini_ptr,gstIccPrintData.ucIssureAppDatalen);
				pstList[i].len = gstIccPrintData.ucIssureAppDatalen;
				iLength = pstList[i].len;
			}
			else if (pstList[i].uiTagNum==0xFF40)
			{
				memcpy(sBuff, szIssuerBuffer,pstList[i].len);
				iLength = pstList[i].len;
			}
			else if (pstList[i].uiTagNum==0xFF44)
			{
				if (PosCom.stTrans.ucInstalPayMode == 0){
					memcpy((char *)sBuff, (char *)pstList[i].ini_ptr,pstList[i].len);
				}
				else{
					memcpy((char *)sBuff, (char *)PosCom.stTrans.sLoyaltAmount,pstList[i].len);
				}
				iLength = pstList[i].len;
			}
			else if (pstList[i].uiTagNum==0xFF22 || pstList[i].uiTagNum==0xFF0B)
			{
				PubAsc2Bcd((char *)pstList[i].ini_ptr,strlen((char *)pstList[i].ini_ptr),(char *)sBuff);
				iLength = strlen((char *)pstList[i].ini_ptr)/2;
			}
			else if (pstList[i].uiTagNum==0xFF30 || pstList[i].uiTagNum==0xFF31)
			{
				strcpy((char *)sBuff, (char *)pstList[i].ini_ptr);
				iLength = strlen((char *)pstList[i].ini_ptr);		
			}
			else
			{
				memcpy((char *)sBuff, (char *)pstList[i].ini_ptr,pstList[i].len);
				iLength = pstList[i].len;
			}
			break;
		case HEX_ITEM:
			memcpy(szIssuerBuffer, (uint8_t *)pstList[i].ini_ptr, pstList[i].len);
			iAmount = PubChar2Long(szIssuerBuffer,pstList[i].len,NULL);
			sprintf((char *)szIssuerBuffer,"%*d",2*pstList[i].len,iAmount);
			PubAsc2Bcd((char *)szIssuerBuffer,2*pstList[i].len,(char *)sBuff);
			iLength = pstList[i].len;
			break;
		case BIN_ITEM:			
			bin_convert(sBuff, (uint8_t *)pstList[i].ini_ptr, pstList[i].len);
			iLength = strlen((char *)sBuff);
			break;
		case BCD_ITEM:
			if (pstList[i].uiTagNum==0xFF24)
				bcd_convert(sBuff, (uint8_t *)szAmount, pstList[i].len);
			else if (pstList[i].uiTagNum==0xFF43)
			{
				sprintf((char *)szIssuerBuffer,"0%s",(uint8_t *)pstList[i].ini_ptr);
				bcd_convert(sBuff, szIssuerBuffer, pstList[i].len);
			}
			else if (pstList[i].uiTagNum==0xFF05)
			{
				bcd_convert(sBuff, (uint8_t *)pstList[i].ini_ptr, pstList[i].len);
			}
			else
				bcd_convert(sBuff, (uint8_t *)pstList[i].ini_ptr, pstList[i].len);
			
			if (pstList[i].uiTagNum==0xFF25)
			{
				if(iTransNo==EC_TOPUP_NORAML)
					strcpy((char *)pstList[i].ini_ptr,(char*)PosCom.stTrans.szCardNo);
				iLength = (strlen((char *)pstList[i].ini_ptr)+1)/2;
				if(strlen((char *)pstList[i].ini_ptr)%2)
				{
					memset(sBuff,0,sizeof(sBuff));
					sprintf((char *)szIssuerBuffer,"0%s",(uint8_t *)pstList[i].ini_ptr);
					bcd_convert(sBuff, szIssuerBuffer, 1+pstList[i].len);
				}
				else
				{
					bcd_convert(sBuff, (uint8_t *)pstList[i].ini_ptr, pstList[i].len);
				}
			}
			else if (pstList[i].uiTagNum==0xFF05)
			{
				if (strlen((char *)pstList[i].ini_ptr))
					iLength = pstList[i].len;
				else
					iLength = 0;
			}
			else
			{
				iLength = pstList[i].len;
			}
			break;
		default:
			break;
		}
		if (iLength!=0)
		{
			MakeTlvString(pstList[i].uiTagNum, sBuff, iLength, &psTemp);
		}
	}
	*piDataLen = (psTemp-psTLVData);
	
	return OK;
}

int jbg_encode_buffer(uint8_t *szPName,uint8_t *psImageBuffer,int *piDataLen)
{
	int iFd, iLen, iRet;

	*piDataLen = 0;
	iFd = fileOpen((char *)szPName, O_RDWR);
	if( iFd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	iLen = fileSize((char *)szPName);
	if ((iLen <= 0) || (iLen > 2000))
	{
		fileClose(iFd);
		return NO_DISP;
	}
	
	iRet = fileRead(iFd, psImageBuffer, iLen);
	fileClose(iFd);
	if( iRet!=iLen )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	*piDataLen = iLen;
	return 0;
}

void   ClearElecSignaturefile(void)
{
	fileRemoveDir(STR_ELECSIGNATURE_FILE_PATH);
}


uint8_t SendCheckOffLineElecSignTrans(uint8_t flag, uint8_t szSettleBatch)
{
	uint8_t		ucRet;
	int		iRecNo, iTransNoBak,iRet,iLen,iNum;
	NEWPOS_LOG_STRC	stLog;
	uint32_t uiStatus;
	int      iCurUploadNum = 1;
	char     szBuffer[40];
//	uint8_t  sDestBuf[2000];
	char     szField63BuffTemp[400];

	if ((szSettleBatch ==0 || szSettleBatch == 4)&&(ChkIfSaveLog()))
	{
		iNum = stTransCtrl.iCheckTransNum - 1;
	}
	else
	{
		iNum = stTransCtrl.iCheckTransNum;
		
	}
	for(iRecNo=0; iRecNo<iNum; iRecNo++)
	{
		if (flag)
		{
			if( kbGetKeyMs(150) == KEY_CANCEL )
			{
				return NO_DISP;
			}
		}

		if (stTransCtrl.sTxnRecIndexList[iRecNo] == INV_TXN_INDEX)
		{
			continue;
		}
		if (!(stTransCtrl.sTxnRecIndexList[iRecNo] & (TS_ELECSIGN_TXN|TS_ELECSIGN_FAIL_TXN)))
		{
			continue;
		}
		if (szSettleBatch==4 && (stTransCtrl.sTxnRecIndexList[iRecNo]&(TS_TXN_OFFLINE|TS_ICC_OFFLINE)))
		{
			continue;
		}
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iRecNo, &stLog);
		if( ucRet )
		{
			return ucRet;
		}
		if (!(stLog.ucElecSignatureflag & TS_ELECSIGN_UP))
		{
			continue;
		}
		if (szSettleBatch == 0 || szSettleBatch == 1 || szSettleBatch == 4)
		{
			if (stTransCtrl.sTxnRecIndexList[iRecNo] & TS_ELECSIGN_FAIL_TXN) 
				continue;
		}
		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		iTransNoBak = PosCom.stTrans.iTransNo;
		PosCom.stTrans.iOldTransNo = iTransNoBak;	
		stTemp.iTransNo = POS_ELEC_SIGN;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		PosCom.stTrans.TransFlag=POS_ELEC_SIGN;
		DispTransName();
		SetCommReqField((uint8_t *)"0820", (uint8_t *)"000000");
//		sprintf((char *)&glSendPack.szField60[8], "800");
//		if (iTransNoBak==EC_TOPUP_NORAML)
//			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szTranferCardNo);
		iRet = jbg_encode_buffer(PosCom.stTrans.szElecfilename,&glSendPack.sICCData[2],&iLen);
		if (iRet != 0)
		{
			continue;
		}

		memset(glSendPack.szPanSeqNo,0,sizeof(glSendPack.szPanSeqNo));
		memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
		memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
		memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
		memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));

		GetPosTime(stTemp.szDate, stTemp.szTime);		
		strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
		strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
		strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
		strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

		sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
									
		strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

		NetpayDealFeild63((uint8_t *)szField63BuffTemp);
		memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
		strcpy((char*)glSendPack.szField63,szField63BuffTemp);

		ShortToByte(glSendPack.sICCData, iLen);
/*		nMHexAsciiToBin(glSendPack.sField50, strlen(glSendPack.sField50)*2, sDestBuf);

	    if( JbigStr2bmp("./chenhaishun.bmp", sDestBuf, strlen(glSendPack.sField50)) )
	    {
			return -1;
	    }
*/

//		SetElecSignReqField(iTransNoBak);

		if (szSettleBatch !=0)
		{
			lcdClrLine(2,7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "Enviando firma digital..");
			sprintf(szBuffer,"Subida %d ",iCurUploadNum);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, szBuffer, szBuffer);
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "Por favor espere ..");
			lcdFlip();
			sysDelayMs(500);
			iCurUploadNum++;
		}
		PosCom.bOnlineTxn = FALSE;
		ucRet = SendRecvPacket();
		if ((ucRet==OK)&&( memcmp(PosCom.szRespCode, "00", 2)==0)/* || memcmp(PosCom.szRespCode, "94", 2)==0 )*/)
		{
			uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
			uiStatus &= ~TS_ELECSIGN_TXN;
			uiStatus &= ~TS_ELECSIGN_FAIL_TXN;
			UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);
		}
		else
		{
			if(ucRet == E_ERR_CONNECT ||ucRet ==  E_MAKE_PACKET || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			
			if(strlen((char *)PosCom.szRespCode)==2)
				dispRetCode(2);

			if (PosCom.bOnlineTxn)
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex] & 0xF0;
				uiStatus = uiStatus>>4;
				uiStatus++; 
				uiStatus = uiStatus<<4;
				uiStatus |= (stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex]&0xffffff0f);				
				uiStatus |= TS_ELECSIGN_FAIL_TXN;		
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
			}
		}
		if (szSettleBatch ==0)
		{
			break; 
		}
	}	//for

	return OK;
}


uint8_t SendOffLineElecSignTrans(uint8_t flag, uint8_t szSettleBatch)
{
	uint8_t		ucRet;
	int		iRecNo, iTransNoBak,iRet,iLen,iNum;
	NEWPOS_LOG_STRC	stLog;
	uint32_t uiStatus;
	int      iCurUploadNum = 1;
	char     szBuffer[40];
//	uint8_t  sDestBuf[2000];
	char     szField63BuffTemp[400];

	if ((szSettleBatch ==0 || szSettleBatch == 4)&&(ChkIfSaveLog()))
	{
		iNum = stTransCtrl.iTransNum - 1;
	}
	else
	{
		iNum = stTransCtrl.iTransNum;
		
	}
	for(iRecNo=0; iRecNo<iNum; iRecNo++)
	{
		if (flag)
		{
			if( kbGetKeyMs(150) == KEY_CANCEL )
			{
				return NO_DISP;
			}
		}

		if (stTransCtrl.sTxnRecIndexList[iRecNo] == INV_TXN_INDEX)
		{
			continue;
		}
		if (!(stTransCtrl.sTxnRecIndexList[iRecNo] & (TS_ELECSIGN_TXN|TS_ELECSIGN_FAIL_TXN)))
		{
			continue;
		}
		if (szSettleBatch==4 && (stTransCtrl.sTxnRecIndexList[iRecNo]&(TS_TXN_OFFLINE|TS_ICC_OFFLINE)))
		{
			continue;
		}
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iRecNo, &stLog);
		if( ucRet )
		{
			return ucRet;
		}
		if (!(stLog.ucElecSignatureflag & TS_ELECSIGN_UP))
		{
			continue;
		}
		if (szSettleBatch == 0 || szSettleBatch == 1 || szSettleBatch == 4)
		{
			if (stTransCtrl.sTxnRecIndexList[iRecNo] & TS_ELECSIGN_FAIL_TXN) 
				continue;
		}
		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		iTransNoBak = PosCom.stTrans.iTransNo;
		PosCom.stTrans.iOldTransNo = iTransNoBak;	
		stTemp.iTransNo = POS_ELEC_SIGN;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		PosCom.stTrans.TransFlag=POS_ELEC_SIGN;
		DispTransName();
		SetCommReqField((uint8_t *)"0820", (uint8_t *)"000000");
//		sprintf((char *)&glSendPack.szField60[8], "800");
//		if (iTransNoBak==EC_TOPUP_NORAML)
//			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szTranferCardNo);
		iRet = jbg_encode_buffer(PosCom.stTrans.szElecfilename,&glSendPack.sICCData[2],&iLen);
		if (iRet != 0)
		{
			continue;
		}

		memset(glSendPack.szPanSeqNo,0,sizeof(glSendPack.szPanSeqNo));
		memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
		memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
		memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
		memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));

		GetPosTime(stTemp.szDate, stTemp.szTime);		
		strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
		strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
		strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
		strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

		sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
									
		strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

		NetpayDealFeild63((uint8_t *)szField63BuffTemp);
		memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
		strcpy((char*)glSendPack.szField63,szField63BuffTemp);

		ShortToByte(glSendPack.sICCData, iLen);
/*		nMHexAsciiToBin(glSendPack.sField50, strlen(glSendPack.sField50)*2, sDestBuf);

	    if( JbigStr2bmp("./chenhaishun.bmp", sDestBuf, strlen(glSendPack.sField50)) )
	    {
			return -1;
	    }
*/

//		SetElecSignReqField(iTransNoBak);

		if (szSettleBatch !=0)
		{
			lcdClrLine(2,7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "Enviando firma digital..");
			sprintf(szBuffer,"Subida %d ",iCurUploadNum);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, szBuffer, szBuffer);
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "Por favor espere ..");
			lcdFlip();
			sysDelayMs(500);
			iCurUploadNum++;
		}
		PosCom.bOnlineTxn = FALSE;
		ucRet = SendRecvPacket();
		if ((ucRet==OK)&&( memcmp(PosCom.szRespCode, "00", 2)==0)/* || memcmp(PosCom.szRespCode, "94", 2)==0 )*/)
		{
			uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
			uiStatus &= ~TS_ELECSIGN_TXN;
			uiStatus &= ~TS_ELECSIGN_FAIL_TXN;
			UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);
		}
		else
		{
			if(ucRet == E_ERR_CONNECT ||ucRet ==  E_MAKE_PACKET || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			
			if(strlen((char *)PosCom.szRespCode)==2)
				dispRetCode(2);

			if (PosCom.bOnlineTxn)
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex] & 0xF0;
				uiStatus = uiStatus>>4;
				uiStatus++; 
				uiStatus = uiStatus<<4;
				uiStatus |= (stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex]&0xffffff0f);				
				uiStatus |= TS_ELECSIGN_FAIL_TXN;		
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
			}
		}
		if (szSettleBatch ==0)
		{
			break; 
		}
	}	//for

	return OK;
}

uint8_t TrickleFeedElecSignTxn(uint8_t ucAllowBreak,uint8_t szSettleBatch)
{
	uint8_t	ucRet;

	if ( (GetElecSignTxnNums()<=0 && GetElecSignFailTxnNums()<=0) ||
		(GetCheckElecSignTxnNums()<=0 && GetElecSignFailTxnNums()<=0) )
		return 0;

	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));	
	memcpy(&BackPosCom, &PosCom, COM_SIZE);
	ucRet = SendOffLineElecSignTrans(ucAllowBreak,szSettleBatch);
	ucRet = SendCheckOffLineElecSignTrans(ucAllowBreak,szSettleBatch);
	memcpy(&PosCom, &BackPosCom, COM_SIZE);
	memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));
	stTemp.iTransNo = PosCom.stTrans.iTransNo;
	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	return ucRet;
}


#endif
