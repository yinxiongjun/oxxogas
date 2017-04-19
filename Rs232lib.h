/****************************************************************************
NAME
    Rs232.h - 

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    pyming     2010.12.29      - created
****************************************************************************/
#ifndef _RS232LIB_H
#define _RS232LIB_H

#define  STR_RS232_PARAM    "115200,8,n,1"

// for RS232 communication
#ifndef  STX
#define  STX             0x02
#define  ETX             0x03
#endif

#define  RS232_TX_PURGE    0x0001   
#define  RS232_RX_PURGE    0x0002    

#define  ERR_RS232_OK              0   
#define  ERR_RS232_FAILURE        -1  
#define  ERR_RS232_RX_DATA        -2  
#define  ERR_RS232_RX_TIMEOUT     -3  
#define  ERR_RS232_RX_LENGTH      -4  
#define  ERR_RS232_RX_CHECKSUM    -5 

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int  Rs232Open(int iportNum, char *pszPortPara);

int  Rs232Close(void);

int  Rs232CheckPurge(unsigned int uiFlag);

int  Rs232ResetPurge(unsigned int uiFlag);

int  MainPosRS232Txd(unsigned char*psTxdData, unsigned int uiDataLen, unsigned int uiTimeOut);

int  MainPosRS232Rxd(unsigned char *psRxdData, unsigned int uiExpLen, unsigned int uiTimeOut, unsigned int *puiOutLen);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _RS232_H */

