
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "st8583.h"
#include "posappapi.h"
#include "emvtranproc.h"
#include "emvapi.h"
#include "otherTxn.h"
#include "Menu.h"

extern int  EMVCoreInit(void);
uint8_t SaleTranGetData(void);
uint8_t SaleVoidGetData(void);
uint8_t RefundTranGetData(void);
uint8_t AuthComfirmGetData(void);
uint8_t PreAuthVoidGetData(void);
uint8_t OffSaleGetData(void);
uint8_t AdjustTranGetData(void);
uint8_t PreAuthGetData(void);

static int    iAidNum, iCapkNum;
static struct terminal_aid_info	*pstCurApp;			
static struct terminal_aid_info	*pstAppList;		
static TermAidList	*pstCurTermAid;		
static struct issuer_ca_public_key	*pstCurCapk;		
static CapkInfo		*pstCapkInfoList;	
static uint8_t		bRidOk, bKeyIDOk, bExpiryOk,bRecvNOK;

// 通信响应测试
uint8_t EchoTest(void)
{
	uint8_t	ucRet,sTempBuff[60];
	char szField63BuffTemp[400];
	
	lcdCls();
	DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_HFONT16, NULL, "PRUEBA COMUNICACION");
	lcdFlip();
	
	SetCommReqField((uint8_t *)"0800", (uint8_t *)"990000");
	sprintf((char *)glSendPack.szNII, "0003");  
	memset(glSendPack.szMerchantID,0,sizeof(glSendPack.szMerchantID));
	
	ShortToByte(glSendPack.sField62, strlen(stPosParam.szUser));
	sprintf((char *)sTempBuff,"%s",stPosParam.szUser);
	sprintf((char*)glSendPack.sField62+2,"%s",(char*)sTempBuff);

	NetpayDealFeild63((uint8_t*)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);	
	ucRet = SendRecvPacket();
	if( ucRet!=OK )
	{
		lcdClrLine(2,7);
		DispMulLanguageString(0, 6, DISP_ASCII|DISP_MEDIACY, NULL, "CONEXION FALLO – 1001");
		DispMulLanguageString(0, 7, DISP_ASCII|DISP_MEDIACY, NULL, "PRESIONE ENTER");
		lcdFlip();
		FailBeep();
		kbGetKeyMs(2000);
		return ucRet;
	}

	if( memcmp(PosCom.szRespCode, "00", 2)==0 )
    {
    	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		lcdClrLine(2,7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PRUEBA COMUNICACION");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "PRUEBA EXITOSA");
		DispMulLanguageString(0, 8, DISP_CFONT|DISP_MEDIACY, NULL, "PRESIONE ENTER");
		lcdFlip();
		OkBeep();
		kbGetKeyMs(1500);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		return NO_DISP;
    }

	return NO_DISP;
}


uint8_t PaymentwithCard(void)
{
	uint8_t	ucRet,sTempBuff[60];
	char szField63BuffTemp[400]; 
	char szField120BuffTemp[999];
//	uint8_t BankReference[22] = {0};
	char szAmount[13] = {0};
	int  iKey;
	long szLongTax;
	long szLongAmount;

	ucRet = PayWithCardBalanceCheck();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	
	stTemp.iTransNo = POS_SALE;
	PosCom.stTrans.iTransNo = POS_SALE;
	PosCom.stTrans.TransFlag = PAYMENT_WITH_CARD;
	while(1)
	{
		ucRet = AppGetAmount(10, TRAN_AMOUNT);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		PubBcd2Asc0(PosCom.stTrans.sAmount,6,szAmount);
		szLongTax = atol(PosComconTrol.szCommissionPercen);
		szLongAmount = atol(szAmount);
		PosComconTrol.szTotalTaxAmount =(szLongAmount * szLongTax) / 10000+ (szLongAmount * szLongTax) / 10000 *0.16;
		PosComconTrol.szTotalAmount = szLongAmount + PosComconTrol.szTotalTaxAmount;
	
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_ASCII, NULL, "PAGO A NETPAY CON TARJETA");
		lcdDisplay(0, 4, DISP_HFONT16, "Monto capturado:    $ %ld.%02ld",szLongAmount/100,szLongAmount%100);
		lcdDisplay(0, 5, DISP_HFONT16, "Comision + iva:         $ %ld.%02ld",PosComconTrol.szTotalTaxAmount/100,PosComconTrol.szTotalTaxAmount%100);
		lcdDisplay(0, 6, DISP_HFONT16, "Total:                          $ %ld.%02ld",PosComconTrol.szTotalAmount/100,PosComconTrol.szTotalAmount%100);
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL ) {
			return NO_DISP;
		}
		else if (iKey == KEY2 ) {
			continue;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			break;
		}
		
	}

	ucRet = SaleTranGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
//	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     PAYMENT WITH CARD      ");
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"776606");
	sprintf((char *)glSendPack.szTranAmt, "%012ld", PosComconTrol.szTotalAmount);	
	AscToBcd(PosCom.stTrans.sAmount,glSendPack.szTranAmt,12);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	strcpy((char *)glSendPack.szNII,"0003");  //newpos 固定0003
	sprintf((char*)glSendPack.szRRN,"%s%s",(char*)(PosCom.stTrans.szDate+2),(char*)PosCom.stTrans.szTime);
	
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	sprintf((char *)glSendPack.szCurrencyCode, "484"); // 货币代码

	NetpayPaywithCardDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);	

	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

/*	if( memcmp(PosCom.szRespCode, "00", 2)!=0 )
    {
 		dispRetCode(1);
	    return NO_DISP;
    }*/

	return OK;
}


// pay with balance
uint8_t PayWithCardBalanceCheck(void)
{
	uint8_t	ucRet;
	int     iRet;
	uint8_t sTempBuff[60];
	char szField63BuffTemp[400];
	char C0[40];

	lcdCls();
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"776608");
	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack2,0,sizeof(glSendPack.szTrack2));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60));
	memcpy(glSendPack.szTranAmt,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",12);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	sprintf((char *)glSendPack.szEntryMode, "0001");
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id


	ShortToByte(glSendPack.sField62, strlen(stPosParam.szUser));
	sprintf((char *)sTempBuff,"%s",stPosParam.szUser);
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	iRet = ProcPayWithCardBalanceCheck(C0);
	if( iRet!=OK )
	{
		return iRet;
	}

	return OK;
}

uint8_t ComfirAmount(uint8_t *szTransAmount)
{
	int iKey;
	char szAmount[20];
	char szdispAmount[30] = {0};
	char szTitle[50] = {0};

	PubBcd2Asc0(szTransAmount,6,szAmount);
	if( memcmp( szTransAmount,"\x00\x00\x00\x00\x00\x00",6 ) == 0)
	{
		return NO_DISP;
	}

	if( stTemp.iTransNo==NETPAY_FORZADA )
	{
		sprintf(szTitle,"VENTA FORZADA");
	}
	else 
	{
		sprintf(szTitle,"VENTA %s",PosCom.stTrans.AscCardType);
	}

	//转换成金额格式显示
	FormAmountFormmat((uint8_t*)szAmount,(uint8_t *)szdispAmount);

	lcdClrLine(2,7);
	DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, szTitle);
	lcdDisplay(0, 2, DISP_CFONT, "MONTO CORRECTO?");
	lcdDisplay(0, 4, DISP_CFONT, "$%s",szdispAmount);
	lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
	lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
	lcdFlip();
	do {
		iKey = PubWaitKey(60);
	}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
		iKey != KEY1 && iKey != KEY2);

	if ( iKey == KEY_CANCEL ) {
		return NO_DISP;
	}
	else if ( iKey == KEY_TIMEOUT) {
		return NO_DISP;
	}
	else if ( iKey == KEY2 ) {
		return RE_SELECT;
	}
	else if ( iKey == KEY_ENTER || iKey == KEY1){
		;
	}

	return OK;
}


uint8_t PaymentwithBalance(void)
{
	uint8_t	ucRet,sTempBuff[60];
	char szField63BuffTemp[400];
	long lAmount = 0;
	uint8_t AscAmt[13] = {0};
	
	ucRet = BalanceCheck();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	while(1)
	{
		ucRet = AppGetAmount(9, TRAN_AMOUNT);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		PubBcd2Asc0(PosCom.stTrans.sAmount, 6, (char*)AscAmt);
		lAmount = atol((char*)AscAmt);
		if( lAmount > atol((char*)PosComconTrol.szAvailablebalance) )
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "Amount invalid ");
			lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "Intente de nuevo");
			lcdFlip();
			FailBeep();
			PubWaitKey(2);
			continue;
		}

		ucRet = ComfirAmount(PosCom.stTrans.sAmount);
		if(ucRet == RE_SELECT){
			continue;
		}
		else if( ucRet!=OK )
		{
			return ucRet;
		}
		else{
			break;
		}
		
	}

	stTemp.iTransNo = PAYMENT_WITH_BALANCE;
	PosCom.stTrans.iTransNo = PAYMENT_WITH_BALANCE;
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"776605");
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	strcpy((char *)glSendPack.szEntryMode,"0001");	
	sprintf((char *)glSendPack.szNII, "0003"); 

	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	return NO_DISP;
}


// pay with balance
uint8_t BalanceCheck(void)
{
	uint8_t	ucRet;
	int     iRet;
	uint8_t sTempBuff[60];
	char szField63BuffTemp[400];
	char H1[40],H2[40]/*,T2[40],T3[40],T4[40],T5[40],T6[40] = {0}*/;

	lcdCls();
	DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, "     Proceso    ");
	DispMulLanguageString(0, 6, DISP_MEDIACY|DISP_CFONT, NULL, "     primario    ");
	lcdFlip();
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"776604");
	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack2,0,sizeof(glSendPack.szTrack2));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60));
	memcpy(glSendPack.szTranAmt,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",12);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	sprintf((char *)glSendPack.szEntryMode, "0001");
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	ShortToByte(glSendPack.sField62, strlen(stPosParam.szUser));
	sprintf((char *)sTempBuff,"%s",stPosParam.szUser);
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( memcmp(PosCom.szRespCode, "00", 2)!=0 )
    {
 		dispRetCode(1);
	    return NO_DISP;
    }

	iRet = ProcBalanceCheck(H1,H2);
	if( iRet!=OK )
	{
		return iRet;
	}
	
	return OK;
}




// pay with balance
uint8_t CheckAllBalance(void)
{
	uint8_t	ucRet;
	int     iRet;
	char szField63BuffTemp[400];
	char T0[40],T1[40],T2[40],T3[40],T4[40],T5[40],T6[40] = {0};

	lcdCls();
	DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, "     Proceso    ");
	DispMulLanguageString(0, 6, DISP_MEDIACY|DISP_CFONT, NULL, "     primario    ");
	lcdFlip();

	SetCommReqField((uint8_t *)"0100", (uint8_t *)"300001");
	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack2,0,sizeof(glSendPack.szTrack2));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60));
	memcpy(glSendPack.szTranAmt,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",12);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	sprintf((char *)glSendPack.szEntryMode, "0001");
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	/*
	if( memcmp(PosCom.szRespCode, "00", 2)!=0 )
    {
 		dispRetCode(1);
	    return NO_DISP;
    }*/

	iRet = ProcCheckAllBanlance(T0,T1,T2,T3,T4,T5,T6);
	if( iRet!=OK )
	{
		return iRet;
	}

	DispBalanceCheck(T0,T1,T2,T3,T4,T5,T6);

	PrintBalanceCheck(T0,T1,T2,T3,T4,T5,T6);
	
	return NO_DISP;
}


void DispBalanceCheck(char *T0,char *T1,char *T2,char *T3,char *T4,char *T5,char *T6)
{	
	lcdCls();	
	if( atol(T6)<0 )
	{
		lcdDisplay(0, 2, DISP_MEDIACY|DISP_HFONT16, "LIMITE DE CREDITO $    %ld.%02ld",atol(T6)/100,atol(T6)*(-1)%100);
	}
	else
	{
		lcdDisplay(0, 2, DISP_MEDIACY|DISP_HFONT16, "LIMITE DE CREDITO $    %ld.%02ld",atol(T6)/100,atol(T6)%100);
	}


	if( atol(T3)<0 )
	{
		lcdDisplay(0, 3, DISP_MEDIACY|DISP_HFONT16, "SALDO DISPONIBLE   $    %ld.%02ld",atol(T3)/100,atol(T3)*(-1)%100);
	}
	else
	{
		lcdDisplay(0, 3, DISP_MEDIACY|DISP_HFONT16, "SALDO DISPONIBLE   $    %ld.%02ld",atol(T3)/100,atol(T3)%100);
	}
	
	lcdDisplay(0, 4, DISP_MEDIACY|DISP_HFONT16, "FECHA CORTE        :%s\n",T0);
	lcdDisplay(0, 5, DISP_MEDIACY|DISP_HFONT16, "FECHA ULTIMO PAGO  :%s\n",T1);

	if( atol(T2)<0 )
	{
		lcdDisplay(0, 6, DISP_MEDIACY|DISP_HFONT16, "PAGO MINIMO      $    %ld.%02ld",atol(T2)/100,atol(T2)*(-1)%100);
	}
	else
	{
		lcdDisplay(0, 6, DISP_MEDIACY|DISP_HFONT16, "PAGO MINIMO      $    %ld.%02ld",atol(T2)/100,atol(T2)%100);
	}
	
	if( atol(T4)<0 )
	{
		lcdDisplay(0, 7, DISP_MEDIACY|DISP_HFONT16, "SALDO VENTAS      $    %ld.%02ld",atol(T4)/100,atol(T4)*(-1)%100);
	}
	else
	{
		lcdDisplay(0, 7, DISP_MEDIACY|DISP_HFONT16, "SALDO VENTAS      $    %ld.%02ld",atol(T4)/100,atol(T4)%100);
	}
	lcdFlip();
	OkBeep();
	kbGetKeyMs(3000);

}


// pay with balance
uint8_t PayWithCard(void)
{
	uint8_t	ucRet;
	int     iRet;
	uint8_t sTempBuff[60];
	char szField63BuffTemp[400];
	char C0[40];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, "     BALANCE CHECK    ");

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"776608");
	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack2,0,sizeof(glSendPack.szTrack2));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60));
	memcpy(glSendPack.szTranAmt,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",12);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

/*	if( memcmp(PosCom.szRespCode, "00", 2)!=0 )
    {
 		dispRetCode(1);
	    return NO_DISP;
    }*/

	iRet = ProcPayWithCardBalanceCheck(C0);
	if( iRet!=OK )
	{
		return iRet;
	}

	return OK;
}


uint8_t ProcTheLastMent(uint8_t *field49,uint8_t *field59,uint8_t *field60,uint8_t *field61,uint8_t *field63)
{
	int  szLen = 0;
	char *Temp1,*Temp2,*Temp3,*Temp4,*Temp5,*Temp6,*Temp7,*Temp8,*Temp9,*Temp10;
	char szParams[6];

	Temp1 = strstr((char*)field59, "Z0");
	if ( Temp1 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent1,Temp1+8,szLen);
		PosComconTrol.szMent1[szLen] = '\0';
	}
	
	Temp2 = strstr((char*)field59, "Z1");
	if ( Temp2 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp2+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent2,Temp2+8,szLen);
		PosComconTrol.szMent2[szLen] = '\0';
	}

	Temp3 = strstr((char*)field60, "Z2");
	if ( Temp3 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp3+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent3,Temp3+8,szLen);
		PosComconTrol.szMent3[szLen] = '\0';
	}

	

	Temp4 = strstr((char*)field60, "Z3");
	if ( Temp4 == NULL ) {
//		return -1;
	}
	else {
		strncpy(szParams,Temp4+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent4,Temp4+8,szLen);
		PosComconTrol.szMent4[szLen] = '\0';
	}

	Temp5 = strstr((char*)field61, "Z4");
	if ( Temp5 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp5+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent5,Temp5+8,szLen);
		PosComconTrol.szMent5[szLen] = '\0';
	}
	
	Temp6 = strstr((char*)field61, "Z5");
	if ( Temp6 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp6+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent6,Temp6+8,szLen);
		PosComconTrol.szMent6[szLen] = '\0';
	}

	Temp7 = strstr((char*)field49, "Z6");
	if ( Temp7 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp7+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent7,Temp7+8,szLen);
		PosComconTrol.szMent7[szLen] = '\0';
	}

	Temp8 = strstr((char*)field49, "Z7");
	if ( Temp8 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp8+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent8,Temp8+8,szLen);
		PosComconTrol.szMent8[szLen] = '\0';
	}
	
	Temp9 = strstr((char*)field63, "Z8");
	if ( Temp9 == NULL ) {
//		return -1;
	}
	else {
		strncpy(szParams,Temp9+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent9,Temp9+8,szLen);
		PosComconTrol.szMent9[szLen] = '\0';
	}
	
	Temp10 = strstr((char*)field63, "Z9");
	if ( Temp10 == NULL ) {
//		return -1;
	}
	else {
		strncpy(szParams,Temp10+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szMent10,Temp10+8,szLen);
		PosComconTrol.szMent10[szLen] = '\0';
	}
	
	return OK;
}


// 最后一个操作
uint8_t LastMovement(void)
{
	uint8_t	ucRet;
	char szField63BuffTemp[400];
	uint8_t BankReference[22] = {0};

	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "Consulta Movimientos");
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"776607");
	memcpy(glSendPack.szTranAmt,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",12);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
	strcpy((char *)glSendPack.szEntryMode,"0001");	
	sprintf((char *)glSendPack.szNII, "0003"); 
	memcpy(BankReference,"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x30\x01\x02\x03\x04\x05\x06\x07\x08\x09\x30\x01\x02\x3",23);
	sprintf((char *)glSendPack.szTrack2,"%s",BankReference);

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	sprintf((char *)glSendPack.szCurrencyCode, "484"); // 货币代码

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);	
	ucRet = SendRecvPacket();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	/*
	if( memcmp(PosCom.szRespCode, "00", 2)!=0 )
    {
 		dispRetCode(1);
	    return NO_DISP;
    }*/

	ucRet = ProcTheLastMent(glRecvPack.szCurrencyCode,glRecvPack.szField59,glRecvPack.szField60,glRecvPack.szOrginalMsg,glRecvPack.szField63);
	if( ucRet != OK )
	{
		return E_TRANS_FAIL;
	}

	NetpayPrtTranTicket(0);
	return NO_DISP;
}

uint8_t NetpayPhoneTopup(void)
{
	int     i = 0;
	uint8_t	ucRet;
	char 	szField63BuffTemp[400];
	char 	szField120BuffTemp[400];
	int     keySub;
	int 	iKey;
	char    szAmount[16];
	char    szPhoneNo[20];
	char    szPhoneNo2[20];
	int     telcel[] = {20,30,50,100,200,300,500};
	int     movistar[] = {20,30,50,60,100,150,200,300,500};
	int     iusacell[] = {50,100,200,300,500};
	int     umefon[] = {50,150,200,300,500};
	int     nextel[] = {30,50,100,200,500};
	//char    PhoneComanyName[20] = {0};
	int     RI = 0;
	
	const char *phone_company[] =
	{
		"TIEMPO AIRE",
		"1-TELCEL",
		"2-MOVISTAR",
		"3-IUSACELL",
		"4-UNEFON",
		"5-NEXTEL"
	};

	lcdCls();
	DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, "TIEMPO AIRE");
	lcdFlip();
	
	keySub = DispMenu3(phone_company, 6, 1, KEY1, KEY5, stPosParam.ucOprtLimitTime);
	if( keySub==KEY1 )
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_HFONT16, NULL, "TIEMPO AIRE TELCEL");
		for( i = 0; i < sizeof(telcel)/4; i++ ){
			lcdDisplay(0, i+1, DISP_ASCII, "%d-$%d",i+1,telcel[i]);
			lcdFlip();
		}
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2 && iKey != KEY3 && iKey != KEY4
			&& iKey != KEY5 && iKey != KEY6 && iKey != KEY7);

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}
		//strcpy(PhoneComanyName,"TELCEL");
		RI = 1;
		sprintf((char *)szAmount, "%012d", telcel[iKey-'1']*100);
		AscToBcd(PosCom.stTrans.sAmount, (uint8_t *)szAmount, 12);
		for(i = 0; i < sizeof(telcel)/4; i++){
			if( telcel[iKey-'1'] == TELCEL_SKUTable[i].szTopupAmount ){
				sprintf(PosComconTrol.szSKU,"%s",TELCEL_SKUTable[i].szSKU);
				PosComconTrol.szTopupAmount = telcel[iKey-'1'];
				break;
			}
		}
		
	}
	else if( keySub==KEY2 )
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_HFONT16, NULL, "TIEMPO AIRE MOVISTAR");
		for( i = 0; i < sizeof(movistar)/4; i++ ){
			lcdDisplay(0, i+1, DISP_ASCII, "%d-$%d",i+1,movistar[i]);
			lcdFlip();
		}
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2 && iKey != KEY3 && iKey != KEY4
			&& iKey != KEY5 && iKey != KEY6 && iKey != KEY7 && iKey != KEY8 && iKey != KEY9);

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}
		
		//strcpy(PhoneComanyName,"MOVISTAR");
		RI = 2;
		sprintf((char *)szAmount, "%012d", movistar[iKey-'1']*100);
		AscToBcd(PosCom.stTrans.sAmount, (uint8_t *)szAmount, 12);
		AscToBcd(PosCom.stTrans.sAmount, (uint8_t *)szAmount, 12);
		for(i = 0; i < sizeof(movistar)/4; i++){
			if( movistar[iKey-'1'] == MOVISTAR_SKUTable[i].szTopupAmount ){
				sprintf(PosComconTrol.szSKU,"%s",MOVISTAR_SKUTable[i].szSKU);
				PosComconTrol.szTopupAmount = movistar[iKey-'1'];
				break;
			}
		}
		
	}
	else if( keySub==KEY3 )
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_HFONT16, NULL, "TIEMPO AIRE IUSACELL");
		for( i = 0; i < sizeof(iusacell)/4; i++ ){
			lcdDisplay(0, i+1, DISP_ASCII, "%d-$%d",i+1,iusacell[i]);
			lcdFlip();
		}
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2 && iKey != KEY3 && iKey != KEY4 && iKey != KEY5 );

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}

		//strcpy(PhoneComanyName,"IUSACELL");
		RI = 3;
		sprintf((char *)szAmount, "%012d", iusacell[iKey-'1']*100);
		AscToBcd(PosCom.stTrans.sAmount, (uint8_t *)szAmount, 12);
		for(i = 0; i < sizeof(iusacell)/4; i++){
			if( iusacell[iKey-'1'] == IUSACEL_SKUTable[i].szTopupAmount ){
				sprintf(PosComconTrol.szSKU,"%s",IUSACEL_SKUTable[i].szSKU);
				PosComconTrol.szTopupAmount = iusacell[iKey-'1'];
				break;
			}
		}	
	}
	else if( keySub==KEY4 )
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_HFONT16, NULL, "TIEMPO AIRE UNEFON");
		for( i = 0; i < sizeof(umefon)/4; i++ ){
			lcdDisplay(0, i+1, DISP_ASCII, "%d-$%d",i+1,umefon[i]);
			lcdFlip();
		}
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2 && iKey != KEY3 && iKey != KEY4 && iKey != KEY5);

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}
		
		//memcpy(PhoneComanyName,"UNEMON",strlen("UNEMON"));
		RI = 4;
		sprintf((char *)szAmount, "%012d", umefon[iKey-'1']*100);
		AscToBcd(PosCom.stTrans.sAmount, (uint8_t *)szAmount, 12);
		for(i = 0; i < sizeof(umefon)/4; i++){
			if( umefon[iKey-'1'] == UNEFON_SKUTable[i].szTopupAmount ){
				sprintf(PosComconTrol.szSKU,"%s",UNEFON_SKUTable[i].szSKU);
				PosComconTrol.szTopupAmount = umefon[iKey-'1'];
				break;
			}
		}
	}
	else if( keySub==KEY5 )
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_HFONT16, NULL, "TIEMPO AIRE NEXTEL");
		for( i = 0; i < sizeof(nextel)/4; i++ ){
			lcdDisplay(0, i+1, DISP_ASCII, "%d-$%d",i+1,nextel[i]);
			lcdFlip();
		}
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2 && iKey != KEY3 && iKey != KEY4 && iKey != KEY5);

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}

		//memcpy(PhoneComanyName,"NEXTEL",strlen("NEXTEL"));
		RI = 6;
		sprintf((char *)szAmount, "%012d", nextel[iKey-'1']*100);
		AscToBcd(PosCom.stTrans.sAmount, (uint8_t *)szAmount, 12);
		for(i = 0; i < sizeof(nextel)/4; i++)
		{
			if( nextel[iKey-'1'] == NEXTEL_SKUTable[i].szTopupAmount )
			{
				sprintf(PosComconTrol.szSKU,"%s",NEXTEL_SKUTable[i].szSKU);
				PosComconTrol.szTopupAmount = nextel[iKey-'1'];
				break;
			}
		}
	}
	else if( keySub==KEY_CANCEL || keySub == KEY_TIMEOUT )
	{
		DirectIntoMenu = 1;     // 返回后直接进入主菜单
		return NO_DISP;
	}
	
	while(1)
	{
		ucRet = InputPhoneNo(1,szPhoneNo);
		if(ucRet != OK)
		{
			return ucRet;
		}

		ucRet = InputPhoneNo(2,szPhoneNo2);
		if(ucRet != OK)
		{
			return ucRet;
		}
		if(strcmp(szPhoneNo,szPhoneNo2) != 0)
		{
			lcdClrLine(2,7);
			lcdDisplay(0, 4, DISP_ASCII, "ERROR NUMERO");
			OkBeep();
			continue;
		}
		else
		{
			break;
		}
	}
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"210016");
	memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));	
	GetPosTime(stTemp.szDate, stTemp.szTime);
	PosCom.stTrans.iTransNo = PHONE_TOPUP;
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	strcpy((char*)glSendPack.szTrack2,szPhoneNo);


	//strcpy((char *)PosCom.stTrans.PhoneNo,szPhoneNo);
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	strcpy((char*)glSendPack.szCurrencyCode,"484");

	PosCom.stTrans.szEntryMode[1] = '1';

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	NetpayDealFeild120((uint8_t *)szField120BuffTemp,RI);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char *)glSendPack.szField120,szField120BuffTemp);

	//memset(PosCom.stTrans.PhoneComanyName,0,sizeof(PosCom.stTrans.PhoneComanyName));
	//strcpy((char *)PosCom.stTrans.PhoneComanyName,PhoneComanyName);
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	return OK;
}


uint8_t InputPhoneNo(int flag,char *szPhoneNo)
{
	char buf[20];	
	int  iRet;

	lcdClrLine(1,7);
	if(flag == 1){
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "INGRESE NUMERO");
	}
	else{
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "CONFIRME NUMERO");
	}
	
	memset(szPhoneNo, 0, sizeof(szPhoneNo));
    lcdFlip();
	memset(buf, 0, sizeof(buf));
	do{
		lcdGoto(65, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return E_TRANS_CANCEL;
		else if ( iRet < 10 && iRet > 0)
		{
			lcdClrLine(2,7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "INGRESE 10 DIGITOS");
			lcdFlip();
			ErrorBeep();
			usleep(20*1000);
			DirectIntoMenu = 1;     // 返回后直接进入主菜单
			return NO_DISP;
		}
		else 
			break;
	}while( 1 );
	
	strcpy(szPhoneNo,buf);
	return OK;
}

uint8_t SaveLogonMsg(uint8_t *szField63)
{	
//	uint8_t	ucRet;
	int  szLen = 0;
	char *Temp1 = NULL;
	char *Temp2 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)szField63, "SS");
	if ( Temp1 == NULL ) {
		return -1;
	}

	strncpy(szParams,Temp1+2,5);
	szLen = atoi(szParams);
	memset(stPosParam.szStreet,0,sizeof(stPosParam.szStreet));
	strncpy(stPosParam.szStreet,Temp1+8,szLen);
	stPosParam.szStreet[szLen] = '\0';

	Temp2 = strstr((char*)szField63, "SA");
	if ( Temp2 == NULL ) {
		return -1;
	}

	strncpy(szParams,Temp2+2,5);
	szLen = atoi(szParams);
	memset(stPosParam.szCity,0,sizeof(stPosParam.szCity));
	strncpy(stPosParam.szCity,Temp2+8,szLen);
	stPosParam.szCity[szLen] = '\0';

	return OK;
}

uint8_t PosOnlineLogon(uint8_t flag)
{
	uint8_t	ucRet, sBuff[10],sTempBuff[60];
	int	lBatchNo;
	int  ret;
	char szField63BuffTemp[400];

	ret = 0;
	memset(sTempBuff,0,sizeof(sTempBuff));
	lcdCls();
	InitCommData();

	if (flag == 1)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE, NULL, "     LOGIN      ");
	}
	
	if( flag==0 || flag==1 )
	{
		ucRet = TellerLogon(flag,1);
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}

	if( flag==0 && stTransCtrl.ucLogonFlag==LOGGEDON && 
		strcmp((char *)stTransCtrl.szNowTellerNo,(char *)stTransCtrl.szLoginTellerNo)==0)
	{
		SaveCtrlParam();
		return OK;
	}

	if(stTransCtrl.ucPosStatus==SETTLE_STATUS || stTransCtrl.ucPosStatus==BATCH_STATUS)
	{
		lcdCls();
		if(stTransCtrl.ucPosStatus==SETTLE_STATUS)
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LAST SETTLE HALT");
		else
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LAST UPLOAD HALT");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "  SETTLE AGAIN  ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(5000);
		return NO_DISP;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     LOGON      ");
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "PROCESSING");
	lcdFlip();
	
	InitCommData();
    stTemp.iTransNo = POS_LOGON;
	PosCom.stTrans.iTransNo = POS_LOGON;
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"210017");
	// NETPAY
	memcpy(glSendPack.szPan,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",16);
	memcpy(glSendPack.szTranAmt,"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",12);
	
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
//	sprintf((char *)glSendPack.szEntryMode, "0001");
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	sprintf((char *)glSendPack.szCurrencyCode, "484"); // 货币代码
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60)); // 无60域
	
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);	
	ucRet = SendRecvPacket();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( memcmp(PosCom.szRespCode, "00", 2)!=0 )
    {
 		NetpaydispRetCode(1);
	    return NO_DISP;
    }

	ucRet = SaveLogonMsg(glRecvPack.szField63);
	if(ucRet != OK)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "SAVE LOGON MSG FAIL");
		lcdFlip();
		PubBeepErr();
		CommHangUp(FALSE);
		kbGetKey();
		return NO_DISP;
		
	}
	SaveAppParam();

	sprintf((char *)sBuff, "%.6s", &glRecvPack.szField60[2]);
	lBatchNo = atol((char *)sBuff);
	if( lBatchNo!=stPosParam.lNowBatchNum && lBatchNo>0 && lBatchNo<999999L )
	{
		stPosParam.lNowBatchNum = lBatchNo;
		SaveAppParam();
	}

#ifdef _OFFLINE_TEST_VERSION
	memcpy(sTempBuff, "\x11\x11\x11\x11\x11\x11\x11\x11\x44\x44\x44\x44\x44\x44\x44\x44", 16);
	ret = PedWritePinKey(PARITY_NONE, stPosParam.ucKeyIndex+1, DOUBLE_PIN_KEY_ID, 16, sTempBuff);
	if( ret!=PED_RET_OK ) 
		return (E_PINPAD_KEY);
	
	//20080730
	memcpy(sTempBuff, "\x11\x11\x11\x11\x11\x11\x11\x11\x22\x22\x22\x22\x22\x22\x22\x22", 16);
	ret = PedWriteMacKey(PARITY_NONE, stPosParam.ucKeyIndex+1, DOUBLE_MAC_KEY_ID, 16, sTempBuff);
	if( ret!=PED_RET_OK ) 
		return (E_PINPAD_KEY);
	
	memcpy(sTempBuff, "\x11\x11\x11\x11\x11\x11\x11\x11\x33\x33\x33\x33\x33\x33\x33\x33", 16);
	ret = PedWriteMacKey(PARITY_NONE, stPosParam.ucKeyIndex+1, DOUBLE_TDK_KEY_ID, 16, sTempBuff);
	if( ret!=PED_RET_OK ) 
			return (E_PINPAD_KEY);
#else
	// NetPay 不需要密钥灌装
/*	if( LogonDivKey(&glRecvPack.sField62[2],flag) )
	{
		stPosParam.ucPINMAC = 1;
		SaveAppParam();
		if(flag != 2)
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "LOAD KEY FAIL");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LOAD MASTER KEY");
			lcdFlip();
			PubBeepErr();
			CommHangUp(FALSE);
			kbGetKey();
		}
		return E_PINPAD_KEY;
	}
	stPosParam.ucPINMAC = 0;
	SaveAppParam();
*/
	if( fileExist("stat.tm")<0 )
	{
		write_stat_time();
		memset(&dial_stat, 0, sizeof(DIAL_STAT));
		write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta");
	}

	if( CheckDate((char *)PosCom.stTrans.szDate)==0 && CheckTime((char *)PosCom.stTrans.szTime)==0 )
	{
		AscToBcd(sBuff,   PosCom.stTrans.szDate+2, 6);
		AscToBcd(sBuff+3, PosCom.stTrans.szTime,   6);
		sysSetTime(sBuff);
	}
#endif

	stTransCtrl.ucLogonFlag = LOGGEDON;
	strcpy((char *)stTransCtrl.szLoginTellerNo, (char *)stTransCtrl.szNowTellerNo);
	SaveCtrlParam();
	stPosParam.szLogonFlag = LOGGEDON;
	SaveAppParam();

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LOGIN EXITOSO");
	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "PRESIONE ENTER");
	lcdFlip();
	OkBeep();
	PubWaitKey(3);

	OnlineDownOrUp();
	return NO_DISP;
}

uint8_t PosOnlineLogoff(uint8_t flag)
{
	uint8_t	ucRet;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    LOGOUT     ");
	lcdFlip();
	InitCommData();
	stTemp.iTransNo = POS_LOGOFF;
	PosCom.stTrans.iTransNo = POS_LOGOFF;
	
	SetCommReqField((uint8_t *)"0820", (uint8_t *)"");

	ucRet = OnlineCommProc();
	if( ucRet==OK )
	{
		stTransCtrl.ucLogonFlag = LOGGEDOFF;
		SaveCtrlParam();
		
		lcdClrLine(2,7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SUCCESS");
		lcdFlip();
		OkBeep();
		kbGetKeyMs(3000);
		ucRet = NO_DISP;
	}

	return ucRet;
}

uint8_t PrintSettleBatch(void)
{
	int ucRet;
	
	kbFlush();
	if( stPosParam.ucDetailPrt==PARAM_OPEN )
	{
/*		lcdClrLine(2, 7);	
		lcdDisplay(0, 2, DISP_CFONT, "PRINT DETAIL?");
		lcdDisplay(0, 4, DISP_CFONT, "0.NO");
		lcdDisplay(0, 6, DISP_CFONT, "1.YES");
		lcdFlip();
		iKey = kbGetKeyMs(30000);
		if( iKey==KEY1 )
		{
			
		}
*/		ucRet = RestaurantPrintDetail(FALSE,TRUE);
	}

/*	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, "PRINT FAIL DETAIL");
	lcdDisplay(0, 4, DISP_CFONT, "0.NO");
	lcdDisplay(0, 6, DISP_CFONT, "1.YES");
	lcdFlip();
	iKey = kbGetKeyMs(30000);
	if( iKey==KEY1 )
	{
		ucRet = PrintAllErrOffTrans(); 
	}

	if (gstPosCapability.uipostsScreen && stPosParam.ucSupportElecSign == PARAM_OPEN)
	{
		if (CheckElecSignFailTxn())
		{
			lcdClrLine(2, 7);	
			lcdDisplay(0, 2, DISP_CFONT, "PRINT FAIL SIGN");
			lcdDisplay(0, 4, DISP_CFONT, "0.NO");
			lcdDisplay(0, 6, DISP_CFONT, "1.YES");
			lcdFlip();
			iKey = kbGetKeyMs(30000);
			if( iKey==KEY1 )
			{
				ucRet = PrintAllErrElecSignTrans(); 
			}
		}
	}
*/	return 0;
}


uint8_t QueryPostCheckInIfFinish(void)
{
	int     iCnt;
	NEWPOS_LOG_STRC	stLog;
	uint8_t ucRet;
	

	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return E_FILE_READ;
		}

		if(stLog.iTransNo == CHECK_IN)
		{
			if(stLog.ucCheckOutId != TRUE  )
			{
				return NO_DISP;				
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	
	return OK;
}


uint8_t QueryPostAuthIfFinish(void)
{
	int     iCnt;
	NEWPOS_LOG_STRC	stLog;
	uint8_t ucRet;
	

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return E_FILE_READ;
		}

		if(stLog.iTransNo == POS_PREAUTH)
		{
			if(stLog.ucAuthCmlId != TRUE && stLog.ucVoidId != TRUE )
			{
				lcdCls();
				lcdDisplay(0, 4, DISP_HFONT16, "CIERRE DE LOTE");
				lcdDisplay(0, 6, DISP_HFONT16|DISP_MEDIACY, "COMPRAS PENDIENTES DE CONFIRMAR");
				lcdDisplay(0, 7, DISP_HFONT16|DISP_MEDIACY, "NO SE PUEDE REALIZAR CIERRE DE LOTE ");
				lcdFlip();
				FailBeep();
				PubWaitKey(3);
				return NO_DISP;				
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	
	return OK;
}

uint8_t PosSettleTran(void)
{
	uint8_t	ucRet;
	char sTempBuff[30];
	char szField63BuffTemp[400];

	stPosParam.SettleSign = FALSE; // 结算标志置为false

	ucRet = CheckParamPass();
	if( ucRet!=1 )
	{
		return NO_DISP;
	}
	
	kbFlush();
	lcdCls(); 	
	if( fileExist(REVERSAL_FILE)>=0 )
	{
		lcdCls();
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PLEASE CONFIRM ");
		lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "REVERSE");
		lcdFlip();
		FailBeep();
		PubWaitKey(3);
		stPosParam.SettleSign = TRUE; // 不需要结算
	}
	
	if( stTransCtrl.iTransNum==0 )
	{
		lcdCls();
		lcdDisplay(0, 4, DISP_CFONT, "CIERRE DE LOTE");
		lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "LOTE VACIO");
		lcdFlip();
		FailBeep();
		PubWaitKey(3);
		stPosParam.SettleSign = TRUE; // 不需要结算
		return NO_DISP;
	}

	lcdClrLine(2, 7);
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "CIERRE DE LOTE");
	lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "UN MOMENTO");
	lcdFlip();

	// 查询预授权是否全部完成(post auth),不完成不能进行结算
	ucRet = QueryPostAuthIfFinish();
	if( ucRet!=OK){
		return ucRet;
	}
	
	lcdClrLine(2, 7);
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "ENVIANDO CIERRE");
	lcdDisplay(0, 6, DISP_CFONT, "TRANSMITIENDO");
	lcdFlip();
	PubWaitKey(1);

	//netpay
	if(stTransCtrl.ucPosStatus!=SETTLE_STATUS && stTransCtrl.ucPosStatus!=BATCH_STATUS)
	{
		ucRet =ReverseTranProc();
		if (ucRet == E_REVERSE_FAIL)
		{
			return ucRet;
		}

		if (gstPosCapability.uipostsScreen && stPosParam.ucSupportElecSign == PARAM_OPEN)
		{
			if (GetElecSignTxnNums())
			{
				ucRet = TrickleFeedElecSignTxn(FALSE,1);
				if (ucRet != 0)
				{
					return ucRet;
				}
			}

			while(GetElecSignFailTxnNums())
			{
				ucRet = TrickleFeedElecSignTxn(FALSE,2);
				if (ucRet != 0)
				{
					return ucRet;
				}
			}
		}
	}

	InitCommData();
	GetPosTime(stTemp.szDate, stTemp.szTime);
    	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);

	stTemp.iTransNo = POS_SETT;
	PosCom.stTrans.iTransNo = POS_SETT;
	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;

	stTemp.ucRmbSettRsp = '1'; 
	stTemp.ucFrnSettRsp = '1'; 

	SetCommReqField((uint8_t *)"0500", (uint8_t *)"920000");
	memset(glSendPack.szPan,0,sizeof(glSendPack.szPan));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
								
//	sprintf((char *)glSendPack.szEntryMode, "0001");
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// 操作员ID

	// newpos is 172839 no change 
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);

	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	//netpay
/*	BcdToAsc0(szBuff[0], stTotal.sRmbDebitAmt,  12);
	BcdToAsc0(szBuff[1], stTotal.sRmbCreditAmt, 12);
	BcdToAsc0(szBuff[2], stTotal.sFrnDebitAmt,  12);
	BcdToAsc0(szBuff[3], stTotal.sFrnCreditAmt, 12);
	sprintf((char *)glSendPack.szField48, "%s%03d%s%03d0%s%03d%s%03d0",
			szBuff[0], stTotal.iRmbDebitNum, szBuff[1], stTotal.iRmbCreditNum,
			szBuff[2], stTotal.iFrnDebitNum, szBuff[3], stTotal.iFrnCreditNum);
	sprintf((char *)glSendPack.szCurrencyCode, "156");
	sprintf((char *)glSendPack.szField63, "%.2s ", stTransCtrl.szNowTellerNo+1);
*/	
	PosCom.stTrans.isCtlplat =0;
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	stTransCtrl.ucPosStatus = WORK_STATUS;
	SaveCtrlParam();
	
	stTotal.lBatchNum = stPosParam.lNowBatchNum;
	strcpy((char *)stTotal.sDateTime, (char *)stTemp.szDate);
	strcpy((char *)(stTotal.sDateTime+8), (char *)stTemp.szTime);
	SaveLastTotal(); 
	
	stTransCtrl.ucClearLog = TRUE;
	SaveCtrlParam();

	
	stPosParam.ucReprint = 2;
	SaveAppParam();
	
	ucRet = PrintTransTotal(1, stPosParam.lNowBatchNum);
	
	
	stPosParam.ucReprint= 3;
	SaveAppParam();

//	PrintSettleBatch();		
	stPosParam.ucReprint= 0;
	SaveAppParam();

	AfterSettleTran();	
	CommHangUp(FALSE);	

	stPosParam.SettleSign = TRUE; // 结算完成
	return OK;
}

void AfterSettleTran()	
{
/*	if ( stPosParam.ucAutoLogoff==PARAM_OPEN )
  	{
    	PosOnlineLogoff(0);
    	stTransCtrl.ucLogonFlag = LOGGEDOFF;
    	SaveCtrlParam();
	}
*/
	stPosParam.lNowBatchNum++;
	SaveAppParam();
}

uint8_t  PosBatchSendTran(void)
{
	uint8_t  result;
	int  ret;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     UPLOAD      ");
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "POS UPLOAD");
	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "PLS WAITING...");
	lcdFlip();
	if( stTransCtrl.ucBatchUpStatus==0 )
	{
		return OK;
	}

	ret = PosBatchUp(stTransCtrl.ucBatchUpStatus);
	if( OK!=ret ) 
		return ret;

	memset((uint8_t *)&stUplTraceSync, 0, sizeof(struct _st_UPLTRACENO_SYNC));
	SaveUplTraceSync();

	stTransCtrl.ucPosStatus = WORK_STATUS;
	stTransCtrl.ucClearLog = TRUE;
	SaveCtrlParam();

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	result = ComputeTransTotal(1);

	memset(&stLimitTotal, 0, sizeof(struct _limit_total_strc));
	SaveLimitTotal();
	
	stTotal.lBatchNum = stPosParam.lNowBatchNum;
	strcpy((char *)stTotal.sDateTime, (char *)stTemp.szDate);
	strcpy((char *)(stTotal.sDateTime+8), (char *)stTemp.szTime);
	SaveLastTotal(); 
	
	stPosParam.ucReprint= 2;
	SaveAppParam();
	result = PrtTransTotal(1, stPosParam.lNowBatchNum);	
	stPosParam.ucReprint= 3;
	SaveAppParam();

	PrintSettleBatch();
	stPosParam.ucReprint= 0;
	SaveAppParam();
	
	AfterSettleTran();
	CommHangUp(FALSE);
	
	return (OK);
}


uint8_t PosBatchUp(uint8_t flag)
{
	uint8_t	ucRet;
	int	iTotalNum;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     UPLOAD      ");
	lcdFlip();

	iTotalNum = 0;	
	bRecvNOK  = 0;

	ReadUplTraceSync();
	ucRet = BatchSendOffTrans1(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	ucRet = BatchICCOffLineTrans(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}
	
	ucRet = BatchMagOnLineTrans(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	ucRet = BatchSendOffTrans2_Refund(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	ucRet = BatchSendOffTrans2_CM(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	ucRet = BatchSendICCOnLineTrans(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	ucRet = TransBatchUpLoadFailureTxn(&iTotalNum, flag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	if (ucRet!=0 || bRecvNOK==1)
	{
		return E_RECV_PACKET;
	}
		

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     UPLOAD      ");
	lcdFlip();
	
	InitCommData();
	stTemp.iTransNo = POS_BATCH_UP;
	PosCom.stTrans.iTransNo = POS_BATCH_UP;
	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
	gUplTrace = 0;
	SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
	sprintf((char *)glSendPack.szField48, "%04d", iTotalNum); 
	if( flag==EMVLOG )
	{
		sprintf((char *)&glSendPack.szField60[8], "207");
	}
	else
	{
		sprintf((char *)&glSendPack.szField60[8], "202");
	}
	
	ucRet = SendRecvPacket();
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
		return ucRet;
	
	return OK;
}

int GetTxnPair(int iTxnTrace)
{
  int var_i;

  if (stUplTraceSync.lCurUplNo == 0)
    return 0;

  for (var_i=0; var_i<stUplTraceSync.lCurUplNo; var_i++)
  {
    if (iTxnTrace == stUplTraceSync.stUplTracePair[var_i].lUplFirstTraceNo)
      return stUplTraceSync.stUplTracePair[var_i].lNowTraceNo;
  }
  return 0;
}

void SaveTxnPair(int iTxnTrace)
{
  stUplTraceSync.stUplTracePair[stUplTraceSync.lCurUplNo].lUplFirstTraceNo = iTxnTrace;
  stUplTraceSync.stUplTracePair[stUplTraceSync.lCurUplNo].lNowTraceNo = stPosParam.lNowTraceNo;
  stUplTraceSync.lCurUplNo ++;
  SaveUplTraceSync();
}

uint8_t BatchMagOnLineTrans(int *piTotalNum, uint8_t ucSendFalg)
{
	uint8_t		ucRet=0, sBuff[30], sField48[512];
	int		iCnt, iLength, iItemNum, k, iField48Len;
	int		aiInsertPoint[8];
	int uiStatus, iNowTrace=stPosParam.lNowTraceNo;
	NEWPOS_LOG_STRC	stLog;

	if( ucSendFalg==EMVLOG )
	{
		return OK;
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return OK; 
	}

	for(k=0; k<8; k++)
	{
		aiInsertPoint[k] = 60000;
	}

	iItemNum = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( stLog.iTransNo==POS_PREAUTH     		|| 
			stLog.iTransNo==POS_PREAUTH_VOID		|| stLog.iTransNo==POS_PREAUTH_ADD	|| 
			stLog.iTransNo==OFF_SALE        || stLog.iTransNo==OFF_ADJUST      ||
			stLog.iTransNo==ADJUST_TIP      || stLog.iTransNo==POS_REFUND      ||
			stLog.iTransNo==POS_OFF_CONFIRM || stLog.iTransNo==EC_REFUND )
		{
			continue;
		}
		
		if (stLog.ucSwipeFlag==CARD_INSERTED )
		{
			if (stLog.iTransNo != POS_SALE_VOID && stLog.iTransNo != POS_VOID_INSTAL
				&& stLog.iTransNo != POS_AUTH_CM && stLog.iTransNo != POS_AUTH_VOID)
			{
				continue;
			}
		}

		if (stLog.ucSwipeFlag & CARD_PASSIVE)
		{
			continue;
		}
		
		if( (ucSendFalg==RMBLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)!=0) )
		{
			continue;	
		}
		if( (ucSendFalg==FRNLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)==0) )
		{
			continue;	
		}

		if( stLog.ucBatchSendFlag==TRUE )
		{
			(*piTotalNum)++;
			continue;
		}

		uiStatus = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
		if (uiStatus > stPosParam.ucResendTimes)
		{
			(*piTotalNum)++;
			continue;
		}

		if (iItemNum == 0)//match UplTracePair,org trace no. or new trace no.
		{
 			gUplTrace = GetTxnPair(stLog.lTraceNo);//match the first tran trace no.
			if (gUplTrace == 0)//mismatch
			{
				iNowTrace = stPosParam.lNowTraceNo;
				SaveTxnPair(stLog.lTraceNo);
			}
			else
				iNowTrace = gUplTrace;
		}

		aiInsertPoint[iItemNum] = iCnt; 
		iItemNum++;

		iLength = (iItemNum-1)*40 + 2;
		if( memcmp(stLog.szCardUnit, "CUP", 3)==0 )
		{
			memcpy(&sField48[iLength], "00", 2);
		}
		else
		{
			memcpy(&sField48[iLength], "01", 2);
		}
		sprintf((char *)&sField48[iLength+2], "%06d", stLog.lTraceNo);
		memset(&sField48[iLength+8],'0',20);
		sprintf((char *)&sField48[iLength+8+(20-strlen((char *)stLog.szCardNo))], "%s", stLog.szCardNo);
		BcdToAsc0(&sField48[iLength+28], stLog.sAmount,12);
		
		if( iItemNum==8 )
		{
			*piTotalNum += iItemNum;
			iField48Len = iItemNum*40 + 2;
			sprintf((char *)sBuff, "%02d", iItemNum);
			memcpy(sField48, sBuff, 2);
			iItemNum = 0;	 

			stTemp.iTransNo = POS_BATCH_UP;
			PosCom.stTrans.iTransNo = POS_BATCH_UP;
			PosCom.stTrans.lTraceNo = iNowTrace;
			SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
			sprintf((char *)glSendPack.szField48, "%.*s", LEN_FIELD48, sField48);

			ucRet = SendRecvPacket();
			if( ucRet!=OK )
			{
				if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
				{
					return ucRet;
				}
				else
				{
					bRecvNOK = 1;
					for(k=0; k<8; k++)
					{
						if( aiInsertPoint[k]==60000 )
						{
							continue;
						}
						if (PosCom.bOnlineTxn)
						{
							uiStatus = stTransCtrl.sTxnRecIndexList[aiInsertPoint[k]];
							uiStatus++; 
							UpdataTxnCtrlStatus(aiInsertPoint[k],uiStatus); 
						}
					}
					continue;
				}
			}
			for(k=0; k<8; k++)
			{
				if( aiInsertPoint[k]==60000 )
				{
					break;
				}
				memset(&stLog, 0, sizeof(stLog));
				ucRet = LoadTranLog(aiInsertPoint[k], &stLog);
				if( ucRet!=OK )
				{
					return ucRet;
				}
				stLog.ucBatchSendFlag = TRUE;
				ucRet = UpdateTranLog(aiInsertPoint[k], &stLog);
				if( ucRet!=OK )
				{
					return ucRet;
				}
				aiInsertPoint[k] = 60000;
			}			
		}
	}

	if( iItemNum!=0 )
	{	
		*piTotalNum += iItemNum;
		iField48Len = iItemNum*40 + 2;
		sprintf((char *)sBuff, "%02d", iItemNum);
		memcpy(sField48, sBuff, 2);
		iItemNum = 0;	

		stTemp.iTransNo = POS_BATCH_UP;
		PosCom.stTrans.iTransNo = POS_BATCH_UP;
		PosCom.stTrans.lTraceNo = iNowTrace;
		SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
		sprintf((char *)glSendPack.szField48, "%.*s", LEN_FIELD48, sField48);

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else
			{
				bRecvNOK = 1;
				for(k=0; k<8; k++)
				{
					if( aiInsertPoint[k]==60000 )
					{
						continue;
					}
					if (PosCom.bOnlineTxn)
					{
						uiStatus = stTransCtrl.sTxnRecIndexList[aiInsertPoint[k]];
						uiStatus++;  
						UpdataTxnCtrlStatus(aiInsertPoint[k],uiStatus); 
					}
				}
				return ucRet;
			}
		}

		for(k=0; k<8; k++)
		{
			if( aiInsertPoint[k]==60000 )
			{
				break;
			}
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(aiInsertPoint[k], &stLog);
			if( ucRet!=OK )
			{
				return ucRet;
			}
			stLog.ucBatchSendFlag = TRUE;
			ucRet = UpdateTranLog(aiInsertPoint[k], &stLog);
			if( ucRet!=OK )
			{
				return ucRet;
			}
			aiInsertPoint[k] = 60000;
		}			
	}

	return ucRet;

}

uint8_t SendOffLineTrans(uint8_t flag, int maxSendTimes)	
{
	uint8_t		ucRet;
	int		iRecNo, iRecIndex;
	NEWPOS_LOG_STRC	stLog;
	uint32_t  uiStatus;
	int      iTotalNum;
	char     szBuffer[40];
	
	iTotalNum = GetOfflineTxnNums(TS_TXN_OFFLINE);
	for(iRecNo=0; iRecNo<iTotalNum; )
	{
		if (flag)
		{
			if( kbGetKeyMs(150) == KEY_CANCEL )
			{
				return NO_DISP;
			}
		}
		
		memset(&stLog, 0, sizeof(stLog));
		iRecIndex = GetMinSendTimesTxn(TS_TXN_OFFLINE);
		if (iRecIndex == -1)
		{
			return 0;
		}
			
		ucRet = LoadTranLog(iRecIndex, &stLog);
		if( ucRet )
		{
			return ucRet;
		}

		if(stLog.ucSendFlag || (stLog.iTransNo!=OFF_SALE && stLog.iTransNo!=OFF_ADJUST && stLog.iTransNo!=ADJUST_TIP))
		{
			uiStatus = stTransCtrl.sTxnRecIndexList[iRecIndex];
			uiStatus &= ~TS_TXN_OFFLINE;
			UpdataTxnCtrlStatus(iRecIndex,uiStatus);
			continue;
		}

		iRecNo++;
		lcdClrLine(2,7);
		DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "Upload Txn...");
		sprintf(szBuffer,"Upload %d ",iRecNo);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, szBuffer, szBuffer);
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "PLS WAIT....");
		lcdFlip();
		sysDelayMs(500);

		InitCommData();
		stTemp.iTransNo = stLog.iTransNo;
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		PosCom.ucSwipedFlag = stLog.ucSwipeFlag;
		SetCommReqField((uint8_t *)"0220", (uint8_t *)"000000");
		sprintf((char *)glSendPack.szAuthCode, "%.6s", stLog.szAuthNo);
		sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s%s%s",
				PosCom.stTrans.lOldBatchNumber, PosCom.stTrans.lOldTraceNo,
				PosCom.stTrans.szOldTxnDate, stLog.szAuthMode, stLog.szAuthUnit);
		sprintf((char *)glSendPack.szField63, "%s", stLog.szCardUnit);
		if( stLog.iTransNo==ADJUST_TIP || stLog.iTransNo==OFF_ADJUST || stLog.iTransNo==OFF_SALE )
		{
			sprintf((char *)glSendPack.szRRN, "%.12s", stLog.szSysReferNo);
			if (stLog.iTransNo==OFF_SALE)
			{
				memcpy((char *)glSendPack.szOrginalMsg,"0000000000000000",16);
			}
		}

		PosCom.bOnlineTxn = FALSE;
		ucRet = SendRecvPacket();
		if( ucRet!=OK || strlen((char*)PosCom.szRespCode)==0 )
		{
			if(ucRet == E_ERR_CONNECT ||ucRet ==  E_MAKE_PACKET || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			if (PosCom.bOnlineTxn)
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
				uiStatus++; 
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus); 
			}
			stLog.ucSendFail = TS_NOT_RECV;
			ucRet = UpdateTranLog(iRecIndex, &stLog);
			if( ucRet!=OK )
			{
				return ucRet;
			}
		}
		else
		{
			if( memcmp(PosCom.szRespCode, "00", 2)==0 || memcmp(PosCom.szRespCode, "94", 2)==0 )
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
				uiStatus &= ~(TS_TXN_OFFLINE);
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);
				stLog.ucSendFlag = TRUE;
				stLog.ucSendFail = FALSE;
				sprintf((char *)stLog.szSysReferNo, "%.12s", PosCom.stTrans.szSysReferNo);
				strcpy((char *)stLog.szIssuerBankId,(char *)PosCom.stTrans.szIssuerBankId);
				strcpy((char *)stLog.szRecvBankId,(char *)PosCom.stTrans.szRecvBankId);
				strcpy((char *)stLog.szCenterId,(char *)PosCom.stTrans.szCenterId);
				strcpy((char *)stLog.szSettDate,(char *)PosCom.stTrans.szSettDate);
				strcpy((char *)stLog.szIssuerResp,(char *)PosCom.stTrans.szIssuerResp);
				strcpy((char *)stLog.szCenterResp,(char *)PosCom.stTrans.szCenterResp);
				strcpy((char *)stLog.szRecvBankResp,(char *)PosCom.stTrans.szRecvBankResp);
				strcpy((char *)stLog.szAuthNo, (char *)PosCom.stTrans.szAuthNo);
				strcpy((char *)stLog.szCurrencyCode, (char *)PosCom.stTrans.szCurrencyCode);
				ucRet = UpdateTranLog(iRecIndex, &stLog);
				if( ucRet!=OK )
				{
					return ucRet;
				}
			}
			else
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
				uiStatus |= 0x0F; 
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);
				stLog.ucSendFail = TS_NOT_CODE;
				ucRet = UpdateTranLog(iRecIndex, &stLog);
				if( ucRet!=OK )
				{
					return ucRet;
				}
			}
		}
	}	//for

	return OK;
}

/*选择卡组织*/
uint8_t SelectCardUnit(uint8_t *szCardUnit)
{
	int iKey;

	kbFlush();
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "   ISSURE CODE  ");
	if (gstPosCapability.uiScreenType)
	{
		lcdDisplay(0, 4, DISP_CFONT, "1 CUP 2 VIS 3 MCC");
		lcdDisplay(0, 6, DISP_CFONT, "4 MAE 5 JCB 6 DCC");
		lcdDisplay(0, 8, DISP_CFONT, "7 AMX            ");
	}
	else
	{
		lcdDisplay(0, 4, DISP_ASCII, "1 CUP 2 VIS 3 MCC");
		lcdDisplay(0, 5, DISP_ASCII, "4 MAE 5 JCB 6 DCC");
		lcdDisplay(0, 6, DISP_ASCII, "7 AMX            ");
	}

	lcdFlip();
	while( 1 )
	{
		iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if( (iKey>=KEY1 && iKey<=KEY7) || iKey==KEY_ENTER || iKey==KEY_CANCEL || iKey==KEY_TIMEOUT)
		{
			break;
		}
	}
	switch(iKey) {  
	case KEY1:
	case KEY_ENTER:
		strcpy((char *)szCardUnit, "CUP");
		break;

	case KEY2:
		strcpy((char *)szCardUnit, "VIS");
		break;

	case KEY3:
		strcpy((char *)szCardUnit, "MCC");
		break;

	case KEY4:
		strcpy((char *)szCardUnit, "MAE");
		break;

	case KEY5:
		strcpy((char *)szCardUnit, "JCB");
		break;

	case KEY6:
		strcpy((char *)szCardUnit, "DCC");
		break;

	case KEY7:
		strcpy((char *)szCardUnit, "AMX");
		break;

	case KEY_CANCEL:
	case KEY_TIMEOUT:
		return E_TRANS_CANCEL;

	default:
		break;
	}

	return(OK);
}

uint8_t BatchSendOffTrans1(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt, uiStatus;
	NEWPOS_LOG_STRC	stLog;

	if( ucSendFlag==EMVLOG )
	{
		return OK;
	}

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( stLog.iTransNo!=OFF_SALE   && stLog.iTransNo!=OFF_ADJUST &&
		    stLog.iTransNo!=ADJUST_TIP )	
		{
			continue;
		}

		if( (ucSendFlag==RMBLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)!=0) )
		{
			continue;	
		}
		if( (ucSendFlag==FRNLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)==0) )
		{
			continue;	
		}
		if( stLog.ucBatchSendFlag==TRUE )
		{
			(*piTotalNum)++;
			continue;
		}

		uiStatus = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
		if (uiStatus > stPosParam.ucResendTimes)
		{
			(*piTotalNum)++;
			continue;
		}

		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		switch( stLog.iTransNo ){
		case OFF_SALE:
			stTemp.iTransNo = OFF_SALE_BAT;
			break;

		case OFF_ADJUST:
			stTemp.iTransNo = OFF_ADJUST_BAT;
			break;

		case ADJUST_TIP:
			stTemp.iTransNo = ADJUST_TIP_BAT;
			break;


		default:
			continue;
		}

		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		PosCom.ucSwipedFlag = stLog.ucSwipeFlag; 
		SetCommReqField((uint8_t *)"0320", (uint8_t *)(stLog.iTransNo==POS_REFUND ? "200000" : "000000"));
		if( stTemp.iTransNo==OFF_ADJUST_BAT || stTemp.iTransNo==ADJUST_TIP_BAT )
		{
			sprintf((char *)glSendPack.szRRN, "%.12s", stLog.szSysReferNo);
		}

		if( (stLog.iTransNo==OFF_SALE) && (memcmp(stLog.szAuthMode, "02", 2)==0) )
		{
			glSendPack.szAuthCode[0] = 0;
		}
		else
		{
			sprintf((char *)glSendPack.szAuthCode, "%.6s", stLog.szAuthNo);
		}
		
		sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s%s%s",
				PosCom.stTrans.lOldBatchNumber, PosCom.stTrans.lOldTraceNo,
				PosCom.stTrans.szOldTxnDate, stLog.szAuthMode, stLog.szAuthUnit);
		if( stLog.iTransNo==OFF_SALE || stLog.iTransNo==OFF_ADJUST || stLog.iTransNo==ADJUST_TIP )
		{
			sprintf((char *)glSendPack.szField63, "%3s", stLog.szCardUnit);
			glSendPack.szField60[13] = 0;
			if( stLog.iTransNo==OFF_SALE )
				memcpy((char *)glSendPack.szOrginalMsg,"0000000000000000",16);
		}

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;
				if (PosCom.bOnlineTxn)
				{
					uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
					uiStatus++; 
					UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus); 
				}
				continue;
			}			
		}

		stLog.ucBatchSendFlag = TRUE;
		ucRet = UpdateTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}

	return ucRet;
}

uint8_t BatchSendOffTrans2_Refund(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt, uiStatus;
	NEWPOS_LOG_STRC	stLog;

	if( ucSendFlag==EMVLOG )
	{
		return OK;
	}

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( stLog.iTransNo!=POS_REFUND && 
			stLog.iTransNo!=EC_REFUND)				
		{
			continue;
		}

		if( (ucSendFlag==RMBLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)!=0) )
		{
			continue;	
		}
		if( (ucSendFlag==FRNLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)==0) )
		{
			continue;	
		}
		if( stLog.ucBatchSendFlag==TRUE )
		{	
			(*piTotalNum)++;
			continue;
		}

		uiStatus = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
		if (uiStatus > stPosParam.ucResendTimes)
		{
			(*piTotalNum)++;
			continue;
		}

		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		if( stLog.iTransNo==POS_REFUND || stLog.iTransNo==POS_OFF_CONFIRM )
		{
			CheckRefundDataFile(stLog.lTraceNo); 
		}
		switch( stLog.iTransNo ){

		case POS_REFUND:
		case EC_REFUND:				
			stTemp.iTransNo = POS_REFUND_BAT;
			break;

		case POS_OFF_CONFIRM:
			stTemp.iTransNo = OFF_CONFIRM_BAT;
			break;

		default:
			continue;
		}

		PosCom.stTrans.iOldTransNo = PosCom.stTrans.iTransNo;	
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		PosCom.ucSwipedFlag = stLog.ucSwipeFlag; 
		SetCommReqField((uint8_t *)"0320", (uint8_t *)(stTemp.iTransNo == POS_REFUND_BAT ? "200000" : "000000"));	
		if( stTemp.iTransNo==POS_REFUND_BAT )
		{
			sprintf((char *)glSendPack.szRRN, "%.12s", stLog.szSysReferNo);
		}

		if( (stLog.iTransNo==OFF_SALE) && (memcmp(stLog.szAuthMode, "02", 2)==0) )
		{
			glSendPack.szAuthCode[0] = 0;
		}
		else
		{
			sprintf((char *)glSendPack.szAuthCode, "%.6s", stLog.szAuthNo);
		}
		
		sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s%s%s",
				PosCom.stTrans.lOldBatchNumber, PosCom.stTrans.lOldTraceNo,
				PosCom.stTrans.szOldTxnDate, stLog.szAuthMode, stLog.szAuthUnit);
		strcpy((char *)glSendPack.szField63, (char *)stLog.szCardUnit);
		if (!strlen((char *)stLog.szCardUnit))
		{
			strcpy((char *)glSendPack.szField63, "000");
		}

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else
			{
				bRecvNOK = 1;
				if (PosCom.bOnlineTxn)
				{
					uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
					uiStatus++; 
					UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
				}
				continue;
			}
		}

		stLog.ucBatchSendFlag = TRUE;
		ucRet = UpdateTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}

	return ucRet;
}


uint8_t BatchSendOffTrans2_CM(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt, uiStatus;
	NEWPOS_LOG_STRC	stLog;

	if( ucSendFlag==EMVLOG )
	{
		return OK;
	}

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( stLog.iTransNo!=POS_OFF_CONFIRM )
		{
			continue;
		}

		if( (ucSendFlag==RMBLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)!=0) )
		{
			continue;	
		}
		if( (ucSendFlag==FRNLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)==0) )
		{
			continue;	
		}
		if( stLog.ucBatchSendFlag==TRUE )
		{	
			(*piTotalNum)++;
			continue;
		}

		uiStatus = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
		if (uiStatus > stPosParam.ucResendTimes)
		{
			(*piTotalNum)++;
			continue;
		}

		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		if( stLog.iTransNo==POS_REFUND || stLog.iTransNo==POS_OFF_CONFIRM )
		{
			CheckRefundDataFile(stLog.lTraceNo);
		}
		switch( stLog.iTransNo ){

		case POS_REFUND:
			stTemp.iTransNo = POS_REFUND_BAT;
			break;

		case POS_OFF_CONFIRM:
			stTemp.iTransNo = OFF_CONFIRM_BAT;
			break;

		default:
			continue;
		}

		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		PosCom.ucSwipedFlag = stLog.ucSwipeFlag;
		SetCommReqField((uint8_t *)"0320", (uint8_t *)(stLog.iTransNo==POS_REFUND ? "200000" : "000000"));
		if( stTemp.iTransNo==POS_REFUND_BAT )
		{
			sprintf((char *)glSendPack.szRRN, "%.12s", stLog.szSysReferNo);
		}

		if( (stLog.iTransNo==OFF_SALE) && (memcmp(stLog.szAuthMode, "02", 2)==0) )
		{
			glSendPack.szAuthCode[0] = 0;
		}
		else
		{
			sprintf((char *)glSendPack.szAuthCode, "%.6s", stLog.szAuthNo);
		}
		
		sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s%s%s",
				PosCom.stTrans.lOldBatchNumber, PosCom.stTrans.lOldTraceNo,
				PosCom.stTrans.szOldTxnDate, stLog.szAuthMode, stLog.szAuthUnit);
		if (stLog.iTransNo == POS_OFF_CONFIRM)
		{
			glSendPack.szField60[13] = 0;
		}

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;
				if (PosCom.bOnlineTxn)
				{
					uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
					uiStatus++;  
					UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
				}
				continue;
			}			
		}

		stLog.ucBatchSendFlag = TRUE;
		ucRet = UpdateTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}

	return ucRet;
}


// check out get the old field37 Reference
uint8_t GetOldReferenceNo(void)
{
	uint8_t buf[50] = {0};
	int iRet;

	lcdClrLine(2,7);
	DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "NUMERO DE CARGO(RRM)");
	lcdFlip();
	lcdGoto(72, 6);
	iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 1, 17, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet<0 )
		return (E_TRANS_CANCEL);
	memset(PosCom.stTrans.szSysReferNo,0,sizeof(PosCom.stTrans.szSysReferNo));
	strcpy((char*)PosCom.stTrans.szSysReferNo,(char *)buf);
	return OK;
}



uint8_t GetOldReference(void)
{
	uint8_t buf[17] = {0};
	int iRet;

	while(1)
	{
		lcdClrLine(2,7);
		if( PosCom.stTrans.TransFlag == NETPAY_REFUND)
		{
			DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "DEVOLUCION");
		}
		
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "NUMERO DE CARGO(PRM)");
		lcdFlip();
		lcdGoto(72, 6);
		iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 0, 16, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return (E_TRANS_CANCEL);
		else if( iRet<1 || iRet>16 )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INGRESE ENTRE");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "1 Y 16 DIGITOS");
			lcdFlip();
			FailBeep();
			sysDelayMs(1500);
			continue;
		}
		else
		{
			break;
		}
	}

	strcpy((char*)PosCom.stTrans.szSysReferNo,(char*)buf);

	return OK;
}


uint8_t GetOldTraceNo(void)
{
	uint8_t buf[10];
	int iRet;

	lcdClrLine(2,7);
	if(PosCom.stTrans.TransFlag == POS_AUTH_CM)
	{
		DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "CONFIRMACION PROPINA");
	}
	else if( PosCom.stTrans.TransFlag == POS_REFUND)
	{
		DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "CANCELACION");
	}
	else if( stTemp.iTransNo==CHECK_OUT )
	{
		DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "CHECK OUT");
	}
	
	DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "NUMERO DE CARGO");
	lcdFlip();
	lcdGoto(72, 6);
	iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 1, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet<0 )
		return (E_TRANS_CANCEL);
	PosCom.stTrans.lOldTraceNo = atol((char *)buf);
	sprintf((char *)PosCom.stTrans.szPrtOldPosSer, "%06ld", atol((char *)buf));	
	return OK;
}

uint8_t GetSysRefNo(void)
{
	uint8_t	szbuf[13];
	int   iRet;

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT REF.NO:");
	lcdFlip();
	lcdGoto(24, 6);
	iRet = PubGetString(ALPHA_IN,12, 12,szbuf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
	{
		return E_TRANS_CANCEL;
	}

	sprintf((char *)PosCom.stTrans.szSysReferNo, "%.12s", szbuf);	
	strcpy((char *)PosCom.stTrans.szPrtOldHostSer, (char *)PosCom.stTrans.szSysReferNo);	
	return OK;
}


uint8_t GetOrignTxnDate(void)
{
	uint8_t buf[6];
	uint8_t chkDate[9];
	int iRet;

	while(1) 
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT DATE:");
		DispMulLanguageString(0, 4, DISP_CFONT, NULL, "(MMDD)");
		lcdFlip();
		lcdGoto(80, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet!=4 )	
			return (E_TRANS_CANCEL);
		memcpy(chkDate, stTemp.szDate, 4);
		memcpy(chkDate+4, buf, 4);
		if( CheckDate((char *)chkDate)==0 )
		{
			strcpy((char *)PosCom.stTrans.szOldTxnDate, (char *)buf);
			break;
		}
		else
		{
			FailBeep();
			lcdClrLine(4, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT, NULL, "FORMAT DATE");
			lcdFlip();
			kbGetKeyMs(2000);
		}
	}
	
	return OK;
}

uint8_t GetAuthNo(void)
{
	uint8_t buf[10];
	int iRet;
	
	memset((char *)buf,0,sizeof(buf));
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT AUTH NO:");
	lcdFlip();
	iRet = PubGetString(ALPHA_IN,1, 6,buf,stPosParam.ucOprtLimitTime,0);
	if( iRet!=0 )
		return (E_TRANS_CANCEL);

	memset((char *)PosCom.stTrans.szAuthNo,0,sizeof((char *)PosCom.stTrans.szAuthNo));
	memset((char *)PosCom.stTrans.szAuthNo,' ',sizeof((char *)PosCom.stTrans.szAuthNo)-1);
	memcpy((char *)PosCom.stTrans.szAuthNo, buf,strlen((char *)buf));
	sprintf((char *)PosCom.stTrans.szPrtOldAuthNo, "%s", PosCom.stTrans.szAuthNo);

	return OK;
}


void UpdateParameters(uint8_t *psPosParam, uint16_t iMaxLen)
{
	uint8_t macbuf[9];
	int i, j;
	int bm;
	uint8_t tmpbuf[45];
	uint8_t env_buf[24];

	i = 0;
	do 
	{
		memcpy(macbuf, psPosParam+i, 2);
		macbuf[2] = 0;
		bm = atoi((char *)macbuf);
		i += 2;
		switch(bm)
		{
		case 11:
			i = i+2;
			break;

		case 12:
			memcpy(macbuf, psPosParam+i, 2);
			macbuf[2] = 0;
			stPosParam.ucCommWaitTime = atoi((char *)macbuf); 
			i += 2;
			break;

		case 13:
			stPosParam.ucDialRetryTimes = psPosParam[i] - 0x30;
			sprintf((char*)env_buf, "%d", (int)stPosParam.ucDialRetryTimes);
			i++;
			break;

		case 14:
			memcpy(tmpbuf, psPosParam+i, 14);
			for(j=0; j<14; j++)
			{
				if( tmpbuf[j]==0x20 )
				{
					break;
				}
				stPosParam.szPhoneNum1[j] = tmpbuf[j];
			}					
			stPosParam.szPhoneNum1[j] = 0;
			i += 14;
			break;

		case 15:
			memcpy(tmpbuf, psPosParam+i, 14);
			for(j=0; j<14; j++)
			{
				if( tmpbuf[j]==0x20 )
				{
					break;
				}
				stPosParam.szPhoneNum2[j] = tmpbuf[j];
			}					
			stPosParam.szPhoneNum2[j] = 0;					
			i += 14;
			break;

		case 16:
			memcpy(tmpbuf, psPosParam+i, 14);
			for(j=0; j<14; j++)
			{
				if( tmpbuf[j]==0x20 )
				{
					break;
				}
				stPosParam.szPhoneNum3[j] = tmpbuf[j];
			}					
			stPosParam.szPhoneNum3[j] = 0;
			i += 14;
			break;

		case 17:
			memcpy(tmpbuf, psPosParam+i, 14);
			for(j=0; j<14; j++)
			{
				if( tmpbuf[j]==0x20 )
				{
					break;
				}
				stPosParam.szMPhone[j] = tmpbuf[j];
			}					
			stPosParam.szMPhone[j] = 0;
			i += 14;
			break;

		case 18:
			stPosParam.ucTipOpen = psPosParam[i];
			env_buf[0] = psPosParam[i];
			env_buf[1] = 0;
			i++;
			break;

		case 19:
			memcpy(macbuf, psPosParam+i, 2);
			macbuf[2] = 0;
			stPosParam.ucTipper = atoi((char *)macbuf);
			sprintf((char*)env_buf, "%d", (int)stPosParam.ucTipper);			
			i += 2;
			break;

		case 20:
			stPosParam.ucManualInput = psPosParam[i];
			env_buf[0] = stPosParam.ucManualInput;
			env_buf[1] = 0;
			i++;
			break;

		case 21:
			stPosParam.ucAutoLogoff = psPosParam[i];
			env_buf[0] = stPosParam.ucAutoLogoff;
			env_buf[1] = 0;
			i++;
			break;

		case 22:
			memcpy(tmpbuf, psPosParam+i, 40);
			for(j=0; j<40; j++)
			{
				if( tmpbuf[j]==0x20 )
				{							
					break;
				}
				stPosParam.szEngName[j] = tmpbuf[j];
			}
			stPosParam.szEngName[j] = 0;
			i += 40;
			break;

		case 23:
			stPosParam.ucResendTimes = psPosParam[i] - 0x30;
			sprintf((char*)env_buf, "%d", (int)stPosParam.ucResendTimes);
			i++;
			break;

		case 24:
			stPosParam.ucSendOffFlag = psPosParam[i];
			env_buf[0] = stPosParam.ucSendOffFlag;
			env_buf[1] = 0;
			i++;
			break;

		case 25:
			stPosParam.ucKeyIndex = psPosParam[i] - 0x30;
			sprintf((char*)env_buf, "%d", (int) stPosParam.ucKeyIndex);
			i++;
			break;

		case 26:
			memcpy(stPosParam.sTransOpen, psPosParam+i, 4);
			BcdToAsc(env_buf, stPosParam.sTransOpen, 8);
			env_buf[8] = 0x00; 
			i += 4;
			break;
		}
	} while( i<(int)iMaxLen );

	SaveAppParam();
	stTransCtrl.ucLoadTask = 0;
	SaveCtrlParam();
}

uint8_t GetAuthType(void)
{
	uint8_t result;
	uint8_t inBuf[17];
	int iKey, iRet;

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "   AUTH MODE    ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "1 POS   2 PHONE");
	DispMulLanguageString(0, 6, DISP_CFONT, NULL, "3 DAI-AUTH     ");
	lcdFlip();
	do {
		iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if( iKey==KEY_CANCEL || iKey==KEY_TIMEOUT)
			return (E_TRANS_CANCEL);
	}while( iKey!=KEY1 && iKey!=KEY2 && iKey!=KEY3 );
	
	if( iKey==KEY1 )
		strcpy((char *)PosCom.stTrans.szAuthMode, "00");
	if( iKey==KEY3 )
	{
		if (stPosParam.ucOfflineSettleFlag == PARAM_CLOSE)
		{
			lcdCls();	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " NOT SUPPORT ");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(2000);					
			return NO_DISP;
		}
		strcpy((char *)PosCom.stTrans.szAuthMode, "02");
	}
	if( iKey==KEY2 )
	{
		strcpy((char *)PosCom.stTrans.szAuthMode, "01");
		while(1)
		{
			memset(inBuf,0,sizeof(inBuf));
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT, NULL, "AUTH CODE");
			iRet = PubGetString(ALPHA_IN,0,11,inBuf,stPosParam.ucOprtLimitTime,0);
			if( iRet != 0 )	
				return (E_TRANS_CANCEL);
			if (strlen((char *)inBuf)==0 || strlen((char *)inBuf) == 11)
			{
				strcpy((char *)PosCom.stTrans.szAuthUnit, (char *)inBuf);
				break;
			}
			lcdClrLine(2,7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "ERROR LENGTH");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "TRY AGAIN");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(3);
		}
	}
	if( iKey==KEY1 || iKey==KEY2 )
	{
		result = GetAuthNo();
		if( result!=OK ) 
			return result;
	}

	return OK;
}

uint8_t GetTxnTableIndex(int txn)
{
	uint8_t	i;

	for(i=0; TrTable[i].iTransNo!=-1; i++)
	{
		if( txn==TrTable[i].iTransNo )
		{
			return i;
		}
	}

	FailBeep();
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "GET TABLE FAIL");
	lcdFlip();
	while(1){
		sysDelayMs(10); // Avoid CPU resources occupation
	};

	return 0xff;
}

void OnlineDownOrUp(void)
{
	uint8_t	ucRet,iCnt;
	int iCAPK=0,iAID=0;	
	
	switch( stTemp.sPacketHead[5] )
	{
	case '1':
		PosDownLoadParam();
		break;

	case '2':
		PosUpLoadParam();
		break;

	case '3':
		stTransCtrl.ucLogonFlag = LOGGEDOFF;
		SaveCtrlParam();
		for (iCnt=0; iCnt<stPosParam.ucResendTimes+1;iCnt++)
		{
			ucRet = PosOnlineLogon(2);
			if ((ucRet==E_PINPAD_KEY || (PosCom.bOnlineTxn && strlen((char*)PosCom.szRespCode)==0)))
			{
				continue;
			}
			else
			{
				if( stTransCtrl.ucLoadTask==1 )
				{
					PosDownLoadParam();
				}
				return;
			}
		}
		if(iCnt == stPosParam.ucResendTimes+1)
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "LOAD KEY FAIL");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LOAD MASTER KEY");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(5);
		}
		break;

	case '4':	
		iCAPK = 1;
		ucRet = DownLoadCAPK(FALSE);
		if( ucRet!=OK )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "DOWNLOAD CAPK FAIL");
			lcdFlip();
			sysDelayMs(1000);
		}
		break;

	case '5':	
		iAID = 1;
		ucRet = DownLoadEmvParam(FALSE);
		if( ucRet!=OK )
		{
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "DOWNLOAD AID FAIL");
			lcdFlip();
			sysDelayMs(1000);
		}
		break;
		
	case '6':
		stTransCtrl.ucDownAppFlag = 1;
		SaveCtrlParam();
		break;

	case '7':
		ucRet = TransDownloadBlkCard();
		break;

	default:
		break;
	}

	if( stTransCtrl.ucLoadTask==1 )
	{
		PosDownLoadParam();
	}

	if( stPosParam.ucEmvSupport!=PARAM_OPEN)
		return ;
	
	if(stTransCtrl.bEmvCapkLoaded==FALSE && iCAPK==0)
	{			
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DownLoad CAPK ");
		if( DownLoadCAPK(FALSE) != OK )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "DOWNLOAD CAPK FAIL");
			lcdFlip();
			sysDelayMs(3000);
			return ;
		}
	}

	if(stTransCtrl.bEmvAppLoaded==FALSE && iAID==0)
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,  NULL, "  DownLoad AID  ");
		ucRet = DownLoadEmvParam(FALSE);
		if( ucRet!=OK )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "DOWNLOAD AID FAIL");
			lcdFlip();
			sysDelayMs(3000);
			return ;
		}
	}
	
	return;
}

uint8_t InputCardNoSale(void)
{
	uint8_t ucRet;
	char szField63BuffTemp[999] = {0};

	PosCom.stTrans.iTransNo = stTemp.iTransNo;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "VENTA FORZADA");
	lcdFlip();
	
	ucRet = PosGetCard(CARD_KEYIN);
	if( ucRet!= OK)
	{
		return ucRet;
	}

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");
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
								
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);

	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	
	
	return OK;
}

uint8_t NeedPIN(int iTxnNo)
{
	if( iTxnNo==/*POS_SALE_VOID*/POS_REFUND || iTxnNo == POS_VOID_INSTAL)
	{	
		return (stPosParam.ucVoidPin==PARAM_OPEN);
	}
	if( iTxnNo==POS_PREAUTH_VOID )
	{
		return (stPosParam.ucPreVoidPin==PARAM_OPEN);
	}

	if( iTxnNo==POS_AUTH_VOID )
	{
		return (stPosParam.ucPreComVoidPin==PARAM_OPEN);
	}

	if( iTxnNo==POS_AUTH_CM )
	{
		return (stPosParam.ucPreComPin==PARAM_OPEN);
	}

	if (iTxnNo == EC_TOPUP_SPEC || iTxnNo == EC_TOPUP_NORAML)
	{
		return TRUE;
	}

	if( PosCom.ucSwipedFlag==CARD_INSERTED && stTemp.iTransNo== POS_QUE)
	{
		if (!PosCom.ucPinEntry)
		{
			return FALSE;
		}
	}

	if( PosCom.ucSwipedFlag==CARD_INSERTED || ((PosCom.ucSwipedFlag==CARD_PASSIVE) && (PosCom.stTrans.ucQPBOCorPBOC==1))
		|| stTemp.iTransNo==POS_OFF_CONFIRM )
	{
		return FALSE;
	}

	return TRUE;
}

uint8_t ICNeedPIN(int iTxnNo)
{
	if( iTxnNo==POS_SALE_VOID )
	{	
		return (stPosParam.ucVoidPin==PARAM_OPEN);
	}
	if( iTxnNo==POS_PREAUTH_VOID )
	{
		return (stPosParam.ucPreVoidPin==PARAM_OPEN);
	}

	if( iTxnNo==POS_AUTH_VOID )
	{
		return (stPosParam.ucPreComVoidPin==PARAM_OPEN);
	}

	if( iTxnNo==POS_AUTH_CM )
	{
		return (stPosParam.ucPreComPin==PARAM_OPEN);
	}

	return TRUE;
}


uint8_t PrintSelectMoneyType(void)
{
	int iKey = 0;

	if( stPosParam.szSupportMultMoneyFlag==PARAM_OPEN )
	{
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "MONEDA DE TRANSACCIONES:");	
		DispMulLanguageString(55, 12, DISP_CFONT, NULL, "                              1-PESOS");
		DispMulLanguageString(55, 14, DISP_CFONT, NULL, "                              2-DOLARES");
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while( iKey != KEY_ENTER   && iKey != KEY_CANCEL  && 
				iKey != KEY_TIMEOUT && iKey != KEY1 	&&
				iKey != KEY2);
		
		
		if (iKey == KEY_CANCEL ) {
			return E_TRANS_CANCEL;
		}
		else if ( iKey == KEY_TIMEOUT ) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			PosCom.stTrans.szMoneyType = PARAM_PESEO;
		}
		else if ( iKey == KEY2){
			PosCom.stTrans.szMoneyType = PARAM_DOLARES;
		}
	}
	else
	{
		if( stPosParam.szMoneyTypeFlag==PARAM_PESEO)
		{
			PosCom.stTrans.szMoneyType = PARAM_PESEO;
		}
		else
		{
			PosCom.stTrans.szMoneyType = PARAM_DOLARES;
			
		}
	}

	
	emv_get_mck_configure(&tEMVCoreInit);	
	if( PosCom.stTrans.szMoneyType==PARAM_PESEO) 
	{
		memcpy(tEMVCoreInit.terminal_country_code,   "\x04\x84", 2);
		memcpy(tEMVCoreInit.transacion_currency_code, "\x04\x84", 2);
		memcpy(tEMVCoreInit.refer_currency_code, "\x04\x84", 2);
	}
	else
	{
		memcpy(tEMVCoreInit.terminal_country_code,   "\x08\x40", 2);
		memcpy(tEMVCoreInit.transacion_currency_code, "\x08\x40", 2);
		memcpy(tEMVCoreInit.refer_currency_code, "\x08\x40", 2);
	}
	emv_set_core_init_parameter(&tEMVCoreInit);
	
	return OK;
}

uint8_t SelectMoneyType(void)
{
	int iKey = 0;

	if( stPosParam.szSupportMultMoneyFlag==PARAM_OPEN )
	{
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "SELECCIONE MONEDA:");
		DispMulLanguageString(55, 12, DISP_CFONT, NULL, "                              1-PESOS");
		DispMulLanguageString(55, 14, DISP_CFONT, NULL, "                              2-DOLARES");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		do {
			iKey = PubWaitKey(60);
		}while( iKey != KEY_ENTER   && iKey != KEY_CANCEL  && 
				iKey != KEY_TIMEOUT && iKey != KEY1 	&&
				iKey != KEY2);
		
		
		if (iKey == KEY_CANCEL ) {
			return E_TRANS_CANCEL;
		}
		else if ( iKey == KEY_TIMEOUT ) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			PosCom.stTrans.szMoneyType = PARAM_PESEO;
		}
		else if ( iKey == KEY2){
			PosCom.stTrans.szMoneyType = PARAM_DOLARES;
		}
	}
	else
	{
		if( stPosParam.szMoneyTypeFlag==PARAM_PESEO)
		{
			PosCom.stTrans.szMoneyType = PARAM_PESEO;
		}
		else
		{
			PosCom.stTrans.szMoneyType = PARAM_DOLARES;
			
		}
	}

	
	emv_get_mck_configure(&tEMVCoreInit);	
	if( PosCom.stTrans.szMoneyType==PARAM_PESEO) 
	{
		memcpy(tEMVCoreInit.terminal_country_code,   "\x04\x84", 2);
		memcpy(tEMVCoreInit.transacion_currency_code, "\x04\x84", 2);
		memcpy(tEMVCoreInit.refer_currency_code, "\x04\x84", 2);
	}
	else
	{
		memcpy(tEMVCoreInit.terminal_country_code,   "\x08\x40", 2);
		memcpy(tEMVCoreInit.transacion_currency_code, "\x08\x40", 2);
		memcpy(tEMVCoreInit.refer_currency_code, "\x08\x40", 2);
	}
	emv_set_core_init_parameter(&tEMVCoreInit);
	
	return OK;
}


uint8_t SelectSaleType(void)
{
	int iKey = 0;

	if( stPosParam.No_Promotions == PARAM_OPEN && 
		stPosParam.Costom_Promotions == PARAM_OPEN )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Parametro incalido");
		lcdFlip();
		FailBeep();
		PubWaitKey(2);
		return NO_DISP;
	}
	
	lcdCls();
	DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "TIPO DE VENTA?");
	DispMulLanguageString(0, 4, DISP_HFONT16, NULL, "Indique una respuesta");
	DispMulLanguageString(40, 7, DISP_HFONT16, NULL, "1-VENTA NORMAL");
	lcdFlip();
	if( stPosParam.No_Promotions == PARAM_CLOSE)
	{
		DispMulLanguageString(40, 8, DISP_HFONT16, NULL, "2-VENTA A MESES SIN INTERESES");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);
		
		
		if (iKey == KEY_CANCEL ) {
			return E_TRANS_CANCEL;
		}
		else if ( iKey == KEY_TIMEOUT ) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			return 101;
		}
		else if(iKey == KEY2){
			return 102;
		}
	}
	else
	{
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1);
		
		if (iKey == KEY_CANCEL ) {
			return E_TRANS_CANCEL;
		}
		else if ( iKey == KEY_TIMEOUT ) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			return 101;
		}
	}
	
	return OK;
}



uint8_t SelectInteresesSaleType(void)
{
	PosCom.stTrans.szintereseMonth = 0;
	lcdCls();
	DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "INDIQUE LA PROMOCION");
	
	PosCom.stTrans.szintereseMonth = MesesMenu();
	if( PosCom.stTrans.szintereseMonth < 0 )
	{
		return E_TRANS_CANCEL;
	}

	PosCom.stTrans.TransFlag = INTERESES_SALE;
	PosCom.stTrans.iTransNo  = POS_SALE;
	stTemp.iTransNo = POS_SALE;
	return OK;
}


uint8_t SaleTranGetData(void)
{
	uint8_t ucRet;
	int     iRet;	
	char  showbuf[50] ={0};
	
	stPosParam.ucEmvSupport = PARAM_OPEN;
	
	lcdCls();
	if (PosCom.ucPBOCFlag==1)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "E-CASH");
	}
	else if( PosCom.stTrans.TransFlag == PAYMENT_WITH_CARD)
	{
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "Pago a Netpay con Tarjeta");
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	else if( stTemp.iTransNo== CHECK_IN )
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   CHECK IN    ");
	}
	else if( stTemp.iTransNo== CASH_ADVACNE)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   RETIRO USD    ");
	}
	else if(stTemp.iTransNo ==PURSE_SALE)
	{
		switch(PosCom.stTrans.TransFlag)
		{
			case PURSE_SODEXO:
				sprintf(TitelName,"   %s   ","VENTA  SODEXO");
				break;
			case PURSE_PUNTO:
				sprintf(TitelName,"   %s   ","VENTA  PUNTO");
				break;
			case PURSE_TODITO:
				sprintf(TitelName,"   %s   ","VENTA  TODITO");
				break;
			case PURSE_EDENRED:
				sprintf(TitelName,"   %s   ","VENTA  EDENRED");
				break;
			default:
				sprintf(TitelName,"   %s   ","VENTA");
				break;
		}		
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, TitelName);
	}
	else
	{
		sprintf(TitelName,"   %s   ","VENTA");
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL,TitelName );
	}

	if (PosCom.ucPBOCFlag==1)
	{
		if( stPosParam.ucEmvSupport==PARAM_OPEN )
			ucRet = PosGetCard(CARD_INSERTED|CARD_PASSIVE);
		else
			ucRet = PosGetCard(CARD_PASSIVE);
	}
	else if (PosCom.ucPBOCFlag==2)
	{
		if( stPosParam.ucEmvSupport==PARAM_OPEN )
		{
			ucRet = PosGetCard(CARD_INSERTED);
		}
		else
		{
			lcdClrLine(2,7);
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"NOT SUPPORT");
			lcdFlip();
			PubWaitKey(3);
			return NO_DISP;
		}			
	}
	else
	{
		if(stPosParam.ucSalePassive==PARAM_OPEN)
		{
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
				ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED|CARD_PASSIVE);
			else
				ucRet = PosGetCard(CARD_SWIPED|CARD_PASSIVE);
		}
		else
		{
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
				ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED);
			else
				ucRet = PosGetCard(CARD_SWIPED);
		}
	}
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( PosCom.ucSwipedFlag==CARD_SWIPED && stTemp.iTransNo ==POS_SALE)
	{
		iRet = EnterSeguridadCode();
		if( iRet != OK ){
			return iRet;
		}
	}	
		
	return OK;
}

uint8_t EnterSeguridadCode(void)
{
	uint8_t iRet;
	uint8_t buf[10];
	uint8_t buf2[100];
	
	while(1)
	{	
		if( PosCom.stTrans.TransFlag == PAYMENT_WITH_CARD )
		{
			sprintf((char*)buf2,"PAGO A NETPAY CON TARJETA %s",PosCom.stTrans.AscCardType);
		}
		else if( stTemp.iTransNo==NETPAY_REFUND )
		{
			sprintf((char*)buf2,"DEVOLUCION %s",PosCom.stTrans.AscCardType);
		}
		else if( stTemp.iTransNo==NETPAY_FORZADA )
		{
			sprintf((char*)buf2,"VENTA FORZADA %s",PosCom.stTrans.AscCardType);
		}
		else
		{
			sprintf((char*)buf2,"VENTA %s",PosCom.stTrans.AscCardType);
		}
		
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)buf2);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "COD.DE SEGURIDAD");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdGoto(120, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 4, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return NO_DISP;
		}
		else if( iRet<3 )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INGRESE ENTRE");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "3 Y 4 DIGITOS");
			lcdFlip();
			FailBeep();
			sysDelayMs(1500);
			continue;
		}
		else if( iRet!=3 && iRet != 4 )
		{
			return NO_DISP;
		}
		memset(PosComconTrol.szCV,0,sizeof(PosComconTrol.szCV));
		strcpy(PosComconTrol.szCV,(char*)buf);
		break;
	}
	return OK;
}



uint8_t NetpayAdjustTranGetData(void)
{
	uint8_t	ucRet, ucSwipeFlag;
	int     iRet,iKey = 0;
//	char    buf[20];
	NEWPOS_LOG_STRC stLog;
	int     reInputFlag = 0;

	lcdCls();
	if( stTemp.iTransNo==POS_REFUND )
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CANCELACION");
	else if( stTemp.iTransNo==NETPAY_ADJUST )
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "AJUSTE");
	else
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "VOID PRE-CMP");
	if( stPosParam.ucEmvSupport==PARAM_OPEN )	
			ucSwipeFlag = CARD_SWIPED|CARD_INSERTED|CARD_PASSIVE;
		else
			ucSwipeFlag = CARD_SWIPED|0x80;

	ucRet = SelectOldTrans();
	if( ucRet != OK )
	{
		return ucRet;
	}
	#if 0
	iRet = CheckManagePass();
	if( iRet!=1 )
	{
		return E_TRANS_FAIL;
	}
	#endif

RESELECT:	
	if(stTemp.iTransNo==POS_REFUND || stTemp.iTransNo==NETPAY_ADJUST)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_HFONT16, NULL, "AJUSTAR ULTIMA TRANS?");
		DispMulLanguageString(210, 7, DISP_HFONT16, NULL, "1-SI");
		DispMulLanguageString(210, 8, DISP_HFONT16, NULL, "2-NO");
		lcdFlip();

		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			if( stTransCtrl.ucLastTransType == 0 )
			{
				ucRet = FindTheLastSale(&stLog);
				if( ucRet != OK ){
					return ucRet;
				}
			}
			else
			{
				ucRet = CheckFindTheLastSale(&stLog);
				if( ucRet != OK ){
					return ucRet;
				}
			}
			
			PosCom.stTrans.lOldTraceNo = stLog.lTraceNo;
		}
		else if( iKey == KEY2 ){
			;
		}
		
	}

REPUT:
	if( (iKey != KEY_ENTER && iKey != KEY1) ||  reInputFlag == 1)
	{
		ucRet = GetOldTraceNo();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}	

	ucRet = CheckData(PosCom.stTrans.lOldTraceNo, NULL);
	if( ucRet == E_REINPUT )
	{
		reInputFlag = 1;
		goto REPUT;
	}
	else if( ucRet == E_RESELECT )
	{
		goto RESELECT;
	}
	else if( ucRet == E_NO_OLD_TRANS)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "UN MOMENTO");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO ENCONTRADO");
		lcdFlip();
		kbGetKeyMs(1000);
		goto REPUT;
	}
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if ( stTemp.iTransNo==POS_REFUND && (PosCom.stTrans.iOldTransNo!=POS_SALE &&
		PosCom.stTrans.iOldTransNo!=PRE_TIP_SALE && PosCom.stTrans.iOldTransNo!=POS_PREAUTH&&
		PosCom.stTrans.iOldTransNo!=INTERESES_SALE) && PosCom.stTrans.iOldTransNo!=CHECK_IN &&
		PosCom.stTrans.iOldTransNo!=CHECK_OUT &&
 		PosCom.stTrans.iOldTransNo!=ICC_OFFSALE && PosCom.stTrans.iOldTransNo!=QPBOC_ONLINE_SALE )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO HAY VENTA");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NO CANCELACION");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);

		return NO_DISP;
	}

	if( PosCom.stTrans.iOldTransNo==ICC_OFFSALE && (!stTemp.bSendId) )
	{	
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "OFFLINE NOT SEND");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NOT VOID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return NO_DISP;
	}


	if( stTemp.iTransNo==POS_AUTH_VOID && PosCom.stTrans.iOldTransNo!=POS_AUTH_CM)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT PRE-CMP");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NOT VOID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);

		return NO_DISP;
	}

	
	if( (stTemp.iTransNo==POS_REFUND && stPosParam.ucVoidSwipe!=PARAM_OPEN) ||
		(stTemp.iTransNo==POS_AUTH_VOID && stPosParam.ucAuthVoidSwipe!=PARAM_OPEN) )
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

	PosCom.stTrans.ReadCardType = PosCom.ucSwipedFlag;
	if( PosCom.ucSwipedFlag==CARD_SWIPED ){
		iRet = EnterSeguridadCode();
		if( iRet != OK ){
			return iRet;
		}
	}	

	return OK;
}


uint8_t SaleVoidGetData(void)
{
	uint8_t	ucRet, ucSwipeFlag;
	int     iRet,iKey = 0;
//	char    buf[20];
	NEWPOS_LOG_STRC stLog;
	int     reInputFlag = 0;

	lcdCls();
	if( stTemp.iTransNo==POS_REFUND )
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CANCELACION");
	else
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "VOID PRE-CMP");
	if( stPosParam.ucEmvSupport==PARAM_OPEN )	
			ucSwipeFlag = CARD_SWIPED|CARD_INSERTED;
		else
			ucSwipeFlag = CARD_SWIPED|0x80;

	ucRet = SelectOldTrans();
	if( ucRet != OK )
	{
		return ucRet;
	}

	#if 0
	iRet = CheckManagePass();
	if( iRet!=1 )
	{
		return E_TRANS_FAIL;
	}
	#endif
	
	if(stTemp.iTransNo==POS_REFUND){
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_HFONT16, NULL, "ANULAR ULTIMA TRANS?");
		DispMulLanguageString(210, 7, DISP_HFONT16, NULL, "1-SI");
		DispMulLanguageString(210, 8, DISP_HFONT16, NULL, "2-NO");
		lcdFlip();

		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL || iKey == KEY_TIMEOUT) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			ucRet = FindTheLastSale(&stLog);
			if( ucRet != OK ){
				return ucRet;
			}
			#if 0
			if( stTransCtrl.ucLastTransType == 0 )
			{
				ucRet = FindTheLastSale(&stLog);
				if( ucRet != OK ){
					return ucRet;
				}
			}
			else
			{
				ucRet = CheckFindTheLastSale(&stLog);
				if( ucRet != OK ){
					return ucRet;
				}
			}
			#endif
			
			PosCom.stTrans.lOldTraceNo = stLog.lTraceNo;
		}
		else if( iKey == KEY2 ){
			;
		}
		
	}

REPUT:
	if( (iKey != KEY_ENTER && iKey != KEY1) ||  reInputFlag == 1){
		ucRet = GetOldTraceNo();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}	

	ucRet = CheckData(PosCom.stTrans.lOldTraceNo, NULL);
	if( ucRet == E_REINPUT )
	{
		reInputFlag = 1;
		goto REPUT;
	}
	else if( ucRet == E_NO_OLD_TRANS)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "UN MOMENTO");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO ENCONTRADO");
		lcdFlip();
		kbGetKeyMs(1000);
		goto REPUT;
	}
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if ( stTemp.iTransNo==POS_REFUND && (PosCom.stTrans.iOldTransNo!=POS_SALE &&
		PosCom.stTrans.iOldTransNo!=PRE_TIP_SALE && PosCom.stTrans.iOldTransNo!=POS_PREAUTH&&
		PosCom.stTrans.iOldTransNo!=INTERESES_SALE) && PosCom.stTrans.iOldTransNo!=CHECK_IN &&
		PosCom.stTrans.iOldTransNo!=CHECK_OUT &&
 		PosCom.stTrans.iOldTransNo!=ICC_OFFSALE && PosCom.stTrans.iOldTransNo!=QPBOC_ONLINE_SALE )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO HAY VENTA");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NO CANCELACION");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return NO_DISP;
	}

	if( PosCom.stTrans.iOldTransNo==ICC_OFFSALE && (!stTemp.bSendId) )
	{	
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "OFFLINE NOT SEND");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NOT VOID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return NO_DISP;
	}


	if( stTemp.iTransNo==POS_AUTH_VOID && PosCom.stTrans.iOldTransNo!=POS_AUTH_CM)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT PRE-CMP");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NOT VOID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);

		return NO_DISP;
	}

	if( (stTemp.iTransNo==POS_REFUND && stPosParam.ucVoidSwipe!=PARAM_OPEN) ||
		(stTemp.iTransNo==POS_AUTH_VOID && stPosParam.ucAuthVoidSwipe!=PARAM_OPEN) )
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

	return OK;
}

uint8_t RefundTranGetData(void)
{
	uint8_t ucRet, ucSwipeMode;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     CANCELACION     ");
/*	ucRet = CheckSupPwd(0);
	if( ucRet!=OK )
	{
		return ucRet;
	}
*/	
	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		ucSwipeMode = CARD_SWIPED|CARD_INSERTED;
	else
		ucSwipeMode = CARD_SWIPED;
	
	ucRet = PosGetCard(ucSwipeMode);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetSysRefNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}

/*	ucRet = GetOrignTxnDate();
	if( ucRet!=OK )
	{
		return ucRet;
	}
*/	
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

	if( PosCom.stTrans.szCardUnit[0]==0 )
	{
		strcpy((char *)PosCom.stTrans.szCardUnit, "000");
	}
	PosCom.stTrans.lOldTraceNo = 0;
	PosCom.stTrans.lOldBatchNumber = 0;

	if (PosCom.ucSwipedFlag == CARD_INSERTED && memcmp(PosCom.stTrans.szAID,STR_PBOC_AID,10) == 0)
	{
		strcpy((char *)PosCom.stTrans.szCardUnit, "CUP");
	}

	return OK;
}

uint8_t ConfirmPreTipAmount(uint8_t *BcdNormalAmount)
{
	int  iRet;
	char buf[20];
	long lNormalAmount;
	long amt;
	char AscAmount[30];
	char AscTotalAmount[20];
	char AscNormalAmount[20];
	
	lcdClrLine(2, 7);		
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "INDIQUE PROPINA");
	DispMulLanguageString(0, 5, DISP_HFONT16|DISP_MEDIACY, NULL, "PROPINA NO PUEDE EXCEDER 25%%");
	lcdFlip();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

	memset(buf, 0, sizeof(buf));
	PubBcd2Asc0(BcdNormalAmount,6,AscNormalAmount);
	lNormalAmount = atol((char*)AscNormalAmount);
	do{
		lcdGoto(120, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return E_TRANS_CANCEL;
		amt = atol((char *)buf);
		if(amt > lNormalAmount * 0.25){
			lcdCls();
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PROPINA SUPERIOR");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "A LA PERMITIDA");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(3);
			return NO_DISP;
		}
		else{
			PosCom.stTrans.lPreAuthTipAmount = amt; // save preauth tip amount(long)
			sprintf(AscAmount,"%010ld%02ld\n",amt/100,amt%100);
			AscToBcd(PosCom.stTrans.preAuthTipAmount,(uint8_t*)AscAmount,12);

			sprintf(AscTotalAmount,"%010ld%02ld\n",(amt+lNormalAmount)/100,(amt+lNormalAmount)%100);
			AscToBcd(PosCom.stTrans.sAmount,(uint8_t*)AscTotalAmount,12);
			break;
		}
	}while(1);
	
	return OK;
}


uint8_t CheckOutGetData(void)
{
	uint8_t ucRet/*, ucSwipeMode*/;
//	int     iRet;

/*	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		ucSwipeMode = CARD_SWIPED|CARD_INSERTED;
	else
		ucSwipeMode = CARD_SWIPED;
	if( stPosParam.ucManualInput==PARAM_OPEN )
	{
		ucSwipeMode |= CARD_KEYIN;
	}
*/
	lcdCls();
	if( stTemp.iTransNo==CHECK_OUT)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CHECK OUT");
	}

	ucRet = GetOldReference();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = SelectMoneyType();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	#if 0
	ucRet = CheckOutCheckData(PosCom.stTrans.lOldTraceNo, NULL);
	if( ucRet == E_NO_OLD_TRANS){
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NM MOMENTO");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO ENCONTRADO");
		lcdFlip();
		kbGetKeyMs(1000);
		return NO_DISP;
	}
	else if( ucRet != OK)
	{
		return ucRet;
	}

	if (PosCom.ucPBOCFlag==1)
	{
		if( stPosParam.ucEmvSupport==PARAM_OPEN )
			ucRet = PosGetCard(CARD_INSERTED|CARD_PASSIVE);
		else
			ucRet = PosGetCard(CARD_PASSIVE);
	}
	else if (PosCom.ucPBOCFlag==2)
	{
		if( stPosParam.ucEmvSupport==PARAM_OPEN )
			ucRet = PosGetCard(CARD_INSERTED);
		else
		{
			lcdClrLine(2,7);
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"NOT SUPPORT");
			lcdFlip();
			PubWaitKey(3);
			return NO_DISP;
		}			
	}
	else
	{
		if(stPosParam.ucSalePassive==PARAM_OPEN)
		{
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
				ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED|CARD_PASSIVE);
			else
				ucRet = PosGetCard(CARD_SWIPED|CARD_PASSIVE);
		}
		else
		{
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
				ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED);
			else
				ucRet = PosGetCard(CARD_SWIPED);
		}
	}
	if( ucRet!=OK )
	{
		return ucRet;
	}
	

	if( strcmp((char*)PosCom.stTrans.szCheckOutCardNo,(char*)PosCom.stTrans.szCardNo) != 0 )
	{
		lcdCls();
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"TARJETA INVALIDA");
		lcdFlip();
		FailBeep();
		PubWaitKey(2);
		return NO_DISP;
	}
	#endif

	
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = ComfirAmount(PosCom.stTrans.sAmount);
	if( ucRet != OK){
		return ucRet;
	}

	#if 0
	if( PosCom.ucSwipedFlag==CARD_SWIPED ){
		ucRet = EnterSeguridadCode();
		if( ucRet != OK ){
			return ucRet;
		}
	}
	#endif

/*	ucRet = PosGetCard(ucSwipeMode);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetOrignTxnDate();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = GetAuthNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}

REINPUT1:	
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( stTemp.iTransNo!=POS_AUTH_CM )
	{
		ucRet = AppConfirmAmount();
		if( ucRet==E_TRANS_CANCEL )
		{
			goto REINPUT1;	
		}
		else if( ucRet==E_TRANS_FAIL )
		{
			return NO_DISP;
		}
	}

	iRet = EnterPIN(0);
	if( iRet!=OK )
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;
	}
*/
	PosCom.stTrans.lOldBatchNumber = 0;
	PosCom.stTrans.lOldTraceNo = 0;
	return OK;
}


uint8_t AuthComfirmGetData(void)
{
	uint8_t ucRet/*, ucSwipeMode*/;
//	int     iRet;

/*	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		ucSwipeMode = CARD_SWIPED|CARD_INSERTED;
	else
		ucSwipeMode = CARD_SWIPED;
	if( stPosParam.ucManualInput==PARAM_OPEN )
	{
		ucSwipeMode |= CARD_KEYIN;
	}
*/
	lcdCls();
	if( stTemp.iTransNo==POS_AUTH_CM )
	{
//		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "PRE CMP(ONLINE)");
	}
	else
	{
//		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "PRE CMP(OFFLINE)");
//		PosCom.stTrans.szOldTxnDate[0] = 0;
	}

	ucRet = GetOldTraceNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = AuthConfirmCheckData(PosCom.stTrans.lOldTraceNo, NULL);
	if( ucRet == E_NO_OLD_TRANS){
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NM MOMENTO");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO ENCONTRADO");
		lcdFlip();
		kbGetKeyMs(1000);
		return NO_DISP;
	}
	else if( ucRet != OK){
		return ucRet;
	}

	ucRet = ConfirmPreTipAmount(PosCom.stTrans.preAuthOriAmount);
	if( ucRet != OK){
		return ucRet;
	}

/*	ucRet = PosGetCard(ucSwipeMode);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetOrignTxnDate();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = GetAuthNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}

REINPUT1:	
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( stTemp.iTransNo!=POS_AUTH_CM )
	{
		ucRet = AppConfirmAmount();
		if( ucRet==E_TRANS_CANCEL )
		{
			goto REINPUT1;	
		}
		else if( ucRet==E_TRANS_FAIL )
		{
			return NO_DISP;
		}
	}

	iRet = EnterPIN(0);
	if( iRet!=OK )
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;
	}
*/
	PosCom.stTrans.lOldBatchNumber = 0;
	PosCom.stTrans.lOldTraceNo = 0;

	return OK;
}

uint8_t PreAuthVoidGetData(void)
{
	uint8_t ucRet, ucSwipeMode;
	int     iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  PREAUTH VOID  ");
	ucRet = CheckSupPwd(0);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		ucSwipeMode = CARD_SWIPED|CARD_INSERTED;
	else
		ucSwipeMode = CARD_SWIPED;
	if( stPosParam.ucManualInput==PARAM_OPEN )
	{
		ucSwipeMode |= CARD_KEYIN;
	}
	ucRet = PosGetCard(ucSwipeMode);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	strcpy((char *)PosCom.stTrans.szOldTxnDate, "0000");
	ucRet = GetOrignTxnDate();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = GetAuthNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	iRet = EnterPIN(0);
	if( iRet!=OK )
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;
	}
	
	PosCom.stTrans.lOldBatchNumber = 0;
	PosCom.stTrans.lOldTraceNo = 0;

	return OK;	
}

uint8_t PreAuthGetData(void)
{
	uint8_t	ucRet;
	int     iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "VENTA");
	if( stPosParam.ucEmvSupport==PARAM_OPEN ) 
		ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED);
	else
		ucRet = PosGetCard(CARD_SWIPED);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	if( PosCom.ucSwipedFlag==CARD_SWIPED ){
		iRet = EnterSeguridadCode();
		if( iRet != OK ){
			return iRet;
		}
	}
	

	//NETPAY 使用TIP
	/*
	ucRet = AppGetAmount(9, TIP_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	iRet = EnterPIN(0);
	if( iRet!=OK)
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;			
	}*/ 
	strcpy((char *)PosCom.stTrans.szAuthMode, "00");

	return OK;
}

uint8_t PreAuthAddInput(void)
{
	uint8_t	ucRet, ucSwipeMode;
	int     iRet;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   PREAUTH ADD  ");
	ucSwipeMode = CARD_SWIPED|0x80;		// DO NOT check service code
	ucRet = PosGetCard(ucSwipeMode);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetAuthNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	iRet = EnterPIN(0);
	if( iRet!=OK )
	{
		DispEppPedErrMsg(iRet);
		return NO_DISP;
	}

	PosCom.stTrans.lOldBatchNumber = 0;
	PosCom.stTrans.lOldTraceNo = 0;
	return OK;
}

uint8_t OffSaleGetData(void)
{
	uint8_t	ucRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " SETTLE OFFLINE ");
	ucRet = PosGetCard(CARD_KEYIN); 
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = GetAuthType(); 
	if( ucRet )
	{
		return ucRet;
	}
	
	ucRet = SelectCardUnit(PosCom.stTrans.szCardUnit);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = AppGetAmount(9, TRAN_AMOUNT);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	sprintf((char *)PosCom.stTrans.szOldTxnDate, (char *)&PosCom.stTrans.szDate[4]);
	sprintf((char *)PosCom.stTrans.szEntryMode, "012");	
	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	PosCom.stTrans.lOldTraceNo = PosCom.stTrans.lTraceNo;
	PosCom.stTrans.lOldBatchNumber = PosCom.stTrans.lBatchNumber;

	IncreaseTraceNo();

	return OK;	
}

uint8_t AdjustTranGetData(void)
{
	uint8_t		ucRet;
	NEWPOS_LOG_STRC	stOldLog;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  SETTLE ADJUST ");
	ucRet = GetOldTraceNo();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	ucRet = CheckData(PosCom.stTrans.lOldTraceNo, &stOldLog);	
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	lcdClrLine(2, 7);	
	if( PosCom.stTrans.iOldTransNo!=OFF_SALE && PosCom.stTrans.iOldTransNo!=POS_SALE &&
		PosCom.stTrans.iOldTransNo!=ICC_OFFSALE )
	{
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "   NOT ADJUST    ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);

		return NO_DISP;
	}
	if(stOldLog.ucSendFail == TS_NOT_CODE)	
	{
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "   NOT ADJUST    ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return NO_DISP;
	}
	if(stOldLog.ucSendFail == TS_NOT_RECV)	
	{
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "   NOT ADJUST    ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return NO_DISP;
	}

	sprintf((char *)PosCom.stTrans.szEntryMode, "012");	
	PosCom.stTrans.ucSwipeFlag = CARD_KEYIN;
	
	if( PosCom.stTrans.iOldTransNo==OFF_SALE )
	{
		ucRet = AppGetAmount(9, ADJUST_AMOUNT);
		if( ucRet!=OK )
		{
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  SETTLE ADJUST ");
			return ucRet;
		}

		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  SETTLE ADJUST ");

		ucRet = CheckAdjustAmt(PosCom.stTrans.sAmount, stOldLog.sAmount, stPosParam.ucAdjustPercent);
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else 
	{
		if( memcmp(PosCom.stTrans.szCardUnit, "CUP", 3)==0 )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "ONLY FRN CARD");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(3000);

			return NO_DISP;
		}
		if( stPosParam.ucTipOpen!=PARAM_OPEN ) 
		{
			return E_TIP_NO_OPEN;
		}
		stTemp.iTransNo =  ADJUST_TIP;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
		
		ucRet = AppGetAmount(9, TIP_AMOUNT);
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	
	if( stOldLog.ucSendFlag==TRUE || stTemp.iTransNo==ADJUST_TIP )
	{
		IncreaseTraceNo();
	}
	PosCom.stTrans.iTransNo = stTemp.iTransNo;

	return OK;
}

uint8_t CheckAdjustAmt(uint8_t *sAmount, uint8_t *sOldAmount, uint8_t ucPercent)
{
	int oldAmt,newAmt;
	int64_t tmp;
	
	if( stPosParam.ucAdjustPercent==0 ) 
		return OK;

	oldAmt = BcdToLong(sOldAmount, 6);
	newAmt = BcdToLong(sAmount, 6);
	
	tmp = oldAmt * stPosParam.ucAdjustPercent; 
	if( newAmt>(tmp/100+oldAmt) )	
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INVALID AMOUNT");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "EXLIMITED");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);
		return NO_DISP;
	}

	return OK;
}

//电子钱包消费交易发包/收包/解析
uint8_t Electronic_wallet_online_Auth(void)
{
	uint8_t	ucRet,iRet;
	char sTempBuff[30] ={0};
	char szField63BuffTemp[500];
	char* Temp2;

	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0200");
	sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "000000");
	//sprintf((char *)glSendPack.szTranAmt,   "%.*s", LEN_PROC_CODE, PosCom.stTrans.sAmount);
	BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
	if(PosCom.stTrans.CardType!=7)
	{
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
	}

	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	//14 filed
	if(PosCom.stTrans.CardType!=7)
	{
		if(strlen(PosCom.stTrans.szExpDate)!=0)
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
		else
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   "0000");
	}

	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char*)glSendPack.szSvrCode,"00"); //25域
	switch(PosCom.ucSwipedFlag)
	{
		case CARD_SWIPED:
			//memcpy(PosCom.stTrans.szEntryMode, "021", 3);
			memcpy(PosCom.stTrans.szEntryMode, "520", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			//sprintf((char *)glSendPack.szField45, "%.*s", strlen((char*)PosCom.szTrack1), PosCom.szTrack1);
			break;
		case CARD_INSERTED:
			//memcpy(PosCom.stTrans.szEntryMode, "050", 3);
			memcpy(PosCom.stTrans.szEntryMode, "550", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);

			if(PosCom.stTrans.CardType==7)
			{
				memset(sTempBuff,0,sizeof(sTempBuff));
				memcpy(sTempBuff,PosCom.szTrack2,16);
				Temp2 = strchr((char *)PosCom.szTrack2, '=');
				if( Temp2==NULL )
				{
					return FALSE;
				}
				memcpy(sTempBuff+16,Temp2,5);
				printf("glSendPack.szTrack2:%s\n",sTempBuff);
				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, sTempBuff);			
			}
			else
			{
				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			}
			PubLong2Char((ulong)PosCom.stTrans.iIccDataLen, 2, (char *)glSendPack.sICCData);
			memcpy(&glSendPack.sICCData[2],PosCom.stTrans.sIccData,PosCom.stTrans.iIccDataLen );	// save for batch upload
		case CARD_KEYIN:
		default:
			break;
	}

	//37 Field
	//sprintf((char*)glSendPack.szRRN,"%s","123123123123");
	printf("37 field:%s\n",PosCom.stTrans.szSysReferNo);
	sprintf((char*)glSendPack.szRRN,"%s",PosCom.stTrans.szSysReferNo);

	/*
	ShortToByte(glSendPack.sField62, strlen("123123123123"));
	sprintf((char *)sTempBuff,"%s","123123123123");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	*/
	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);
	sprintf((char*)glSendPack.szField48, "%.*s",LEN_TERM_ID -4 ,stPosParam.szST);   // store id
	sprintf((char *)glSendPack.szCurrencyCode, "484");
	if(PRE_TIP_SALE ==PosCom.stTrans.TransFlag)
	{
		//sprintf((char *)glSendPack.szExtAmount,"%.*s",LEN_EXT_AMOUNT,PosCom.stTrans.sTipAmount);
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.sTipAmount, 12);
	}
	memset(sTempBuff,0,sizeof(sTempBuff));
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	
	NetpayDealFeild63((uint8_t *)szField63BuffTemp);	
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);
	
	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	
	PosCom.stTrans.isCtlplat =0;
	PrintDebug("%s%d", "PosCom.stTrans.isCtlplat=",PosCom.stTrans.isCtlplat);

	//ucRet = OnlineCommProc();
	while(1)
	{
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			UpdateReversalFile(ucRet); 
			if(stPosParam.ucReversalTime == PARAM_OPEN)
			{
				iRet = ReverseTranProc();
				if (iRet==0)
				{
					lcdClrLine(2,7);
					lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"REVERSAL SUCCESS");
					lcdFlip();
					PubBeepOk();
					PubWaitKey(5);
					return 0;
				}
				else if (iRet == 1)
				{
					return ucRet;
				}
				return iRet;
			}
			return ucRet;
		}
		
		fileRemove(REVERSAL_FILE);
		break;
	}
	return AfterCommProc(); 
}
//电子钱包预授权交易发包/收包/解析
uint8_t Electronic_wallet_online_PreAuth(void)
{
	uint8_t	ucRet,iRet;
	char sTempBuff[60] ={0};
	uint8_t pricebuf[13] ={0};
	
	char szField63BuffTemp[500];
	char *Temp2;
	long amount =0;
	int tmpoil =0;

	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0100");
	if(PosCom.stTrans.CardType==7)
	{
		sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "840000");
	}
	else
	{
		sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "820000");
	}
	//sprintf((char *)glSendPack.szTranAmt,   "%.*s", LEN_PROC_CODE, PosCom.stTrans.sAmount);
	//BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
	if(PosCom.stTrans.CardType==7)
	{
		memset(sTempBuff,0,sizeof(sTempBuff));
		Temp2 = strchr(PosCom.stTrans.oil_price,'.');
		if(Temp2 ==NULL)
		{
			return NO_DISP;
		}
		ucRet = Temp2-PosCom.stTrans.oil_price;
		printf("Temp2-stTemp.oil_price=%d\n",ucRet);
		memcpy(pricebuf,PosCom.stTrans.oil_price,ucRet);
		memcpy(pricebuf+ucRet,Temp2+1,strlen(PosCom.stTrans.oil_price)-ucRet-1);
		printf("pricebuf:%s\n",pricebuf);

		amount = AscToLong(pricebuf, strlen(pricebuf));
		amount *=BcdToLong(PosCom.stTrans.sLiters, 6);

		amount = amount/100;
		tmpoil = amount%100;
		if(tmpoil >50)
		{
			amount+=1;
		}
		//tmpoil= BcdToLong(PosCom.stTrans.sLiters, 6);

		//memset(pricebuf,0,sizeof(pricebuf));
		sprintf(glSendPack.szTranAmt,"%010ld%02ld",amount/100,amount%100);
		printf("glSendPack.szTranAmt:%s\n",glSendPack.szTranAmt);
		
		AscToBcd(PosCom.stTrans.sAmount, glSendPack.szTranAmt, 12);	
		
	}
	else
	{
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
	}

	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	//14 filed
	if(PosCom.stTrans.CardType!=7)
	{
		if(strlen(PosCom.stTrans.szExpDate)!=0)
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
		else
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   "0000");
	}

	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char*)glSendPack.szSvrCode,"00"); //25域
	switch(PosCom.ucSwipedFlag)
	{
		case CARD_SWIPED:
			//memcpy(PosCom.stTrans.szEntryMode, "021", 3);
			memcpy(PosCom.stTrans.szEntryMode, "520", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
			if(PosCom.stTrans.CardType==7)
			{
				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			}
			//sprintf((char *)glSendPack.szField45, "%.*s", strlen((char*)PosCom.szTrack1), PosCom.szTrack1);
			break;
		case CARD_INSERTED:
			//memcpy(PosCom.stTrans.szEntryMode, "050", 3);
			memcpy(PosCom.stTrans.szEntryMode, "550", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);

			if(PosCom.stTrans.CardType==7)
			{
				memset(sTempBuff,0,sizeof(sTempBuff));
				memcpy(sTempBuff,PosCom.szTrack2,16);
				Temp2 = strchr((char *)PosCom.szTrack2, '=');
				if( Temp2==NULL )
				{
					return FALSE;
				}
				memcpy(sTempBuff+16,Temp2,5);
				printf("glSendPack.szTrack2:%s\n",sTempBuff);
				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, sTempBuff);			
			}	

			PubLong2Char((ulong)PosCom.stTrans.iIccDataLen, 2, (char *)glSendPack.sICCData);

			memcpy(&glSendPack.sICCData[2],PosCom.stTrans.sIccData,PosCom.stTrans.iIccDataLen );	// save for batch upload
			
		case CARD_KEYIN:
		default:
			break;
	}

	//37 Field
	//sprintf((char*)glSendPack.szRRN,"%s","123123123123");
	printf("37 field:%s\n",PosCom.stTrans.szSysReferNo);
	
	sprintf((char*)glSendPack.szRRN,"%s",PosCom.stTrans.szSysReferNo);
	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);
	sprintf((char*)glSendPack.szField48, "%.*s",LEN_TERM_ID -4 ,stPosParam.szST);   // store id
	sprintf((char *)glSendPack.szCurrencyCode, "484");
	if(PRE_TIP_SALE ==PosCom.stTrans.TransFlag)
	{
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.sTipAmount, 12);
	}
	
	memset(sTempBuff,0,sizeof(sTempBuff));
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	
	NetpayDealFeild63((uint8_t *)szField63BuffTemp);	
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);
	
	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	
	PosCom.stTrans.isCtlplat =0;
	PrintDebug("%s%d", "PosCom.stTrans.isCtlplat=",PosCom.stTrans.isCtlplat);

	//ucRet = OnlineCommProc();

	while(1)
	{
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			UpdateReversalFile(ucRet); 
			if(stPosParam.ucReversalTime == PARAM_OPEN)
			{
				iRet = ReverseTranProc();
				if (iRet==0)
				{
					lcdClrLine(2,7);
					lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"REVERSAL SUCCESS");
					lcdFlip();
					PubBeepOk();
					PubWaitKey(5);
					return 0;
				}
				else if (iRet == 1)
				{
					return ucRet;
				}
				return iRet;
			}
			return ucRet;
		}
		
		fileRemove(REVERSAL_FILE);
		break;
	}
	return AfterCommProc(); 
}
uint8_t Electronic_wallet_online(void)
{
	uint8_t	ucRet,iRet;
	char sTempBuff[20] ={0};
	char szField63BuffTemp[999];

	long sAmount =0;
	int     iKey;

	ucRet = SaleTranGetData();

	if( ucRet!=OK )
	{
		PrintDebug("%s", "SaleTranGetData ucret!=ok");
		return ucRet;
	}

	PrintDebug("%s%s","AppGetCtlvalue:",PosCom.stTrans.szCardNo);
	iRet = AppGetCtlvalue(PosCom.stTrans.TransFlag);

	if( iRet!=OK )
	{
		return iRet;
	}

	if(PosCom.stTrans.TransFlag ==PURSE_PUNTO ||PosCom.stTrans.TransFlag ==PURSE_EDENRED)
	{
		stTemp.iTransNo = PURSE_AUTH;
		iRet = Electronic_wallet_online_PreAuth();
		if(iRet!=OK)
		{
			PrintDebug("%s%d","PreAuth fail:",iRet);
			return iRet;
		}

		lcdCls();
		lcdDisplay(0, 2, DISP_HFONT16, "CONFIRMACION");
		lcdDisplay(0, 4, DISP_CFONT, "REALIZAR LA VENTA?");		
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");//DISP_HFONT16
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		 if(  iKey != KEY1 && iKey !=KEY_ENTER)
		{
			return NO_DISP;
		}

		memset(sTempBuff,0,sizeof(sTempBuff));
		BcdToAsc(sTempBuff, PosCom.stTrans.sAmount, 12);
		sAmount= atol((char *)sTempBuff);
		lcdCls();
		lcdDisplay(0, 2, DISP_HFONT16, "CONFIRMACION");

		if(PosCom.stTrans.saletype[0] == '1')
		{
			lcdDisplay(0, 4, DISP_CFONT, "IMPORTE: %ld.%02ld ",(sAmount)/100,(sAmount)%100);
		}
		else if(PosCom.stTrans.saletype[0] == '2')
		{
			lcdDisplay(0, 4, DISP_CFONT, "LITROS: %ld.%02ld ",(sAmount)/100,(sAmount)%100);
		}
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");//DISP_CFONT
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if ( iKey != KEY_ENTER &&  iKey != KEY1)
		{
			return NO_DISP;
		}
	}

	stTemp.iTransNo = PURSE_SALE;
	PosCom.stTrans.iTransNo = PURSE_SALE;
	
	iRet = Electronic_wallet_online_Auth();
	if( iRet!=OK )
	{
		PrintDebug("%s %d", "Auth fail:",iRet);
		return iRet;
	}

	if(stPosParam.stVolContrFlg ==PARAM_OPEN)
	{
		ucRet = ComfirmAmount_ToGasPlat();
		return ucRet;
	}
	else
	{
		return OK;
	}
}
uint8_t OnlineSale(void)
{
	uint8_t	ucRet;
	char sTempBuff[20] ={0};
	char szField63BuffTemp[999];

	if( PosCom.ucSwipedFlag==NO_SWIPE_INSERT )
	{
		PosCom.stTrans.TransFlag = 0;    //将交易标志位
	}
	
	//判断是否带小费
	if(stPosParam.szpreTip == PARAM_OPEN)
	{
		PosCom.stTrans.TransFlag = PRE_TIP_SALE;
	}
	else
	{
		PosCom.stTrans.TransFlag = NORMAL_SALE;
	}

	ucRet = SaleTranGetData();
	
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	//SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");

	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0200");
	sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "000000");
	//sprintf((char *)glSendPack.szTranAmt,   "%.*s", LEN_PROC_CODE, PosCom.stTrans.sAmount);
	BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);


	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char*)glSendPack.szSvrCode,"00"); //25域
	switch(PosCom.ucSwipedFlag)
	{
		case CARD_SWIPED:
			//memcpy(PosCom.stTrans.szEntryMode, "021", 3);
			memcpy(PosCom.stTrans.szEntryMode, "520", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			sprintf((char *)glSendPack.szField45, "%.*s", strlen((char*)PosCom.szTrack1), PosCom.szTrack1);
			break;
		case CARD_INSERTED:
			//memcpy(PosCom.stTrans.szEntryMode, "050", 3);
			memcpy(PosCom.stTrans.szEntryMode, "550", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
		case CARD_KEYIN:
		default:
			break;
	}

	//37 Field
	sprintf((char*)glSendPack.szRRN,"%s","123123123123");

	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);
	sprintf((char*)glSendPack.szField48, "%.*s",LEN_TERM_ID -4 ,stPosParam.szST);   // store id
	sprintf((char *)glSendPack.szCurrencyCode, "484");
	if(PRE_TIP_SALE ==PosCom.stTrans.TransFlag)
	{
		//sprintf((char *)glSendPack.szExtAmount,"%.*s",LEN_EXT_AMOUNT,PosCom.stTrans.sTipAmount);
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.sTipAmount, 12);
	}
	memset(sTempBuff,0,sizeof(sTempBuff));
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	
	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);
	
	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	
	PosCom.stTrans.isCtlplat =0;
	PrintDebug("%s%d", "PosCom.stTrans.isCtlplat=",PosCom.stTrans.isCtlplat);

	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	if(stPosParam.stVolContrFlg ==PARAM_OPEN)
	{
		ucRet = ComfirmAmount_ToGasPlat();
		return ucRet;
	}
	else
	{
		return OK;
	}
}


uint8_t CheckIn(void)
{
	uint8_t	ucRet;
//	int     iRet;
//	uint8_t sTempBuff[60];
	char szField63BuffTemp[999];
//	char szField120BuffTemp[999];

	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	ucRet = SaleTranGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	SetCommReqField((uint8_t *)"0100", (uint8_t *)"777710");
	
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	memset(PosCom.stTrans.szTransDate,0,sizeof(PosCom.stTrans.szTransDate));
	memset(PosCom.stTrans.szTransTime,0,sizeof(PosCom.stTrans.szTransTime));
	strcpy((char *)PosCom.stTrans.szTransDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTransTime, (char *)stTemp.szTime);

	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	sprintf((char *)glSendPack.szNII, "0003");  

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	return OK;
}




uint8_t  CashAdvacne(void)
{
	uint8_t	ucRet;
//	int     iRet;
//	uint8_t sTempBuff[60];
	char szField63BuffTemp[999];
//	char szField120BuffTemp[999];

	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	ucRet = SaleTranGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	SetCommReqField((uint8_t *)"0200", (uint8_t *)"100000");
	
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	memset(PosCom.stTrans.szTransDate,0,sizeof(PosCom.stTrans.szTransDate));
	memset(PosCom.stTrans.szTransTime,0,sizeof(PosCom.stTrans.szTransTime));
	strcpy((char *)PosCom.stTrans.szTransDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTransTime, (char *)stTemp.szTime);

	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	sprintf((char *)glSendPack.szNII, "0003");  

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	return OK;
}



uint8_t CheckOut(void)
{
	uint8_t	ucRet /*,sBuff[10]*/;
	char szField63BuffTemp[400];
//	char szField120BuffTemp[999];

	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	ucRet = CheckOutGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	
	
	SetCommReqField((uint8_t *)"0100", (uint8_t *)"777711");
	sprintf((char *)glSendPack.szPan,"%s",PosCom.stTrans.szCardNo);
//	sprintf((char *)glSendPack.szAuthCode, "%.*s", LEN_AUTH_CODE, PosCom.stTrans.szAuthNo);
	GetPosTime(stTemp.szDate, stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(stTemp.szDate+4));

	memset(glSendPack.szAuthCode,0,sizeof(glSendPack.szAuthCode));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	sprintf((char*)glSendPack.szRRN,"%s",(char*)PosCom.stTrans.szSysReferNo);
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	sprintf((char *)glSendPack.szNII, "0003");

	PosCom.ucSwipedFlag = CARD_KEYIN;
	strcpy((char*)PosCom.stTrans.szEntryMode,"012");
//	BcdToAsc((char*)glSendPack.szExtAmount,(char*)PosCom.stTrans.preAuthTipAmount,12);
	
	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
	
	ucRet = OnlineCommProc(); 
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;
}




uint8_t AdjustTran(void)
{
	uint8_t	ucRet;
	char szField63BuffTemp[400] = {0};
//	char szField120BuffTemp[999];

	PosCom.stTrans.iTransNo=stTemp.iTransNo;
	
	ucRet = NetpayAdjustTranGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"030000");
	//NETPAY 不上送三磁道
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60)); // 无60域
	memset(glSendPack.szPanSeqNo,0,sizeof(glSendPack.szPanSeqNo));
	
	sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	if( stTemp.iTransNo == NETPAY_ADJUST && PosCom.stTrans.iOldTransNo== PRE_TIP_SALE)
	{
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.preTipAmount, 12);	
	}

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}

uint8_t SelectOldTrans(void)
{
	uint8_t	ucRet;

	if( stPosParam.Switch_Hotel==PARAM_OPEN )
	{
		if( stTemp.iTransNo==POS_REFUND )
		{
			InitMenu(MENU_MODE_1, "CANCELACION");
			MainMenuAddMenuItem(PARAM_OPEN,    					POS_SALE,    "VENTA NORMAL       ",         NULL);
			MainMenuAddMenuItem(stPosParam.CheckInFlag,    		CHECK_IN,       "CHECK IN           ",         NULL);
			MainMenuAddMenuItem(stPosParam.CheckOutFlag,    	CHECK_OUT,      "CHECK OUT          ",         NULL);

		}
		else if( stTemp.iTransNo==NETPAY_REFUND || stTemp.iTransNo==NETPAY_ADJUST )
		{
			InitMenu(MENU_MODE_1, "DEVOLUCION");
			MainMenuAddMenuItem(PARAM_OPEN,    					POS_SALE,    "VENTA NORMAL       ",         NULL);
			MainMenuAddMenuItem(stPosParam.CheckOutFlag,    	CHECK_OUT,      "CHECK OUT          ",         NULL);

		}

		ucRet = DispDynamicMenu(1);
		if( ucRet == 255 )		//NO_TRANS
			return NO_DISP;
		else	
			PosCom.stTrans.iSelectOldTransNo = ucRet;
	}
	
	return OK;
}


uint8_t RefundTran(void)
{
	uint8_t	ucRet;
	char szField63BuffTemp[400];
//	char szField120BuffTemp[999];
	
	ucRet = SaleVoidGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"200000");
	//NETPAY 不上送三磁道
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60)); // 无60域
	
	sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}



uint8_t Netpay_RefundGetData(void)
{
	uint8_t	ucRet, ucSwipeFlag;
//	int     iRet,iKey = 0;
//	char    buf[20];
//	NEWPOS_LOG_STRC stLog;
//	int     reInputFlag = 0;

	PosCom.stTrans.iTransNo = stTemp.iTransNo;

	lcdCls();
	if( stTemp.iTransNo==NETPAY_REFUND )
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "DEVOLUCION");
	if( stPosParam.ucEmvSupport==PARAM_OPEN )	
			ucSwipeFlag = CARD_SWIPED|CARD_INSERTED;
		else
			ucSwipeFlag = CARD_SWIPED|0x80;

	ucRet = SelectOldTrans();
	if( ucRet != OK )
	{
		return ucRet;
	}
	#if 0
	iRet = CheckManagePass();
	if( iRet!=1 )
	{
		return E_TRANS_FAIL;
	}
	#endif
	
	ucRet = GetOldReference();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	PosCom.stTrans.iOldTransNo = PosCom.stTrans.iSelectOldTransNo;	

	if (PosCom.ucPBOCFlag==1)
	{
		if( stPosParam.ucEmvSupport==PARAM_OPEN )
			ucRet = PosGetCard(CARD_INSERTED|CARD_PASSIVE);
		else
			ucRet = PosGetCard(CARD_PASSIVE);
	}
	else if (PosCom.ucPBOCFlag==2)
	{
		if( stPosParam.ucEmvSupport==PARAM_OPEN )
			ucRet = PosGetCard(CARD_INSERTED);
		else
		{
			lcdClrLine(2,7);
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"NOT SUPPORT");
			lcdFlip();
			PubWaitKey(3);
			return NO_DISP;
		}			
	}
	else
	{
		if(stPosParam.ucSalePassive==PARAM_OPEN)
		{
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
				ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED|CARD_PASSIVE);
			else
				ucRet = PosGetCard(CARD_SWIPED|CARD_PASSIVE);
		}
		else
		{
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
				ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED);
			else
				ucRet = PosGetCard(CARD_SWIPED);
		}
	}
	if( ucRet!=OK )
	{
		return ucRet;
	}

	#if 0
	if( PosCom.ucSwipedFlag==CARD_SWIPED ){
		ucRet = EnterSeguridadCode();
		if( ucRet != OK ){
			return iRet;
		}
	}
	#endif

	return OK;
}


uint8_t Netpay_RefundTran(void)
{
	uint8_t	ucRet;
	char szField63BuffTemp[400] = {0};
//	char szField120BuffTemp[999] = {0};
	
	ucRet = Netpay_RefundGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"020000");
	//NETPAY 不上送三磁道
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
//	memset(glSendPack.szPan,0,sizeof(glSendPack.szPan));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60)); // 无60域

	sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);


	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}


uint8_t VoidSaleAuth(void)
{
	uint8_t	ucRet;
	
	ucRet = SaleVoidGetData();
	if( ucRet!=OK )
	{
		return ucRet;
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
	if( stTemp.iTransNo==POS_AUTH_VOID )
	{
		sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s", PosCom.stTrans.lOldBatchNumber,
				PosCom.stTrans.lOldTraceNo, PosCom.stTrans.szOldTxnDate);
	}
	else
	{
		sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d", PosCom.stTrans.lOldBatchNumber,
				PosCom.stTrans.lOldTraceNo);
	}

	ucRet = OnlineCommProc();  
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}
#if 0
uint8_t NetpayDealFeild63(uint8_t *szField63Buff)
{
	uint8_t	/*ucRet,*/sTempBuff[60] = {0};
	char szSNTemp[60] = {0};
	char szPR[60] = {0};
	char szUserTemp[60]= {0};
	char szPasswordTemp[60] = {0};
	char szQ6Temp[60] = {0};
	char szSTTemp[60] = {0};
	char szLGTemp[60] = {0};
	char szAUTemp[60] = {0};//auth code
	char szSPTemp[60] = {0}; // the last 4 card no
	char szDM[60] = {0};
	char szAC[60] = {0};
	char szCV[20] = {0};
	char szRC[20] = {0};
	char szV1[20] = {0};
	char szAv[20] = {0};
	
	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200] = {0};
	int  szTotalLen = 0;
	char AscTipAmount[20] = {0};
	char AscPreAuthTipAmount[20] = {0};

	memset(sTempBuff,0,sizeof(sTempBuff));
	sysReadSN(stPosParam.szSN);
	GetCardType();
/*
	sprintf(szUserTemp,"US%05d %s!",strlen(stPosParam.szUser),stPosParam.szUser);
	szParaNum++;
	szParaLen += strlen(szUserTemp);
	
	sprintf(szPasswordTemp,"PW%05d %s!",strlen(stPosParam.szPassword),stPosParam.szPassword);
	szParaNum++;
	szParaLen += strlen(szPasswordTemp);
*/
	sprintf(szSTTemp,"ST%05d%s!",strlen(stPosParam.szST),stPosParam.szST);
	szParaNum++;
	szParaLen += strlen(szSTTemp);

	sprintf(szDM,"DM%05d%s!",strlen("00"),"00");
	szParaNum++;
	szParaLen += strlen(szDM);

	sprintf(szSNTemp,"XK%05d%2.2s-%3.3s-%3.3s!",strlen(stPosParam.szSN)+2,
		stPosParam.szSN,stPosParam.szSN+2,stPosParam.szSN+5);
	szParaNum++;
	szParaLen += strlen(szSNTemp);


	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG%05d%s",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);	

	sprintf(szV1,"V1000011");
	szParaNum++;
	szParaLen += strlen(szV1);
		
	sprintf(szAv,"AV00006OXGS16");
	szParaNum++;
	szParaLen += strlen(szAv);

	if(strlen(PosComconTrol.szCV) > 0)
	{
		sprintf(szCV,"CV%05d%s!",strlen(PosComconTrol.szCV),PosComconTrol.szCV);
		szParaNum++;
		szParaLen += strlen(szCV);
	}

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	//szTotalLen += szParaNum;     // 加上空格
	sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

	if(strlen(PosComconTrol.szCV) > 0)
	{
		sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szCV,szSNTemp,szLGTemp,szV1,szAv);
	}
	else
	{
		sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv);
	}
		
/*
	if( stTemp.iTransNo == POS_ELEC_SIGN)
	{
		sprintf(szAUTemp,"AU%05d %s!",strlen((char*)PosCom.stTrans.szAuthNo),PosCom.stTrans.szAuthNo);
		szParaNum++;
		szParaLen += strlen(szAUTemp);

		sprintf(szSPTemp,"SP00004 %s!",PosCom.stTrans.szCardNo+strlen((char*)PosCom.stTrans.szCardNo)-4);
		szParaNum++;
		szParaLen += strlen(szSPTemp);
	}

	if(stTemp.iTransNo == POS_AUTH_CM){
		BcdToAsc((uint8_t*)AscTipAmount,PosCom.stTrans.preAuthTipAmount,12);
		sprintf(AscPreAuthTipAmount,"%ld%02ld",atol(AscTipAmount)/100,atol(AscTipAmount)%100);
		sprintf(szPR,"PR%05d %s!",strlen(AscPreAuthTipAmount),AscPreAuthTipAmount);
		szParaNum++;
		szParaLen += strlen(szPR);
	}

	if( stTemp.iTransNo == POS_AUTH_CM ||
		stTemp.iTransNo == CHECK_IN ||
		stTemp.iTransNo == POS_SALE ||
		stTemp.iTransNo == CHECK_OUT ||
		stTemp.iTransNo == NETPAY_REFUND||
		stTemp.iTransNo == NETPAY_ADJUST ||
		PosCom.stTrans.TransFlag == INTERESES_SALE||
		stTemp.iTransNo == NETPAY_FORZADA ||
		stTemp.iTransNo == POS_REFUND ||
		stTemp.iTransNo == POS_PREAUTH)
	{
		sprintf(szDM,"DM000%02d %s!",strlen("00"),"00");
		szParaNum++;
		szParaLen += strlen(szDM);
	}

	if( stTemp.iTransNo == NETPAY_FORZADA )
	{
		sprintf(szAC,"AC000%02d %s!",strlen((char*)PosCom.stTrans.szAC),(char*)PosCom.stTrans.szAC);
		szParaNum++;
		szParaLen += strlen(szAC);
	}
	
	if(stTemp.iTransNo == POS_REFUND )
	{
		if( PosCom.stTrans.iOldTransNo == POS_PREAUTH || 
			PosCom.stTrans.iOldTransNo == CHECK_IN || 
			PosCom.stTrans.iOldTransNo == CHECK_OUT )
		{
			if( (PosCom.stTrans.ucAuthCmlId != TRUE&&PosCom.stTrans.iOldTransNo == POS_PREAUTH) || 
				PosCom.stTrans.iOldTransNo == CHECK_IN )
			{
				strcpy(szRC,"RC00001 2!");
				szParaNum++;
				szParaLen += strlen(szRC);
			}
			else 
			{
				strcpy(szRC,"RC00001 3!");
				szParaNum++;
				szParaLen += strlen(szRC);
			}
		}
		else
		{
			strcpy(szRC,"RC00001 1!");
			szParaNum++;
			szParaLen += strlen(szRC);
		}
		
		
	}

	if(strlen(PosComconTrol.szCV) > 0){
		sprintf(szCV,"CV%05d %s!",strlen(PosComconTrol.szCV),PosComconTrol.szCV);
		szParaNum++;
		szParaLen += strlen(szCV);
	}

	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG%05d %s",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	szTotalLen += szParaNum;     // 加上空格
	sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);	

	if(PosCom.stTrans.szintereseMonth != 0 && PosCom.stTrans.TransFlag == INTERESES_SALE)
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szDM,szQ6Temp,szSTTemp,szCV,szSNTemp,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szDM,szQ6Temp,szSTTemp,szSNTemp,szLGTemp);
		}
	}
	else if(stTemp.iTransNo == POS_AUTH_CM)
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szCV,szPR,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szDM,szSTTemp,szSNTemp,szPR,szLGTemp);
		}
	}
	else if(stTemp.iTransNo == POS_REFUND)
	{
		if( PosCom.stTrans.iOldTransNo == POS_PREAUTH || 
			PosCom.stTrans.iOldTransNo == CHECK_IN ||
			PosCom.stTrans.iOldTransNo == CHECK_OUT )
		{
			if(strlen(PosComconTrol.szCV) > 0)
			{
				sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szRC,szDM,szSNTemp,szLGTemp);
			}
			else
			{
				sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",  szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szRC,szDM,szLGTemp);
			}
		}
		else
		{
			if(strlen(PosComconTrol.szCV) > 0)
			{
				sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szDM,szRC,szSNTemp,szLGTemp);
			}
			else
			{
				sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",  szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szDM,szRC,szSNTemp,szLGTemp);
			}
	
		}
		
	}
	else if( stTemp.iTransNo == POS_ELEC_SIGN)
	{
		sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szAUTemp,szSPTemp,szLGTemp);
	}
	else if(stTemp.iTransNo == NETPAY_REFUND || stTemp.iTransNo ==NETPAY_ADJUST)
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szSNTemp,szDM,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szDM,szLGTemp);
		}
	}
	else if(stTemp.iTransNo == CHECK_OUT || stTemp.iTransNo == CHECK_IN)
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szSNTemp,szDM,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szDM,szLGTemp);
		}
	}
	else if( stTemp.iTransNo == NETPAY_FORZADA )
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szSNTemp,szDM,szAC,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szDM,szAC,szLGTemp);
		}
	}
	else if( stTemp.iTransNo == POS_PREAUTH)
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szDM,szSNTemp,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szDM,szSNTemp,szLGTemp);
		}
	}
	else 
	{
		if(strlen(PosComconTrol.szCV) > 0)
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szCV,szSNTemp,szLGTemp);
		}
		else
		{
			sprintf((char*)szField63Buff,"%s %s %s %s %s %s",szTotalLenTemp,szUserTemp,szPasswordTemp,szSTTemp,szSNTemp,szLGTemp);
		}
	}

	memset(PosComconTrol.szCV,0,sizeof(PosComconTrol.szCV)); // 将安全号置为零
*/
	return OK;
}
#endif
uint8_t Get_Rule_Balance_Feild63(uint8_t *szField63Buff)
{
	uint8_t	sTempBuff[60] = {0};
	char szSNTemp[60] = {0};
	char szSTTemp[60] = {0};
	char szLGTemp[60] = {0};

	char szDM[60] = {0};

	char szV1[20] = {0};
	char szAv[20] = {0};

	//electronic wallet
	char szTemp1[60] = {0};
	char szTemp2[60] = {0};
	char szTemp3[60] = {0};
	char szTemp4[60] = {0};
	char szTemp5[60] = {0}; 
	char szTemp6[60] = {0}; 
	char szTemp7[60] = {0}; 

	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200] = {0};
	int  szTotalLen = 0;

	memset(sTempBuff,0,sizeof(sTempBuff));
	sysReadSN(stPosParam.szSN);
	GetCardType();
	sprintf(szSTTemp,"ST%05d%s!",strlen(stPosParam.szST),stPosParam.szST);
	szParaNum++;
	szParaLen += strlen(szSTTemp);

	sprintf(szDM,"DM%05d%s!",strlen("00"),"00");
	szParaNum++;
	szParaLen += strlen(szDM);

	sprintf(szSNTemp,"XK%05d%2.2s-%3.3s-%3.3s!",strlen(stPosParam.szSN)+2,
		stPosParam.szSN,stPosParam.szSN+2,stPosParam.szSN+5);
	szParaNum++;
	szParaLen += strlen(szSNTemp);


	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG%05d%s!",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);	

	sprintf(szV1,"V1000011!");
	szParaNum++;
	szParaLen += strlen(szV1);

	if(stTemp.iTransNo ==PURSE_GETRULE)
	{
		sprintf(szAv,"AV00006OXGS16!");
		szParaNum++;
		szParaLen += strlen(szAv);
		
		sprintf(szTemp1,"Y1%05d%s!",strlen("1234"),"1234");
		szParaNum++;
		szParaLen += strlen(szTemp1);

		sprintf(szTemp2,"Y2%05d%s!",strlen(stPosParam.stPCpass),stPosParam.stPCpass);
		szParaNum++;
		szParaLen += strlen(szTemp2);
		
		sprintf(szTemp3,"Y3%05d%s!",strlen(stPosParam.stPCPid),stPosParam.stPCPid);
		szParaNum++;
		szParaLen += strlen(szTemp3);

		sprintf(szTemp4,"Y4%05d%s!",strlen(stPosParam.stPCSerial),stPosParam.stPCSerial);
		szParaNum++;
		szParaLen += strlen(szTemp4);

		sprintf(szTemp5,"Y5%05d%s",strlen(PosCom.stTrans.oil_id),PosCom.stTrans.oil_id);
		szParaNum++;
		szParaLen += strlen(szTemp5);
		
		//complete fill in  packet	
		szTotalLen += 13;            // 本身长度
		szTotalLen += szParaLen;     // 其他参数长度
		sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

		sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
			szTemp1,szTemp2,szTemp3,szTemp4,szTemp5);
		

/*
		if(stPosParam.stVolContrFlg ==PARAM_OPEN)
		{
			sprintf(szTemp5,"Y5%05d%s",strlen(PosCom.stTrans.oil_id),PosCom.stTrans.oil_id);
			szParaNum++;
			szParaLen += strlen(szTemp5);
			
			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp2,szTemp3,szTemp4,szTemp5);
			}
		}
		else
		{	
			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp2,szTemp3,szTemp4);
			}
		}
*/	
	}
	else if(stTemp.iTransNo ==PURSE_GETBALANCE)
	{
		if(PosCom.stTrans.TransFlag == PURSE_GETRULE)
		{
			sprintf(szAv,"AV00006OXGS16!");
			szParaNum++;
			szParaLen += strlen(szAv);
			
			sprintf(szTemp1,"Y1%05d%s!",strlen("1234"),"1234");
			szParaNum++;
			szParaLen += strlen(szTemp1);

			sprintf(szTemp2,"Y2%05d%s!",strlen(stPosParam.stPCpass),stPosParam.stPCpass);
			szParaNum++;
			szParaLen += strlen(szTemp2);
			
			sprintf(szTemp3,"Y3%05d%s!",strlen(stPosParam.stPCPid),stPosParam.stPCPid);
			szParaNum++;
			szParaLen += strlen(szTemp3);

			sprintf(szTemp4,"Y4%05d%s!",strlen(stPosParam.stPCSerial),stPosParam.stPCSerial);
			szParaNum++;
			szParaLen += strlen(szTemp4);

			sprintf(szTemp5,"Y5%05d%c!",1,'1');
			szParaNum++;
			szParaLen += strlen(szTemp5);

			sprintf(szTemp6,"Y6%05d%c!",1,'1');
			szParaNum++;
			szParaLen += strlen(szTemp5);
			
			sprintf(szTemp7,"Y7%05d%s",8,"SGE-9012");
			szParaNum++;
			szParaLen += strlen(szTemp5);
			
			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
				szTemp1,szTemp2,szTemp3,szTemp4,szTemp5,szTemp6,szTemp7);
	
		}
		else
		{
			sprintf(szAv,"AV00006OXGS16!");
			szParaNum++;
			szParaLen += strlen(szAv);
			
			sprintf(szTemp1,"E1%05d%s!",strlen(stPosParam.stETerm),stPosParam.stETerm);
			szParaNum++;
			szParaLen += strlen(szTemp1);

			if(strlen(PosCom.stTrans.oil_id) ==0 && strlen(PosCom.stTrans.oil_amount) ==0)
			{
				sprintf(szTemp3,"E4%05d%c!",1,'0');
				szParaNum++;
				szParaLen += strlen(szTemp3);
			}
			else
			{
				sprintf(szTemp4,"E5%05d%s!",strlen(PosCom.stTrans.oil_id),PosCom.stTrans.oil_id);
				szParaNum++;
				szParaLen += strlen(szTemp4);
				
				sprintf(szTemp2,"E3%05d%s!",strlen(PosCom.stTrans.oil_amount),PosCom.stTrans.oil_amount);
				szParaNum++;
				szParaLen += strlen(szTemp2);

				sprintf(szTemp3,"E4%05d%s!",strlen(PosCom.stTrans.oil_price),PosCom.stTrans.oil_price);
				szParaNum++;
				szParaLen += strlen(szTemp3);
			}

			sprintf(szTemp5,"EI%05d%s",strlen(stPosParam.stETId),stPosParam.stETId);
			szParaNum++;
			szParaLen += strlen(szTemp5);
			
			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			if(strlen(PosCom.stTrans.oil_id) ==0 && strlen(PosCom.stTrans.oil_amount) ==0)
			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp3,szTemp5);
			}
			else
			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp4,szTemp2,szTemp3,szTemp5);
			}
		}
	}
		
	return OK;
}
uint8_t NetpayDealFeild63(uint8_t *szField63Buff)
{
	uint8_t	sTempBuff[60] = {0};
	char showbuff[30] ={0};
	
	int len;
	char szSNTemp[60] = {0};
	char szSTTemp[60] = {0};
	char szLGTemp[60] = {0};
	char szDM[60] = {0};
	char szCV[20] = {0};
	char szV1[20] = {0};
	char szAv[20] = {0};

	//electronic wallet
	char szTemp1[60] = {0};
	char szTemp2[60] = {0};
	char szTemp3[60] = {0};
	char szTemp4[60] = {0};
	char szTemp5[60] = {0}; 
	char szTemp6[60] = {0};
	char szTemp7[60] = {0};
	char szTemp8[60] = {0};
	char szTemp9[60] = {0};
	char szTemp10[60] = {0};
	char szTemp11[60] = {0};	
	char szTemp12[60] = {0};


	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200] = {0};
	int  szTotalLen = 0;
	long literamount =0;  
	
	memset(sTempBuff,0,sizeof(sTempBuff));
	sysReadSN(stPosParam.szSN);
	GetCardType();
	sprintf(szSTTemp,"ST%05d%s!",strlen(stPosParam.szST),stPosParam.szST);
	szParaNum++;
	szParaLen += strlen(szSTTemp);

	sprintf(szDM,"DM%05d%s!",strlen("00"),"00");
	szParaNum++;
	szParaLen += strlen(szDM);

	sprintf(szSNTemp,"XK%05d%2.2s-%3.3s-%3.3s!",strlen(stPosParam.szSN)+2,
		stPosParam.szSN,stPosParam.szSN+2,stPosParam.szSN+5);
	szParaNum++;
	szParaLen += strlen(szSNTemp);


	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG%05d%s!",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);	

	sprintf(szV1,"V1000011!");
	szParaNum++;
	szParaLen += strlen(szV1);
		
	switch(PosCom.stTrans.TransFlag)
	{
		case PURSE_SODEXO:

			sprintf(szAv,"AV00006OXGS16!");
			szParaNum++;
			szParaLen += strlen(szAv);

			sprintf(szTemp1,"S3%05d%s!",strlen(PosCom.stTrans.kilometer),PosCom.stTrans.kilometer);
			szParaNum++;
			szParaLen += strlen(szTemp1);

			sprintf(szTemp2,"S2%05d%s!",strlen(PosCom.stTrans.plate),PosCom.stTrans.plate);
			szParaNum++;
			szParaLen += strlen(szTemp2);

			BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
			//printf("glSendPack.szTranAmt:%s\n",glSendPack.szTranAmt);
			for(len =0;len <12;len++)
			{
				printf("szTranAmt[%d]:%c;",len,glSendPack.szTranAmt[len]);
				if(glSendPack.szTranAmt[len] !='0')
					break;
			}
			memcpy(sTempBuff,glSendPack.szTranAmt+len,12-len);

			sprintf(szTemp3,"S1%05d%s:%s!",strlen(sTempBuff)+strlen(PosCom.stTrans.oil_id)+1,PosCom.stTrans.oil_id,sTempBuff);
			szParaNum++;
			szParaLen += strlen(szTemp3);
			
			sprintf(szTemp4,"S4%05d%s!",strlen(PosCom.stTrans.nip),PosCom.stTrans.nip);
			szParaNum++;
			szParaLen += strlen(szTemp4);

			//操作员编号，怎么体现??waiterNo
			memset(sTempBuff,0,sizeof(sTempBuff));
			sprintf(sTempBuff,"%02d",stPosParam.waiterNo);
			sprintf(szTemp5,"S5%05d%s!",strlen(sTempBuff),sTempBuff);
			
			//sprintf(szTemp5,"S5%05d%s!",strlen(stTemp.oil_id),stTemp.oil_id);
			szParaNum++;
			szParaLen += strlen(szTemp5);

			sprintf(szTemp6,"S6%05d%s",strlen(stPosParam.szSN),stPosParam.szSN);
			szParaNum++;
			szParaLen += strlen(szTemp6);

			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp2,szTemp3,szTemp4,szTemp5,szTemp6);
			
			break;
		case PURSE_PUNTO:
			sprintf(szAv,"AV00006OXGS16!");
			szParaNum++;
			szParaLen += strlen(szAv);
			
			sprintf(szTemp1,"Y1%05d%s!",strlen("1234"),"1234");
			szParaNum++;
			szParaLen += strlen(szTemp1);

			sprintf(szTemp2,"Y2%05d%s!",strlen(stPosParam.stPCpass),stPosParam.stPCpass);
			szParaNum++;
			szParaLen += strlen(szTemp2);
			
			sprintf(szTemp3,"Y3%05d%s!",strlen(stPosParam.stPCPid),stPosParam.stPCPid);
			szParaNum++;
			szParaLen += strlen(szTemp3);

			sprintf(szTemp4,"Y4%05d%s!",strlen(stPosParam.stPCSerial),stPosParam.stPCSerial);
			szParaNum++;
			szParaLen += strlen(szTemp4);

			sprintf(szTemp5,"Y5%05d%s!",strlen(PosCom.stTrans.oil_id),PosCom.stTrans.oil_id);
			szParaNum++;
			szParaLen += strlen(szTemp5);

			sprintf(szTemp6,"Y6%05d%s!",strlen(PosCom.stTrans.saletype),PosCom.stTrans.saletype);
			szParaNum++;
			szParaLen += strlen(szTemp6);

			sprintf(szTemp7,"Y7%05d%s!",strlen(PosCom.stTrans.kilometer),PosCom.stTrans.kilometer);
			szParaNum++;
			szParaLen += strlen(szTemp7);

			sprintf(szTemp8,"Y8%05d%s!",strlen(PosCom.stTrans.plate),PosCom.stTrans.plate);
			szParaNum++;
			szParaLen += strlen(szTemp8);

			if(PosCom.stTrans.isnip[0] =='0')
			{				
				sprintf(szTemp9,"%s","YN000011!");
				szParaNum++;
				szParaLen += strlen(szTemp9);
				
				sprintf(szTemp10,"Y9%05d%s",strlen(PosCom.stTrans.nnip),PosCom.stTrans.nnip);
				szParaNum++;
				szParaLen += strlen(szTemp10);
			}
			else
			{
				sprintf(szTemp9,"%s","YN000010");
				szParaNum++;
				szParaLen += strlen(szTemp9);
			}

			
			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp2,szTemp3,szTemp4,szTemp5,szTemp6,szTemp7,szTemp8,szTemp9,szTemp10);
			}
			break;
		case PURSE_TODITO:

			sprintf(szAv,"AV00006OXGS16!");
			szParaNum++;
			szParaLen += strlen(szAv);

			if(strlen(PosCom.stTrans.oil_id)==1)
			{
				memset(sTempBuff,0,sizeof(sTempBuff));
				sprintf(sTempBuff,"0%s",PosCom.stTrans.oil_id);
				sprintf(szTemp1,"T1%05d%s!",strlen(sTempBuff),sTempBuff);
			}
			else
			{
				sprintf(szTemp1,"T1%05d%s!",strlen(PosCom.stTrans.oil_id),PosCom.stTrans.oil_id);
			}

			szParaNum++;
			szParaLen += strlen(szTemp1);

			sprintf(szTemp2,"T2%05d%s!",strlen(PosCom.stTrans.nip),PosCom.stTrans.nip);
			szParaNum++;
			szParaLen += strlen(szTemp2);

			sprintf(szTemp3,"T4%05d%s!",strlen(PosCom.stTrans.oil_price),PosCom.stTrans.oil_price);
			szParaNum++;
			szParaLen += strlen(szTemp3);

			sprintf(szTemp4,"T3%05d%s",strlen(PosCom.stTrans.oil_liters),PosCom.stTrans.oil_liters);
			szParaNum++;
			szParaLen += strlen(szTemp4);

			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
					szTemp1,szTemp2,szTemp3,szTemp4);
			}
			break;
		case PURSE_EDENRED:
			{
				sprintf(szAv,"AV00006OXGS16!");
				szParaNum++;
				szParaLen += strlen(szAv);
				
				sprintf(szTemp1,"E1%05d%s!",strlen(stPosParam.stETerm),stPosParam.stETerm);
				szParaNum++;
				szParaLen += strlen(szTemp1);
				
				sprintf(szTemp5,"E5%05d%s!",strlen(PosCom.stTrans.oil_id),PosCom.stTrans.oil_id);
				szParaNum++;
				szParaLen += strlen(szTemp5);

				memset(sTempBuff,0,sizeof(sTempBuff));
				//BcdToAsc0(sTempBuff, PosCom.stTrans.sAmount, 12); 
				BcdToAsc0(sTempBuff, PosCom.stTrans.sLiters, 12); 
				
				literamount = AscToLong(sTempBuff,strlen(sTempBuff));
				PrintDebug("%s%d", "literamount:",literamount);

				sprintf(showbuff,"%d.%d",literamount/100,literamount%100);
				PrintDebug("%s%s", "showbuf",showbuff);

				sprintf(szTemp3,"E3%05d%s!",strlen(showbuff),showbuff);

				//sprintf(szTemp3,"E3%05d%s!",strlen(stTemp.oil_amount),stTemp.oil_amount);
				szParaNum++;
				szParaLen += strlen(szTemp3);

				sprintf(szTemp4,"E4%05d%s!",strlen(PosCom.stTrans.oil_price),PosCom.stTrans.oil_price);
				szParaNum++;
				szParaLen += strlen(szTemp4);

				//NEED ADD
				sprintf(szTemp6,"E6%05d%s!",strlen(PosCom.stTrans.Driver_num),PosCom.stTrans.Driver_num);
				szParaNum++;
				szParaLen += strlen(szTemp6);

				//NEED ADD
				sprintf(szTemp7,"E7%05d%s!",strlen(PosCom.stTrans.kilometer),PosCom.stTrans.kilometer);
				szParaNum++;
				szParaLen += strlen(szTemp7);
				
				//NEED ADD
				sprintf(szTemp8,"E8%05d%s!",strlen(PosCom.stTrans.plate),PosCom.stTrans.plate);
				szParaNum++;
				szParaLen += strlen(szTemp8);

				if(PosCom.stTrans.isnip[0] =='1')
				{
					//NEED ADD
					sprintf(szTemp12,"EN%05d%s!",strlen(PosCom.stTrans.isnip),PosCom.stTrans.isnip);
					szParaNum++;
					szParaLen += strlen(szTemp12);
					
					//NEED ADD
					sprintf(szTemp9,"E9%05d%s!",strlen(PosCom.stTrans.nip),PosCom.stTrans.nip);
					szParaNum++;
					szParaLen += strlen(szTemp9);

				}
				
				//NEED ADD
				sprintf(szTemp10,"ET%05d%s!",strlen(PosCom.stTrans.Trans_Id),PosCom.stTrans.Trans_Id);
				szParaNum++;
				szParaLen += strlen(szTemp10);
				
				sprintf(szTemp11,"EI%05d%s",strlen(stPosParam.stETId),stPosParam.stETId);
				szParaNum++;
				szParaLen += strlen(szTemp11);
				
				//complete fill in  packet	
				szTotalLen += 13;            // 本身长度
				szTotalLen += szParaLen;     // 其他参数长度
				sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

				if(PosCom.stTrans.isnip[0] =='1')
				{
					sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
						szTemp1,szTemp5,szTemp3,szTemp4,szTemp6,szTemp7,szTemp8,szTemp12,szTemp9,szTemp10,szTemp11);
				}
				else
				{
					sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv,
						szTemp1,szTemp5,szTemp3,szTemp4,szTemp6,szTemp7,szTemp8,szTemp10,szTemp11);
				}
				
			}
			break;
		default:
			sprintf(szAv,"AV00006OXGS16");
			szParaNum++;
			szParaLen += strlen(szAv);

			if(strlen(PosComconTrol.szCV) > 0)
			{
				sprintf(szCV,"CV%05d%s!",strlen(PosComconTrol.szCV),PosComconTrol.szCV);
				szParaNum++;
				szParaLen += strlen(szCV);
			}
			//complete fill in  packet	
			szTotalLen += 13;            // 本身长度
			szTotalLen += szParaLen;     // 其他参数长度
			sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);

			if(strlen(PosComconTrol.szCV) > 0)
			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szCV,szSNTemp,szLGTemp,szV1,szAv);
			}
			else
			{
				sprintf((char*)szField63Buff,"%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1,szAv);
			}
			break;
	}
	return OK;
}


//上传打印信息，后台保存到数据库
uint8_t DealFeild120(uint8_t *szField120Buff)
{
//	char szSNTemp[60];
	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200];
	int  szTotalLen = 0;

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	uint8_t TransDate[20];
	uint8_t szLGTemp[60] = {0};
        
	uint8_t szT1Temp[60] = {0};   // stPrintEntryMode C/M		
	uint8_t szLNTemp[60] = {0};	// water no 
	uint8_t szCnTemp[60] = {0};	// batch no	f
	uint8_t szTnTemp[60] = {0};   // terminal no
	uint8_t szThTemp[100] = {0};   // CardHolderName
	uint8_t szCkTemp[100] = {0};   // 和preauth区分
	uint8_t szAlTemp[60] = {0};   
	uint8_t szMtTemp[60] = {0};   // card type
	uint8_t szFcTemp[60] = {0};   // trans date
	uint8_t szVtTemp[60] = {0};   // version

	sprintf((char*)szT1Temp,"T100001 %c!",GetEntryMode());
	szParaNum++;
	szParaLen += strlen((char*)szT1Temp);
	
	sprintf((char*)szLNTemp,"LN00006 %06d!",PosCom.stTrans.lBatchNumber);
	szParaNum++;
	szParaLen += strlen((char*)szLNTemp);
	
	sprintf((char*)szCnTemp,"CN00006 %06d!",PosCom.stTrans.lTraceNo );
	szParaNum++;
	szParaLen += strlen((char*)szCnTemp);

	sprintf((char*)szTnTemp,"TN%05d %s!",strlen((char*)PosCom.stTrans.szPosId),PosCom.stTrans.szPosId);
	szParaNum++;
	szParaLen += strlen((char*)szTnTemp);

	sprintf((char*)szThTemp,"TH%05d %s!",strlen((char*)PosCom.stTrans.CardHolderName),PosCom.stTrans.CardHolderName);
	szParaNum++;
	szParaLen += strlen((char*)szThTemp);

	sprintf((char*)szAlTemp,"AL%05d %s!",strlen((char*)PosCom.stTrans.szAppLable),PosCom.stTrans.szAppLable);
	szParaNum++;
	szParaLen += strlen((char*)szAlTemp);

	if ( stTemp.iTransNo==CHECK_IN || stTemp.iTransNo==CHECK_OUT )
	{
		sprintf((char*)szCkTemp,"CK00001 1!");
		szParaNum++;
		szParaLen += strlen((char*)szCkTemp);
	}
	

	memcpy((char*)szAscCardNoTemp,PosCom.stTrans.szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡		
	}
	sprintf((char*)szMtTemp,"MT%05d %s!",strlen((char *)szCardType),szCardType);
	szParaNum++;
	szParaLen += strlen((char*)szMtTemp);

	MakeFormatPrintDate(PosCom.stTrans.szDate,TransDate);
	sprintf((char*)szFcTemp,"FC%05d %s!",strlen((char *)TransDate),TransDate);
	szParaNum++;
	szParaLen += strlen((char *)szFcTemp);

	sprintf((char*)szVtTemp,"VT%05d %s!",strlen(gstPosVersion.szVersion),gstPosVersion.szVersion);
	szParaNum++;
	szParaLen += strlen((char*)szVtTemp);

	strcpy(stPosParam.szLG,"123456");
	sprintf((char*)szLGTemp,"LG%05d %s",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen((char*)szLGTemp);

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	szTotalLen += szParaNum;     // 加上空格
	sprintf((char*)szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);	

	if ( stTemp.iTransNo==CHECK_IN || stTemp.iTransNo==CHECK_OUT )
	{
		sprintf((char*)szField120Buff,"%s %s %s %s %s %s %s %s %s %s %s %s",
		szTotalLenTemp,szT1Temp,szLNTemp,szCnTemp,szTnTemp,
		szThTemp,szAlTemp,szMtTemp,szFcTemp,szVtTemp,szCkTemp,szLGTemp);
	}
	else
	{
		sprintf((char*)szField120Buff,"%s %s %s %s %s %s %s %s %s %s %s",
			szTotalLenTemp,szT1Temp,szLNTemp,szCnTemp,szTnTemp,
			szThTemp,szAlTemp,szMtTemp,szFcTemp,szVtTemp,szLGTemp);
	}
	

	return OK;
}



uint8_t NetpayDealFeild120(uint8_t *szField120Buff,int RI)
{
	uint8_t	/*ucRet,*/sTempBuff[60];
	char szUserTemp[60];
	char szSKTemp[60];
	char szRITemp[60];
	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200];
	int  szTotalLen = 0;

	memset(sTempBuff,0,sizeof(sTempBuff));
	sprintf(szRITemp,"RI00001 %d!",RI);
	szParaNum++;
	szParaLen += strlen(szRITemp);

	sprintf(szSKTemp,"SK000%02d %s!",strlen(PosComconTrol.szSKU),PosComconTrol.szSKU);
	szParaNum++;
	szParaLen += strlen(szSKTemp);

	sprintf(szUserTemp,"OP000%02d %s",strlen(stPosParam.szUser),stPosParam.szUser);
	szParaNum++;
	szParaLen += strlen(szUserTemp);

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	szTotalLen += szParaNum;     // 加上空格
	sprintf(szTotalLenTemp,"& 000%02d000%02d!",szParaNum,szTotalLen);	

	sprintf((char*)szField120Buff,"%s %s %s %s",szTotalLenTemp,szRITemp,szSKTemp,szUserTemp);
	return OK;
}



uint8_t NetpayPaywithCardDealFeild63(uint8_t *szField63Buff)
{
	uint8_t	/*ucRet,*/sTempBuff[60];
	char szSNTemp[60];
	char szUserTemp[60];
	char szPasswordTemp[60];
	char szSTTemp[60];
	char szLGTemp[60];
	char szDM[20];
	char szOrigAmount[20];     //原交易金额
	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200];
	int  szTotalLen = 0;
	char szAmount[20];
	char szValidAmount[13];

	memset(sTempBuff,0,sizeof(sTempBuff));
//	memset(szSN,0,sizeof(szSN));
	sysReadSN(stPosParam.szSN);

	sprintf(szUserTemp,"US000%02d %s!",strlen(stPosParam.szUser),stPosParam.szUser);
	szParaNum++;
	szParaLen += strlen(szUserTemp);
	
	sprintf(szPasswordTemp,"PW000%02d %s!",strlen(stPosParam.szPassword),stPosParam.szPassword);
	szParaNum++;
	szParaLen += strlen(szPasswordTemp);
	
	sprintf(szSTTemp,"ST000%02d %s!",strlen(stPosParam.szST),stPosParam.szST);
	szParaNum++;
	szParaLen += strlen(szSTTemp);

	sprintf(szSNTemp,"XK000%02d %2.2s-%3.3s-%3.3s!",strlen(stPosParam.szSN)+2,
		stPosParam.szSN,stPosParam.szSN+2,stPosParam.szSN+5);
	szParaNum++;
	szParaLen += strlen(szSNTemp);

	sprintf(szDM,"DM000%02d %s!",strlen("00"),"00");
	szParaNum++;
	szParaLen += strlen(szDM);

	PubBcd2Asc0(PosCom.stTrans.sAmount,6,szAmount);
	sprintf(szValidAmount,"%ld.%02ld",atol(szAmount)/100,atol(szAmount)%100);
	sprintf(szOrigAmount,"A5000%03d %ld.%02ld!",strlen(szValidAmount),atol(szAmount)/100,atol(szAmount)%100);
	szParaNum++;
	szParaLen += strlen(szOrigAmount);

	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG000%02d %s",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	szTotalLen += szParaNum;     // 加上空格
	sprintf(szTotalLenTemp,"& 000%02d00%03d!",szParaNum,szTotalLen);	

	sprintf((char*)szField63Buff,"%s %s %s %s %s %s %s %s",szTotalLenTemp,szSTTemp,szUserTemp,szPasswordTemp,szDM,szOrigAmount,szSNTemp,szLGTemp);
	return OK;
}

/*
uint8_t RefundTran(void)
{
	uint8_t	ucRet,sTempBuff[60];
	char szField63BuffTemp[400];
	
	ucRet = RefundTranGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"200000");
	//NETPAY 不上送三磁道
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.szField60,0,sizeof(glSendPack.szField60)); // 无60域
	
	sprintf((char *)glSendPack.szRRN, "%.*s", LEN_RRN, PosCom.stTrans.szSysReferNo);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	sprintf((char *)glSendPack.szEntryMode, "0001");
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	sprintf((char *)glSendPack.szCurrencyCode, "484"); // 货币代码
		
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}
*/
uint8_t PreAuth(void)
{
	uint8_t	ucRet/*,sBuff[10],*/;
	char szField63BuffTemp[999];
//	char szField120BuffTemp[999];

	PosCom.stTrans.TransFlag = POS_PREAUTH;
	ucRet = PreAuthGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	if(PosCom.stTrans.TransFlag == INTERESES_SALE)
	{
		SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");
	}
	else
	{
		SetCommReqField((uint8_t *)"0100", (uint8_t *)"776610");
	}
	
	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	memset(PosCom.stTrans.szTransDate,0,sizeof(PosCom.stTrans.szTransDate));
	memset(PosCom.stTrans.szTransTime,0,sizeof(PosCom.stTrans.szTransTime));
	strcpy((char *)PosCom.stTrans.szTransDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTransTime, (char *)stTemp.szTime);

	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	memset(glSendPack.szTrack3,0,sizeof(glSendPack.szTrack3));
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));

	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id

	sprintf((char *)glSendPack.szNII, "0003");  
	sprintf((char*)glSendPack.szExtAmount,"%d",stPosParam.ucTipper);

	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	#if 0
	DealFeild120((uint8_t *)szField120BuffTemp);
	memset(glSendPack.szField120,0,sizeof(glSendPack.szField120));
	strcpy((char*)glSendPack.szField120,szField120BuffTemp);
	#endif
		
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}

uint8_t VoidPreAuth(void)
{
	uint8_t	ucRet;
	
	ucRet = PreAuthVoidGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	SetCommReqField((uint8_t *)"0100", (uint8_t *)"200000");
	sprintf((char *)glSendPack.szAuthCode, "%.6s", PosCom.stTrans.szAuthNo);
	sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s", PosCom.stTrans.lOldBatchNumber,
			PosCom.stTrans.lOldTraceNo, PosCom.stTrans.szOldTxnDate);
	ucRet = OnlineCommProc(); 
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}

uint8_t PreAuthAdd(void)
{
	uint8_t	ucRet;
	
	ucRet = PreAuthAddInput();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	SetCommReqField((uint8_t *)"0100", (uint8_t *)"030000");
	sprintf((char *)glSendPack.szAuthCode, "%.6s", PosCom.stTrans.szAuthNo);
	sprintf((char *)glSendPack.szOrginalMsg, "%016d", 0);
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}


uint8_t OnlineAuthConfirm(void)
{
	uint8_t	ucRet /*,sBuff[10]*/;
	char szField63BuffTemp[400];

	PosCom.stTrans.TransFlag = POS_AUTH_CM;
	ucRet = AuthComfirmGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	SetCommReqField((uint8_t *)"0100", (uint8_t *)"776611");
	sprintf((char *)glSendPack.szPan,"%s",PosCom.stTrans.szCardNo);
//	sprintf((char *)glSendPack.szAuthCode, "%.*s", LEN_AUTH_CODE, PosCom.stTrans.szAuthNo);
	GetPosTime(stTemp.szDate, stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(stTemp.szDate+4));

	memset(glSendPack.szAuthCode,0,sizeof(glSendPack.szAuthCode));
	memset(glSendPack.szCaptureCode,0,sizeof(glSendPack.szCaptureCode));
	sprintf((char*)glSendPack.szRRN,"%s",(char*)PosCom.stTrans.szSysReferNo);
	strcpy((char*)glSendPack.szField48,stPosParam.szST);	// store id
	sprintf((char *)glSendPack.szNII, "0003");


//	BcdToAsc((char*)glSendPack.szExtAmount,(char*)PosCom.stTrans.preAuthTipAmount,12);
	
	NetpayDealFeild63((uint8_t *)szField63BuffTemp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	
	ucRet = OnlineCommProc(); 
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;
}

uint8_t OfflineAuthConfirm(void)
{
	uint8_t	ucRet;
	
	ucRet = AuthComfirmGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	SetCommReqField((uint8_t *)"0220", (uint8_t *)"000000");
	sprintf((char *)glSendPack.szAuthCode, "%.*s", LEN_AUTH_CODE, PosCom.stTrans.szAuthNo);
	sprintf((char *)glSendPack.szOrginalMsg, "%06d%06d%4.4s", PosCom.stTrans.lOldBatchNumber,
			PosCom.stTrans.lOldTraceNo, PosCom.stTrans.szOldTxnDate);
	
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}

uint8_t ProcBalanceCheck(char* H1,char* H2)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)glRecvPack.szField63, "H1");
	if ( Temp1 == NULL ) {
		return -1;
	}

	strncpy(szParams,Temp1+2,5);
	szLen = atoi(szParams);
	strncpy(H1,Temp1+8,szLen);
	H1[szLen] = '\0';

	sprintf(PosComconTrol.szAmount,"%s",H1);

	Temp1 = strstr((char*)glRecvPack.szField63, "H2");
	if ( Temp1 == NULL ) {
		return -1;
	}
	memset(szParams,0,sizeof(szParams));
	strncpy(szParams,Temp1+2,5);
	szLen = atoi(szParams);
	strncpy(H2,Temp1+8,szLen);
	H2[szLen] = '\0';

	sprintf(PosComconTrol.szAvailablebalance,"%s",H2);

	return OK;
}


uint8_t ProcCheckAllBanlance(char* T0,char* T1,char* T2,char* T3,char* T4,char* T5,char* T6)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)glRecvPack.szField59, "T3");
	if ( Temp1 == NULL ) {
		return -1;
	}

	strncpy(szParams,Temp1+2,5);
	szLen = atoi(szParams);
	strncpy(T3,Temp1+8,szLen);
	T3[szLen] = '\0';

	Temp1 = strstr((char*)glRecvPack.szField59, "T4");
	if ( Temp1 == NULL ) {
//		return -1;
	}
	else{
		memset(szParams,0,sizeof(szParams));
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(T4,Temp1+8,szLen);
		T4[szLen] = '\0';
	}
	

	Temp1 = strstr((char*)glRecvPack.szField60, "T6");
	if ( Temp1 == NULL ) {
//		return -1;
	}
	else{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(T6,Temp1+8,szLen);
		T6[szLen] = '\0';
	}
	
	

	Temp1 = strstr((char*)glRecvPack.szField60, "T5");
	if ( Temp1 == NULL ) {
		return -1;
	}
	else{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(T5,Temp1+8,szLen);
		T5[szLen] = '\0';
	}
	

	Temp1 = strstr((char*)glRecvPack.szField63, "T0");
	if ( Temp1 == NULL ) {
		return -1;
	}
	else{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(T0,Temp1+8,szLen);
		T0[szLen] = '\0';
	}
	

	Temp1 = strstr((char*)glRecvPack.szField63, "T1");
	if ( Temp1 == NULL ) {
		return -1;
	}
	else{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(T1,Temp1+8,szLen);
		T1[szLen] = '\0';
	}
	

	Temp1 = strstr((char*)glRecvPack.szField63, "T2");
	if ( Temp1 == NULL ) {
		return -1;
	}
	else{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(T2,Temp1+8,szLen);
		T2[szLen] = '\0';
	}
	

	return OK;
}



uint8_t ProcPayWithCardBalanceCheck(char* C0)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)glRecvPack.szField63, "C0");
	if ( Temp1 == NULL ) {
		return -1;
	}

	strncpy(szParams,Temp1+2,5);
	szLen = atoi(szParams);
	strncpy(C0,Temp1+8,szLen);
	C0[szLen] = '\0';

	strcpy(PosComconTrol.szCommissionPercen,C0);
	return OK;
}


uint8_t QueryBalance(void)
{
	uint8_t	ucRet;
	int     iRet;
	
	// 交易数据收集
	DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, "     查余额     ", "     BALANCE    ");
	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		ucRet = PosGetCard(CARD_SWIPED|CARD_INSERTED|CARD_PASSIVE);
	else
		ucRet = PosGetCard(CARD_SWIPED);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if (PosCom.ucSwipedFlag!=CARD_PASSIVE)
	{
		iRet = EnterPIN(0);
		if( iRet!=OK )
		{
			DispEppPedErrMsg(iRet);
			return NO_DISP;
		}
	}

	// 设置交易请求数据
	SetCommReqField((uint8_t *)"0200", (uint8_t *)"310000");
	ucRet = OnlineCommProc();   // 联机通讯过程
	if( ucRet!=OK )
	{
		return ucRet;
	}
		
	return NO_DISP;
}

uint8_t OfflineSale(void)
{
	uint8_t	ucRet;
	
	ucRet = OffSaleGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	strcpy((char *)PosCom.szRespCode, "00");
	ucRet = AfterCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	return NO_DISP;
}

uint8_t OfflineAdjust(void)
{
	uint8_t	ucRet;
	
	ucRet = AdjustTranGetData();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	strcpy((char *)PosCom.szRespCode, "00"); 
	ucRet = AfterCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	return NO_DISP;
}


uint8_t TestResponse(void)
{	
	uint8_t	ucRet;
	
	ucRet = OneTwoSelect("RESPONE", "TEST", "CANCEL");

	if( ucRet!=KEY1 )
	{
		return NO_DISP;
	}

	InitCommData();
	stTemp.iTransNo = RESPONSE_TEST;
	SetCommReqField((uint8_t *)"0820", (uint8_t *)"");
	ucRet = SendRecvPacket(); 
	CommHangUp(FALSE);
	DispResult(ucRet);

	return NO_DISP;
}

uint8_t AfterCommProc(void)
{
	uint8_t	ucRet, ucTransOk, ucPrintSlip;
	int iRet =0;

	if( stTemp.iTransNo==POS_SETT ||PosCom.stTrans.isCtlplat ==1)
	{
		return OK;
	}
	//test autoupdate
	//PosComconTrol.AutoUpdateFlag[0] = '1';
	//PosComconTrol.AutoUpdateFlag[1] = '\0';

	PrintDebug("%s %s", "AutoUpdateFlag",PosComconTrol.AutoUpdateFlag);
	if (ChkAcceptTxnCode((char*)PosCom.szRespCode))
	{
		if(PURSE_GETBALANCE== stTemp.iTransNo)
		{
			return OK;
		}
		ucTransOk = TRUE; 
		if (OFF_ADJUST != stTemp.iTransNo)
		{
			ledSetStatus(LED_NOT_READY );
			lcdClrLine(2, 7);											//SUCCESS
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "APROBADO");
			lcdFlip();
		}
	}
	else
	{
		NetpaydispRetCode(0);
		ucTransOk = FALSE;
	}

	if( ucTransOk==FALSE )
	{
		AfterTransBatch();
		PrintFaildTicket(0,glRecvPack.szRspCode,(uint8_t*)PosComconTrol.szRM);
		return NO_DISP;
	}
	
	if( stTemp.iTransNo==POS_PREAUTH_ADD )
	{
		AscToBcd(PosCom.stTrans.sPreAddAmount, &glRecvPack.szExtAmount[2], 12);
	}
	ucPrintSlip = FALSE;

	//保存卡类型
	iRet = GetCardType();

	if (ChkIfSaveLog())
	{
		if (PosCom.bOnlineTxn &&(stTemp.iTransNo == EC_QUICK_SALE || stTemp.iTransNo == EC_NORMAL_SALE))
		{
			PosCom.stTrans.ucSendFlag = TRUE;
		}

		ucRet = SaveLogFile();
		
		if( ucRet!=OK )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "FILE ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(2000);
		}
		else
		{
			SaveCtrlParam();
		}
		
		update_limit_total();
		ucPrintSlip = FALSE;	
		
		if( stTemp.iTransNo==OFF_SALE || stTemp.iTransNo == OFF_ADJUST || stTemp.iTransNo == ADJUST_TIP)
		{	
			stTransCtrl.sTxnRecIndexList[PosCom.stTrans.uiTxnRecIndex] = TS_TXN_OFFLINE;
			SaveCtrlParam();
		}
		if (stTemp.iTransNo==ICC_OFFSALE || 
			(stTemp.iTransNo == EC_QUICK_SALE && !PosCom.bOnlineTxn) || 
			(stTemp.iTransNo == EC_NORMAL_SALE && !PosCom.bOnlineTxn) )
		{
			stTransCtrl.sTxnRecIndexList[PosCom.stTrans.uiTxnRecIndex] = TS_ICC_OFFLINE;
			SaveCtrlParam();
		}
	}
	
	if( ChkIfUpdateLog())
	{
		ucRet = UpdateLogFile();
		if( ucRet!=OK )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "FILE ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(2000);
		}
	}


	if( stTemp.iTransNo==POS_QUE)
	{
		DispBalance();
		return OK;
	}
		
	if(strlen((char *)glRecvPack.szAuthCode) != 0 && strcmp((char *)glRecvPack.szRspCode,"00") == 0)
	{
		lcdClrLine(2, 7);	
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "OPERACION APROB");
		lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "%s",(char *)PosCom.stTrans.szAuthNo);
		lcdFlip();
		OkBeep();
		kbGetKeyMs(1000);
	}
	
	
	if(stTemp.iTransNo==POS_SALE ||stTemp.iTransNo==PURSE_SALE ||stTemp.iTransNo==PURSE_AUTH)
	{
		ucPrintSlip=TRUE; 
	}

	if( ucPrintSlip==TRUE )
	{
		stPosParam.ucReprint= 1;

		SaveAppParam();	
		if (ChkIfElecSignature())
		{	
			ucRet = TransElecSignature();
		}
		else if( stTemp.iTransNo==POS_SALE  ||stTemp.iTransNo==PURSE_SALE )			
		{
			NetpayPrtTranTicket(0);
		}
		else if(stTemp.iTransNo==PURSE_AUTH)		
		{
			PrtPreAuthTicket(0);
		}
		stPosParam.ucReprint= 0;

		SaveAppParam();
	}

	if(stTemp.iTransNo != PURSE_AUTH)
	{
		AfterTransBatch();
	}
	return OK;
}

void SetCommReqField(uint8_t *pszMsgCode, uint8_t *pszProcCode)
{
	uint8_t	ucIndex, sBuff[200];
	char    sTempBuff[60];

	// set common bits
	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  pszMsgCode);
	sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, pszProcCode);
	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	// set bit 11
	if( stTemp.iTransNo!=RESPONSE_TEST      && stTemp.iTransNo!=POS_DOWNLOAD_PARAM &&
		stTemp.iTransNo!=POS_UPLOAD_PARAM   && stTemp.iTransNo!=QUERY_EMV_CAPK     &&
		stTemp.iTransNo!=QUERY_EMV_PARAM    && stTemp.iTransNo!=DOWNLOAD_EMV_CAPK  &&
		stTemp.iTransNo!=DOWNLOAD_EMV_PARAM && stTemp.iTransNo!=END_EMV_CAPK       &&
		stTemp.iTransNo!=END_EMV_PARAM && stTemp.iTransNo!=END_DOWNLOADBLK )    
	{
		sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	}

	// set bit 60
	SetCommField60();
	if( stTemp.iTransNo==POS_LOGON && stPosParam.ucKeyMode==KEY_MODE_3DES )
	{
		return;
	}
	if( stTemp.iTransNo==ICC_BATCHUP || stTemp.iTransNo==ICC_FAIL_BATCHUP)
	{
		return;
	}

	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	
	ucIndex = GetTxnTableIndex(stTemp.iTransNo);
	if( stTemp.iTransNo==POS_LOGON          || stTemp.iTransNo==POS_LOGOFF         ||
		stTemp.iTransNo==RESPONSE_TEST      || stTemp.iTransNo==POS_DOWNLOAD_PARAM ||
		stTemp.iTransNo==POS_UPLOAD_PARAM   || stTemp.iTransNo==QUERY_EMV_CAPK     ||
		stTemp.iTransNo==QUERY_EMV_PARAM    || stTemp.iTransNo==DOWNLOAD_EMV_CAPK  ||
		stTemp.iTransNo==DOWNLOAD_EMV_PARAM || stTemp.iTransNo==END_EMV_CAPK       ||
		stTemp.iTransNo==END_EMV_PARAM      || stTemp.iTransNo==POS_SETT           ||
		stTemp.iTransNo==POS_BATCH_UP       || stTemp.iTransNo==END_DOWNLOADBLK)     
	{
/*		sprintf((char *)glSendPack.szField60, "%s%06d%s", TrTable[ucIndex].szMessTypeCode,
				stPosParam.lNowBatchNum, TrTable[ucIndex].szNetManInfoCode);
*/		return;
	}

	// set bit 4, txn amount
	if( (PosCom.stTrans.iTransNo==ADJUST_TIP || PosCom.stTrans.iTransNo==ADJUST_TIP_BAT)
		|| (PosCom.stTrans.iTransNo==POS_ELEC_SIGN &&
		(PosCom.stTrans.iOldTransNo==ADJUST_TIP || PosCom.stTrans.iOldTransNo==ADJUST_TIP_BAT)))
	{
		memcpy(sBuff, PosCom.stTrans.sAmount, 6);
		BcdAdd(sBuff, PosCom.stTrans.sTipAmount, 6);
		BcdToAsc0(glSendPack.szTranAmt, sBuff, 12);
	}
	else if( !AmountIsZero(PosCom.stTrans.sAmount, 6) )
	{
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
	}

	if (PosCom.stTrans.iTransNo == POS_ELEC_SIGN)
	{		
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
		if( HaveMacData(stTemp.iTransNo) )
		{
			ShortToByte(glSendPack.sMacData, LEN_MAC_DATA);
		}
		return;
	}

	//sprintf((char *)glSendPack.szNII, "0003");      // 24域NII

	// set entry mode(bit 22) & track 2/3
	if( stTemp.iTransNo!=POS_REFUND )
	{
		sprintf((char *)PosCom.stTrans.szEntryMode, "000");
	}	

	PosCom.stTrans.ReadCardType = PosCom.ucSwipedFlag;
	switch( PosCom.ucSwipedFlag ){
	case CARD_KEYIN:
		if( stTemp.iTransNo==POS_REFUND )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);
			break;
		}
		memcpy(PosCom.stTrans.szEntryMode, "01", 2);
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
		sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);

		if( stTemp.iTransNo==NETPAY_FORZADA && PosCom.ucSwipedFlag==CARD_KEYIN)
		{
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);						
		}
		break;
		
	case CARD_SWIPED:
		if( stTemp.iTransNo==POS_REFUND )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);
			break;
		}
		memcpy(PosCom.stTrans.szEntryMode, "052", 3);
		if (stPosParam.ucTrackEncrypt == PARAM_CLOSE)
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,   PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, PosCom.szTrack3);
			sprintf((char *)glSendPack.szField45, "%.*s", strlen((char*)PosCom.szTrack1), PosCom.szTrack1);
		}
		else
		{
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			EncrypteFieldTrac23(PosCom.szTrack2,glSendPack.szTrack2);
			EncrypteFieldTrac23(PosCom.szTrack3,glSendPack.szTrack3);
			sprintf((char *)glSendPack.szField45, "%.*s", strlen((char*)PosCom.szTrack1), (char*)PosCom.szTrack1);
		}
		break;
	case CARD_INSERTED:
		if( stTemp.iTransNo==POS_REFUND )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);
			break;
		}
		memcpy(PosCom.stTrans.szEntryMode, "05", 2);
		if (PosCom.stTrans.iTransNo == EC_TOPUP_NORAML)
		{
			if( PosCom.stTrans.ucEcTopOldSwipeFlag == CARD_SWIPED )
			{
				if (stPosParam.ucTrackEncrypt == PARAM_CLOSE)
				{
					sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
					sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, PosCom.szTrack3);
//					sprintf((char *)glSendPack.szField45, "%.*s", LEN_TRACK1, PosCom.szTrack1);
				}
				else
				{
					EncrypteFieldTrac23(PosCom.szTrack2,glSendPack.szTrack2);
					EncrypteFieldTrac23(PosCom.szTrack3,glSendPack.szTrack3);
				}
			}
			sprintf((char *)glSendPack.szPan,      "%.*s", LEN_PAN,    PosCom.stTrans.szTranferCardNo);
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
			if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
			{	// TIP interoper.04-T02-SC01 
				sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
			}
		}
		else
		{
			if (PosCom.stTrans.iTransNo != EC_TOPUP_SPEC)
			{
				if(PosCom.stTrans.iTransNo != POS_MAGTOPUP_ACCT || PosCom.ucSwipedFlag==CARD_SWIPED)
				{
					if (stPosParam.ucTrackEncrypt == PARAM_CLOSE)
					{
						sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);						
						sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, PosCom.szTrack3);
//						sprintf((char *)glSendPack.szField45, "%.*s", LEN_TRACK1, PosCom.szTrack1);
					}
					else
					{
						EncrypteFieldTrac23(PosCom.szTrack2,glSendPack.szTrack2);
						EncrypteFieldTrac23(PosCom.szTrack3,glSendPack.szTrack3);
					}
				}
			}	

			sprintf((char *)glSendPack.szPan,      "%.*s", LEN_PAN,    PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
			if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
			{	// TIP interoper.04-T02-SC01 
				sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
			}
		}
		
		
		break;
		
	case FALLBACK_SWIPED:
		if( stTemp.iTransNo==POS_REFUND )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);
			break;
		}
		memcpy(PosCom.stTrans.szEntryMode, "02", 2);
		if (stPosParam.ucTrackEncrypt == PARAM_CLOSE)
		{
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, PosCom.szTrack3);
		}
		else
		{
			EncrypteFieldTrac23(PosCom.szTrack2,glSendPack.szTrack2);
			EncrypteFieldTrac23(PosCom.szTrack3,glSendPack.szTrack3);
		}
		sprintf((char *)glSendPack.szPan,      "%.*s", LEN_PAN,    PosCom.stTrans.szCardNo);
		PosCom.stTrans.ucFallBack = TRUE;
		break;

	case CARD_PASSIVE:
		if( stTemp.iTransNo==POS_REFUND )
		{
			sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);
			break;
		}
		memcpy(PosCom.stTrans.szEntryMode, "07", 2);
		sprintf((char *)glSendPack.szPan,      "%.*s", LEN_PAN,    PosCom.stTrans.szCardNo);
		if (stPosParam.ucTrackEncrypt == PARAM_CLOSE)
		{
			sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, PosCom.szTrack3);
		}
		else
		{
			EncrypteFieldTrac23(PosCom.szTrack2,glSendPack.szTrack2);
			EncrypteFieldTrac23(PosCom.szTrack3,glSendPack.szTrack3);
		}
		if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
		{	// TIP interoper.04-T02-SC01 
			sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
		}
		break;
	}
//	UpdateEntryModeByPIN();
	sprintf((char *)glSendPack.szEntryMode, "%s0", PosCom.stTrans.szEntryMode);

	if( stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.stTrans.ucEcTopOldSwipeFlag == CARD_SWIPED) 
	{
		memset((char *)glSendPack.szPan,0,sizeof(glSendPack.szPan));
		glSendPack.szEntryMode[1] = '2';
	}

	// set bit 25 & 49
	sprintf((char *)glSendPack.szSvrCode, "%s", TrTable[ucIndex].szSevPointCode);
	if( PosCom.stTrans.szMoneyType==PARAM_DOLARES)
	{
		sprintf((char *)glSendPack.szCurrencyCode, "840");		//dollars
	}
	else
	{
		sprintf((char *)glSendPack.szCurrencyCode, "484");
	}
	
	
	// set bit 48, tip amount
	if( PosCom.stTrans.iTransNo==OFF_SALE       || PosCom.stTrans.iTransNo==OFF_ADJUST   ||
		PosCom.stTrans.iTransNo==ADJUST_TIP     || PosCom.stTrans.iTransNo==OFF_SALE_BAT ||
		PosCom.stTrans.iTransNo==OFF_ADJUST_BAT || PosCom.stTrans.iTransNo==ADJUST_TIP_BAT )
	{
		if( !AmountIsZero(PosCom.stTrans.sTipAmount, 6) )
		{
			BcdToAsc0(glSendPack.szField48, PosCom.stTrans.sTipAmount, 12);
		}
	}

	if (PosCom.stTrans.iTransNo == EC_TOPUP_NORAML)
	{
		strcpy((char*)glSendPack.szField48,"0520");	
		ShortToByte(glSendPack.sField62, strlen((char*)PosCom.stTrans.szCardNo));
		strcpy((char *)&glSendPack.sField62[2],(char*)PosCom.stTrans.szCardNo);
	}

	// set length bytes of bit 64 for iPack8583()
	if( HaveMacData(stTemp.iTransNo) )
	{
		ShortToByte(glSendPack.sMacData, LEN_MAC_DATA);
	}

	// NETPAY 无效PIN
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));  
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));
}

uint8_t DownLoadCAPK(uint8_t ucNeedBak)
{
	uint8_t		ucRet, szProcInfo[16+1];
	int		iCnt;
	struct issuer_ca_public_key	stNewCapkList[MAX_KEY_NUM];
	CapkInfo	stCapkInfoList[MAX_KEY_NUM];

	if (ucNeedBak)
	{
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));	
		memcpy(&BackPosCom, &PosCom, COM_SIZE);
	}

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_CFONT|DISP_INVLINE,NULL, "   LOAD IC KEY  ");
	lcdFlip();

	if( stPosParam.ucEmvSupport!=PARAM_OPEN )
	{
		stTransCtrl.bEmvCapkLoaded = TRUE;
		SaveCtrlParam();
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_MEDIACY|DISP_CFONT,NULL, "NOT IC KEY");
		lcdFlip();
		kbGetKeyMs(2000);
		return NO_DISP;
	}

	iCapkNum        = 0;
	pstCapkInfoList = &stCapkInfoList[0];
	memset(&stCapkInfoList[0], 0, sizeof(stCapkInfoList));
	memset(&stNewCapkList[0],  0, sizeof(stNewCapkList));
	
	ucRet = QueryEmvCapk();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	
	emv_delete_all_ca_public_keys();
	stTransCtrl.bEmvCapkLoaded = FALSE;
	SaveCtrlParam();
	fileRemove("emv.capk");

	lcdClrLine(2, 7);	
	DisplayConnectCenter();
	for(iCnt=0; iCnt<iCapkNum; iCnt++)
	{
		sprintf((char *)szProcInfo, "%d/%d", iCnt+1, iCapkNum);
		lcdDisplay(96, 3, DISP_ASCII, "%5.5s", szProcInfo);
		lcdFlip();

		if( stCapkInfoList[iCnt].bNeedDownLoad==FALSE )
		{	
			continue;
		}

		InitCommData();
		stTemp.iTransNo = DOWNLOAD_EMV_CAPK;
		SetCommReqField((uint8_t *)"0800", (uint8_t *)"");
		ShortToByte(glSendPack.sField62, (uint16_t)(3+5+3+1));
		memcpy(&glSendPack.sField62[2],     "\x9F\x06\x05", 3);
		memcpy(&glSendPack.sField62[2+3],   stCapkInfoList[iCnt].sRid, 5);
		memcpy(&glSendPack.sField62[2+3+5], "\x9F\x22\x01", 3);
		glSendPack.sField62[2+3+5+3] = stCapkInfoList[iCnt].ucKeyID;

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 || glRecvPack.sField62[2]!='1' )
		{
			continue;	
		}


		//Analytical CAPK
		pstCurCapk = &stNewCapkList[iCnt];
		
		ucRet = UnpackTlvData(SaveEmvCapkItem);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		sysBeef(3, 100);
	}

	ucRet = SaveNewEmvCapk(&stNewCapkList[0]);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	SaveCtrlParam();

	ucRet = FinishDownLoad(END_EMV_CAPK);

	if (ucNeedBak)
	{
		memcpy(&PosCom, &BackPosCom, COM_SIZE);
		memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));	
		stTemp.iTransNo = PosCom.stTrans.iTransNo;
		PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
		sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	}

	return ucRet;
}

uint8_t DownLoadEmvParam(uint8_t ucNeedBak)
{
	int			iCnt;
	uint8_t			ucRet, szProcInfo[16+1];
	struct terminal_aid_info stAppList[MAX_APP_NUM];
	TermAidList		stTermAidList[MAX_APP_NUM];

	if (ucNeedBak)
	{
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));
		memcpy(&BackPosCom, &PosCom, COM_SIZE);
	}

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_CFONT|DISP_INVLINE, NULL, " LOAD EMV PARA  ");
	lcdFlip();

	if( stPosParam.ucEmvSupport!=PARAM_OPEN )
	{
		stTransCtrl.bEmvAppLoaded = TRUE;
		SaveCtrlParam();
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_MEDIACY|DISP_CFONT,NULL, "NO IC PARA");
		lcdFlip();
		kbGetKeyMs(2000);
		return NO_DISP;
	}

	iAidNum    = 0;
	pstAppList = &stAppList[0];
	memset(&stAppList[0], 0, sizeof(stAppList));
	memset(&stTermAidList[0], 0, sizeof(stTermAidList));

	ucRet = QueryEmvParam();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	stTransCtrl.bEmvAppLoaded = FALSE;
	SaveCtrlParam();

	emv_delete_all_terminal_aids();
	fileRemove("emv.aid");		
	fileRemove(TERM_AIDLIST);	

	lcdClrLine(2, 7);	
	DisplayConnectCenter();
	for(iCnt=0; iCnt<iAidNum; iCnt++)
	{
		sprintf((char *)szProcInfo, "%d/%d", iCnt+1, iAidNum);
		lcdDisplay(96, 3, DISP_ASCII, "%5.5s", szProcInfo);
		lcdFlip();

		InitCommData();
		stTemp.iTransNo = DOWNLOAD_EMV_PARAM;
		SetCommReqField((uint8_t *)"0800", (uint8_t *)"");
		ShortToByte(glSendPack.sField62, (uint16_t)(stAppList[iCnt].aid_len+2+1));
		memcpy(&glSendPack.sField62[2], "\x9F\x06", 2);
		glSendPack.sField62[2+2] = stAppList[iCnt].aid_len;
		memcpy(&glSendPack.sField62[2+2+1], stAppList[iCnt].aid, stAppList[iCnt].aid_len);

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 || glRecvPack.sField62[2]!='1' )
		{
			return E_TRANS_FAIL;
		}

		// Analytical EMV PARAM
		pstCurApp     = &stAppList[iCnt];
		pstCurTermAid = &stTermAidList[iCnt];
		ucRet = UnpackTlvData(SaveAppParamItem);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		memcpy(pstCurTermAid->sAID, pstCurApp->aid, pstCurApp->aid_len);
		pstCurTermAid->ucAidLen = pstCurApp->aid_len;
		sysBeef(3, 100);
	}

	ucRet = SaveEmvAppParam(&stTermAidList[0]);
	if( ucRet!=OK )
	{
		return ucRet;
	}

	ucRet =  FinishDownLoad(END_EMV_PARAM);

	if (ucNeedBak)
	{
		memcpy(&PosCom, &BackPosCom, COM_SIZE);
		memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));	
		stTemp.iTransNo = PosCom.stTrans.iTransNo;
		PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
		sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	}

	return ucRet;
}

uint8_t QueryEmvParam(void)
{
	uint8_t	ucRet;

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "QUERYING,WAIT");
	lcdFlip();
	while(1)
	{
		InitCommData();
		stTemp.iTransNo = QUERY_EMV_PARAM;
		SetCommReqField((uint8_t *)"0820", (uint8_t *)"");
		ShortToByte(glSendPack.sField62, 3);		
		sprintf((char *)&glSendPack.sField62[2], "1%02d", iAidNum);
		
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
		{
			return E_TRANS_FAIL;
		}

		ucRet = UnpackTlvData(SaveOneAid);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( glRecvPack.sField62[2]!='2' )
		{
			break;
		}
	}
	if( iAidNum==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "NO PARAM");
		lcdFlip();
		kbGetKeyMs(2000);
		SaveCtrlParam();
	}

	return OK;
}

uint8_t UnpackTlvData(SaveTLVData pfSaveData)
{
	int	iRet, iTotalLen;
	uint8_t	*psTemp, *psTLVData;
	
	iTotalLen = ByteToShort(glRecvPack.sField62) - 1;
	psTLVData = &glRecvPack.sField62[2+1];	// bit 62: len(2) + control(1) + TLV Data(n)

	for(psTemp=psTLVData; psTemp<psTLVData+iTotalLen; )
	{
		iRet = GetTlvValue(&psTemp, psTLVData+iTotalLen-psTemp, pfSaveData, FALSE);
		if( iRet<0 )
		{
			return E_TRANS_FAIL;
		}
	}
	
	return OK;
}

void SaveOneAid(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	if( uiTag==0x9F06 && iAidNum<MAX_APP_NUM )
	{
		memcpy(pstAppList[iAidNum].aid, psData, MIN(16, iDataLen));
		pstAppList[iAidNum].aid_len = MIN(16, iDataLen);
		iAidNum++;
	}
}

void SaveAppParamItem(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x9F06:	// AID
		if( (uint8_t)iDataLen!=pstCurApp->aid_len ||
			memcmp(pstCurApp->aid, psData, iDataLen)!=0 )
		{	
			pstCurApp->aid_len = 0;
		}
		break;
	
	case 0xDF01:
		pstCurApp->support_partial_aid_select = ((*psData==0) ? PART_MATCH:FULL_MATCH);
		break;
	
	case 0x9F08:
	case 0x9F09:
		memcpy(pstCurApp->application_version, psData, MIN(iDataLen, 2));
		break;
	
	case 0xDF11:
		memcpy(pstCurApp->terminal_action_code_default, psData, MIN(iDataLen, 5));
		break;
	
	case 0xDF12:
		memcpy(pstCurApp->terminal_action_code_online, psData, MIN(iDataLen, 5));
		break;
		
	case 0xDF13:
		memcpy(pstCurApp->terminal_action_code_denial, psData, MIN(iDataLen, 5));
		break;
		
	case 0x9F1B:
		Char2Long(psData, (uint16_t)MIN(iDataLen, 4), &pstCurApp->terminal_floor_limit);
		break;
		
	case 0xDF15:
		Char2Long(psData, (uint16_t)MIN(iDataLen, 4), &pstCurApp->threshold_value);
		break;
		
	case 0xDF16:
		pstCurApp->maximum_target_percentage = (psData[0]>>4)*10 + (psData[0] & 0x0F);
		break;
		
	case 0xDF17:
		pstCurApp->target_percentage = (psData[0]>>4)*10 + (psData[0] & 0x0F);
		break;
		
	case 0xDF14:
		pstCurApp->default_tdol[0] = (uint8_t)iDataLen;
		memcpy(&pstCurApp->default_tdol[1], psData, MIN(iDataLen, (int)sizeof(pstCurApp->default_tdol)-1));
		break;
		
	case 0xDF18:
		pstCurTermAid->bOnlinePin = *psData;
		break;

	case 0x9F7B:
		pstCurTermAid->m_EcMaxTxnAmount = PubBcd2Long(psData, (uint16_t)MIN(iDataLen, 6),NULL);
		break;
		
	case 0xDF19:
		pstCurApp->cl_Floor_Limit = PubBcd2Long(psData, (uint16_t)MIN(iDataLen, 6),NULL);
		break;

	case 0xDF20:
		pstCurApp->cl_ReaderMaxTransAmount = PubBcd2Long(psData, (uint16_t)MIN(iDataLen, 6),NULL);
		break;

	case 0xDF21:
		pstCurApp->cl_CVM_Amount = PubBcd2Long(psData, (uint16_t)MIN(iDataLen, 6),NULL);
		break;
		
	default:
		break;
	}
}

uint8_t SaveEmvAppParam(TermAidList *pstTermAidList)
{
	int	iRet, iCnt, iAidFile, iErrNo;
	for(iCnt=0; iCnt<iAidNum; iCnt++)
	{
		iRet = emv_add_one_aid_info(&pstAppList[iCnt]);
		iErrNo = iRet?errno:0;

		if( iErrNo!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}
		emvSaveAIDList(iCnt, &pstAppList[iCnt]);
	}

	iAidFile = fileOpen(TERM_AIDLIST, O_RDWR|O_CREAT);
	if( iAidFile<0 )
	{
		return E_TRANS_FAIL;
	}
	for(iCnt=0; iCnt<iAidNum; iCnt++)
	{
		if( pstTermAidList[iCnt].ucAidLen==0 )
		{
			continue;
		}
		iRet = fileWrite(iAidFile, (uint8_t *)&pstTermAidList[iCnt], sizeof(TermAidList));
		if( iRet!=sizeof(TermAidList) )
		{
			fileClose(iAidFile);
			return E_TRANS_FAIL;
		}
	}
	fileClose(iAidFile);

	return ReadTermAidList();
}

uint8_t FinishDownLoad(int iTranNo)
{
	uint8_t	ucRet;

	lcdCls();	
	if( iTranNo==END_EMV_PARAM )
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  LOAD PARA END ");
		if( iAidNum > 0 )	
			stTransCtrl.bEmvAppLoaded = TRUE;
	}
	else if (iTranNo== END_DOWNLOADBLK)
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "LOAD BLKCARD END");
	}
	else 
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  LOAD KEY END  ");
		if( iCapkNum > 0 )	
			stTransCtrl.bEmvCapkLoaded = TRUE;
	}
	lcdFlip();
	SaveCtrlParam();
	
	InitCommData();
	stTemp.iTransNo = iTranNo;
	SetCommReqField((uint8_t *)"0800", (uint8_t *)"");
	ucRet = SendRecvPacket();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
	{
		return E_TRANS_FAIL;
	}
	
	return OK;
}

uint8_t QueryEmvCapk(void)
{
	uint8_t	ucRet;

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "QUERYING,WAIT");
	lcdFlip();
	while(1)
	{
		InitCommData();
		stTemp.iTransNo = QUERY_EMV_CAPK;
		SetCommReqField((uint8_t *)"0820", (uint8_t *)"");
		ShortToByte(glSendPack.sField62, 3);
		sprintf((char *)&glSendPack.sField62[2], "1%02d", iCapkNum);
		
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
		{
			return E_TRANS_FAIL;
		}
		
		bRidOk = bKeyIDOk = bExpiryOk = FALSE;
		ucRet = UnpackTlvData(SaveCapkInfoItem);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		if( glRecvPack.sField62[2]!='2' )
		{
			break;
		}
	}

	if( iCapkNum==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, "无公钥下载", "NO KEY");
		lcdFlip();
		kbGetKeyMs(2000);
		SaveCtrlParam();
	}
	
	return OK;
}

void SaveCapkInfoItem(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	if( iCapkNum>=MAX_KEY_NUM )
	{
		return;
	}

	switch( uiTag )
	{
	case 0x9F06:
		memcpy(pstCapkInfoList[iCapkNum].sRid, psData, MIN(iDataLen, 5));
		bRidOk = TRUE;
		break;
	
	case 0x9F22:
		pstCapkInfoList[iCapkNum].ucKeyID = *psData;
		bKeyIDOk = TRUE;
		break;
	
	case 0xDF05:
		if( iDataLen==8 )
		{
			AscToBcd(pstCapkInfoList[iCapkNum].sExpDate, psData, 8);
		}
		else
		{
			memcpy(pstCapkInfoList[iCapkNum].sExpDate, psData, MIN(iDataLen, 4));
		}
		bExpiryOk = TRUE;
		break;
	}

	if( bRidOk && bKeyIDOk && bExpiryOk )
	{
		bRidOk = bKeyIDOk = bExpiryOk = FALSE;
		pstCapkInfoList[iCapkNum].bNeedDownLoad = TRUE;
		iCapkNum++;
	}
}

int SearchCapkInfo(uint8_t *psRid, uint8_t ucKeyID)
{
	int		iCnt;

	for(iCnt=0; iCnt<iCapkNum; iCnt++)
	{
		if( ucKeyID==pstCapkInfoList[iCnt].ucKeyID   &&
			memcmp(psRid, pstCapkInfoList[iCnt].sRid, 5)==0 )
		{
			return iCnt;
		}
	}

	return -1;
}

void SaveEmvCapkItem(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	if( iDataLen<=0 )
	{
		return;
	}

	switch( uiTag )
	{
	case 0x9F06:
		memcpy(pstCurCapk->RID, psData, MIN(iDataLen, 5));
		break;
	
	case 0x9F22:
		pstCurCapk->Index   = *psData;
		break;
	
	case 0xDF05:
		if( iDataLen==8 )
		{
			AscToBcd(pstCurCapk->expiration_date, psData+2, 6);
		}
		else
		{
			memcpy(pstCurCapk->expiration_date, psData+1, 3);
		}
		break;
			
	case 0xDF02:
		pstCurCapk->len_of_modulus = (uint8_t)MIN(iDataLen, 248);
		memcpy(pstCurCapk->modulus, psData, pstCurCapk->len_of_modulus);
		break;
		
	case 0xDF04:
		pstCurCapk->len_of_exponent = (uint8_t)MIN(iDataLen, 3);
		memcpy(pstCurCapk->exponent, psData, pstCurCapk->len_of_exponent);
		break;
		
	case 0xDF03:
		memcpy(pstCurCapk->checksum, psData, MIN(iDataLen, 20));
		break;
	}
}

uint8_t SaveNewEmvCapk(struct issuer_ca_public_key *pstNewCapkList)
{
	int		iRet, iCnt;
    uint8_t       szDate[9], szTime[7];

    uint8_t       szBuff[20];
	int    ifileindex;

    memset(szDate, 0, sizeof(szDate));
    memset(szTime, 0, sizeof(szTime));
    GetPosTime(szDate, szTime); 
	emv_delete_all_ca_public_keys();
	fileRemove("emv.capk");

    
	for(iCnt=0; iCnt<iCapkNum; iCnt++)
	{
		if( pstCapkInfoList[iCnt].bNeedDownLoad==FALSE )
		{
			continue;
		}
        memset(szBuff, 0, sizeof(szBuff));
        BcdToAsc0(szBuff, pstNewCapkList[iCnt].expiration_date, 6);
        
        
		if( memcmp(&szDate[2], szBuff, 6) >0 )
		{
			continue;
		}		
        iRet = emv_add_one_ca_public_key(pstNewCapkList+iCnt);
		if( iRet!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}
		if (fileExist("emv.capk")>=0)
		{
			ifileindex = fileSize("emv.capk");
		}
		else
		{
			ifileindex = 0;
		}
		ifileindex = ifileindex/(sizeof(struct issuer_ca_public_key));
		ifileindex = MIN(ifileindex,iCnt);
		emvSaveCAPKList(ifileindex, pstNewCapkList+iCnt);
	}

	return OK;
}

uint8_t PosDownLoadParam(void)
{
	uint8_t	ucRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   LOAD PARA    ");
	lcdFlip();

	stTransCtrl.ucLoadTask = 0;
	SaveCtrlParam();

	InitCommData();
	stTemp.iTransNo = POS_DOWNLOAD_PARAM;
	SetCommReqField((uint8_t *)"0800", (uint8_t *)"");
	ucRet = SendRecvPacket();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY,NULL, "LOAD PARAM FAIL");
		lcdFlip();
		sysDelayMs(1000);
		return NO_DISP;
	}
	UpdateParameters(&glRecvPack.sField62[2], ByteToShort(glRecvPack.sField62));

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SUCCESS");
	lcdFlip();
	kbGetKeyMs(3000);

	return OK;
}

uint8_t PosUpLoadParam(void)
{
	uint8_t	ucRet;
	int		iLen;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  STATUS UPLOAD ");
	lcdFlip();

	InitCommData();
	stTemp.iTransNo = POS_UPLOAD_PARAM;
	SetCommReqField((uint8_t *)"0820", (uint8_t *)"");
	memset(glSendPack.sField62,0,sizeof(glSendPack.sField62));
	iLen = MakeTermStatusData(&glSendPack.sField62[2]);
	ShortToByte(glSendPack.sField62, (uint16_t)iLen);

	ucRet = SendRecvPacket();
	if( ucRet!=OK )
	{
		return ucRet;
	}
	if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
	{
		return E_TRANS_FAIL;
	}
	init_dial_statics();

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SUCCESS");
	lcdFlip();
	kbGetKeyMs(3000);

	return OK;
}

int MakeTermStatusData(uint8_t *pStatus) 
{
	strcpy((char *)pStatus, "011021031041051");	
	strcpy((char *)pStatus+15, "1160");	
	sprintf((char *)pStatus+19, "12%02d", (int)stPosParam.ucCommWaitTime);
	sprintf((char *)pStatus+23, "13%d", (int)stPosParam.ucDialRetryTimes);
	strcpy((char *)pStatus+26, "14              ");
	memcpy(pStatus+28, stPosParam.szPhoneNum1, strlen((char *)stPosParam.szPhoneNum1));					
	strcpy((char *)pStatus+42, "15              ");
	memcpy(pStatus+44, stPosParam.szPhoneNum2, strlen((char *)stPosParam.szPhoneNum2));					
	strcpy((char *)pStatus+58, "16              ");
	memcpy(pStatus+60, stPosParam.szPhoneNum3, strlen((char *)stPosParam.szPhoneNum3));
	strcpy((char *)pStatus+74, "17              ");
	memcpy(pStatus+76, stPosParam.szMPhone, strlen((char *)stPosParam.szMPhone));					
	
	strcpy((char *)pStatus+90, "18");					
	pStatus[92] = stPosParam.ucTipOpen;
	sprintf((char *)pStatus+93,"19%02d", (int)stPosParam.ucTipper);
	
	strcpy((char *)pStatus+97, "20");
	pStatus[99] = stPosParam.ucManualInput;
	
	strcpy((char *)pStatus+100, "21");
	pStatus[102] = stPosParam.ucAutoLogoff;
	
	sprintf((char *)pStatus+103, "23%1d", (int)stPosParam.ucResendTimes);	
	sprintf((char *)pStatus+106, "25%1d", (int)stPosParam.ucKeyIndex);
	sprintf((char *)pStatus+109,"27%2d", (int)stPosParam.ucMaxOfflineTxn);
	
	if( calculate_dial_statics()!=OK ) 
		memcpy(pStatus + 113 ,"51007200999980",14);
	else 
		sprintf((char*)pStatus+113, "51%04d%05d%03d", dial_stat.hours, dial_stat.dials, dial_stat.suc_rate);
	pStatus[127] = 0;
	
	return 127;
}

uint8_t SendICCOffLineTrans(uint8_t flag, int maxSendTimes)
{
	uint8_t		ucRet;
	int		iRecNo, iRecIndex;
	NEWPOS_LOG_STRC	stLog;
	uint32_t  uiStatus;
	int      iTotalNum;
	char     szBuffer[40];
	
	iTotalNum = GetOfflineTxnNums(TS_ICC_OFFLINE);
	for(iRecNo=0; iRecNo<iTotalNum; )
	{
		if (flag)
		{
			if( kbGetKeyMs(150) == KEY_CANCEL )
			{
				return NO_DISP;
			}
		}

		memset(&stLog, 0, sizeof(stLog));
		iRecIndex = GetMinSendTimesTxn(TS_ICC_OFFLINE);
		if (iRecIndex == -1)
		{
			return 0;
		}
		
		ucRet = LoadTranLog(iRecIndex, &stLog);
		if( ucRet )
		{
			return ucRet;
		}

		if( (stLog.iTransNo!=ICC_OFFSALE && stLog.iTransNo != EC_NORMAL_SALE && stLog.iTransNo != EC_QUICK_SALE) 
			|| stLog.ucSendFlag ||	stLog.ucVoidId )
		{
			uiStatus = stTransCtrl.sTxnRecIndexList[iRecIndex];
			uiStatus &= ~TS_ICC_OFFLINE;
			UpdataTxnCtrlStatus(iRecIndex,uiStatus);
			continue;
		}

		iRecNo++;
		InitCommData();
		stTemp.iTransNo = stLog.iTransNo;
		PosCom.ucSwipedFlag = CARD_INSERTED;
		if (stLog.iTransNo == EC_QUICK_SALE)
		{
			PosCom.ucSwipedFlag = CARD_PASSIVE;
			PosCom.stTrans.ucSwipeFlag = CARD_PASSIVE;
		}
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		SetCommReqField((uint8_t *)"0200", (uint8_t *)"000000");

		stLog.iIccDataLen += 4;
		ShortToByte(glSendPack.sICCData, (uint16_t)stLog.iIccDataLen);
		memcpy(&glSendPack.sICCData[2], stLog.sIccData, stLog.iIccDataLen-4);
		memcpy(&glSendPack.sICCData[2+stLog.iIccDataLen-4], "\x8A\x02\x59\x31", 4);
		stLog.iIccDataLen -= 4;
		strcpy((char *)glSendPack.szField63, "CUP");
		glSendPack.szField60[13] = 0;

		lcdClrLine(2,7);
		DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "Upload Txn...");
		sprintf(szBuffer,"Upload %d ",iRecNo);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, szBuffer, szBuffer);
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "PLS WAIT....");
		lcdFlip();
		sysDelayMs(500);
		
		PosCom.bOnlineTxn = FALSE; 
		ucRet = SendRecvPacket();
		if( ucRet!=OK || strlen((char*)PosCom.szRespCode)==0)
		{
			if(ucRet == E_ERR_CONNECT ||ucRet ==  E_MAKE_PACKET || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			if (PosCom.bOnlineTxn)
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
				uiStatus++; 
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
			}
			stLog.ucSendFail = TS_NOT_RECV;
			ucRet = UpdateTranLog(iRecIndex, &stLog);
			if( ucRet!=OK )
			{
				return ucRet;
			}
		}
		else
		{
			if( memcmp(PosCom.szRespCode, "00", 2)==0 || memcmp(PosCom.szRespCode, "94", 2)==0 )
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
				uiStatus &= ~TS_ICC_OFFLINE;
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);
				sprintf((char *)stLog.szSysReferNo, "%.12s", PosCom.stTrans.szSysReferNo);
				strcpy((char *)stLog.szIssuerBankId,(char *)PosCom.stTrans.szIssuerBankId);
				strcpy((char *)stLog.szRecvBankId,(char *)PosCom.stTrans.szRecvBankId);
				strcpy((char *)stLog.szCenterId,(char *)PosCom.stTrans.szCenterId);
				strcpy((char *)stLog.szSettDate,(char *)PosCom.stTrans.szSettDate);
				strcpy((char *)stLog.szIssuerResp,(char *)PosCom.stTrans.szIssuerResp);
				strcpy((char *)stLog.szCenterResp,(char *)PosCom.stTrans.szCenterResp);
				strcpy((char *)stLog.szRecvBankResp,(char *)PosCom.stTrans.szRecvBankResp);
				stLog.ucSendFlag = TRUE;
				stLog.ucSendFail = FALSE;
				ucRet = UpdateTranLog(iRecIndex, &stLog);
				if( ucRet!=OK )
				{
					return ucRet;
				}
			}
			else
			{
				uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
				uiStatus |= 0x0F; 
				UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);

				stLog.ucSendFail = TS_NOT_CODE;
				ucRet = UpdateTranLog(iRecIndex, &stLog);
				if( ucRet!=OK )
				{
					return ucRet;
				}
			}
		}
		
	}	//for

	return OK;
}

uint8_t BatchICCOffLineTrans(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iRecNo, uiStatus;
	NEWPOS_LOG_STRC	stLog;

	if( ucSendFlag==EMVLOG )
	{
		return OK;
	}

	for(iRecNo=0; iRecNo<stTransCtrl.iTransNum; iRecNo++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iRecNo, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( !(stLog.iTransNo ==ICC_OFFSALE || stLog.iTransNo == EC_QUICK_SALE || stLog.iTransNo == EC_NORMAL_SALE ))
		{
			continue;
		}

		if( (ucSendFlag==RMBLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)!=0) )
		{
			continue;	
		}
		if( (ucSendFlag==FRNLOG) && (memcmp(stLog.szCardUnit, "CUP", 3)==0) )
		{
			continue;	
		}

		if( stLog.ucBatchSendFlag==TRUE )
		{
			(*piTotalNum)++;
			continue;
		}

		uiStatus = stTransCtrl.sTxnRecIndexList[iRecNo] & 0x0000000F;
		if (uiStatus > stPosParam.ucResendTimes)
		{
			(*piTotalNum)++;
			continue;
		}

		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		stTemp.iTransNo = ICC_BATCHUP;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		if (stLog.iTransNo == EC_QUICK_SALE)
		{
			PosCom.ucSwipedFlag = CARD_PASSIVE;
			PosCom.stTrans.ucSwipeFlag = CARD_PASSIVE;
		}
		SetCommReqField((uint8_t *)"0320", (uint8_t *)"000000");
		sprintf((char *)glSendPack.szPan, "%.19s", PosCom.stTrans.szCardNo);
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
		sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE, PosCom.stTrans.szExpDate);
		sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
		sprintf((char *)glSendPack.szSvrCode, "00");
		sprintf((char *)glSendPack.szCurrencyCode, "156");

		if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
		{
			sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
		}
		stLog.iIccDataLen += 4;
		ShortToByte(glSendPack.sICCData, (uint16_t)stLog.iIccDataLen);
		memcpy(&glSendPack.sICCData[2], stLog.sIccData, stLog.iIccDataLen-4);
		memcpy(&glSendPack.sICCData[2+stLog.iIccDataLen-4], "\x8A\x02\x59\x31", 4);
		
		stLog.iIccDataLen -= 4;

		strcpy((char *)glSendPack.szField63, "CUP");

		if (stLog.iTransNo == EC_QUICK_SALE || PosCom.stTrans.ucSwipeFlag & CARD_PASSIVE)
		{
			sprintf((char *)glSendPack.szField60, "36%06d00060", PosCom.stTrans.lBatchNumber);
		}
		else
		{
			sprintf((char *)glSendPack.szField60, "36%06d00060", PosCom.stTrans.lBatchNumber);
		}

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;
				if (PosCom.bOnlineTxn)
				{
					uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
					uiStatus++;
					UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
				}
				continue;
			}			
		}		
		
		stLog.ucBatchSendFlag = TRUE;
		ucRet = UpdateTranLog(iRecNo, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;

	}	//for

	return ucRet;
}

uint8_t BatchSendICCOnLineTrans(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt, uiStatus;
	NEWPOS_LOG_STRC	stLog;

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( (stLog.ucSwipeFlag!=CARD_INSERTED && stLog.ucSwipeFlag!=CARD_PASSIVE) ||
			(stLog.iTransNo!=POS_SALE && stLog.iTransNo!=POS_PREAUTH && stLog.iTransNo != EC_TOPUP_CASH 
			&& stLog.iTransNo != QPBOC_ONLINE_SALE && stLog.iTransNo!=POS_INSTALLMENT) )
		{
			continue;
		}

		if(stLog.iTransNo==QPBOC_ONLINE_SALE)
		{
			continue;	
		}

		if((stLog.sTVR[4] & 0x40) )
		{
			continue;
		}

		if( stLog.ucBatchSendFlag==TRUE )
		{
			(*piTotalNum)++;
			continue;
		}

		uiStatus = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
		if (uiStatus > stPosParam.ucResendTimes)
		{
			(*piTotalNum)++;
			continue;
		}

		InitCommData();
		memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
		stTemp.iTransNo = ICC_BATCHUP;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
		sprintf((char *)glSendPack.szPan, "%.19s", PosCom.stTrans.szCardNo);
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
		sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
		if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
		{
			sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
		}
		ShortToByte(glSendPack.sICCData, PosCom.stTrans.iIccDataLen);
		memcpy(&glSendPack.sICCData[2], PosCom.stTrans.sIccData, PosCom.stTrans.iIccDataLen);

		memcpy(&glSendPack.sICCData[5], PosCom.stTrans.sAppCrypto, 8); // 9F26
		memcpy(&glSendPack.sICCData[5+8+3], "\x40", 1); // 9F27

		if( ucSendFlag!=EMVLOG )
			memcpy((char *)&glSendPack.szField60[8], "205", 3);
		memcpy(glSendPack.sField62, "\x00\x15\x36\x31", 4);
		sprintf((char *)&glSendPack.sField62[4], "0000%s156", glSendPack.szTranAmt);
		if( memcmp(PosCom.stTrans.szCardUnit, "CUP", 3)!=0 )
		{
			memcpy(&glSendPack.sField62[4], "01", 2);
		}

		glSendPack.szField60[13] = 0;

		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;
				if (PosCom.bOnlineTxn)
				{
					uiStatus = stTransCtrl.sTxnRecIndexList[stLog.uiTxnRecIndex];
					uiStatus++;
					UpdataTxnCtrlStatus(stLog.uiTxnRecIndex,uiStatus);					
				}
				continue;
			}			
		}		

		stLog.ucBatchSendFlag = TRUE;
		ucRet = UpdateTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}

	return ucRet;
}

void UpdateEntryModeByPIN(void)
{
	if( PosCom.ucPinEntry )
	{
		PosCom.stTrans.szEntryMode[2] = HAVE_INPUT_PIN;
		sprintf((char *)glSendPack.szCaptureCode, "12");
		ShortToByte(glSendPack.sPINData, LEN_PIN_DATA);
		memcpy(&glSendPack.sPINData[2], PosCom.sPIN, LEN_PIN_DATA);
	
	}
	else
	{
		//add by yxj
		PosCom.stTrans.szEntryMode[2] = PARAM_CLOSE;
		//PosCom.stTrans.szEntryMode[2] = NOT_INPUT_PIN;

	}

	if ( PosCom.ucPinEntry || 
		(stPosParam.ucTrackEncrypt == PARAM_OPEN && (strlen((char *)glSendPack.szTrack2)!=0 || strlen((char *)glSendPack.szTrack3)!=0)))
	{
		strcpy((char *)glSendPack.szSecurityInfo, "0000000000000000");
		if (PosCom.ucPinEntry)
		{
			glSendPack.szSecurityInfo[0] = '2';
		}

		// set bit 53
		if( stPosParam.ucKeyMode!=KEY_MODE_DES )
		{
			glSendPack.szSecurityInfo[1] = '6';
		}
	}

	sprintf((char *)glSendPack.szEntryMode, "%s0", PosCom.stTrans.szEntryMode);
}


void  EncrypteFieldTrac23(uint8_t *pszTrack, uint8_t *pszEncrypData)
{
	char  szTemp[300],szBuffer[300],szData[10];
	int   iRet, iDatalen, iCnt;
	
	memset(szBuffer,0,sizeof(szBuffer));
	memset(szTemp,0,sizeof(szTemp));
	memset(szData,0,sizeof(szData));
	if (pszTrack == NULL || strlen((char *)pszTrack)==0)
	{
		return;
	}

	iDatalen = strlen((char *)pszTrack);
	strcpy((char *)szTemp,(char *)pszTrack);
	if (iDatalen < 17)
	{
		PubAddHeadChars(szTemp,'F',17);
		iDatalen = 17;
	}

	if (iDatalen%2==0)
		memcpy(szBuffer,&szTemp[iDatalen-18],16);
	else
		memcpy(szBuffer,&szTemp[iDatalen-17],16);

	for (iCnt=0; iCnt<8; iCnt++)
	{
		szData[iCnt] = ((szBuffer[2*iCnt]&0x0F)<<4) | (szBuffer[2*iCnt+1]&0x0F);
	}
	if( stPosParam.ucSupPinpad==PARAM_OPEN)
	{
      iRet = EppGetMac(KEY_TYPE_MAC,DOUBLE_TDK_KEY_ID, (uint8_t *)szData, 8, MAC_MODE_1, (uint8_t *)szBuffer);
	}
	else
	{
	  iRet = PedGetMac(DOUBLE_TDK_KEY_ID, (uint8_t *)szData, 8, MAC_MODE_1, (uint8_t *)szBuffer);

	}
	if( iRet!=PED_RET_OK )
			return ;

	if (iDatalen%2==0)
		PubBcd2Asc(szBuffer,8,&szTemp[iDatalen-18]);
	else
		PubBcd2Asc(szBuffer,8,&szTemp[iDatalen-17]);

	if (pszEncrypData != NULL)
		strcpy((char *)pszEncrypData,szTemp);
}


void  SetCommField60(void)
{
	uint8_t	ucIndex;

	return;       //NETPAY
	if( stTemp.iTransNo==POS_LOGON )
	{
		if (stPosParam.ucKeyMode==KEY_MODE_3DES)
		{
			if(stPosParam.ucTrackEncrypt == PARAM_OPEN)
				sprintf((char *)glSendPack.szField60, "00%06d004", PosCom.stTrans.lBatchNumber);
			else
				sprintf((char *)glSendPack.szField60, "00%06d003", PosCom.stTrans.lBatchNumber);
		}
		else
			sprintf((char *)glSendPack.szField60, "00%06d001", PosCom.stTrans.lBatchNumber);
		return;
	}
	else
	{
		if(stTemp.iTransNo == POS_REFUND_BAT)
			ucIndex = GetTxnTableIndex(PosCom.stTrans.iOldTransNo);
		else
			ucIndex = GetTxnTableIndex(stTemp.iTransNo);
		
		sprintf((char *)glSendPack.szField60, "%s%06d%s600", TrTable[ucIndex].szMessTypeCode,
					PosCom.stTrans.lBatchNumber, TrTable[ucIndex].szNetManInfoCode);

		if( PosCom.ucSwipedFlag==FALLBACK_SWIPED )
		{
			glSendPack.szField60[12] = '2';
		}

		if (PosCom.stTrans.ucSwipeFlag & CARD_PASSIVE)
		{
			glSendPack.szField60[11] = '6';
		}
	}
}

uint8_t TransDownloadBlkCard(void)
{
	uint8_t		ucRet,  ucFirstTime;
	int		iCurTotalNum, iTotalNum, uiCardlen;
	ST_BLACKLIST_TOTAL stBlkCard;
	uint8_t  *psTemp;
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_CFONT|DISP_INVLINE, NULL, "LOAD BLKCARD");
	lcdFlip();

	PreDial();
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_CFONT|DISP_INVLINE, NULL, "LOAD BLKCARD");
	lcdFlip();
	
	memset((char*)&stBlkCard,0,sizeof(stBlkCard));
	stBlkCard.ulBlackListNum = 0;
	ucFirstTime = TRUE;
	iCurTotalNum = iTotalNum = 0;
	while(1)
	{
		InitCommData();
		stTemp.iTransNo = POS_DOWNLOADBLK;
		PosCom.stTrans.iTransNo = POS_DOWNLOADBLK;
		GetPosTime(stTemp.szDate, stTemp.szTime);		
		strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
		strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
		
		SetCommReqField((uint8_t *)"0800", (uint8_t *)"");
		memset(glSendPack.szSTAN,0,sizeof(glSendPack.szSTAN));
		memset(glSendPack.szEntryMode,0,sizeof(glSendPack.szEntryMode));
		memset(glSendPack.szSvrCode,0,sizeof(glSendPack.szSvrCode));
		memset(glSendPack.szCurrencyCode,0,sizeof(glSendPack.szCurrencyCode));
		sprintf((char *)glSendPack.szField60, "00%06d390",stPosParam.lNowBatchNum);
		ShortToByte(glSendPack.sField62, 3);
		if (iTotalNum)
			sprintf((char*)glSendPack.sField62+2,"%03d",iTotalNum+1);	
		else
			sprintf((char*)glSendPack.sField62+2,"%03d",iTotalNum);		
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if( memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
		{
 			dispRetCode(1);
			return NO_DISP;
		}

		psTemp = &glRecvPack.sField62[2];
		psTemp++;
		iCurTotalNum = PubAsc2Long((char*)psTemp,3,NULL);
		psTemp += 3;

		while(iTotalNum<iCurTotalNum)
		{
			if (psTemp == NULL)
			{
				lcdClrLine(2,7);
				lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"DATA ERROR");
				lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"PLS RETRY");
				lcdFlip();
				PubBeepErr();
				PubWaitKey(5);
				return NO_DISP;
			}

			if (iTotalNum >= MAX_BLACKLIST_NUM)
			{
				lcdClrLine(2,7);
				lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"OVER LIMITED");
				lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"PLS RETRY");
				lcdFlip();
				PubBeepErr();
				PubWaitKey(5);
				return NO_DISP;
			}

			uiCardlen = PubAsc2Long((char*)psTemp,2,NULL);
			psTemp += 2;
			stBlkCard.szBlackList[iTotalNum][0] = uiCardlen;
			PubAsc2Bcd((char*)psTemp,uiCardlen,&stBlkCard.szBlackList[iTotalNum][1]);
			psTemp+=14;
			iTotalNum++;
		}

		stBlkCard.ulBlackListNum = iTotalNum;
		if (glRecvPack.sField62[2] != '2')
		{
			break;
		}
	}

	if (stBlkCard.ulBlackListNum != 0)
	{
		SaveBlkCard(&stBlkCard);
	}

	ucRet = FinishDownLoad(END_DOWNLOADBLK);
	if (ucRet != 0)
	{
		return ucRet;
	}

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_CFONT|DISP_INVLINE, NULL, "LOAD BLKCARD");
	lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"SUCCESS");
	lcdFlip();
	PubBeepErr();
	PubWaitKey(5);
	return OK;	
}


uint8_t TransBatchUpLoadFailureTxn(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t ucRet;

	// AAC
	ucRet = TransBatchUpLoadIccAACTxn(piTotalNum, ucSendFlag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	// other fail
	ucRet = TransBatchUpLoadIccFailureTxn(piTotalNum, ucSendFlag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	ucRet = TransBatchUploadQpbocFailureTxnLog(piTotalNum, ucSendFlag);
	if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET || ucRet==E_MEM_ERR || ucRet==E_NO_TRANS)
	{
		return ucRet;
	}

	return OK;
}

//The issuer authentication failure
uint8_t TransBatchUpLoadIccFailureTxn(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt,iTotalRecNum,iTransTypeBak,iFilesize=0;

	iFilesize = fileSize(ICC_FAILUR_TXN_FILE);
	if (iFilesize <=0 )
	{
		return 0;
	}	

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "UPLOAD FAIL TXN");
	lcdFlip();
	iTotalRecNum = iFilesize/COM_SIZE;
	for(iCnt=0; iCnt<iTotalRecNum; iCnt++)
	{
		InitCommData();
		memset(&PosCom, 0, sizeof(PosCom));
		ucRet = ReadIccFailureFile(iCnt);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if (!(PosCom.stTrans.ucSwipeFlag & CARD_INSERTED))
		{
			continue;
		}

		if(!(PosCom.stTrans.sTVR[4] & 0x40) )
		{
			continue;
		}

		if( PosCom.stTrans.ucBatchSendFlag==TRUE )
		{
			(*piTotalNum)++;
			continue;
		}

		iTransTypeBak = PosCom.stTrans.iTransNo;
		stTemp.iTransNo = ICC_FAIL_BATCHUP;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
		sprintf((char *)glSendPack.szPan, "%.19s", PosCom.stTrans.szCardNo);
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
		sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
		if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
		{
			sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
		}
		else
		{
			strcpy((char *)glSendPack.szPanSeqNo, "000");
		}
		ShortToByte(glSendPack.sICCData, PosCom.stTrans.iIccDataLen);
		memcpy(&glSendPack.sICCData[2], PosCom.stTrans.sIccData, PosCom.stTrans.iIccDataLen);

		memcpy(&glSendPack.sICCData[5], PosCom.stTrans.sAppCrypto, 8); // 9F26
		memcpy(&glSendPack.sICCData[5+8+3], "\x40", 1); // 9F27

		if( ucSendFlag!=EMVLOG )
			memcpy((char *)&glSendPack.szField60[8], "206", 3);
		memcpy(glSendPack.sField62, "\x00\x17\x37\x31", 4);
		sprintf((char *)&glSendPack.sField62[4], "0005%s15622", glSendPack.szTranAmt);
		if( memcmp(PosCom.stTrans.szCardUnit, "CUP", 3)!=0 )
		{
			memcpy(&glSendPack.sField62[4], "01", 2);
		}
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;//no response
				continue;
			}			
		}

		PosCom.stTrans.ucBatchSendFlag = TRUE;
		PosCom.stTrans.iTransNo = iTransTypeBak;
		ucRet = UpDateIccFailureFile(iCnt);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}

	return ucRet;
}


uint8_t TransBatchUpLoadIccAACTxn(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt, iTotalRecNum, iTransTypeBak;
	int     iFilesize = 0;
	
	iFilesize = fileSize(ICC_FAILUR_TXN_FILE);
	if (iFilesize <=0 )
	{
		return 0;
	}	
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "UPLOAD FAIL TXN");
	lcdFlip();
	iTotalRecNum = iFilesize/COM_SIZE;
	for(iCnt=0; iCnt<iTotalRecNum; iCnt++)
	{
		InitCommData();
		memset(&PosCom, 0, sizeof(PosCom));
		ucRet = ReadIccFailureFile(iCnt);
		if( ucRet!=OK )
		{
			return ucRet;
		}

		if (!(PosCom.stTrans.ucSwipeFlag & CARD_INSERTED))
		{
			continue;
		}

		if((PosCom.stTrans.sTVR[4] & 0x40) )
		{
			continue;
		}
		
		if( PosCom.stTrans.ucBatchSendFlag==TRUE )
		{	//
			(*piTotalNum)++;
			continue;
		}
		
		iTransTypeBak = PosCom.stTrans.iTransNo;
		stTemp.iTransNo = ICC_FAIL_BATCHUP;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
		sprintf((char *)glSendPack.szEntryMode, "%3.3s0",  PosCom.stTrans.szEntryMode);
		if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
		{
			sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
		}
		else
		{
			strcpy((char *)glSendPack.szPanSeqNo, "000");			
		}
				
		ShortToByte(glSendPack.sICCData, PosCom.stTrans.iIccDataLen);
		memcpy(&glSendPack.sICCData[2], PosCom.stTrans.sIccData, PosCom.stTrans.iIccDataLen);		
		memcpy(&glSendPack.sICCData[5], PosCom.stTrans.sAppCrypto, 8); // 9F26
		
		if( ucSendFlag!=EMVLOG )
			memcpy((char *)&glSendPack.szField60[8], "206", 3);
		memcpy(glSendPack.sField62, "\x00\x17\x37\x31", 4);
		sprintf((char *)&glSendPack.sField62[4], "0011%s15610", glSendPack.szTranAmt);
		glSendPack.szField60[13] = 0; 
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;
				continue;
			}			
		}
		
		PosCom.stTrans.ucBatchSendFlag = TRUE;
		PosCom.stTrans.iTransNo = iTransTypeBak;
		ucRet = UpDateIccFailureFile(iCnt);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}
	
	return ucRet;
}

int  TransBatchUploadQpbocFailureTxnLog(int *piTotalNum, uint8_t ucSendFlag)
{
	uint8_t		ucRet=0;
	int		iCnt, iTotalRecNum, iTransTypeBak;
	int     iFilesize = 0;
	
	iFilesize = fileSize(ICC_FAILUR_TXN_FILE);
	if (iFilesize <=0 )
	{
		return 0;
	}	
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "UPLOAD FAIL TXN");
	lcdFlip();
	iTotalRecNum = iFilesize/COM_SIZE;
	for(iCnt=0; iCnt<iTotalRecNum; iCnt++)
	{
		InitCommData();
		memset(&PosCom, 0, sizeof(PosCom));
		ucRet = ReadIccFailureFile(iCnt);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		if ((PosCom.stTrans.ucSwipeFlag & CARD_INSERTED))
		{
			continue;
		}
		
		if( PosCom.stTrans.ucBatchSendFlag==TRUE )
		{
			(*piTotalNum)++;
			continue;
		}
		
		iTransTypeBak = PosCom.stTrans.iTransNo;
		stTemp.iTransNo = ICC_FAIL_BATCHUP;
		PosCom.stTrans.iTransNo = stTemp.iTransNo;
		SetCommReqField((uint8_t *)"0320", (uint8_t *)"");
		BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
		sprintf((char *)glSendPack.szEntryMode, "%3.3s0",  PosCom.stTrans.szEntryMode);
		if( PosCom.stTrans.bPanSeqNoOk )	// ADVT case 43 
		{
			sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, PosCom.stTrans.ucPanSeqNo);
		}
		else
		{
			strcpy((char *)glSendPack.szPanSeqNo, "000");			
		}
		
		ShortToByte(glSendPack.sICCData, PosCom.stTrans.iIccDataLen);
		memcpy(&glSendPack.sICCData[2], PosCom.stTrans.sIccData, PosCom.stTrans.iIccDataLen);		
		memcpy(&glSendPack.sICCData[5], PosCom.stTrans.sAppCrypto, 8); // 9F26
		memcpy(&glSendPack.sICCData[5+8+3], "\x40", 1); //9F27
		
		if( ucSendFlag!=EMVLOG )
			memcpy((char *)&glSendPack.szField60[8], "206", 3);
		memcpy(glSendPack.sField62, "\x00\x17\x37\x31", 4);
		sprintf((char *)&glSendPack.sField62[4], "0011%s15610", glSendPack.szTranAmt);
		glSendPack.szField60[13] = 0; 
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return ucRet;
			}
			else 
			{
				bRecvNOK = 1;
				continue;
			}			
		}
		
		PosCom.stTrans.ucBatchSendFlag = TRUE;
		PosCom.stTrans.iTransNo = iTransTypeBak;
		ucRet = UpDateIccFailureFile(iCnt);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		(*piTotalNum)++;
	}
	
	return ucRet;
}



uint8_t AdjustConfirmTipAmount(void)
{
	int     ucRet;	
	uint8_t sAmountTemp[7];
	long    szLongNorAmount;
	char    AscNormalAmount[20];
	char    AscaddAmount[20];
	long    szLongAddAmount;
//	int     iKey;

	memcpy(sAmountTemp,PosCom.stTrans.sAmount,6);
	PubBcd2Asc0(sAmountTemp,6,AscNormalAmount);
	PosCom.stTrans.lpreTipOriAmount = atol((char*)AscNormalAmount); //消费交易原始金额

	//显示正常消费金额
	memcpy(PosCom.stTrans.preTipOriAmount,PosCom.stTrans.sAmount,sizeof(PosCom.stTrans.sAmount));
	PubBcd2Asc0(PosCom.stTrans.sAmount,6,AscNormalAmount);
	szLongNorAmount = atol((char*)AscNormalAmount);
	
	//显示总金额
	szLongAddAmount = szLongNorAmount + PosCom.stTrans.lpreTipAmount;
	sprintf((char *)AscaddAmount, "%012ld", szLongAddAmount);

	//计算ARQC需用到交易金额加小费金额，这里先把金额置为总金额，计算完成后恢复组包
	//memcpy(PosCom.stTrans.ucNormalTipAmt,PosCom.stTrans.sAmount,6);		//恢复的时候用
	memset(PosCom.stTrans.sAmount,0,sizeof(PosCom.stTrans.sAmount));
	PubAsc2Bcd(AscaddAmount,12,(char *)PosCom.stTrans.sAmount);

	ucRet = ComfirAmount(PosCom.stTrans.preTipOriAmount);
	if( ucRet != OK )
	{
		return ucRet;
	}
	return OK;
}

/********************
***确认小费金额
********************/
uint8_t ConfirmTipAmount(void)
{
	uint8_t sAmountTemp[7];
	long    szLongNorAmount;
	char    AscNormalAmount[20];
	char    AsTipAmount[20];
	long    SzTipAmount;
	char    AsaddAmount[20];
	long    SzaddAmount;  //总金额
	int     iKey;

	
/*	
	if(stPosParam.szpreTip == PARAM_OPEN && stTemp.iTransNo == POS_SALE &&
		PosCom.stTrans.TransFlag == PRE_TIP_SALE )
*/
	{
		//交易原始金额
		memcpy(PosCom.stTrans.preTipOriAmount,PosCom.stTrans.sAmount,sizeof(PosCom.stTrans.sAmount));
		memcpy(sAmountTemp,PosCom.stTrans.sAmount,6);

		//显示正常消费金额
		PubBcd2Asc0(PosCom.stTrans.sAmount,6,AscNormalAmount);
		szLongNorAmount = atol((char*)AscNormalAmount);
		
		//显示小费金额
		PubBcd2Asc0(PosCom.stTrans.sTipAmount,6,AsTipAmount);
		SzTipAmount= atol((char*)AsTipAmount);

		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, "VENTA");
		lcdDisplay(0, 4, DISP_HFONT16, "MONTO PROPINA CORRECTO?");
		lcdDisplay(0, 5, DISP_HFONT16, "$ %ld.%02ld ",(SzTipAmount)/100,(SzTipAmount)%100);
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if ( iKey == KEY_ENTER || iKey == KEY1)
		{
			//计算ARQC需用到交易金额加小费金额，这里先把金额置为总金额，计算完成后恢复组包
			SzaddAmount = szLongNorAmount +SzTipAmount;
			sprintf(AsaddAmount,"%010ld%02ld\n",SzaddAmount/100,SzaddAmount%100);
			AscToBcd(PosCom.stTrans.sAmount,(uint8_t*)AsaddAmount,12);	
			return OK;

		}
		else if(  iKey == KEY2 ){
			return RE_SELECT;
		}
		else{
			return NO_DISP;
		}	
	}
	return OK;
}


uint8_t InputWaiterNo(void)
{
	char buf[20];
	int iRet;

	if( (stPosParam.szpreTip == PARAM_OPEN && stTemp.iTransNo == POS_SALE && 
		PosCom.stTrans.TransFlag == PRE_TIP_SALE) ||
		(stTemp.iTransNo == POS_PREAUTH && PosCom.stTrans.TransFlag == POS_PREAUTH) ){
	REINPUT:
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_HFONT16|DISP_INVLINE, NULL, "VENTA");
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "Mesero:");
		lcdFlip();
		lcdGoto(140, 6);
		iRet = kbGetString(KB_EN_NUM/*+KB_EN_BIGFONT*/+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 3, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
		{
			return -1;
		}			
		PosCom.stTrans.waiterNo = atol(buf);	

		if( PosCom.stTrans.waiterNo == 0)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "Mesero invalido");
			lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "Intente de nuevo");
			lcdFlip();
			FailBeep();
			PubWaitKey(2);
			goto REINPUT;
		}
	}
	return OK;
}

int TransInit(uint8_t ucTranType)
{
	int iRet;
	
	lcdClrLine(2, 7);	
	stTemp.iTransNo= ucTranType;
	iRet = CheckTransValid();
	if( iRet != OK ){
		return iRet;
	}

	return 0;
}
//gasplatform
/*************************************************************
***说明:根据输入的bomba，向后台请求交易金额
**************************************************************/
uint8_t GetAmount_FromGasPlat()
{
	uint8_t	ucRet =0;
	uint8_t sTempBuff[60] ={0};
	char szField63BuffTemp[999];
	//char *Temp1 = NULL;

	char szSNTemp[60] = {0};
	char szSTTemp[60] = {0};
	char szLGTemp[60] = {0};
	char szV1temp[60]  ={0};
	char szCBtemp[60] = {0};
	char szCNtemp[20] = {0};
	char szC1temp[20] = {0};

	char szDM[60] = {0};
   
	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200] = {0};
	int  szTotalLen = 0;


	char AscNormalAmount[20] ={0};//后台返回金额的ascii码
	char AscAmount[20] ={0};//后台返回金额的ascii码

	long szAmount =0;//后台返回的总金额
	int     iKey;
	int     szlen =0;
	
	ucRet = AppGetBomba();
	if( ucRet!=OK )
	{
		return NO_DISP;
	}
	
	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0100");
	sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "770000");
	sprintf((char *)glSendPack.szTranAmt,  "000000000000");
	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	if(PosCom.ucSwipedFlag == CARD_SWIPED)
	{
		sprintf((char *)glSendPack.szEntryMode,"520" );//目前不知道这个域代表什么
	}
	else if(PosCom.ucSwipedFlag == CARD_INSERTED)
	{
		sprintf((char *)glSendPack.szEntryMode,"550" );
	}
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char *)glSendPack.szSvrCode, "00");
	
	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);

	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);

	memset(sTempBuff,0,sizeof(sTempBuff));
	sysReadSN(stPosParam.szSN);

	sprintf(szSTTemp,"ST%05d%s!",strlen(stPosParam.szST),stPosParam.szST);
	szParaNum++;
	szParaLen += strlen(szSTTemp);

	sprintf(szDM,"DM%05d%s!",strlen("00"),"00");
	szParaNum++;
	szParaLen += strlen(szDM);

	sprintf(szSNTemp,"XK%05d%2.2s-%3.3s-%3.3s!",strlen(stPosParam.szSN)+2,
		stPosParam.szSN,stPosParam.szSN+2,stPosParam.szSN+5);
	szParaNum++;
	szParaLen += strlen(szSNTemp);

	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG%05d%s!",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);

	strcpy(szV1temp,"V1000011!");
	szParaNum++;
	szParaLen += strlen(szV1temp);
	
	memcpy(stPosParam.szCB,PosCom.stTrans.szCardNo,6);
	sprintf(szCBtemp,"CB%05d%s",strlen(stPosParam.szCB),stPosParam.szCB);
	szParaNum++;
	szParaLen += strlen(szLGTemp);
	ucRet = GetCardType();	
	
	switch(PosCom.stTrans.CardType)
	{
		case 1:
			strcpy(szCNtemp,"CN000011!");
			break;
		case 2:
			strcpy(szCNtemp,"CN000012!");
			break;
		case 3:
			strcpy(szCNtemp,"CN000013!");
			break;
		case 5:
			strcpy(szCNtemp,"CN000015!");
			break;
		case 6:
			strcpy(szCNtemp,"CN000016!");
			break;
		case 7:
			strcpy(szCNtemp,"CN000017!");
			break;
		case 8:
			strcpy(szCNtemp,"CN000018!");
			break;
		default:
			strcpy(szCNtemp,"CN000010!"); //有待完善，电子钱包卡还没细分
			break;
	}
	szParaNum++;
	szParaLen += strlen(szCNtemp);

	sprintf(szC1temp,"C1%05d%s",strlen(PosComconTrol.szBomba),PosComconTrol.szBomba);
	szParaNum++;
	szParaLen += strlen(szC1temp);

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	//szTotalLen += szParaNum;     // 加上空格
	sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);	

	sprintf((char*)szField63BuffTemp,"%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1temp,szCBtemp,szCNtemp,szC1temp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);

	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));

	PosCom.stTrans.isCtlplat =1; //连接控制平台

	ucRet = SendRecvPacket();
	
	if( ucRet!=OK )
	{
		return ucRet;
	}
	//显示后台返回的加油信息
	//punto clave and EDENRED card should show amount in next step
	PrintDebug("%s%d", "PosCom.stTrans.CardType:",PosCom.stTrans.CardType);

	if(PosCom.stTrans.CardType!= 4 && PosCom.stTrans.CardType !=7)
	{
		//显示正常消费金额
		PrintDebug("%s%s", "stTemp.oil_amount=",PosCom.stTrans.oil_amount);

		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, "VENTA");
		lcdDisplay(0, 4, DISP_HFONT16, "MONTO PROPINA CORRECTO?");
		lcdDisplay(0, 5, DISP_HFONT16, "$ %s ",(char*)PosCom.stTrans.oil_amount);
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();

		szlen =strlen(PosCom.stTrans.oil_amount);
		strncpy(AscNormalAmount,PosCom.stTrans.oil_amount,szlen-3);
		strncpy(AscNormalAmount+szlen-3,PosCom.stTrans.oil_amount+szlen-2,2);

		szAmount = atoi((char*)AscNormalAmount);

		sprintf(AscAmount,"%010ld%02ld\n",szAmount/100,szAmount%100);
		AscToBcd(PosCom.stTrans.sAmount,(uint8_t*)AscAmount,12);

		PrintDebug("%s%s", "AscAmount:",AscAmount);

		
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if ( iKey == KEY_ENTER || iKey == KEY1)
		{

		}
		else if(  iKey == KEY2 ){
			return RE_SELECT;
		}
		else{
			return NO_DISP;
		}	
	}
	return OK;
}

//gasplatform
/*************************************************************
***说明:从后台获取余额
**************************************************************/
uint8_t GetBalance_FromBankPlat(int flag)
{
	uint8_t	ucRet,iRet;
	uint8_t   Amount[6];
	
	uint8_t sTempBuff[60] ={0};
	char szField63BuffTemp[999] ={0};
	char buf[50] ={0};
	
	char *Temp1 = NULL;
	char *Temp2 =NULL;

	char AscNormalAmount[20] ={0};//后台返回金额的ascii码
	char AscAmount[20] ={0};//后台返回金额的ascii码

	long szAmount =0;//后台返回的总金额
	long szcount =0; //输入的金额或容量
	long tmpnum =0;
	long tmpnum2 =0;
	
	int     iKey;
	int     szlen =0;

	uint8_t menuflag1 = 0;
	uint8_t menuflag2 = 0;
	uint8_t menuflag3 = 0;

	char useflag =0;
	char showflag =0;
	char tmpvalue[4] ={0};
	
	//此处需要刷卡
	if(flag ==PURSE_GETBALANCE || flag ==PURSE_GETRULE)
	{
		ucRet = SaleTranGetData();
		PrintDebug("%s%d", "SaleTranGetData:",ucRet);
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}

	{
	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0100");
	if(flag ==PURSE_GETRULE)
	{
		sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "820000");
		sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);
			
		if(strlen(PosCom.stTrans.szExpDate)!=0)
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
		else
			sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   "0000");
	}
	else
	{
		sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "830000");
	}
	PrintDebug("%s%s", "cardno:",PosCom.stTrans.szCardNo);

	memset(Amount,0,sizeof(Amount));
	BcdToAsc0(glSendPack.szTranAmt, Amount, 12);

	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));

	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char*)glSendPack.szSvrCode,"00"); //25域
	switch(PosCom.ucSwipedFlag)
	{
		case CARD_SWIPED:
			//memcpy(PosCom.stTrans.szEntryMode, "021", 3);
			memcpy(PosCom.stTrans.szEntryMode, "520", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
			if(flag != PURSE_GETRULE)
			{
				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			}
			break;
		case CARD_INSERTED:
			//memcpy(PosCom.stTrans.szEntryMode, "050", 3);
			memcpy(PosCom.stTrans.szEntryMode, "550", 3);
			PrintDebug("%s%d", "glSendPack.szTrack2:",PosCom.stTrans.CardType);

			if(flag ==PURSE_GETRULE)
			{
				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, PosCom.szTrack2);
			}
			else
			{
				memset(sTempBuff,0,sizeof(sTempBuff));
				memcpy(sTempBuff,PosCom.szTrack2,16);
				Temp2 = strchr((char *)PosCom.szTrack2, '=');
				if( Temp2==NULL )
				{
					return FALSE;
				}
				memcpy(sTempBuff+16,Temp2,5);
				PrintDebug("%s%s", "glSendPack.szTrack2:",sTempBuff);

				sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, sTempBuff);
			}
			break;
		case CARD_KEYIN:
		default:
			break;
	}

	//37 Field
	sprintf((char*)glSendPack.szRRN,"%s","123123123123");

	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);
	sprintf((char*)glSendPack.szField48, "%.*s",LEN_TERM_ID -4 ,stPosParam.szST);   // store id
	sprintf((char *)glSendPack.szCurrencyCode, "484");
	if(PRE_TIP_SALE ==PosCom.stTrans.TransFlag)
	{
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.sTipAmount, 12);
	}
	memset(sTempBuff,0,sizeof(sTempBuff));
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	
	//NetpayDealFeild63((uint8_t *)szField63BuffTemp);	
	Get_Rule_Balance_Feild63((uint8_t *)szField63BuffTemp);	

	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);
	
	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	
	PosCom.stTrans.isCtlplat =0;

	}
	//ucRet = SendRecvPacket();
	ucRet = OnlineCommProc();
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if(flag ==PURSE_GETBALANCE || flag ==PURSE_GETRULE)
	{
		//显示正常消费金额
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, "Consultar Saldo");
		lcdDisplay(0, 4, DISP_HFONT16, "Saldo:   $ %s ",(char*)PosCom.stTrans.Balance_Amount);
		usleep(3*1000);
		NetpayPrtBalance();
		return OK;
	}

	//根据返回值提示的交互界面
	{
	#if 0
		if(stPosParam.stVolContrFlg == PARAM_CLOSE)
		{
		//choice produce
			Temp2 = strchr(stTemp.oil_id,'1');
			if(Temp2 !=NULL)
			{
				menuflag1 ='1';
			}
			Temp2 = strchr(stTemp.oil_id,'2');
			if(Temp2 !=NULL)
			{
				menuflag2 ='1';
			}
			Temp2 = strchr(stTemp.oil_id,'3');
			if(Temp2 !=NULL)
			{
				menuflag3 ='1';
			}
			if(menuflag1 ==0 && menuflag2 ==0 && menuflag3 ==0)
			{
				printf("GetRules_FromBankPlat->Y1 ERROR!!!\n");
				return NO_DISP;
			}

			InitMenu(MENU_MODE_1, "SELECIONE PRODUCTO");
			MainMenuAddMenuItem(menuflag1,    1,       		   "MAGNA              ",         NULL);		
			MainMenuAddMenuItem(menuflag2,    2,       		   "PREMIUM             ",         NULL);
			MainMenuAddMenuItem(menuflag3,    3,       		   "DIESEL",         NULL);
		

		    	iRet = DispDynamicMenu(1);
			if( iRet == 255 )		//NO_TRANS
			{
				return NO_DISP;
			}
			else
			{
				memset(stTemp.oil_id,0,sizeof(stTemp.oil_id));
				switch(iRet)
				{
					case 1:
						stTemp.oil_id[0] = '1';
						break;
					case 2:
						stTemp.oil_id[0] = '2';
						break;
					case 3:
						stTemp.oil_id[0] = '3';
						break;
					default:
						break;
				}
			}
		}
#endif

	//choice consule	
		PrintDebug("%s%s:%c", "stTemp.oil_id:",PosCom.stTrans.saletype,PosCom.stTrans.oil_id[0]);

		Temp2 = strchr(PosCom.stTrans.saletype,PosCom.stTrans.oil_id[0]);
		if(Temp2 ==NULL)
		{
			return LITER_WITHOUT;
		}
		
		memset(PosCom.stTrans.saletype,0,sizeof(PosCom.stTrans.saletype));
		PosCom.stTrans.saletype[0] ='2' ;//stTemp.oil_id[0];
		PosCom.stTrans.szInputType ='2';


	//输入driver num和nip
	//	default value:
		memcpy(PosCom.stTrans.Driver_num,"1234",4);
		if(memcmp(PosCom.stTrans.DriverRule,"0|1",3)==0 ||memcmp(PosCom.stTrans.DriverRule,"1|0",3)==0)
		{
			iRet =AppInputDrivernum_nip(0);
			if(iRet !=OK)
			{
				return iRet;
			}
			if(memcmp(PosCom.stTrans.DriverRule,"0|1",3)==0)
			{
				PosCom.stTrans.isnip[0] ='1';
			}
			else
			{
				PosCom.stTrans.isnip[0] ='0';
			}
		}

		lcdCls();
		lcdDisplay(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, "LITROS");
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "INDIQUE LITROS:");
		lcdFlip();
	
		memset(sTempBuff,0,sizeof(sTempBuff));
		do{
			lcdGoto(120, 7);
			lcdDisplay(0, 7, DISP_CFONT, "           $");
			lcdFlip();
			iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)sTempBuff);
			if( iRet<0 )
			{
				return E_TRANS_CANCEL;
			}		
			szcount= atol((char *)sTempBuff);
			break;
		}while(1);  

		memset(sTempBuff,0,sizeof(sTempBuff));
		sprintf((char *)sTempBuff, "%012d", szcount);
		AscToBcd(PosCom.stTrans.sLiters, sTempBuff, 12);
		
//choice place
	#if 1
		Temp1 = strchr(PosCom.stTrans.plate,'|');
		if(Temp1 !=NULL)
		{
			memset(sTempBuff,0,sizeof(sTempBuff));
			szlen = strlen(PosCom.stTrans.plate);
			useflag = PosCom.stTrans.plate[0];
			showflag = PosCom.stTrans.plate[szlen-1];
			memcpy(sTempBuff,PosCom.stTrans.plate+2,szlen-4);
		}
		if(showflag =='1')
		{
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "Ingrese Placas:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
			iRet = PubGetString(ALPHA_IN|ECHO_IN,0,8,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.plate,0,sizeof(PosCom.stTrans.plate));
				strcpy(PosCom.stTrans.plate,(char*) buf);
			}
		}
		else
		{
			strcpy(PosCom.stTrans.plate,(char*) sTempBuff);
		}
		PrintDebug("%s%s", "stTemp.plate:",PosCom.stTrans.plate);

		//input Horometer
		Temp1 = strchr(PosCom.stTrans.Horometer,'|');
		if(Temp1 !=NULL)
		{
			memset(sTempBuff,0,sizeof(sTempBuff));
			szlen = strlen(PosCom.stTrans.Horometer);
			showflag = PosCom.stTrans.Horometer[szlen-1];
			memcpy(tmpvalue,PosCom.stTrans.Horometer,szlen-2);
			tmpnum = AscToLong(tmpvalue, strlen(tmpvalue));
			PrintDebug("%s%d", "stTemp.Horometer:",tmpnum);

		}
		if(showflag =='1')
		{
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "Horometer:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
			iRet = PubGetString(NUM_IN,0,6,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				tmpnum2 = AscToLong(buf, strlen(buf));
				if(tmpnum2 <tmpnum)
				{
					return NO_DISP;
				}
				memset(PosCom.stTrans.Horometer,0,sizeof(PosCom.stTrans.Horometer));
				strcpy(PosCom.stTrans.Horometer,(char*) buf);
			}
		}
		else
		{
			//input Kilometer
			Temp1 = strchr(PosCom.stTrans.kilometer,'|');
			if(Temp1 !=NULL)
			{
				memset(sTempBuff,0,sizeof(sTempBuff));
				szlen = strlen(PosCom.stTrans.kilometer);
				showflag = PosCom.stTrans.kilometer[szlen-1];
				memcpy(tmpvalue,PosCom.stTrans.kilometer,szlen-2);
				tmpnum = AscToLong(tmpvalue, strlen(tmpvalue));
				PrintDebug("%s%d", "stTemp.kilometer:",tmpnum);


			}
			if(showflag =='1')
			{
				memset(buf,0,sizeof(buf));
				lcdCls();
				lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
				DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
				DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "kilometer:");
				lcdFlip();
				lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
				lcdGoto(120, 5);
				iRet = PubGetString(NUM_IN,0,6,buf,stPosParam.ucOprtLimitTime,0);
				if( iRet!=0 )
				{
					return NO_DISP;
				}
				else
				{
					tmpnum2 = AscToLong(buf, strlen(buf));
					if(tmpnum2 <tmpnum)
					{
						return NO_DISP;
					}
					memset(PosCom.stTrans.kilometer,0,sizeof(PosCom.stTrans.kilometer));
					strcpy(PosCom.stTrans.kilometer,(char*) buf);
				}
			}
			else
			{
				memset(PosCom.stTrans.kilometer,0,sizeof(PosCom.stTrans.kilometer));
				sprintf(PosCom.stTrans.kilometer,"%d",tmpnum);
			}
		}

		//Ingrese NIP
		Temp1 = strchr(PosCom.stTrans.DriverRule,'|');
		if(Temp1 !=NULL)
		{
			memset(sTempBuff,0,sizeof(sTempBuff));
			szlen = strlen(PosCom.stTrans.DriverRule);
			showflag = PosCom.stTrans.DriverRule[szlen-1];
			PrintDebug("%s%c", "stTemp.DriverRule:",showflag);


		}
		if(showflag =='1')
		{
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "Ingrese NIP:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);

			iRet = kbGetString(KB_BIG_PWD,0,4,0,buf);

			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.nip,0,sizeof(PosCom.stTrans.nip));
				strcpy(PosCom.stTrans.nip,(char*) buf);
			}
		}
	#endif

#if 1
		//DISP_CFONT|DISP_MEDIACY|DISP_INVLINE
		lcdCls();
		lcdDisplay(0, 2, DISP_HFONT16, "CONFIRMACION");
		lcdDisplay(0, 4, DISP_CFONT, "REALIZAR LA VENTA?");		
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		 if(  iKey != KEY1 && iKey !=KEY_ENTER)
		{
			return NO_DISP;
		}
		
		lcdCls();
		lcdDisplay(0, 2, DISP_HFONT16, "CONFIRMACION");
	
		if(PosCom.stTrans.saletype[0] == '1')
		{
			lcdDisplay(0, 4, DISP_CFONT, "IMPORTE: %ld.%02ld ",(szcount)/100,(szcount)%100);
		}
		else if(PosCom.stTrans.saletype[0] == '2')
		{
			lcdDisplay(0, 4, DISP_CFONT, "LITROS: %ld.%02ld ",(szcount)/100,(szcount)%100);
		}
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if ( iKey != KEY_ENTER &&  iKey != KEY1)
		{
			return NO_DISP;
		}
#endif
	}
	return OK;
}
/*************************************************************
***说明:从后台获取交易详细信息
**************************************************************/
uint8_t GetRules_FromBankPlat(int flag)
{
	uint8_t	ucRet,iRet;
	uint8_t   Amount[6];
	
	uint8_t sTempBuff[60] ={0};

	char szField63BuffTemp[999];
	char *Temp2 =NULL;

	char AscNormalAmount[20] ={0};//后台返回金额的ascii码
	char AscAmount[20] ={0};//后台返回金额的ascii码

	long szAmount =0;//后台返回的总金额
	long szcount =0; //输入的金额或容量
	
	int     iKey;
	int     szlen =0;

	uint8_t menuflag1 = 0;
	uint8_t menuflag2 = 0;
	uint8_t menuflag3 = 0;
	{
	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0100");
	sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "810000");
	sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      PosCom.stTrans.szCardNo);

	memset(Amount,0,sizeof(Amount));
	BcdToAsc0(glSendPack.szTranAmt, Amount, 12);


	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	GetPosTime(stTemp.szDate, stTemp.szTime);		
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);
	strcpy((char*)glSendPack.szLocalTime,(char*)PosCom.stTrans.szTime);
	strcpy((char*)glSendPack.szLocalDate,(char*)(PosCom.stTrans.szDate+4));
	if(strlen(PosCom.stTrans.szExpDate)!=0)
		sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   PosCom.stTrans.szExpDate);
	else
		sprintf((char *)glSendPack.szExpDate,  "%.*s", LEN_EXP_DATE,   "0000");
	
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char*)glSendPack.szSvrCode,"00"); //25域
	switch(PosCom.ucSwipedFlag)
	{
		case CARD_SWIPED:
			//memcpy(PosCom.stTrans.szEntryMode, "021", 3);
			memcpy(PosCom.stTrans.szEntryMode, "520", 3);
			sprintf((char *)glSendPack.szEntryMode, "%s0",  PosCom.stTrans.szEntryMode);
			break;
		case CARD_INSERTED:
			//memcpy(PosCom.stTrans.szEntryMode, "050", 3);
			memcpy(PosCom.stTrans.szEntryMode, "550", 3);
		case CARD_KEYIN:
		default:
			break;
	}

	//37 Field
	sprintf((char*)glSendPack.szRRN,"%s","123123123123");

	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);
	sprintf((char*)glSendPack.szField48, "%.*s",LEN_TERM_ID -4 ,stPosParam.szST);   // store id
	sprintf((char *)glSendPack.szCurrencyCode, "484");
	if(PRE_TIP_SALE ==PosCom.stTrans.TransFlag)
	{
		//sprintf((char *)glSendPack.szExtAmount,"%.*s",LEN_EXT_AMOUNT,PosCom.stTrans.sTipAmount);
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.sTipAmount, 12);
	}
	memset(sTempBuff,0,sizeof(sTempBuff));
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);
	
	//NetpayDealFeild63((uint8_t *)szField63BuffTemp);	
	Get_Rule_Balance_Feild63((uint8_t *)szField63BuffTemp);	

	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char*)glSendPack.szField63,szField63BuffTemp);
	
	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	
	PosCom.stTrans.isCtlplat =0;

	}
	ucRet = SendRecvPacket();
	
	if( ucRet!=OK )
	{
		return ucRet;
	}
	//显示后台返回的加油信息
	//如果容量控制打开，此处才显示
#if 0
	if(stPosParam.stVolContrFlg == PARAM_OPEN)
	{
		//显示正常消费金额
		PrintDebug("%s%s", "stTemp.oil_amount=",stTemp.oil_amount);
	
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, "VENTA");
		lcdDisplay(0, 4, DISP_HFONT16, "MONTO PROPINA CORRECTO?");
		lcdDisplay(0, 5, DISP_HFONT16, "$ %s ",(char*)stTemp.oil_amount);
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if ( iKey == KEY_ENTER || iKey == KEY1)
		{
			szlen =strlen(stTemp.oil_amount);
			strncpy(AscNormalAmount,stTemp.oil_amount,szlen-3);
			strncpy(AscNormalAmount+szlen-3,stTemp.oil_amount+szlen-2,2);

			szAmount = atoi((char*)AscNormalAmount);

			sprintf(AscAmount,"%010ld%02ld\n",szAmount/100,szAmount%100);
			AscToBcd(PosCom.stTrans.sAmount,(uint8_t*)AscAmount,12);
			PrintDebug("%s%s","AscAmount:",AscAmount);
		}
		else if(  iKey == KEY2 )
		{
			return RE_SELECT;
		}
		else{
			return NO_DISP;
		}	
	}
#endif

	//根据返回值提示的交互界面
	{
	#if 0
		if(stPosParam.stVolContrFlg == PARAM_CLOSE)
		{
		//choice produce
			Temp2 = strchr(stTemp.oil_id,'1');
			if(Temp2 !=NULL)
			{
				menuflag1 ='1';
			}
			Temp2 = strchr(stTemp.oil_id,'2');
			if(Temp2 !=NULL)
			{
				menuflag2 ='1';
			}
			Temp2 = strchr(stTemp.oil_id,'3');
			if(Temp2 !=NULL)
			{
				menuflag3 ='1';
			}
			if(menuflag1 ==0 && menuflag2 ==0 && menuflag3 ==0)
			{
				printf("GetRules_FromBankPlat->Y1 ERROR!!!\n");
				return NO_DISP;
			}

			InitMenu(MENU_MODE_1, "SELECIONE PRODUCTO");
			MainMenuAddMenuItem(menuflag1,    1,       		   "MAGNA              ",         NULL);		
			MainMenuAddMenuItem(menuflag2,    2,       		   "PREMIUM             ",         NULL);
			MainMenuAddMenuItem(menuflag3,    3,       		   "DIESEL",         NULL);
		

		    	iRet = DispDynamicMenu(1);
			if( iRet == 255 )		//NO_TRANS
			{
				return NO_DISP;
			}
			else
			{
				memset(stTemp.oil_id,0,sizeof(stTemp.oil_id));
				switch(iRet)
				{
					case 1:
						stTemp.oil_id[0] = '1';
						break;
					case 2:
						stTemp.oil_id[0] = '2';
						break;
					case 3:
						stTemp.oil_id[0] = '3';
						break;
					default:
						break;
				}
			}
		}
	#endif

	//make sure if input new nip
		//如果没有nip，需要重新设置nip
		if(PosCom.stTrans.isnip[0] =='0')
		{
			iRet =AppInputNip();
		}
	//choice consule
		menuflag1 =0;
		menuflag2 =0;
		
 		Temp2 = strchr(PosCom.stTrans.saletype,'1');
		if(Temp2 !=NULL)
		{
			menuflag1 ='1';
		}
		Temp2 = strchr(PosCom.stTrans.saletype,'2');
		if(Temp2 !=NULL)
		{
			menuflag2 ='1';
		}
		
		lcdCls();
		InitMenu(MENU_MODE_1, "Consumo");
		MainMenuAddMenuItem(menuflag1,    1,       		   "IMPORTE              ",         NULL);		
		MainMenuAddMenuItem(menuflag2,    2,       		   "LITROS             ",         NULL);
	
	    	iRet = DispDynamicMenu(1);
		if( iRet == 255 )		//NO_TRANS
		{
			return NO_DISP;
		}
		else
		{
			memset(PosCom.stTrans.saletype,0,sizeof(PosCom.stTrans.saletype));
			switch(iRet)
			{
				case 1:
					PosCom.stTrans.saletype[0] = '1';
					PosCom.stTrans.szInputType ='1';
					break;
				case 2:
					PosCom.stTrans.saletype[0] = '2';
					PosCom.stTrans.szInputType ='2';
					break;
				default:
					break;
			}
		}

		if(PosCom.stTrans.saletype[0] != '1' && PosCom.stTrans.saletype[0] != '2')
		{
			return NO_DISP;
		}
//INPUT count
		while(1)
		{
			if(PosCom.stTrans.saletype[0] == '1')
			{
				lcdCls();
				lcdDisplay(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, "IMPORTE");
				lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "INDIQUE IMPORTE:");
				lcdFlip();
			}
			else
			{
				lcdCls();
				lcdDisplay(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, "LITROS");
				lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "INDIQUE LITROS:");
				lcdFlip();
			}


			memset(sTempBuff,0,sizeof(sTempBuff));
			{
				lcdGoto(120, 7);
				lcdDisplay(0, 7, DISP_CFONT, "           $");
				lcdFlip();
				iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)sTempBuff);
				if( iRet<0 )
				{
					return E_TRANS_CANCEL;
				}		
				szcount= atol((char *)sTempBuff);
				if(PosCom.stTrans.saletype[0] == '1')
				{
				/*
					szAmount = BcdToLong(PosCom.stTrans.sAmount, 6);
					if(szcount > szAmount)
					{
						continue;
					}
				*/
				}
				else if(PosCom.stTrans.saletype[0] == '2')
				{
					memset(sTempBuff,0,sizeof(sTempBuff));
					sprintf((char *)sTempBuff, "%012d", szcount);
					AscToBcd(PosCom.stTrans.sLiters, sTempBuff, 12);	
				}
				memset(sTempBuff,0,sizeof(sTempBuff));
				sprintf((char *)sTempBuff, "%012d", szcount);
				AscToBcd(PosCom.stTrans.sAmount, sTempBuff, 12);
				PrintDebug("%s%s", "PosCom.stTrans.sAmount:=",sTempBuff);
				break;
			} 
		}
	}
	return OK;
}
//gasplatform
/*************************************************************
***说明:将预授权成功金额发给加油平台确认
**************************************************************/
uint8_t ComfirmAmount_ToGasPlat()
{
	uint8_t	ucRet;
	uint8_t sTempBuff[60] ={0};
	char szField63BuffTemp[999];

	char szSNTemp[60] = {0};
	char szSTTemp[60] = {0};
	char szLGTemp[60] = {0};
	char szV1temp[60]  ={0};
	char szCBtemp[60] = {0};
	char szCNtemp[20] = {0};
	char szC1temp[20] = {0};

	char szDM[60] = {0};
   
	int  szParaNum = 0;        // 63域参数个数
	int  szParaLen = 0;        // 63域参数长度
	char szTotalLenTemp[200] = {0};
	int  szTotalLen = 0;

	int     iKey;
	if(0)
	{
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "ENTRE TO CONFIRM");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdGoto(120, 5);
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT);
		if(iKey ==KEY_CANCEL)
		{
			return NO_DISP;
		}
	}
	
	memset(&glSendPack, 0, sizeof(STISO8583));
	sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,  "0200");
	sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE, "770000");
	//消费总额
	BcdToAsc0(glSendPack.szTranAmt, PosCom.stTrans.sAmount, 12);
	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);

	if(PosCom.ucSwipedFlag == CARD_SWIPED)
	{
		sprintf((char *)glSendPack.szEntryMode,"520" );//目前不知道这个域代表什么
	}
	else if(PosCom.ucSwipedFlag == CARD_INSERTED)
	{
		sprintf((char *)glSendPack.szEntryMode,"550" );
	}
	sprintf((char *)glSendPack.szNII, "0003");      // 24域NII
	sprintf((char *)glSendPack.szSvrCode, "00");
	
	sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,   PosCom.stTrans.szPosId);
	sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, PosCom.szUnitNum);

	//小费金额
	BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.sTipAmount, 12);

	//60域
	ShortToByte(glSendPack.szField60, strlen(PosCom.stTrans.oil_filed_60));
	sprintf((char*)glSendPack.szField60+2,"%s",PosCom.stTrans.oil_filed_60);
	
	ShortToByte(glSendPack.sField62, strlen("172839"));
	sprintf((char *)sTempBuff,"%s","172839");
	sprintf((char*)glSendPack.sField62+2,"%s",sTempBuff);


	memset(sTempBuff,0,sizeof(sTempBuff));
	sysReadSN(stPosParam.szSN);

	sprintf(szSTTemp,"ST%05d%s!",strlen(stPosParam.szST),stPosParam.szST);
	szParaNum++;
	szParaLen += strlen(szSTTemp);

	sprintf(szDM,"DM%05d%s!",strlen("00"),"00");
	szParaNum++;
	szParaLen += strlen(szDM);

	sprintf(szSNTemp,"XK%05d%2.2s-%3.3s-%3.3s!",strlen(stPosParam.szSN)+2,
		stPosParam.szSN,stPosParam.szSN+2,stPosParam.szSN+5);
	szParaNum++;
	szParaLen += strlen(szSNTemp);

	strcpy(stPosParam.szLG,"123456");
	sprintf(szLGTemp,"LG%05d%s",strlen(stPosParam.szLG),stPosParam.szLG);
	szParaNum++;
	szParaLen += strlen(szLGTemp);

	strcpy(szV1temp,"V1000011!");
	szParaNum++;
	szParaLen += strlen(szV1temp);
	
	//memcpy(stPosParam.szCB,PosCom.stTrans.szCardNo,6);
	sprintf(szCBtemp,"CB%05d%s",strlen(stPosParam.szCB),stPosParam.szCB);
	szParaNum++;
	szParaLen += strlen(szLGTemp);

	switch(PosCom.stTrans.CardType)
	{
		case 1:
			strcpy(szCNtemp,"CN000011!");
			break;
		case 2:
			strcpy(szCNtemp,"CN000012!");
			break;
		case 3:
			strcpy(szCNtemp,"CN000013!");
			break;
		case 5:
			strcpy(szCNtemp,"CN000015!");
			break;
		case 6:
			strcpy(szCNtemp,"CN000016!");
			break;
		case 7:
			strcpy(szCNtemp,"CN000017!");
			break;
		case 8:
			strcpy(szCNtemp,"CN000018!");
			break;
		default:
			strcpy(szCNtemp,"CN000010!"); //有待完善，电子钱包卡还没细分
			break;
	}
	szParaNum++;
	szParaLen += strlen(szCNtemp);

	sprintf(szC1temp,"C1%05d%s",strlen(PosComconTrol.szBomba),PosComconTrol.szBomba);
	szParaNum++;
	szParaLen += strlen(szC1temp);

	szTotalLen += 13;            // 本身长度
	szTotalLen += szParaLen;     // 其他参数长度
	sprintf(szTotalLenTemp,"& 000%02d%05d!",szParaNum,szTotalLen);	

	sprintf((char*)szField63BuffTemp,"%s%s%s%s%s%s%s%s%s",szTotalLenTemp,szSTTemp,szDM,szSNTemp,szLGTemp,szV1temp,szCBtemp,szCNtemp,szC1temp);
	memset(glSendPack.szField63,0,sizeof(glSendPack.szField63));
	strcpy((char *)glSendPack.szField63,szField63BuffTemp);
	memset(glSendPack.sMacData,0,sizeof(glSendPack.sMacData));
	PosCom.stTrans.isCtlplat =1; //连接控制平台
	ucRet = OnlineCommProc();
	return ucRet;

}

// end of file
