/*******************************************************************************************
*  version: V1.0.0
*******************************************************************************************/

#include "stdarg.h"
#include "posapi.h"
#include "../glbvar.h"
#include "showDebug.h"

#define DEBUG_LOG "debug_msg.log"

#ifdef __DEBUG
/*************************************************************************************
   功能: 非常方便的调试语句输出函数
   参数:
    1. szFileName和uiLine是调用了本函数在工程里的位置, 分别是文件名和行号。这两个参数无需输入，
       因为采用了宏定义默认获取__FILE__和__LINE__。
    2. ulHexLen: 当所需要输出的数据是十六进制的时候，在这里传入数据长度，可传入0
    3. pHexHead: 当ulHexLen不为0时，这里传入十六进制数据的头指针。可传入NULL
    4. FmtBuf:   格式化输出字符串，类似于C语言的printf。

   本函数首头文件里的几个宏控制:
   //#define __DEBUG                     此宏打开后，调试函数生效
   //#define  _SendToComm                此宏打开后，调试信息通过串口输出
   //#define _ShowOnScreen               此宏打开后，调试信息在屏幕上显示
   //#define _Debug_Save_Log             此宏打开后，调试信息保存到pos机文件存储区，文件名是"debug_msg.log"
   //#define _Debug_Prn                  此宏打开后，调试信息被打印机打印出来

   只有当__DEBUG被打开后，后面几个宏才能发挥作用；后面几个信息输出方式的宏可同时打开

   限制:
        调试信息的总长度，包括十六进制和格式化输出的内容，不能大于_DEBUG_MSG_LEN-1。
        目前定义的_DEBUG_MSG_LEN 为1024*3。可根据实际需要修改。
        一定要注意的是，如果调试数据比_DEBUG_MSG_LEN-1大，那么将会内存溢出!
 **************************************************************************************/
void subShowDebugMessage(char *szFileName, uint32_t uiLine, uint32_t ulHexLen, const uint8_t *pHexHead, char *FmtBuf, ...)
{
#define _DEBUG_MSG_LEN 1024 * 3
	char szMessage[_DEBUG_MSG_LEN], sCurTime[7], szBuffer[_DEBUG_MSG_LEN];
	va_list argptr;
	int i;
	const uint8_t *pos;

	//get dateTime
	sysGetTime(sCurTime);
	sprintf((char *)szMessage, "%02X%02X/%02X/%02X %02X:%02X:%02X FILE:%s LINE:%u\r\n",
			0x20, sCurTime[0], sCurTime[1],
			sCurTime[2], sCurTime[3], sCurTime[4], sCurTime[5], szFileName, uiLine);

	//show string
	if (NULL != FmtBuf)
	{
		va_start(argptr, FmtBuf);
		vsprintf(szBuffer, FmtBuf, argptr);
#ifdef _ShowOnScreen
		lcdCls();
		lcdFlip();
		lcdPrintf("%s", szBuffer);
		kbGetKey();
#endif
		sprintf(szMessage + strlen(szMessage), "%s", szBuffer);
		va_end(argptr);
	}

	//show hex
	if (NULL != pHexHead && (ulHexLen < _DEBUG_MSG_LEN - 2 - strlen(szMessage)))
	{
		sprintf(szMessage + strlen(szMessage), "\r\nHex:\r\n");
		pos = pHexHead;

		memset(szBuffer, 0, sizeof(szBuffer));
		for (i = 0; i < ulHexLen; ++i)
		{
			sprintf(szBuffer + strlen(szBuffer), "%02x ", *(pos++));
		}

#ifdef _ShowOnScreen
		lcdCls();
		lcdFlip();
		lcdPrintf("%s", szBuffer);
		kbGetKey();
#endif
		strcat(szMessage + strlen(szMessage), szBuffer);
	}

	//the end
	strcat(szMessage, "\r\n\r\n");

#ifdef _Debug_Save_Log
    SaveToDebugLog(szMessage);
#endif

#ifdef _SendToComm
    // send msg to com1
#ifdef _POS_TYPE_8210
    printf("%s\n",szMessage);
#endif
    portClose(gstPosCapability.uiPortType);
    portOpen(gstPosCapability.uiPortType, (void *)"115200,8,n,1");
    i = portSends(gstPosCapability.uiPortType, (uint8_t *)szMessage, strlen(szMessage));
#endif

#ifdef _Debug_Prn
    prnInit();
    prnPrintf("%s", szMessage);
    PrintData();
#endif
}

#endif

void ListenComm(int iPort)
{
    int iRet;
    char szBuffer[1024];

    portClose(PORT_MODEM);
    portClose(gstPosCapability.uiPortType);
    iRet = portOpen(iPort, (void *)"9600,8,n,1");

    if (0 != iRet)
    {
        lcdCls();
        lcdPrintf("open port: %d fail", iPort);
        kbGetKey();
        sysReset();
    }
    else
    {
        portReset(iPort);
    }
    memset(szBuffer, 0, sizeof(szBuffer));
    lcdCls();
    lcdPrintf("listening...");

    while(1)
    {
        iRet = portRecv(iPort, (uint8_t *)szBuffer, 100);

        if (iRet == PORT_RET_OK)
        {
            lcdCls();
            lcdPrintf("Oh yeah! it comes!");
            kbGetKey();
            portReset(iPort);
            lcdCls();
            lcdPrintf("listening...");
        }
    }
}

void SaveToDebugLog(char *pszIn)
{
    INT32 iRet, iLogFile;

    iLogFile = fileOpen(DEBUG_LOG, O_RDWR | O_CREAT);

    if( iLogFile < 0 )
    {
        return;
    }
    iRet = fileSeek(iLogFile, 0, SEEK_END);

    if( iRet < 0 )
    {
        fileClose(iLogFile);
        return;
    }
    iRet = fileWrite(iLogFile, (UINT8 *)pszIn, strlen(pszIn));
    fileClose(iLogFile);

    if( iRet != strlen(pszIn) )
    {
        return;
    }
}


/*****************************************************************************
** Descriptions:	获取BitMap每一位值
** Parameters:          uint8_t *lpOut
                               uint8_t *lpIn
                               uint32_t nLen
** Returned value:
** Created By:		liangzhu  2013.12.12
** Remarks:
*****************************************************************************/
void BitmapToDomain(uint8_t *lpOut, uint8_t *lpIn, uint32_t nLen)
{
    uint32_t i, j, k;

    for(i = 0; i < nLen; i++)
    {
        for(j = 0; j < 8; j++)
        {
            k = i * 8 + j;
            lpOut[k] = 0;

            if(((lpIn[i] << j) & 0x80) == 0x80)
            {
                lpOut[k] = k + 1;
            }
        }
    }
}


/*****************************************************************************
** Descriptions:	打印串口信息
** Parameters:          
** Returned value:
** Created By:		liangzhu  2013.03.29
** Remarks:
*****************************************************************************/
void PrintMsg(uint8_t *szData, uint32_t iLen, uint8_t ucFiled, uint8_t ucFlag)
{
    uint8_t szMessage[4048];
    int i;

    portClose(gstPosCapability.uiPortType);
    portOpen(gstPosCapability.uiPortType, (void *)"115200,8,n,1");
	
	memset(szMessage, 0, sizeof(szMessage));
	if(ucFlag == 1)
	{
		sprintf((char *)szMessage, "\r\n域=[%d] 长度=[%d]\r\n", ucFiled, iLen);	
		sprintf((char *)szMessage + strlen((char *)szMessage), "================================================\r\n");
	}
	else if(ucFlag == 2)
	{
		sprintf((char *)szMessage, "\r\n[发送包] 长度=[%d]\r\n",  iLen);	
		sprintf((char *)szMessage + strlen((char *)szMessage), "================================================\r\n");
	}
	else if(ucFlag == 3)
	{
		sprintf((char *)szMessage, "\r\n[接收包] 长度=[%d]\r\n",  iLen);	
		sprintf((char *)szMessage + strlen((char *)szMessage), "================================================\r\n");
	}
	portSends(gstPosCapability.uiPortType, szMessage, strlen((char *)szMessage));
	#ifdef _POS_TYPE_8210
	printf("%s",szMessage);
	#endif
	memset(szMessage, 0, sizeof(szMessage));
    for(i = 0; i < iLen; i++)
    {
    	sprintf((char *)szMessage + strlen((char *)szMessage), "%02X ", *(szData++));
    	if(((i+1) < iLen)&& ((i + 1)%16) == 0)
    	{
    		sprintf((char *)szMessage + strlen((char *)szMessage), "\r\n");
    	}
    }
	sprintf((char *)szMessage + strlen((char *)szMessage), "\r\n");
    portSends(gstPosCapability.uiPortType, szMessage, strlen((char *)szMessage));    
    portClose(gstPosCapability.uiPortType);
#ifdef _POS_TYPE_8210
printf("%s",szMessage);
#endif
}
//end of file

