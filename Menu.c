
#include <openssl/sha.h>
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "emvtranproc.h"
#include "Menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


extern int gl_FirstRunSelApp;
extern int(*__ICCARD_EXCHANGE_APDU)(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdat);
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

uint8_t SelectEcFunc(void)
{
	int keySub;
	const char *Ec_menu[] = 
	{
		"1.QPBOC         ",
		"2.E-CASH        ",
		"3.LOAD          ",
		"4.BALANCE       ",
		"5.DETAIL        ",
		"6.EC REFUND     "
	};

	const char *Ec_Topup_menu[] = 
	{
		"1.CASH LOAD",
		"2.ACCT LOAD",
		"3.NON-ACCT LOAD",
		"4.VOID CASH LOAD"
	};

	keySub = DispMenu(Ec_menu, 6, 0, KEY1, KEY6, stPosParam.ucOprtLimitTime);
	if( keySub==KEY1 ) 
		stTemp.iTransNo = EC_QUICK_SALE;
	else if( keySub==KEY2 ) 
		stTemp.iTransNo = EC_NORMAL_SALE;
	else if( keySub==KEY3 ) 
	{
		keySub = DispMenu(Ec_Topup_menu, 4, 0, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		if( keySub==KEY1 )
			stTemp.iTransNo = EC_TOPUP_CASH;		
		else if( keySub==KEY2 )
			stTemp.iTransNo = EC_TOPUP_SPEC;
		else if( keySub==KEY3 )
			stTemp.iTransNo = EC_TOPUP_NORAML;
		else if( keySub==KEY4 )
			stTemp.iTransNo = EC_VOID_TOPUP;
		else
			return NO_DISP;
	}
	else if( keySub==KEY4 ) 
		stTemp.iTransNo = EC_BALANCE;
	else if( keySub==KEY5 ) 
		stTemp.iTransNo = EC_QUE_TXN;
	else if( keySub==KEY6 ) 
		stTemp.iTransNo = EC_REFUND;
	else 
		return NO_DISP;

	return OK;
}

uint8_t SelectInstalFunc(void)
{
	int keySub;
	const char *Instal_menu[] = 
	{
		"1.INSTALLMENT",
		"2.VOID INSTAL"
	};

	keySub = DispMenu(Instal_menu, 2, 0, KEY1, KEY2, stPosParam.ucOprtLimitTime);
	if( keySub==KEY1 ) 
		stTemp.iTransNo = POS_INSTALLMENT;
	else if( keySub==KEY2 ) 
		stTemp.iTransNo = POS_VOID_INSTAL;
	else 
		return NO_DISP;
	
	return OK;
}

uint8_t TransEcSale(int iTransType)
{
	uint8_t	ucRet;

	emv_pboc_ec_enable(1);

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "E-CASH SALE");
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = PosGetCard(CARD_INSERTED);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if (ChkIfBlkCard((char*)PosCom.stTrans.szCardNo))
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"BLACK LIST");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");
	// offline transaction fixed into cup
	strcpy((char *)PosCom.stTrans.szCardUnit, "CUP");
	return ProcEmvTran();
}

uint8_t TransEcTopup(int iTransType)
{
	uint8_t	ucRet,ucSwipedFlagBak; 
	char    szProcCode[7];
	int     iRet;
	uint8_t  szTrack2Bak[TRACK2_LEN+1];
	uint8_t  szTrack3Bak[TRACK3_LEN+1];
	uint8_t  szExpDate[5];

	lcdCls();
	if(iTransType == EC_TOPUP_CASH)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CASH LOAD");
	}
	else if(iTransType == EC_TOPUP_SPEC)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "ACCT LOAD");
	}
	else
	{
		PosCom.ucTransferFlag = TRUE;
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "NON-ACCT LOAD");
		ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		PosCom.stTrans.ucEcTopOldSwipeFlag = ucSwipedFlagBak = PosCom.ucSwipedFlag;
		strcpy((char*)PosCom.stTrans.szTranferCardNo,(char*)PosCom.stTrans.szCardNo);
		PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
		strcpy((char*)szTrack2Bak,(char*)PosCom.szTrack2);
		strcpy((char*)szTrack3Bak,(char*)PosCom.szTrack3);
		strcpy((char*)szExpDate,(char*)PosCom.stTrans.szExpDate);
		PosCom.ucTransferFlag = FALSE;
		PosCom.ucFallBack = FALSE;
		if (ucSwipedFlagBak == CARD_INSERTED)
		{
			if(iccDetect(ICC_USERCARD)==ICC_SUCCESS)
			{
				lcdClrLine(2,7);
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "   READ CARD OK ");
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "  RETIRE TARJETA");
				lcdFlip();
				while( iccDetect(ICC_USERCARD)==ICC_SUCCESS )
				{
					sysBeep();
					sysDelayMs(500);
				}
			}
		}
		else if (ucSwipedFlagBak == CARD_PASSIVE)
		{
			sysBeef(6, 200);
			lcdClrLine(2,7);
			lcdDisplay(0, 4, DISP_MEDIACY | DISP_CFONT, "RETIRE TARJETA");
			lcdFlip();
			contactless_poweroff(imif_fd);
			contactless_close(imif_fd);
			imif_fd = -1;
		}
	}
	
	ucRet = PosGetCard(CARD_INSERTED);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if(iTransType!=EC_TOPUP_CASH)
	{
		iRet = EnterPIN(0x80);
		if( iRet!=OK )
		{
			if (iRet != PED_RET_NOPIN)
			{
				DispEppPedErrMsg(iRet);
				return NO_DISP;
			}
		}
	}	
	
	memset(szProcCode,0,sizeof(szProcCode));
	if (iTransType == EC_TOPUP_CASH)
	{
		strcpy(szProcCode,"630000");
	}
	else if (iTransType == EC_TOPUP_NORAML)
	{
		strcpy((char*)szProcCode,"620000");
		strcpy((char*)PosCom.szTrack2,(char*)szTrack2Bak);
		strcpy((char*)PosCom.szTrack3,(char*)szTrack3Bak);
		strcpy((char*)PosCom.stTrans.szExpDate,(char*)szExpDate);
	}
	else
	{
		strcpy((char*)szProcCode,"600000");
	}

	SetCommReqField((uint8_t *)"0200", (uint8_t *)szProcCode);
	glSendPack.szField60[13] = 0;
	ucRet = ProcEmvTran();
	if (PosCom.ucSwipedFlag == CARD_PASSIVE)
	{
		sysBeef(6, 200);
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_MEDIACY | DISP_CFONT, "RETIRE TARJETA");
		lcdFlip();
		contactless_poweroff(imif_fd);  // This function will block until the card removed		
		contactless_close(imif_fd);
		imif_fd = -1;
	}
	return ucRet;
}

uint8_t TransEcVoidTopup(int iTransType)
{	
	uint8_t	ucRet, ucSwipeFlag;
		
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "VOID CASH LOAD");
	ucRet = CheckSupPwd(0);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetOldTraceNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = CheckData(PosCom.stTrans.lOldTraceNo, NULL);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	lcdClrLine(2, 7);
	if(PosCom.stTrans.iOldTransNo != EC_TOPUP_CASH )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT CASH");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NOT VOID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);	
		return NO_DISP;
	}

	ucSwipeFlag = CARD_INSERTED;
	ucRet = PosGetCard(ucSwipeFlag);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"170000");
	if( PosCom.stTrans.szSysReferNo[0]==0 )
	{
		sprintf((char *)glSendPack.szRRN, "%0*ld", LEN_RRN, 0L);
	}
	else
	{
		sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
	}
	sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d", PosCom.stTrans.lOldBatchNumber,
		PosCom.stTrans.lOldTraceNo);
	glSendPack.szField60[13] = 0;
	ucRet = ProcEmvTran();
	if (PosCom.ucSwipedFlag == CARD_PASSIVE)
	{
		sysBeef(6, 200);
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_MEDIACY | DISP_CFONT, "PLS REMOVE CARD");
		lcdFlip();
		contactless_poweroff(imif_fd);
		contactless_close(imif_fd);
		imif_fd = -1;
	}
	return ucRet;
}

uint8_t TransEcRefund(void)
{	
	uint8_t ucRet, ucSwipeMode;
	uint8_t buf[10];
	int iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC REFUND    ");
	ucRet = CheckSupPwd(0);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucSwipeMode = CARD_INSERTED|CARD_PASSIVE;	
	ucRet = PosGetCard(ucSwipeMode);
	if( ucRet!=OK )
	{
		__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
		return ucRet;
	}
	
	memset((char*)buf,0,sizeof(buf));
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC REFUND    ");	
	ucRet = GetOrignTxnDate();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT ORG TID:");
	iRet = PubGetString(NUM_IN,8,8,(uint8_t*)buf,stPosParam.ucOprtLimitTime,0);
	if( iRet != 0 )	
		return (E_TRANS_CANCEL);
	memset((char*)PosCom.stTrans.szInstalCode,0,sizeof(PosCom.stTrans.szInstalCode));
	strcpy((char*)PosCom.stTrans.szInstalCode,(char*)buf);
	
	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS INPUT:");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "ORG BATCH NO.:");
	iRet = PubGetString(NUM_IN,1,6,(uint8_t*)buf,stPosParam.ucOprtLimitTime,0);
	if( iRet != 0 )	
		return (E_TRANS_CANCEL);
	PosCom.stTrans.lOldBatchNumber = atol((char *)buf);	
		
	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS INPUT:");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "ORG VOUCHER NO.:");
	iRet = PubGetString(NUM_IN,1,6,(uint8_t*)buf,stPosParam.ucOprtLimitTime,0);
	if( iRet != 0 )	
		return (E_TRANS_CANCEL);
	PosCom.stTrans.lOldTraceNo = atol((char *)buf);
	sprintf((char *)PosCom.stTrans.szPrtOldPosSer, "%06ld", atol((char *)buf));
	
REINPUT:	
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	if( (stPosParam.lMaxRefundAmt!=0) &&
		(BcdToLong(PosCom.stTrans.sAmount, 6)>stPosParam.lMaxRefundAmt) )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INVALID AMOUNT");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "EXLIMITED");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);		
		return NO_DISP;
	}
	ucRet = AppConfirmAmount();
	if( ucRet==E_TRANS_CANCEL )
	{
		goto REINPUT;
	}
	else if( ucRet==E_TRANS_FAIL )
	{
		return NO_DISP;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC REFUND    ");	
	if( PosCom.stTrans.szCardUnit[0]==0 )
	{
		strcpy((char *)PosCom.stTrans.szCardUnit, "CUP");
	}
	
	SetCommReqField((uint8_t *)"0220", (uint8_t *)"200000");
	sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s", PosCom.stTrans.lOldBatchNumber,
		PosCom.stTrans.lOldTraceNo, PosCom.stTrans.szOldTxnDate);
	sprintf((char *)glSendPack.szField63, "%.3s", PosCom.stTrans.szCardUnit);
	ShortToByte(glSendPack.sField62, 8);
	sprintf((char *)&glSendPack.sField62[2], "%8.8s", PosCom.stTrans.szInstalCode);
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;
}

uint8_t TransEcBalance(void)
{
	int  iRet,ierrno;
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	uint8_t	sBalance[6], szBuff[20],sDispBuf1[30],sDispBuf2[30];
	uint32_t uiLen;
	char    Atr[500];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC BALANCE   ");
	while(1)
	{
		ShowGetCardScreen(CARD_INSERTED|CARD_PASSIVE,sDispBuf1,sDispBuf2);
		iRet = DetectCardEvent(CARD_INSERTED|CARD_PASSIVE,sDispBuf1,sDispBuf2,&ierrno);
		if( iRet ==CARD_KEYIN )
		{
			if( kbGetKey()==KEY_CANCEL )
			{
				return NO_DISP;
			}
		}
		else if( iRet==CARD_INSERTED )
		{
			break;
		}
		else if( iRet==CARD_PASSIVE )
		{
			iRet = TransQpbocQueAmt();
			return iRet;
		}
		else if(iRet == KEY_CANCEL)
			return NO_DISP;
	}

	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_ISO);
	iRet = iccPowerUp(ICC_USERCARD, &uiLen, Atr);
	if( iRet )
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ CARD ERR");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;		
	}
	icc_fd = iccGetSlotFd(ICC_USERCARD);//get fd
	gl_FirstRunSelApp = 0x01;
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC BALANCE   ");
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "WAITING...");
	lcdFlip();
	iRet = emv_application_select_no_gpo(icc_fd);
	if(iRet != 0)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ ERR");
		lcdFlip();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	memset((char*)&stSendApdu,0,sizeof(stSendApdu));
	memset((char*)&stRespApdu,0,sizeof(stRespApdu));	
	stSendApdu.CLA = 0x80;
	stSendApdu.INS = 0xCA;
	stSendApdu.P1 = 0x9F;
	stSendApdu.P2 = 0x79;
	stSendApdu.Lc = 0;
	stSendApdu.Le = 0;
	stSendApdu.LeFlag = 0x01;	
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu);
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"REMOVE CARD");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	if(stRespApdu.LenOut != 0x09
		|| stRespApdu.DataOut[0] != 0x9F
		|| stRespApdu.DataOut[1] != 0x79
		|| stRespApdu.DataOut[2] != 0x06)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"REMOVE CARD");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	
	PosCom.ucSwipedFlag = CARD_INSERTED;
	memset(sBalance,0,sizeof(sBalance));
	memset(szBuff,0,sizeof(szBuff));
	memcpy(sBalance,stRespApdu.DataOut+3,6);
	szBuff[0] = ' ';
	ConvBcdAmount(sBalance, &szBuff[1]);

	DispBalAmt(szBuff,NULL);	
	return (OK);	
}


uint8_t TransEcViewRecord(void)
{
	int  iRet, iCurRec, iAtcNum,ierrno;
	uint8_t     szBuffer[50], szAmout[20],szAmountBuf[25],szotherAmt[20], szCurCode[10], szTxnCurCode[10],sDispBuf1[30],sDispBuf2[30];
	uint8_t     szDataTime[30], szTxnAtc[4], szMerchantName[40], szTxnType[4];
	uint8_t     iTotalRecNum, iDatalen;
	uint32_t    uiLen;
	char        Atr[500];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC RECORD   ");	
	while(1)
	{
		ShowGetCardScreen(CARD_INSERTED,sDispBuf1,sDispBuf2);
		iRet = DetectCardEvent(CARD_INSERTED,sDispBuf1,sDispBuf2,&ierrno);
		if( iRet ==CARD_KEYIN )
		{
			if( kbGetKey()==KEY_CANCEL )
			{
				return NO_DISP;
			}
		}
		else if( iRet==CARD_INSERTED )
		{
			break;
		}
		else if(iRet == KEY_CANCEL)
			return NO_DISP;
	}

	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_ISO);
	iRet = iccPowerUp(ICC_USERCARD, &uiLen, Atr);
	if( iRet )
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ ERR");
		lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"REMOVE CARD");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	gl_FirstRunSelApp = 0x01;
	icc_fd = iccGetSlotFd(ICC_USERCARD);
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	
	iRet = emv_application_select_no_gpo(icc_fd);
	if(iRet != 0)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ ERR");
		lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"REMOVE CARD");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}

	iRet = emv_check_data_element("\x9F\x4D",2);
	if( iRet!= 0 )
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ ERR");
		lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"REMOVE CARD");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}

	iTotalRecNum = 0;
	iCurRec = 1;
	memset(szBuffer,0,sizeof(szBuffer));
	emv_get_data_element("\x9F\x4D",2,&iTotalRecNum,szBuffer);
	iTotalRecNum = szBuffer[1];
	while(1)
	{
		iRet = emv_read_log_record(icc_fd,iCurRec);
		if(iRet)
		{
			if((emv_get_last_iccard_sw() == 0x6A83) || (iRet == EMV_ERRNO_DATA))
			{
				lcdClrLine(2,7);
				lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"END OF RECORD");
				lcdFlip();
				PubWaitKey(3);
				iCurRec = 1;
				return NO_DISP;
			}
			else
			{
				lcdClrLine(2,7);
				lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ ERR");
				lcdFlip();
				PubWaitKey(5);
				iCurRec++;
				if (iCurRec > iTotalRecNum)
				{
					iCurRec = 1;
				}
				return NO_DISP;
			}			
		}
		
		lcdCls();
		sprintf((char*)szBuffer,"%d/%d",iCurRec,(int)iTotalRecNum);
		lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "TRAN LIST%8.8s",szBuffer);
		
		memset(szBuffer,0,sizeof(szBuffer));
		memset(szAmout,0,sizeof(szAmout));
		memset(szAmountBuf,0,sizeof(szAmountBuf));
		memset(szDataTime,0,sizeof(szDataTime));
		memset(szTxnAtc,0,sizeof(szTxnAtc));
		memset(szMerchantName,0,sizeof(szMerchantName));
		memset(szTxnType,0,sizeof(szTxnType));
		// time
		emv_get_log_item("\x9A",1, szBuffer, &iDatalen);
		memcpy(szDataTime,"20",2);
		PubBcd2Asc0(szBuffer,3,(char*)szDataTime+2);
		memset(szBuffer,0x00,sizeof(szBuffer));
	    emv_get_log_item("\x9F\x21",2, szBuffer, &iDatalen);
		PubBcd2Asc0(szBuffer,3,(char*)szDataTime+8);
		// count
		memset(szBuffer,0x00,sizeof(szBuffer));
	    emv_get_log_item("\x9F\x36",2, szTxnAtc, &iDatalen);		
		// merchant name
		memset(szMerchantName,0x00,sizeof(szMerchantName));
		emv_get_log_item("\x9F\x4E",2, szMerchantName, &iDatalen);	
		// tran type
		memset(szTxnType,0x00,sizeof(szTxnType));
		emv_get_log_item("\x9C",1, szTxnType, &iDatalen);
		// amount
		memset(szBuffer,0x00,sizeof(szBuffer));
		emv_get_log_item("\x9F\x02",2, szBuffer, &iDatalen);
		PubBcd2Asc0(szBuffer,6,(char*)szAmout);
		// other amount
		memset(szBuffer,0x00,sizeof(szBuffer));
		emv_get_log_item("\x9F\x03",2, szBuffer, &iDatalen);
		PubBcd2Asc0(szBuffer,6,(char*)szotherAmt);
		// currency code
		memset(szBuffer,0x00,sizeof(szBuffer));
		emv_get_log_item("\x9F\x1A",2, szBuffer, &iDatalen);
		PubBcd2Asc0(szBuffer,2,(char*)szCurCode);
		// tran currency code
		memset(szBuffer,0x00,sizeof(szBuffer));
		emv_get_log_item("\x5F\x2A",2, szBuffer, &iDatalen);
		PubBcd2Asc0(szBuffer,2,(char*)szTxnCurCode);

		PubConvAmount(NULL,szAmout,2,szAmountBuf,0);
		memset(szBuffer,0x00,sizeof(szBuffer));
		iAtcNum = PubChar2Long(szTxnAtc,2,NULL);
		switch (szTxnType[0])
		{
		case 0x01:
			sprintf((char*)szBuffer,"TYPE:CASH");
			break;
		case 0x09:
			sprintf((char*)szBuffer,"TYPE:CASH BACK");
			break;
		case 0x60:
		case 0x62:
		case 0x63:
			sprintf((char*)szBuffer,"TYPE:LOAD");
			break;
		default:
			sprintf((char*)szBuffer,"TYPE:SALE");
			break;
		}
		lcdClrLine(2,7);
		if (gstPosCapability.uiScreenType)
		{
			lcdDisplay(0,2,DISP_ASCII,"AMT:%s",szAmountBuf);
			lcdDisplay(0,3,DISP_ASCII,(char*)szBuffer);
			lcdDisplay(0,4,DISP_ASCII,"ATC: %02d",iAtcNum);
			lcdDisplay(0,5, DISP_ASCII, "%4.4s/%2.2s/%2.2s %2.2s:%2.2s:%2.2s", szDataTime,
			szDataTime+4,szDataTime+6,szDataTime+8,szDataTime+10,szDataTime+12);
			memset(szBuffer,0x00,sizeof(szBuffer));
			PubConvAmount(NULL,szotherAmt,2,szBuffer,0);
			lcdDisplay(0,6,DISP_ASCII,"OTHER AMT: %s",szBuffer);
			lcdDisplay(0,7,DISP_ASCII,"CUR CODE:%s",(char*)&szCurCode[1]);
			lcdDisplay(0,8,DISP_ASCII,"TXN CODE:%s",(char*)&szTxnCurCode[1]);
			lcdDisplay(0,9,DISP_ASCII,"MERCHANT NAME:");
			lcdDisplay(0,10,DISP_ASCII,"%s",szMerchantName);
			lcdFlip();
		}
		else
		{
			lcdDisplay(0,2,DISP_CFONT,"AMT:%s",szAmountBuf);
			lcdDisplay(0,4,DISP_CFONT,(char*)szBuffer);
			lcdDisplay(0,6,DISP_ASCII,"ATC: %02d",iAtcNum);
			lcdDisplay(0,7, DISP_ASCII, "%4.4s/%2.2s/%2.2s %2.2s:%2.2s:%2.2s", szDataTime,
				szDataTime+4,szDataTime+6,szDataTime+8,szDataTime+10,szDataTime+12);
			lcdFlip();

			while(1)
			{
				iRet = PubWaitKey(60);
				if( iRet==KEY_CANCEL || iRet==KEY_INVALID )
				{
					return 0;
				}
				else if( iRet==KEY_DOWN || iRet== KEY_ENTER )
				{
					lcdClrLine(2,7);
					memset(szBuffer,0x00,sizeof(szBuffer));
					PubConvAmount(NULL,szotherAmt,2,szBuffer,0);
					lcdDisplay(0,2,DISP_ASCII,"OTHER AMT: %s",szBuffer);
					lcdDisplay(0,3,DISP_ASCII,"CUR CODE:%s",(char*)&szCurCode[1]);
					lcdDisplay(0,4,DISP_ASCII,"TXN CODE:%s",(char*)&szTxnCurCode[1]);
					lcdDisplay(0,5,DISP_ASCII,"MERCHANT NAME:");
					lcdDisplay(0,6,DISP_CFONT,"%s",szMerchantName);
					lcdFlip();
					break;
				}
				else
				{
					PubBeepErr();
					continue;
				}
			}
		}

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
				lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"END OF RECORD");
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
				iCurRec = iTotalRecNum;
		}
		else
		{
			PubBeepErr();
		}	
	}

	return NO_DISP;	
}

uint8_t GetEcElementData(uint8_t *tag, uint8_t *len, uint8_t *val)
{
	int  iRet;
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	
	memset((char*)&stSendApdu,0,sizeof(stSendApdu));
	memset((char*)&stRespApdu,0,sizeof(stRespApdu));	
	stSendApdu.CLA = 0x80;
	stSendApdu.INS = 0xCA;
	stSendApdu.P1 = tag[0];
	stSendApdu.P2 = tag[1];
	stSendApdu.Lc = 0;
	stSendApdu.Le = 0;
	stSendApdu.LeFlag = 0x01;	
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu);
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		return NO_DISP;
	}
	
	if(stRespApdu.LenOut != 0x09
		|| stRespApdu.DataOut[0] != tag[0]
		|| stRespApdu.DataOut[1] != tag[1] 
		|| stRespApdu.DataOut[2] != 0x06)
	{
		return NO_DISP;
	}
		
	if (val != NULL)
	{
		*len = stRespApdu.DataOut[2];
		memcpy((char*)val,(char*)stRespApdu.DataOut+3,stRespApdu.DataOut[2]);
	}

	return (OK);
}

uint8_t TransInstallment(void)
{
	uint8_t ucRet;
	int     iRet;
	char    szBuffer[100];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   INSTALLMENT  ");
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED);
	else
		ucRet = PosGetCard(CARD_SWIPED);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	lcdClrLine(2, 7);
	memset(szBuffer,0,sizeof(szBuffer));	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT INSTAL NO:");
	iRet = PubGetString(NUM_IN,1,2,(uint8_t*)szBuffer,stPosParam.ucOprtLimitTime,0);
	if( iRet != 0 )	
		return (E_TRANS_CANCEL);
	
	PosCom.stTrans.ucInstallmentNum = atoi(szBuffer);

	lcdClrLine(2, 7);
	memset(szBuffer,0,sizeof(szBuffer));	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS INPUT");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "COMMODITY CODE:");
	iRet = PubGetString(NUM_IN|ALPHA_IN,1,30,(uint8_t*)szBuffer,stPosParam.ucOprtLimitTime,0);
	if( iRet != 0 )	
		return (E_TRANS_CANCEL);
	strcpy((char*)PosCom.stTrans.szInstalCode,szBuffer);

	ucRet = 1;
	while(ucRet)
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "FEE");
		lcdDisplay(0,2,DISP_CFONT,"1.ONLY ONE");
		lcdDisplay(0,4,DISP_CFONT,"2.INSTAL");
		lcdDisplay(0,6,DISP_CFONT,"CHOOSE 1 OR 2");
		lcdFlip();
		iRet = PubWaitKey(stPosParam.ucOprtLimitTime);
		switch(iRet)
		{
		case KEY1: 
			PosCom.stTrans.ucInstalPayMode = 0; 
			ucRet = 0;
			break;
		case KEY2: 
			PosCom.stTrans.ucInstalPayMode = 1; 
			ucRet = 0;
			break;
		case KEY_CANCEL:
		case KEY_TIMEOUT:
			return NO_DISP;
		default:
			PubBeepErr();
			break;
		}
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   INSTALLMENT  ");
	
	iRet = EnterPIN(0);
	if( iRet!=OK )
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;
	}
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");
	ShortToByte(glSendPack.sField62, 62);
	memset((char *)&glSendPack.sField62[2],' ',62);
	sprintf((char *)&glSendPack.sField62[2], "%02d", PosCom.stTrans.ucInstallmentNum);
	memcpy((char *)&glSendPack.sField62[4],PosCom.stTrans.szInstalCode,strlen((char*)PosCom.stTrans.szInstalCode));
	sprintf(szBuffer,"1%d",PosCom.stTrans.ucInstalPayMode);
	memcpy((char *)&glSendPack.sField62[34],szBuffer,strlen(szBuffer));
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;	
}

uint8_t TransVoidInstal(void)
{
	uint8_t	ucRet, ucSwipeFlag;
	int     iRet;
	
	if( stPosParam.ucEmvSupport==PARAM_OPEN )	
		ucSwipeFlag = CARD_SWIPED|CARD_INSERTED;
	else
		ucSwipeFlag = CARD_SWIPED|0x80;
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "VOID INSTALLMENT");
	
	ucRet = CheckSupPwd(0);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetOldTraceNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = CheckData(PosCom.stTrans.lOldTraceNo, NULL);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	lcdClrLine(2, 7);	
	if(PosCom.stTrans.iOldTransNo != POS_INSTALLMENT )
	{	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT INSTAL");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NOT VOID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);	
		return NO_DISP;
	}

	if( stTemp.iTransNo==POS_VOID_INSTAL && stPosParam.ucVoidSwipe!=PARAM_OPEN)
	{
		PosCom.ucSwipedFlag = CARD_KEYIN;
		PreDial();
	} 
	else
	{
		ucRet = PosGetCard(ucSwipeFlag);
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	
	iRet = EnterPIN(0);
	if( iRet!=OK )
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;
	}
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"200000");
	if( PosCom.stTrans.szSysReferNo[0]==0 )
	{
		sprintf((char *)glSendPack.szRRN, "%0*ld", LEN_RRN, 0L);
	}
	else
	{
		sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
	}
	sprintf((char *)glSendPack.szAuthCode, "%.*s", LEN_AUTH_CODE, PosCom.stTrans.szAuthNo);
	sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d", PosCom.stTrans.lOldBatchNumber,
		PosCom.stTrans.lOldTraceNo);
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;	
}
// end of file
