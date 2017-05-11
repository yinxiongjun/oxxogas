#include "posapi.h"
#include "posparams.h"
#include "glbvar.h"
#include "tranfunclist.h"

PARAMS myParams[]=
{
	{"merchant no",		STR_PARAMS(stPosParam.szUnitNum),	  STRING_ITEM},	
	//{"terminal no",		STR_PARAMS(stPosParam.szPosId),	      STRING_ITEM},
	{"terminal no",		STR_PARAMS(stPosParam.szMechId),	      STRING_ITEM},
	{"merchant name",    STR_PARAMS(stPosParam.szEngName), STRING_ITEM},
	{"pabx",	STR_PARAMS(stPosParam.szExtNum),	  STRING_ITEM},
	{"telphone1",	STR_PARAMS(stPosParam.szPhoneNum1),	  STRING_ITEM},
	{"telphone2",	STR_PARAMS(stPosParam.szPhoneNum2),	  STRING_ITEM},	
	{"telphone3",	STR_PARAMS(stPosParam.szPhoneNum3),	  STRING_ITEM},		
	{"No_Promotions", CHAR_PARAMS(stPosParam.No_Promotions), ASSCII_ITEM},
	{"Costom_Promotions", CHAR_PARAMS(stPosParam.Costom_Promotions), ASSCII_ITEM},
	{"No_Promotions", CHAR_PARAMS(stPosParam.No_Promotions), ASSCII_ITEM},
	{"MonthFlag3", CHAR_PARAMS(stPosParam.MonthFlag3), ASSCII_ITEM},
	{"MonthFlag6", CHAR_PARAMS(stPosParam.MonthFlag6), ASSCII_ITEM},
	{"MonthFlag9", CHAR_PARAMS(stPosParam.MonthFlag9), ASSCII_ITEM},
	{"MonthFlag12", CHAR_PARAMS(stPosParam.MonthFlag12), ASSCII_ITEM},
	{"MonthFlag18", CHAR_PARAMS(stPosParam.MonthFlag18), ASSCII_ITEM},
	{"CHECK IN", CHAR_PARAMS(stPosParam.CheckInFlag), ASSCII_ITEM},
	{"CHECK OUT", CHAR_PARAMS(stPosParam.CheckOutFlag), ASSCII_ITEM},
	{"DEVOLUCION", CHAR_PARAMS(stPosParam.Netpay_RefundFlag), ASSCII_ITEM},
	{"AJUSTES", CHAR_PARAMS(stPosParam.Netpay_AdjustFlag), ASSCII_ITEM},
	{"Retail", CHAR_PARAMS(stPosParam.Switch_Retail), ASSCII_ITEM},
	{"Hotel", CHAR_PARAMS(stPosParam.Switch_Hotel), ASSCII_ITEM},
	{"Restaurant", CHAR_PARAMS(stPosParam.Switch_Restaurant), ASSCII_ITEM},
	{"USD", CHAR_PARAMS(stPosParam.Switch_USD), ASSCII_ITEM},
	{"CANCELACION", CHAR_PARAMS(stPosParam.Netpay_CancelFlag), ASSCII_ITEM},
	{"MULTIMONEY", CHAR_PARAMS(stPosParam.szSupportMultMoneyFlag), ASSCII_ITEM},
	{"MONEY TYPE", CHAR_PARAMS(stPosParam.szMoneyTypeFlag), ASSCII_ITEM},
	{"FORZADA", CHAR_PARAMS(stPosParam.Netpay_ForzadaFlag), ASSCII_ITEM},

	{"trace no",	LONG_PARAMS(stPosParam.lNowTraceNo),  BIN_ITEM},
	{"batch no",	LONG_PARAMS(stPosParam.lNowBatchNum), BIN_ITEM},
	{"param pwd", 	STR_PARAMS(stPosParam.szParamsPwd),	STRING_ITEM},
	{"Manage pwd", 	STR_PARAMS(stPosParam.szManagePwd),	STRING_ITEM},
	{"safe pwd", 	STR_PARAMS(stPosParam.szSafepwd),	STRING_ITEM},
	{"resend times",    INT_PARAMS(stPosParam.ucResendTimes), BIN_ITEM},
	{"offline send times",    INT_PARAMS(stPosParam.ucOfflineSendTimes), BIN_ITEM},
	{"offline send mode", CHAR_PARAMS(stPosParam.ucSendOffFlag), ASSCII_ITEM},
	{"tip percent", 	INT_PARAMS(stPosParam.ucTipper), BIN_ITEM},
	{"tip open", 	CHAR_PARAMS(stPosParam.ucTipOpen), ASSCII_ITEM},
	{"index", 	INT_PARAMS(stPosParam.ucKeyIndex), BIN_ITEM},
	{"TPDU",		STR_PARAMS(stPosParam.szTpdu),	STRING_ITEM},
	{"control param",STR_PARAMS(stPosParam.szTransCtl),	STRING_ITEM},
	{"comm wait time", 	INT_PARAMS(stPosParam.ucCommWaitTime), BIN_ITEM},
	{"service",   	STR_PARAMS(stPosParam.szMPhone),	STRING_ITEM},
	{"auto logoff", 	CHAR_PARAMS(stPosParam.ucAutoLogoff), ASSCII_ITEM},
	{"pretip_enable", 	CHAR_PARAMS(stPosParam.szpreTip), ASSCII_ITEM},
	{"preauth_enable", 	CHAR_PARAMS(stPosParam.szpreAuth), ASSCII_ITEM},
	{"EMV FALLBACK", 	CHAR_PARAMS(stPosParam.EmvFallBack), ASSCII_ITEM},
	{"manual", CHAR_PARAMS(stPosParam.ucManualInput), ASSCII_ITEM},
	{"print detail", CHAR_PARAMS(stPosParam.ucDetailPrt), ASSCII_ITEM},
	{"max trans", 	INT_PARAMS(stPosParam.iMaxTransTotal), BIN_ITEM},
	{"print num", INT_PARAMS(stPosParam.ucTicketNum), BIN_ITEM},
	{"void swipe", CHAR_PARAMS(stPosParam.ucVoidSwipe), ASSCII_ITEM},
	{"pre-comp swipe", CHAR_PARAMS(stPosParam.ucAuthVoidSwipe), ASSCII_ITEM},
	{"max refund amount",	LONG_PARAMS(stPosParam.lMaxRefundAmt), BIN_ITEM},
	{"max e-cash amount",	LONG_PARAMS(stPosParam.iEcMaxAmount), BIN_ITEM},
	{"max dial times", INT_PARAMS(stPosParam.ucDialRetryTimes), BIN_ITEM},
	{"void pin", CHAR_PARAMS(stPosParam.ucVoidPin), ASSCII_ITEM},
	{"void preauth pin", CHAR_PARAMS(stPosParam.ucPreVoidPin), ASSCII_ITEM},
	{"void pre-comp pin", CHAR_PARAMS(stPosParam.ucPreComVoidPin), ASSCII_ITEM},
	{"pre-comp pin", CHAR_PARAMS(stPosParam.ucPreComPin), ASSCII_ITEM},
	{"operate timeout", INT_PARAMS(stPosParam.ucOprtLimitTime), BIN_ITEM},
	{"pre-comp mode", CHAR_PARAMS(stPosParam.ucConfirm), ASSCII_ITEM},
	{"DES", 	CHAR_PARAMS(stPosParam.ucKeyMode), ASSCII_ITEM},
	{"adjust percent", 	INT_PARAMS(stPosParam.ucAdjustPercent), BIN_ITEM},
	{"EMV", 	CHAR_PARAMS(stPosParam.ucEmvSupport), ASSCII_ITEM},
	{"QPBOC", 	CHAR_PARAMS(stPosParam.ucqPbocSupport), ASSCII_ITEM},
	{"server ip1",	STR_PARAMS(stPosParam.szPOSRemoteIP),	STRING_ITEM},
	{"server port1", 	STR_PARAMS(stPosParam.szPOSRemotePort),	    STRING_ITEM},
	{"server ip2",	STR_PARAMS(stPosParam.szPOSRemoteIP2),	STRING_ITEM},
	{"server port2", 	STR_PARAMS(stPosParam.szPOSRemotePort2),	STRING_ITEM},
	{"wireless username",      STR_PARAMS(stPosParam.stTxnCommCfg.stWirlessPara.szUID), STRING_ITEM},
	{"wireless pwd",    STR_PARAMS(stPosParam.stTxnCommCfg.stWirlessPara.szPwd), STRING_ITEM},
	{"wireless apn",    STR_PARAMS(stPosParam.stTxnCommCfg.stWirlessPara.szAPN), STRING_ITEM},
	{"send timeout", 	INT_PARAMS(stPosParam.stTxnCommCfg.ulSendTimeOut), BIN_ITEM},

	{"remote ip 1",     STR_PARAMS(gstManageTcpCfg.szRemoteIP),	STRING_ITEM},
	{"remote port 1",   STR_PARAMS(gstManageTcpCfg.szRemotePort),	STRING_ITEM},
	{"remote ip 2",     STR_PARAMS(gstManageTcpCfg.szRemoteIP2),	STRING_ITEM},
	{"remote port 2",   STR_PARAMS(gstManageTcpCfg.szRemotePort2),	STRING_ITEM},

	{"remote wireless username", STR_PARAMS(gstManageTcpCfg.szPPPUID), STRING_ITEM},
	{"remote wireless pwd",   STR_PARAMS(gstManageTcpCfg.szPPPPwd), STRING_ITEM},
	{"PPP Dial Number",	   STR_PARAMS(gstManageTcpCfg.szPPPAPN), STRING_ITEM},
	

	{"remote wireless apn",	   STR_PARAMS(stPosParam.stDownParamCommCfg.stWirlessPara.szAPN), STRING_ITEM},
	{"remote send timeout",   INT_PARAMS(stPosParam.stDownParamCommCfg.ulSendTimeOut), BIN_ITEM},
	{"test mode", 	CHAR_PARAMS(stPosParam.ucTestFlag), ASSCII_ITEM},
	{"communication mode", 	    CHAR_PARAMS(stPosParam.stTxnCommCfg.ucCommType), ASSCII_ITEM},
	{"remote telphone",	STR_PARAMS(stPosParam.szDownloadTel),	  STRING_ITEM},
	{"remote TPDU",STR_PARAMS(stPosParam.szAsyncTpdu),	  STRING_ITEM},
	{"auto send count", CHAR_PARAMS(stPosParam.ucMaxOfflineTxn), BIN_ITEM},
	{"default print title", CHAR_PARAMS(stPosParam.ucPrnTitleFlag), ASSCII_ITEM},
	{"print title",	 STR_PARAMS(stPosParam.szPrnTitle),	  STRING_ITEM},
	{"support PINPAD", CHAR_PARAMS(stPosParam.ucSupPinpad), ASSCII_ITEM},
	{"predail", CHAR_PARAMS(stPosParam.bPreDial), ASSCII_ITEM},
	{"track encrypt", 	CHAR_PARAMS(stPosParam.ucTrackEncrypt), ASSCII_ITEM},
	{"default trans", 	CHAR_PARAMS(stPosParam.ucDefaltTxn), ASSCII_ITEM},
	{"hot key", 	    CHAR_PARAMS(stPosParam.ucSetFastKey), ASSCII_ITEM},
	{"electronic signature",    CHAR_PARAMS(stPosParam.ucSupportElecSign), ASSCII_ITEM},
	{"signature send times",    INT_PARAMS(stPosParam.iElecSignReSendTimes), BIN_ITEM},
	{"signature timeout",INT_PARAMS(stPosParam.iElecSignTimeOut), BIN_ITEM},
	{"server mode", 	CHAR_PARAMS(stPosParam.ucCommHostFlag), ASSCII_ITEM},
	{"Internet access", 	    CHAR_PARAMS(stPosParam.open_internet), ASSCII_ITEM},
	{"dns",	STR_PARAMS(stPosParam.stTxnCommCfg.stTcpIpPara.szDNS),  STRING_ITEM},

	{"volum control",              CHAR_PARAMS(stPosParam.stVolContrFlg), ASSCII_ITEM},
	{"stSodexoTId",    STR_PARAMS(stPosParam.stSodexoTId), STRING_ITEM},
	{"stPCSerial",    STR_PARAMS(stPosParam.stPCSerial), STRING_ITEM},
	{"stPCPid",    STR_PARAMS(stPosParam.stPCPid), STRING_ITEM},
	{"stPCpass",    STR_PARAMS(stPosParam.stPCpass), STRING_ITEM},
	{"stETId",    STR_PARAMS(stPosParam.stETId), STRING_ITEM},
	{"stETerm",    STR_PARAMS(stPosParam.stETerm), STRING_ITEM},
	{"stHeader1",    STR_PARAMS(stPosParam.stHeader1), STRING_ITEM},
	{"stAddress1",    STR_PARAMS(stPosParam.stAddress1), STRING_ITEM},
	{"stAddress2",    STR_PARAMS(stPosParam.stAddress2), STRING_ITEM},
	{"stAddress3",    STR_PARAMS(stPosParam.stAddress3), STRING_ITEM},
	{"stAffId",    STR_PARAMS(stPosParam.stAffId), STRING_ITEM},
	{"stTerminalId",    STR_PARAMS(stPosParam.stTerminalId), STRING_ITEM},
	{"*****************", NULL, 0,							0}
};

void str_convert(uint8_t *dest, uint8_t *srcStr, uint8_t len)
{
	memset(dest, 0x00, len);
	if( strlen((char *)srcStr)<len )
	{
		strcpy((char *)dest, (char *)srcStr);
	}
	else
	{
		memcpy((char *)dest, (char *)srcStr, len-1);
	}
}

void ascii_convert(uint8_t *dest, uint8_t *srcStr, uint8_t len)
{
	memset(dest, 0x00, len);
	memcpy(dest, srcStr, len);
}

void bcd_convert(uint8_t *dest, uint8_t *srcStr, uint8_t len)
{
	memset(dest, 0x00, len);
	PubAsc2Bcd((char *)srcStr, strlen((char *)srcStr), (char *)dest);
}


static uint8_t char_to_bin(uint8_t bchar)
{
	
	if( (bchar>='0')&&(bchar<='9') )
	{
		return(bchar-'0');
	}
	else if( (bchar>='A')&&(bchar<='F') )
	{
		return(bchar-'A'+10);
	}
	else if( (bchar>='a')&&(bchar<='f') )
	{
		return(bchar-'a'+10);
	}
	else
	{
		return 0;
	}
}

char hex_convert(uint8_t *dest, uint8_t *srcStr, uint8_t len)
{
	uint8_t buff[5];
	uint8_t i, k, *ptr, bin[2], achar;
	uint8_t srclen;

	srclen = strlen((char *)srcStr);
	if( srclen==0 )
	{
		memset(dest, 0x00, len);
		return 0;
	}
	ptr = srcStr;
	for(i=0; (i<len)&&(ptr<srcStr+srclen); )
	{
		memset(buff, 0x00, 5);
		for(k=0; (k<4)&&(ptr<srcStr+srclen); ptr++)
		{
			if( (*ptr!=',') && (*ptr!=' ') )
			{
				buff[k++] = *ptr;
			}
		}
		if( strlen((char *)buff)!=4 )
		{
			continue;
		}
		if( memcmp(buff, "0x", 2) )
		{
			continue;
		}
		bin[0] = char_to_bin(buff[2]);
		bin[0] = bin[0]<< 4;
		bin[1] = char_to_bin(buff[3]);
		achar = bin[0] + bin[1];
		dest[i++] = achar;
	}
	return 0;
}

void bin_convert(uint8_t *dest, uint8_t *srcStr, uint8_t len)
{
	uint8_t  achar;
	int16_t  i;
	int  l;

	switch(len)
	{
	case sizeof(achar):
		achar = (uint8_t)atoi((char *)srcStr);
		*dest = achar;
		break;
	case sizeof(i):
		i = atoi((char *)srcStr); 
		memcpy(dest, (uint8_t *)&i, len); 
		break;
	case sizeof(l):
		l = atol((char *)srcStr);
		memcpy(dest, (uint8_t *)&l, len);
		break;
	default:
		break;
	}
}


static void AllTrim(uint8_t *str)
{
	int len, count;
	uint8_t *p_str;

	len = strlen((char *)str);
	if (0 == len) return;

	p_str = str;
	p_str += len-1;
	while( (*p_str==' ')||(*p_str==0x09)||(*p_str==0x0d)||(*p_str==0x0a) )
	{	
		*p_str = 0x00;
		if(p_str == str)
		{
			break;
		}
		else
		{
			--p_str;
		}
	}
	p_str = str;
	len = strlen((char *)str);
	count = 0;
	while( (*p_str==' ')||(*p_str==0x09)||(*p_str==0x0d)||(*p_str==0x0a) )
	{
		if( p_str<(str+len) )
		{
			p_str++;
			count++;
		}
		else
		{
			break;
		}
	}
	if( count>0 )
	{
		memmove(str, p_str, len-count);
		memset(str+(len-count), 0x00, count);
	}
}


static int read_cnt;
static int readline(int fd, void *vptr, int maxlen)
{
	int n;
	uint8_t c, *ptr;
	static uint8_t *read_ptr, read_buf[400];

	ptr = vptr;
	for(n=1; n<maxlen; n++)
	{
		if( read_cnt<=0 )
		{
			if( (read_cnt=fileRead(fd, read_buf, sizeof(read_buf)))<0 )
			{
				return -1;
			} 
			else if( read_cnt==0 )
			{
				return 0;
			}
			read_ptr = read_buf;
		}
		read_cnt--;
		c = *read_ptr++;
	
		*ptr++ = c;
		if( c==0x0a )
		{
			break;
		}
	}
	*ptr = 0x00;
	return (n);
}



static int NextParam(int fid, uint8_t *name, uint8_t *value)
{
	uint8_t aline[200+1];
	uint8_t *ptr;
	int ret;

	memset(aline, 0x00, sizeof(aline));
	ret = readline(fid, aline, sizeof(aline)-1);
	if( ret<=0 )
	{
		return -1;
	}
	if( aline[0]=='&' )
	{
		return -1;
	}

	ptr = memchr(aline, '#', strlen((char *)aline));
	if ( ptr!=NULL )
	{
		if( aline[0]=='#')
    	{
   			*ptr = 0x00;
    	}
	}
	
	AllTrim(aline);
	ptr = memchr(aline, '=', strlen((char *)aline));
	if( ptr==NULL )
	{
		return 1;
	}

	memcpy(name, aline, ptr-aline);
	strcpy((char *)value, (char *)(ptr+1));
	AllTrim(name);
	AllTrim(value);
	return 0;	
}


int GetParams(char *filename, PARAMS *myParams)
{
	int i, j;
	PARAMS *q;
	uint8_t name[80], value[80];
	int ParamsFid;	

	ParamsFid = fileOpen(filename, O_RDWR);
	if( ParamsFid<0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FILE OPEN ERROR");
		lcdFlip();
		kbFlush();
		kbGetKey();
		return 1;
	}
	read_cnt = 0;//very important.

	for(;;)
	{
		memset(name, 0x00, sizeof(name));
		memset(value, 0x00, sizeof(value));
		j = NextParam(ParamsFid, name, value);
		if( j<0 )
		{
			break;
		}
		if( j>0 )
		{
			continue;
		}
		if( value[0]==0x0a )
		{
			continue;
		}
		for(i=0, q=myParams; ;i++, q++)
		{
			if( q->name[0]=='*' )
			{
				break;
			}
			if( strcmp((char *)name, (char *)q->name) )
			{
				continue;
			}

			switch(q->type)
			{
			case STRING_ITEM:	
				str_convert((uint8_t *)q->ini_ptr, value, q->len);
				break;
			case ASSCII_ITEM:
				ascii_convert((uint8_t *)q->ini_ptr, value, q->len);
				break;
			case HEX_ITEM:
				hex_convert((uint8_t *)q->ini_ptr, value, q->len);
				break;
			case BIN_ITEM:	
				bin_convert((uint8_t *)q->ini_ptr, value, q->len);
				break;
			default:
				break;
			}
			break;
		}
	}
	fileClose(ParamsFid);
	return 0;
}


int  GetParamfileName(void)
{
	int   iRet, iFileNums , iCnt;
	FILE_INFO  stFileInfoList[356];
	char  *pTemp,*pTemp1;
	
	memset(gszParamFileName,0,sizeof(gszParamFileName));
	strcpy(gszParamFileName,"appparam.txt");
	memset(stFileInfoList,0,sizeof(stFileInfoList));
	iFileNums = fileGetInfo(stFileInfoList);
	if (iFileNums <= 0)
	{
		return iFileNums;
	}
	
	for (iCnt=0; iCnt<iFileNums; iCnt++)
	{		
		pTemp = strstr(stFileInfoList[iCnt].name,".TXT");
		pTemp1 = strstr(stFileInfoList[iCnt].name,".txt");
		if (pTemp == NULL && pTemp1 == NULL)
		{
			continue;
		}
		
		iRet = fileOpen(stFileInfoList[iCnt].name,O_RDWR);
		if (iRet >= 0)
		{
			fileClose(iRet);
			memset(gszParamFileName,0,sizeof(gszParamFileName));
			memcpy(gszParamFileName,stFileInfoList[iCnt].name,sizeof(stFileInfoList[iCnt].name));
			return 0;
		}
		fileClose(iRet);
	}
	
	return 0;
}

int TxtToEnv(void)
{
	int iRet;
	
	if( fileExist(gszParamFileName)<0 )
	{
		LoadDefaultAppParam();
		SaveAppParam();
		return 0;
	}

	iRet = ReadParamsFile();
	if( iRet!=0 )
	{
		lcdCls();	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "READ PARA FAIL");
		lcdFlip();
		ErrorBeep();
		kbGetKey();					
		return 1;
	}

	AdjustAppParam();
	SaveAppParam();
	
	return 0;	
}

int  ReadParamsFile(void)
{
	int  ret;
	char sn_num[9] ={0};

	stPosParam.iDispLanguage = 0; 
	
	ret = GetParams(gszParamFileName, myParams);
	if( ret==0 )
	{
		fileRemove(gszParamFileName);
		lcdCls();	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " READ PARA SUC! ");
		lcdFlip();
		ErrorBeep();
		kbGetKey();	
	}
	else
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " READ PARA FAIL");
		lcdFlip();
		ErrorBeep();
		kbGetKey();	
		return ret;
	}

	PrintDebug("%s %d", "stTxnCommCfg.ucCommType:",stPosParam.stTxnCommCfg.ucCommType);
	sys_get_sn(sn_num, 9);	
	PrintDebug("%s %s","sn_num:",sn_num);
	PrintDebug("%s %d","sn_num_length:",strlen(sn_num));
	memcpy((char *)stPosParam.szPosId,sn_num,strlen(sn_num));
	PrintDebug("%s %s","stPosParam.szPosId:",stPosParam.szPosId);
	return 0;
}
//end of line

