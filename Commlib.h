
/****************************************************************************
NAME
	commlib.h

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	pyming     2008.09.05      - created
****************************************************************************/

#ifndef _COMMLIB_H
#define _COMMLIB_H

#include "posapi.h"
#include "modem_iface.h"
#include "modem.h"

#define MASK_COMM_TYPE			0xFF00000		
#define MASK_ERR_CODE			0x00FFFFF		

#define ERR_COMM_MODEMPPP_BASE	0x2000000		// ModemPPP
#define ERR_COMM_RS232_BASE		0x100000		// RS232
#define ERR_COMM_MODEM_BASE		0x200000		// Modem
#define ERR_COMM_TCPIP_BASE		0x400000		// TCPIP
#define ERR_COMM_WIRELESS_BASE	0x800000		// GPRS/CDMA
#define ERR_COMM_WIFI_BASE      0x4000000		// WIFI

#define ERR_COMM_ALL_BASE		0x0000		
#define ERR_COMM_INV_PARAM		0x0001		
#define ERR_COMM_INV_TYPE		0x0002		
#define ERR_COMM_CANCEL			0x0003	
#define ERR_COMM_TIMEOUT		0x0004		
#define ERR_COMM_COMERR			0x0005
#define	ERR_COMM_TOOBIG			0x0006
#define ERR_COMM_STATUS_TIMEOUT 0x0007     
#define ERR_COMM_MODEM_INIT     0x0008     
#define ERR_COMM_MODEM_NOPARAM  0x0009      
#define ERR_COMM_NOT_ATTACHED   0x000A     

// dial method
#define PREDIAL_MODE		    0		
#define ACTDIAL_MODE			1	
#define PPP_ONLY_MODE		2		//只连PPP，连上才返回


#define CM_RAW				0
#define CM_SYNC				1	
#define CM_ASYNC			2	

// for RS232 communication
#define STX             0x02
#define ETX             0x03
#define ENQ             0x05
#define ACK             0x06
#define NAK             0x15

// PSTN config 
typedef struct _tagST_PSTN_PARA
{
	uint8_t		        ucSendMode;		    
	uint8_t             szTxnTelNo[100+1];   
	ModemDialParms_t    stDialPara;          
}ST_PSTN_PARA; 

// RS232 config 
typedef struct _tagRS232_INFO
{
	uint8_t	ucPortNo;			// Port #, COM1, COM2 ....
	uint8_t	ucSendMode;			
	uint8_t	szAttr[20+1];		// "9600,8,n,1", ....
}ST_RS232_INFO;

// TCP/IP config
typedef struct _tagST_TCPIP_INFO
{
	uint8_t	szNetMask[15+1];
	uint8_t	szGatewayIP[15+1];
	uint8_t	szLocalIP[15+1];
	uint8_t	szRemoteIP[15+1];
	uint8_t	szRemotePort[5+1];
	uint8_t	szDNS[32+1];
}ST_TCPIP_INFO;


// GPRS/CDMA configurations
typedef struct _tagST_WIRELESS_INFO
{
	uint8_t		szAPN[64+1];  ///CDMA: #777; GPRS: cmnet
	uint8_t		szUID[64+1];
	uint8_t		szPwd[16+1];
	uint8_t		szSimPin[16+1];     // SIM card PIN
	uint8_t		szDNS[32+1];
	uint8_t		szRemoteIP[20+1];
	uint8_t		szRemotePort[5+1];
}ST_WIRELESS_INFO;

// WIFI  AP config
typedef struct _tagST_WIFI_AP_INFO
{
	int         iChannel;                  //  Wireless LAN communication channel(0 - 13)
	int         iSecurityType;             //  Sets the Wireless LAN security type
	int         iWEPKeyIdx;                //  WEP key index (1 - 4) 
    char        szListWEPKey[4][32];       //  4 sets of WEP Key
	char        szWPAPSK[64];              //  Personal Shared Key Pass-Phrase,
	char        szSSID[64];                //  SSID
}ST_WIFI_AP_INFO;

typedef struct _tagCOMM_ERR_MSG
{
	uint8_t		szChnMsg[16+1];
	uint8_t		szEngMsg[16+1];	
}COMM_ERR_MSG;


#define CT_RS232	0x01			
#define CT_MODEM	0x02			
#define CT_TCPIP	0x04			
#define	CT_CDMA		0x08			
#define CT_GPRS		0x10			
#define CT_WIFI     0x20            
#define CT_MODEMPPP 0x40
#define CT_ALLCOMM  0x7F           

// communicaton config
typedef struct _tagST_COMM_CONFIG
{
	uint8_t			  ucCommType;		    // (RS232/modem/tcp...)
    ulong             ulSendTimeOut;        // 
	ST_RS232_INFO	  stRS232Para;	        // RS232 
	ST_TCPIP_INFO	  stTcpIpPara;	        // TCP 
	ST_WIRELESS_INFO  stWirlessPara;        // GRPS/CDMA
	ST_PSTN_PARA	  stPSTNPara;		    // modem 
	ST_TCPIP_INFO	  stWIFIIpPara;	        // WIFI 
}ST_COMM_CONFIG;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int CommInitModule(ST_COMM_CONFIG *pstCfg);
int CommSetCfgParam(ST_COMM_CONFIG *pstCfg,uint8_t CheckWire);
int CommDial(uint8_t ucDialMode);
int CommTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
int CommRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
int CommOnHook(uint8_t bReleaseAll);
void CommGetErrMsg(int iErrCode, COMM_ERR_MSG *pstCommErrMsg);
int  GetCurSocket(void);
int   InitWiFiModule(uint8_t bInitModule);
int   GetWiFiStatus(void);
void DispCommErrMsg(int iErrCode);
int     WirelessClose(uint8_t bRelease,uint8_t bPowerDown,uint8_t ucFlag);
void    DispWirelessSignal(void);
int     CommOnHookMode(uint8_t ucCommType);
void    SetPreDialDisplayFlag(int idisplayflag);
int     CheckPreDialDisplayFlag(void);
void    DispDialErrMsg(int iErrCode);
int     PSTNOnHook(uint8_t bReleaseAll);
int     InitWirelessModule(ST_WIRELESS_INFO *pstParam);
int     WirelessDial(uint8_t ucDialFlag);
int     WirelessRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
int     WirelessTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
int  	TcpTxd(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut);
int  	TcpRxd(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen);
int  	TcpOnHook(uint8_t bReleaseAll);
int     WiFiDial(uint8_t ucDialMode);
int     TcpDial(uint8_t ucDialMode);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _CommLIB_H

// end of file
