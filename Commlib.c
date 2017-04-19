#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "Commlib.h"
#include "glbvar.h"
#include "posmacro.h"
#include "tranfunclist.h"
#include "New8210.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>


#define PosixGeterrno()  errno

/********************** Internal macros declaration ************************/
#define   NET_ERR_RETRY         1000
#define   MODEM_PATHNAME        "/dev/ttyS1"
#define   MODEM_PATHNAME1       "/dev/modem0"
#define   INVALID_SOCKET        0xFFFF
#define   ERR_PPP_CONTIMUE      (ERR_COMM_MODEMPPP_BASE|0x00FFF)
/********************** Internal structure declaration *********************/
typedef struct _tagERR_INFO
{
	int		iErrCode;
	uint8_t	szChnMsg[16+1];
	uint8_t	szEngMsg[16+1];
}ERR_INFO;

/********************** Internal functions declaration *********************/
//static int CheckLine(int iFirstStatus, int ipppcheckRet);
static int  ModemPPPDial(uint8_t ucDialMode);
static int  ModemPPPRxd(uint8_t *psRxdData, UINT32 uiExpLen, UINT32 uiTimeOut, UINT32 *puiOutLen);
static int  ModemPPPTxd(uint8_t *psTxdData, UINT32 uiDataLen, UINT32 uiTimeOut);
static int  ModemPPPClose(uint8_t bRelease);
static int RS232Dial(uint8_t ucDialMode);
static int RS232Txd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
static int RS232RawTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
 int RS232NacTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
static int RS232Rxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
static int RS232RawRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
 int RS232NacRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
static int RS232OnHook(uint8_t bReleaseAll);
static int  PSTNDial(uint8_t ucDialMode);
static int  PSTNTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
static int  PSTNRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
static int  PSTNSyncRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
static int  PSTNAsyncRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
static int  PSTNGetStatus(int *piStatus);
static int  PSTNInitModem(ModemDialParms_t *pstModemParam);
static int  PSTNOpenModem(void);
static int   SetSockAddr(ST_COMM_CONFIG *pstCfg);
static void  GetAllErrMsg(int iErrCode, ERR_INFO *pstInfo, COMM_ERR_MSG *pstCommErrMsg);
static uint8_t CalcLRC(uint8_t *psData, uint32_t uiLength, uint8_t ucInit);
/********************** Internal variables declaration *********************/
static int		    sg_ModemPPPsocket = -1;    //***MODEMPPP**
static int		    sg_ModemPPP_PreDial_Ret;
static int          sg_Wiresocket = -1 ;   
static int          sg_Tcpsocket =  -1 ;
static int		    sg_ModemPPP_PreDial_Ret;
static int          sg_iModemfd = -1;
static int          sg_iModemPPPfd = -1;
static int		    sg_PredialdisplayMsgFlag = 0;    //Predial whether to display the information on screen
static int          sg_InitModulRet = 0 ;
static struct sockaddr     sg_stWiresockaddr;
static struct sockaddr     sg_stTcpsockaddr;
static struct sockaddr     sg_stModemPPPsockaddr; //***MODEMPPP**
#ifndef _POS_TYPE_8210
static int     WirelessReset(ST_WIRELESS_INFO *pstParam);
static UINT32  sg_uiWirePPPLogoutTime = 0; //Storage before the first PPP successful hang time
static UINT8   sg_ucWirePPPFlag = 1;	   // wireless connect status 0-hang up 1-no hang up
static UINT8   sg_pppWaitFlag = 0; 		   // after ppp need to delay
static int     sg_Wire_PreDial_Ret;
#endif



static int   SSLConnectHost(int *CurSocket);
static INT32 sslDisconnectHost(int bShutDownSSL);
static SSL 		*ssl=NULL;
static SSL_CTX 	*ctx=NULL;
static BIO *bio_err = NULL;
static BIO *bio_out = NULL;
static int verify_depth = 3;
static int verify_error = 0;

#define METHOD_SSLV23			0
#define METHOD_SSLV2			1
#define METHOD_SSLV3			2
#define METHOD_TLSV1			3
#define METHOD_TLSV1_2			4


static char theCAfile[64] = "ca-cert.pem";
static char theCAfile2[64] = "ca-cert2.pem";
static int theSSLMethod = METHOD_TLSV1_2;//METHOD_SSLV3;//METHOD_TLSV1_2 
static unsigned char gbSslCreate = FALSE;//SSL create


static ST_COMM_CONFIG	sg_stCurCfg = {0xFF};
static uint8_t		sg_sWorkBuf[MAX_PACKET_LEN+50];
static ERR_INFO		sg_stCommErrMsg[] =
{
	{ERR_COMM_INV_PARAM, "",     "INVALID PARAM"},
	{ERR_COMM_INV_TYPE,  "",     "INV COMM TYPE"},
	{ERR_COMM_CANCEL,    "",     "USER CANCEL"},
	{ERR_COMM_TIMEOUT,   "",     "TIMEOUT"},
	{ERR_COMM_COMERR,    "",     "COMM ERROR"},
	{ERR_COMM_TOOBIG,    "",     "DATA TOO BIG"},
	{0, "", ""},
};

static ERR_INFO		sg_stRS232ErrMsg[] =
{          
	{PORT_RET_NOTEMPTY,    "",   "OVERFLOW"},
	{PORT_RET_PORTERR,     "",   "INVALID PORT"},
	{PORT_RET_NOTOPEN,     "",   "PORT CLOSED"},
	{PORT_RET_BUSY,        "",   "NO PORT AVAIL"},
	{PORT_RET_MODEM_INUSE, "",   "PORT OCCUPY"},
	{PORT_RET_PARAERR,     "",   "INVALID PARAM"},
	{PORT_RET_TIMEOUT,     "",   "TIMEOUT"},
	{0, "", ""},
};

static ERR_INFO		sg_stPPPErrMsg[] =
{
	{NET_ERR_RETRY,          "",          "TIMEOUT"},
	{ERR_COMM_NOT_ATTACHED,  "",          "ATTACHED ERROR"},
	{NET_ERR_RSP,            "",     		"CHECK MODULE"},
	{NET_ERR_NOSIM,          "",          "NO SIM CARD"},
	{NET_ERR_PIN,            "",       	"NEED SIM PIN"},
	{NET_ERR_PUK,            "",       	"NEED SIM PUK"},
	{NET_ERR_PWD,            "",          "SIM PIN ERROR"},
	{NET_ERR_SIMDESTROY,     "",          "NO SIM/NEED PIN"},
	{NET_ERR_CSQWEAK,        "",          "SIGNAL TOO WEAK"},
	{NET_ERR_LINKCLOSED,     "",          "NO CARRIER"},
	{NET_ERR_LINKOPENING,    "",          "LINK OK"},
	{NET_ERR_ATTACHING,      "",          "SEEKING NETWORK"},
	{NET_ERR_DETTACHED,      "",          "DETTACH NETWORK"},
	{NET_ERR_EMERGENCY,      "",          "PLS CHECK SIM"},
	{NET_ERR_RING,           "",          "ACCEPTED ERR"},
	{NET_ERR_BUSY,           "",          "COMMINICATE ERR"},
	{NET_ERR_DIALING,        "",          "LINKING"},
	{NET_ERR_UNKNOWN,        "",          "UNKNOW ERR"},
	{NET_ERR_ABNORMAL,       "",          "UNKNOW ERR"},
	{0, "", ""},
};

//#define	EINTR		 4	/* Interrupted system call */
//#define	EIO		 5	/* I/O error */
#ifndef ETIME
#define  ETIME   62
#endif

static ERR_INFO	sg_stModemErrMsg[] =
{
	{-MODEM_ERRNO_ERROR,             "",   "MODEM ERROR"},
	{-MODEM_ERRNO_BUSY,              "",   "MODEM BUSY"},
	{-MODEM_ERRNO_NO_DIALTONE,       "",   "NO DIAL TONE"},
	{-MODEM_ERRNO_NO_CARRIER,        "",   "LINE BREAK"},
	{-MODEM_ERRNO_NO_LINE,           "",   "LINE READY ?"},
	{-MODEM_ERRNO_NO_ANSWER,         "",   "NO ACK"},
	{-MODEM_ERRNO_OFF_HOOK,          "",   "PLS ONHOOK"},
	{-MODEM_ERRNO_LINE_IN_USE,       "",   "LINE BUSY"},
	{-MODEM_ERRNO_UN_OBTAINABLE,     "",   "UNOBTAINABLE"},
	{-MODEM_ERRNO_LINE_REVERSAL,     "",   "LINE REVERSAL"},
	{-MODEM_ERRNO_EXTENSION_IN_USE,  "",   "PHONE OCCUPIED"},
	{ERR_COMM_STATUS_TIMEOUT,        "",   "TIME OUT"},
	{ERR_COMM_MODEM_INIT,            "",   "MODEM INIT ERROR"},
	{NET_ERR_RETRY,                  "",   "TIMEOUT"},
	{4,                              "",   "LINE BREAK "},
	{5,                              "",   "LINE BREAK "},
	{16,                             "",   "MODEM BUSY"},
	{22,                             "",   "INVALID ARGUMENT"},
	{ETIMEDOUT,                      "",   "TIME OUT"},
	{ETIME,                          "",   "TIME OUT"},
	{0, "", ""},
};

/********************** external reference declaration *********************/
extern pthread_t  PreDialTid,InitCommTid;
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

int  GetCurSocket(void)
{
	if (sg_stCurCfg.ucCommType == CT_MODEMPPP)  //*******MODEMPPP********
	{
		return sg_ModemPPPsocket;
	}
	else if (sg_stCurCfg.ucCommType == CT_MODEM)  //*******MODEM********
	{
		return sg_iModemfd;
	}
	else if (sg_stCurCfg.ucCommType == CT_CDMA || sg_stCurCfg.ucCommType == CT_GPRS)
	{
		return sg_Wiresocket;
	}
	else if (sg_stCurCfg.ucCommType == CT_TCPIP || sg_stCurCfg.ucCommType == CT_WIFI)
	{
		return sg_Tcpsocket;
	}

	return -1;
}
 
int   SetSockAddr(ST_COMM_CONFIG *pstCfg)
{
	int  iRet,iPort;
	char temp[20], *p, *t,szBuffer[20];
	int  flag = 0,iCnt;

	if(pstCfg->ucCommType == CT_WIFI)
		p = (char *)pstCfg->stWIFIIpPara.szRemoteIP;
	else if(pstCfg->ucCommType==CT_TCPIP)
		p = (char *)pstCfg->stTcpIpPara.szRemoteIP;
	else
		p = (char *)pstCfg->stWirlessPara.szRemoteIP;
	
	t = temp;
	while (*p) {
		if (flag) {
			*t++ = *p;
			if (*p == '.')
				flag = 0;
			p++;
		} else {
			if (*p != '0')
				flag = 1;
			else
				p++;
		}
	}
	*t = '\0';
	
	if (temp[0]=='.')
	{
		memmove(&temp[1],temp,strlen(temp));
		temp[0] = '0';
		temp[strlen(temp)+1] = 0;
	}
	
	for (iCnt=0;iCnt<strlen(temp);iCnt++)
	{
		if ((temp[iCnt]=='.')&&(temp[iCnt+1]=='.'))
		{
			strcpy(szBuffer,&temp[iCnt+1]);
			temp[iCnt+1] = '0';
			strcpy(&temp[iCnt+2],szBuffer);
		}
	}
	
	if(pstCfg->ucCommType == CT_MODEMPPP)  //***********MODEMPPP************
	{	
		iPort = atoi((char*)pstCfg->stWirlessPara.szRemotePort);
		iRet = SockAddrset(&sg_stModemPPPsockaddr,temp,iPort);	
	}
	else if(pstCfg->ucCommType == CT_WIFI)
	{
		iPort = atoi((char *)pstCfg->stWIFIIpPara.szRemotePort);
		iRet = SockAddrset(&sg_stTcpsockaddr,temp,iPort);
	}
	else if(pstCfg->ucCommType==CT_TCPIP)
	{
		iPort = atoi((char *)pstCfg->stTcpIpPara.szRemotePort);
		iRet = SockAddrset(&sg_stTcpsockaddr,temp,iPort);
	}
	else
	{
		iPort = atoi((char *)pstCfg->stWirlessPara.szRemotePort);
		iRet = SockAddrset(&sg_stWiresockaddr,temp,iPort);		
	} 
	return iRet;
}

int CommInitModule(ST_COMM_CONFIG *pstCfg)
{
	int    iRet;

	CommSetCfgParam(pstCfg,0);

	switch (pstCfg->ucCommType)
	{
	case CT_WIFI:
		if ( (gstPosCapability.uiCommType & CT_GPRS)
			|| (gstPosCapability.uiCommType & CT_CDMA))
		{
			WnetSelectModule(1);
		}
		iRet = InitWiFiModule(TRUE);
		break;

	case CT_CDMA:
	case CT_GPRS:
		if ( gstPosCapability.uiCommType & CT_WIFI)
		{
			WnetSelectModule(0);
		}
		iRet = InitWirelessModule(&pstCfg->stWirlessPara);
		break;

	case CT_MODEM:
		iRet = PSTNInitModem(&pstCfg->stPSTNPara.stDialPara);
		break;

	default:
		iRet = 0;
		break;
	}

	sg_InitModulRet = iRet;
	if ((iRet & MASK_ERR_CODE) != 0)
	{
		return iRet;	
	}
	return 0;
}

// set comm param
int CommSetCfgParam(ST_COMM_CONFIG *pstCfg,uint8_t CheckWire)
{
	int		iLen;

	if( pstCfg==NULL )
	{
		return ERR_COMM_INV_PARAM;
	}

	switch( pstCfg->ucCommType )
	{
	case CT_MODEMPPP:
		if (!(gstPosCapability.uiCommType & CT_MODEMPPP))  //*******MODEMPPP******
		{
			return ERR_COMM_INV_PARAM;
		}
		
		if( pstCfg->stWirlessPara.szRemoteIP[0]==0	 ||
			pstCfg->stWirlessPara.szRemotePort[0]==0 )
		{
			return ERR_COMM_INV_PARAM;
		}
		break;

	case CT_RS232:
		iLen = strlen((char *)pstCfg->stRS232Para.szAttr);
		if( iLen<10 || iLen>20 )
		{
			return ERR_COMM_INV_PARAM;
		}
		break;

	case CT_MODEM:
		if (!(gstPosCapability.uiCommType & CT_MODEM))
		{
			return ERR_COMM_INV_PARAM;
		}
		if(  pstCfg->stPSTNPara.szTxnTelNo[0]==0  )
		{
			return ERR_COMM_INV_PARAM;
		}
		if( pstCfg->stPSTNPara.ucSendMode!=MODEM_COMM_ASYNC &&
			pstCfg->stPSTNPara.ucSendMode!=MODEM_COMM_SYNC )
		{
			return ERR_COMM_INV_PARAM;
		} 
		break;

	case CT_TCPIP:
		if (!(gstPosCapability.uiCommType & CT_TCPIP))
		{
			return ERR_COMM_INV_PARAM;
		}

		if( pstCfg->stTcpIpPara.szRemoteIP[0]==0   ||
			pstCfg->stTcpIpPara.szRemotePort[0]==0 )
		{
			return ERR_COMM_INV_PARAM;
		}
	    break;

	case CT_CDMA:
	case CT_GPRS:
		if (!(gstPosCapability.uiCommType & CT_CDMA) && !(gstPosCapability.uiCommType & CT_GPRS))
		{
			return ERR_COMM_INV_PARAM;
		}

		if( pstCfg->stWirlessPara.szAPN[0]==0     ||
			pstCfg->stWirlessPara.szRemoteIP[0]==0 )
		{
			return ERR_COMM_INV_PARAM;
		}
#ifndef _POS_TYPE_8110
		if(CheckWire)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PROCESANDO");
			lcdDisplay(0, 7, DISP_CFONT, "     TRANSMITIENDO   ");
			lcdFlip();
			usleep(200);
			
			CheckWireless();
			#if 0
			if (((gstPosCapability.uiCommType & CT_CDMA)&&(pstCfg->ucCommType==CT_GPRS))
				||((gstPosCapability.uiCommType & CT_GPRS)&&(pstCfg->ucCommType==CT_CDMA)))
			{
				return ERR_COMM_INV_PARAM;
			}
			#endif
		}
#endif		
		break;

	case CT_WIFI:
		if (!(gstPosCapability.uiCommType & CT_WIFI))
		{
			return ERR_COMM_INV_PARAM;
		}

		if( pstCfg->stWIFIIpPara.szRemoteIP[0]==0   ||
			pstCfg->stWIFIIpPara.szRemotePort[0]==0 )
		{
			return ERR_COMM_INV_PARAM;
		}
		break;

	default:
		return ERR_COMM_INV_TYPE;
	}

	memcpy(&sg_stCurCfg, pstCfg, sizeof(ST_COMM_CONFIG));

	return 0;
}


// 通讯模块拨号(MODEM拨号或者TCP建立TCP连接等)
int CommDial(uint8_t ucDialMode)
{
	int		iRet;

	switch( sg_stCurCfg.ucCommType )
	{
	case CT_MODEMPPP: 
		iRet = ModemPPPDial(ucDialMode);	 //*********MODEMPPP*******
		break;

	case CT_RS232:
		iRet = RS232Dial(ucDialMode);
		break;
		
	case CT_MODEM:
		iRet = PSTNDial(ucDialMode);
		break;
		
	case CT_TCPIP:
		lcdClrLine(2,7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_CLRLINE|DISP_MEDIACY, NULL, "PROCESANDO");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_CLRLINE, NULL, "     SEND DATA       ");
		lcdFlip();
		iRet = TcpDial(ucDialMode);
		if(ucDialMode!=PREDIAL_MODE && stPosParam.open_internet==PARAM_OPEN)
		{
			if(iRet == OK)
			{
				sg_Tcpsocket = GetCurTcpSocket();
				iRet = SSLConnectHost(&sg_Tcpsocket);
			}
		}
		break;
		
	case CT_CDMA:
	case CT_GPRS:
		iRet = WirelessDial(ucDialMode);
		lcdClrLine(2,7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_CLRLINE|DISP_MEDIACY, NULL, "PROCESANDO");
		DispMulLanguageString(0, 7, DISP_CFONT|DISP_CLRLINE, NULL, "     SEND DATA       ");
		lcdFlip();
		if(ucDialMode!=PREDIAL_MODE && stPosParam.open_internet==PARAM_OPEN)
		{
			if(iRet == OK)
			{
				sg_Wiresocket = GetCurWirelessSocket();
				iRet = SSLConnectHost(&sg_Wiresocket);	
			}
		}		
		break;
		
	case CT_WIFI:
		iRet = WiFiDial(ucDialMode);
		if(ucDialMode!=PREDIAL_MODE && stPosParam.open_internet==PARAM_OPEN)
		{
			if(iRet == OK)
			{
				sg_Tcpsocket = GetCurTcpSocket();
				iRet = SSLConnectHost(&sg_Tcpsocket);
			}
		}
		break;
		
	default:
		iRet = ERR_COMM_INV_TYPE;
		break;
	}

	return iRet;
}

// 通讯模块发送数据
int CommTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int		iRet;

	if( psTxdData==NULL )
	{
		return ERR_COMM_INV_PARAM;
	}

	switch( sg_stCurCfg.ucCommType )
	{
	case CT_MODEMPPP:
		iRet = ModemPPPTxd(psTxdData, uiDataLen, uiTimeOut);  //*****MODEMPPP*****
		break;

	case CT_RS232:
		iRet = RS232Txd(psTxdData, uiDataLen, uiTimeOut);
		break;

	case CT_MODEM:
		iRet = PSTNTxd(psTxdData, uiDataLen, uiTimeOut);
		break;

	case CT_TCPIP:
	case CT_WIFI:		
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = TcpTxd(psTxdData, uiDataLen, uiTimeOut);
		}
		else
		{
#ifndef _POS_TYPE_8210
			iRet = TcpTxd(psTxdData, uiDataLen, uiTimeOut);
#else
			iRet = TcpTxdNormal(psTxdData, uiDataLen, uiTimeOut);
#endif
		}	
		break;

	case CT_CDMA:
	case CT_GPRS:	
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = WirelessTxd(psTxdData, uiDataLen, uiTimeOut);
		}
		else
		{
#ifndef _POS_TYPE_8210
			iRet = WirelessTxd(psTxdData, uiDataLen, uiTimeOut);
#else
			iRet = WirelessTxdNormal(psTxdData, uiDataLen, uiTimeOut);
#endif
		}
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
		break;
	}

	return iRet;
}

// 通讯模块接收数据
int CommRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int		iRet;

	if( psRxdData==NULL )
	{
		return ERR_COMM_INV_PARAM;
	}

	switch( sg_stCurCfg.ucCommType )
	{
	case CT_MODEMPPP:
		if (uiTimeOut<=20)
		{
			uiTimeOut = 20;
		}
		iRet = ModemPPPRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
		break;

	case CT_RS232:
		if (uiTimeOut<=5)
		{
			uiTimeOut = 5;
		}
		iRet = RS232Rxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
		RS232OnHook(FALSE);
		break;

	case CT_MODEM:
		if (uiTimeOut<=15)
		{
			uiTimeOut = 15;
		}
		iRet = PSTNRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
		break;

	case CT_TCPIP:
	case CT_WIFI:
		if (uiTimeOut<=10)
		{
			uiTimeOut = 10;
		}	
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = TcpRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
			sslDisconnectHost(TRUE);//必须要每次加载SSL，不加载SSL的话，创建会失败
			TcpOnHook(FALSE);
		}
		else
		{
#ifndef _POS_TYPE_8210
			iRet = TcpRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
			TcpOnHook(FALSE);
#else		
			iRet = TcpRxdNormal(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
			TcpOnHook(FALSE);
#endif
		}	
		break;

	case CT_CDMA:
	case CT_GPRS:
		if (uiTimeOut<=15)
		{
			uiTimeOut = 15;
		}
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = WirelessRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
			sslDisconnectHost(TRUE);//必须要每次加载SSL，不加载SSL的话，创建会失败
			WirelessClose(FALSE,0,0);
		}
		else
		{
#ifndef _POS_TYPE_8210
			iRet = WirelessRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
			WirelessClose(FALSE,0,0);
#else		
			iRet = WirelessRxdNormal(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
#endif
		}
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
		break;
	}

	return iRet;
}

// hang up
int CommOnHook(uint8_t bReleaseAll)
{
	int		iRet = 0;

	if(stPosParam.open_internet==PARAM_OPEN)
	{
		iRet = sslDisconnectHost(TRUE);
	}

//stDownParamCommCfg need to assign
	if(PosCom.stTrans.iTransNo != DOWNLOAD)
		sg_stCurCfg.ucCommType = stPosParam.stTxnCommCfg.ucCommType;
	switch( sg_stCurCfg.ucCommType )
	{
	case CT_MODEMPPP:
		iRet = ModemPPPClose(TRUE);  //*****MDEMPPP********** //
		break;

	case CT_RS232:
		iRet = RS232OnHook(bReleaseAll);
		break;

	case CT_MODEM:
		iRet = PSTNOnHook(bReleaseAll);
		break;

	case CT_TCPIP:
	case CT_WIFI:
		iRet = TcpOnHook(bReleaseAll);
		break;

	case CT_CDMA:
	case CT_GPRS:
		iRet = WirelessClose(bReleaseAll,0,1);
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
		break;
	}

	return iRet;
}
 
void CommGetErrMsg(int iErrCode, COMM_ERR_MSG *pstCommErrMsg)
{
	int  iReErrCode;

	if( pstCommErrMsg==NULL )
	{
		return;
	}

	iReErrCode = iErrCode & MASK_ERR_CODE;

	switch( iErrCode & MASK_COMM_TYPE )
	{
	case ERR_COMM_MODEMPPP_BASE:				//*********MODEMPPP*********
		sprintf((char *)pstCommErrMsg->szEngMsg, "ModemPPP ERR:%04X", iReErrCode);
		GetAllErrMsg(iErrCode, sg_stPPPErrMsg, pstCommErrMsg);
		break;

	case ERR_COMM_ALL_BASE:
		sprintf((char *)pstCommErrMsg->szEngMsg, "COMM ERR:%04X", iReErrCode);
		GetAllErrMsg(iErrCode, sg_stCommErrMsg, pstCommErrMsg);
		break;

	case ERR_COMM_RS232_BASE:
		sprintf((char *)pstCommErrMsg->szEngMsg, "PORT ERR:%04X", iReErrCode);
		GetAllErrMsg(iErrCode, sg_stRS232ErrMsg, pstCommErrMsg);
		break;

	case ERR_COMM_MODEM_BASE:
		sprintf((char *)pstCommErrMsg->szEngMsg, "MODEM ERR:%04X", iReErrCode);
		GetAllErrMsg(iErrCode, sg_stModemErrMsg, pstCommErrMsg);
	    break;

	case ERR_COMM_TCPIP_BASE:
		sprintf((char *)pstCommErrMsg->szEngMsg, "TCPIP ERR:%04X", iReErrCode);
		GetAllErrMsg(iErrCode, sg_stPPPErrMsg, pstCommErrMsg);
		break;
	case ERR_COMM_WIRELESS_BASE:
		sprintf((char *)pstCommErrMsg->szEngMsg, "WIRE ERR:%04X", iReErrCode);
		GetAllErrMsg(iErrCode, sg_stPPPErrMsg, pstCommErrMsg);
		break;
	default:
		sprintf((char *)pstCommErrMsg->szEngMsg, "ERR CODE:%04X", iReErrCode);
		break;
	}
}

void GetAllErrMsg(int iErrCode, ERR_INFO *pstInfo, COMM_ERR_MSG *pstCommErrMsg)
{
	int		iCnt;

	for(iCnt=0; pstInfo[iCnt].iErrCode!=0; iCnt++)
	{
		if( pstInfo[iCnt].iErrCode==iErrCode ||
			pstInfo[iCnt].iErrCode==(iErrCode & MASK_ERR_CODE) )
		{
			sprintf((char *)pstCommErrMsg->szEngMsg, "%.16s", pstInfo[iCnt].szEngMsg);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// RS232 
//////////////////////////////////////////////////////////////////////////
int RS232Dial(uint8_t ucDialMode)
{
	int 	iRet;

	if( ucDialMode==PREDIAL_MODE )
	{
		return 0;
	}

	iRet = portOpen(sg_stCurCfg.stRS232Para.ucPortNo,(char *)sg_stCurCfg.stRS232Para.szAttr);
	if( iRet != PORT_RET_OK )
	{
		return (ERR_COMM_RS232_BASE | iRet);
	}

	return PORT_RET_OK;
}

// send data
int RS232Txd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int		iRet;

	if (sg_stCurCfg.stRS232Para.ucSendMode == CM_RAW)
	{	
		iRet = RS232RawTxd(psTxdData, uiDataLen, uiTimeOut);
	}
	else
	{
		iRet = RS232NacTxd(psTxdData, uiDataLen, uiTimeOut);
	}

	return iRet;
}

#ifdef _POS_TYPE_8210
int RS232RawTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int iFd;
	
	iFd = GetPortFd(sg_stCurCfg.stRS232Para.ucPortNo);
	return TcpSendData(iFd, psTxdData, uiDataLen);
}
#else
// receive data
int RS232RawTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int 	iRet;
	int     iOldTime,iNowTime;

	if (uiTimeOut<=0)
	{
		uiTimeOut = 2;
	}
	iOldTime = sysGetTimerCount();
	iRet = 0;
	while (1)
	{
		iNowTime = sysGetTimerCount();
		if ((iNowTime-iOldTime) >= uiTimeOut*1000)
		{
			iRet =  ERR_COMM_RS232_BASE | iRet;
			break;
		}

		iRet = portSends(sg_stCurCfg.stRS232Para.ucPortNo, psTxdData,uiDataLen);
		if( iRet == PORT_RET_OK )
		{
			break;
		}
		sysDelayMs(50);
	}
 
	return iRet;
}
#endif

// STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
int RS232NacTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int		iRet;

	if( uiDataLen>MAX_PACKET_LEN )
	{
		return ERR_COMM_TOOBIG;
	}

	sg_sWorkBuf[0] = STX;
	sg_sWorkBuf[1] = (uiDataLen/1000)<<4    | (uiDataLen/100)%10;	// convert to BCD
	sg_sWorkBuf[2] = ((uiDataLen/10)%10)<<4 | uiDataLen%10;
	memcpy(&sg_sWorkBuf[3], psTxdData, uiDataLen);
	sg_sWorkBuf[3+uiDataLen]   = ETX;
	sg_sWorkBuf[3+uiDataLen+1] = CalcLRC(psTxdData, uiDataLen, (uint8_t)(sg_sWorkBuf[1] ^ sg_sWorkBuf[2] ^ ETX));

	iRet = RS232RawTxd(sg_sWorkBuf, (uint32_t)(uiDataLen+5), uiTimeOut);	// data
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}

// receive data
int RS232Rxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int		iRet;

	if (sg_stCurCfg.stRS232Para.ucSendMode == CM_RAW)
	{
		iRet = RS232RawRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
	}
	else
	{
		iRet = RS232NacRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
	}

	return iRet;
}

// receive data
int RS232RawRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int   iRet,iFlag;
	uint32_t	uiReadCnt, uiTemp;
	uint32_t  iOldTimeMs,iNewTimeMs;

	uiReadCnt = iFlag = 0;
	uiTemp = 5000;
	iOldTimeMs = sysGetTimerCount();
	while( uiReadCnt<uiExpLen )
	{
		if (iFlag==0)
		{
			iNewTimeMs= sysGetTimerCount();
			if( (iNewTimeMs-iOldTimeMs) >= (uint32_t)(uiTimeOut *1000) )	// check the timer
			{
				if( uiReadCnt>0 )	// have data
				{
					break;
				}

				return ERR_COMM_TIMEOUT;
			}

			lcdDisplay(87,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNewTimeMs-iOldTimeMs)/1000);
			iRet = portCheckRecvBuf(sg_stCurCfg.stRS232Para.ucPortNo);
			if (iRet != PORT_RET_NOTEMPTY)
			{
				if (iRet != PORT_RET_OK )
				{
					return (ERR_COMM_RS232_BASE | iRet);
				}
				
				continue;
			}
			else
			{
				iFlag = 1;
			}
		}

		iRet = portRecv(sg_stCurCfg.stRS232Para.ucPortNo, psRxdData, uiTemp);
		if( iRet==0x00 )
		{	// receive successful
			uiTemp = 80;
			psRxdData++;
			uiReadCnt++;
		}
		else if( iRet==0xFF )
		{
			if( uiReadCnt>0 )
			{
				break;
			}

			return ERR_COMM_TIMEOUT;
		}
		else
		{
			return (ERR_COMM_RS232_BASE | iRet);
		}
	}   // end of while( uiReadCnt<uiExpLen

	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiReadCnt;
	}

	return 0;
}

#ifdef _POS_TYPE_8210
int RS232NacRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int iRet=0,ierror,iLen=0,iLenbak=0,iOldTime,iNewTime,iNewTimeOut = 0,iFd;
	uint8_t ucTmpBuf[MAX_PACKET_LEN];

	iFd = GetPortFd(sg_stCurCfg.stRS232Para.ucPortNo);

	memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
	iOldTime = sysGetTimerCount();
	while (1)
	{
		iRet = wc_recv_data(iFd,ucTmpBuf,1,uiTimeOut-iNewTimeOut,DispRecvTime);//get 0X02
		iNewTime = sysGetTimerCount(); //get the time of wc_recv_data
		if (iRet == 1 && ucTmpBuf[0] == 0x02)
			break;		
		else if(iRet<=0)
		{
			ierror = errno;
			if(iNewTime-iOldTime > uiTimeOut*1000)
				DispFailComm(TIMEOUT,ierror);
			else
				DispFailComm(iRet,ierror);
			return NO_DISP;
		}
		iNewTimeOut = (iNewTime-iOldTime)/1000;  
		if(iNewTimeOut > uiTimeOut)				   //timeout
		{
			ierror = errno;
			DispFailComm(TIMEOUT,ierror);
			return NO_DISP;
		}
	}

	iNewTimeOut = (iNewTime-iOldTime)/1000;
	while(1)
	{
		iRet = wc_recv_data(iFd,ucTmpBuf+1+iLenbak,uiExpLen-iLenbak-1, uiTimeOut-iNewTimeOut,DispRecvTime);
		iNewTime = sysGetTimerCount(); 
		if(iRet<=0)
		{
			ierror = errno;
			if(iNewTime-iOldTime > uiTimeOut*1000)
				DispFailComm(TIMEOUT,ierror);
			else
				DispFailComm(iRet,ierror);
			return NO_DISP;
		}

		iLenbak += iRet;
		if(iLenbak >= 2)
			iLen = PubBcd2Long(&ucTmpBuf[1], 2, NULL);	

		if(iLenbak>=iLen+2+2)// 2 byte len，1byte ETX，1 byte check bit
		{
			break;
		}
		iNewTimeOut = (iNewTime-iOldTime)/1000; 
		if(iNewTimeOut > uiTimeOut)				   
		{
			ierror = errno;
			DispFailComm(TIMEOUT,ierror);
			return NO_DISP;
		}
	}

	if (ucTmpBuf[1 + 2 + iLen] != 0x03)//etx
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "comm fail");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}

    if( CalcLRC(ucTmpBuf + 1, 2 + iLen + 1,0x00) != ucTmpBuf[1+2+iLen+1])// check bit
    {
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "comm fail");
		DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL, "crc fail");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
    }

	memcpy(psRxdData, &ucTmpBuf[3], iLen);
	if( puiOutLen!=NULL )
	{
		*puiOutLen = iLen;
	}
	
	return 0;
}
#else
// STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
int RS232NacRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int   iRet ,iFlag;
	uint32_t	uiReadCnt, uiLength;
	uint32_t  iOldTimeMs,iNewTimeMs,uiTemp;
	
	if( uiExpLen>MAX_PACKET_LEN)
	{
		return ERR_COMM_TOOBIG;
	}
	
	iFlag = 0;
	uiTemp = 5000;
	uiReadCnt = uiLength = 0;
	memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));
	iOldTimeMs = sysGetTimerCount();
	while( 1 )
	{
		if (iFlag==0)
		{
			iNewTimeMs= sysGetTimerCount();
			if( (iNewTimeMs-iOldTimeMs) >= (uint32_t)(uiTimeOut *1000) )	// check the timer
			{
				if( uiReadCnt>0 )	// have data
				{
					break;
				}
				
				return ERR_COMM_TIMEOUT;
			}
			
			lcdDisplay(87,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNewTimeMs-iOldTimeMs)/1000);
			iRet = portCheckRecvBuf(sg_stCurCfg.stRS232Para.ucPortNo);
			if (iRet != PORT_RET_NOTEMPTY)
			{
				if (iRet != PORT_RET_OK )
				{
					return (ERR_COMM_RS232_BASE | iRet);
				}
				
				continue;
			}
			else
			{
				iFlag = 1;
			}
		}
		
		iRet = portRecv(sg_stCurCfg.stRS232Para.ucPortNo, &sg_sWorkBuf[uiReadCnt], uiTemp);
		if( iRet!=0 )
		{
			if( iRet==0xFF )
			{
				continue;
			}
			return ERR_COMM_RS232_BASE|iRet;
		}
		uiTemp = 100;
		if( sg_sWorkBuf[0]!=STX )
		{
			continue;
		}
		
		uiReadCnt++;
		if( uiReadCnt==3 )
		{
			uiLength =  ((sg_sWorkBuf[1]>>4) & 0x0F) * 1000 + (sg_sWorkBuf[1] & 0x0F) * 100 +
				((sg_sWorkBuf[2]>>4) & 0x0F) * 10   + (sg_sWorkBuf[2] & 0x0F);
		}
		if( uiReadCnt==uiLength+5 )
		{	// read data ok, verify it ...
			if( sg_sWorkBuf[uiReadCnt-2]==ETX &&
				CalcLRC(&sg_sWorkBuf[1], (uint32_t)(uiReadCnt-1), 0)==0 )
			{
				break;
			}
			return ERR_COMM_COMERR;
		}
	}
	
	memcpy(psRxdData, &sg_sWorkBuf[3], uiLength);
	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiLength;
	}
	
	return 0;
}
#endif


// close rs232
int RS232OnHook(uint8_t bReleaseAll)
{
	int	iRet;

	iRet = portClose(sg_stCurCfg.stRS232Para.ucPortNo);
	if( iRet ==0 )
	{
		return 0;
	}

	return (ERR_COMM_RS232_BASE | iRet);
}

#define MODEM_COMM_ERR(a)		(ERR_COMM_MODEM_BASE|(a))
#define MODEMPPP_ERR(a)	        (ERR_COMM_MODEMPPP_BASE|(a))

// Modem hang up
INT32 ModemOnHook(void)
{
	int  iRet,iStatus;

	modem_wait_until_sent(sg_iModemPPPfd);

	// Exclude repeatedly hung request
	iRet = modem_get_status(sg_iModemPPPfd, &iStatus);
	if ( ! (iStatus == (0x80000000 | MODEM_STATE_CONNECT)) )
		iRet = modem_hangup(sg_iModemPPPfd);

	while(1)
	{
		iRet = modem_get_status(sg_iModemPPPfd, &iStatus);
		if (iRet != 0)
			break;
		
		if (iStatus & 0x80000000) // At present the highest - 1 : the operation has not yet completed, need to check again
		{
			sysDelayMs(100);
			continue;
		}
		break;
	}
	iRet = modem_close(sg_iModemPPPfd);
	sg_iModemPPPfd = -1;
	
	if (iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEM_COMM_ERR(iRet);
	}
	return iRet;
}

#if 0
int  CheckLine(int iFirstStatus, int ipppcheckRet)
{
	int iRet,status;

	iRet = status = 0;

#ifdef _POS_TYPE_8110
	if (iFirstStatus==1 && ipppcheckRet == -100)
	{
		return ERR_PPP_CONTIMUE;
	}
	else
	{
		return 0;
	}
#endif

	if (sg_iModemPPPfd>=0)
	{
		ModemOnHook();
	}
	sg_iModemPPPfd = PSTNOpenModem(); 
	if (sg_iModemPPPfd < 0)
	{
		return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT); 
	}

	while(1)
	{
		if( (iRet = modem_get_status(sg_iModemPPPfd, &status)) )
		{
			ModemOnHook();
			iRet = ABS(iRet);
			iRet = MODEM_COMM_ERR(iRet);
			return iRet;
		}
		if(status & 0x80000000)
		{
			sysDelayMs(10*1000);
		}
		else
		{
			break;
		}
	}
	iRet = modem_check_extension(sg_iModemPPPfd);
	if (iRet!=0)
	{
		ModemOnHook();
		if(iRet != 0)
		{
			iRet = ABS(iRet);
			iRet = MODEM_COMM_ERR(iRet);
		}
	}
	else
	{			
		iRet = ModemOnHook();
	}		
	return iRet;
}
#endif

//////////////////////////////////////////////////////////////////////////
// modem ppp
//////////////////////////////////////////////////////////////////////////
#define MODEMPPP_ERR(a)	(ERR_COMM_MODEMPPP_BASE|(a))

INT32 ModemPPPDial(uint8_t ucDialMode)
{
	int iRet,iFlag,iRetyTimes,iDialTimes;
	UINT32   uiOldTime,uiNewTime;
	char     szApn[100];
	int      iCheckFistLogon;

	iCheckFistLogon = 0;
	kbFlush();
	memset(szApn,0,sizeof(szApn));
	if( ucDialMode==PREDIAL_MODE )
	{
		sg_ModemPPP_PreDial_Ret = 0;
		iRet = PPPCheck(OPT_DEVMODEM);
		if (iRet < 0)
		{
			sprintf(szApn,"%s",sg_stCurCfg.stWirlessPara.szAPN);
			iRet = PPPLogin(OPT_DEVMODEM, szApn, (char *)sg_stCurCfg.stWirlessPara.szUID,
								(char *)sg_stCurCfg.stWirlessPara.szPwd,PPP_ALG_PAP, 60);
			if (iRet<0)
			{
				iRet = -iRet;
			}
			sg_ModemPPP_PreDial_Ret = iRet;
		}
		return MODEMPPP_ERR(iRet);
	}

	iDialTimes = 1;
	iFlag = 1;
	iRetyTimes = 1;
	uiOldTime = sysGetTimerCount();	
	
	lcdClrLine(2,7);
	lcdDisplay(0,3,DISP_CFONT,"DIALING...");
	DrawRect(0, 17, 127, 63);
	lcdFlip();

	while (iFlag)
	{
		uiNewTime = sysGetTimerCount();	
		if( (uiNewTime-uiOldTime) >= sg_stCurCfg.ulSendTimeOut)
		{ 
			ModemPPPClose(TRUE);
			return MODEMPPP_ERR(NET_ERR_RETRY);
		}
		if( PubChkKeyInput() )
		{
			if (kbGetKey() == KEY_CANCEL)
			{
				return ERR_USERCANCEL;
			}
		}

		lcdDisplay(87,3,DISP_CFONT,"(%d)",(uiNewTime-uiOldTime)/1000);
		lcdFlip();

		iRet = PPPCheck(OPT_DEVMODEM);
		if (iRet == -NET_ERR_LINKOPENING)
		{
			sysDelayMs(400);
			iDialTimes--;
			continue;
		}	

		if (iRet < 0)
		{
//			iRet = CheckLine(iCheckFistLogon, iRet);
//			if(iRet!=0)
//			{
//				if (iRet != ERR_PPP_CONTIMUE)
//				{
//					return iRet;
//				}
//
//				sysDelayMs(400);
//				iDialTimes--;
//				continue;
//			}

			sprintf(szApn,"%s",sg_stCurCfg.stWirlessPara.szAPN);
			iRet = PPPLogin(OPT_DEVMODEM, szApn, (char *)sg_stCurCfg.stWirlessPara.szUID,
								(char *)sg_stCurCfg.stWirlessPara.szPwd,PPP_ALG_PAP, 60);
			if (iRet<0)
			{
				iRet = -iRet;
			}			
			sg_ModemPPP_PreDial_Ret = iRet;
			iCheckFistLogon = 1;
			sysDelayMs(100);
			continue;
		}
	
		if (iRetyTimes > 3 )
		{    
			ModemPPPClose(TRUE);
			return MODEMPPP_ERR(NET_ERR_RETRY);
		}
		
		//tcp
		if (sg_ModemPPPsocket == -1)
		{
			sysDelayMs(1000);
			sg_ModemPPPsocket = NetSocket(AF_INET, SOCK_STREAM, 0);
			if(sg_ModemPPPsocket < 0)
			{
				iRet = sg_ModemPPPsocket;
				if (iRet<0)
				{
					iRet = -iRet;
				}

				ModemPPPClose(FALSE);
				return MODEMPPP_ERR(iRet);
			}
			iRet = SetSockAddr(&sg_stCurCfg);
			if (iRet != NET_OK)
			{
				if (iRet<0)
				{
					iRet = -iRet;
				}
				ModemPPPClose(FALSE);
				return MODEMPPP_ERR(iRet);
			}

			lcdClrLine(2,7);
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"Modem CONNECT(%d)",iRetyTimes++);
			DrawRect(0, 17, 127, 63);
			lcdFlip();

			iRet = NetConnect(sg_ModemPPPsocket,&sg_stModemPPPsockaddr,sizeof(struct sockaddr));
			if (iRet != NET_OK)
			{
				ModemPPPClose(FALSE);
				iRet = MODEMPPP_ERR(iRet);
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{ 
			return 0;
		}
	}
	
	if (iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEMPPP_ERR(iRet);
	}

	return iRet;
}

//Modem PPP receive data 
INT32 ModemPPPRxd(uint8_t *psRxdData, UINT32 uiExpLen, UINT32 uiTimeOut, UINT32 *puiOutLen)
{
	int  iRet,  iRetryCnt;
	int  iOldTime,iNowTime;
	UINT32  ulRealRecvLen = 0;
	unsigned long  ulTemp = 0;
	struct timeval stTimeVal;
	socklen_t      Socklen;
	char szBuffer[5]; 

	stTimeVal.tv_sec = 1;
	stTimeVal.tv_usec = 0;
	Socklen = sizeof(stTimeVal);
	NetSetsockopt(sg_ModemPPPsocket,SOL_SOCKET,SO_RCVTIMEO,&stTimeVal,Socklen);
	
	iRetryCnt = 0;
	iOldTime = sysGetTimerCount();
	while( 1 )
	{
		iNowTime= sysGetTimerCount();
		if( (iNowTime-iOldTime) >= (UINT32)(uiTimeOut *1000) )	
		{
			iRet = ERR_COMM_TIMEOUT;
			ModemPPPClose(FALSE);
			break;
		}

		lcdDisplay(87,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNowTime-iOldTime)/1000);	
		iRet = NetRecv(sg_ModemPPPsocket,psRxdData+ulRealRecvLen,uiExpLen,0);
		if (iRet > 0)
		{
#ifdef _TCP_BCD_LEN_BYTE
			PubBcd2Long(psRxdData,  2, &ulTemp);
#else
			memcpy(szBuffer, psRxdData, 4);
			ulTemp = atoi(szBuffer);
#endif
			ulRealRecvLen += iRet;
			if (ulRealRecvLen < ulTemp+4)
			{
				continue;
			}
			*puiOutLen = ulRealRecvLen;
			iRet = 0;
			break;
		}
		else
		{
			iRetryCnt++;
			if (iRetryCnt>3)
			{
				if (iRet<0)
				{
					iRet = -iRet;
				}
				iRet = MODEMPPP_ERR(iRet);
			}

			sysDelayMs(20);
		}
	}	
	return iRet;
}

// Modem PPP send data
INT32 ModemPPPTxd(uint8_t *psTxdData, UINT32 uiDataLen, UINT32 uiTimeOut)
{
	int  iRet,iRelSendlen;
	struct timeval stTimeVal;
	
	stTimeVal.tv_sec = 30;
	stTimeVal.tv_usec = 0;
	NetSetsockopt(sg_ModemPPPsocket,SOL_SOCKET,SO_SNDTIMEO,&stTimeVal,sizeof(stTimeVal));

	for (iRelSendlen=0; iRelSendlen<uiDataLen;)
	{
		iRet = NetSend(sg_ModemPPPsocket,psTxdData+iRelSendlen,(uiDataLen-iRelSendlen),0);
		if (iRet < 0)
		{			  
		    ModemPPPClose(TRUE);
			iRet = -iRet;
			return MODEMPPP_ERR(iRet);
		}
		iRelSendlen += iRet;
	}
	return 0;
}


// PPP  hang up
INT32 ModemPPPClose(uint8_t bRelease)
{
	int iRet;

	iRet = NetClose(sg_ModemPPPsocket);
	sg_ModemPPPsocket = -1;

	if (bRelease)
	{	
		iRet = PPPLogout(OPT_DEVMODEM);
		if (iRet<0)
		{
			iRet = PPPLogout(OPT_DEVMODEM); //Found in the tests after completion of the transaction line and murmur, no hang up
		}
	}
	if (iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEMPPP_ERR(iRet);
	}
	return iRet;
}

#ifndef _POS_TYPE_8210
//////////////////////////////////////////////////////////////////////////
// TCP 
//////////////////////////////////////////////////////////////////////////
int TcpDial(uint8_t ucDialMode)
{
	int iRet,iFlag,iRetyTimes;
	uint32_t   uiOldTime;

	if( ucDialMode==PREDIAL_MODE )
	{
		return 0;
	}

	iFlag = 1;
	iRetyTimes = 1;
	uiOldTime = sysGetTimerCount();	
	while (iFlag)
	{
		if ( sysGetTimerCount() >= (uiOldTime+sg_stCurCfg.ulSendTimeOut) )
		{ 
			TcpOnHook(FALSE);
			return (ERR_COMM_TCPIP_BASE|NET_ERR_RETRY);
		}
		if( PubChkKeyInput() )
		{
			if (kbGetKey() == KEY_CANCEL)
			{
				TcpOnHook(FALSE);
				return ERR_USERCANCEL;
			}
		}
		
		if (iRetyTimes > 3 )
		{
			TcpOnHook(FALSE);
			return (ERR_COMM_TCPIP_BASE|NET_ERR_RETRY);
		}
		
		if (sg_Tcpsocket < 0)
		{
			sg_Tcpsocket = NetSocket(AF_INET,SOCK_STREAM,0);
			if (sg_Tcpsocket < 0)
			{
				TcpOnHook(FALSE);
				iRet = sg_Tcpsocket;
				if (iRet<0)
				{
					iRet = -iRet;
				}
				return (ERR_COMM_TCPIP_BASE|iRet);
			}

			iRet = SetSockAddr(&sg_stCurCfg);
			if (iRet != NET_OK)
			{
				if (iRet<0)
				{
					iRet = -iRet;
				}
				TcpOnHook(TRUE);
				return ERR_COMM_TCPIP_BASE|iRet;
			}

			lcdClrLine(2,7);
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"TCPIP CONNECT(%d)",iRetyTimes++);
			DrawRect(0, 17, 127, 63);
			lcdFlip();

			iRet = NetConnect(sg_Tcpsocket,&sg_stTcpsockaddr,sizeof(struct sockaddr));		
			if (iRet != NET_OK)
			{
				CommOnHook(FALSE);  
				if (iRet<0)
				{
					iRet = -iRet;
				}
				iRet = ERR_COMM_TCPIP_BASE|iRet;
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			return 0;
		}
	}

	return iRet;
}
#endif

#if 0
// send data
int TcpTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int  iRet,iRelSendlen;
	struct timeval stTimeVal;
	
	stTimeVal.tv_sec = 30;
	stTimeVal.tv_usec = 0;
	NetSetsockopt(sg_Tcpsocket,SOL_SOCKET,SO_SNDTIMEO,&stTimeVal,sizeof(stTimeVal));
	
	for (iRelSendlen=0; iRelSendlen<uiDataLen;)
	{
		iRet = NetSend(sg_Tcpsocket,psTxdData+iRelSendlen,(uiDataLen-iRelSendlen),0);
		printf("NetSend iRet = %d\n",iRet);
		if (iRet < 0)
		{
			TcpOnHook(TRUE);
			iRet = -iRet;
			return ERR_COMM_TCPIP_BASE|iRet;
		}
		iRelSendlen += iRet;
	}
	
	return 0;
}
#endif


// 发送数据
int TcpTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int  iRet,iRelSendlen;
	struct timeval stTimeVal;
	
	stTimeVal.tv_sec = 30;
	stTimeVal.tv_usec = 0;
	PrintDebug("%s%d","TcpTxd sg_Tcpsocket:",sg_Tcpsocket);
	NetSetsockopt(sg_Tcpsocket,SOL_SOCKET,SO_SNDTIMEO,&stTimeVal,sizeof(stTimeVal));
	
	for (iRelSendlen=0; iRelSendlen<uiDataLen;)
	{
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = SSL_write(ssl, psTxdData+iRelSendlen, (uiDataLen-iRelSendlen));
		}
		else 
		{
			iRet = NetSend(sg_Tcpsocket,psTxdData+iRelSendlen,(uiDataLen-iRelSendlen),0);
		}
		
		if (iRet < 0)
		{
			TcpOnHook(TRUE);
			iRet = -iRet;
			return ERR_COMM_TCPIP_BASE|iRet;
		}
		iRelSendlen += iRet;
	}
	
	return 0;
}


// receive data
int TcpRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int  iRet,iRetryCnt;
	int  iOldTime,iNowTime;
	uint32_t  ulRealRecvLen = 0;
	unsigned long  ulTemp;
	struct timeval stTimeVal;
	socklen_t      Socklen;
	
	stTimeVal.tv_sec = 1;
	stTimeVal.tv_usec = 0;
	Socklen = sizeof(stTimeVal);
	NetSetsockopt(sg_Tcpsocket,SOL_SOCKET,SO_RCVTIMEO,&stTimeVal,Socklen);

	iRetryCnt = 0;
	iOldTime = sysGetTimerCount();
	while( 1 )
	{
		iNowTime= sysGetTimerCount();
		if( (iNowTime-iOldTime) >= (uint32_t)(uiTimeOut *1000) )
		{
			iRet = ERR_COMM_TIMEOUT;
			TcpOnHook(TRUE);
			break;
		}

		lcdDisplay(260,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNowTime-iOldTime)/1000);
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = SSL_read(ssl, psRxdData+ulRealRecvLen,uiExpLen-ulRealRecvLen);
		}
		else
		{
			iRet = NetRecv(sg_Wiresocket,psRxdData+ulRealRecvLen,uiExpLen-ulRealRecvLen,0);
		}
		
		if (iRet > 0)
		{
			PubChar2Long(psRxdData, 2, &ulTemp);
			ulRealRecvLen += iRet;
			if (ulRealRecvLen < ulTemp+2)
			{
				continue;
			}

			*puiOutLen = ulRealRecvLen;
			iRet = 0;
			break;
		}
		else
		{
			iRetryCnt++;
			if (iRetryCnt>3)
			{
				if (iRet<0)
				{
					iRet = -iRet;
				}
				iRet = ERR_COMM_TCPIP_BASE|iRet;
			}
			
			sysDelayMs(20);
		}
	}

	return iRet;
}

#ifndef _POS_TYPE_8210
int TcpOnHook(uint8_t bReleaseAll)
{
	int  iRet;

	iRet = NetClose(sg_Tcpsocket);
	sg_Tcpsocket = -1;

	if (bReleaseAll)
	{
		iRet = PPPLogout(OPT_DEVETH);
	}

	if (iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = ERR_COMM_TCPIP_BASE|iRet;
	}

	return iRet;
}
#endif


//////////////////////////////////////////////////////////////////////////
// MODEM
//////////////////////////////////////////////////////////////////////////
#define MODEM_COMM_ERR(a)		(ERR_COMM_MODEM_BASE|(a))
int PSTNDial(uint8_t ucDialMode)
{
	int      iRet, iStatus, iDialTimes, iLastErr;
	UINT32   uiOldTime,uiNewTime;	

	if (ucDialMode == PREDIAL_MODE)
	{
		iDialTimes = 0;
		iStatus = MODEM_STATE_NOT_INIT;

		if (sg_PredialdisplayMsgFlag)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, " PREDIALING..."); 
			DrawRect(0, 17, 127, 63);
			lcdFlip();
		}
		
		uiOldTime = sysGetTimerCount();
		while (1)
		{
			if ( sysGetTimerCount() >= (uiOldTime+sg_stCurCfg.ulSendTimeOut) )
			{
				PSTNOnHook(TRUE);
				return MODEM_COMM_ERR(NET_ERR_RETRY);
			}

			if (iDialTimes>3) 
			{
				modem_get_last_errno(sg_iModemfd,&iLastErr);
				iLastErr = ABS(iLastErr);
				if (iLastErr == 0)
				{
					iLastErr = -MODEM_ERRNO_NO_CARRIER;
				}
				return MODEM_COMM_ERR(iLastErr);
			}
			
			iStatus = MODEM_STATE_NOT_INIT;
			iRet = PSTNGetStatus(&iStatus);
			if (iRet !=0 ) 
			{
				if (sg_iModemfd < 0 )
				{
					PSTNOnHook(TRUE);
					sg_iModemfd = PSTNOpenModem();
					if (sg_iModemfd < 0)
					{
						return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT); 
					}
					continue;
				}
				else
				{
					iRet = ABS(iRet);
					return MODEM_COMM_ERR(iRet);
				}
			}

			switch (iStatus)
			{
			case MODEM_STATE_NOT_INIT:
				PSTNOnHook(TRUE);
				sg_iModemfd = PSTNOpenModem();
				if (sg_iModemfd < 0)
				{
					return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT); 
				}
				break;

			case MODEM_STATE_NO_SET_MODE:
				iRet = modem_set_dial_parms(sg_iModemfd,&sg_stCurCfg.stPSTNPara.stDialPara);
				if (iRet !=0 )
				{
					return MODEM_COMM_ERR(ERR_COMM_MODEM_NOPARAM);
				}
				break;

			case MODEM_STATE_SYNC_MODE:  // setting
			case MODEM_STATE_ASYN_MODE:  //setting
				break;

			case MODEM_STATE_DISCONNECT:
				iDialTimes++;
				iRet = modem_get_last_errno(sg_iModemfd,&iLastErr);
				if (iLastErr != 0)
				{
					PSTNOnHook(FALSE);
				}

				iRet = modem_dialing(sg_iModemfd,(char *)sg_stCurCfg.stPSTNPara.szTxnTelNo);
				iRet = ABS(iRet);
				if (iRet != 0)
				{
					dial_stat.fails++;
					write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta");
					return MODEM_COMM_ERR(iRet);
				}
				else
				{
					dial_stat.dials++;
					write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta");

					return 0;
				}
				break;

			case MODEM_STATE_WAITTING:
				PSTNOnHook(FALSE);
				break;

			default:
				return 0;
			}
		}

		return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT);  
	}

	iDialTimes = 0;
	uiOldTime = sysGetTimerCount();
	while (1)
	{
		uiNewTime = sysGetTimerCount();	
		if( (uiNewTime-uiOldTime) >= sg_stCurCfg.ulSendTimeOut)
		{
			PSTNOnHook(TRUE);
			return MODEM_COMM_ERR(NET_ERR_RETRY);
		}

		if( PubChkKeyInput() )
		{
			if (kbGetKey() == KEY_CANCEL)
			{
				PSTNOnHook(FALSE);
				return ERR_USERCANCEL;
			}
		}

		if (iDialTimes > 3)
		{
			modem_get_last_errno(sg_iModemfd,&iLastErr);
			iLastErr = ABS(iLastErr);
			if (iLastErr == 0)
			{
				iLastErr = -MODEM_ERRNO_NO_CARRIER;
			}
			return MODEM_COMM_ERR(iLastErr);
		}

		lcdClrLine(2, 7);	
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY|DISP_CLRLINE, "  DIALING...(%d)", iDialTimes);
		DrawRect(0, 17, 127, 63);
		lcdFlip();

		iStatus = MODEM_STATE_NOT_INIT;
		iRet = PSTNGetStatus(&iStatus);
		if (iRet !=0 ) 
		{
			if (sg_iModemfd < 0)
			{
				PSTNOnHook(TRUE);
				sg_iModemfd = PSTNOpenModem();
				if (sg_iModemfd < 0)
				{
					return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT); 
				}
				continue;
			}
			else
			{
				iRet = ABS(iRet);
				return MODEM_COMM_ERR(iRet);
			}
		}	
		switch (iStatus)
		{
		case MODEM_STATE_NOT_INIT:
			PSTNOnHook(TRUE);
			sg_iModemfd = PSTNOpenModem();
			if (sg_iModemfd < 0)
			{
				return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT); 
			}
			break;

		case MODEM_STATE_NO_SET_MODE:
			iRet = modem_set_dial_parms(sg_iModemfd,&sg_stCurCfg.stPSTNPara.stDialPara);
			if (iRet !=0 )
			{
				return MODEM_COMM_ERR(ERR_COMM_MODEM_NOPARAM);
			}
			break;

		case MODEM_STATE_SYNC_MODE:  // setting
		case MODEM_STATE_ASYN_MODE:  // setting
		case MODEM_STATE_DAILING:
		case MODEM_STATE_CONNECT_SDLC:
			sysDelayMs(100);
			break;
			
		case MODEM_STATE_DISCONNECT:
			iDialTimes++;
			iRet = modem_dialing(sg_iModemfd, (char *)sg_stCurCfg.stPSTNPara.szTxnTelNo);
			if (iRet != 0)
			{
				dial_stat.fails++;
				write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta");
				iRet = ABS(iRet);
				return MODEM_COMM_ERR(iRet);
			}
			else
			{
				dial_stat.dials++;
				write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta");
			}
			break;
			
		case MODEM_STATE_WAITTING:
			PSTNOnHook(FALSE);
			break;
	
		case MODEM_STATE_CONNECT:
			return 0;

		default:
			iStatus = ABS(iStatus);
			return MODEM_COMM_ERR(iStatus);
		}
	}
	
	iRet = -MODEM_ERRNO_ERROR;
	return MODEM_COMM_ERR(iRet);
}

// Modem send data
int PSTNTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int  iRet,iRelSendlen;
		
	for (iRelSendlen=0; iRelSendlen<uiDataLen;)
	{
		iRet = modem_write_timeout(sg_iModemfd,psTxdData+iRelSendlen,(uiDataLen-iRelSendlen),uiTimeOut);
		if (iRet < 0)
		{
			PSTNOnHook(FALSE);
			iRet = ABS(iRet);
			return MODEM_COMM_ERR(iRet);
		}
		iRelSendlen += iRet;
	}

	return 0;
}

// Modem receive data
int PSTNRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int		iRet;
	
	switch( sg_stCurCfg.stPSTNPara.ucSendMode )
	{
	case MODEM_COMM_ASYNC:
		iRet = PSTNAsyncRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
		break;
		
	case MODEM_COMM_SYNC:
		iRet = PSTNSyncRxd(psRxdData, uiExpLen, uiTimeOut, puiOutLen);
		break;
		
	default:
		iRet = MODEM_COMM_ERR(ERR_COMM_INV_PARAM);
		break;
	}
	
	return iRet;
}


// Modem sync
int PSTNSyncRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int  iRet,  iRetryCnt;
	int  iOldTime,iNowTime;
	uint32_t  ulRealRecvLen = 0;
	int  iRecvFlag=0;

	iRet = MODEM_ERRNO_ERROR;
	iRetryCnt = 0;
	iOldTime = sysGetTimerCount();
	while( 1 )
	{
		iNowTime= sysGetTimerCount();
		if( (iNowTime-iOldTime) >= (uint32_t)(uiTimeOut *1000) )
		{
			iRet = ERR_COMM_TIMEOUT;
			break;
		}
		
		lcdDisplay(87,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNowTime-iOldTime)/1000);
		iRet = modem_read_timeout(sg_iModemfd,psRxdData+ulRealRecvLen,uiExpLen-ulRealRecvLen,10);
		if (iRet > 0)
		{
			ulRealRecvLen += iRet;
			iRecvFlag = 1;			
			*puiOutLen = ulRealRecvLen;
		}
		else if( iRet==0 )
		{
			if( iRecvFlag==0 )
			{
				iRetryCnt++;
				if (iRetryCnt>3)
				{
					iRet = ABS(iRet);
					iRet = MODEM_COMM_ERR(iRet);
				}
			}
			if( iRecvFlag==1 )
			{
				iRet = 0;
				break;
			}

		}
		else
		{
			if (iRet < 0)
			{
				iRet = PosixGeterrno();
				iRet = ABS(iRet);
				iRet = MODEM_COMM_ERR(iRet);
				return iRet;
			}
			
			sysDelayMs(100);
		}
	}
	
	if(iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEM_COMM_ERR(iRet);
	}
	return iRet;
}

// Modem async
int PSTNAsyncRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int  iRet,  iRetryCnt;
	int  iOldTime,iNowTime;
	uint32_t  ulRealRecvLen = 0;
	unsigned long  ulTemp = 0;
	
	iRet = MODEM_ERRNO_ERROR;
	iRetryCnt = 0;
	iOldTime = sysGetTimerCount();
	while( 1 )
	{
		iNowTime= sysGetTimerCount();
		if( (iNowTime-iOldTime) >= (uint32_t)(uiTimeOut *1000) )
		{
			iRet = ERR_COMM_TIMEOUT;
			break;
		}
		
		lcdDisplay(87,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNowTime-iOldTime)/1000);
		iRet = modem_read_timeout(sg_iModemfd,psRxdData+ulRealRecvLen,uiExpLen-ulRealRecvLen,1000);
		if (iRet > 0)
		{			
			PubBcd2Long(psRxdData, 2, &ulTemp);
			ulRealRecvLen += iRet;
			if (ulRealRecvLen < ulTemp+2)
			{
				continue;
			}
			
			*puiOutLen = ulRealRecvLen;
			iRet = 0;
			break;
		}
		else
		{			
			if (iRet < 0)
			{
				iRet = PosixGeterrno();
				iRet = ABS(iRet);
				iRet = MODEM_COMM_ERR(iRet);
				return iRet;
			}
			
			iRetryCnt++;
			if (iRetryCnt>3)
			{
				iRet = ABS(iRet);
				iRet = MODEM_COMM_ERR(iRet);
			}
			
			sysDelayMs(100);
		}
	}
	
	if(iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEM_COMM_ERR(iRet);
	}
	return iRet;
}

// Modem hang up
int PSTNOnHook(uint8_t bReleaseAll)
{
	int  iRet,iStatus;

	modem_wait_until_sent(sg_iModemfd);

	// Exclude repeatedly hung request
	iRet = modem_get_status(sg_iModemfd, &iStatus);
	if ( ! (iStatus == (0x80000000 | MODEM_STATE_CONNECT)) )
		iRet = modem_hangup(sg_iModemfd);

	if (bReleaseAll)
	{
		while(1)
		{
			iRet = modem_get_status(sg_iModemfd, &iStatus);
			if (iRet != 0)
				break;
			
			if (iStatus & 0x80000000)  // At present the highest - 1 : the operation has not yet completed, need to check again
			{
				sysDelayMs(100);
				continue;
			}

			break;
		}
		iRet = modem_close(sg_iModemfd);		
		sg_iModemfd = -1;
	}
	
	if(iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEM_COMM_ERR(iRet);
	}
	
	return iRet;
}


int  PSTNGetStatus(int *piStatus)
{
	int  iRet, iStatus;
	int  uiOldTime , iTimeOut;

	iTimeOut = sg_stCurCfg.stPSTNPara.stDialPara.dial_timeo;
	if (iTimeOut<=0)
	{
		iTimeOut = 30;
	}
	uiOldTime = sysGetTimerCount();
	while (1) 
	{
		if( PubChkKeyInput() )
		{
			if (kbGetKey() == KEY_CANCEL)
			{
				return ERR_USERCANCEL;
			}
		}

		if ( sysGetTimerCount() >= (uiOldTime+iTimeOut*1000) )
		{ 
			modem_get_last_errno(sg_iModemfd,&iStatus);
			iStatus = ABS(iStatus);	
			if (iStatus != 0)
			{
				return MODEM_COMM_ERR(iStatus);
			}
			else
			{
				return MODEM_COMM_ERR(ERR_COMM_STATUS_TIMEOUT);
			}			
		}

		iRet = modem_get_status(sg_iModemfd, &iStatus);
		if (iRet != 0)
			break;
		
		if (iStatus & 0x80000000)  // At present the highest - 1 : the operation has not yet completed, need to check again
		{
			sysDelayMs(100);
			continue;
		}
		
		*piStatus = iStatus & 0x7FFFFFFF;
		break;
	}
	
	if(iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEM_COMM_ERR(iRet);
	}

	return iRet;
}

int  PSTNInitModem(ModemDialParms_t *pstModemParam)
{
	int  iRet, iStatus;
	ModemDialParms_t  stOldModemParam;
	
	iStatus = MODEM_STATE_NOT_INIT;
	iRet = PSTNGetStatus(&iStatus);
	if (iRet !=0 ) 
	{
		if (sg_iModemfd < 0 )
		{
			PSTNOnHook(TRUE);
			sg_iModemfd = PSTNOpenModem(); 
			if (sg_iModemfd < 0)
			{
				return MODEM_COMM_ERR(ERR_COMM_MODEM_INIT); 
			}
		}
		else
		{
			iRet = ABS(iRet);
			return MODEM_COMM_ERR(iRet);
		}
	}

	iStatus = MODEM_STATE_NOT_INIT;
	iRet = PSTNGetStatus(&iStatus);
	if (iRet != 0 || iStatus == MODEM_STATE_NOT_INIT)
	{
		return MODEM_COMM_ERR(MODEM_STATE_NOT_INIT);
	}

	iRet = modem_get_dial_parms(sg_iModemfd, &stOldModemParam);
	if (iRet != 0 || memcmp(&stOldModemParam,pstModemParam,sizeof(ModemDialParms_t)) !=0 )
	{
		iRet = modem_set_dial_parms(sg_iModemfd,pstModemParam);
	}
	else
	{
		iRet = 0;
	}
	
	if(iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = MODEM_COMM_ERR(iRet);
	}

	return iRet;  
}


int  PSTNOpenModem(void)
{
	int   iFd;

	iFd = modem_open(MODEM_PATHNAME, O_RDWR); 
	if (iFd < 0)
	{
		iFd = modem_open(MODEM_PATHNAME1, O_RDWR); 
	}

	return iFd;
}
//////////////////////////////////////////////////////////////////////////
// GPRS/CDMA 
//////////////////////////////////////////////////////////////////////////
#define WIRELESS_ERR(a)		(ERR_COMM_WIRELESS_BASE|(a))
#ifndef _POS_TYPE_8210
int InitWirelessModule(ST_WIRELESS_INFO *pstParam)
{
	int   iRet,iCnt;

	lcdClrLine(2, 7);
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "INITING");
	lcdDisplay(0, 5, DISP_CFONT|DISP_MEDIACY, "PLS WAITING..");
	lcdFlip();
	
	sg_InitModulRet = sg_Wiresocket = -1;	
	iRet = WnetInit(20000);
	if( iRet!= NET_OK )
	{
		iRet = ABS(iRet);
		return WIRELESS_ERR(iRet);
	}

	iCnt = 2;  // To prevent the SIM card power on time the problem is not enough
	while(iCnt--)
	{
		// check PIN of SIM card
		iRet = WnetCheckSim();
		if(iRet == -NET_ERR_PIN)
		{
			iRet =  WnetInputSimPin((char *)pstParam->szSimPin);
			if( iRet != NET_OK )
			{
				iRet = ABS(iRet);
				return WIRELESS_ERR(iRet);
			}

			break;
		}
		else
		{
			if (iRet != NET_OK)
			{
				iRet = ABS(iRet);
				iRet = WIRELESS_ERR(iRet);
				sysDelayMs(1000);
				continue;
			}
			else
			{
				break;
			}
		}
	}
	if (iRet != 0)
	{
		iRet = ABS(iRet);
		return WIRELESS_ERR(iRet);
	}

	iRet = CheckWirelessAttached(30*1000);
	if (iRet != 0)
	{
		sg_InitModulRet = WIRELESS_ERR(ERR_COMM_NOT_ATTACHED);
		return sg_InitModulRet;
	}

	DispWirelessSignal();
	sg_InitModulRet = 0;
	return iRet;
}

int WirelessDial(uint8_t ucDialFlag)
{
	int iRet,iFlag,iRetyTimes,iDialTimes, iPPPRet;
	uint32_t   uiOldTime;
	int        iDevType = OPT_DEVWNET;
	static   int  iTimeoutTimes = 0;

	// 8210?ppp???????????????,????GPRS,CDMA
#ifdef _POS_TYPE_8210
	if (sg_stCurCfg.ucCommType == CT_CDMA)
		iDevType = PPP_DEV_CDMA;
	else
		iDevType = PPP_DEV_GPRS;
#endif

	kbFlush();
	if( ucDialFlag== PREDIAL_MODE )  
	{
		if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
		{
			if (sg_InitModulRet != WIRELESS_ERR(ERR_COMM_NOT_ATTACHED) && sg_InitModulRet != -2){
				return sg_InitModulRet;
			}
		}

		if (sg_PredialdisplayMsgFlag)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, " PREDIALING..."); 
			DrawRect(0, 17, 127, 63);
			lcdFlip();
		}

		uiOldTime = sysGetTimerCount();
		while (1)
		{
			iRet = PPPCheck(iDevType);
			if (iRet < 0)
			{	
				iPPPRet = iRet;
				if ( sysGetTimerCount() >= (uiOldTime+30*1000) )//30S
				{ 
					iTimeoutTimes++;
					if (iTimeoutTimes<3){
						WirelessClose(TRUE,0,1);
					}
					else{
						WirelessReset(&sg_stCurCfg.stWirlessPara); //reset module
						iTimeoutTimes =0;
					}			
					return WIRELESS_ERR(NET_ERR_RETRY);
				}
				
				if( PubChkKeyInput() )
				{
					if (kbGetKey() == KEY_CANCEL)
					{
						return ERR_USERCANCEL;
					}
				}		

				if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
				{
					if (sg_InitModulRet == WIRELESS_ERR(ERR_COMM_NOT_ATTACHED))
					{
						iRet = CheckWirelessAttached(30*1000);
						if (iRet!=0 && iRet !=WIRELESS_ERR(0))
						{
							iRet = WirelessReset(&sg_stCurCfg.stWirlessPara);
							if (iRet!=0 && iRet !=WIRELESS_ERR(0))
							{
								return iRet;
							}
						}
					}
					else
					{
						iRet = WirelessReset(&sg_stCurCfg.stWirlessPara);
						if (iRet!=0 && iRet !=WIRELESS_ERR(0))
						{
							return iRet;
						}
					}
				}
				sg_Wire_PreDial_Ret = 0;
				if (iPPPRet == -NET_ERR_LINKOPENING)
				{
					sysDelayMs(400);
					break;
				}

#ifdef _POS_TYPE_8210
				iRet = -ENETDOWN;  // 8210 ?pppcheck???0?PPP_LINK_BUILDING,?????,????
#endif
				if (iRet == -ETIMEDOUT || iRet == -ENETDOWN)    // iRet == -NET_ERR_UNKNOWN || iRet == -ENETDOWN)
				{   // ??????????
					if (iRet == -ETIMEDOUT){
						sg_InitModulRet = -1;
					}
					if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
					{
						iRet = WirelessReset(&sg_stCurCfg.stWirlessPara);
						if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
						{
							iRet = ABS(iRet);
							return WIRELESS_ERR(iRet);
						}
					}
				}

#ifdef _POS_TYPE_8110
        		while ((UINT32)(sysGetTimerCount() - sg_uiWirePPPLogoutTime) < 3000); // ??????3????????
#else
        		PPPLogout(iDevType);
        		sysDelayMs(3000);
#endif
        		iRet = PPPLogin(iDevType,(char *)sg_stCurCfg.stWirlessPara.szAPN,
				            	(char *)sg_stCurCfg.stWirlessPara.szUID,
								(char *)sg_stCurCfg.stWirlessPara.szPwd,stPosParam.iPPPAuth,60);
				sg_ucWirePPPFlag = 1;			
				iRet = ABS(iRet);
				sg_Wire_PreDial_Ret = iRet;
				sg_pppWaitFlag = 1; // PPP dial needs after delay
				break;
			}
			return WIRELESS_ERR(iRet);
		}
		return WIRELESS_ERR(iRet);
	}

	iDialTimes = 1;
	iFlag = 1;
	iRetyTimes = 1;
	uiOldTime = sysGetTimerCount();
	while (iFlag)
	{
		iRet = PPPCheck(iDevType);
		if (iRet < 0)
		{
			iPPPRet = iRet;
			if( PubChkKeyInput() )
			{
				if (kbGetKey() == KEY_CANCEL)
				{
					return ERR_USERCANCEL;
				}
			}

			if ( sysGetTimerCount() >= (uiOldTime+sg_stCurCfg.ulSendTimeOut) )
			{ 
				iTimeoutTimes++;
				if (iTimeoutTimes<3){
					WirelessClose(TRUE,0,1);
				}
				else{
					WirelessReset(&sg_stCurCfg.stWirlessPara);
					iTimeoutTimes =0;
				}			
				return WIRELESS_ERR(NET_ERR_RETRY);
			}

			if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
			{
				lcdClrLine(2, 7);
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "INITING");
				lcdDisplay(0, 5, DISP_CFONT|DISP_MEDIACY, "PLS WAITING..");
				lcdFlip();
				if (sg_InitModulRet == WIRELESS_ERR(ERR_COMM_NOT_ATTACHED))
				{
					iRet = CheckWirelessAttached(30*1000);  
					if (iRet != 0)
					{
						iRet = WirelessReset(&sg_stCurCfg.stWirlessPara);
						if (iRet!=0 && iRet !=WIRELESS_ERR(0))
						{
							return iRet;
						}
					}
				}
				else
				{
					iRet = WirelessReset(&sg_stCurCfg.stWirlessPara);
					if (iRet!=0 && iRet !=WIRELESS_ERR(0))
					{
						return iRet;
					}
				}				
			}
			
			if( sg_Wire_PreDial_Ret==NET_ERR_NOSIM || sg_Wire_PreDial_Ret==NET_ERR_SIMDESTROY)
			{
				WirelessClose(TRUE,0,1);
				return WIRELESS_ERR(sg_Wire_PreDial_Ret);
			}

			lcdClrLine(2, 7);	
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY|DISP_CLRLINE, "PPP CONNECT(%d)", iDialTimes); 
			DrawRect(0, 17, 127, 63);
			lcdFlip();

			if (iPPPRet== -NET_ERR_LINKOPENING)
			{
				sysDelayMs(400);
				continue;
			}
		
#ifdef _POS_TYPE_8210
			iRet = -ENETDOWN;  // 8210 ?pppcheck???0?PPP_LINK_BUILDING,?????,????
#endif
			CommOnHook(FALSE);// ?????
			if (iRet == -ETIMEDOUT || iRet == -ENETDOWN)    // iRet == -NET_ERR_UNKNOWN || iRet == -ENETDOWN)
			{   // ??????????
				if (iRet == -ETIMEDOUT){
					sg_InitModulRet = -1;
				}
				if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
				{
					lcdClrLine(2, 7);
					lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "INITING");
					lcdDisplay(0, 5, DISP_CFONT|DISP_MEDIACY, "PLS WAITING..");
					lcdFlip();
					sg_InitModulRet = -1;
					iRet = WirelessReset(&sg_stCurCfg.stWirlessPara);
					if (sg_InitModulRet!=0 && sg_InitModulRet !=WIRELESS_ERR(0))
					{
						iRet = ABS(iRet);
						return WIRELESS_ERR(iRet);
					}
				}
			}

#ifdef _POS_TYPE_8110
        	while ((UINT32)(sysGetTimerCount() - sg_uiWirePPPLogoutTime) < 3000); // ??????3????????
#else
        	PPPLogout(iDevType);
        	sysDelayMs(3000);
#endif

			iDialTimes++;
			iRet = PPPLogin(iDevType,(char *)sg_stCurCfg.stWirlessPara.szAPN,
				        (char *)sg_stCurCfg.stWirlessPara.szUID,
						(char *)sg_stCurCfg.stWirlessPara.szPwd,stPosParam.iPPPAuth,60);

			sg_ucWirePPPFlag = 1;	
			if (iRet<0)
			{
				iRet = -iRet;
				
				if (iDialTimes > 3)
				{
					WirelessReset(&sg_stCurCfg.stWirlessPara);
					return WIRELESS_ERR(NET_ERR_RETRY);
				}
			}
			sg_Wire_PreDial_Ret = iRet;
			sysDelayMs(1000);  //after  ppplogin delay 1s，otherwise pppcheck return error
			sg_pppWaitFlag = 1; // afetr ppp need to delay
			continue;
		}
		//iCheckTimes++;
		//if(iCheckTimes == 1) //????check??,?????PPP?????????
		//{
			//sysDelayMs(200); //??200ms
			//continue;
		//}
		iTimeoutTimes =0; //?????????0
#ifdef _POS_TYPE_8110
		// ?????????????0,1,2 . ????sg_Wiresocket???,??????(??)
		if(sg_Wiresocket<0 || sg_Wiresocket>3)  
			CommOnHook(FALSE);
#endif			
		if (sg_Wiresocket <= -1)
		{
			lcdClrLine(2,7);	
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"TCPIP CONNECT(%d)",iRetyTimes++); 
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			
			 if (sg_pppWaitFlag)
            {
                 sysDelayMs(2000);
                 sg_pppWaitFlag = 0;
            }
            
			//  tcp
			sg_Wiresocket = NetSocket(AF_INET,SOCK_STREAM,0);
			if (sg_Wiresocket < 0)
			{
				iRet = sg_Wiresocket;
				WirelessClose(FALSE,0,1);
				iRet = ABS(iRet);
				return WIRELESS_ERR(iRet);
			}

			iRet = SetSockAddr(&sg_stCurCfg);
			if (iRet != NET_OK)
			{
				iRet = ABS(iRet);
				WirelessClose(FALSE,0,1);
				return WIRELESS_ERR(iRet);
			}

			iRet = NetConnect(sg_Wiresocket,&sg_stWiresockaddr,sizeof(struct sockaddr));						
			if (NET_OK == iRet)
			{
				sysDelayMs(200);
				return iRet;
			}

			if (-NET_ERR_USER_CANCEL == iRet)
			{
				return ERR_USERCANCEL;
			}	
		}
		else
		{ 
			return 0;
		}
		
		if (iRetyTimes > 3 )
		{
			WirelessClose(TRUE,0,1);
			return WIRELESS_ERR(NET_ERR_RETRY);
		}
		else
		{
			CommOnHook(FALSE);
			iRet = ABS(iRet);
			iRet = WIRELESS_ERR(iRet);
			continue;
		}
	}
	
	if (iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = WIRELESS_ERR(iRet);
	}
	return iRet;
}

int WirelessReset(ST_WIRELESS_INFO *pstParam)
{
    int   iRet;
	
	WirelessClose(TRUE,1,1);

#ifndef _POS_TYPE_8210
    iRet = WnetReset();
    if ( iRet!= NET_OK )
    {
      iRet = ABS(iRet);
      return WIRELESS_ERR(iRet);
    }
#endif
//	sysDelayMs(5000); // ???,??5s --????,???????
	
	iRet = InitWirelessModule(pstParam);
	return iRet;
}
#endif

//wireless receive data
int WirelessRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int  iRet,  iRetryCnt;
	int  iOldTime,iNowTime;
	uint32_t  ulRealRecvLen = 0;
	unsigned long  ulTemp = 0;
	struct timeval stTimeVal;
	socklen_t      Socklen;

	stTimeVal.tv_sec = 1;
	stTimeVal.tv_usec = 0;
	Socklen = sizeof(stTimeVal);
	NetSetsockopt(sg_Wiresocket,SOL_SOCKET,SO_RCVTIMEO,&stTimeVal,Socklen);
	
	iRetryCnt = 0;
	iOldTime = sysGetTimerCount();
	while( 1 )
	{
		iNowTime= sysGetTimerCount();
		if( (iNowTime-iOldTime) >= (uint32_t)(uiTimeOut *1000) )
		{
			iRet = ERR_COMM_TIMEOUT;
			break;
		}
		
		lcdDisplay(260,4,DISP_CFONT|DISP_FLIP,"(%d)",(iNowTime-iOldTime)/1000);
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = SSL_read(ssl, psRxdData+ulRealRecvLen,uiExpLen-ulRealRecvLen);
		}
		else
		{
			iRet = NetRecv(sg_Wiresocket,psRxdData+ulRealRecvLen,uiExpLen-ulRealRecvLen,0);
		}
			
		if (iRet > 0)
		{
			PubChar2Long(psRxdData, 2, &ulTemp);
			ulRealRecvLen += iRet;			
			if (ulRealRecvLen < ulTemp+2)
			{
				continue;
			}
	
			*puiOutLen = ulRealRecvLen;
			iRet = 0;
			break;
		}
		else
		{
			iRetryCnt++;
			if (iRetryCnt>3)
			{
				iRet = ABS(iRet);
				iRet = WIRELESS_ERR(iRet);
			}

			sysDelayMs(20);
		}
	}

	return iRet;
}

// wireless send data
int WirelessTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	int  iRet,iRelSendlen;
	struct timeval stTimeVal;
	
	stTimeVal.tv_sec = 30;
	stTimeVal.tv_usec = 0;
	NetSetsockopt(sg_Wiresocket,SOL_SOCKET,SO_SNDTIMEO,&stTimeVal,sizeof(stTimeVal));

	for (iRelSendlen=0; iRelSendlen<uiDataLen;)
	{
		if(stPosParam.open_internet==PARAM_OPEN)
		{
			iRet = SSL_write(ssl, psTxdData+iRelSendlen, (uiDataLen-iRelSendlen));
		}
		else 
		{
			iRet = NetSend(sg_Wiresocket,psTxdData+iRelSendlen,(uiDataLen-iRelSendlen),0);
		}
			
		if (iRet < 0)
		{
			WirelessClose(TRUE,0,1);
			iRet = -iRet;
			return WIRELESS_ERR(iRet);
		}
		iRelSendlen += iRet;
	}

	return 0;
}

#ifndef _POS_TYPE_8210
int WirelessClose(uint8_t bRelease,uint8_t bPowerDown,uint8_t ucFlag)
{
	int  iRet = 0, iCnt = 0;
	int  iDevType = OPT_DEVWNET;
	
	// 8210?ppp???????????????,????GPRS,CDMA
#ifdef _POS_TYPE_8210
	wnet_moduleinfo_t info;
	wnet_getmodinfo(&info);
	if (info.Type == CDMA_MODULE_TYPE)
		iDevType = PPP_DEV_CDMA;
	else
		iDevType = PPP_DEV_GPRS;
#endif

	kbFlush();
	if(sg_Wiresocket != -1)
	{
		iRet = NetClose(sg_Wiresocket);
	}
	sg_Wiresocket = -1;

	if (bRelease && 0 != sg_ucWirePPPFlag)
	{
		for (iCnt = 0; iCnt < 2; iCnt++) // Can not be the user press the Cancel button to stop ppplogout,only 2 times
		{
			iRet = PPPLogout(iDevType);
			if (-NET_ERR_USER_CANCEL == iRet)
			{
				continue;
			}
			else
			{
				break;
			}
		}

#ifdef _POS_TYPE_8210
		if(info.Type == CDMA_MODULE_TYPE || bPowerDown==1)
		{
			wnet_power_down();  // 8210?????gsmMuxd?? CDMA???????,??????
			sg_InitModulRet = -2;  // ???????
		}
#endif
		PreDialTid = InitCommTid=-1;		
		if (NET_OK == iRet || -NET_ERR_LINKCLOSED == iRet)
		{
			if (NET_OK == iRet){
				sg_uiWirePPPLogoutTime = sysGetTimerCount();
			}		
			sg_ucWirePPPFlag = 0;
			iRet = NET_OK;  
		}		
	}

	if(iRet != 0)
	{
		iRet = ABS(iRet);
		iRet = WIRELESS_ERR(iRet);
	}
	return iRet;
}
#endif

void DispWirelessSignal(void)//8210 kernel display signal
{
#ifdef _POS_TYPE_8110
	int  iRet, iSignal;
	
	if( stPosParam.stTxnCommCfg.ucCommType!=CT_GPRS && stPosParam.stTxnCommCfg.ucCommType!=CT_CDMA )
	{
		return;
	}

	iRet = WnetSignal(&iSignal);
	if( (iRet != 0 ) || (iSignal == 99) )
	{
		lcdSetIcon(ICON_SIGNAL, CLOSEICON);
		return;
	}
	
    if((iSignal >= 1) && (iSignal <= 7))
    {
        iSignal = 1;
    }
    else if((iSignal >= 8) && (iSignal <= 13))
    {
        iSignal = 3;
    }
    else if((iSignal >= 14) && (iSignal <= 19))
    {
        iSignal = 4;
    }
    else if((iSignal >= 20) && (iSignal <= 25))
    {
        iSignal = 5;
    }
    else if((iSignal >= 26) && (iSignal <= 31))
    {
        iSignal = 6;
    }

	lcdSetIcon(ICON_SIGNAL, iSignal);
#endif
}

#ifndef _POS_TYPE_8210
//////////////////////////////////////////////////////////////////////////
// WIFI
//////////////////////////////////////////////////////////////////////////
int   WiFiDial(uint8_t ucDialMode)
{
	int iRet,iFlag,iRetyTimes;
	uint32_t   uiOldTime;
	
	if( ucDialMode==PREDIAL_MODE )
	{
		if (sg_PredialdisplayMsgFlag)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, " PREDIALING..."); 
			DrawRect(0, 17, 127, 63);
			lcdFlip();
		}
		
		iRet = InitWiFiModule(FALSE);
		return iRet;
	}
	
	iFlag = 1;
	iRetyTimes = 1;
	uiOldTime = sysGetTimerCount();	
	while (iFlag)
	{
		if ( sysGetTimerCount() >= (uiOldTime+sg_stCurCfg.ulSendTimeOut) )
		{ 
			TcpOnHook(FALSE);
			return (ERR_COMM_TCPIP_BASE|NET_ERR_RETRY);
		}
				
		if( PubChkKeyInput() )
		{
			if (kbGetKey() == KEY_CANCEL)
			{
				TcpOnHook(FALSE);
				return ERR_USERCANCEL;
			}
		}

		if (iRetyTimes > 3 )
		{
			TcpOnHook(FALSE);
			return (ERR_COMM_TCPIP_BASE|NET_ERR_RETRY);
		}

		iRet = GetWiFiStatus();
		if (iRet != 0)
		{
			return iRet;
		}

		if (sg_Tcpsocket < 0)
		{
			sg_Tcpsocket = NetSocket(AF_INET,SOCK_STREAM,0);
			if (sg_Tcpsocket < 0)
			{
				TcpOnHook(FALSE);
				iRet = sg_Tcpsocket;
				if (iRet<0)
				{
					iRet = -iRet;
				}
				return (ERR_COMM_TCPIP_BASE|iRet);
			}
			iRet = SetSockAddr(&sg_stCurCfg);
			if (iRet != NET_OK)
			{
				if (iRet<0)
				{
					iRet = -iRet;
				}
				TcpOnHook(TRUE);
				return ERR_COMM_TCPIP_BASE|iRet;
			}
			
			lcdClrLine(2,7);	
		    	lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"TCPIP CONNECT(%d)",iRetyTimes++);
			DrawRect(0, 17, 127, 63);
			lcdFlip();
			
			iRet = NetConnect(sg_Tcpsocket,&sg_stTcpsockaddr,sizeof(struct sockaddr));
			if (iRet != NET_OK)
			{
				CommOnHook(FALSE);
				if(iRet<0)
				{
					iRet = -iRet;
				}
				iRet = ERR_COMM_TCPIP_BASE|iRet;
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			return 0;
		}
	}
	
	return iRet;
}
#endif

uint8_t CalcLRC(uint8_t *psData, uint32_t uiLength, uint8_t ucInit)
{
	while( uiLength>0 )
	{
		ucInit ^= *psData++;
		uiLength--;
	}

	return ucInit;
}


void DispCommErrMsg(int iErrCode)
{
	COMM_ERR_MSG	stCommErrMsg;

	if(iErrCode == NO_DISP)
		return;
	memset((char*)&stCommErrMsg,0,sizeof(stCommErrMsg));
	CommGetErrMsg(iErrCode, &stCommErrMsg);
	lcdClrLine(2,7);	
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, (char *)stCommErrMsg.szEngMsg);
	DrawRect(0, 17, 127, 63);
	lcdFlip();
	sysBeef(1, 200);
	kbGetKeyMs(5000);
}

void DispDialErrMsg(int iErrCode)
{
	COMM_ERR_MSG	stCommErrMsg;
	
	memset((char*)&stCommErrMsg,0,sizeof(stCommErrMsg));
	CommGetErrMsg(iErrCode, &stCommErrMsg);
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, (char *)stCommErrMsg.szEngMsg);
}

int CommOnHookMode(uint8_t ucCommType )
{
	int iRet;

	switch( ucCommType )
	{
	case CT_MODEMPPP:
		iRet = ModemPPPClose(TRUE);  //*****MDEMPPP********** 
		break;

	case CT_RS232:
		iRet = RS232OnHook(TRUE);
		break;

	case CT_MODEM:
		iRet = PSTNOnHook(TRUE);
		break;

	case CT_TCPIP:
	case CT_WIFI:
		iRet = TcpOnHook(TRUE);
		break;

	case CT_CDMA:
	case CT_GPRS:
		iRet = WirelessClose(TRUE,1,1);
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
		break;
	}

	InitCommTid = -1;
	return iRet;
}

void    SetPreDialDisplayFlag(int idisplayflag)
{
	sg_PredialdisplayMsgFlag = idisplayflag;
}

int     CheckPreDialDisplayFlag(void)
{
	return sg_PredialdisplayMsgFlag;
}

static void PosRand(unsigned char *buff, int num)
{	
	unsigned char temp[8];
	int i, cnt;		
	
	for (i = 0; i < num;) 	
	{		
		cnt = num >= 8 ? 8 : num;		
		memcpy(&buff[i], temp, cnt);		
		i += cnt;	
	}
}

static void rand_device_init(void)
{	
	unsigned char randbuf[32];	
	while(! RAND_status()) 	
	{		
		PosRand(randbuf, sizeof(randbuf));		
		RAND_add(randbuf, sizeof(randbuf), sizeof(randbuf));	
	}
}

void ssl_initialize(void)
{
	rand_device_init();
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	
	bio_err = BIO_new_fp(stderr,BIO_NOCLOSE);
	bio_out = BIO_new_fp(stdout,BIO_NOCLOSE);
}

static int verify_callback(int ok, X509_STORE_CTX *ctx)
{
	char buf[256];
	X509 *err_cert;
	int err,depth;

	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err      = X509_STORE_CTX_get_error(ctx);
	depth    = X509_STORE_CTX_get_error_depth(ctx);

	X509_NAME_oneline(X509_get_subject_name(err_cert),buf,sizeof buf);
	BIO_printf(bio_err, "depth=%d %s\n",depth,buf);
	if (!ok) {
		BIO_printf(bio_err,"verify error:num=%d:%s\n",err,X509_verify_cert_error_string(err));
		if (verify_depth >= depth) 	{
			ok=1;
			verify_error=X509_V_OK;
		}
		else {
			ok=0;
			verify_error=X509_V_ERR_CERT_CHAIN_TOO_LONG;
		}
	}
	switch (ctx->error) {
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
		X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),buf,sizeof buf);
		BIO_printf(bio_err,"issuer= %s\n",buf);
		break;
	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		BIO_printf(bio_err,"notBefore=");
		ASN1_TIME_print(bio_err,X509_get_notBefore(ctx->current_cert));
		BIO_printf(bio_err,"\n");
		break;
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		BIO_printf(bio_err,"notAfter=");
		ASN1_TIME_print(bio_err,X509_get_notAfter(ctx->current_cert));
		BIO_printf(bio_err,"\n");
		break;
	default:
		break;
	}
	
	BIO_printf(bio_err,"verify return:%d\n",ok);
	return(ok);
}

static SSL_METHOD *load_ssl_method(void)
{
	SSL_METHOD *method;
	
	switch (theSSLMethod) {
	case METHOD_SSLV23:
		method = (SSL_METHOD *)SSLv23_method();
		break;
	case METHOD_SSLV2:
		method = (SSL_METHOD *)SSLv2_method();
		break;
	case METHOD_SSLV3:
		method = (SSL_METHOD *)SSLv3_method();
		break;
	case METHOD_TLSV1:
		method = (SSL_METHOD *)TLSv1_method();
		break;
	case METHOD_TLSV1_2:
		method =  (SSL_METHOD *)TLSv1_2_method();
		break;
	default:
		method = NULL;
		break;
	}
	
	return method;
}

void ssl_print_error(void)
{
	ulong err;
	const char  *file;
	char err_str[128]={0};
	int   line;
	err = ERR_get_error_line(&file, &line);
	sprintf(err_str, "%s:%d:%s\n", file, line, ERR_error_string(err, NULL));	
}

static int SSLConnectHost(int *CurSocket)
{
	SSL_CTX		*ctx=NULL;
	SSL_METHOD	*meth=NULL;
	X509		*server_cert;
	char		*str;
	int         iRet;
	int         iOldTimeOut = 0;
	
	if(gbSslCreate==FALSE)
	{
		meth = load_ssl_method();
		if(!meth)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 2, DISP_CFONT, "LOAD SSL FAIL");
			PubWaitKey(3);
			return NO_DISP;
		}
		ctx  = SSL_CTX_new(meth);
		if (!ctx) 
		{
			sslDisconnectHost(TRUE);
			lcdClrLine(2, 7);
			lcdDisplay(0, 2, DISP_CFONT, "CREATE CTX FAIL");
			PubWaitKey(3);
			return NO_DISP;
		}
		
		if( stPosParam.ucSecondIpFlag == 1 )
		{
			if (fileExist(theCAfile)>=0) 
			{
				SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
				iRet = SSL_CTX_load_verify_locations(ctx, theCAfile, NULL);
				if(iRet!=1)
				{
					sslDisconnectHost(TRUE);
					lcdClrLine(2, 7);
					lcdDisplay(0, 2, DISP_CFONT, "LOAD CA FAIL:%d",iRet);
					PubWaitKey(3);	
					return NO_DISP;
				}
			}
		}
		else
		{
			if (fileExist(theCAfile2)>=0) 
			{
				SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
				iRet = SSL_CTX_load_verify_locations(ctx, theCAfile2, NULL);
				if(iRet!=1)
				{
					sslDisconnectHost(TRUE);
					lcdClrLine(2, 7);
					lcdDisplay(0, 2, DISP_CFONT, "LOAD CA FAIL:%d",iRet);
					PubWaitKey(3);	
					return NO_DISP;
				}
			}
		}
		
		gbSslCreate = TRUE;
	}

	ssl = SSL_new(ctx);
	if(ssl == NULL)
	{	
		sslDisconnectHost(TRUE);
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "CREATE SSL FAIL");
		PubWaitKey(3);	
		return NO_DISP;
	}

	SSL_set_fd(ssl, *CurSocket);
	iOldTimeOut = sysGetTimerCount();
	while(1)
	{	
		lcdClrLine(2,7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_CLRLINE|DISP_MEDIACY, NULL, "PROCESANDO...");
		lcdFlip();
		iRet = SSL_connect(ssl);
		lcdClrLine(2,7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_CLRLINE|DISP_MEDIACY, NULL, "PROCESANDO...");
		lcdFlip();
		if(iRet < 0)
		{	
			if (SSL_get_error(ssl, -1) == SSL_ERROR_WANT_READ)
			{
//				ssl_print_error();
			    if (sysGetTimerCount()-iOldTimeOut < 30*1000)
			    {
					sysDelayMs(10);
					continue;
			    }
				else{
					return -1;
				}
			}
			else{
//				ssl_print_error();
				return -1;
			}
		}
		break;
	}

	if (iRet < 0) 
	{
		sslDisconnectHost(TRUE);
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "SSL CONNECT FAIL");
		PubWaitKey(3);	
		return NO_DISP;
	}

	server_cert = SSL_get_peer_certificate(ssl);
	if(server_cert==NULL)
	{
		sslDisconnectHost(TRUE);
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "NO CA");
		PubWaitKey(3);	
		return NO_DISP;
	}
	else
	{
		str = X509_NAME_oneline(X509_get_subject_name(server_cert),0,0);
		OPENSSL_free(str);
		str = X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0);
		OPENSSL_free(str);
		X509_free(server_cert);
		return 0;
	}
}

static int sslDisconnectHost(int bShutDownSSL)
{
	if(ssl)
	{
		SSL_shutdown(ssl);	/* send SSL/TLS close_notify */
	}

	if(bShutDownSSL)
	{
		if (ssl)
		{ 
			SSL_free(ssl);
			ssl=NULL;
		}
		if (ctx)
		{
			SSL_CTX_free(ctx);
			ctx = NULL;
		}

		gbSslCreate = FALSE;
	}
	
	return OK;
}

// end of file
