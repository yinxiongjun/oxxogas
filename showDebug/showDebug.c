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
   ����: �ǳ�����ĵ�������������
   ����:
    1. szFileName��uiLine�ǵ����˱������ڹ������λ��, �ֱ����ļ������кš������������������룬
       ��Ϊ�����˺궨��Ĭ�ϻ�ȡ__FILE__��__LINE__��
    2. ulHexLen: ������Ҫ�����������ʮ�����Ƶ�ʱ�������ﴫ�����ݳ��ȣ��ɴ���0
    3. pHexHead: ��ulHexLen��Ϊ0ʱ�����ﴫ��ʮ���������ݵ�ͷָ�롣�ɴ���NULL
    4. FmtBuf:   ��ʽ������ַ�����������C���Ե�printf��

   ��������ͷ�ļ���ļ��������:
   //#define __DEBUG                     �˺�򿪺󣬵��Ժ�����Ч
   //#define  _SendToComm                �˺�򿪺󣬵�����Ϣͨ���������
   //#define _ShowOnScreen               �˺�򿪺󣬵�����Ϣ����Ļ����ʾ
   //#define _Debug_Save_Log             �˺�򿪺󣬵�����Ϣ���浽pos���ļ��洢�����ļ�����"debug_msg.log"
   //#define _Debug_Prn                  �˺�򿪺󣬵�����Ϣ����ӡ����ӡ����

   ֻ�е�__DEBUG���򿪺󣬺��漸������ܷ������ã����漸����Ϣ�����ʽ�ĺ��ͬʱ��

   ����:
        ������Ϣ���ܳ��ȣ�����ʮ�����ƺ͸�ʽ����������ݣ����ܴ���_DEBUG_MSG_LEN-1��
        Ŀǰ�����_DEBUG_MSG_LEN Ϊ1024*3���ɸ���ʵ����Ҫ�޸ġ�
        һ��Ҫע����ǣ�����������ݱ�_DEBUG_MSG_LEN-1����ô�����ڴ����!
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
** Descriptions:	��ȡBitMapÿһλֵ
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
** Descriptions:	��ӡ������Ϣ
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
		sprintf((char *)szMessage, "\r\n��=[%d] ����=[%d]\r\n", ucFiled, iLen);	
		sprintf((char *)szMessage + strlen((char *)szMessage), "================================================\r\n");
	}
	else if(ucFlag == 2)
	{
		sprintf((char *)szMessage, "\r\n[���Ͱ�] ����=[%d]\r\n",  iLen);	
		sprintf((char *)szMessage + strlen((char *)szMessage), "================================================\r\n");
	}
	else if(ucFlag == 3)
	{
		sprintf((char *)szMessage, "\r\n[���հ�] ����=[%d]\r\n",  iLen);	
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

