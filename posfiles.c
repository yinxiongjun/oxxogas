

#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h" 

#define FILE_APP_LOG    "SysParam.log"
#define FILE_CTRL_LOG	"ctrl.log"
#define FILE_COMM_PAR	"comm.par"
#define FILE_OPER_LOG	"oper.log"
#define FILE_TOTAL_LMT	"total.lmt"
#define FILE_LAST_TOTAL	"lasttotal"
#define FILE_EMV_AID    "emv.aid"
#define FILE_EMV_CAPK   "emv.capk"
#define FILE_UPL_TRACE  "txntrace.pair"
#define FILE_NETPAY_PARA  "GASPLAT.para"     //TMS 参数列表文件

uint8_t DispOrignalTrans(NEWPOS_LOG_STRC *pstLog);
uint8_t CheckOutDispOrignalTrans(NEWPOS_LOG_STRC *pstLog,uint8_t ChangeAmountFlag,uint8_t *sAmount);


uint8_t SaveCtrlParam(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_CTRL_LOG, O_RDWR|O_CREAT);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)&stTransCtrl, sizeof(struct _TRANS_CTRL_PARAM));
	if( ret!=sizeof(struct _TRANS_CTRL_PARAM) )
	{
		DispFileErrInfo();
		fileClose(fd);	
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}


uint8_t ReadCtrlParam(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_CTRL_LOG, O_RDWR);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
	ret = fileRead(fd, (uint8_t *)&stTransCtrl, sizeof(struct _TRANS_CTRL_PARAM));
	fileClose(fd);
	
	if( ret!=sizeof(struct _TRANS_CTRL_PARAM) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	return (OK);
}

uint8_t ReadOperFile(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_OPER_LOG, O_RDWR);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileRead(fd, (uint8_t *)&operStrc, MAX_OPER_NUM*sizeof(struct _oper_structure));
	if( ret!=MAX_OPER_NUM*sizeof(struct _oper_structure) )
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	
	return OK;
}

uint8_t WriteOperFile()
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_OPER_LOG, O_RDWR | O_CREAT);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)&operStrc, MAX_OPER_NUM*sizeof(struct _oper_structure));
	if( ret!=MAX_OPER_NUM*sizeof(struct _oper_structure) )
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}

uint8_t SaveLimitTotal(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_TOTAL_LMT, O_RDWR|O_CREAT);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)&stLimitTotal, sizeof(struct _limit_total_strc));
	if( ret!=sizeof(struct _limit_total_strc) )
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}

uint8_t ReadLimitTotal(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_TOTAL_LMT, O_RDWR);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileRead(fd, (uint8_t *) & stLimitTotal, sizeof(struct _limit_total_strc));
	if( ret!=sizeof(struct _limit_total_strc) )
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}

uint8_t ReadLastTotal(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_LAST_TOTAL, O_RDWR);
	if( fd<0 ) 
	{
		lcdCls();
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		lcdCls();
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileRead(fd, (uint8_t *)&stTotal, sizeof(struct _total_strc));
	if( ret!=sizeof(struct _total_strc) )
	{
		lcdCls();
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	
	if(memcmp(stTotal.sDateTime, "00000000000000", 14)==0)	
	{
		lcdCls();
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"NO EXIST TRANS");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return (E_MEM_ERR);
	}
	
	return (OK);
}

uint8_t SaveLastTotal(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_LAST_TOTAL, O_RDWR|O_CREAT);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)&stTotal, sizeof(struct _total_strc));
	if( ret!=sizeof(struct _total_strc) )
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}

void DispFileErrInfo(void)
{
	int err;
	char  buf[32];
	
	err = errno;
	
	switch(err) 
	{
	case FILE_EXIST: 
		strcpy(buf, "FILE EXISTED");
		break;
	case FILE_NOEXIST: 
		strcpy(buf, "FILE NO EXIST");
		break;
	case MEM_OVERFLOW: 
		strcpy(buf, "MEM OVERFLOW");
		break;
	case TOO_MANY_FILES: 
		strcpy(buf, "TOO MANY FILES");
		break;
	case INVALID_HANDLE: 
		strcpy(buf, "INVALID HANDLE");
		break;
	case INVALID_MODE: 
		strcpy(buf, "INVALID MODE");
		break;
	case FILE_NOT_OPENED: 
		strcpy(buf, "NOT OPENED");
		break;
	case END_OVERFLOW: 
		strcpy(buf, "END OVERFLOW");
		break;
	case TOP_OVERFLOW: 
		strcpy(buf, "TOP OVERFLOW");
		break;
	case NO_PERMISSION: 
		strcpy(buf, "NO_PERMISSION");
		break;
	default: 
		strcpy(buf, "OTHER");
		break;
	}
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, "  FILE ERROR    ");
	lcdDisplay(0, 6, DISP_MEDIACY|DISP_CFONT, buf);
	lcdFlip();
	FailBeep();
	kbGetKeyMs(3000);
}


uint8_t SaveCheckLogFile(void)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(POS_LOG_CHECK_FILE, O_RDWR|O_CREAT);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return E_FILE_OPEN;
	}

	iRet = fileSeek(iLogFile, (int)(stTransCtrl.iCheckTransNum*LOG_RECORD_LEN), SEEK_SET);
	if( iRet<0 ) 
	{
		DispFileErrInfo();
		fileClose(iLogFile);
		return E_FILE_SEEK;
	}
	
	PosCom.stTrans.uiTxnRecIndex = stTransCtrl.iCheckTransNum;
	iRet = fileWrite(iLogFile, (uint8_t *)&PosCom.stTrans, LOG_RECORD_LEN);
	fileClose(iLogFile);
	if( iRet!=LOG_RECORD_LEN )
	{
		DispFileErrInfo();
		return E_FILE_WRITE;
	}
	
	stTransCtrl.iCheckTransNum++;
	if( stTemp.iTransNo==POS_SALE    || stTemp.iTransNo==POS_PREAUTH ||
		stTemp.iTransNo==POS_AUTH_CM || stTemp.iTransNo==POS_QUE     ||
		stTemp.iTransNo==OFF_ADJUST  || stTemp.iTransNo==ADJUST_TIP  ||
		stTemp.iTransNo==OFF_SALE    || stTemp.iTransNo==POS_REFUND  ||
		stTemp.iTransNo==ICC_OFFSALE || stTemp.iTransNo==POS_OFF_CONFIRM ||
		(stTemp.iTransNo>=EC_QUICK_SALE && stTemp.iTransNo<=POS_VOID_INSTAL)||
		stTemp.iTransNo==PAYMENT_WITH_BALANCE || stTemp.iTransNo==CHECK_OUT)
	{	
		sprintf((char *)stTransCtrl.szLastCheckTransDate, "%.8s", PosCom.stTrans.szDate);
	}
	SaveCtrlParam();

	if( stTemp.iTransNo==POS_REFUND || stTemp.iTransNo==POS_OFF_CONFIRM )
	{
		SaveRefundDataFile();
	}

	return OK;
} 


uint8_t SaveLogFile(void)
{
	int		iRet, iLogFile;


	printf("<<<<<<<<<<<<<<<<SaveLogFile>>>>>>>>>>>>>>\n");
	iLogFile = fileOpen(POS_LOG_FILE, O_RDWR|O_CREAT);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return E_FILE_OPEN;
	}

	iRet = fileSeek(iLogFile, (int)(stTransCtrl.iTransNum*LOG_RECORD_LEN), SEEK_SET);
	if( iRet<0 ) 
	{
		DispFileErrInfo();
		fileClose(iLogFile);
		return E_FILE_SEEK;
	}
	
	PosCom.stTrans.uiTxnRecIndex = stTransCtrl.iTransNum;
	iRet = fileWrite(iLogFile, (uint8_t *)&PosCom.stTrans, LOG_RECORD_LEN);
	fileClose(iLogFile);
	if( iRet!=LOG_RECORD_LEN )
	{
		DispFileErrInfo();
		return E_FILE_WRITE;
	}
	
	stTransCtrl.iTransNum++;
	if( stTemp.iTransNo==POS_SALE    || stTemp.iTransNo==POS_PREAUTH ||
		stTemp.iTransNo==POS_AUTH_CM || stTemp.iTransNo==POS_QUE     ||
		stTemp.iTransNo==OFF_ADJUST  || stTemp.iTransNo==ADJUST_TIP  ||
		stTemp.iTransNo==OFF_SALE    || stTemp.iTransNo==POS_REFUND  ||
		stTemp.iTransNo==ICC_OFFSALE || stTemp.iTransNo==POS_OFF_CONFIRM ||
		(stTemp.iTransNo>=EC_QUICK_SALE && stTemp.iTransNo<=POS_VOID_INSTAL)||
		stTemp.iTransNo==PURSE_SALE)
	{	
		sprintf((char *)stTransCtrl.szLastTransDate, "%.8s", PosCom.stTrans.szDate);
	}
	SaveCtrlParam();
	return OK;
} 

uint8_t SaveRefundDataFile(void)
{
	int fd, ret;
	struct _REFUND_TRACK_STRUCT  stLog;
	
	memset(&stLog, 0, sizeof(struct _REFUND_TRACK_STRUCT));
	stLog.lTraceNo = PosCom.stTrans.lTraceNo;
	memcpy(stLog.szTrack2 ,PosCom.szTrack2, TRACK2_LEN+1);
	memcpy(stLog.szTrack3 ,PosCom.szTrack3, TRACK3_LEN+1);
	strcpy((char *)stLog.szOldDate, (char *)PosCom.stTrans.szOldTxnDate);

	fd = fileOpen(REFUND_TRACK_FILE, O_RDWR|O_CREAT);
	if( fd<0 )
	{
		DispFileErrInfo();
		return(E_FILE_OPEN);
	}
	ret = fileSeek(fd, 0, SEEK_END);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_FILE_SEEK);
	}
	ret = fileWrite(fd, (uint8_t *)&stLog, sizeof(struct _REFUND_TRACK_STRUCT));
	fileClose(fd);
	if( ret!=sizeof(struct _REFUND_TRACK_STRUCT) ) 
	{
		DispFileErrInfo();
		return(E_FILE_WRITE);
	}
	return(OK);
} 

uint8_t CheckRefundDataFile(int lTrace)
{
	int  fd, ret;
	struct _REFUND_TRACK_STRUCT  stLog;
	
	memset(&stLog, 0, sizeof(struct _REFUND_TRACK_STRUCT));

	fd = fileOpen(REFUND_TRACK_FILE, O_RDWR);
	if( fd<0 )
	{
		return OK;
	}
	ret = fileSeek(fd, 0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_FILE_SEEK);
	}
	while(1) 
	{
		ret = fileRead(fd, (uint8_t *)&stLog, sizeof(struct _REFUND_TRACK_STRUCT));
		if( ret!=sizeof(struct _REFUND_TRACK_STRUCT) ) 
		{
			fileClose(fd);
			DispFileErrInfo();
			return(E_FILE_READ);
		}
		if( stLog.lTraceNo==lTrace )
		{
			memcpy(PosCom.szTrack2, stLog.szTrack2, TRACK2_LEN+1);
			memcpy(PosCom.szTrack3, stLog.szTrack3, TRACK3_LEN+1);
			strcpy((char *)PosCom.stTrans.szOldTxnDate, (char *)stLog.szOldDate);
			break;
		}
	}
	fileClose(fd);
	return(OK);
} 


uint8_t   UpdateCheckLogFile(void)
{
	int  ret, fd;
	NEWPOS_LOG_STRC stLog;
	
	fd = fileOpen(POS_LOG_CHECK_FILE, O_RDWR);
	if( fd<0 )
	{
		return(E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)(stTemp.iLogFilePostion*LOG_RECORD_LEN), SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
   	ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
	if( ret!=LOG_RECORD_LEN ) 
	{
		fileClose(fd);
		return(E_MEM_ERR);
	}
	if( stTemp.iTransNo==ADJUST_TIP )
		stLog.ucAdjusted = TRUE;
	else if( stTemp.iTransNo==OFF_ADJUST )
	{
		if( stLog.iTransNo!=OFF_SALE ) 
		{
			fileClose(fd);
			return(OK);
		}
		if( stLog.ucSendFlag!=TRUE )
		{
			memcpy(stLog.sAmount, PosCom.stTrans.sAmount, 6);
			stLog.ucAdjusted = TRUE;
			stLog.iTransNo = OFF_SALE;
			stLog.ucChgFrmAdjust = TRUE;
			strcpy((char *)stLog.szDate, (char *)PosCom.stTrans.szDate);
			strcpy((char *)stLog.szTime, (char *)PosCom.stTrans.szTime);
		}
		else
		{
			stLog.ucAdjusted = TRUE;
		}
	}
	else if( stTemp.iTransNo == POS_AUTH_CM )
	{
		stLog.ucAuthCmlId = TRUE;  // 预授权完成标志
//		showHex("UpdateLogFile PosCom.stTrans.sAmount",PosCom.stTrans.sAmount,6);
		memcpy(stLog.sAmount, PosCom.stTrans.sAmount, 6);
		memcpy(stLog.preAuthTipAmount,PosCom.stTrans.preAuthTipAmount,sizeof(PosCom.stTrans.lPreAuthTipAmount));
		stLog.lPreAuthTipAmount = PosCom.stTrans.lPreAuthTipAmount;
	}
	else if( stTemp.iTransNo == CHECK_OUT )
	{
		stLog.ucCheckOutId = TRUE;;   // CheckOut完成标志
	}
	else 
	{
		stLog.ucVoidId = TRUE;
	}
	
	ret = fileSeek(fd, (int)(stTemp.iLogFilePostion*LOG_RECORD_LEN), SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
    ret = fileWrite(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
	if( ret!=LOG_RECORD_LEN )
	{
		fileClose(fd);
		return(E_MEM_ERR);
	}
		
	fileClose(fd);
	return(OK);
}


uint8_t   UpdateLogFile()
{
	int  ret, fd;
	NEWPOS_LOG_STRC stLog;
	
	fd = fileOpen(POS_LOG_FILE, O_RDWR);
	if( fd<0 )
	{
		return(E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)(stTemp.iLogFilePostion*LOG_RECORD_LEN), SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
   	ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
	if( ret!=LOG_RECORD_LEN ) 
	{
		fileClose(fd);
		return(E_MEM_ERR);
	}
	if( stTemp.iTransNo==NETPAY_ADJUST )
	{
		stLog.ucNetpayAdjusted = TRUE;
		memcpy(stLog.sNetpayAdjustedAmount,stLog.preTipOriAmount,6);
		memcpy(stLog.sAmount,PosCom.stTrans.sAmount,6);
		memcpy(stLog.preTipOriAmount,PosCom.stTrans.preTipOriAmount,6);

		showHex((char*)"stLog.sNetpayAdjustedAmount",stLog.sNetpayAdjustedAmount,6);
		showHex((char*)"保存金额",stLog.sAmount,6);
		showHex((char*)"preTipOriAmount",stLog.preTipOriAmount,6);
	}

	if( stTemp.iTransNo==ADJUST_TIP )
		stLog.ucAdjusted = TRUE;
	else if( stTemp.iTransNo==OFF_ADJUST )
	{
		if( stLog.iTransNo!=OFF_SALE ) 
		{
			fileClose(fd);
			return(OK);
		}
		if( stLog.ucSendFlag!=TRUE )
		{
			memcpy(stLog.sAmount, PosCom.stTrans.sAmount, 6);
			stLog.ucAdjusted = TRUE;
			stLog.iTransNo = OFF_SALE;
			stLog.ucChgFrmAdjust = TRUE;
			strcpy((char *)stLog.szDate, (char *)PosCom.stTrans.szDate);
			strcpy((char *)stLog.szTime, (char *)PosCom.stTrans.szTime);
		}
		else
		{
			stLog.ucAdjusted = TRUE;
		}
	}
	else if( stTemp.iTransNo == POS_AUTH_CM )
	{
		stLog.ucAuthCmlId = TRUE;  // 预授权完成标志
//		showHex("UpdateLogFile PosCom.stTrans.sAmount",PosCom.stTrans.sAmount,6);
		memcpy(stLog.sAmount, PosCom.stTrans.sAmount, 6);
		memcpy(stLog.preAuthTipAmount,PosCom.stTrans.preAuthTipAmount,sizeof(PosCom.stTrans.lPreAuthTipAmount));
		stLog.lPreAuthTipAmount = PosCom.stTrans.lPreAuthTipAmount;
	}
	else if( stTemp.iTransNo == CHECK_OUT )
	{
		stLog.ucCheckOutId = TRUE;;   // CheckOut完成标志
	}
	else 
	{
		stLog.ucVoidId = TRUE;
	}
	
	ret = fileSeek(fd, (int)(stTemp.iLogFilePostion*LOG_RECORD_LEN), SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
    	ret = fileWrite(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
	if( ret!=LOG_RECORD_LEN )
	{
		fileClose(fd);
		return(E_MEM_ERR);
	}
		
	fileClose(fd);
	return(OK);
}

uint8_t CheckData(int lPosStan, NEWPOS_LOG_STRC *pstOutLog)
{
	uint8_t		    ucRet;
    	int			iCnt;
	NEWPOS_LOG_STRC	stLog;
	
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( lPosStan==stLog.lTraceNo )
		{
			break;
		}
	}
	if( iCnt>=stTransCtrl.iTransNum )
	{
		return E_NO_OLD_TRANS;
	}

	if( stTemp.iTransNo==NETPAY_ADJUST )
	{
		if( stLog.ucAuthCmlId!=TRUE && stLog.iOldTransNo==POS_PREAUTH)
		{
			lcdClrLine(2, 7);	
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO SE PUEDE HACER AJUSTES");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "SIN CONFIRMAR PROPINA  ");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

			ErrorBeep();
			kbGetKeyMs(2000);
			return E_RESELECT;
		}
	}
	
	if( stLog.ucVoidId==TRUE )
	{
		return E_TRANS_VOIDED;
	}
	if( stLog.ucAdjusted==TRUE )
	{
		return E_TRANS_HAVE_ADJUESTED;		
	}
	if( stLog.ucSendFlag!=TRUE )
	{
		stTemp.bSendId = FALSE;	
	}	
	
	ucRet = DispOrignalTrans(&stLog);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	stTemp.iLogFilePostion = iCnt;
	
	strcpy((char *)PosCom.stTrans.szCardNo, (char *)stLog.szCardNo);	
	if( stLog.iOldTransNo != CHECK_OUT)
	{
		GetCardType();		//获取原交易卡片交易类型
		if( strlen((char*)stLog.szExpDate) != 0 )
		{
			strcpy((char*)PosCom.stTrans.szExpDate,(char*)stLog.szExpDate);
		}

	}
	
	strcpy((char *)PosCom.stTrans.szAuthNo, (char *)stLog.szAuthNo);
	PosCom.stTrans.iOldTransNo     = stLog.iTransNo;
	PosCom.stTrans.szMoneyType     = stLog.szMoneyType;


	PosCom.stTrans.lOldBatchNumber = stLog.lBatchNumber;
//	showHex("CheckData stLog.sAmount",stLog.sAmount,6);
	memcpy((char*)PosCom.stTrans.sAmount, (char*)stLog.sAmount, 6);
	memcpy((char*)PosCom.stTrans.preAuthOriAmount, (char*)stLog.preAuthOriAmount, 6);
	strcpy((char*)PosCom.stTrans.szAppLable,(char*)stLog.szAppLable);
	strcpy((char*)PosCom.stTrans.szAID,(char*)stLog.szAID);
	strcpy((char*)PosCom.stTrans.sIccData,(char*)stLog.sIccData);
	PosCom.stTrans.iIccDataLen = stLog.iIccDataLen;
	strcpy((char*)PosCom.stTrans.CardHolderName,(char*)stLog.CardHolderName);
	strcpy((char *)PosCom.stTrans.szCardUnit, (char *)stLog.szCardUnit);
	strcpy((char *)PosCom.stTrans.szSysReferNo, (char *)stLog.szSysReferNo);
	strcpy((char *)PosCom.stTrans.szOldTxnDate, (char *)stLog.szDate+4);
	sprintf((char *)PosCom.stTrans.szEntryMode, "%.3s", stLog.szEntryMode);
	PosCom.stTrans.uiTxnRecIndex = stLog.uiTxnRecIndex;
	PosCom.stTrans.ucAuthCmlId = stLog.ucAuthCmlId;

	// 小费交易调整时不算小费在内,保持原来的
	if( stTemp.iTransNo == NETPAY_ADJUST && stLog.TransFlag == PRE_TIP_SALE)
	{
		memcpy((char*)PosCom.stTrans.sAmount, (char*)stLog.preTipOriAmount, 6);
		memcpy((char*)PosCom.stTrans.preTipAmount, (char*)stLog.preTipAmount, 6);
		PosCom.stTrans.lpreTipAmount = stLog.lpreTipAmount;
	}

	#if 0
	if( PosCom.stTrans.iOldTransNo == POS_PREAUTH && stLog.ucAuthCmlId != TRUE)
	{
		memset(PosCom.stTrans.sAmount,0,sizeof(PosCom.stTrans.sAmount));
		memcpy(PosCom.stTrans.sAmount,stLog.preAuthOriAmount,6);
	}
	#endif


	if (stLog.iTransNo >= POS_INSTALLMENT || stLog.iTransNo <= POS_VOID_INSTAL)
	{
		memcpy(PosCom.stTrans.sTipAmount, stLog.sTipAmount, 6);
		memcpy(PosCom.stTrans.sPreAddAmount, stLog.sPreAddAmount, 6);
		memcpy(PosCom.stTrans.sLoyaltAmount, stLog.sLoyaltAmount, 6);
		PosCom.stTrans.ucInstallmentNum     = stLog.ucInstallmentNum;
		PosCom.stTrans.ucInstalPayMode = stLog.ucInstalPayMode;
		memcpy(PosCom.stTrans.szInstalCode, stLog.szInstalCode, 30);
		memcpy(PosCom.stTrans.szInstalCurrCode,stLog.szInstalCurrCode, 3);
	}

	if( stTemp.iTransNo==POS_AUTH_VOID && stLog.iTransNo==POS_AUTH_CM )
	{
		strcpy((char *)PosCom.stTrans.szAuthNo, (char *)stLog.szPrtOldAuthNo);
		strcpy((char *)PosCom.stTrans.szPrtOldAuthNo, (char *)stLog.szPrtOldAuthNo);
	}

	if (stTemp.iTransNo == EC_VOID_TOPUP)
	{
		PosCom.stTrans.iIccDataLen = stLog.iIccDataLen;
		memcpy(PosCom.stTrans.sIccData,stLog.sIccData,PosCom.stTrans.iIccDataLen);
	}

	if( stTemp.iTransNo==OFF_ADJUST ) 
	{
		strcpy((char *)PosCom.stTrans.szAuthMode, (char *)stLog.szAuthMode);
		strcpy((char *)PosCom.stTrans.szAuthUnit, (char *)stLog.szAuthUnit);
		strcpy((char *)PosCom.stTrans.szExpDate,  (char *)stLog.szExpDate);
		if( stLog.iTransNo==POS_SALE ) 
		{
			strcpy((char *)PosCom.stTrans.szAuthMode, "00");
			strcpy((char *)PosCom.stTrans.szCardUnit, (char *)stLog.szCardUnit);
		}

		strcpy((char *)PosCom.stTrans.szIssuerResp,(char *)stLog.szIssuerResp);
		strcpy((char *)PosCom.stTrans.szCenterResp,(char *)stLog.szCenterResp);
		strcpy((char *)PosCom.stTrans.szRecvBankResp,(char *)stLog.szRecvBankResp);
	}

	if( pstOutLog!=NULL )
	{
		memcpy(pstOutLog, &stLog, sizeof(NEWPOS_LOG_STRC));
	}

	return(OK);	
}



uint8_t CheckOutCheckData(int lPosStan, NEWPOS_LOG_STRC *pstOutLog)
{
	uint8_t		    ucRet;
    int			iCnt;
	NEWPOS_LOG_STRC	stLog;
	uint8_t ReinputAmountFalg = 0;
	
	if( stTransCtrl.iCheckTransNum==0 )
	{
		return E_NO_TRANS;
	}

	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( lPosStan==stLog.lTraceNo )
		{
			if(	stLog.iTransNo!= CHECK_IN )
			{
									//保留，等客户需要判断非check in 的时候再增加上去
			}
			break;
		}
	}
	if( iCnt>=stTransCtrl.iCheckTransNum )
	{
		return E_NO_OLD_TRANS;
	}
	if( stLog.ucCheckOutId==TRUE )
	{
		return E_CHECK_OUT_FINISH;
	}
	if( stLog.ucSendFlag!=TRUE )
	{
		stTemp.bSendId = FALSE;	
	}

REINPUT_AMOUNT:
	ucRet = CheckOutDispOrignalTrans(&stLog,ReinputAmountFalg,PosCom.stTrans.sAmount);
	if( ucRet == E_REINPUT_AMOUNT )
	{	
		ucRet = AppGetAmount(9, TRAN_AMOUNT);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		ReinputAmountFalg = 1;
		goto REINPUT_AMOUNT;
	}
	else if( ucRet!=OK )
	{
		return ucRet;
	}

	stTemp.iLogFilePostion = iCnt;
	
	strcpy((char *)PosCom.stTrans.szCardNo, (char *)stLog.szCardNo);	
	strcpy((char *)PosCom.stTrans.szCheckOutCardNo, (char *)stLog.szCardNo);	
	
	strcpy((char *)PosCom.stTrans.szAuthNo, (char *)stLog.szAuthNo);
	PosCom.stTrans.iOldTransNo     = stLog.iTransNo;
	PosCom.stTrans.lOldBatchNumber = stLog.lBatchNumber;
	memcpy(PosCom.stTrans.sAmount, stLog.sAmount, 6);
	
	memcpy(PosCom.stTrans.preAuthOriAmount, stLog.preAuthOriAmount, 6);
	
	strcpy((char *)PosCom.stTrans.szCardUnit, (char *)stLog.szCardUnit);
	strcpy((char *)PosCom.stTrans.szSysReferNo, (char *)stLog.szSysReferNo);
	strcpy((char *)PosCom.stTrans.szOldTxnDate, (char *)stLog.szDate+2);
	strcpy((char *)PosCom.stTrans.szOldTxnTime, (char *)stLog.szTime);
	sprintf((char *)PosCom.stTrans.szEntryMode, "%.3s", stLog.szEntryMode);
	PosCom.stTrans.uiTxnRecIndex = stLog.uiTxnRecIndex;

	if(stLog.iTransNo==POS_AUTH_CM )
	{
		strcpy((char *)PosCom.stTrans.szAuthNo, (char *)stLog.szPrtOldAuthNo);
		strcpy((char *)PosCom.stTrans.szPrtOldAuthNo, (char *)stLog.szPrtOldAuthNo);
	}

	if( pstOutLog!=NULL )
	{
		memcpy(pstOutLog, &stLog, sizeof(NEWPOS_LOG_STRC));
	}

	return(OK);	
}



uint8_t AuthConfirmCheckData(int lPosStan, NEWPOS_LOG_STRC *pstOutLog)
{
	uint8_t		    ucRet;
    int			iCnt;
	NEWPOS_LOG_STRC	stLog;
	
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( lPosStan==stLog.lTraceNo )
		{
			break;
		}
	}
	if( iCnt>=stTransCtrl.iTransNum )
	{
		return E_NO_OLD_TRANS;
	}
	if( stLog.ucAuthCmlId==TRUE )
	{
		return E_TRANS_VOIDED;
	}
	if( stLog.ucSendFlag!=TRUE )
	{
		stTemp.bSendId = FALSE;	
	}

	ucRet = DispOrignalTrans(&stLog);
	if( ucRet!=OK )
	{
		return ucRet;
	}
	stTemp.iLogFilePostion = iCnt;
	
	strcpy((char *)PosCom.stTrans.szCardNo, (char *)stLog.szCardNo);	
	strcpy((char *)PosCom.stTrans.szAuthNo, (char *)stLog.szAuthNo);
	PosCom.stTrans.iOldTransNo     = stLog.iTransNo;
	PosCom.stTrans.lOldBatchNumber = stLog.lBatchNumber;
//	memcpy(PosCom.stTrans.sAmount, stLog.sAmount, 6);
	
	memcpy(PosCom.stTrans.preAuthOriAmount, stLog.preAuthOriAmount, 6);
	
	strcpy((char *)PosCom.stTrans.szCardUnit, (char *)stLog.szCardUnit);
	strcpy((char *)PosCom.stTrans.szSysReferNo, (char *)stLog.szSysReferNo);
	strcpy((char *)PosCom.stTrans.szOldTxnDate, (char *)stLog.szDate+2);
	strcpy((char *)PosCom.stTrans.szOldTxnTime, (char *)stLog.szTime);
	sprintf((char *)PosCom.stTrans.szEntryMode, "%.3s", stLog.szEntryMode);
	PosCom.stTrans.uiTxnRecIndex = stLog.uiTxnRecIndex;

	if(stLog.iTransNo==POS_AUTH_CM )
	{
		strcpy((char *)PosCom.stTrans.szAuthNo, (char *)stLog.szPrtOldAuthNo);
		strcpy((char *)PosCom.stTrans.szPrtOldAuthNo, (char *)stLog.szPrtOldAuthNo);
	}

	if( pstOutLog!=NULL )
	{
		memcpy(pstOutLog, &stLog, sizeof(NEWPOS_LOG_STRC));
	}

	return(OK);	
}


uint8_t ComputeTransTotal(int flag)
{
	int  i, fd, ret;
	NEWPOS_LOG_STRC stLog;
	
	memset(&stTotal, 0, TOTAL_STRC_SIZE);

	if( stTransCtrl.iTransNum==0 )
		return(OK);

	fd = fileOpen(POS_LOG_FILE, O_RDWR);
	if( fd<0 ) 
		return(E_MEM_ERR);
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 )
	{
		fileClose(fd);
		return(E_MEM_ERR);
	}
	
	for(i=0; i<stTransCtrl.iTransNum; i++)
	{
		if( fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN)!=LOG_RECORD_LEN )
		{
			fileClose(fd);	
			return(E_MEM_ERR);
		}
		
		if(ChkIfDebitTxn(stLog.iTransNo) || 
		  ((stLog.iTransNo==POS_SALE || stLog.iTransNo==ICC_OFFSALE) && stLog.ucAdjusted!=TRUE )) 
		{
			if( stLog.iTransNo==OFF_SALE && stLog.ucAdjusted==TRUE && stLog.ucChgFrmAdjust!=TRUE )
				continue;
			
			if( (memcmp(stLog.szCardUnit, "CUP", 3)==0) || (stLog.iTransNo==ICC_OFFSALE) ) 
			{
				stTotal.iRmbDebitNum ++;
				BcdAdd(stTotal.sRmbDebitAmt, stLog.sAmount, 6);

				if (stLog.iTransNo == EC_QUICK_SALE || stLog.iTransNo == EC_NORMAL_SALE)
				{
					stTotal.iRmbEcNum++;
					BcdAdd(stTotal.sRmbEcAmt, stLog.sAmount, 6);
				}
			} 
			else 
			{	
				stTotal.iFrnDebitNum ++;
				BcdAdd(stTotal.sFrnDebitAmt, stLog.sAmount, 6);
				
				if( stLog.iTransNo==ADJUST_TIP )
					BcdAdd(stTotal.sFrnDebitAmt, stLog.sTipAmount, 6);

				if (stLog.iTransNo == EC_QUICK_SALE || stLog.iTransNo == EC_NORMAL_SALE)
				{
					stTotal.iFrnEcNum++;
					BcdAdd(stTotal.sFrnEcAmt, stLog.sAmount, 6);
				}
			}
		}
		if(ChkIfCreditTxn(stLog.iTransNo))
		{
			if( (memcmp(stLog.szCardUnit, "CUP", 3)==0) || ((stLog.iTransNo==POS_REFUND)&&(memcmp(stLog.szCardUnit, "000", 3)==0)) ) 
			{
				stTotal.iRmbCreditNum ++;
				BcdAdd(stTotal.sRmbCreditAmt, stLog.sAmount, 6);
			}
			else 
			{
				stTotal.iFrnCreditNum++;
				BcdAdd(stTotal.sFrnCreditAmt, stLog.sAmount, 6);
			}

			if (stLog.iTransNo== EC_TOPUP_CASH )
			{
				stTotal.iTopUpNum++;
				BcdAdd(stTotal.sTopUpAmt, stLog.sAmount, 6);
			}
		}
		
		if(ChkIfSaleTxn(stLog.iTransNo))
		{
			if( stLog.ucVoidId==TRUE || stLog.ucAdjusted==TRUE )
				continue;
			if( (memcmp(stLog.szCardUnit, "CUP", 3)==0) || (stLog.iTransNo==ICC_OFFSALE) ) 
			{
				stTotal.iRmbSaleNum++;
				BcdAdd(stTotal.sRmbSaleAmt, stLog.sAmount, 6);
			}
			else 
			{
				stTotal.iFrnSaleNum++;
				BcdAdd(stTotal.sFrnSaleAmt, stLog.sAmount, 6);
				BcdAdd(stTotal.sFrnSaleAmt, stLog.sTipAmount, 6);
			}
		}
		if( stLog.iTransNo==POS_AUTH_CM )
		{
			if( stLog.ucVoidId==TRUE )
				continue;
			if( memcmp(stLog.szCardUnit, "CUP", 3)==0 ) 
			{	
				stTotal.iRmbConfirmNum++;
				BcdAdd(stTotal.sRmbConfirmAmt, stLog.sAmount, 6);
			} 
			else 
			{	
				stTotal.iFrnConfirmNum++;
				BcdAdd(stTotal.sFrnConfirmAmt, stLog.sAmount, 6);
			}
		}
		if( stLog.iTransNo==POS_OFF_CONFIRM )
		{
			if( stLog.ucVoidId==TRUE )
				continue;
			if( memcmp(stLog.szCardUnit, "CUP", 3)==0 ) 
			{	
				stTotal.iRmbOffConfirmNum++;
				BcdAdd(stTotal.sRmbOffConfirmAmt, stLog.sAmount, 6);
			} 
			else 
			{	
				stTotal.iFrnOffConfirmNum++;
				BcdAdd(stTotal.sFrnOffConfirmAmt, stLog.sAmount, 6);
			}
		}
		if( stLog.iTransNo==POS_REFUND || stLog.iTransNo==EC_REFUND)
		{
			if( (memcmp(stLog.szCardUnit, "CUP", 3)==0) || (memcmp(stLog.szCardUnit, "000", 3)==0) ) 
			{
				stTotal.iRmbRefundNum++;
				BcdAdd(stTotal.sRmbRefundAmt, stLog.sAmount, 6);
			}
			else 
			{
				stTotal.iFrnRefundNum++;
				BcdAdd(stTotal.sFrnRefundAmt, stLog.sAmount, 6);
			}
		}
		if( stLog.iTransNo==OFF_SALE || stLog.iTransNo==OFF_ADJUST )
		{
			if( stLog.ucVoidId==TRUE )
				continue;
			if( (stLog.ucAdjusted==TRUE) && (stLog.ucChgFrmAdjust!=TRUE) )
				continue;
			if( memcmp(stLog.szCardUnit, "CUP", 3)==0 ) 
			{	
				stTotal.iRmbOffNum++;
				BcdAdd(stTotal.sRmbOffAmt, stLog.sAmount, 6);
			} 
			else 
			{	
				stTotal.iFrnOffNum++;
				BcdAdd(stTotal.sFrnOffAmt, stLog.sAmount, 6);
				if( stLog.iTransNo==ADJUST_TIP ) 
					BcdAdd(stTotal.sFrnOffAmt, stLog.sTipAmount, 6);
			}			
		}
	}
	
	fileClose(fd);
	return(OK);
}

void update_limit_total()
{			
	if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==ICC_OFFSALE || stTemp.iTransNo==POS_AUTH_CM || 
		stTemp.iTransNo==POS_OFF_CONFIRM || stTemp.iTransNo==OFF_SALE || stTemp.iTransNo==OFF_ADJUST 
		|| stTemp.iTransNo==PURSE_SALE) 
	{ 
		if (memcmp(PosCom.stTrans.szCardUnit, "CUP", 3)==0 ) 
		{	
			BcdAdd(stLimitTotal.sRmbDebitAmt, PosCom.stTrans.sAmount, 6);
		} 
		else 
		{	
			BcdAdd(stLimitTotal.sFrnDebitAmt, PosCom.stTrans.sAmount , 6);
			if( stTemp.iTransNo==ADJUST_TIP ) 
				BcdAdd(stLimitTotal.sFrnDebitAmt, PosCom.stTrans.sTipAmount, 6);  
		}
	}
	if( stTemp.iTransNo==POS_SALE_VOID || stTemp.iTransNo==POS_REFUND || stTemp.iTransNo==POS_AUTH_VOID ) 
	{	
		if( memcmp(PosCom.stTrans.szCardUnit, "CUP", 3)==0 ) 
		{	
			BcdAdd(stLimitTotal.sRmbCreditAmt, PosCom.stTrans.sAmount, 6);
		} 
		else 
		{	
			BcdAdd(stLimitTotal.sFrnCreditAmt, PosCom.stTrans.sAmount, 6);
		}
	}
	SaveLimitTotal();
	return;
}


uint8_t CheckFindTheLastSale(NEWPOS_LOG_STRC *stLog)
{
	int fd,i,ret;
	
	fd = fileOpen(POS_LOG_CHECK_FILE, O_RDWR);
	if( fd<0 )
	{
		return(E_MEM_ERR);
	}
	for(i=1; i<stTransCtrl.iCheckTransNum+1; i++)
	{
		ret = fileSeek(fd, (int)((stTransCtrl.iCheckTransNum-i)*LOG_RECORD_LEN), SEEK_SET);
		if( ret<0 ) 
		{
			fileClose(fd);
			return (E_MEM_ERR);
		}
		ret = fileRead(fd, (uint8_t *)stLog, LOG_RECORD_LEN);
		if( ret<0 ) 
		{
			fileClose(fd);
			return(E_MEM_ERR);
		}
		if(stLog->iTransNo == CHECK_IN || stLog->iTransNo == CHECK_OUT )
			break;		
	}
	return OK;
}


uint8_t FindTheLastSale(NEWPOS_LOG_STRC *stLog)
{
	int fd,i,ret;
	
	fd = fileOpen(POS_LOG_FILE, O_RDWR);
	if( fd<0 )
	{
		return(E_MEM_ERR);
	}
	for(i=1; i<stTransCtrl.iTransNum+1; i++)
	{
		ret = fileSeek(fd, (int)((stTransCtrl.iTransNum-i)*LOG_RECORD_LEN), SEEK_SET);
		if( ret<0 ) 
		{
			fileClose(fd);
			return (E_MEM_ERR);
		}
		ret = fileRead(fd, (uint8_t *)stLog, LOG_RECORD_LEN);
		if( ret<0 ) 
		{
			fileClose(fd);
			return(E_MEM_ERR);
		}
		if( stLog->iTransNo == POS_SALE || 
			stLog->iTransNo == PRE_TIP_SALE ||
			stLog->iTransNo == INTERESES_SALE || 
			stLog->iTransNo == POS_PREAUTH ||
			stLog->iTransNo == CHECK_IN ||
			stLog->iTransNo == CHECK_OUT ||
			stLog->iTransNo == NETPAY_FORZADA)
		{
			if( stPosParam.Switch_Hotel==PARAM_OPEN )
			{
				if( PosCom.stTrans.iSelectOldTransNo == CHECK_IN )
				{
					if(stLog->iTransNo == CHECK_IN)
						break;
				}
				else if( PosCom.stTrans.iSelectOldTransNo == CHECK_OUT)
				{
					if(stLog->iTransNo == CHECK_OUT)
						break;
				}
				else if( PosCom.stTrans.iSelectOldTransNo == POS_SALE )
				{
					if( stLog->iTransNo == POS_SALE || 
						stLog->iTransNo == PRE_TIP_SALE ||
						stLog->iTransNo == INTERESES_SALE || 
						stLog->iTransNo == POS_PREAUTH ||
						stLog->iTransNo == NETPAY_FORZADA)
						break;
				}

				if( i==stTransCtrl.iTransNum/*+1*/ ) 
				{
					lcdClrLine(2, 7);	
					DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "UN MOMENTO");
					DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "BATCH VACIO");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);
					return NO_DISP;
				}
			}
			else
			{
				if( stLog->iTransNo == POS_SALE || 
					stLog->iTransNo == PRE_TIP_SALE ||
					stLog->iTransNo == INTERESES_SALE || 
					stLog->iTransNo == POS_PREAUTH ||
					stLog->iTransNo == NETPAY_FORZADA)
					break;
			}
				
		}

		
					
	}
	
	return OK;
}


uint8_t reprintCheckTrans(uint8_t prt_bz,int lTraceNo)
{
//	uint8_t buf[17];
	int i, ret, fd;
//	int /*lTraceNo,*/ iRet;
	NEWPOS_LOG_STRC stLog;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	if( stTransCtrl.iCheckTransNum==0 ) 
       	return(E_NO_TRANS);
	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	lcdFlip();
	
    if( prt_bz==0 )
	{	
        fd = fileOpen(POS_LOG_CHECK_FILE, O_RDWR);
		if( fd<0 )
		{
			return(E_MEM_ERR);
		}
		for(i=1; i<stTransCtrl.iCheckTransNum+1; i++)
		{
			ret = fileSeek(fd, (int)((stTransCtrl.iCheckTransNum-i)*LOG_RECORD_LEN), SEEK_SET);
			if( ret<0 ) 
			{
				fileClose(fd);
				return (E_MEM_ERR);
			}
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret<0 ) 
			{
				fileClose(fd);
				return(E_MEM_ERR);
			}
			PosCom.stTrans.TransFlag = stLog.iTransNo;
			if(1)
				break;
		}
		fileClose(fd);
		if( i==stTransCtrl.iCheckTransNum+1 ) 
			return(E_NO_TRANS);
	}
	else 
	{
		#if 0
		lcdClrLine(2, 7);	
		lcdDisplay(0, 2, DISP_CFONT, "NUMERO DE CARGO");
		lcdGoto(72, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 0, 6, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return E_TRANS_CANCEL;
		
		if( iRet == 0 || memcmp(buf,"000000",6) == 0 )
		{
			reprintTrans(0);
			return NO_DISP;
		}
		#endif

//		lTraceNo = atol((char *)buf);		
		fd = fileOpen(POS_LOG_CHECK_FILE, O_RDWR);
		if( fd<0 ) 
		{
			return(E_MEM_ERR);
		}
		ret = fileSeek(fd, (int)0, SEEK_SET);
		if( ret<0 ) 
		{
			fileClose(fd);
			return (E_MEM_ERR);
		}
		
        for(i=0; i<stTransCtrl.iCheckTransNum; i++)
		{
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret!=LOG_RECORD_LEN ) 
			{
				i = stTransCtrl.iCheckTransNum;
				break;
			}
			if( stLog.lTraceNo==lTraceNo ) 
			{
				PosCom.stTrans.TransFlag = stLog.iTransNo;
				break;
			}				   
		}
		fileClose(fd);
		if( i==stTransCtrl.iCheckTransNum ) 
			return(E_NO_OLD_TRANS);
	}

	lcdClrLine(2, 7);	
	memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
//	PrtTranTicket(REPRINT);
	NetpayPrtTranTicket(REPRINT);
	return NO_DISP;
	
}




uint8_t CheckOutreprintTrans(uint8_t prt_bz)
{
	uint8_t buf[17];
	int i, ret, fd;
	int lTraceNo, iRet;
	NEWPOS_LOG_STRC stLog;
	uint8_t ucRet = 0;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	if( stTransCtrl.iTransNum==0 ) 
       	return(E_NO_TRANS);
	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	lcdFlip();
	
    if( prt_bz==0 )
	{	
        fd = fileOpen(POS_LOG_FILE, O_RDWR);
		if( fd<0 )
		{
			return(E_MEM_ERR);
		}
		for(i=1; i<stTransCtrl.iTransNum+1; i++)
		{
			ret = fileSeek(fd, (int)((stTransCtrl.iTransNum-i)*LOG_RECORD_LEN), SEEK_SET);
			if( ret<0 ) 
			{
				fileClose(fd);
				return (E_MEM_ERR);
			}
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret<0 ) 
			{
				fileClose(fd);
				return(E_MEM_ERR);
			}
			if( stLog.iTransNo!=CHECK_OUT )
			{
				continue;
			}
			PosCom.stTrans.TransFlag = stLog.iTransNo;
			if(1)
				break;
		}
		
		fileClose(fd);
		if( i==stTransCtrl.iTransNum+1 ) 
			return(E_NO_TRANS);
	}
	else 
	{
		lcdClrLine(2, 7);	
		lcdDisplay(0, 2, DISP_CFONT, "NUMERO DE CARGO");
		lcdGoto(72, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 0, 6, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return E_TRANS_CANCEL;;
		if( iRet == 0 || memcmp(buf,"000000",6) == 0 )
		{
			ucRet = CheckOutreprintTrans(0);
			return OK; 
		}

		lTraceNo = atol((char *)buf);		
		fd = fileOpen(POS_LOG_FILE, O_RDWR);
		if( fd<0 ) 
		{
			return(E_MEM_ERR);
		}
		ret = fileSeek(fd, (int)0, SEEK_SET);
		if( ret<0 ) 
		{
			fileClose(fd);
			return (E_MEM_ERR);
		}

        for(i=0; i<stTransCtrl.iTransNum; i++)
		{
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret!=LOG_RECORD_LEN ) 
			{
				i = stTransCtrl.iTransNum;
				break;
			}
			if( stLog.lTraceNo==lTraceNo ) 
			{
				PosCom.stTrans.TransFlag = stLog.iTransNo;
				break;
			}				   
		}
		if( stLog.iTransNo!=CHECK_IN )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "BATCH VACIO");
			lcdFlip();
			ErrorBeep();
			fileClose(fd);
			kbGetKeyMs(2000);
		}
		fileClose(fd);
		if( i==stTransCtrl.iTransNum ) 
			return(E_NO_OLD_TRANS);
	}

	lcdClrLine(2, 7);	
	memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
//	PrtTranTicket(REPRINT);
	NetpayPrtTranTicket(REPRINT);
	return OK;
	
}


uint8_t CheckInreprintTrans(uint8_t prt_bz)
{
	uint8_t buf[17];
	int i, ret, fd;
	int lTraceNo, iRet;
	NEWPOS_LOG_STRC stLog;
	uint8_t ucRet = 0;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	if( stTransCtrl.iTransNum==0 ) 
       	return(E_NO_TRANS);
	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	lcdFlip();
	
    if( prt_bz==0 )
	{	
        fd = fileOpen(POS_LOG_FILE, O_RDWR);
		if( fd<0 )
		{
			return(E_MEM_ERR);
		}
		for(i=1; i<stTransCtrl.iTransNum+1; i++)
		{
			ret = fileSeek(fd, (int)((stTransCtrl.iTransNum-i)*LOG_RECORD_LEN), SEEK_SET);
			if( ret<0 ) 
			{
				fileClose(fd);
				return (E_MEM_ERR);
			}
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret<0 ) 
			{
				fileClose(fd);
				return(E_MEM_ERR);
			}
			if( stLog.iTransNo!=CHECK_IN )
			{
				continue;
			}
			PosCom.stTrans.TransFlag = stLog.iTransNo;
			if(1)
				break;
		}
		
		fileClose(fd);
		if( i==stTransCtrl.iTransNum+1 ) 
			return(E_NO_TRANS);
	}
	else 
	{
		lcdClrLine(2, 7);	
		lcdDisplay(0, 2, DISP_CFONT, "NUMERO DE CARGO");
		lcdGoto(72, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 0, 6, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return E_TRANS_CANCEL;;
		if( iRet == 0 || memcmp(buf,"000000",6) == 0 )
		{
			ucRet = CheckInreprintTrans(0);
			return OK; 
		}

		lTraceNo = atol((char *)buf);		
		fd = fileOpen(POS_LOG_FILE, O_RDWR);
		if( fd<0 ) 
		{
			return(E_MEM_ERR);
		}
		ret = fileSeek(fd, (int)0, SEEK_SET);
		if( ret<0 ) 
		{
			fileClose(fd);
			return (E_MEM_ERR);
		}

        for(i=0; i<stTransCtrl.iTransNum; i++)
		{
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret!=LOG_RECORD_LEN ) 
			{
				i = stTransCtrl.iTransNum;
				break;
			}
			if( stLog.lTraceNo==lTraceNo ) 
			{
				PosCom.stTrans.TransFlag = stLog.iTransNo;
				break;
			}				   
		}
		if( stLog.iTransNo!=CHECK_IN )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "BATCH VACIO");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(2000);
			fileClose(fd);
			return NO_DISP;
		}
		fileClose(fd);
		if( i==stTransCtrl.iTransNum ) 
			return(E_NO_OLD_TRANS);
	}

	lcdClrLine(2, 7);	
	memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
//	PrtTranTicket(REPRINT);
	NetpayPrtTranTicket(REPRINT);
	return OK;
	
}


//打印任意一笔
uint8_t reprintTrans(uint8_t prt_bz)
{
	uint8_t buf[17];
	int i, ret, fd;
	int lTraceNo, iRet;
	NEWPOS_LOG_STRC stLog;
	uint8_t ucRet = 0;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	if( stTransCtrl.iTransNum==0 ) 
		return(E_NO_TRANS);

	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    REIMPRESION     ");
	lcdFlip();

	if( prt_bz==0 )
	{	
		fd = fileOpen(POS_LOG_FILE, O_RDWR);
		if( fd<0 )
		{
			return(E_MEM_ERR);
		}
		for(i=1; i<stTransCtrl.iTransNum+1; i++)
		{
			ret = fileSeek(fd, (int)((stTransCtrl.iTransNum-i)*LOG_RECORD_LEN), SEEK_SET);
			if( ret<0 ) 
			{
				fileClose(fd);
				return (E_MEM_ERR);
			}
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret<0 ) 
			{
				fileClose(fd);
				return(E_MEM_ERR);
			}
			PosCom.stTrans.TransFlag = stLog.iTransNo;
			if(1)
				break;
		}

		fileClose(fd);
		if( i==stTransCtrl.iTransNum+1 ) 
			return(E_NO_TRANS);
	}
	else 
	{
		lcdClrLine(2, 7);	
		lcdDisplay(0, 2, DISP_CFONT, "NUMERO DE CARGO");
		lcdGoto(72, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 0, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return E_TRANS_CANCEL;;
		if( iRet == 0 || memcmp(buf,"000000",6) == 0 )
		{
			ucRet = reprintTrans(0);
			return OK; 
		}

		lTraceNo = atol((char *)buf);		
		fd = fileOpen(POS_LOG_FILE, O_RDWR);
		if( fd<0 ) 
		{
			return(E_MEM_ERR);
		}
		ret = fileSeek(fd, (int)0, SEEK_SET);
		if( ret<0 ) 
		{
			fileClose(fd);
			return (E_MEM_ERR);
		}

		for(i=0; i<stTransCtrl.iTransNum; i++)
		{
			ret = fileRead(fd, (uint8_t *)&stLog, LOG_RECORD_LEN);
			if( ret!=LOG_RECORD_LEN ) 
			{
				i = stTransCtrl.iTransNum;
				break;
			}
			if( stLog.lTraceNo==lTraceNo ) 
			{
				PosCom.stTrans.TransFlag = stLog.iTransNo;
				break;
			}				   
		}
		fileClose(fd);
		if( i==stTransCtrl.iTransNum ) 
			return(E_NO_OLD_TRANS);
	}

	lcdClrLine(2, 7);	
	memcpy(&PosCom.stTrans, &stLog, sizeof(stLog));
	//	PrtTranTicket(REPRINT);
	NetpayPrtTranTicket(REPRINT);
	return OK;

}

#if 0
uint8_t PrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
	char    buf1[200];
	int     i;
	uint8_t printDate[30];
	int     iKey;
	int     printNum = 1;

Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 && stTransCtrl.iCheckTransNum==0)
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
//	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",printDate, 
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

	memset(buf1,0,sizeof(buf1));
	GetTransPrtName(PosCom.stTrans.iTransNo, buf1);    
	prnPrintf("TIPO DE TRANSACCION: \n");
	prnPrintf("               %s\n",buf1);

	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		ucRet = CalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}
	
	prnPrintf("CIERRE DE LOTE OK\n");
	prnPrintf("AB      $          %ld.%02ld\n\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Total    ");

	for( i = 0; i < 8; i++){
		if( i == 6 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}

	prnPrintf("\nTOTAL CONSUMOS   $  %ld.%02ld\n",AllTransTable[7].lTotalTransAmount/100,
		AllTransTable[7].lTotalTransAmount%100);
	prnPrintf("TOTAL           $  %ld.%02ld\n",AllTransTable[7].lTotalTransAmount/100,
		AllTransTable[7].lTotalTransAmount%100);

	prnPrintf("\n              %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	lcdClrLine(2,7);
	lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "CIERRE DE LOTE. APROBADO");
	lcdFlip();
	usleep(2*1000);

	if( printNum == 1 )
	{
		printNum = 0;
		
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "DESEA UNA COPIA DE CIERRE?");
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL || iKey == KEY2) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			goto Print;
		}
	}
	
	
	return OK;
}
#endif



uint8_t HotelPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
	char    buf1[200];
	int     i;
	uint8_t printDate[30];
	int     iKey;
	int     printNum = 1;
	
Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
//	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",printDate, 
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

	memset(buf1,0,sizeof(buf1));
	GetTransPrtName(PosCom.stTrans.iTransNo, buf1);    
	prnPrintf("TIPO DE TRANSACCION: \n");
	prnPrintf("               %s\n",buf1);

	/* **********************GRAN USD************************** 
	*************************GRAN USD*************************/
	
	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_PESEO )
		{
			continue;
		}
		if( stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
		{
			continue;
		}
		ucRet = HotelSettleCalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}

	prnPrintf("CIERRE DE LOTE OK\n");
	prnPrintf("AB      $          %ld.%02ld\n\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL MXN\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"S/Inte   ");
	strcpy(AllTransTable[2].szTransName,"Diferidos");
	strcpy(AllTransTable[3].szTransName,"Cancel   ");
	strcpy(AllTransTable[4].szTransName,"Devol    ");
	strcpy(AllTransTable[5].szTransName,"Descuento");
	strcpy(AllTransTable[6].szTransName,"Total    ");

	for( i = 0; i < 7; i++){
		if( i == 5 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}

	/* **********************USD************************** 
	*************************USD*************************/

	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_DOLARES )
		{
			continue;
		}
		if( stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
		{
			continue;
		}
		ucRet = HotelSettleCalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}



	PrnBlackEngData();
	prnPrintf("\nGRAN TOTAL USD\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"S/Inte   ");
	strcpy(AllTransTable[2].szTransName,"Diferidos");
	strcpy(AllTransTable[3].szTransName,"Cancel   ");
	strcpy(AllTransTable[4].szTransName,"Devol    ");
	strcpy(AllTransTable[5].szTransName,"Descuento");
	strcpy(AllTransTable[6].szTransName,"Total    ");
	

	for( i = 0; i < 7; i++){
		if( i == 5 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}

	prnPrintf("\nTOTAL CONSUMOS   $  %ld.%02ld\n",/*AllTransTable[7].lTotalTransAmount/100*/0,
		/*AllTransTable[7].lTotalTransAmount%100*/0);
	prnPrintf("TOTAL           $  %ld.%02ld\n",/*AllTransTable[6].lTotalTransAmount/100*/0,
		/*AllTransTable[6].lTotalTransAmount%100*/0);


							/* 1 */
	/* **********************CHECK IN MXN************************** 
	*************************CHECK IN MXN*************************/
	
	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_PESEO )
		{
			continue;
		}
		if( stLog.iTransNo!=CHECK_IN && stLog.iTransNo!=POS_REFUND)
		{
			continue;
		}
		if( stLog.iTransNo==POS_REFUND )
		{
			if( stLog.iOldTransNo==CHECK_IN)
			{
				continue;
			}
		}
		ucRet = HotelCheckInSettlePrtOneTranTxn(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}


	PrnBlackEngData();
	prnPrintf("\nCHECK IN MXN\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Total    ");

	for( i = 0; i < 3; i++)
	{
		prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
			AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
			AllTransTable[i].lTotalTransAmount%100);	
	}


								/* 2 */
	/* **********************CHECK IN USD************************** 
	*************************CHECK IN USD*************************/

	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_DOLARES )
		{
			continue;
		}
		if( stLog.iTransNo!=CHECK_IN && stLog.iTransNo!=POS_REFUND)
		{
			continue;
		}
		if( stLog.iTransNo==POS_REFUND )
		{
			if( stLog.iOldTransNo==CHECK_IN)
			{
				continue;
			}
		}
		ucRet = HotelCheckInSettlePrtOneTranTxn(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}


	PrnBlackEngData();
	prnPrintf("\nCHECK IN USD\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Total    ");
	

	for( i = 0; i < 3; i++)
	{
		prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
			AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
			AllTransTable[i].lTotalTransAmount%100);
	}


								/* 3 */
	/* **********************CHECK OUT MXN************************** 
	*************************CHECK OUT MXN*************************/
	
	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_PESEO )
		{
			continue;
		}
		if( stLog.iTransNo!=CHECK_OUT &&  
			stLog.iTransNo!=POS_REFUND &&
			stLog.iTransNo!=NETPAY_REFUND )
		{
			continue;
		}

		if( stLog.iTransNo==POS_REFUND ||
			stLog.iTransNo==NETPAY_REFUND )
		{
			if( stLog.iOldTransNo!=CHECK_OUT )
			{	
				continue;
			}
			
		}
		ucRet = HotelCheckOutSettlePrtOneTranTxn(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}


	PrnBlackEngData();
	prnPrintf("\nCHECK OUT MXN\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); 
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Devol.   ");
	strcpy(AllTransTable[3].szTransName,"Total    ");

	for( i = 0; i < 4; i++)
	{
		prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
			AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
			AllTransTable[i].lTotalTransAmount%100);
	}


								/* 4 */
	/* **********************CHECK OUT USD************************** 
	*************************CHECK OUT USD*************************/

	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_DOLARES )
		{
			continue;
		}
		if( stLog.iTransNo!=CHECK_OUT &&  
			stLog.iTransNo!=POS_REFUND &&
			stLog.iTransNo!=NETPAY_REFUND )
		{
			continue;
		}

		if( stLog.iTransNo==POS_REFUND ||
			stLog.iTransNo==NETPAY_REFUND )
		{
			if( stLog.iOldTransNo!=CHECK_OUT )
			{	
				continue;
			}
			
		}
		ucRet = HotelCheckOutSettlePrtOneTranTxn(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}



	PrnBlackEngData();
	prnPrintf("\nCHECK OUT USD\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); 
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Devol.   ");
	strcpy(AllTransTable[3].szTransName,"Total    ");
	

	for( i = 0; i < 4; i++)
	{
		prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
			AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
			AllTransTable[i].lTotalTransAmount%100);
	}


							/* end */
	/* **********************88888************************** 
	************************8888888*************************/

	
		
	prnPrintf("\n              %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	lcdClrLine(2,7);
	lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "CIERRE DE LOTE. APROBADO");
	lcdFlip();
	usleep(2*1000);

	if( printNum == 1 )
	{
		printNum = 0;
		
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "DESEA UNA COPIA DE CIERRE?");
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL || iKey == KEY2) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			goto Print;
		}
	}
	
	
	return OK;
}



uint8_t RestaurantPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
	char    buf1[200];
	int     i;
	uint8_t printDate[30];
	int     iKey;
	int     printNum = 1;
	
Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
//	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",printDate, 
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

	memset(buf1,0,sizeof(buf1));
	GetTransPrtName(PosCom.stTrans.iTransNo, buf1);    
	prnPrintf("TIPO DE TRANSACCION: \n");
	prnPrintf("               %s\n",buf1);

	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_PESEO )
		{
			continue;
		}
		ucRet = RestaurantCalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}

	#if 0
	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		ucRet = CalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}
	#endif
	
	prnPrintf("CIERRE DE LOTE OK\n");
	prnPrintf("AB      $          %ld.%02ld\n\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL MXN\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Total    ");

	#if 0
	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Check in ");
	strcpy(AllTransTable[8].szTransName,"Check out");
	strcpy(AllTransTable[9].szTransName,"Total    ");
	#endif
	

	for( i = 0; i < 8; i++){
		if( i == 6 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}


	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_DOLARES )
		{
			continue;
		}
		ucRet = RestaurantCalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}

	#if 0
	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		ucRet = CalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}
	#endif
	
	prnPrintf("CIERRE DE LOTE OK\n");
	prnPrintf("AB      $          %ld.%02ld\n\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL USD\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Total    ");

	#if 0
	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Check in ");
	strcpy(AllTransTable[8].szTransName,"Check out");
	strcpy(AllTransTable[9].szTransName,"Total    ");
	#endif
	

	for( i = 0; i < 8; i++){
		if( i == 6 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}
	

	prnPrintf("\nTOTAL CONSUMOS   $  %ld.%02ld\n",/*AllTransTable[7].lTotalTransAmount/100*/0,
		/*AllTransTable[7].lTotalTransAmount%100*/0);
	prnPrintf("TOTAL           $  %ld.%02ld\n",/*AllTransTable[6].lTotalTransAmount/100*/0,
		/*AllTransTable[6].lTotalTransAmount%100*/0);

	prnPrintf("\n              %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	lcdClrLine(2,7);
	lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "CIERRE DE LOTE. APROBADO");
	lcdFlip();
	usleep(2*1000);

	if( printNum == 1 )
	{
		printNum = 0;
		
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "DESEA UNA COPIA DE CIERRE?");
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL || iKey == KEY2) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			goto Print;
		}
	}
	
	
	return OK;
}


uint8_t RetailPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
	char    buf1[200];
	int     i;
	uint8_t printDate[30];
	int     iKey;
	int     printNum = 1;
	
Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
//	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",printDate, 
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

	memset(buf1,0,sizeof(buf1));
	GetTransPrtName(PosCom.stTrans.iTransNo, buf1);    
	prnPrintf("TIPO DE TRANSACCION: \n");
	prnPrintf("               %s\n",buf1);

	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_PESEO )
		{
			continue;
		}
		ucRet = RetailCalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}

	#if 0
	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		ucRet = CalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}
	#endif
	
	prnPrintf("CIERRE DE LOTE OK\n");
	prnPrintf("AB      $          %ld.%02ld\n\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL MXN\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"S/Inte   ");
	strcpy(AllTransTable[2].szTransName,"Diferidos");
	strcpy(AllTransTable[3].szTransName,"Cancel   ");
	strcpy(AllTransTable[4].szTransName,"Devol    ");
	strcpy(AllTransTable[5].szTransName,"Descuento");
	strcpy(AllTransTable[6].szTransName,"Total    ");

	#if 0
	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Check in ");
	strcpy(AllTransTable[8].szTransName,"Check out");
	strcpy(AllTransTable[9].szTransName,"Total    ");
	#endif
	

	for( i = 0; i < 7; i++){
		if( i == 5 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}


	memset(&AllTransTable,0,sizeof(AllTransTable));
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		if(stLog.szMoneyType != PARAM_DOLARES )
		{
			continue;
		}
		ucRet = RetailCalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}

	#if 0
	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		ucRet = CalTotalTransNo(&stLog);
		if( ucRet != OK){
			return ucRet;
		}
	}
	#endif
	
	prnPrintf("CIERRE DE LOTE OK\n");
	prnPrintf("AB      $          %ld.%02ld\n\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL USD\n");	
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"S/Inte   ");
	strcpy(AllTransTable[2].szTransName,"Diferidos");
	strcpy(AllTransTable[3].szTransName,"Cancel   ");
	strcpy(AllTransTable[4].szTransName,"Devol    ");
	strcpy(AllTransTable[5].szTransName,"Descuento");
	strcpy(AllTransTable[6].szTransName,"Total    ");

	#if 0
	strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"S/Inte   ");
	strcpy(AllTransTable[3].szTransName,"Diferidos");
	strcpy(AllTransTable[4].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Devol    ");
	strcpy(AllTransTable[6].szTransName,"Descuento");
	strcpy(AllTransTable[7].szTransName,"Check in ");
	strcpy(AllTransTable[8].szTransName,"Check out");
	strcpy(AllTransTable[9].szTransName,"Total    ");
	#endif
	

	for( i = 0; i < 7; i++){
		if( i == 5 )   //Descuento need to add "-"
		{
			prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else 
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		
	}
	

	prnPrintf("\nTOTAL CONSUMOS   $  %ld.%02ld\n",/*AllTransTable[7].lTotalTransAmount/100*/0,
		/*AllTransTable[7].lTotalTransAmount%100*/0);
	prnPrintf("TOTAL           $  %ld.%02ld\n",/*AllTransTable[6].lTotalTransAmount/100*/0,
		/*AllTransTable[6].lTotalTransAmount%100*/0);

	prnPrintf("\n              %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	lcdClrLine(2,7);
	lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "CIERRE DE LOTE. APROBADO");
	lcdFlip();
	usleep(2*1000);

	if( printNum == 1 )
	{
		printNum = 0;
		
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_HFONT16|DISP_MEDIACY, "DESEA UNA COPIA DE CIERRE?");
		lcdDisplay(210, 8, DISP_HFONT16 ,"1-SI");
		lcdDisplay(210, 9, DISP_HFONT16, "2-NO");
		lcdFlip();
		do {
			iKey = PubWaitKey(60);
		}while(iKey != KEY_ENTER && iKey != KEY_CANCEL && iKey != KEY_TIMEOUT &&
			iKey != KEY1 && iKey != KEY2);

		if (iKey == KEY_CANCEL || iKey == KEY2) {
			return NO_DISP;
		}
		else if ( iKey == KEY_ENTER || iKey == KEY1){
			goto Print;
		}
	}
	
	
	return OK;
}

uint8_t PrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet, nowtime[8];
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
	uint8_t tmpdate_time[20] ={0};//current date & time
	
	char buf[200] ={0};
	char buf1[200] ={0};
	char buf2[200]={0};
	char buf3[200] ={0};
	char tmp_printdata[200] ={0};
	
	int     i =0;
	uint8_t printDate[30];
	int     iKey =0;
	int     printNum = 1;
	int     TransNum =0;

	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 && stTransCtrl.iCheckTransNum==0)
	{
		return E_NO_TRANS;
	}

	sysGetTime(nowtime);    //YYMMDDhhmmss
	memcpy(tmpdate_time,"20",2);
	PubBcd2Asc0(nowtime, 6, tmpdate_time+2);
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    		lcdFlip();
    		sysDelayMs(3000);
		return NO_DISP;
  	}
	
	newprnLogo(85,0,200,120,(uint8_t *)Bmp_Prn);

	PrnSmallFontData();
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf1));
	memset(buf3,0,sizeof(buf1));

	sprintf(buf,"               %s\n",stPosParam.stHeader1);
	sprintf(buf1,"                %s\n",stPosParam.stAddress1);
	sprintf(buf2,"                  %s\n",stPosParam.stAddress2);
	sprintf(buf3,"                  %s\n",stPosParam.stAddress3);
	prnPrintf("%s", buf);
	prnPrintf("%s", buf1);  
	prnPrintf("%s", buf2); 
	prnPrintf("%s", buf3); 
	prnPrintf("\n");  


	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              TERMINAL %s\n",stPosParam.stTerminalId);
	prnPrintf("%s", tmp_printdata);  

	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              AFILIACION  %s\n",stPosParam.stAffId);
	prnPrintf("%s", tmp_printdata);  

	//MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);
	printf("PosCom.stTrans.szTransDate:%s\n",PosCom.stTrans.szTransDate);
	

	PrintDebug("%s %s", "CURRENT_TIME",tmpdate_time);

	memset(tmp_printdata,0,sizeof(tmp_printdata));
	MakeFormatPrintDate(tmpdate_time,tmp_printdata);

	prnPrintf("%s%s           %s%.2s:%.2s:%.2s\n","FECHA: ",tmp_printdata, "HORA: ",
			&tmpdate_time[8], &tmpdate_time[10], &tmpdate_time[12]);


//YYMMDDhhmmss
	prnPrintf("TOTALES");

	prnPrintf("\n");
	
	memset(&AllTransTable,0,sizeof(AllTransTable));
	printf("stTransCtrl.iTransNum:%d\n",stTransCtrl.iTransNum);

	while(1)
	{
		//visa
		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK)
			{
				return ucRet;
			}
		}

		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nVISA\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}
		break;
	}
	
	while(1)
	{
		//MASTERCARD
		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 2 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nMCARD\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}
		
		break;
	}

	while(1)
	{
 		// 3 AMEX 	
 		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 3 )
			{
				continue;
			}
			TransNum++;   
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nAMEX\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}
		break;
	}


	while(1)
	{
		// 4 PCLAVE 	
		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 4 )
			{
				continue;
			}
			TransNum++; 
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nPCLAVE\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}
		break;
	}
	while(1)
	{
		// 5 SODEXO	
		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 5 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nSODEXO\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}

		break;
	}
	while(1)
	{
		// 6 TODITO 
		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 6 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nTODITO\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}
		
		break;
	}
	while(1)
	{
		//7 EDENRED 
		memset(&SIGNALTransTable,0,sizeof(SIGNALTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 7 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		if(bPrintAll ==1)
		{
			if(TransNum !=0)
			{
				PrnBlackEngData();
				prnPrintf("\nEDENRED\n");	
				PrnBigFontChnData();

				strcpy(SIGNALTransTable[0].szTransName,"Ventas  :"); // 消费名称
				strcpy(SIGNALTransTable[1].szTransName,"Litros  :");

				//PrnSmallFontData();
				for( i = 0; i < 2; i++)
				{
					prnPrintf("%s    %d  $  %ld.%02ld\n",SIGNALTransTable[i].szTransName,
						SIGNALTransTable[i].iTotalTransNo,SIGNALTransTable[i].lTotalTransAmount/100,
						SIGNALTransTable[i].lTotalTransAmount%100);
				}
			}
		}
		
		break;
	}

	//PRINT TOTAL
	{
		PrnBlackEngData();
		prnPrintf("\nGRAN TOTAL\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Ventas  :"); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Litros  :");

		//PrnSmallFontData();

		for( i = 0; i < 2; i++)
		{
			prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		
		}
	}
		
	prnPrintf("\n            %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n");
	PrintData();	
	
	return OK;
}




uint8_t HotelNetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	int     i;
	uint8_t printDate[30];
//	int     iKey;
//	int     printNum = 1;
	int     TransNum;
	int 	ICheckInOutNum = 0;

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}

//Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 &&  stTransCtrl.iCheckTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(stTemp.szDate,printDate);
	prnPrintf("\n%s    %.2s:%.2s:%.2s\n",printDate, 
		&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);

   	if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
	{
		prnPrintf("TOTALES:  MXN\n");
	}
	else 
	{
		prnPrintf("TOTALES:  USD\n");
	}
	

	memset(&AllTransTable,0,sizeof(AllTransTable));
	while(1)
	{
		//visa
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}

			if(stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
			{
				ICheckInOutNum++;
				if( ICheckInOutNum == stTransCtrl.iTransNum)
				{
					lcdClrLine(2, 7);	
					DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "REPORTE DE TOTALES");
					DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "LOTE VACIO");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000);
					return NO_DISP;				
				}
				continue;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			if( stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
			{
				continue;
			}

			TransNum++;   //统计visa卡交易笔数
			ucRet = HotelCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nVISA\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Cancel   ");
		strcpy(AllTransTable[2].szTransName,"Devol    ");
		strcpy(AllTransTable[3].szTransName,"Descuento");
		strcpy(AllTransTable[4].szTransName,"Total    ");
		
		for( i = 0; i < 5; i++){
			if( i == 3 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	
	while(1)
	{
		//master
		TransNum = 0;
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 2 )
			{
				continue;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			if( stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
			{
				continue;
			}
			
			TransNum++;   //统计mcard卡交易笔数
			ucRet = HotelCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 2 )
			{
				continue;
			}
			TransNum++;    //统计mcard卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nMCARD\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Cancel   ");
		strcpy(AllTransTable[2].szTransName,"Devol    ");
		strcpy(AllTransTable[3].szTransName,"Descuento");
		strcpy(AllTransTable[4].szTransName,"Total    ");

		for( i = 0; i < 5; i++){
			if( i == 3 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	while(1)
	{
		//AMEX
		memset(&AllTransTable,0,sizeof(AllTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 3 )
			{
				continue;
			}
			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			if( stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
			{
				continue;
			}
			
			TransNum++;
			ucRet = HotelCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 3 )
			{
				continue;
			}
			TransNum++;   //统计AMEX卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nAMEX\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Cancel   ");
		strcpy(AllTransTable[2].szTransName,"Devol    ");
		strcpy(AllTransTable[3].szTransName,"Descuento");
		strcpy(AllTransTable[4].szTransName,"Total    ");

		for( i = 0; i < 5; i++){
			if( i == 3 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			
		}
		prnPrintf("\n");
		break;
	}
	while(1)
	{	
		//total
		prnPrintf("\n");
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			if( stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
			{
				continue;
			}
			
			ucRet = HotelCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		PrnBlackEngData();
		prnPrintf("\nGRAN TOTAL\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Cancel   ");
		strcpy(AllTransTable[2].szTransName,"Devol    ");
		strcpy(AllTransTable[3].szTransName,"Descuento");
		strcpy(AllTransTable[4].szTransName,"Total    ");

		for( i = 0; i < 5; i++){
			if( i == 3 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}

		prnPrintf("\nTOTAL CONSUMOS  $  %ld.%02ld\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[5].lTotalTransAmount%100);
		prnPrintf("TOTAL           $  %ld.%02ld\n",AllTransTable[6].lTotalTransAmount/100,
			AllTransTable[4].lTotalTransAmount%100);
		break;
	}
		
	prnPrintf("\n            %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	return NO_DISP;
}



uint8_t RestaurantNetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	int     i;
	uint8_t printDate[30];
//	int     iKey;
//	int     printNum = 1;
	int     TransNum;

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}

//Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 &&  stTransCtrl.iCheckTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(stTemp.szDate,printDate);
	prnPrintf("\n%s    %.2s:%.2s:%.2s\n",printDate, 
		&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);

   	if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
	{
		prnPrintf("TOTALES:  MXN\n");
	}
	else 
	{
		prnPrintf("TOTALES:  USD\n");
	}
	

	memset(&AllTransTable,0,sizeof(AllTransTable));
	while(1)
	{
		//visa
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}


			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			TransNum++;   //统计visa卡交易笔数
			ucRet = RestaurantCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nVISA\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Prop     ");
		strcpy(AllTransTable[2].szTransName,"S/Inte   ");
		strcpy(AllTransTable[3].szTransName,"Diferidos");
		strcpy(AllTransTable[4].szTransName,"Cancel   ");
		strcpy(AllTransTable[5].szTransName,"Devol    ");
		strcpy(AllTransTable[6].szTransName,"Descuento");
		strcpy(AllTransTable[7].szTransName,"Total    ");

		for( i = 0; i < 8; i++){
			if( i == 6 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	
	while(1)
	{
		//master
		TransNum = 0;
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 2 )
			{
				continue;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			
			TransNum++;   //统计mcard卡交易笔数
			ucRet = RestaurantCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 2 )
			{
				continue;
			}
			TransNum++;    //统计mcard卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nMCARD\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Prop     ");
		strcpy(AllTransTable[2].szTransName,"S/Inte   ");
		strcpy(AllTransTable[3].szTransName,"Diferidos");
		strcpy(AllTransTable[4].szTransName,"Cancel   ");
		strcpy(AllTransTable[5].szTransName,"Devol    ");
		strcpy(AllTransTable[6].szTransName,"Descuento");
		strcpy(AllTransTable[7].szTransName,"Total    ");

		for( i = 0; i < 8; i++){
			if( i == 6 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	while(1)
	{
		//AMEX
		memset(&AllTransTable,0,sizeof(AllTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 3 )
			{
				continue;
			}
			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			
			TransNum++;
			ucRet = RestaurantCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 3 )
			{
				continue;
			}
			TransNum++;   //统计AMEX卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nAMEX\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Prop     ");
		strcpy(AllTransTable[2].szTransName,"S/Inte   ");
		strcpy(AllTransTable[3].szTransName,"Diferidos");
		strcpy(AllTransTable[4].szTransName,"Cancel   ");
		strcpy(AllTransTable[5].szTransName,"Devol    ");
		strcpy(AllTransTable[6].szTransName,"Descuento");
		strcpy(AllTransTable[7].szTransName,"Total    ");

		for( i = 0; i < 8; i++){
			if( i == 6 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			
		}
		prnPrintf("\n");
		break;
	}
	while(1)
	{	
		//total
		prnPrintf("\n");
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			
			ucRet = RestaurantCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		PrnBlackEngData();
		prnPrintf("\nGRAN TOTAL\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"Prop     ");
		strcpy(AllTransTable[2].szTransName,"S/Inte   ");
		strcpy(AllTransTable[3].szTransName,"Diferidos");
		strcpy(AllTransTable[4].szTransName,"Cancel   ");
		strcpy(AllTransTable[5].szTransName,"Devol    ");
		strcpy(AllTransTable[6].szTransName,"Descuento");
		strcpy(AllTransTable[7].szTransName,"Total    ");

		for( i = 0; i < 8; i++){
			if( i == 6 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}

		prnPrintf("\nTOTAL CONSUMOS  $  %ld.%02ld\n",AllTransTable[8].lTotalTransAmount/100,
			AllTransTable[8].lTotalTransAmount%100);
		prnPrintf("TOTAL           $  %ld.%02ld\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);
		break;
	}
		
	prnPrintf("\n            %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	return NO_DISP;
}


uint8_t RetailNetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	int     i;
	uint8_t printDate[30];
//	int     iKey;
//	int     printNum = 1;
	int     TransNum;

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}

//Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 &&  stTransCtrl.iCheckTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(stTemp.szDate,printDate);
	prnPrintf("\n%s    %.2s:%.2s:%.2s\n",printDate, 
		&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);

   	if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
	{
		prnPrintf("TOTALES:  MXN\n");
	}
	else 
	{
		prnPrintf("TOTALES:  USD\n");
	}
	

	memset(&AllTransTable,0,sizeof(AllTransTable));
	while(1)
	{
		//visa
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}


			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			TransNum++;   //统计visa卡交易笔数
			ucRet = RetailCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nVISA\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	
	while(1)
	{
		//master
		TransNum = 0;
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 2 )
			{
				continue;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			
			TransNum++;   //统计mcard卡交易笔数
			ucRet = RetailCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 2 )
			{
				continue;
			}
			TransNum++;    //统计mcard卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nMCARD\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	while(1)
	{
		//AMEX
		memset(&AllTransTable,0,sizeof(AllTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 3 )
			{
				continue;
			}
			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			
			TransNum++;
			ucRet = RetailCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 3 )
			{
				continue;
			}
			TransNum++;   //统计AMEX卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nAMEX\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			
		}
		prnPrintf("\n");
		break;
	}
	while(1)
	{	
		//total
		prnPrintf("\n");
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
			{
				if( stLog.szMoneyType!=PARAM_PESEO )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				if( stLog.szMoneyType!=PARAM_DOLARES )
				{
					if( iCnt == stTransCtrl.iTransNum - 1  )
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}
			
			ucRet = RetailCalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		PrnBlackEngData();
		prnPrintf("\nGRAN TOTAL\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}

		prnPrintf("\nTOTAL CONSUMOS  $  %ld.%02ld\n",AllTransTable[7].lTotalTransAmount/100,
			AllTransTable[7].lTotalTransAmount%100);
		prnPrintf("TOTAL           $  %ld.%02ld\n",AllTransTable[6].lTotalTransAmount/100,
			AllTransTable[6].lTotalTransAmount%100);
		break;
	}
		
	prnPrintf("\n            %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	return NO_DISP;
}


uint8_t NetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	int     i;
	uint8_t printDate[30];
//	int     iKey;
//	int     printNum = 1;
	int     TransNum;

//Print:
	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 &&  stTransCtrl.iCheckTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	
//	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
//	{
		newprnLogo(85,0,200,120,(uint8_t *)Bmp_Prn);
//	}

	PrnBigFontChnData();
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");

	prnPrintf("%s       TERMINAL: %s\n", stPosParam.szST,PosCom.stTrans.szPosId);

	prnPrintf("CARGO:%06ld    LOTE NUM:%06ld\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.lBatchNumber);//water NO
	
	GetPosTime(stTemp.szDate, stTemp.szTime);	
	MakeFormatPrintDate(stTemp.szDate,printDate);
	prnPrintf("\n%s    %.2s:%.2s:%.2s\n",printDate, 
		&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);
   
	prnPrintf("TOTALES: \n");

	memset(&AllTransTable,0,sizeof(AllTransTable));
	while(1)
	{
		//visa
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 1 )
			{
				continue;
			}
			TransNum++;   //统计visa卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nVISA\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	
	while(1)
	{
		//master
		TransNum = 0;
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 2 )
			{
				continue;
			}
			TransNum++;   //统计mcard卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 2 )
			{
				continue;
			}
			TransNum++;    //统计mcard卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nMCARD\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}
		break;
	}
	while(1)
	{
		//AMEX
		memset(&AllTransTable,0,sizeof(AllTransTable));
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			if( stLog.CardType != 3 )
			{
				continue;
			}
			TransNum++;
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		#if 0
		TransNum = 0;
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadCheckTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}

			if( stLog.CardType != 3 )
			{
				continue;
			}
			TransNum++;   //统计AMEX卡交易笔数
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}
		#endif

		#if 0
		if( TransNum == 0 )
		{
			break;
		}
		#endif

		PrnBlackEngData();
		prnPrintf("\nAMEX\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			
		}
		prnPrintf("\n");
		break;
	}
	while(1)
	{	
		//total
		prnPrintf("\n");
		memset(&AllTransTable,0,sizeof(AllTransTable));
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iCnt, &stLog);
			if( ucRet!=OK )
			{
				break;
			}
			
			ucRet = CalTotalTransNo(&stLog);
			if( ucRet != OK){
				return ucRet;
			}
		}

		PrnBlackEngData();
		prnPrintf("\nGRAN TOTAL\n");	
		PrnBigFontChnData();

		strcpy(AllTransTable[0].szTransName,"Regular  "); // 消费名称
		strcpy(AllTransTable[1].szTransName,"S/Inte   ");
		strcpy(AllTransTable[2].szTransName,"Diferidos");
		strcpy(AllTransTable[3].szTransName,"Cancel   ");
		strcpy(AllTransTable[4].szTransName,"Devol    ");
		strcpy(AllTransTable[5].szTransName,"Descuento");
		strcpy(AllTransTable[6].szTransName,"Total    ");

		for( i = 0; i < 7; i++){
			if( i == 5 )   //Descuento need to add "-"
			{
				prnPrintf("%s    %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
			else 
			{
				prnPrintf("%s    %d  $  %ld.%02ld\n",AllTransTable[i].szTransName,
					AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
					AllTransTable[i].lTotalTransAmount%100);
			}
		}

		prnPrintf("\nTOTAL CONSUMOS  $  %ld.%02ld\n",AllTransTable[6].lTotalTransAmount/100,
			AllTransTable[6].lTotalTransAmount%100);
		prnPrintf("TOTAL           $  %ld.%02ld\n",AllTransTable[6].lTotalTransAmount/100,
			AllTransTable[6].lTotalTransAmount%100);
		break;
	}
		
	prnPrintf("\n            %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();

	return NO_DISP;
}



uint8_t CheckInPrint(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;

	memset(AllTransTable,0,sizeof(AllTransTable));

	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s    TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

    PrtTranDetailHead();
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		ucRet = NetpayCheckInPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");

	PrnBlackEngData();
	prnPrintf("GRAN TOTAL\n");
	PrnBigFontChnData();

	strcpy(AllTransTable[0].szTransName,"Regular   "); // 消费名称
	strcpy(AllTransTable[2].szTransName,"Cancel   ");
	strcpy(AllTransTable[5].szTransName,"Total    ");

	//总金额(SALE-CANCEL)
	AllTransTable[5].lTotalTransAmount += AllTransTable[0].lTotalTransAmount + AllTransTable[1].lTotalTransAmount;
	AllTransTable[5].lTotalTransAmount -= AllTransTable[2].lTotalTransAmount;

	for( i = 0; i < 6; i++){
		if( i == 0 || i == 2 || i == 5)
		{
			prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
	}
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}



#if 0
uint8_t PrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;

	memset(AllTransTable,0,sizeof(AllTransTable));

	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s     TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

    PrtTranDetailHead();
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		ucRet = NetpayPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");

	PrnBigFontChnData();
	strcpy(AllTransTable[0].szTransName,"Ventas   "); // 消费名称
	strcpy(AllTransTable[1].szTransName,"Prop     ");
	strcpy(AllTransTable[2].szTransName,"Cancel   ");
	strcpy(AllTransTable[3].szTransName,"Devol    ");
	strcpy(AllTransTable[4].szTransName,"Descuento");
	strcpy(AllTransTable[5].szTransName,"Total    ");

	//总金额(SALE-CANCEL)
	AllTransTable[5].lTotalTransAmount += AllTransTable[0].lTotalTransAmount + AllTransTable[1].lTotalTransAmount;
	AllTransTable[5].lTotalTransAmount -= AllTransTable[2].lTotalTransAmount;

	for( i = 0; i < 6; i++){
		if(i == 4)
		{
			prnPrintf("%s   %d -$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else
		{
			prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
	}
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}
#endif


uint8_t HotelPrintCheckOutDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;
	long     lTotalConsuTransAmount = 0;		 //总消费金额
	int		ICheckInOutNum = 0;

	memset(AllTransTable,0,sizeof(AllTransTable));

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}
	
	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s     TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

   	if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
	{
		prnPrintf("CHECK OUTS. MXN\n\n");
	}
	else 
	{
		prnPrintf("CHECK INS. USD\n\n");
	}
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		if( stLog.iTransNo!=CHECK_OUT && stLog.iTransNo!=POS_REFUND &&
			stLog.iTransNo!=NETPAY_REFUND  )
		{
			ICheckInOutNum++;
			if( ICheckInOutNum == stTransCtrl.iTransNum)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "REPORTE DE CHECK IN");
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "LOTE VACIO");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				return NO_DISP;				
			}
			continue;
		}

		if( stLog.iTransNo==POS_REFUND ||
			stLog.iTransNo==NETPAY_REFUND )
		{
			if( stLog.iOldTransNo!=CHECK_OUT )
			{	
				continue;
			}
			
		}

		if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
		{
			if( stLog.szMoneyType!=PARAM_PESEO )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			if( stLog.szMoneyType!=PARAM_DOLARES )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}

		ucRet = HotelCheckOutNetpayPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");

	PrnBigFontChnData();
	strcpy(AllTransTable[0].szTransName,"Regular  "); 
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Devol.   ");
	strcpy(AllTransTable[3].szTransName,"Total    ");

	//总金额(SALE-CANCEL)
	lTotalConsuTransAmount +=  AllTransTable[0].lTotalTransAmount;
	AllTransTable[3].lTotalTransAmount += AllTransTable[0].lTotalTransAmount;
	AllTransTable[3].lTotalTransAmount -= (AllTransTable[1].lTotalTransAmount + AllTransTable[2].lTotalTransAmount);

	for( i = 0; i < 4; i++){
		prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
			AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
			AllTransTable[i].lTotalTransAmount%100);
	}

	prnPrintf("\n");
	prnPrintf("Total CONSUMOS $  %ld.%02ld\n",lTotalConsuTransAmount/100,lTotalConsuTransAmount%100);
	prnPrintf("Total          $  %ld.%02ld\n",AllTransTable[i-1].lTotalTransAmount/100,
		AllTransTable[i-1].lTotalTransAmount%100);
	
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}


uint8_t HotelPrintCheckInDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;
	long     lTotalConsuTransAmount = 0;		 //总消费金额
	int		ICheckInOutNum = 0;

	memset(AllTransTable,0,sizeof(AllTransTable));

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}
	
	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s     TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

   	if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
	{
		prnPrintf("CHECK INS. MXN\n\n");
	}
	else 
	{
		prnPrintf("CHECK INS. USD\n\n");
	}
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		if( stLog.iTransNo!=CHECK_IN && stLog.iTransNo!=POS_REFUND )
		{
			ICheckInOutNum++;
			if( ICheckInOutNum == stTransCtrl.iTransNum)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "REPORTE DE CHECK IN");
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "LOTE VACIO");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				return NO_DISP;				
			}
			continue;
		}

		if( stLog.iTransNo==POS_REFUND )
		{
			if( stLog.iOldTransNo!=CHECK_IN)
			{
				continue;
			}
		}

		if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
		{
			if( stLog.szMoneyType!=PARAM_PESEO )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			if( stLog.szMoneyType!=PARAM_DOLARES )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}

		ucRet = HotelCheckInNetpayPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");

	PrnBigFontChnData();
	strcpy(AllTransTable[0].szTransName,"Regular  "); 
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Total    ");

	//总金额(SALE-CANCEL)
	lTotalConsuTransAmount +=  AllTransTable[0].lTotalTransAmount;
	AllTransTable[2].lTotalTransAmount += AllTransTable[0].lTotalTransAmount;
	AllTransTable[2].lTotalTransAmount -= (AllTransTable[1].lTotalTransAmount);

	for( i = 0; i < 3; i++){
		prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
			AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
			AllTransTable[i].lTotalTransAmount%100);
	}

	prnPrintf("\n");
	prnPrintf("Total CONSUMOS $  %ld.%02ld\n",lTotalConsuTransAmount/100,lTotalConsuTransAmount%100);
	prnPrintf("Total          $  %ld.%02ld\n",AllTransTable[i-1].lTotalTransAmount/100,
		AllTransTable[i-1].lTotalTransAmount%100);
	
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}


uint8_t HotelPrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
//	char    buf1[200];
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;
	long     lTotalConsuTransAmount = 0;		 //总消费金额
	int		ICheckInOutNum = 0;

	memset(AllTransTable,0,sizeof(AllTransTable));

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}

	
	
	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s     TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

    PrtTranDetailHead();
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}

		}

		if(stLog.iTransNo==CHECK_IN || stLog.iTransNo==CHECK_OUT)
		{
			ICheckInOutNum++;
			if( ICheckInOutNum == stTransCtrl.iTransNum)
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "REPORTE DE DETALLAS");
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "LOTE VACIO");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				return NO_DISP;				
			}
			continue;
		}

		if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
		{
			if( stLog.szMoneyType!=PARAM_PESEO )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			if( stLog.szMoneyType!=PARAM_DOLARES )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}

		ucRet = HotelNetpayPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");

	PrnBigFontChnData();
	strcpy(AllTransTable[0].szTransName,"Regular  "); 
	strcpy(AllTransTable[1].szTransName,"Cancel   ");
	strcpy(AllTransTable[2].szTransName,"Devol    ");
	strcpy(AllTransTable[3].szTransName,"Descuento");
	strcpy(AllTransTable[4].szTransName,"Total    ");

	//总金额(SALE-CANCEL)
	lTotalConsuTransAmount +=  AllTransTable[0].lTotalTransAmount;
	AllTransTable[4].lTotalTransAmount += AllTransTable[0].lTotalTransAmount;
	AllTransTable[4].lTotalTransAmount -= (AllTransTable[1].lTotalTransAmount+AllTransTable[2].lTotalTransAmount);

	for( i = 0; i < 5; i++){
		if(i == 3)
		{
			prnPrintf("%s    %d-$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else
		{
			prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
	}

	prnPrintf("\n");
	prnPrintf("Total CONSUMOS $  %ld.%02ld\n",lTotalConsuTransAmount/100,lTotalConsuTransAmount%100);
	prnPrintf("Total          $  %ld.%02ld\n",AllTransTable[i-1].lTotalTransAmount/100,
		AllTransTable[i-1].lTotalTransAmount%100);
	
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}

//打印明细
uint8_t PrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
	char buf[200]={0};
	char buf1[200]={0};
	char buf2[200]={0};
	char buf3[200]={0};
	char tmp_printdata[200] ={0};
	char printDate[50] ={0};
	
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;
	long     lTotalConsuTransAmount = 0;		 //总消费金额

	memset(AllTransTable,0,sizeof(AllTransTable));

	
	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    		lcdFlip();
    		sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,120,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	PrnSmallFontData();

	//打印小票头
	memset(buf,0,sizeof(buf1));
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf1));
	memset(buf3,0,sizeof(buf1));

	sprintf(buf,"               %s\n",stPosParam.stHeader1);
	sprintf(buf1,"                %s\n",stPosParam.stAddress1);
	sprintf(buf2,"                  %s\n",stPosParam.stAddress2);
	sprintf(buf3,"                  %s\n",stPosParam.stAddress3);

	prnPrintf("%s", buf);
	prnPrintf("%s", buf1);  
	prnPrintf("%s", buf2);  
	prnPrintf("%s", buf3);  

	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              TERMINAL %s\n",stPosParam.stTerminalId);
	prnPrintf("%s", tmp_printdata);  

	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              AFILIACION  %s\n",stPosParam.stAffId);
	prnPrintf("%s", tmp_printdata);  
	prnPrintf("\n");
	
	MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);

	prnPrintf("%s%s           %s%.2s:%.2s:%.2s\n","FECHA: ",printDate, "HORA: ",
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);
	prnPrintf("\n");

	prnPrintf("LISTA DE TRANS.");

	prnPrintf("\n\n");
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}

		}
		
		ucRet = GasstationPrtOneTranTxn(&stLog,1);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");

	PrnBigFontChnData();
	strcpy(AllTransTable[0].szTransName,"Total   :   "); 
	strcpy(AllTransTable[1].szTransName,"Litros  :   ");

	for( i = 0; i < 2; i++)
	{
		prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
		AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
		AllTransTable[i].lTotalTransAmount%100);
	}

	prnPrintf("\n");

	prnPrintf("\n            %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}


uint8_t RestaurantPrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i;

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}

	memset(AllTransTable,0,sizeof(AllTransTable));

	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE DE DETALLES  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "   IMPRIMIENDO...   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s     TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

    PrtTranDetailHead();
	
	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
		{
			if( stLog.szMoneyType!=PARAM_PESEO )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			if( stLog.szMoneyType!=PARAM_DOLARES )
			{
				if( iCnt == stTransCtrl.iTransNum - 1  )
				{
					iNumOfOnePage++;
					break;
				}
				else
				{
					continue;
				}
			}
		}

		ucRet = RestaurantNetpayPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	#if 0
	for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadCheckTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		ucRet = NetpayPrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}
	#endif

	prnPrintf("\n");

	PrnBigFontChnData();
	strcpy(AllTransTable[0].szTransName,"Ventas   "); 
	strcpy(AllTransTable[1].szTransName,"Prop     "); 
	strcpy(AllTransTable[2].szTransName,"Cancel   ");
	strcpy(AllTransTable[3].szTransName,"Devol    ");
	strcpy(AllTransTable[4].szTransName,"Descuento");
	strcpy(AllTransTable[5].szTransName,"Total    ");

	//总金额(SALE-CANCEL)
	AllTransTable[5].lTotalTransAmount += AllTransTable[0].lTotalTransAmount + AllTransTable[1].lTotalTransAmount;
	AllTransTable[5].lTotalTransAmount -= (AllTransTable[2].lTotalTransAmount+AllTransTable[3].lTotalTransAmount);

	for( i = 0; i < 6; i++){
		if(i == 4)
		{
			prnPrintf("%s    %d-$  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
		else
		{
			prnPrintf("%s    %d $  %ld.%02ld\n",AllTransTable[i].szTransName,
				AllTransTable[i].iTotalTransNo,AllTransTable[i].lTotalTransAmount/100,
				AllTransTable[i].lTotalTransAmount%100);
		}
	}
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}

//打印容量控制信息
uint8_t PrintCtlvolume(uint8_t bPrintAll)
{
	uint8_t		ucRet;
	int		iCnt,iRet;
	int          iNumOfOnePage =0; 
	NEWPOS_LOG_STRC	stLog;
	char buf[200] ={0};
	char buf1[200] ={0};
	char buf2[200]={0};
	char buf3[200] ={0};
	char tmp_printdata[200] ={0};
	
	int     i =0;
	uint8_t printDate[30];
	int     iKey =0;
	int     printNum = 1;
	int     TransNum =0;

	if( bPrintAll )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " IMPRIMIENDO ");
		lcdFlip();
	}
	if( stTransCtrl.iTransNum==0 && stTransCtrl.iCheckTransNum==0)
	{
		return E_NO_TRANS;
	}
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    		lcdFlip();
    		sysDelayMs(3000);
		return NO_DISP;
  	}
	
	newprnLogo(85,0,200,120,(uint8_t *)Bmp_Prn);

	PrnSmallFontData();
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf1));
	memset(buf3,0,sizeof(buf1));

	sprintf(buf,"               %s\n",stPosParam.stHeader1);
	sprintf(buf1,"                %s\n",stPosParam.stAddress1);
	sprintf(buf2,"                  %s\n",stPosParam.stAddress2);
	sprintf(buf3,"                  %s\n",stPosParam.stAddress3);
	prnPrintf("%s", buf);
	prnPrintf("%s", buf1);  
	prnPrintf("%s", buf2); 
	prnPrintf("%s", buf3); 
	prnPrintf("\n");  


	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              TERMINAL %s\n",stPosParam.stTerminalId);
	prnPrintf("%s", tmp_printdata);  

	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              AFILIACION  %s\n",stPosParam.stAffId);
	prnPrintf("%s", tmp_printdata);  

	GetPosTime(stTemp.szDate, stTemp.szTime);
	//MakeFormatPrintDate(stTemp.szDate,printDate);
		
	prnPrintf("%s%.2s/%.2s/%.2s           %s%.2s:%.2s:%.2s\n","FECHA: ",&stTemp.szDate[6],&stTemp.szDate[4],&stTemp.szDate[2],
			"HORA: ",&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);

	prnPrintf("LISTA DE TRANS. CONTROL GAS");

	prnPrintf("\n\n");

	
	//memset(&AllTransTable,0,sizeof(AllTransTable));
	PrintDebug("%s %d","stTransCtrl.iTransNum:",stTransCtrl.iTransNum);
	//PrnSmallFontData();

	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}
		PrintDebug("%s%d", "stLog->CtlFlag:",stLog.CtlFlag);
		if(stLog.CtlFlag !=1)
		{
			continue;
		}
		
		ucRet = GasstationPrtOneTranTxn(&stLog,0);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    					DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
	    			lcdFlip();
	    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}	
		
	prnPrintf("\n                %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n");
	PrintData();	
	
	return OK;
}

//打印小费明细
uint8_t PrintTipDetail(uint8_t bPrintAll,uint8_t ucTitleFlag)
{
	uint8_t		ucRet;
	int		iCnt, iNumOfOnePage,iRet;
	NEWPOS_LOG_STRC	stLog;
	char    buf1[200];
	char szDate1[14+1] = {0};
	char szDate2[21+1] = {0};
	int  i,j;
	long szTempTipAmt[999] = {0};
	int  SumMesero = 0;
	int  MeseroArr[999] = {0};
	int  waiterNo;  //服务员编号
	uint8_t szAmount[16];

	ucRet = PrintSelectMoneyType();
	if( ucRet != OK )
	{
		return ucRet;
	}

	memset(AllTransTable,0,sizeof(AllTransTable));

	if( bPrintAll )
	{
		lcdCls();
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  REPORTE POR MESERO  ");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	}
	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "   IMPRIMIENDO   ");
	lcdFlip();
	
   	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf((char *)"\n");
		PrnSmallFontData();
	}
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,120,(uint8_t *)Bmp_Prn);
		prnPrintf("\n");
	}
	else
	{
		prnPrintf("      SETTLE DETAIL\n");
	}

	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  

	prnPrintf("%s     TERMINAL:%s\n", stPosParam.szST,PosCom.stTrans.szPosId);  //store ID
	prnPrintf("               LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);	

	

	GetPosTime((uint8_t *)szDate1, (uint8_t *)szDate1+8);
	MakeFormatPrintDate((uint8_t *)szDate1,(uint8_t *)szDate2);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",szDate2, &szDate1[8], &szDate1[10], &szDate1[12]);

    	prnPrintf("\n\nPROPINA FINAL POR MESERO\n");
	
	
	PrnBigFontChnData();
	iNumOfOnePage = 0;

	j = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		waiterNo = stLog.waiterNo;

		switch( stLog.iTransNo )
		{				
		case PRE_TIP_SALE:
			//没有撤销
			if( stLog.ucVoidId != TRUE )
			{
				if( szTempTipAmt[waiterNo-1] == 0 )
				{
					SumMesero++;
					MeseroArr[j++] = stLog.waiterNo;
				}
				
				szTempTipAmt[waiterNo-1] += stLog.lpreTipAmount;
			}		
			break;
			
		case POS_PREAUTH:
			if(stLog.ucAuthCmlId == TRUE )
			{
				if( szTempTipAmt[waiterNo-1] == 0 )
				{
					SumMesero++;
					MeseroArr[j++] = stLog.waiterNo;
				}
				szTempTipAmt[waiterNo-1] += stLog.lPreAuthTipAmount;
			}
			else
			{
				if( szTempTipAmt[waiterNo-1] == 0 )
				{
					SumMesero++;
					MeseroArr[j++] = stLog.waiterNo;
				}
				szTempTipAmt[waiterNo-1] += 0;
			}
			break;
			
		default:
			break;
		}
	}

	for( i = 0; i < SumMesero; i++ )
	{
		j = MeseroArr[i];
		memset(PosCom.stTrans.sAmount,0,sizeof(PosCom.stTrans.sAmount));
		sprintf((char *)szAmount, "%012ld", szTempTipAmt[j-1]);
		AscToBcd(PosCom.stTrans.sAmount, szAmount, 12);
		
		memset(buf1,0,sizeof(buf1));
		ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);
		prnPrintf("Mesero:%d  .PROP FINAL:%s\n\n",MeseroArr[i],buf1);
	}
	PrnSmallFontData();
	prnPrintf("\n   DESGLOSE DE PROPINA POR TRANSACCION   \n\n\n");

	PrnSmallFontData();
	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			break;
		}

		if( !bPrintAll )
		{
			if(!ChkIfPrtDetill(stLog.iTransNo))
			{
				continue;
			}
			if( stLog.ucVoidId==TRUE )
			{
				continue;
			}
			if( stLog.ucAdjusted && stLog.ucChgFrmAdjust!=TRUE )
			{
				continue;
			}
		}

		ucRet = NetpayPrtOneTranTip(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}

	prnPrintf("\n");
	
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			PrnBigFontChnData();			
			prnPrintf("\n\n          %s\n\n",gstPosVersion.szVersion);
			prnPrintf("\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}

	return NO_DISP;
}


uint8_t queryDetail(int flag)
{
	uint8_t		ucRet;
	int		iRecNo,iKey;
	NEWPOS_LOG_STRC	stLog;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   VIEW DETAIL  ");
	lcdFlip();
	
	if( stTransCtrl.iTransNum==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO TRANS");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);					
		return E_NO_TRANS;
	}
	
	iRecNo=stTransCtrl.iTransNum-1;
	while(1)
	{
		if( iRecNo>=stTransCtrl.iTransNum )
		{
			lcdCls();	
			DispMulLanguageString(0, 3, DISP_MEDIACY|DISP_CFONT, NULL, "END OF BATCH");
			lcdFlip();
		}
		else if( iRecNo<0 )
		{
			lcdCls();	
			DispMulLanguageString(0, 3, DISP_MEDIACY|DISP_CFONT, NULL, "START OF BATCH");
			lcdFlip();
		}
		else
		{
			memset(&stLog, 0, sizeof(stLog));
			ucRet = LoadTranLog(iRecNo, &stLog);
			if( ucRet!=OK )
			{
				lcdCls();	
				return ucRet;
			}
			DispOneTrans(&stLog);
		}		
		
		iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if( iKey==KEY_CANCEL )
		{
			lcdCls();	
			return E_TRANS_CANCEL;
		}
		else if( iKey==KEY_DOWN || iKey==KEY_CLEAR  )
		{
			if( iRecNo>=stTransCtrl.iTransNum )
				break;
			iRecNo++;
		}
		else if( iKey==KEY_UP || iKey==KEY_ENTER )
		{
			if( iRecNo<0 )
				break;
			iRecNo--;
		}
	}
	return NO_DISP;
}

void DispOneTrans(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	  szBuff[45], sTranAmt[6]; 
	uint8_t   szCardTemp[20], szStatuInfo[20];
	
	lcdCls();	
	GetTransName(pstLog->iTransNo, szBuff);
	memset(szStatuInfo,0,sizeof(szStatuInfo));
	GetTransStatusInfo(pstLog,(char*)szStatuInfo);

	if (pstLog->iTransNo ==POS_AUTH_CM && pstLog->ucVoidId == TRUE)
	{
		lcdClrLine(0,2);
		lcdDisplay(0, 0, DISP_CFONT, "COMP(VOID)");
	}
	else
		lcdDisplay(0, 0, DISP_CFONT, "%s%s",(char *)szBuff,szStatuInfo);

    if ( (((pstLog->iTransNo==POS_PREAUTH) || (pstLog->iTransNo==POS_PREAUTH_ADD) )
    	 && (stPosParam.ucPreAuthMaskPan != PARAM_OPEN)) ||	
         (pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
         (pstLog->iTransNo==ICC_OFFSALE) || (pstLog->iTransNo==EC_QUICK_SALE) ||
         (pstLog->iTransNo==EC_NORMAL_SALE) )
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}

	lcdDisplay(0, 2, DISP_ASCII, "%.20s", szCardTemp);
	lcdDisplay(0, 3, DISP_ASCII, "VOUCHER NO:%06ld", pstLog->lTraceNo);
	lcdDisplay(0, 4, DISP_ASCII, "AUTH NO   :%s",    pstLog->szAuthNo);	
	
	memcpy(sTranAmt, pstLog->sAmount, 6);
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(sTranAmt, pstLog->sTipAmount, 6);
	}

	ConvBcdAmount(sTranAmt, szBuff);
	lcdDisplay(0, 5, DISP_ASCII, "AMOUNT    :%s", szBuff);
	lcdDisplay(0, 6, DISP_ASCII, "%.4s/%.2s/%.2s %.2s:%.2s:%.2s",
			&pstLog->szDate[0], 
			&pstLog->szDate[4], &pstLog->szDate[6],
			&pstLog->szTime[0], &pstLog->szTime[2], &pstLog->szTime[4]);
	lcdDisplay(0, 7, DISP_ASCII, "BATCH NO  :%06ld", pstLog->lBatchNumber);
	lcdFlip();
}

void  GetTransStatusInfo(NEWPOS_LOG_STRC *pstLog,char *pszStatus)
{
	char  szBuffer[20];

	memset(szBuffer,0,sizeof(szBuffer));

	if (pstLog->ucVoidId == TRUE)
	{
		strcpy(szBuffer,"VOID");
	}
	else
	{
		if (pstLog->ucSendFlag == TRUE && pstLog->ucAdjusted == TRUE)
		{
			strcpy(szBuffer,"ADJUST");
		}
		else
		{
			if (pstLog->ucSendFlag == TRUE || pstLog->ucSendFail == TS_NOT_CODE)
			{
				strcpy(szBuffer,"SENT");
			}
			
			if (pstLog->ucAdjusted == TRUE)
			{
				strcpy(szBuffer,"ADJUST");
			}
		}
	}

	if (strlen(szBuffer))
	{
		sprintf(pszStatus,"(%s)",szBuffer);
	}
}

uint8_t queryTrans(void)
{
	uint8_t		ucRet, szTraceNo[6+1];
	int		iCnt, iRet;
	int		lTraceNo;
	NEWPOS_LOG_STRC	stLog;
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  QUERY TRANS");

	if( stTransCtrl.iTransNum==0 )
	{
		return E_NO_TRANS;
	}
	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT TRACE:");
	lcdFlip();
	lcdGoto(72, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 1, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)szTraceNo);
	if( iRet<0 )
	{
		return E_TRANS_CANCEL;
	}
	lTraceNo = atol((char *)szTraceNo);
	
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iCnt, &stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		if( stLog.lTraceNo==lTraceNo )
		{
			break;
		}
	}
	if( iCnt>=stTransCtrl.iTransNum )
	{
		return E_NO_OLD_TRANS;
	}
	
	DispOneTrans(&stLog);
	kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  QUERY TRANS");
	
	return NO_DISP;
}

uint8_t QueryTransTotal(void)
{
	uint8_t	ucRet, szBuff[30];
	uint8_t   szTotal[10];
	int iKey;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  QUERY TOTAL");
	lcdFlip();

	ucRet = ComputeTransTotal(1);
	if( ucRet!=OK && fileExist(REVERSAL_FILE)<0)
	{
		return ucRet;
	}

	sprintf((char *)szBuff, "%03d", stTotal.iRmbDebitNum);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, " CUP DEBIT:");
	lcdDisplay(0, 4, DISP_CFONT, "NUM: %s", szBuff);
	
	memset(szTotal, 0, sizeof(szTotal));
	memcpy(szTotal, stTotal.sRmbDebitAmt, 6);
	ConvBcdAmount(szTotal, szBuff);
	lcdDisplay(0, 6, DISP_CFONT, "AMOUNT:%s", szBuff);
	lcdFlip();
	iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
	if( iKey==KEY_CANCEL )
	{
		return NO_DISP;
	}
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  QUERY TOTAL");	
	sprintf((char *)szBuff, "%03d", stTotal.iRmbCreditNum);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "CUP CREDIT:");
	lcdDisplay(0, 4, DISP_CFONT, "NUM: %s", szBuff);
	memset(szTotal, 0, sizeof(szTotal));
	memcpy(szTotal, stTotal.sRmbCreditAmt, 6);
	ConvBcdAmount(szTotal, szBuff);
	lcdDisplay(0, 6, DISP_CFONT, "AMOUNT:%s", szBuff);
	lcdFlip();
	iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
	if( iKey==KEY_CANCEL )
	{
		return NO_DISP;
	}

	lcdClrLine(2, 7);
	sprintf((char *)szBuff, "%03d", stTotal.iFrnDebitNum);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FOREIGN DEBIT:");
	lcdDisplay(0, 4, DISP_CFONT, "NUM: %s", szBuff);
	
	memset(szTotal, 0, sizeof(szTotal));
	memcpy(szTotal, stTotal.sFrnDebitAmt, 6);
	ConvBcdAmount(szTotal, szBuff);
	lcdDisplay(0, 6, DISP_CFONT, "AMOUNT:%s", szBuff);
	lcdFlip();
	iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
	if( iKey==KEY_CANCEL )
	{
		return NO_DISP;
	}
	
	lcdClrLine(2, 7);
	sprintf((char *)szBuff, "%03d", stTotal.iFrnCreditNum);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FOREIGN CREDIT:");
	lcdDisplay(0, 4, DISP_CFONT, "NUM: %s", szBuff);
	memset(szTotal, 0, sizeof(szTotal));
	memcpy(szTotal, stTotal.sFrnCreditAmt, 6);
	ConvBcdAmount(szTotal, szBuff);
	lcdDisplay(0, 6, DISP_CFONT, "AMOUNT:%s", szBuff);
	lcdFlip();
	iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
	if( iKey==KEY_CANCEL )
	{
		return NO_DISP;
	}

	return OK;
}


uint8_t CheckOutDispOrignalTrans(NEWPOS_LOG_STRC *pstLog,uint8_t ChangeAmountFlag,uint8_t *sAmount)
{
	uint8_t	szBuff[33];
	uint8_t szMaskCardNo[50];
	int     iKey;
//	uint8_t iRet;
	uint8_t szCardType[20];
	uint8_t szTitle[40];
CONFIR:
	if( pstLog->CardType == 1 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( pstLog->CardType == 2 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( pstLog->CardType == 3 )
	{
		strcpy((char*)szCardType,"AMEX");
	}
	if( PosCom.stTrans.TransFlag == POS_AUTH_CM )
	{
		sprintf((char*)szTitle,"CONFIRMACION PROPINA %s",(char*)szCardType);
	}
	else if( stTemp.iTransNo==CHECK_OUT)
	{
		sprintf((char*)szTitle,"CHECK OUT");
	}
	else
	{
		sprintf((char*)szTitle,"CANCELACION %s",(char*)szCardType);
	}
	
 
	memset(szMaskCardNo,0,sizeof(szMaskCardNo));
	MaskPan(pstLog->szCardNo,szMaskCardNo);

	lcdCls();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)szTitle);
	GetTransName(pstLog->iTransNo, szBuff);
	lcdDisplay(0, 4, DISP_CFONT, "CHECK OUT  ******%s", szMaskCardNo+6);
	if(pstLog->iTransNo==POS_PREAUTH && pstLog->ucAuthCmlId != TRUE)
	{
		ConvBcdAmount(pstLog->preAuthOriAmount, szBuff);
	}
	else 
	{
		if( ChangeAmountFlag==1 )
		{
			memcpy(pstLog->sAmount,PosCom.stTrans.sAmount,6);
		}
		ConvBcdAmount(pstLog->sAmount, szBuff);
	}
	
	lcdDisplay(0, 6, DISP_CFONT, "$ %10.10s", szBuff);
	lcdFlip();
	while(1)
	{
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		lcdDisplay(130, 8,  DISP_CFONT, "1-SI");
		lcdDisplay(130, 10, DISP_CFONT, "2-Modificar cantidad");
		if( PosCom.stTrans.TransFlag != POS_AUTH_CM )
		{
			lcdDisplay(130, 12, DISP_CFONT, "3-NO");
		}
		
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if( iKey==KEY_ENTER || iKey==KEY1 )
		{
			return OK;
		}
		else if( iKey==KEY_CANCEL || iKey==KEY_TIMEOUT )
		{
			return E_TRANS_CANCEL;
		}
		else if( iKey==KEY3 )
		{
			if( stTemp.iTransNo==CHECK_OUT )
			{
				return NO_DISP;
			}
			
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CANCELACION");
			lcdDisplay(0, 6, DISP_CFONT, "ANULAR ULTIMA TRANS?");
			lcdFlip();
			lcdDisplay(210, 10,  DISP_CFONT, "1-SI");
			lcdDisplay(210, 12, DISP_CFONT, "2-NO");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
			if( iKey==KEY_ENTER || iKey==KEY1 )
			{
				goto CONFIR;
			}
			else if( iKey==KEY2 )
			{
				return E_REINPUT;
			}
			else if( iKey==KEY_CANCEL || iKey==KEY_TIMEOUT )
			{
				return E_TRANS_CANCEL;
			}		
		}
		else if( iKey==KEY2 && stTemp.iTransNo == CHECK_OUT )
		{
			return E_REINPUT_AMOUNT;
		}
		
	}

	return OK;
}


uint8_t DispOrignalTrans(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	szBuff[33];
	uint8_t szMaskCardNo[50];
	int     iKey;
//	uint8_t iRet;
	uint8_t szCardType[20];
	uint8_t szTitle[40];
CONFIR:
	if( pstLog->CardType == 1 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( pstLog->CardType == 2 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( pstLog->CardType == 3 )
	{
		strcpy((char*)szCardType,"AMEX");
	}
	
	if( PosCom.stTrans.TransFlag == POS_AUTH_CM )
	{
		sprintf((char*)szTitle,"CONFIRMACION PROPINA %s",(char*)szCardType);
	}
	else if( stTemp.iTransNo==CHECK_OUT)
	{
		sprintf((char*)szTitle,"CHECK OUT");
	}
	else if( stTemp.iTransNo==NETPAY_ADJUST)
	{
		if( pstLog->iOldTransNo==CHECK_OUT )
		{
			sprintf((char*)szTitle,"AJUSTE");
		}
		else
		{
			sprintf((char*)szTitle,"AJUSTE %s",(char*)szCardType);
		}
	}
	else if( stTemp.iTransNo==POS_REFUND)
	{
		if( pstLog->iOldTransNo==CHECK_OUT )
		{
			sprintf((char*)szTitle,"CANCELACION");
		}
		else
		{
			sprintf((char*)szTitle,"CANCELACION %s",(char*)szCardType);
		}
	}
	
 
	memset(szMaskCardNo,0,sizeof(szMaskCardNo));
	MaskPan(pstLog->szCardNo,szMaskCardNo);

	lcdCls();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)szTitle);
	GetTransName(pstLog->iTransNo, szBuff);
	if( (stTemp.iTransNo==NETPAY_ADJUST && stPosParam.Switch_Retail==PARAM_OPEN)||
		(stTemp.iTransNo==POS_REFUND&& stPosParam.Switch_Retail==PARAM_OPEN))
	{
		lcdDisplay(0, 4, DISP_CFONT, "        ******%s", szMaskCardNo+6);
	}
	else
	{
		lcdDisplay(0, 4, DISP_CFONT, "VENTA  ******%s", szMaskCardNo+6);
	}

	
	if(pstLog->iTransNo==POS_PREAUTH && pstLog->ucAuthCmlId != TRUE)
	{
		ConvBcdAmount(pstLog->preAuthOriAmount, szBuff);
	}
	else if( stTemp.iTransNo==NETPAY_ADJUST && pstLog->TransFlag == PRE_TIP_SALE)
	{
		ConvBcdAmount(pstLog->preTipOriAmount, szBuff);
	}
	else
	{
		ConvBcdAmount(pstLog->sAmount, szBuff);
	}
	
	lcdDisplay(0, 6, DISP_CFONT, "$ %10.10s", szBuff);
	lcdFlip();
	while(1)
	{
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		lcdDisplay(210, 8,  DISP_CFONT, "1-SI");
		lcdDisplay(210, 10, DISP_CFONT, "2-NO");
		if( PosCom.stTrans.TransFlag != POS_AUTH_CM )
		{
			lcdDisplay(210, 12, DISP_CFONT, "3-PROX");
		}
		
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if( iKey==KEY_ENTER || iKey==KEY1 )
		{
			return OK;
		}
		else if( iKey==KEY_CANCEL || iKey==KEY_TIMEOUT )
		{
			return E_TRANS_CANCEL;
		}
		else if( iKey==KEY2 )
		{
			if( PosCom.stTrans.TransFlag == POS_AUTH_CM )
			{
				return NO_DISP;
			}
			
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CANCELACION");
			if( stTemp.iTransNo==NETPAY_ADJUST)
			{
				lcdDisplay(0, 6, DISP_CFONT, "AJUSTE ULTIMA TRANS?");
			}
			else
			{
				lcdDisplay(0, 6, DISP_CFONT, "ANULAR ULTIMA TRANS?");

			}
			lcdFlip();
			lcdDisplay(210, 10,  DISP_CFONT, "1-SI");
			lcdDisplay(210, 12, DISP_CFONT, "2-NO");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
			if( iKey==KEY_ENTER || iKey==KEY1 )
			{
				goto CONFIR;
			}
			else if( iKey==KEY2 )
			{
				return E_REINPUT;
			}
			else if( iKey==KEY_CANCEL || iKey==KEY_TIMEOUT )
			{
				return E_TRANS_CANCEL;
			}		
		}
		else if(iKey==KEY3 && PosCom.stTrans.TransFlag == POS_AUTH_CM )
		{
			continue;
		}
		else if( iKey==KEY3 && stTemp.iTransNo == CHECK_OUT )
		{
			return E_REINPUT_AMOUNT;
		}
		else if(iKey==KEY3 && PosCom.stTrans.TransFlag != POS_AUTH_CM )
		{
			return E_REINPUT;
		}
	}

	return OK;
}

uint8_t read_data(void* pdata, int size, char* filename)
{
	int fd;
	int ret;
	
	fd = fileOpen(filename, O_RDWR);
	if( fd<0 ) 
	{
		return (E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
	ret = fileRead(fd, (uint8_t *)pdata, size);
	fileClose(fd);
	
	if( ret!=size )
		return (E_MEM_ERR);
	
	return (OK);
}

uint8_t write_data(void* pdata, int size, char* filename)
{
	int fd;
	int ret;
	
	fd = fileOpen(filename, O_RDWR|O_CREAT);
	if( fd<0 ) 
	{
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)pdata, size);
	if( ret!=size )
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}

uint8_t ReadTermAidList(void)
{
	int		iRet, iAidFile;

	glTermAidNum = 0;
	memset(&glTermAidList[0], 0, sizeof(glTermAidList));
	iAidFile = fileOpen(TERM_AIDLIST, O_RDWR);
	if( iAidFile<0 )
	{
		return E_MEM_ERR;
	}

	iRet = fileRead(iAidFile, (uint8_t *)&glTermAidList[0], sizeof(glTermAidList));
	fileClose(iAidFile);
	if( iRet<0 )
	{
		return E_MEM_ERR;
	}
	glTermAidNum = iRet/sizeof(TermAidList);

	return OK;
}

uint8_t WriteScriptFile(void)
{
	int		fd;
		
	fd = fileOpen(ICC_SCRIPT_FILE, O_RDWR|O_CREAT);
	if( fd<0 )
	{
		return E_MEM_ERR;
	}
	
	if( fileWrite(fd, (uint8_t *)&PosCom, COM_SIZE)!=COM_SIZE )
	{
		fileClose(fd);
		return E_MEM_ERR;
	}
	fileClose(fd);
	
	return OK;
}

uint8_t ReadScriptFile(void)
{
	int		fd;
	
	fd = fileOpen(ICC_SCRIPT_FILE, O_RDWR);
	if( fd<0 )
	{
		return E_MEM_ERR;
	}
	
	if( fileRead(fd, (uint8_t *)&PosCom, COM_SIZE)!=COM_SIZE )
	{
		fileClose(fd);
		return E_MEM_ERR;
	}
	fileClose(fd);
	
	return OK;
}


uint8_t LoadCheckTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(POS_LOG_CHECK_FILE, O_RDWR);
	if( iLogFile<0 )
	{
		return E_MEM_ERR;
	}
	
	iRet = fileSeek(iLogFile, (int)(iRecNo * LOG_RECORD_LEN), SEEK_SET);
	if( iRet<0 )
	{
		fileClose(iLogFile);
		return E_MEM_ERR;
	}
	
	iRet = fileRead(iLogFile, (uint8_t *)pstTranLog, LOG_RECORD_LEN);
	fileClose(iLogFile);
	if( iRet!=LOG_RECORD_LEN )
	{
		return E_MEM_ERR;
	}
	
	return OK;
}


uint8_t LoadTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(POS_LOG_FILE, O_RDWR);
	if( iLogFile<0 )
	{
		return E_MEM_ERR;
	}
	
	iRet = fileSeek(iLogFile, (int)(iRecNo * LOG_RECORD_LEN), SEEK_SET);
	if( iRet<0 )
	{
		fileClose(iLogFile);
		return E_MEM_ERR;
	}
	
	iRet = fileRead(iLogFile, (uint8_t *)pstTranLog, LOG_RECORD_LEN);
	fileClose(iLogFile);
	if( iRet!=LOG_RECORD_LEN )
	{
		return E_MEM_ERR;
	}
	
	return OK;
}


uint8_t UpdateTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(POS_LOG_FILE, O_RDWR);
	if( iLogFile<0 )
	{
		return E_MEM_ERR;
	}
	
	iRet = fileSeek(iLogFile, (int)(iRecNo * LOG_RECORD_LEN), SEEK_SET);
	if( iRet<0 )
	{
		fileClose(iLogFile);
		return E_MEM_ERR;
	}
	
	iRet = fileWrite(iLogFile, (uint8_t *)pstTranLog, LOG_RECORD_LEN);
	fileClose(iLogFile);
	if( iRet!=LOG_RECORD_LEN )
	{
		return E_MEM_ERR;
	}
	
	return OK;
}


uint8_t UpdateCheckTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(POS_LOG_CHECK_FILE, O_RDWR);
	if( iLogFile<0 )
	{
		return E_MEM_ERR;
	}
	
	iRet = fileSeek(iLogFile, (int)(iRecNo * LOG_RECORD_LEN), SEEK_SET);
	if( iRet<0 )
	{
		fileClose(iLogFile);
		return E_MEM_ERR;
	}
	
	iRet = fileWrite(iLogFile, (uint8_t *)pstTranLog, LOG_RECORD_LEN);
	fileClose(iLogFile);
	if( iRet!=LOG_RECORD_LEN )
	{
		return E_MEM_ERR;
	}
	
	return OK;
}

static uint32_t sGetDefaultCommType(uint32_t uiCommAbility)
{
	uint32_t szAlignedCommType[] = {CT_CDMA,CT_GPRS,CT_WIFI, CT_TCPIP,
			CT_MODEM, CT_MODEMPPP, CT_RS232,
	};
	uint32_t szAlignedCommType1[] = {CT_TCPIP,CT_GPRS, CT_CDMA, CT_WIFI,
			CT_MODEM, CT_MODEMPPP, CT_RS232,
	};
	int i;
	int TypesCounts;
	
	if( gstPosCapability.uiPosType == 5 )  //7210 Default GPRS
	{
		TypesCounts = sizeof(szAlignedCommType)/sizeof(uint32_t);
		for(i=0; i<TypesCounts; ++i)
		{
			if(szAlignedCommType[i]&uiCommAbility)
			{
				return szAlignedCommType[i];
			}
		}
	}
	else if( gstPosCapability.uiPosType == 1 )  //8110PS Default TCP
	{
		TypesCounts = sizeof(szAlignedCommType1)/sizeof(uint32_t);
		for(i=0; i<TypesCounts; ++i)
		{
			if(szAlignedCommType1[i]&uiCommAbility)
			{
				return szAlignedCommType1[i];
			}
		}
	}
	else
	{
		TypesCounts = sizeof(szAlignedCommType)/sizeof(uint32_t);
		for(i=0; i<TypesCounts; ++i)
		{
			if(szAlignedCommType[i]&uiCommAbility)
			{
				return szAlignedCommType[i];
			}
		}
	}
	
	return 0;
}


void  LoadDefaultAppParam(void)
{
	unsigned long       ulTimeOut;
	uint8_t sn_num[9] ={0};
	
	memset(&stPosParam,0,sizeof(stPosParam));
	
	ulTimeOut = stPosParam.ucCommWaitTime;
	strcpy((char *)stPosParam.szPhoneNum1,"0625");
	strcpy((char *)stPosParam.szPhoneNum2,"0625");
	strcpy((char *)stPosParam.szPhoneNum3,"0625");
	stPosParam.szExtNum[0] = 0;
	stPosParam.szMPhone[0] = 0;
	strcpy((char *)stPosParam.szDownloadTel,"0625");
	strcpy((char *)stPosParam.szUnitNum,"000000000000000");
	strcpy((char *)stPosParam.szPosId,"00000000");
	stPosParam.ucSendOffFlag = PARAM_CLOSE;
	memcpy((char *)stPosParam.sTpdu,"\x60\x00\x03\x00\x00", 5);
	stPosParam.ucTipOpen  = PARAM_OPEN ;
	memcpy((char *)stPosParam.sTransOpen,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",8); 	 
	stPosParam.ucAutoLogoff = PARAM_OPEN;	
	stPosParam.ucManualInput = PARAM_CLOSE ;
	stPosParam.ucErrorPrt = PARAM_CLOSE;
	stPosParam.ucDetailPrt = PARAM_OPEN;
	stPosParam.ucOfflineSettleFlag = PARAM_OPEN;
	strcpy((char *)stPosParam.szUnitChnName,"NEWPOS TEST MERCHANT");
	strcpy((char *)stPosParam.szEngName,"NEWPOS TEST MERCHANT");	
	stPosParam.ucTestFlag = PARAM_CLOSE; 	
	stPosParam.ucVoidSwipe  = PARAM_OPEN;	
	stPosParam.ucAuthVoidSwipe  = PARAM_OPEN;	
	stPosParam.ucConfirm = '0';	
	stPosParam.ucVoidPin =  PARAM_OPEN;	
	stPosParam.ucPreVoidPin = PARAM_OPEN;
	stPosParam.ucPreComVoidPin = PARAM_OPEN;
	stPosParam.ucPreComPin = PARAM_OPEN;
	stPosParam.ucKeyMode = KEY_MODE_3DES;    
	stPosParam.ucEmvSupport = PARAM_OPEN ;
	stPosParam.ucqPbocSupport = PARAM_OPEN ;
	strcpy((char *)stPosParam.szParamsPwd,"000000");
	strcpy((char *)stPosParam.szManagePwd,"567347");
	strcpy((char *)stPosParam.szSafepwd, "000000");
 	strcpy((char *)stPosParam.szTpdu,"6000030000");  
	strcpy((char *)stPosParam.szAsyncTpdu,"6000030000");  
	strcpy((char *)stPosParam.szTransCtl,"FFFFFFFFFFFFFFFF"); 
	stPosParam.bPreDial = PARAM_OPEN ;     
	stPosParam.ucTicketNum = '2';	
	stPosParam.ucOprtLimitTime = 60;  
	stPosParam.ucCommWaitTime = 30 ; 
	stPosParam.ucTipper = 10;		
	stPosParam.ucAdjustPercent = 0;	
	stPosParam.ucResendTimes = 2;
	stPosParam.ucOfflineSendTimes = 2;
	stPosParam.ucDialRetryTimes = 2;	
	stPosParam.ucKeyIndex = 0;		
	stPosParam.iMaxTransTotal = 500;  
	stPosParam.lNowTraceNo = 12; 	
	stPosParam.lNowBatchNum = 2;	
	stPosParam.lMaxRefundAmt = 1000000;
	stPosParam.iEcMaxAmount = 10000;
	stPosParam.iDispLanguage = 0;  
	stPosParam.ucMaxOfflineTxn = 1;  
	stPosParam.ucPrnTitleFlag = PARAM_OPEN;
	stPosParam.ucPreAuthMaskPan=PARAM_CLOSE;
	stPosParam.ucSalePassive=PARAM_OPEN;
	strcpy((char *)stPosParam.szPrnTitle,"POS");
	stPosParam.ucSupportElecSign = PARAM_OPEN;
	stPosParam.ucTelIndex = 0;
	stPosParam.ucTCPIndex = 0;
	stPosParam.ucTrackEncrypt = PARAM_OPEN;
	stPosParam.ucDefaltTxn = PARAM_OPEN;
	stPosParam.ucEnterSupPwd = PARAM_OPEN;
	stPosParam.ucReversalTime = PARAM_CLOSE;
	stPosParam.ucPrnNegative = PARAM_OPEN;
	stPosParam.ucDownBlkCard = PARAM_CLOSE;
	stPosParam.ucLongConnection = PARAM_OPEN;
	stPosParam.ucSupPinpad = PARAM_CLOSE;
	stPosParam.ucSetFastKey = PARAM_OPEN;
	stPosParam.open_internet = PARAM_OPEN;

	stPosParam.No_Promotions=PARAM_CLOSE;
	stPosParam.Costom_Promotions=PARAM_OPEN;
	stPosParam.MonthFlag3=PARAM_OPEN;
	stPosParam.MonthFlag6=PARAM_OPEN;
	stPosParam.MonthFlag9=PARAM_OPEN;
	stPosParam.MonthFlag12=PARAM_OPEN;
	stPosParam.MonthFlag18=PARAM_OPEN;
	stPosParam.CheckInFlag=PARAM_CLOSE;
	stPosParam.CheckOutFlag=PARAM_CLOSE;
	stPosParam.Netpay_RefundFlag=PARAM_OPEN;
	stPosParam.Switch_Retail=PARAM_CLOSE;
	stPosParam.Switch_Hotel=PARAM_CLOSE;
	stPosParam.Switch_Restaurant=PARAM_CLOSE;
	stPosParam.Switch_USD=PARAM_CLOSE;
	
	stPosParam.iElecSignReSendTimes = 1;
	stPosParam.iElecSignTimeOut = 40;
	stPosParam.ucPINMAC = 0;
	stPosParam.ucCommHostFlag = PARAM_CLOSE;
	strcpy((char *)stPosParam.stTxnCommCfg.stTcpIpPara.szDNS, "app.newpostech.com");
	stPosParam.ucWnetMode = PARAM_OPEN; 

	stPosParam.stTxnCommCfg.ucCommType  = sGetDefaultCommType(gstPosCapability.uiCommType);
	stPosParam.stTxnCommCfg.stRS232Para.ucPortNo   = gstPosCapability.uiPortType;
	stPosParam.stTxnCommCfg.stRS232Para.ucSendMode = CM_SYNC;
	stPosParam.stTxnCommCfg.ulSendTimeOut = 60;		
	stPosParam.stDownParamCommCfg.ulSendTimeOut = 60;
	strcpy((char *)stPosParam.stTxnCommCfg.stRS232Para.szAttr, "9600,8,n,1");
	strcpy((char *)stPosParam.szPOSRemoteIP,  "210.12.207.186");
	strcpy((char *)stPosParam.szPOSRemotePort, "9000");
	strcpy((char *)stPosParam.szPOSRemoteIP2,  "210.12.207.186");
	strcpy((char *)stPosParam.szPOSRemotePort2, "9000");
	
	strcpy((char *)stPosParam.stTxnCommCfg.stWirlessPara.szAPN, "CMNET");
	strcpy((char *)stPosParam.stTxnCommCfg.stWirlessPara.szUID, "CARD");
	strcpy((char *)stPosParam.stTxnCommCfg.stWirlessPara.szPwd, "CARD");

	strcpy((char *)stPosParam.stTxnCommCfg.stPSTNPara.szTxnTelNo,   "82790625");
	stPosParam.stTxnCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_SYNC;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_mode = MODEM_COMM_SYNC;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_speed = 1200;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_mode = MODEM_DAIL_DTMF;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_pause = 1;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_timeo = 30;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.extension_chk = 1;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.idle_timeo = 60;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.region = ModemRegion(USA);
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dtmf_level = 10;
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.t_answertone = 10;
	
	stPosParam.stDownParamCommCfg.ucCommType = sGetDefaultCommType(gstPosCapability.uiCommType);
	strcpy((char *)stPosParam.stDownParamCommCfg.stWirlessPara.szAPN, "CMNET");
	strcpy((char *)stPosParam.stDownParamCommCfg.stWirlessPara.szRemoteIP,   "210.22.14.210");
	strcpy((char *)stPosParam.stDownParamCommCfg.stWirlessPara.szRemotePort, "9006");
	strcpy((char *)stPosParam.stDownParamCommCfg.stWirlessPara.szUID, "CARD");
	strcpy((char *)stPosParam.stDownParamCommCfg.stWirlessPara.szPwd, "CARD");
	stPosParam.stDownParamCommCfg.stRS232Para.ucPortNo   = gstPosCapability.uiPortType;
	stPosParam.stDownParamCommCfg.stRS232Para.ucSendMode = CM_SYNC;
	strcpy((char *)stPosParam.stDownParamCommCfg.stRS232Para.szAttr, "9600,8,n,1");
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.connect_speed = MODEM_CONNECT_33600BPS;
	stPosParam.stVolContrFlg = PARAM_OPEN;
	
	strcpy((char*)stPosParam.szDialNum,"*99***1#");
	IfSupportPinpad();
	if(stPosParam.ucSupportElecSign==PARAM_OPEN)
	{
		if(stPosParam.iElecSignTimeOut<=20)
		{
			stPosParam.iElecSignTimeOut = 20;
		}
	}

	sys_get_sn(sn_num, 9);
	PrintDebug("%s %s","sn_num:",sn_num);
	PrintDebug("%s %d","sn_num_length:",strlen(sn_num));

	memcpy((char *)stPosParam.szPosId,sn_num,strlen(sn_num));
	PrintDebug("%s %s","stPosParam.szPosId:",stPosParam.szPosId);
	SaveAppParam();
}

void  AdjustAppParam(void)
{
	stPosParam.iDispLanguage = 0;
	if (stPosParam.ucOprtLimitTime < 5)
	{
		stPosParam.ucOprtLimitTime = 5;
	}

	AscToBcd(stPosParam.sTpdu,stPosParam.szTpdu,10);
	AscToBcd(stPosParam.sTransOpen,stPosParam.szTransCtl,16);
	stPosParam.ucTicketNum |= 0x30;
	stPosParam.ucDialRetryTimes = stPosParam.ucDialRetryTimes & 0x0F;
	stPosParam.ucTelIndex = 0;
	stPosParam.ucTCPIndex = 0;
	stPosParam.ucWnetMode = PARAM_OPEN;
	strcpy((char*)stPosParam.szDialNum,"*99***1#");

	stPosParam.stTxnCommCfg.ucCommType = sGetDefaultCommType(gstPosCapability.uiCommType);

	//stPosParam.stTxnCommCfg.ucCommType = 'W';//先写死用于测试
	switch (stPosParam.stTxnCommCfg.ucCommType)
	{
	case 'G':
	case 'g':
		stPosParam.stTxnCommCfg.ucCommType = CT_GPRS;
		break;
	case 'C':
	case 'c':
		stPosParam.stTxnCommCfg.ucCommType = CT_CDMA;
		break;
	case 't':
	case 'T':
		stPosParam.stTxnCommCfg.ucCommType = CT_TCPIP;
		break;
	case 'r':
	case 'R':
		stPosParam.stTxnCommCfg.ucCommType = CT_RS232;
		break;
	case 'w':
	case 'W':
		stPosParam.stTxnCommCfg.ucCommType = CT_WIFI;
		break;
	case 'm':
	case 'M':
		stPosParam.stTxnCommCfg.ucCommType = CT_MODEM;
		break;
	default:
		stPosParam.stTxnCommCfg.ucCommType = sGetDefaultCommType(gstPosCapability.uiCommType);
		break;
	}

	if(!(gstPosCapability.uiCommType & stPosParam.stTxnCommCfg.ucCommType))
	{
		stPosParam.stTxnCommCfg.ucCommType = sGetDefaultCommType(gstPosCapability.uiCommType);
	}

	if (gstPosCapability.uiCommType & CT_MODEM)
	{
		stPosParam.stTxnCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_SYNC;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_mode = MODEM_COMM_SYNC;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_speed = 1200;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_mode = MODEM_DAIL_DTMF;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_pause = 1;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_timeo = 30;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.extension_chk = 1;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.idle_timeo = 60;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.region = ModemRegion(USA);
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dtmf_level = 10;
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.t_answertone = 10;
		stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.connect_speed = MODEM_CONNECT_33600BPS;
		stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_SYNC;
	}
	
	stPosParam.stDownParamCommCfg.stRS232Para.ucPortNo   = gstPosCapability.uiPortType;
	stPosParam.stDownParamCommCfg.stRS232Para.ucSendMode = CM_SYNC;
	strcpy((char *)stPosParam.stDownParamCommCfg.stRS232Para.szAttr, "9600,8,n,1");
	strcpy((char *)stPosParam.stDownParamCommCfg.stTcpIpPara.szRemoteIP, (char *)stPosParam.stDownParamCommCfg.stWirlessPara.szRemoteIP);
	strcpy((char *)stPosParam.stDownParamCommCfg.stTcpIpPara.szRemotePort, (char *)stPosParam.stDownParamCommCfg.stWirlessPara.szRemotePort);
	stPosParam.stDownParamCommCfg.stWIFIIpPara  = stPosParam.stDownParamCommCfg.stTcpIpPara;
	stPosParam.stDownParamCommCfg.ucCommType  = stPosParam.stTxnCommCfg.ucCommType;	

	if (!gstPosCapability.uiUnContactlessCard)
	{
		stPosParam.ucqPbocSupport = PARAM_CLOSE;
	}

	IfSupportPinpad();
	if(stPosParam.ucSupportElecSign==PARAM_OPEN)
	{
		if(stPosParam.iElecSignTimeOut<=20)
		{
			stPosParam.iElecSignTimeOut = 20;
		}
	}
	return ;
}


int  SaveAppParam(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_APP_LOG, O_RDWR|O_CREAT);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)&stPosParam, sizeof(struct  _NEWPOS_PARAM_STRC));
	if( ret!=sizeof(struct  _NEWPOS_PARAM_STRC) )
	{
		DispFileErrInfo();
		fileClose(fd);	
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}


int   LoadAppParam(void)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_APP_LOG, O_RDWR);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
	ret = fileRead(fd, (uint8_t *)&stPosParam, sizeof(struct _NEWPOS_PARAM_STRC));
	fileClose(fd);
	
	if( ret!=sizeof(struct _NEWPOS_PARAM_STRC) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	return (OK);
}


int  emvSaveAIDList(int iNo, const struct terminal_aid_info *info)
{
	int		iRet, iLogFile;

	iLogFile = fileOpen(FILE_EMV_AID, O_RDWR|O_CREAT);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}

	iRet = fileSeek(iLogFile, (iNo*sizeof(*info)), SEEK_SET);
	if( iRet<0 ) 
	{
		DispFileErrInfo();
		fileClose(iLogFile);
		return (E_MEM_ERR);
	}

	iRet = fileWrite(iLogFile, (uint8_t *)info, sizeof(*info));
	fileClose(iLogFile);
	if( iRet!=sizeof(*info) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	

	return OK;
} 

int emvLoadAIDList(int iNo, struct terminal_aid_info *info)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(FILE_EMV_AID, O_RDWR);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}

	iRet = fileSeek(iLogFile, iNo*sizeof(*info), SEEK_SET);
	if( iRet<0 ) 
	{
		DispFileErrInfo();
		fileClose(iLogFile);
		return (E_MEM_ERR);
	}

	iRet = fileRead(iLogFile, (uint8_t *)info, sizeof(*info));
	fileClose(iLogFile);
	if( iRet!=sizeof(*info) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	

	return OK;
} 

int  emvSaveCAPKList(int iNo, const struct issuer_ca_public_key *info)
{
	int		iRet, iLogFile;

	
	iLogFile = fileOpen(FILE_EMV_CAPK, O_RDWR|O_CREAT);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}

	iRet = fileSeek(iLogFile, (iNo*sizeof(*info)), SEEK_SET);
	if( iRet<0 ) 
	{
		DispFileErrInfo();
		fileClose(iLogFile);
		return (E_MEM_ERR);
	}

	iRet = fileWrite(iLogFile, (uint8_t *)info, sizeof(*info));
	fileClose(iLogFile);
	if( iRet!=sizeof(*info) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	

	return OK;
} 

int emvLoadCAPKList(int iNo, struct issuer_ca_public_key *info)
{
	int		iRet, iLogFile;
	
	iLogFile = fileOpen(FILE_EMV_CAPK, O_RDWR);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}

	iRet = fileSeek(iLogFile, iNo*sizeof(*info), SEEK_SET);
	if( iRet<0 ) 
	{
		DispFileErrInfo();
		fileClose(iLogFile);
		return (E_MEM_ERR);
	}

	iRet = fileRead(iLogFile, (uint8_t *)info, sizeof(*info));
	fileClose(iLogFile);
	if( iRet!=sizeof(*info) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	

	return OK;
} 

void   ClearAllTxnStatus(void)
{
	int iCnt;

	for (iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		stTransCtrl.sTxnRecIndexList[iCnt] = INV_TXN_INDEX;
	}
}

void   SetTxnCtrlStatus(POS_COM *pstTxnLog)
{
	uint32_t  uiStatus  = 0;

	if( pstTxnLog->stTrans.iTransNo ==OFF_SALE || pstTxnLog->stTrans.iTransNo ==OFF_ADJUST ||
		pstTxnLog->stTrans.iTransNo ==ADJUST_TIP )
	{
		uiStatus |= TS_TXN_OFFLINE;
	}

	stTransCtrl.sTxnRecIndexList[pstTxnLog->stTrans.uiTxnRecIndex] = uiStatus;
	SaveCtrlParam();
}

void   UpdataTxnCtrlStatus(uint32_t uiTxnIndex,uint32_t uiStatus)
{
	if (uiTxnIndex >= MAX_TRANLOG)
	{
		return;
	}

	stTransCtrl.sTxnRecIndexList[uiTxnIndex] = uiStatus;
	SaveCtrlParam();
}

int    GetOfflineTxnNums(uint32_t uiTxnType)
{
	int iCnt , iTotalNum, iSendTimes;

	iTotalNum = 0;
	if(stTemp.iTransNo==CHECK_IN || stTemp.iTransNo==CHECK_OUT )
	{
		for(iCnt=0; iCnt<stTransCtrl.iCheckTransNum; iCnt++)
		{
			if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
			{
				continue;
			}

			iSendTimes = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
	 		if (iSendTimes>=stPosParam.ucOfflineSendTimes+1)
			{
				continue;
			}

			if ((stTransCtrl.sTxnRecIndexList[iCnt]&TS_TXN_OFFLINE)&&(uiTxnType&TS_TXN_OFFLINE))
			{
				iTotalNum++;
			}

			if ((stTransCtrl.sTxnRecIndexList[iCnt]&TS_ICC_OFFLINE) && (uiTxnType&TS_ICC_OFFLINE))
			{
				iTotalNum++;
			}

			if ((stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_TXN) && 
				(!(stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_FAIL_TXN))
				&& (uiTxnType&TS_ELECSIGN_TXN))
			{
				iTotalNum++;
			}
		}
	}
	else
	{
		for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
		{
			if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
			{
				continue;
			}

			iSendTimes = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
	 		if (iSendTimes>=stPosParam.ucOfflineSendTimes+1)
			{
				continue;
			}

			if ((stTransCtrl.sTxnRecIndexList[iCnt]&TS_TXN_OFFLINE)&&(uiTxnType&TS_TXN_OFFLINE))
			{
				iTotalNum++;
			}

			if ((stTransCtrl.sTxnRecIndexList[iCnt]&TS_ICC_OFFLINE) && (uiTxnType&TS_ICC_OFFLINE))
			{
				iTotalNum++;
			}

			if ((stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_TXN) && 
				(!(stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_FAIL_TXN))
				&& (uiTxnType&TS_ELECSIGN_TXN))
			{
				iTotalNum++;
			}
		}
	}
	

	return iTotalNum;
}

int    GetElecSignFailTxnNums(void)
{
	int iCnt , iTotalNum, iSendTimes;
	
	iTotalNum = 0;
	for(iCnt=0; iCnt<=stTransCtrl.iTransNum; iCnt++)
	{
		if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
		{
			continue;
		}
		
		iSendTimes = stTransCtrl.sTxnRecIndexList[iCnt] & 0x000000F0;
		iSendTimes = iSendTimes>>4;
		
		if (iSendTimes>=stPosParam.iElecSignReSendTimes+1)
		{
			continue;
		}
		
		if (stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_FAIL_TXN)
		{
			iTotalNum++;
		}
	}	

	return iTotalNum;
}

int    CheckElecSignFailTxn(void)
{
	int iCnt , iTotalNum;
	
	iTotalNum = 0;
	for(iCnt=0; iCnt<=stTransCtrl.iTransNum; iCnt++)
	{
		if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
		{
			continue;
		}
		
		if (stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_FAIL_TXN)
		{
			iTotalNum++;
		}
	}
	
	return iTotalNum;
}

int     SaveBlkCard(ST_BLACKLIST_TOTAL *pstBlkCard)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_BLKCARD_LOG, O_RDWR|O_CREAT);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	ret = fileWrite(fd, (uint8_t *)pstBlkCard, sizeof(ST_BLACKLIST_TOTAL));
	if( ret!=sizeof(ST_BLACKLIST_TOTAL) )
	{
		DispFileErrInfo();
		fileClose(fd);	
		return (E_MEM_ERR);
	}
	fileClose(fd);
	return (OK);
}


int     LoadBlkCard(ST_BLACKLIST_TOTAL *pstBlkCard)
{
	int fd;
	int ret;
	
	fd = fileOpen(FILE_BLKCARD_LOG, O_RDWR);
	if( fd<0 ) 
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	ret = fileSeek(fd, (int)0, SEEK_SET);
	if( ret<0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}
	
	ret = fileRead(fd, (uint8_t *)pstBlkCard, sizeof(ST_BLACKLIST_TOTAL));
	fileClose(fd);
	
	if( ret!=sizeof(ST_BLACKLIST_TOTAL) )
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	return (OK);
}

int    GetMinSendTimesTxn(uint32_t uiTxnType)
{
	int iCnt , iMinSends, iSendTimes;
	int  iTxnIndex;
	
	iTxnIndex = -1;
	iMinSends = 0x0F;
	for(iCnt=0; iCnt<stTransCtrl.iTransNum; iCnt++)
	{
		if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
		{
			continue;
		}

		if (!(stTransCtrl.sTxnRecIndexList[iCnt] & uiTxnType))
		{
			continue;
		}
		
		iSendTimes = stTransCtrl.sTxnRecIndexList[iCnt] & 0x0000000F;
		if (iSendTimes >= stPosParam.ucOfflineSendTimes+1)
		{
			continue;
		}

		if (iMinSends > iSendTimes)
		{
			iMinSends = iSendTimes;
			iTxnIndex = iCnt;
		}
	}
	
	return iTxnIndex;
}



int    GetCheckElecSignTxnNums(void)
{
	int iCnt, iTotalNum;
	
	iTotalNum = 0;
	for(iCnt=0; iCnt<=stTransCtrl.iCheckTransNum; iCnt++)
	{
		if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
		{
			continue;
		}
		
		if (stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_TXN)
		{
			iTotalNum++;
		}
	}

	return iTotalNum+1;
}



int    GetElecSignTxnNums(void)
{
	int iCnt, iTotalNum;
	
	iTotalNum = 0;
	for(iCnt=0; iCnt<=stTransCtrl.iTransNum; iCnt++)
	{
		if (stTransCtrl.sTxnRecIndexList[iCnt] == INV_TXN_INDEX)
		{
			continue;
		}
		
		if (stTransCtrl.sTxnRecIndexList[iCnt]&TS_ELECSIGN_TXN)
		{
			iTotalNum++;
		}
	}

	return iTotalNum+1;
}


void  ClearAllTxnUploadNum(void)
{
	int iCnt;
	
	for (iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		stTransCtrl.sTxnRecIndexList[iCnt] &= 0xFFFFFFF0;
	}
}

uint8_t SaveUplTraceSync(void)
{
  int fd;
  int ret;

  fd = fileOpen(FILE_UPL_TRACE, O_RDWR|O_CREAT);
  if ( fd<0 )
  {
    DispFileErrInfo();
    return (E_MEM_ERR);
  }

  ret = fileSeek(fd, (int)0, SEEK_SET);
  if ( ret<0 )
  {
    DispFileErrInfo();
    fileClose(fd);
    return (E_MEM_ERR);
  }
  ret = fileWrite(fd, (uint8_t *)&stUplTraceSync, sizeof(struct _st_UPLTRACENO_SYNC));
  if ( ret!=sizeof(struct _st_UPLTRACENO_SYNC) )
  {
    DispFileErrInfo();
    fileClose(fd);
    return (E_MEM_ERR);
  }
  fileClose(fd);
  return (OK);
}

uint8_t ReadUplTraceSync(void)
{
  int fd;
  int ret;

  fd = fileOpen(FILE_UPL_TRACE, O_RDWR);
  if ( fd<0 )
  {
    memset((uint8_t *)&stUplTraceSync, 0, sizeof(struct _st_UPLTRACENO_SYNC));
    SaveUplTraceSync();
    return (E_MEM_ERR);
  }
  ret = fileSeek(fd, (int)0, SEEK_SET);
  if ( ret<0 )
  {
    DispFileErrInfo();
    fileClose(fd);
    return (E_MEM_ERR);
  }

  ret = fileRead(fd, (uint8_t *)&stUplTraceSync, sizeof(struct _st_UPLTRACENO_SYNC));
  fileClose(fd);

  if ( ret!=sizeof(struct _st_UPLTRACENO_SYNC) )
  {
    DispFileErrInfo();
    return (E_MEM_ERR);
  }

  return (OK);
}

uint8_t ReadNetpay( char *temp)
{
  int fd;
  int ret;
  int szfileSize = 0;  // 文件大小

  fd = fileOpen(FILE_NETPAY_PARA, O_RDWR);
  if ( fd<0 )
  {
    return (E_MEM_ERR);
  }

  szfileSize = fileSize(FILE_NETPAY_PARA);
  if ( fd<0 )
  {
    return (E_MEM_ERR);
  }
  
  ret = fileSeek(fd, (int)0, SEEK_SET);
  if ( ret<0 )
  {
    DispFileErrInfo();
    fileClose(fd);
    return (E_MEM_ERR);
  }

  ret = fileRead(fd, (uint8_t *)temp, szfileSize);
  fileClose(fd);

  if ( ret!=szfileSize )
  {
    DispFileErrInfo();
    return (E_MEM_ERR);
  }

  fileRemove(FILE_NETPAY_PARA);   //处理完TMS参数文件后移除
  return (OK);
}

uint8_t DealTmsParam(uint8_t *temp)
{
	uint8_t szVersionMsg[33]; 
	int     szParamNum;    // 参数个数
	int     szParamLen;    //参数长度
	int     szParamiIdx;   //参数标签偏移量
	int     szParamContentiIdx;   //参数内容偏移量  
	int     szParamContentLen;    //  参数内容长度
	int 	iIdx = 0;      // 偏移量
	int  	i;

	if( temp == NULL )
	{
		return NO_DISP;
	}

	iIdx = 0;
	memcpy(szVersionMsg,temp,iIdx+32);  // 版本信息
	iIdx += 32;

	szParamNum = temp[iIdx]+temp[iIdx+1]*256;   // 参数个数
	iIdx += 2;

	for( i = 0; i < szParamNum; i++ )
	{
		szParamLen  = 0;
		szParamiIdx = 0;
		szParamContentiIdx = 0;
		szParamContentLen  = 0;
		szParamiIdx = temp[iIdx]+temp[iIdx+1]*256;   // 标签i偏移地址
		iIdx += 2;

		szParamLen  = temp[iIdx]+temp[iIdx+1]*256;   // 标签i长度
		iIdx += 2;
		
		if( memcmp(temp+szParamiIdx,"01000099",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			memset(stPosParam.szUnitNum,0,sizeof(stPosParam.szUnitNum));
			memcpy(stPosParam.szUnitNum,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666001",szParamLen) == 0 )	//print head1
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			memset(stPosParam.stHeader1,0,sizeof(stPosParam.stHeader1));
			memcpy(stPosParam.stHeader1,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666002",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;

			memset(stPosParam.stAddress1,0,sizeof(stPosParam.stAddress1));
			memcpy(stPosParam.stAddress1,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666003",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.stAddress2,0,sizeof(stPosParam.stAddress2));
			memcpy(stPosParam.stAddress2,temp+szParamContentiIdx,szParamContentLen);		
		}
		else if( memcmp(temp+szParamiIdx,"66666004",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.stAddress3,0,sizeof(stPosParam.stAddress3));
			memcpy(stPosParam.stAddress3,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666005",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;

			memset(stPosParam.stAffId,0,sizeof(stPosParam.stAffId));
			memcpy(stPosParam.stAffId,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666006",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;

			memset(stPosParam.stTerminalId,0,sizeof(stPosParam.stTerminalId));
			memcpy(stPosParam.stTerminalId,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666007",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;

			memset(stPosParam.stETerm,0,sizeof(stPosParam.stETerm));
			memcpy(stPosParam.stETerm,temp+szParamContentiIdx,szParamContentLen);		
		}
		else if( memcmp(temp+szParamiIdx,"66666008",szParamLen) == 0 )  //电子签名
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;

			memset(stPosParam.stETId,0,sizeof(stPosParam.stETId));
			memcpy(stPosParam.stETId,temp+szParamContentiIdx,szParamContentLen);	
		}
		else if( memcmp(temp+szParamiIdx,"66666009",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;


			memset(stPosParam.stPCpass,0,sizeof(stPosParam.stPCpass));
			memcpy(stPosParam.stPCpass,temp+szParamContentiIdx,szParamContentLen);
		}		//33333009已经被使用
		else if( memcmp(temp+szParamiIdx,"66666010",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.stPCPid,0,sizeof(stPosParam.stPCPid));
			memcpy(stPosParam.stPCPid,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666011",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.stPCSerial,0,sizeof(stPosParam.stPCSerial));
			memcpy(stPosParam.stPCSerial,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666012",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.stSodexoTId,0,sizeof(stPosParam.stSodexoTId));
			memcpy(stPosParam.stSodexoTId,temp+szParamContentiIdx,szParamContentLen);		
		}
		else if( memcmp(temp+szParamiIdx,"66666013",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.szPOSRemoteIP2,0,sizeof(stPosParam.szPOSRemoteIP2));
			memcpy(stPosParam.szPOSRemoteIP2,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666014",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			memset(stPosParam.szPOSRemotePort2,0,sizeof(stPosParam.szPOSRemotePort2));
			memcpy(stPosParam.szPOSRemotePort2,temp+szParamContentiIdx,szParamContentLen);
		}
		else if( memcmp(temp+szParamiIdx,"66666015",szParamLen) == 0 )
		{
			szParamContentiIdx  = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容偏移地址
			iIdx += 2;

			szParamContentLen = temp[iIdx]+temp[iIdx+1]*256;   // 标签内容长度
			iIdx += 2;
			
			stPosParam.stVolContrFlg= temp[szParamContentiIdx];
		}
		else
		{
			iIdx +=4;      //没有该标签跳到下一个标签
		}	
	}
	
	return OK;
}

// end of file
