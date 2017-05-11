
#include "posapi.h"
#include "posmacro.h"
#include "glbvar.h"
#include "terstruc.h"
#include "tranfunclist.h"
#include "st8583.h"
#include "posappapi.h"
#include "emvtranproc.h"
#include "wireless.h"

uint8_t OnlineCommProc(void)
{
	uint8_t	ucRet;
	int16_t	PinEntryTimes;
	int     iRet;
	if( PosCom.ucSwipedFlag==CARD_INSERTED && 
		(stTemp.iTransNo==POS_SALE || 
		stTemp.iTransNo==POS_PREAUTH || 
		stTemp.iTransNo==POS_QUE   ||
		stTemp.iTransNo==CHECK_IN  ||
		stTemp.iTransNo==CHECK_OUT  ||
		stTemp.iTransNo==POS_INSTALLMENT ||
		stTemp.iTransNo==CASH_ADVACNE ||
		stTemp.iTransNo==NETPAY_REFUND ||
		stTemp.iTransNo == PURSE_SALE||
		stTemp.iTransNo== PURSE_GETBALANCE))
	{
		if(PosCom.stTrans.isCtlplat==0)
		{
			return ProcEmvTran();
		}
	}

	if ( PosCom.ucSwipedFlag==CARD_PASSIVE && PosCom.stTrans.ucQPBOCorPBOC==1 && stTemp.iTransNo!=EC_REFUND)
	{
		ucRet = ProcEmvTran();
		sysBeef(6, 200);
		lcdClrLine(2,7);
		lcdDisplay(0, 4, DISP_MEDIACY | DISP_CFONT, "RETIRE TARJETA");
		lcdFlip();
		contactless_poweroff(imif_fd);
		contactless_close(imif_fd);
		imif_fd = -1;
		return ucRet;
	}

	if (PosCom.ucSwipedFlag == CARD_PASSIVE)
	{
		if (stTemp.iTransNo!=EC_REFUND)
		{
			return ProcQpbocTran();
		}
	}
	if(PosCom.stTrans.isCtlplat== 0)  //流量控制不需要上送脚本和冲正
	{	
	
		ucRet = SendScriptResult();
		if (ucRet == E_SCRIPT_FAIL)
		{
			return ucRet;
		}
		
		ucRet = ReverseTranProc();
		if (ucRet == E_REVERSE_FAIL)
		{
			return ucRet;
		}
	
	}

	DispTransName();
	lcdFlip();

	PinEntryTimes = 0;
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

uint8_t ReverseTranProc(void)
{
	uint8_t ucTryCnt, ucRet=0;

	if( fileExist(REVERSAL_FILE)<0 )
	{
		return 1;
	}
	//needn't reversal
	return 1;
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  AUTO REVERSAL ");
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "AUTO REVERSAL... ");
	lcdFlip();
	PosCom.stTrans.iTransNo = stTemp.iTransNo;

	memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));
	memcpy(&BackPosCom, &PosCom, COM_SIZE);
	for(ucTryCnt=1; ucTryCnt<stPosParam.ucResendTimes+2; ucTryCnt++) 
	{
		ucRet = ReadReversalTranData();	// fileRead reversal data
		if( ucRet!=OK )
		{
			break;
		}
		
		stTemp.iTransNo = POS_REVERSE;
		
		MakeReversalPack();
		ucRet = SendRecvPacket();
		if( ucRet!=OK )
		{
			if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
			{
				return E_REVERSE_FAIL;
			}
			return E_REVERSE_FAIL;
		}
		// 接收到响应码就删除冲正文件(必须确保冲正文件完整)
		if( /*memcmp(PosCom.szRespCode, "00", 2)==0 || memcmp(PosCom.szRespCode, "12", 2)==0 ||
			memcmp(PosCom.szRespCode, "25", 2)==0 */strlen((char*)PosCom.szRespCode) == 2)
		{
			ucRet = OK;
			break;
		}
		else
		{
			return E_REVERSE_FAIL;
		}
	}

	fileRemove(REVERSAL_FILE);
	if( ucTryCnt>=stPosParam.ucResendTimes+2 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "REVERSAL FAIL");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "CONNECT BANK");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);		
	}

	memcpy(&PosCom, &BackPosCom, COM_SIZE);
	memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));
	stTemp.iTransNo = PosCom.stTrans.iTransNo;
	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
	if( ucTryCnt>=stPosParam.ucResendTimes+2 )
	{
		return E_TRANS_FAIL;
	}

	lcdCls();
	return ucRet;
}

uint8_t SendRecvPacket(void)
{
	int16_t	iResult;
	uint8_t	ucRet;
	unsigned long  ulTemp;
	int   ibacklight_set_time_bak, iret;

#ifndef _GET_BASETATION
	kbFlush();
	memset(glSendPack.sPINData,0,sizeof(glSendPack.sPINData));
	memset(glSendPack.szSecurityInfo,0,sizeof(glSendPack.szSecurityInfo));
	iResult = MakeReqPacket();
	if( iResult<0 )
	{
		return E_MAKE_PACKET;
	}
#endif

	start_timer(30, 0);
	sysGetbacklighttime(&ibacklight_set_time_bak);
	iret = sysSetbacklighttime(-1);
	if( ConnectHost()!=OK )
	{
		stop_timer();
		sysSetbacklighttime(ibacklight_set_time_bak);
		PrintFaildTicket(1,(uint8_t *)"\x00\x00",(uint8_t *)"\x00\x00");
		return E_ERR_CONNECT;
	}

#ifdef _GET_BASETATION
	GetBaseStationInfo(glSendPack.szField63+3);
	kbFlush();
	iResult = MakeReqPacket();
	if( iResult<0 )
	{
		stop_timer();
		sysSetbacklighttime(ibacklight_set_time_bak);
		return E_MAKE_PACKET;
	}

#endif
	if( gstCurCommCfg.ucCommType==CT_TCPIP ||
		gstCurCommCfg.ucCommType==CT_GPRS  ||
		gstCurCommCfg.ucCommType==CT_CDMA  ||
		gstCurCommCfg.ucCommType==CT_WIFI )	
	{
		memmove(&sSendPacketBuff[2], sSendPacketBuff, usSendPacketLen);
		PubLong2Char((ulong)usSendPacketLen, 2, (char *)sSendPacketBuff);
		usSendPacketLen += 2;
	}

	showHex((char *)"sSendPacketBuff",sSendPacketBuff,usSendPacketLen);

	ucRet = SendPacket(sSendPacketBuff, usSendPacketLen);
	if( ucRet!=OK )
	{
		stop_timer();
		sysSetbacklighttime(ibacklight_set_time_bak);
		return E_SEND_PACKET;
	}

	//NacTxd(sSendPacketBuff, usSendPacketLen);

	if(stTemp.iTransNo==POS_SETT)
	{
		stTransCtrl.ucPosStatus = SETTLE_STATUS;
		SaveCtrlParam();
	}
	PosCom.bOnlineTxn = TRUE;
	PosCom.bGetHostResp = FALSE; 
	memset((char*)&glRecvPack,0,sizeof(glRecvPack));

	//银行卡平台才需要冲正和结算
	if(PosCom.stTrans.isCtlplat==0)
	{
		memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));
		// NETPAY 
		//消息发送出去就保存冲正数据
		//接收的时候如果响应码为"00",就删除冲正文件
		//反之则从冲正文件中读取交易数据冲正
		//冲正记录文件:dup_file
		if( TranNeedReversal(stTemp.iTransNo) )	
		{
			strcpy((char *)PosCom.szRespCode, "68");
			SaveReversalTranData();
			if( PosCom.ucSwipedFlag==CARD_INSERTED )
			{
				//如果是emv流程
				//冲正的时候需要用到55域数据
				UpdateEmvRevFile();
			}
			if( stTemp.iTransNo == QPBOC_ONLINE_SALE )
			{
				UpdateEmvRevFile();
			}
		}

		if( TranNeedIncTraceNo(stTemp.iTransNo) )
		{
			IncreaseTraceNo();
		}

	}
	
	lcdClrLine(2, 7);
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PROCESANDO");
	lcdDisplay(0, 7, DISP_CFONT, "     RECIBIENDO   ");
	lcdFlip();
	usleep(200);
	ucRet = RecvPacket(sRecvPacketBuff, &usRecvPacketLen, stPosParam.ucCommWaitTime);
	if( ucRet!=OK )
	{
		stop_timer();
		sysSetbacklighttime(ibacklight_set_time_bak);
		return E_RECV_PACKET;
	}
	
	showHex((char *)"Recva:",sRecvPacketBuff,usRecvPacketLen);
	
	stop_timer();
	sysSetbacklighttime(ibacklight_set_time_bak);

	
	if( gstCurCommCfg.ucCommType==CT_TCPIP ||
		gstCurCommCfg.ucCommType==CT_GPRS  ||
		gstCurCommCfg.ucCommType==CT_CDMA  ||
		gstCurCommCfg.ucCommType==CT_WIFI )	
	{
		PubChar2Long(sRecvPacketBuff, 2, &ulTemp);
		if( ulTemp+2!=(ulong)usRecvPacketLen )
		{
			Display2StringInRect(NULL, "RECV DATA ERR");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(2);
			return NO_DISP;
		}
		memmove(sRecvPacketBuff, &sRecvPacketBuff[2], (uint)ulTemp);
		usRecvPacketLen = (uint32_t)ulTemp;
	}

	//NacTxd(sRecvPacketBuff, usRecvPacketLen);
	PosCom.bGetHostResp = TRUE;
	//这个函数中保存交易信息，
	//包括写入到log中的信息:PosCom.stTrans
	iResult = ProcRspPacket();
	if( iResult<0 ) 
	{
		return ((iResult==-2) ? E_CHECK_MAC_VALUE : E_RESOLVE_PACKET);
	}

	return OK;	
}

uint8_t SendPacket(uint8_t *sSendData, uint16_t PacketLen)
{
	int   iRet;

#ifdef _OFFLINE_TEST_VERSION
	return 0;
#endif

	iRet = CommTxd(sSendData,PacketLen,stPosParam.ucCommWaitTime);
	if (iRet != 0)
	{
		DispCommErrMsg(iRet);
		return NO_DISP;
	}

	return iRet;
}

uint8_t RecvPacket(uint8_t *psPacket, uint16_t *usPacketLen, uint8_t psWaitTime)
{
	int    iRet;
	uint32_t  ulRecvLen;

#ifdef _OFFLINE_TEST_VERSION
	return 0;
#endif

	iRet = CommRxd(psPacket, MAX_PACKET_LEN, psWaitTime, &ulRecvLen);
	if (iRet != 0)
	{
		DispCommErrMsg(iRet);
		return NO_DISP;
	}
	*usPacketLen = (uint16_t)ulRecvLen;
	
	return iRet;
}

int ConnectHost(void)
{
	int		iRet, iCnt, iKey, iHostNum=1;

#ifdef _OFFLINE_TEST_VERSION
	return 0;
#endif

#ifdef _POS_TYPE_8210
	if (((PosCom.stTrans.iTransNo == DOWNLOAD)&&(stPosParam.stDownParamCommCfg.ucCommType == CT_GPRS 
		|| stPosParam.stDownParamCommCfg.ucCommType == CT_CDMA || stPosParam.stDownParamCommCfg.ucCommType == CT_WIFI
		|| stPosParam.stDownParamCommCfg.ucCommType == CT_TCPIP))||
		((PosCom.stTrans.iTransNo != DOWNLOAD)&&(stPosParam.stTxnCommCfg.ucCommType == CT_GPRS 
		|| stPosParam.stTxnCommCfg.ucCommType == CT_CDMA || stPosParam.stTxnCommCfg.ucCommType == CT_WIFI
		|| stPosParam.stTxnCommCfg.ucCommType == CT_TCPIP)))
	{
		iRet = AdjustCommParam(1);
		if( iRet!=0 )
		{
			DispCommErrMsg(iRet);
			return NO_DISP;
		}
		iRet = CommDial(ACTDIAL_MODE);
		return iRet;
	}		
#endif
	if ((stPosParam.stTxnCommCfg.ucCommType == CT_MODEM)&&(PosCom.stTrans.iTransNo != DOWNLOAD))
	{
		if (stPosParam.szPhoneNum2[0] != 0)
		{
			iHostNum++;
		}
		
		if (stPosParam.szPhoneNum3[0] != 0)
		{
			iHostNum++;
		}
	}
	else
	{
		iHostNum = 2;
	}
	iRet = NO_DISP;
	if(PosCom.stTrans.iTransNo != DOWNLOAD)
	{
		if (stPosParam.stTxnCommCfg.ucCommType == CT_MODEM)
			PosCom.ucConnectCnt = stPosParam.ucTelIndex;
#ifndef _POS_TYPE_8210
		else if (stPosParam.stTxnCommCfg.ucCommType==CT_WIFI || stPosParam.stTxnCommCfg.ucCommType==CT_TCPIP
			|| stPosParam.stTxnCommCfg.ucCommType==CT_GPRS|| stPosParam.stTxnCommCfg.ucCommType==CT_CDMA)
			PosCom.ucConnectCnt = stPosParam.ucTCPIndex;
#endif	
	}
	for (iCnt=0;iCnt<iHostNum;iCnt++)
	{	
		iRet = AdjustCommParam(1);
		if( iRet!=0 )
		{
			DispCommErrMsg(iRet);
			return NO_DISP;
		}
		iRet = CommDial(ACTDIAL_MODE);
		if( iRet==0 )
		{
			if(PosCom.stTrans.iTransNo == DOWNLOAD)
				return 0;
			if (stPosParam.stTxnCommCfg.ucCommType == CT_MODEM)
				stPosParam.ucTelIndex = PosCom.ucConnectCnt;
#ifndef _POS_TYPE_8210
			else if (stPosParam.stTxnCommCfg.ucCommType==CT_WIFI || stPosParam.stTxnCommCfg.ucCommType==CT_TCPIP
				|| stPosParam.stTxnCommCfg.ucCommType==CT_GPRS|| stPosParam.stTxnCommCfg.ucCommType==CT_CDMA)
				stPosParam.ucTCPIndex = PosCom.ucConnectCnt;
#endif			
			SaveAppParam();
			return 0;
		}
		else if (iRet == ERR_USERCANCEL)
		{
			DispResult(iRet);
			return NO_DISP;
		}
		sysBeef(1, 200);
		lcdClrLine(2,7);
		DispDialErrMsg(iRet);
		if((gstCurCommCfg.ucCommType==CT_MODEM)||(gstCurCommCfg.ucCommType==CT_MODEMPPP))
			DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY,NULL, "DAIL FAIL,RETRY?");
		else
			DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY,NULL, "DAIL FAIL,RETRY?");
		lcdDisplay(104, 7, DISP_ASCII, "%d", (iRet & MASK_ERR_CODE));
		lcdFlip();
		iKey = kbGetKeyMs(30000);
		if( iKey==KEY_CANCEL || iKey==KEY_INVALID )
		{
			return ERR_USERCANCEL;
		}
		PosCom.ucConnectCnt++;	
		PosCom.ucConnectCnt = PosCom.ucConnectCnt % iHostNum;
	}

	return  NO_DISP;
}

void CommHangUp(uint8_t bReleaseAll)
{
	if((stPosParam.stTxnCommCfg.ucCommType==CT_GPRS || stPosParam.stTxnCommCfg.ucCommType==CT_CDMA)
		&&(stPosParam.ucLongConnection==PARAM_CLOSE))
	{
		CommOnHook(TRUE);
	}
	else
	{
		CommOnHook(bReleaseAll);
	}
}

uint8_t SaveReversalTranData(void)
{
	int16_t fd;
	int16_t len;	

	fd = fileOpen(REVERSAL_FILE, O_CREAT|O_RDWR);
	if( fd<0 )
	{
		return (E_MEM_ERR);
	}

	len = fileWrite(fd, (uint8_t *)&PosCom, COM_SIZE);
	if( len!=COM_SIZE )
	{
		fileClose(fd);
		return (E_MEM_ERR);
	}
	fileClose(fd);

	return (OK);
}

uint8_t ReadReversalTranData(void)
{
	int16_t		fd;
	int bOnlineTxnBak;
	
    
	fd = fileOpen(REVERSAL_FILE, O_RDWR);
	if( fd<0 )
	{
		return E_MEM_ERR;
	}
	
	bOnlineTxnBak = PosCom.bOnlineTxn;
	if( fileRead(fd, (uint8_t *)&PosCom, COM_SIZE)!=COM_SIZE )
	{
		fileClose(fd);
		return E_MEM_ERR;
	}
	PosCom.bOnlineTxn = bOnlineTxnBak;

	fileClose(fd);

	return OK;
}

uint8_t UpdateReversalFile(uint8_t Err)
{
	uint8_t	szBuff[10];
	uint8_t szBuff1[10];
	//needn't reversal
	return OK;

	
	memset(szBuff, 0, sizeof(szBuff));
	if( Err==E_RECV_PACKET || Err==E_CHECK_MAC_VALUE || Err==E_RESOLVE_PACKET )
	{
		if( fileExist(REVERSAL_FILE)>=0 ) 
		{
			if( strlen((char *)PosCom.stTrans.szAuthNo)>0 )
			{
				sprintf((char *)szBuff, "%s", PosCom.stTrans.szAuthNo);
			}
			if( strlen((char *)PosCom.stTrans.szDate)>0 )
			{
				sprintf((char *)szBuff1, "%.8s", PosCom.stTrans.szDate);
			}

			ReadReversalTranData();

			if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==POS_PREAUTH )
			{
				if( strlen((char *)szBuff)>0 )
				{
					sprintf((char *)PosCom.stTrans.szAuthNo, "%s", szBuff);
				}
			}
			
			if( (stTemp.iTransNo==POS_PREAUTH_VOID || stTemp.iTransNo==POS_AUTH_CM 
				|| stTemp.iTransNo==POS_AUTH_VOID) )
			{
				if( Err==E_CHECK_MAC_VALUE )
				{
					if( strlen((char *)szBuff1)>0 )
					{
						sprintf((char *)PosCom.stTrans.szDate, "%s", szBuff1);
					}
				}
				else
				{
					strcpy((char *)PosCom.stTrans.szDate, "00000000");
				}
			}

			if( Err==E_RECV_PACKET )
			{
				strcpy((char *)PosCom.szRespCode, "98"); 
			}
			else if( Err==E_CHECK_MAC_VALUE )
			{
				strcpy((char *)PosCom.szRespCode, "A0");  
			}
			else 										
			{
				strcpy((char *)PosCom.szRespCode, "06");  
			}

			SaveReversalTranData();
		}
	}
	
	return Err;	
}

int MakeReqPacket(void)
{
	int16_t	iRet;
	uint8_t	 sMacData[LEN_MAC_DATA];
	uint8_t	 szBuffer[10];
	uint16_t uiLen;

	memset(sSendPacketBuff, 0, sizeof(sSendPacketBuff));
	memset(sRecvPacketBuff, 0, sizeof(sRecvPacketBuff));
	usRecvPacketLen = 0;

	memcpy(sSendPacketBuff, stPosParam.sTpdu, 5);	// TPDU
	//memcpy(sSendPacketBuff, "\x60\x00\x00\x00\x03", 5);	// TPDU
	if( stPosParam.ucTestFlag==PARAM_OPEN )			
	{
//		memcpy(&sSendPacketBuff[5], "\x60\x31\x10\x31\x00\x00", 6);
	}
	else
	{
        memcpy(&sSendPacketBuff[5], "\x60\x31\x00", 3);
        memset(szBuffer, 0, sizeof(szBuffer));
        sprintf((char *)szBuffer, gstPosVersion.szVersion);
        PubAsc2Bcd((char *)&szBuffer[1], 6, (char *)&sSendPacketBuff[8]);
	}

	iRet = iPack8583(glMsg0, glData0, &glSendPack, &sSendPacketBuff[5/*+6*/], &uiLen);
	printf("iPack8583:%d\n",iRet);
	if( iRet<0 )
	{
		return iRet;
	}

	usSendPacketLen = (uint16_t)(uiLen+5);
	// NETPAY不需要校验MAC
	if( HaveMacData(stTemp.iTransNo) )
	{
		if( CalMAC(&sSendPacketBuff[5+6], (int16_t)(uiLen-LEN_MAC_DATA), sMacData)!=OK )
		{
			return -1;
		}
		memcpy(&sSendPacketBuff[usSendPacketLen-8], sMacData, LEN_MAC_DATA);
	}

	return 0;
}

int ProcRspPacket(void)
{
	int16_t	iRet,szlen;
	int	lTemp1, lTemp2;
	char *TempPoint = NULL;
	char szParams[20] ={0};
	
	if( usRecvPacketLen<5/*+6*/+2+8 )
	{
		return -1;
	}

	BcdToAsc0(stTemp.sPacketHead, &sRecvPacketBuff[5], 6);  /*Netpay:报头只需要TPDU*/
	iRet = iUnPack8583(glMsg0, glData0, &sRecvPacketBuff[5/*+6*/], usRecvPacketLen-5/*-6*/, &glRecvPack);

	//printf("iUnPack8583:37field->%s\n",glRecvPack.szRRN);
	if( iRet<0 )
	{
		return -1;
	}

	/*
	if(PosCom.stTrans.isCtlplat==0 && (stTemp.iTransNo ==PURSE_SALE ||
		stTemp.iTransNo ==PURSE_AUTH||stTemp.iTransNo ==POS_SALE))
	*/
	{
		SaveRspPackData();
	}
	
	lTemp1 = atol((char *)glSendPack.szMsgCode);
	lTemp2 = atol((char *)glRecvPack.szMsgCode);
	if( lTemp2!=(lTemp1+10) )	
	{
		if(stTemp.iTransNo == POS_REVERSE)	
		{
			return -1;
		}
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PROCODE ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return -1;
	}

	if( glSendPack.szProcCode[0]!=0 &&
		memcmp(glSendPack.szProcCode, glRecvPack.szProcCode, LEN_PROC_CODE)!=0 )
	{
		if(stTemp.iTransNo == POS_REVERSE)
			return -1;
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PROCODE ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return -1;
	}

//gasplat msg =0100, stan is not same between sendpacket and recvpacket
//流水号在测试平台没有同步，先屏蔽
/*
	if(memcmp(glSendPack.szMsgCode, "0100", LEN_PROC_CODE -2)!=0)
	{
		if( glSendPack.szSTAN[0]!=0 &&
			memcmp(glSendPack.szSTAN, glRecvPack.szSTAN, LEN_POS_STAN)!=0 )
		{
			if(stTemp.iTransNo == POS_REVERSE)
				return -1;
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY,NULL, "TRACE ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(3000);
			return -1;
		}
	}
*/	

	//volume control platform
	if(PosCom.stTrans.isCtlplat ==1)
	{
		//63 field
		if( strcmp((char*)glRecvPack.szRspCode,"00") == 0 )
		{
			ProcComField63(glRecvPack.szField63,glRecvPack.szMsgCode);
		}
		else
		{
			ProcFaildTransField63(glRecvPack.szField63);
			return -1;
		}
		//60 field
		if(strlen(glRecvPack.szField60) !=0)
		{
			strcpy(PosCom.stTrans.oil_filed_60,glRecvPack.szField60);
		}
		//62 field
		if(strlen(glRecvPack.sField62) !=0)
		{
			strcpy(PosCom.stTrans.oil_field_62,glRecvPack.sField62);
		}
	}
	else 	//bank platform
	{
		PrintDebug("%s %s \%s", "glRecvPack.szMsgCode:",glRecvPack.szMsgCode,glRecvPack.szRspCode);
		if( strcmp((char*)glRecvPack.szRspCode,"00") != 0 )
		{
			ProcFaildTransField63(glRecvPack.szField63);
			//return OK;
			return -1;
		}
		if( strcmp((char*)glRecvPack.szMsgCode,"0110") == 0 )
		{			
			//GetRules 
			if(strcmp(glRecvPack.szProcCode,"810000")==0)
			{
				GetRules_Field63(glRecvPack.szField63,glRecvPack.szMsgCode);
			}
			// PreAuth
			else if(strcmp(glRecvPack.szProcCode,"820000")==0)
			{
				TempPoint = strstr((char*)glRecvPack.szField63, "YS");
				if ( TempPoint == NULL ) {
					lcdClrLine(2, 7);	
					DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Balance de Eeeor");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000);
					return -1;
				}
				else 
				{
					strncpy(szParams,TempPoint+2,5);
					szlen= atoi(szParams);
					strncpy((char*)PosCom.stTrans.Balance_Amount,TempPoint+7,szlen);
					PosCom.stTrans.Balance_Amount[szlen] = '\0';
				}		
			}
			//GetBalance
			else if(strcmp(glRecvPack.szProcCode,"830000")==0)
			{	
				GetBalance_Field63(glRecvPack.szField63,glRecvPack.szMsgCode);
			}
		}
		else if( strcmp((char*)glRecvPack.szMsgCode,"0210") == 0 )//auth 
		{
			TempPoint = strstr((char*)glRecvPack.szField63, "Z5");
			if ( TempPoint == NULL ) 
			{
				lcdClrLine(2, 7);	
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Codigos de Error");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				return -1;
			}
			else 
			{
				strncpy(szParams,TempPoint+2,5);
				szlen= atoi(szParams);
				strncpy((char*)PosCom.stTrans.stpromptmsg,TempPoint+7,szlen);
				PosCom.stTrans.stpromptmsg[szlen] = '\0';
				printf("stTemp.stpromptmsg:%s\n",PosCom.stTrans.stpromptmsg);
			}
			GetAuth_Field63(glRecvPack.szField63, PosCom.stTrans.TransFlag);
		}
		else
		{
			return OK;
		}

	}
	return OK;
}
/************************************
*****解析auth返回的63域数据
*************************************/
void GetAuth_Field63(uint8_t *field63,uint8_t tflag)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	switch(tflag)
	{
		case PURSE_EDENRED:
			break;
		case PURSE_SODEXO:
			Temp1 = strstr((char*)field63, "SR");
			if ( Temp1 == NULL ) {
			//		return -1;
			}
			else 
			{
				strncpy(szParams,Temp1+2,5);
				szLen = atoi(szParams);
				strncpy((char*)PosCom.stTrans.Prt_SR,Temp1+7,szLen);
				PosCom.stTrans.Prt_SR[szLen] = '\0';
				printf("stTemp.Prt_SR:%s\n",PosCom.stTrans.Prt_SR);
			}

			Temp1 = strstr((char*)field63, "SB");
			if ( Temp1 == NULL ) {
			//		return -1;
			}
			else 
			{
				strncpy(szParams,Temp1+2,5);
				szLen = atoi(szParams);
				strncpy((char*)PosCom.stTrans.Prt_SB,Temp1+7,szLen);
				PosCom.stTrans.Prt_SB[szLen] = '\0';
				printf("stTemp.Prt_SB:%s\n",PosCom.stTrans.Prt_SB);
			}

			Temp1 = strstr((char*)field63, "ST");
			if ( Temp1 == NULL ) {
			//		return -1;
			}
			else 
			{
				strncpy(szParams,Temp1+2,5);
				szLen = atoi(szParams);
				strncpy((char*)PosCom.stTrans.Prt_ST,Temp1+7,szLen);
				PosCom.stTrans.Prt_ST[szLen] = '\0';
				printf("stTemp.Prt_ST:%s\n",PosCom.stTrans.Prt_ST);
			}

			Temp1 = strstr((char*)field63, "SA");
			if ( Temp1 == NULL ) {
			//		return -1;
			}
			else 
			{
				strncpy(szParams,Temp1+2,5);
				szLen = atoi(szParams);
				strncpy((char*)PosCom.stTrans.SA,Temp1+7,szLen);
				PosCom.stTrans.SA[szLen] = '\0';
				printf("stTemp.SA:%s\n",PosCom.stTrans.SA);
			}
			break;
		case PURSE_TODITO:
			break;
		case PURSE_PUNTO:
			break;
		default:
			break;
	}
	
}
/************************************
*****解析getrules返回的63域数据
*************************************/
void GetRules_Field63(uint8_t *field63,uint8_t *szMsgCode)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)field63, "Y1");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.oil_id,Temp1+7,szLen);
		PosCom.stTrans.oil_id[szLen] = '\0';
		printf("stTemp.oil_liters:%s\n",PosCom.stTrans.oil_liters);
	}

	Temp1 = strstr((char*)field63, "Y2");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.saletype,Temp1+7,szLen);
		PosCom.stTrans.saletype[szLen] = '\0';
	}

	Temp1 = strstr((char*)field63, "Y3");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.isnip,Temp1+7,szLen);
		PosCom.stTrans.isnip[szLen] = '\0';
	}

	Temp1 = strstr((char*)field63, "Y4");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.origi_kilometer,Temp1+7,szLen);
		PosCom.stTrans.origi_kilometer[szLen] = '\0';
	}

	Temp1 = strstr((char*)field63, "Y5");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.origi_plate,Temp1+7,szLen);
		PosCom.stTrans.origi_plate[szLen] = '\0';
	}

	Temp1 = strstr((char*)field63, "YA");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.kilometer,Temp1+7,szLen);
		PosCom.stTrans.kilometer[szLen] = '\0';
	}

	Temp1 = strstr((char*)field63, "YB");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.plate,Temp1+7,szLen);
		PosCom.stTrans.plate[szLen] = '\0';
	}

}
/************************************
*****解析getbalance返回的63域数据
*************************************/
void GetBalance_Field63(uint8_t *field63,uint8_t *szMsgCode)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)field63, "E1");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.saletype,Temp1+7,szLen);
		PosCom.stTrans.saletype[szLen] = '\0';
		PrintDebug("%s%s","stTemp.productid:",PosCom.stTrans.saletype);

	}

	Temp1 = strstr((char*)field63, "E2");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.Horometer,Temp1+7,szLen);
		PosCom.stTrans.Horometer[szLen] = '\0';
		PrintDebug("%s%s","stTemp.Horometer:",PosCom.stTrans.Horometer);
	}

	Temp1 = strstr((char*)field63, "E3");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.kilometer,Temp1+7,szLen);
		PosCom.stTrans.kilometer[szLen] = '\0';
		PrintDebug("%s%s","stTemp.kilometer:",PosCom.stTrans.kilometer);


	}

	Temp1 = strstr((char*)field63, "E4");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.Liters,Temp1+7,szLen);
		PosCom.stTrans.Liters[szLen] = '\0';
		PrintDebug("%s%s","stTemp.Liters:",PosCom.stTrans.Liters);
	}

	Temp1 = strstr((char*)field63, "E5");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.plate,Temp1+7,szLen);
		PosCom.stTrans.plate[szLen] = '\0';
		PrintDebug("%s%s","stTemp.plate:",PosCom.stTrans.plate);

	}

	Temp1 = strstr((char*)field63, "ED");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.DriverRule,Temp1+7,szLen);
		PosCom.stTrans.DriverRule[szLen] = '\0';
		PrintDebug("%s%s","stTemp.DriverRule:",PosCom.stTrans.DriverRule);

	}

	Temp1 = strstr((char*)field63, "EB");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.Balance_Amount,Temp1+7,szLen);
		PosCom.stTrans.Balance_Amount[szLen] = '\0';
		PrintDebug("%s%s","stTemp.Balance_Amount:",PosCom.stTrans.Balance_Amount);
	}

	Temp1 = strstr((char*)field63, "ET");
	if ( Temp1 == NULL ) {
	//		return -1;
	}
	else 
	{
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy((char*)PosCom.stTrans.Trans_Id,Temp1+7,szLen);
		PosCom.stTrans.Trans_Id[szLen] = '\0';
		PrintDebug("%s%s","stTemp.Trans_Id:",PosCom.stTrans.Trans_Id);
	}

}
/************************
*****根据类型去解析
*************************/
void ProcComField63(uint8_t *field63,uint8_t *szMsgCode)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	//容量申请返回报文
	if(strcmp(szMsgCode,"0110") ==0)
	{
		Temp1 = strstr((char*)field63, "C2");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.oil_liters,Temp1+7,szLen);

			PosCom.stTrans.oil_liters[szLen] = '\0';
			PrintDebug("%s%s","stTemp.oil_liters:",PosCom.stTrans.oil_liters);

		}

		Temp1 = strstr((char*)field63, "C3");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.oil_price,Temp1+7,szLen);
			PosCom.stTrans.oil_price[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "C4");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.oil_amount,Temp1+7,szLen);
			PosCom.stTrans.oil_amount[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "C5");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.oil_id,Temp1+7,szLen);
			PosCom.stTrans.oil_id[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "C6");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.nato,Temp1+7,szLen);
			PosCom.stTrans.nato[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "C7");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.folio,Temp1+7,szLen);
			PosCom.stTrans.folio[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "C8");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.max_tip,Temp1+7,szLen);
			PosCom.stTrans.max_tip[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "C9");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.pemex,Temp1+7,szLen);
			PosCom.stTrans.pemex[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "CP");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.cp,Temp1+7,szLen);
			PosCom.stTrans.cp[szLen] = '\0';
		}

		Temp1 = strstr((char*)field63, "CF");
		if ( Temp1 == NULL ) {
		//		return -1;
		}
		else 
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy((char*)PosCom.stTrans.cf,Temp1+7,szLen);
			PosCom.stTrans.cf[szLen] = '\0';
		}
	}
}

void ProcFaildTransField63(uint8_t *field63)
{
	int  szLen = 0;
	char *Temp1 = NULL;
	char szParams[6];

	Temp1 = strstr((char*)field63, "RM");
	if ( Temp1 != NULL ) {
		strncpy(szParams,Temp1+2,5);
		szLen = atoi(szParams);
		strncpy(PosComconTrol.szRM,Temp1+8,szLen);
		PosComconTrol.szRM[szLen] = '\0';
	}
	else 
	{
		Temp1 = strstr((char*)field63, "Z5");
		if(Temp1 !=NULL)
		{
			strncpy(szParams,Temp1+2,5);
			szLen = atoi(szParams);
			strncpy(PosComconTrol.szRM,Temp1+8,szLen);
			PosComconTrol.szRM[szLen] = '\0';

			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, PosComconTrol.szRM);
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(3000);
		}

	}
}

uint8_t TranNeedReversal(int iTranType)
{
	if(PosCom.stTrans.TransFlag == NORMAL_SALE    || 
		PosCom.stTrans.TransFlag == INTERESES_SALE ||
		PosCom.stTrans.TransFlag == PRE_TIP_SALE ||
		iTranType==POS_SALE  ||iTranType==PURSE_SALE)
	{
		if(PosCom.stTrans.isCtlplat==0)
			return TRUE;
	}

	return FALSE;
}

uint8_t TranNeedIncTraceNo(int iTranType)
{
	if (iTranType == POS_BATCH_UP)
	{
		if (gUplTrace == 0)
			return TRUE;
		return FALSE;
	}

	if( iTranType==POS_SALE        || iTranType==POS_SALE_VOID    ||
		iTranType==POS_PREAUTH     || iTranType==POS_PREAUTH_VOID ||
		iTranType==POS_AUTH_CM     || iTranType==POS_AUTH_VOID    ||
		iTranType==POS_PREAUTH_ADD || 
		iTranType==POS_REFUND      || iTranType==POS_QUE          ||
		iTranType==POS_LOGON       || iTranType==POS_LOGOFF       ||
		iTranType==POS_SETT        || iTranType==POS_BATCH_UP     ||
		iTranType==POS_OFF_CONFIRM || iTranType==UPLOAD_SCRIPT_RESULT ||
		iTranType==ICC_OFFSALE 	   || iTranType == QPBOC_ONLINE_SALE ||
		(iTranType>=EC_TOPUP_CASH && iTranType<=END_DOWNLOADBLK) ||
		iTranType==CHECK_IN || iTranType==CHECK_OUT || 
		iTranType==CASH_ADVACNE || iTranType==NETPAY_REFUND ||
		iTranType==NETPAY_ADJUST || iTranType==PURSE_SALE)
	{
		return TRUE;
	}
	
	return FALSE;
}

uint8_t HaveMacData(int iTranType)
{
	// NETPAY 不需要MAC
	return FALSE;
	if( iTranType==POS_SALE        || iTranType==POS_SALE_VOID    ||
		/*iTranType==POS_PREAUTH     ||*/ iTranType==POS_PREAUTH_VOID ||
		iTranType==POS_AUTH_CM     || iTranType==POS_AUTH_VOID    ||
		iTranType==POS_PREAUTH_ADD || 
		iTranType==POS_REFUND      || iTranType==POS_QUE          ||
		iTranType==POS_REVERSE     || iTranType==OFF_SALE         ||
		iTranType==OFF_ADJUST      || iTranType==ADJUST_TIP       ||
		iTranType==POS_OFF_CONFIRM || iTranType==POS_REVERSE      ||
		iTranType==ICC_OFFSALE     || iTranType==UPLOAD_SCRIPT_RESULT ||
		(iTranType>=EC_QUICK_SALE && iTranType<=POS_VOID_INSTAL)    ||
		iTranType==POS_ELEC_SIGN)
	{
		return TRUE;
	}

	return FALSE;
}

uint8_t TrickleFeedOffLineTxn(uint8_t ucAllowBreak)
{
	uint8_t	ucRet;

	if (GetOfflineTxnNums(TS_TXN_OFFLINE)<=0)
	{
		return 0;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  SEND OFFLINE  ");
	lcdFlip();

	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));
	memcpy(&BackPosCom, &PosCom, COM_SIZE);
	ucRet = SendOffLineTrans(ucAllowBreak, stPosParam.ucOfflineSendTimes);
	memcpy(&PosCom, &BackPosCom, COM_SIZE);
	memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));
	stTemp.iTransNo = PosCom.stTrans.iTransNo;

	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	return ucRet;
}

uint8_t TrickleFeedICOffLineTxn(uint8_t ucAllowBreak)
{
	uint8_t	ucRet;

	if (GetOfflineTxnNums(TS_ICC_OFFLINE)<=0)
	{
		return 0;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  SEND OFFLINE  ");
	lcdFlip();

	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));	
	memcpy(&BackPosCom, &PosCom, COM_SIZE);
	ucRet = SendICCOffLineTrans(ucAllowBreak, 0);
	memcpy(&PosCom, &BackPosCom, COM_SIZE);
	memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));	
	stTemp.iTransNo = PosCom.stTrans.iTransNo;
	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;
	sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	return ucRet;
}

void MakeReversalPack(void)
{
	memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));

	sprintf((char *)glSendPack.szMsgCode, "0400");
#if 0
	sprintf((char *)glSendPack.szAuthCode, "%s", PosCom.stTrans.szAuthNo);
	sprintf((char *)glSendPack.szRRN,  "%s", PosCom.stTrans.szSysReferNo);
	sprintf((char *)glSendPack.szRspCode,  "%s", PosCom.szRespCode);
//	glSendPack.szTrack2[0] = 0;
	glSendPack.szTrack3[0] = 0;
	memcpy(glSendPack.sPINData, "\x00\x00", 2);	
	glSendPack.szCaptureCode[0]  = 0;
	glSendPack.szSecurityInfo[0] = 0;
//	glSendPack.szRRN[0] = 0; 
	glSendPack.szExtAmount[0] = 0;

	if( PosCom.stTrans.iTransNo==POS_SALE || PosCom.stTrans.iTransNo==POS_QUE || PosCom.stTrans.iTransNo==POS_PREAUTH ||
		PosCom.stTrans.iTransNo== EC_TOPUP_CASH || PosCom.stTrans.iTransNo== EC_TOPUP_NORAML || PosCom.stTrans.iTransNo== EC_TOPUP_SPEC ||
		PosCom.stTrans.iTransNo== EC_VOID_TOPUP || PosCom.stTrans.iTransNo== QPBOC_ONLINE_SALE || PosCom.stTrans.iTransNo== PRE_TIP_SALE ||
		PosCom.stTrans.iTransNo== INTERESES_SALE || PosCom.stTrans.iTransNo==PAYMENT_WITH_CARD)
	{
		ShortToByte(glSendPack.sICCData, (uint16_t)PosCom.iReversalDE55Len);
		memcpy(&glSendPack.sICCData[2], PosCom.sReversalDE55, PosCom.iReversalDE55Len);
	}

	if ( PosCom.stTrans.iTransNo==POS_PREAUTH )
	{
		glSendPack.szRspCode[0] = 0;
//		sprintf(glSendPack.szTranAmt,"%s","000000000125");
	}

	// NETPAY
	if( PosCom.stTrans.iTransNo==POS_SALE || 
		PosCom.stTrans.iTransNo==PRE_TIP_SALE ||
		PosCom.stTrans.iTransNo==INTERESES_SALE)
	{
		sprintf((char *)glSendPack.szProcCode, "%s", "000000");
		
	}
	else if ( PosCom.stTrans.iTransNo==POS_PREAUTH)
	{
		sprintf((char *)glSendPack.szProcCode, "%s", "776610");
	}
	else if ( PosCom.stTrans.iTransNo==POS_AUTH_CM)
	{
		sprintf((char *)glSendPack.szProcCode, "%s", "776611");
	}
	else if (PosCom.stTrans.iTransNo==PAYMENT_WITH_CARD)
	{
		sprintf((char *)glSendPack.szProcCode, "%s", "776606");
	}
	else if ( PosCom.stTrans.iTransNo==PRT_CHECK_IN)
	{
		sprintf((char *)glSendPack.szProcCode, "%s", "777710");
	}
	else if (PosCom.stTrans.iTransNo==PRT_CHECK_OUT)
	{
		sprintf((char *)glSendPack.szProcCode, "%s", "777711");
	}
#endif
	if( HaveMacData(stTemp.iTransNo) )
	{
		ShortToByte(glSendPack.sMacData, LEN_MAC_DATA);
		memset(glSendPack.szLocalTime,0,sizeof(glSendPack.szLocalTime));
		memset(glSendPack.szLocalDate,0,sizeof(glSendPack.szLocalDate));
	}
}

void SaveRspPackData(void)
{
	int16_t		iLen;
	char        szBuffer[20],szTemp[20];

	//if (PosCom.stTrans.iTransNo != EC_TOPUP_NORAML)
	if(PosCom.stTrans.TransFlag !=PURSE_PUNTO && PosCom.stTrans.TransFlag !=PURSE_TODITO)
	{
		//sprintf((char *)PosCom.stTrans.szCardNo,       "%.19s", glRecvPack.szPan);
	}
	sprintf((char *)PosCom.szRespCode,             "%.2s",  glRecvPack.szRspCode);
	sprintf((char *)PosCom.stTrans.szTime,         "%.6s",  glRecvPack.szLocalTime);
	sprintf((char *)&PosCom.stTrans.szDate[4],     "%.4s",  glRecvPack.szLocalDate);
	sprintf((char *)PosCom.stTrans.szExpDate,      "%.4s",  glRecvPack.szExpDate);
	sprintf((char *)PosCom.stTrans.szSysReferNo,   "%.12s", glRecvPack.szRRN);
	if( glRecvPack.szAuthCode[0]!=0 )
	{	
		sprintf((char *)PosCom.stTrans.szAuthNo,   "%.6s",  glRecvPack.szAuthCode);
	}
	sprintf((char *)PosCom.stTrans.szIssuerBankId, "%.11s",  glRecvPack.szAddRspData);
	sprintf((char *)PosCom.stTrans.szRecvBankId,   "%.11s",  &glRecvPack.szAddRspData[11]);
	sprintf((char *)PosCom.stTrans.szCenterId,     "%.8s",  glRecvPack.szAcquirerID);
	sprintf((char *)PosCom.stTrans.szSettDate,     "%.4s",  glRecvPack.szSettleDate);
	sprintf((char *)PosCom.stTrans.szCurrencyCode, "%.3s",  glRecvPack.szCurrencyCode);	
	if( stTemp.iTransNo==POS_SETT )
	{
		stTemp.ucRmbSettRsp = glRecvPack.szField48[30];
		stTemp.ucFrnSettRsp = glRecvPack.szField48[61];
	}
	
	if( stTemp.iTransNo!=POS_SETT && stTemp.iTransNo!=POS_LOGON )
	{
		memset(PosCom.stTrans.szIssuerResp,0,sizeof(PosCom.stTrans.szIssuerResp));
		memset(PosCom.stTrans.szCenterResp,0,sizeof(PosCom.stTrans.szCenterResp));
		memset(PosCom.stTrans.szRecvBankResp,0,sizeof(PosCom.stTrans.szRecvBankResp));
		sprintf((char *)PosCom.stTrans.szCardUnit, "%.3s", glRecvPack.szField63);
		iLen = strlen((char *)glRecvPack.szField63);
		if( iLen>3 && iLen<=23 )
		{
			memcpy(PosCom.stTrans.szIssuerResp, &glRecvPack.szField63[3], iLen-3);
		}
		else if ( iLen>23 && iLen<=43 )
		{
			memcpy(PosCom.stTrans.szIssuerResp, &glRecvPack.szField63[3], 20);
			memcpy(PosCom.stTrans.szCenterResp, &glRecvPack.szField63[23], iLen-23);
		}
		else if ( iLen>43 && iLen<=63 )
		{
			memcpy(PosCom.stTrans.szIssuerResp, &glRecvPack.szField63[3], 20);
			memcpy(PosCom.stTrans.szCenterResp, &glRecvPack.szField63[23], 20);
			memcpy(PosCom.stTrans.szRecvBankResp, &glRecvPack.szField63[43], iLen-43);
		}
	}	
	
	if(strlen((char*)glRecvPack.szExtAmount)>=20)
		AscToBcd(PosCom.stTrans.sExtAmount, &glRecvPack.szExtAmount[8], 12);
	else
		memcpy(PosCom.stTrans.sExtAmount,"\x00\x00\x00\x00\x00\x00",6);

}

void IncreaseTraceNo(void)
{
	uint8_t	szBuff[LEN_POS_STAN+1];

	stPosParam.lNowTraceNo++;
	if( stPosParam.lNowTraceNo<=0 || stPosParam.lNowTraceNo>999999L )
	{
		stPosParam.lNowTraceNo = 1L;
	}
	sprintf((char*)szBuff, "%0*d", LEN_POS_STAN, stPosParam.lNowTraceNo);
	SaveAppParam();
}


int AdjustCommParam(uint8_t ucCheckWire)
{
	char  szTelNo[100];
	char  szRemoteIP[15+1], szRemotePort[5+1];
	if (PosCom.stTrans.iTransNo == DOWNLOAD)
	{
		memcpy(&gstCurCommCfg,&stPosParam.stDownParamCommCfg,sizeof(ST_COMM_CONFIG));
		sprintf((char *)gstCurCommCfg.stPSTNPara.szTxnTelNo,"%s%s",stPosParam.szExtNum,(char *)stPosParam.szDownloadTel);
	}
	else
	{
		memcpy(&gstCurCommCfg,&stPosParam.stTxnCommCfg,sizeof(ST_COMM_CONFIG));	
		if (gstCurCommCfg.ucCommType == CT_MODEM)
		{
			memset(szTelNo,0,sizeof(szTelNo));
			switch (PosCom.ucConnectCnt)
			{
			case 1:
				sprintf(szTelNo, "%s%s", stPosParam.szExtNum,stPosParam.szPhoneNum1);
				if (stPosParam.szPhoneNum2[0]!=0)
				{
					sprintf((char *)szTelNo, "%s%s", stPosParam.szExtNum,stPosParam.szPhoneNum2);
				}
				break;
			case 2:
				sprintf(szTelNo, "%s%s", stPosParam.szExtNum,stPosParam.szPhoneNum1);
				if (stPosParam.szPhoneNum3[0]!=0)
				{
					sprintf((char *)szTelNo, "%s%s", stPosParam.szExtNum,stPosParam.szPhoneNum3);
				}
				break;
			case 0:
			default:
				sprintf(szTelNo, "%s%s", stPosParam.szExtNum,stPosParam.szPhoneNum1);
				break;
			}
			strcpy((char *)gstCurCommCfg.stPSTNPara.szTxnTelNo,szTelNo);
		}
		else
		{
			memset(szTelNo,0,sizeof(szTelNo));
			//PosCom.ucConnectCnt =1;//强制命令为1 ;add by yxj
			switch (PosCom.ucConnectCnt)
			{
			case 1:
				memcpy(szRemoteIP,stPosParam.szPOSRemoteIP,sizeof(stPosParam.szPOSRemoteIP));
				memcpy(szRemotePort,stPosParam.szPOSRemotePort,sizeof(stPosParam.szPOSRemotePort));
				if (stPosParam.szPOSRemoteIP2[0]!=0)
				{
					memcpy(szRemoteIP,stPosParam.szPOSRemoteIP2,sizeof(stPosParam.szPOSRemoteIP2));
					memcpy(szRemotePort,stPosParam.szPOSRemotePort2,sizeof(stPosParam.szPOSRemotePort2));
				}
			/*
				if(PosCom.stTrans.isCtlplat ==1)
				{
					memcpy(szRemoteIP,stPosParam.szPOSRemoteIP,sizeof(stPosParam.szPOSRemoteIP));
					memcpy(szRemotePort,stPosParam.szPOSRemotePort,sizeof(stPosParam.szPOSRemotePort));
				}
				else
				{
					memcpy(szRemoteIP,stPosParam.szPOSRemoteIP2,sizeof(stPosParam.szPOSRemoteIP2));
					memcpy(szRemotePort,stPosParam.szPOSRemotePort2,sizeof(stPosParam.szPOSRemotePort2));
				}
			*/
				break;
			case 0:
			default:
				memcpy(szRemoteIP,stPosParam.szPOSRemoteIP,sizeof(stPosParam.szPOSRemoteIP));
				memcpy(szRemotePort,stPosParam.szPOSRemotePort,sizeof(stPosParam.szPOSRemotePort));
				break;
			}
			sprintf((char *)gstCurCommCfg.stTcpIpPara.szRemoteIP,"%s",szRemoteIP);
			sprintf((char *)gstCurCommCfg.stTcpIpPara.szRemotePort,"%s",szRemotePort);
			sprintf((char *)gstCurCommCfg.stWIFIIpPara.szRemoteIP,"%s",szRemoteIP);
			sprintf((char *)gstCurCommCfg.stWIFIIpPara.szRemotePort,"%s",szRemotePort);
			sprintf((char *)gstCurCommCfg.stWirlessPara.szRemoteIP,"%s",szRemoteIP);
			sprintf((char *)gstCurCommCfg.stWirlessPara.szRemotePort,"%s",szRemotePort);

			//printf("szRemoteIP:%s ;szRemotePort:%s\n",szRemoteIP,szRemotePort);

		}
	}
		
	if (gstCurCommCfg.ulSendTimeOut <= 30)
	{
		gstCurCommCfg.ulSendTimeOut = 30;
	}
	if (gstCurCommCfg.ulSendTimeOut >= 250)
	{
		gstCurCommCfg.ulSendTimeOut = 250;
	}
	if (gstCurCommCfg.ucCommType ==CT_MODEM )
	{
		gstCurCommCfg.stPSTNPara.stDialPara.dial_timeo = gstCurCommCfg.ulSendTimeOut;
	}
	gstCurCommCfg.ulSendTimeOut = gstCurCommCfg.ulSendTimeOut*1000;
		
	return CommSetCfgParam(&gstCurCommCfg,ucCheckWire);
}

int CheckCapkAid(void)
{
	uint8_t ucRet;
	int ret = 0;
	int iTransNoTmp;
	uint8_t sAmount[6];
	
	iTransNoTmp = stTemp.iTransNo;
	memcpy(sAmount, PosCom.stTrans.sAmount, 6);
	
	if( stPosParam.ucEmvSupport!=PARAM_OPEN)
		return 0;
	
	if(stTransCtrl.bEmvCapkLoaded==TRUE && stTransCtrl.bEmvAppLoaded==TRUE)
		return 0;
	
	if(stTransCtrl.bEmvCapkLoaded == FALSE)
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DownLoad CAPK ");
		DispMulLanguageString(0,4,DISP_CFONT|DISP_MEDIACY,NULL,"CAPK NOT EXIST  ");
		DispMulLanguageString(0,6,DISP_CFONT|DISP_MEDIACY,NULL,"DownLoad now?   ");
		lcdFlip();
		PubBeepErr();
		if(PubYesNo(60) != TRUE)
		{
			ret = -1;
			goto exit;
		}
		ucRet = DownLoadCAPK(TRUE);
		if( ucRet!=OK )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "DOWNLOAD CAPK FAIL");
			lcdFlip();
			sysDelayMs(3000);
			ret = -2;
			goto exit;
		}
	}

	if(stTransCtrl.bEmvAppLoaded==FALSE)
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DownLoad AID  ");
		DispMulLanguageString(0,4,DISP_CFONT|DISP_MEDIACY,NULL,"AID NOT EXIST   ");
		DispMulLanguageString(0,6,DISP_CFONT|DISP_MEDIACY,NULL,"DownLoad now?   ");
		lcdFlip();
		PubBeepErr();
		if(PubYesNo(60) != TRUE)
		{
			ret = -3;
			goto exit;
		}
		ucRet = DownLoadEmvParam(TRUE);
		if( ucRet!=OK )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "DOWNLOAD AID FAIL");
			lcdFlip();
			sysDelayMs(3000);
			ret = -4;
			goto exit;
		}
	}
exit:
	stTemp.iTransNo = iTransNoTmp;
	memcpy(PosCom.stTrans.sAmount, sAmount, 6);
	DispTransName();
	return ret;
}
