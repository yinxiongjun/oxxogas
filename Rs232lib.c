
#include "Rs232lib.h"
#include "glbvar.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
static int          sg_PortFd =  -1 ;
/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

int  Rs232Open(int iportNum, char *pszPortPara)
{
	int  iRet;

	iRet = portOpen(iportNum,pszPortPara);
	if (iRet == PORT_RET_OK)
	{
		sg_PortFd = iportNum;	
	}
	else
	{
		iRet = ERR_RS232_FAILURE;
	}

	return iRet;
}

int  Rs232Close(void)
{
	int  iRet;
	
	iRet = portClose(sg_PortFd);
	if (iRet != PORT_RET_OK)
	{
		iRet = ERR_RS232_FAILURE;
	}
	
	return iRet;
}

int  Rs232CheckPurge(unsigned int uiFlag)
{
	int  iRet = 0;
	
	if (uiFlag & RS232_TX_PURGE)
	{
		iRet = portCheckSendBuf(sg_PortFd);
	}

	if (uiFlag & RS232_RX_PURGE)
	{
		iRet = portCheckRecvBuf(sg_PortFd);
	}
	
	if (iRet != PORT_RET_NOTEMPTY)
	{
		iRet = ERR_RS232_FAILURE;
	}
	else
	{
		iRet = ERR_RS232_OK;
	}

	return iRet;
}

int  Rs232ResetPurge(unsigned int uiFlag)
{
	int  iRet;
	
	iRet = portReset(sg_PortFd);
	if (iRet != PORT_RET_OK)
	{
		iRet = ERR_RS232_FAILURE;
	}
	
	return iRet;
}


int  MainPosRS232Txd(unsigned char*psTxdData, unsigned int uiDataLen, unsigned int uiTimeOut)
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
			break;
		}
		
		iRet = portSends(sg_PortFd, psTxdData,uiDataLen);
		if( iRet == PORT_RET_OK )
		{
			break;
		}
		else
		{
			iRet = ERR_RS232_FAILURE;
		}
		sysDelayMs(50);
	}
	
	return iRet;
}

int  MainPosRS232Rxd(unsigned char *psRxdData, unsigned int uiExpLen, unsigned int uiTimeOut, unsigned int *puiOutLen)
{
	int             iRet;
	unsigned int 	uiReadCnt, uiLength;
	unsigned int    iOldTimeMs,iNewTimeMs;
	unsigned short  usCRCRet;
	
	uiReadCnt = uiLength = 0;
	iOldTimeMs = sysGetTimerCount();
	while( 1 )
	{
		iNewTimeMs= sysGetTimerCount();
		if( (iNewTimeMs-iOldTimeMs) >= (unsigned int)(uiTimeOut *1000) )
		{
			if( uiReadCnt>0 )
			{
				break;
			}			
			return ERR_RS232_RX_TIMEOUT;
		}		
		iRet = portCheckRecvBuf(sg_PortFd);
		if (iRet != PORT_RET_NOTEMPTY)
		{
			if (iRet != PORT_RET_OK )
			{				
				return ERR_RS232_RX_DATA;
			}			
			continue;
		}
		
		uiReadCnt = 0;
		iRet = portRecv(sg_PortFd, &psRxdData[uiReadCnt], 1000); 
		if( iRet!=0 )
		{
			if( iRet==0xFF )
			{
				continue;
			}
			return ERR_RS232_RX_DATA;
		}
		if( psRxdData[0]!=STX )
		{
			continue;
		}
		uiReadCnt++;

		for (; uiReadCnt < 5; uiReadCnt++)
		{
			iRet = portRecv(sg_PortFd, &psRxdData[uiReadCnt], 100);
			if( iRet!=0 )
			{
				return ERR_RS232_RX_DATA;
			}
		}

		// LEN
		uiLength = PubChar2Long(&psRxdData[uiReadCnt-2],2,NULL);
		if (uiLength > uiExpLen)
		{
			return ERR_RS232_RX_LENGTH;
		}

		uiLength += 5+2;
		for (; uiReadCnt < uiLength; uiReadCnt++)
		{
			iRet = portRecv(sg_PortFd, &psRxdData[uiReadCnt], 100);
			if( iRet!=0 )
			{
				return ERR_RS232_RX_DATA;
			}
		}

		usCRCRet = PubGetCRC((char *)(psRxdData+1),uiReadCnt-3);
		uiLength = PubChar2Long(psRxdData+uiReadCnt-2,2,NULL);
		if (usCRCRet != uiLength)
		{
			return ERR_RS232_RX_CHECKSUM;
		}
		
		break;
	}
	
	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiReadCnt;
	}
	
	return ERR_RS232_OK;
}  
// end of file
