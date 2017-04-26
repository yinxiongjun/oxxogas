
#include <openssl/sha.h>
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "emvtrantest.h"
#include "emvapi.h"
#include "wireless.h"
#include "posparams.h"
#include "EMVCallBack.h"
#include "Menu.h"
#include "otherTxn.h"
#include "keydownload.h"

//TMS
#include "sapptms.h"
#include "libsbTms.h"

#define _ZYL_DEBUG_

int  EMVCoreInit(void);
void   GetPosCapablity(void);
extern int(*__ICCARD_EXCHANGE_APDU)(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdat);


const APPINFO AppInfo = {
	"CUP+EMV",
	"V1.0.0",
	"NEW POS",
	__DATE__,
	"CUP PBOC",
	0
};

typedef struct
{
	uint8_t Tag;
	uint8_t Len;
	char    *pVal;
	uint8_t State;
} PARA_TLV_t;

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
#define MAX_PARA_TLV 20

int giWriteFd, giReadFd;
PARA_TLV_t g_ParaList[MAX_PARA_TLV];
int        g_ParaCnt;

void DealManageComm(void)
{
	int i;

//��ȡͨѶ��Ϣ��tag����
	for(i = 0; i < g_ParaCnt; i++)
	{
		if(g_ParaList[i].Tag == 0xF0)
		{
			switch(g_ParaList[i].pVal[0])
			{
				case 'G':
					gucManageCommType = CT_GPRS;
					break;

				case 'M':
					gucManageCommType = CT_MODEM;
					break;

				case 'T':
					gucManageCommType = CT_TCPIP;
					break;

				case 'C':
					gucManageCommType = CT_CDMA;
					break;

				case 'W':
					gucManageCommType = CT_WIFI;
					break;

				case 'R':
					gucManageCommType = CT_RS232;
					break;

				default:
					gucManageCommType = CT_ALLCOMM;
					break;
			}
			giIfInitWirelessSuccess = g_ParaList[i].pVal[1] - 0x30;
		}
	}
}

static void ReadPubComm(void)
{
//��ȡ�����ļ������ͨѶ��ʽ
	ReadPubInfoFile();
	switch(stPubAppInfo.szCommType)
	{
		case 'G':
			stPosParam.stTxnCommCfg.ucCommType = CT_GPRS;
			break;

		case 'M':
			stPosParam.stTxnCommCfg.ucCommType = CT_MODEM;
			break;

		case 'T':
			stPosParam.stTxnCommCfg.ucCommType = CT_TCPIP;
			break;

		case 'C':
			stPosParam.stTxnCommCfg.ucCommType = CT_CDMA;
			break;

		case 'W':
			stPosParam.stTxnCommCfg.ucCommType = CT_WIFI;
			break;

		case 'R':
			stPosParam.stTxnCommCfg.ucCommType = CT_RS232;
			break;

		default:
			stPosParam.stTxnCommCfg.ucCommType = CT_ALLCOMM;
			break;
	}
	SaveAppParam();
}

void WritePubComm(void)
{
//ͨѶ��ʽд�������ļ�
	switch(stPosParam.stTxnCommCfg.ucCommType)
	{
		case CT_GPRS:
			stPubAppInfo.szCommType = 'G';
			break;

		case CT_MODEM:
			stPubAppInfo.szCommType = 'M';
			break;

		case CT_TCPIP:
			stPubAppInfo.szCommType = 'T';
			break;

		case CT_CDMA:
			stPubAppInfo.szCommType = 'C';
			break;

		case CT_WIFI:
			stPubAppInfo.szCommType = 'W';
			break;

		case CT_RS232:
			stPubAppInfo.szCommType = 'R';
			break;

		default:
			stPubAppInfo.szCommType = 'A';
			break;
	}
	stPubAppInfo.ucActivateFlag = 1;   //��Ҫ�󲹵�,����ֱ����1
	WritePubInfoFile();
}

static void DealDownParm(void)//��Ӧ�����õ�ͨѶ��ʽͬ��������
{
	int iRet;
	
	POS_PARAM_STRC_MAIN pSaveParam;

	iRet = Get_Ser_Comm_Param(&pSaveParam);
	strcpy((char*)pSaveParam.szPOSRemoteIP,(char*)gstManageTcpCfg.szRemoteIP);
	strcpy((char*)pSaveParam.szPOSRemoteWifiIp,(char*)gstManageTcpCfg.szRemoteIP);
	strcpy((char*)pSaveParam.szPOSRemoteGprsIp,(char*)gstManageTcpCfg.szRemoteIP);
	strcpy((char*)pSaveParam.szPOSRemoteCdmaIp,(char*)gstManageTcpCfg.szRemoteIP);	
	strcpy((char*)pSaveParam.szPOSRemotePPPIp,(char*)gstManageTcpCfg.szRemoteIP);
	
	strcpy((char*)pSaveParam.szPOSRemoteIP2,(char*)gstManageTcpCfg.szRemoteIP2);
	strcpy((char*)pSaveParam.szPOSRemoteWifiIp2,(char*)gstManageTcpCfg.szRemoteIP2);
	strcpy((char*)pSaveParam.szPOSRemoteGprsIp2,(char*)gstManageTcpCfg.szRemoteIP2);
	strcpy((char*)pSaveParam.szPOSRemoteCdmaIp2,(char*)gstManageTcpCfg.szRemoteIP2);
	
	strcpy((char*)pSaveParam.szPOSRemotePort,(char*)gstManageTcpCfg.szRemotePort);
	strcpy((char*)pSaveParam.szPOSRemoteWifiPort,(char*)gstManageTcpCfg.szRemotePort);
	strcpy((char*)pSaveParam.szPOSRemoteGprsPort,(char*)gstManageTcpCfg.szRemotePort);
	strcpy((char*)pSaveParam.szPOSRemoteCdmaPort,(char*)gstManageTcpCfg.szRemotePort);
	strcpy((char*)pSaveParam.szPOSRemotePPPPort,(char*)gstManageTcpCfg.szRemotePort);
	
	strcpy((char*)pSaveParam.szPOSRemotePort2,(char*)gstManageTcpCfg.szRemotePort2);
	strcpy((char*)pSaveParam.szPOSRemoteWifiPort2,(char*)gstManageTcpCfg.szRemotePort2);
	strcpy((char*)pSaveParam.szPOSRemoteGprsPort2,(char*)gstManageTcpCfg.szRemotePort2);
	strcpy((char*)pSaveParam.szPOSRemoteCdmaPort2,(char*)gstManageTcpCfg.szRemotePort2);
	
	strcpy((char*)pSaveParam.szPOSGprsApn,(char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN);
	strcpy((char*)pSaveParam.szPOSGprsUID,(char*)stPosParam.stTxnCommCfg.stWirlessPara.szUID);
	strcpy((char*)pSaveParam.szPOSGprsPwd,(char*)stPosParam.stTxnCommCfg.stWirlessPara.szPwd);
	strcpy((char*)pSaveParam.szPOSCdmaApn,(char*)stPosParam.stTxnCommCfg.stWirlessPara.szAPN);
	strcpy((char*)pSaveParam.szPOSCdmaUID,(char*)stPosParam.stTxnCommCfg.stWirlessPara.szUID);
	strcpy((char*)pSaveParam.szPOSCdmaPwd,(char*)stPosParam.stTxnCommCfg.stWirlessPara.szPwd);
	strcpy((char*)pSaveParam.PPPNetTel,(char*)gstManageTcpCfg.szPPPAPN);
	strcpy((char*)pSaveParam.PPPUser,(char*)gstManageTcpCfg.szPPPUID);
	strcpy((char*)pSaveParam.PPPPwd,(char*)gstManageTcpCfg.szPPPPwd);
	iRet = Set_Ser_Comm_Param(&pSaveParam);
}

int UnpackParaData(char *src_data, int src_len)
{
	int i=0;

	while(1)
	{
		memset(&g_ParaList[g_ParaCnt], 0, sizeof(PARA_TLV_t));

		if(i+4 > src_len)
			break;
		g_ParaList[g_ParaCnt].Tag = src_data[i++];	
		g_ParaList[g_ParaCnt].Len = PubAsc2Long(&src_data[i], 3, NULL);
		i += 3;
		g_ParaList[g_ParaCnt].pVal = &src_data[i];
		i += g_ParaList[g_ParaCnt].Len;
		
#ifdef _ZYL_DEBUG_
		printf("Tag:%02x    state:%d\n", g_ParaList[g_ParaCnt].Tag, g_ParaList[g_ParaCnt].State);
		printf("g_ParaList[%d].pVal:%s\n", g_ParaCnt, g_ParaList[g_ParaCnt].pVal);
		printf("i = %d,  src_len = %d\n", i, src_len);
#endif

        if(i > src_len)
        {
            break;
        }

        g_ParaCnt++;

        if(g_ParaCnt >= MAX_PARA_TLV)
        {
            break;
        }
    }

    return g_ParaCnt;
}
int CheckIfTAGNeedBack(void)
{
	int i, j;
	uint8_t ucNeedBackTagTable[] = 
	{
		'\xE0','\xE1','\xE2','\xEB','\xEC',
		'\xED','\xEE','\x00'
	};
	
	if(g_ParaCnt == 0)
	{
		return 0;
	}
	for(i = 0; i < g_ParaCnt; i++)
	{
		for(j = 0; ucNeedBackTagTable[j] != 0; j++)
		{
			if(g_ParaList[i].Tag == ucNeedBackTagTable[j])
			{
				return 1;
			}
		}
	}
	return 0;
}
/*?????????*/
void main_checkstatus_poweron(void)
{
	int i;

    i = 0;

    stTemp.ucPowerOnFlg = 0;
    
	for (i = 0; i < g_ParaCnt; i++) {
		if (g_ParaList[i].Tag == 0xF1) {
            stTemp.ucPowerOnFlg = 1;
		}
	}
}

int CheckFinlish(void)		//����Ƿ����������Ѵ���
{
    int i;

	if(g_ParaCnt == 0)
	{
		return 0;
	}

    for(i = 0; i < g_ParaCnt; i++)
    {
        if(g_ParaList[i].State == 0)            //???
        {
            return 0;
        }
    }

    return 1;
}
int ManageProcess(char *rsp, int *rsp_len)
{
	int i,len;
	int iTmpLen = 0;
	
	len = *rsp_len;
	rsp[len++] = 0xA5;
	for(i=0; i<g_ParaCnt; i++)
	{
#ifdef _ZYL_DEBUG_
		printf("===========================\n");
		printf("g_ParaList[%d].Tag: %02x\n", i, g_ParaList[i].Tag);
		printf("g_ParaList[%d].State: %d\n\n", i, g_ParaList[i].State);
#endif	
		if(g_ParaList[i].State != 0)	//�Ѵ���
			continue;
		
		switch(g_ParaList[i].Tag)
		{
		case 0xE0:	//��ѯ�ն˺š��̻��ź���Ӧ������
			rsp[len++] = 0xEC;			//�ն˺�
			memcpy(rsp+len, "008",3);
			len += 3;
			memcpy(rsp+len, stPosParam.szPosId,8);
			len += 8;
			
		#ifdef _ZYL_DEBUG_
			printf("=========�ն˺�(EC)======\n");
			printf("stPosParam.szPosId: %s\n\n", stPosParam.szPosId);
		#endif
			
			rsp[len++] = 0xEB;			//�̻���
			memcpy(rsp+len, "015",3);
			len += 3;
			memcpy(rsp+len, stPosParam.szUnitNum,15);
			len += 15;
		#ifdef _ZYL_DEBUG_
			printf("=========�̻���(EB)======\n");
			printf("stPosParam.szUnitNum: %s\n\n", stPosParam.szUnitNum);
		#endif
			rsp[len++] = 0xED;			//��Ӧ������ED
			iTmpLen = strlen((char *)AppInfo.AppName);
			sprintf(rsp+len, "%03d", iTmpLen);
//			memcpy(rsp+len, "020",3);
			len += 3;
			memcpy(rsp+len, AppInfo.AppName,iTmpLen);
			len += iTmpLen;
		#ifdef _ZYL_DEBUG_
			printf("=========��Ӧ������ED======\n");
			printf("AppInfo.AppName: %s\n\n", AppInfo.AppName);
		#endif

			SaveAppTmsParam(); 
			rsp[len++] = 0xEE;			//����·��EE
			printf("ParamPath = %s\n",ParamPath);
			iTmpLen = strlen(ParamPath);
			sprintf(rsp+len, "%03d", iTmpLen);
//			memcpy(rsp+len, "032",3);
			len += 3;
			memcpy(rsp+len, ParamPath,iTmpLen);
			len += iTmpLen;
		#ifdef _ZYL_DEBUG_
			printf("=========����·��EE======\n");
			printf("ParamPath: %s\n\n", ParamPath);
		#endif			
			g_ParaList[i].State=1;
		#ifdef _ZYL_DEBUG_
			printf("make tag '0xE0' OK!!!\n");
		#endif
			break;   
		case 0xE1:	//ѯ���Ƿ����
//			break;
		case 0xE2:	//�������ǩ
			rsp[len++] = 0xE2;
			memcpy(rsp+len, "001",3);
			len += 3;
			if(stTransCtrl.iTransNum == 0)
				rsp[len++] = '1';
			else
				rsp[len++] = '0';
			g_ParaList[i].State=1;
			break;
		case 0xE3:	//֪ͨ��Ӧ����Ӧ�ô�����
			break;
		case 0xE4:	//�´���������ʱ�� 14λ��ʼʱ��14λ����ʱ��
			break;
		case 0xE5:	//��Ӧ��֪ͨ���أ�posp֪ͨ����������ʱ�䡣Ԥ��
			break;
		case 0xE6:	//������Ӧ��Ҫ��ʾ��棬��Ӧ��ֻ��ȡǰ100���ֽ�
			break;
		case 0xE7:	//���ر�ǩ12000012�Զ�����������ʾʱ��
			break;
		case 0xE8:	//���ر�ǩ12000013�Զ�����������ʾ��Ϣ
			break;
		case 0xE9:	//���ر�ǩ12000015�ͷ��绰
			break;
		case 0xEA:	//���ر�ǩ12000029�����Զ�����������������Ϊ0����û������㲻�����Զ���������
			break;
		case 0xEB:	//�̻���
			rsp[len++] = 0xEB;
			memcpy(rsp+len, "015",3);
			len += 3;
			memcpy(rsp+len, stPosParam.szUnitNum,15);
			len += 15;
			g_ParaList[i].State=1;
			break;
		case 0xEC:	//???
			rsp[len++] = 0xEC;
			memcpy(rsp+len, "008",3);
			len += 3;
			memcpy(rsp+len, stPosParam.szPosId,8);
			len += 8;
			g_ParaList[i].State=1;
			break;
		case 0xED:	//?????
			rsp[len++] = 0xED;
			memcpy(rsp+len, "020",3);
			len += 3;
			memcpy(rsp+len, AppInfo.AppName,20);
			len += 20;
			g_ParaList[i].State=1;
			break;
		case 0xEE:	//??????????,?????????
			SaveAppTmsParam(); 
			g_ParaList[i].State=1;
			break;
		default:			//??????
			g_ParaList[i].State=1;
			break;
		}
	}
	*rsp_len = len;
	return CheckFinlish();
}

int check_img_version(char *version)	//zyl+ ��UMS�汾����0��UMS����1��ʧ�ܷ��ظ���
{
//	int fd;
	int ret;
	char buf[256];
	char *p;
	FILE *fp=NULL;
	
	*version = 0;
	
//	fd = fileOpen("/etc/image_version.txt", O_RDWR);
//	if( fd<0 )
	fp = fopen("/etc/image_version.txt", "r");
	if(fp == NULL)
	{
		return -1;
	}

	memset(buf,0,sizeof(buf));
//	ret = fileRead(fd, buf, sizeof(buf));
//	fileClose(fd);
	ret= fread(buf,1,sizeof(buf),fp);
	fclose(fp);
	if(ret < 0)
		return -2;

	p = strstr(buf, "Image");
	if(p == NULL)
	{
		p = strstr(buf, "image");
		if(p == NULL)
			return -3;
	}
	p += strlen("Image");

	while(*p)
	{
		if(*p == 'v' || *p == 'V')
			break;
		else
			p++;
	}
	if(*p != 'v' && *p != 'V')
		return -4;

	if(strstr(p,"ums") != NULL)
		ret = 1;
	else
		ret = 0;
	
	while(*p)
	{
		*version++ = *p++;
		if(*p == ' ')
		{
			*version = 0;
			break;
		}
	}
	return ret;
}
void SaveParamAndExit(int *pfd, int exit_manager)
{
    int iRspLen = 0;
    uint8_t szRspData[1024];


	if(stPosParam.stTxnCommCfg.ucCommType != CT_CDMA 
		&& stPosParam.stTxnCommCfg.ucCommType != CT_GPRS)
	{
		CommOnHook(TRUE);
	}
	SaveAppTmsParam();
	//����ͨѶ״̬������
	szRspData[iRspLen++] = 0xA5;
	szRspData[iRspLen++] = 0xF0;
	memcpy(&szRspData[iRspLen], "002", 3);
	iRspLen += 3;
	switch(stPosParam.stTxnCommCfg.ucCommType)
	{
		case CT_CDMA:
			szRspData[iRspLen++] = 'C';
			break;

		case CT_GPRS:
			szRspData[iRspLen++] = 'G';
			break;

		case CT_MODEM:
			szRspData[iRspLen++] = 'M';
			break;

		case CT_RS232:
			szRspData[iRspLen++] = 'R';
			break;

		case CT_TCPIP:
			szRspData[iRspLen++] = 'T';
			break;

		case CT_WIFI:
			szRspData[iRspLen++] = 'W';
			break;

		default:
			szRspData[iRspLen++] = 'A';
			break;
			
	}
	szRspData[iRspLen++] = giIfInitWirelessSuccess + 0x30;
	
	if(exit_manager)
	{
    	szRspData[iRspLen++] = 0xF1;        //�Ƿ���Ҫ��������?
    	memcpy(&szRspData[iRspLen], "001", 3);
    	iRspLen += 3;
    	szRspData[iRspLen++] = '1';
    }
	
	close(pfd[0]);
	write(pfd[1], szRspData, iRspLen);
	exit(0);
}

int main(int argc, char *argv[])
{
	uint8_t	ucRet;
	struct terminal_aid_info sTempAid;
	struct issuer_ca_public_key sTempCapk;
	int iCnt=0;
	int ret;
	char img_ver[100];
	
	int fd[2];
    	int iDataLen,iRet=0;
    	unsigned char *pszTLVPara;
    	int iRspLen=0;
    	uint8_t szRspData[1024];
	
	g_ParaCnt=0;
	memset(szRspData, 0, sizeof(szRspData));

	ret = BootInit(argc,argv);
	if (ret == 1)
	{
		exit(0);
	}

	if(argc >= 5)
	{
		if(argv[0] != NULL)       //�г��������ֶ� add andy.wang 2013-9-12 23:54:03
		{
			snprintf(s_gszAppName, sizeof(s_gszAppName), "%s", argv[0]);
		}
		iDataLen = strlen(argv[2]);	//���ݳ���
		pszTLVPara = (unsigned char*)argv[2];		//���ط�������������

		fd[0] = atoi(argv[3]);		//�ܵ�0,��
		fd[1] = atoi(argv[4]);		//�ܵ�1,д
		giReadFd = atoi(argv[3]);
		giWriteFd = atoi(argv[4]);
		if(argv[2][0] ==  '\xA5') 	//Э��ͷ
		{
			iRet = 1;
			UnpackParaData((char *)&pszTLVPara[1], iDataLen-1);
			iRet = CheckIfTAGNeedBack();  //����Ƿ���Tag��Ҫ��������
			if(iRet == 0)
			{
				DealManageComm();  //�������ص�ͨѶ add andy.wang 2014-4-10 14:18:55
			}
		}
	}

	GetPosCapablity();	//  ��ȡ�ն���������
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;

	stPubAppInfo.ucActivateFlag = 1;   //��Ҫ�󲹵�,����ֱ����1
		
	main_checkstatus_poweron();     //����Ƿ����ϵ�״̬
	InitPos(iRet);

	if(iRet){    //��Ҫ�������أ�һ���ǿ������һ�����У���ȡӦ����Ϣ
		ret = ManageProcess((char*)szRspData, &iRspLen);
		if(ret)					//�Ѵ�����
		{
			close(fd[0]);
			write(fd[1], szRspData, iRspLen);
    			exit(0);
    		}
	}

	PosDeviceDetect(); //zyl ���Ӳ��״̬����ӡ�����˺�����InitPos()���������

	if (gstPosCapability.uiScreenType)
		g_ums_img = check_img_version(img_ver);
	else
		g_ums_img = 0;

	if( fileExist("emv.aid")>=0 )
	{
		for(iCnt=0; iCnt<(fileSize("emv.aid")/sizeof(struct terminal_aid_info)); iCnt++)
		{
			emvLoadAIDList(iCnt, &sTempAid);
			ret = emv_add_one_aid_info(&sTempAid);
		}
	}
	if( fileExist("emv.capk")>=0 )
	{
		for(iCnt=0; iCnt<(fileSize("emv.capk")/sizeof(struct issuer_ca_public_key)); iCnt++)
		{
			emvLoadCAPKList(iCnt, &sTempCapk);
			emv_add_one_ca_public_key(&sTempCapk);
		}
	}
	get_tms_file(&tms_info);
	CheckPinPad();  // ���PINPAD�������������볬ʱʱ��60s
	if(stPosParam.open_internet == PARAM_OPEN)
	{
		ssl_initialize();
	}
	//stPosParam.stVolContrFlg ==PARAM_OPEN; //test
  	if (stPosParam.ucReprint!= 0)	//Printing is not complete
	{
		InitAllTransData();
	  	while(1)
	  	{
	  		lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	  		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "Reimpresion");
	  		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL,"Reimprimir?");
	  		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL,"Presione enter para continuar");
	  		DispMulLanguageString(0, 8, DISP_CFONT|DISP_MEDIACY, NULL,"Presione cancel para cancelar");
	  		lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	  		kbFlush();
	  		ucRet=kbGetKey();
	  		if(ucRet==KEY_ENTER)
	  		{
        		if (stPosParam.ucReprint==1)
          			reprintTrans(0);
        		else if(stPosParam.ucReprint==2)
        		{
          			ucRet = ReadLastTotal();
          			ucRet = PrtTransTotal(0, 0);
				PrintSettleBatch();
				AfterSettleTran();
        		}
        		else if(stPosParam.ucReprint==3)
        		{
				PrintSettleBatch();
				AfterSettleTran();
        		}

        		break;
	  		}
	  		else if(ucRet==KEY_CANCEL)
	  		{
				if(stPosParam.ucReprint!=1)
				{
					PrintSettleBatch();
					AfterSettleTran();
				}
	  			break;
	  		}
	  	}
	  	stPosParam.ucReprint=0;
		stPosParam.ucReCheckprint=0;
		SaveAppParam();
	  	strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
	  	SaveCtrlParam();
	}

	if( stTransCtrl.ucClearLog==TRUE )
	{
		ClearAllTranLog();
	}
	if( stTransCtrl.ucClearCheckLog==TRUE )
	{
		CleaCheckInTranLog();
	}
	read_uicc_mutecard();
	
	magClose();
	magOpen();
	
	while(1)
	{
		//init Global variables
		InitAllTransData();
		//check terminal status
		if( TermCheck() )
		{
			continue;
		}

		stTransCtrl.ucLogonFlag=LOGGEDON;
		DispMainScreen();
		ucRet = ProcEvent();
		if( ucRet==OK )
		{
			ucRet = ProcSelectTran();	// process transaction
			if( ucRet==E_NEED_FALLBACK )
			{
				CommOnHook(FALSE);
				lcdClrLine(2,7);
				DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "INSERT ERROR");
				DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL, "PLS SWIPE");
				lcdFlip();
				WaitRemoveICC();
				PosCom.ucFallBack    = TRUE;
				PosCom.bSkipEnterAmt = TRUE;
				PosCom.ucSwipedFlag  = NO_SWIPE_INSERT;
				ucRet = PosGetCard(CARD_SWIPED);
				if( ucRet==OK )
				{
					ucRet = ProcSelectTran();
				}
			}
			// for TIP fallback when 1st GAC return 6985
		}
		CommHangUp(FALSE);
		if(ucRet == E_APP_EXIT)
		{
			SaveParamAndExit(fd, 0);
		}
		else if(ucRet == E_APP_RESET)
		{
			SaveParamAndExit(fd, 1);
		}
		DispResult(ucRet);	// ��ʾ���׽�� 
	} // end while(1)   
}

void DispResult(uint8_t rej)
{
	uint8_t szDispBuf[30];

	//�������ױ�־��Ϊ0
	PosCom.stTrans.TransFlag = 0;

	if( rej<NO_DISP || rej==53 )
	{
		lcdClrLine(2, 7);
		ConvErrCode(rej, (char *)szDispBuf);
		
		if( rej == OK )
		{
			lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, (char *)szDispBuf);
		}
		else if( rej != E_TRANS_CANCEL )
		{
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, (char *)szDispBuf);
		}	
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		if( rej!=OK && rej!=E_TRANS_CANCEL)
		{
			sysBeef(3, 1000);
			kbGetKeyMs(2000);
		}
		else if(rej!=E_TRANS_CANCEL)
		{
			OkBeep();
			kbGetKeyMs(2000);
		}		
	}

	DispRemoveICC();
}

void InitAllTransData(void)
{
	char tmp[5] ={0};
	memset(&stTemp, 0, sizeof(stTemp));
	memcpy(tmp,PosComconTrol.AutoUpdateFlag,5);
	memset(&PosComconTrol,0,sizeof(PosComconTrol));
	memcpy(PosComconTrol.AutoUpdateFlag,tmp,5);

	stTemp.iTransNo   = NO_TRANS_SELECT;
	stTemp.bSendId    = TRUE;
	stTemp.ucPinEnterTime = 1;
	start_from_manager =0;
	memset(TitelName,0,sizeof(TitelName));
	
	InitCommData();
}

void InitCommData(void)
{
	uint8_t	buf[7];

	memset(&PosCom, 0, COM_SIZE);
	memcpy(PosCom.szUnitNum, stPosParam.szUnitNum, 15);
	PosCom.szUnitNum[15] = 0;
	//memcpy(PosCom.stTrans.szPosId, stPosParam.szPosId, 8);
	memcpy(PosCom.stTrans.szPosId, stPosParam.szMechId, 8);
	PosCom.stTrans.szPosId[8] = 0;

	//memcpy( stPosParam.szST,stPosParam.szPosId,4);//store id = first 4 num in termid
	memcpy( stPosParam.szST,stPosParam.szMechId,4);//store id = first 4 num in termid
	if( stPosParam.lNowTraceNo<=0 || stPosParam.lNowTraceNo>999999L )
	{
		stPosParam.lNowTraceNo = 1;
		sprintf((char*)buf, "%06d", stPosParam.lNowTraceNo);
	}
	if(stPosParam.lNowBatchNum<=0 || stPosParam.lNowBatchNum>999999L )
	{
		stPosParam.lNowBatchNum = 1;
		sprintf((char*)buf, "%06d", stPosParam.lNowBatchNum);
	}
	SaveAppParam();
	PosCom.stTrans.lTraceNo     = stPosParam.lNowTraceNo;
	PosCom.stTrans.lBatchNumber = stPosParam.lNowBatchNum;

	strcpy((char *)PosCom.stTrans.szTellerNo, (char *)stTransCtrl.szNowTellerNo);
	memset(stTemp.sPacketHead, 0, 7);

	PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
	PosCom.ucPinEntry   = FALSE;
	PosCom.bOnlineTxn   = FALSE;
	PosCom.bIsFirstGAC  = TRUE;
	PosCom.stTrans.ucFallBack = FALSE;
	PosCom.bSkipEnterAmt = FALSE;
}

#if 0
void  InitPos(int iRet)
{
	GetParamfileName();

	if (fileExist(gszParamFileName)<0)
	{
		if (fileExist("SysParam.log")<0)
		{
			lcdCls();
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "NO PARAM FILE");
			lcdDisplay(0, 5, DISP_CFONT|DISP_MEDIACY, "PLS PRESS '0'");
			lcdFlip();
			while(KEY0 != kbGetKey());
			lcdCls();
			LoadDefaultAppParam();
		}
		else
		{
			LoadAppParam();
		}
	}
	else
	{
		LoadDefaultAppParam();
		TxtToEnv();
		fileRemove(gszParamFileName);
	}
	
	
	if(stPosParam.iPPPAuth == 0)//wireless authentication mode
		stPosParam.iPPPAuth = PPP_ALG_PAP;
	if(strlen((char*)stPosParam.szDialNum)==0)
		strcpy((char*)stPosParam.szDialNum,"*99***1#");
	if(stPosParam.ucCommHostFlag!=PARAM_OPEN && stPosParam.ucCommHostFlag!=PARAM_CLOSE)
		stPosParam.ucCommHostFlag = PARAM_CLOSE;
	if(stPosParam.ucWnetMode!=PARAM_OPEN && stPosParam.ucWnetMode!=PARAM_CLOSE)
		stPosParam.ucWnetMode = PARAM_OPEN;
	if(stPosParam.ucPreAuthMaskPan!=PARAM_OPEN && stPosParam.ucPreAuthMaskPan!=PARAM_CLOSE)
		stPosParam.ucPreAuthMaskPan = PARAM_CLOSE;
	if(stPosParam.ucSalePassive!=PARAM_OPEN && stPosParam.ucSalePassive!=PARAM_CLOSE)
		stPosParam.ucSalePassive = PARAM_OPEN;
	SaveAppParam();

	fileMakeDir(STR_ELECSIGNATURE_FILE_PATH);
	if( fileExist("ctrl.log")<0 )
	{
		AppFirstRun();
	}
#ifdef _POS_TYPE_8210
	if (wifi_power_control_force)
	{
		if(stPosParam.stTxnCommCfg.ucCommType==CT_WIFI)
			wifi_power_control_force(1);
		else
			wifi_power_control_force(0);
	}
#endif
	CommInitModule(&stPosParam.stTxnCommCfg);

	EMVCoreInit();

	ReadCtrlParam();
	ReadOperFile();
	ReadLimitTotal();
	ReadTermAidList();
	kbFlush();

	if(stPosParam.ucReprint==0)
		strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
	
	LoadAppTmsFile();
	SaveCtrlParam();
}
#endif

void InitPos(int iRet)
{	
	uint8_t TmsParaTemp[2048*5];
	GetParamfileName();  // �ڴλ�ȡ�����ļ����ļ�����
	
	if (fileExist(gszParamFileName)<0)
	{
		if (fileExist("SysParam.log")<0)
		{
			// �����һ�����з���û�в����ļ�����ô��ʾһ�£����ڹ�����ͬ�·���
			lcdCls();
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "�޲����ļ�!");
			lcdDisplay(0, 5, DISP_CFONT|DISP_MEDIACY, "��[0]������");
			lcdFlip();	
			while(KEY0 != kbGetKey());
			lcdCls();
			LoadDefaultAppParam();
			DealDownParm();//��Ӧ�����õ�ͨѶ��ʽͬ��������
			WritePubComm();//ͨѶ��ʽ���浽�����ļ�
		}
		else
		{
			LoadAppParam();
		}
	}
	else
	{
		LoadDefaultAppParam();//�ȳ�ʼ������
		TxtToEnv();
		fileRemove(gszParamFileName);
		DealDownParm();//��Ӧ�����õ�ͨѶ��ʽͬ��������
		WritePubComm();//ͨѶ��ʽ���浽�����ļ�
	}
	if(stPosParam.iPPPAuth == 0)//������֤��ʽ
		stPosParam.iPPPAuth = PPP_ALG_PAP;
	if(strlen((char*)stPosParam.szDialNum)==0)
		strcpy((char*)stPosParam.szDialNum,"*99***1#");
	if(stPosParam.ucCommHostFlag!=PARAM_OPEN && stPosParam.ucCommHostFlag!=PARAM_CLOSE)
		stPosParam.ucCommHostFlag = PARAM_CLOSE;
	if(stPosParam.ucWnetMode!=PARAM_OPEN && stPosParam.ucWnetMode!=PARAM_CLOSE)
		stPosParam.ucWnetMode = PARAM_OPEN;
	if(stPosParam.ucPreAuthMaskPan!=PARAM_OPEN && stPosParam.ucPreAuthMaskPan!=PARAM_CLOSE)
		stPosParam.ucPreAuthMaskPan = PARAM_CLOSE;	
	if(stPosParam.ucSalePassive!=PARAM_OPEN && stPosParam.ucSalePassive!=PARAM_CLOSE)
		stPosParam.ucSalePassive = PARAM_OPEN;
	if(stPosParam.ucSignInputMobile!=PARAM_OPEN && stPosParam.ucSignInputMobile!=PARAM_CLOSE)
		stPosParam.ucSignInputMobile = PARAM_OPEN;

	stPosParam.ucCommDNS = PARAM_OPEN; //�����������������ÿ�ο�����һ�ʽ�������������
		
	if(stPosParam.ucContactnessMethod!=PARAM_OPEN && stPosParam.ucContactnessMethod!=PARAM_CLOSE)
		stPosParam.ucContactnessMethod = PARAM_OPEN;

    if(0 == stPosParam.iMuteRetryTimeouts)
		stPosParam.iMuteRetryTimeouts = 10;

    if(0 == stPosParam.iMuteLogTimeouts)
		stPosParam.iMuteLogTimeouts = 60;

    if(0 == stPosParam.iMaxMuteNum)
		stPosParam.iMaxMuteNum = 3;	

	SaveAppParam();

	ReadPubComm();//��ȡ��ǰͨѶ��ʽ���ӹ����ļ�����ȥȡ
	
	if(stPosParam.ucSupportSM == PARAM_OPEN)
		pboc_sm_enable(1); //֧�ֹ����㷨
	else
		pboc_sm_enable(0);

	fileMakeDir(STR_ELECSIGNATURE_FILE_PATH);  // �������ڱ������ǩ��ͼƬ��Ŀ¼,û�оʹ���
	if( fileExist("ctrl.log")<0 )  // ��һ������
	{
		AppFirstRun();
	}

	if(iRet == 0)
	{
		if((gstPosCapability.uiCommType & (CT_GPRS|CT_CDMA))&&
			(stPosParam.stTxnCommCfg.ucCommType!=CT_GPRS)&&(stPosParam.stTxnCommCfg.ucCommType!=CT_CDMA))
		{
			wnet_power_down();
		}
		
#ifdef _POS_TYPE_8210
		if (wifi_power_control_force)
		{
			if(stPosParam.stTxnCommCfg.ucCommType==CT_WIFI)
			{
				wifi_power_control_force(1);
				wifi_status = 1;
			}
			else
			{
				wifi_status = 0;
				wifi_power_control_force(0);
			}
		}
#endif

		CommInitModule(&stPosParam.stTxnCommCfg);
	}
	EMVCoreInit();
//	LoadQpbocParam();
	
	//ÿ�ο����󣬶��뱣��ı��� 
	ReadCtrlParam();
	LoadAppTmsFile();
	ReadOperFile();
	ReadLimitTotal();
	ReadTermAidList();

	memset(TmsParaTemp,0,sizeof(TmsParaTemp));
	ReadNetpay((char*)TmsParaTemp);
	DealTmsParam(TmsParaTemp);
	SaveAppParam();
	
//	kbSound(3, 80); 
	kbFlush();
	
//	PosDeviceDetect(); //���Ӳ��״̬����ӡ��
//TMS
	if(iRet == 0 && 1 == stTemp.ucPowerOnFlg)
	{
		PosDeviceDetect(); //���Ӳ��״̬����ӡ��
		if(stPosParam.ucReprint==0)//û�жϵ���ӡ���ʼ������Ա
		{
			strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
			SaveCtrlParam();
		}
	}
}


int EMVCoreInit(void)
{
	int iRet;

	emv_set_kernel_type(KERNEL_TYPE_PBOC);
	//emv_set_kernel_type(KERNEL_TYPE_EMV);


	emv_init_data_element();

	emv_get_mck_configure(&tConfig);
	memcpy(tConfig.terminal_capabilities, "\xE0\xF9\xC8", 3);
	memcpy(tConfig.additional_terminal_capabilities, "\xE0\x00\xF0\xA0\x01", 5);
	tConfig.terminal_type = 0x22;

	tConfig.support_forced_online_capability = 0;
	tConfig.support_get_pin_try_counter = 1;
	tConfig.support_pse_selection_method = 1;
	tConfig.support_cardholder_confirmation = 1;
	tConfig.support_floor_limit_checking = 1;
	tConfig.support_advices = 1;
	tConfig.support_batch_data_capture = 1;
	tConfig.support_bypass_pin_entry = 1;
	tConfig.support_default_ddol = 1;
	tConfig.support_forced_acceptance_capability = 0;
	tConfig.support_issuer_initiated_voice_referrals = 1;
	tConfig.support_multi_language = 1;
	tConfig.support_online_data_capture = 1;
	tConfig.support_random_transaction_selection = 1;
	tConfig.support_default_tdol = 1;
	tConfig.support_terminal_action_codes = 1;
	tConfig.support_transaction_log = 1;
	tConfig.support_trm_based_on_aip = 1;
	tConfig.support_velocity_checking = 1;

 	tEMVCoreInit.transaction_type = EMV_GOODS;

	memcpy(tEMVCoreInit.terminal_id, "19790414", 8);
    	memcpy(tEMVCoreInit.merchant_id,"123456789ABCDEF", 15);
    	memcpy(tEMVCoreInit.merchant_cate_code, "\x00\x01", 2);
    	tEMVCoreInit.merchant_name_loc_len = 28;
    	memcpy(tEMVCoreInit.merchant_name_loc, stPosParam.szEngName, 28);

	memcpy(tEMVCoreInit.terminal_country_code,   "\x04\x84", 2);
	memcpy(tEMVCoreInit.transacion_currency_code, "\x04\x84", 2);
	memcpy(tEMVCoreInit.refer_currency_code, "\x04\x84", 2);
	
	
    	tEMVCoreInit.transacion_currency_exponent = 0x02;
    	tEMVCoreInit.refer_currency_exponent = 0x02;
    	tEMVCoreInit.refer_currency_coefficient = 1000;

	SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);
	emv_set_mck_configure(&tConfig);

	emv_set_core_init_parameter(&tEMVCoreInit);

    	tEMVCallBackFunc.emv_candidate_apps_selection = emv_candidate_apps_selection;
    	tEMVCallBackFunc.emv_advice_process = emv_advice_process;
    	tEMVCallBackFunc.emv_check_certificate = emv_check_certificate;

	tEMVCallBackFunc.emv_get_pin = emv_get_pin;
	tEMVCallBackFunc.emv_return_pin_verify_result = emv_return_pin_verify_result;

    	tEMVCallBackFunc.emv_check_online_pin = emv_check_online_pin;

	tEMVCallBackFunc.emv_get_amount = emv_get_amount;

    	tEMVCallBackFunc.emv_online_transaction_process = emv_online_transaction_process;
    	tEMVCallBackFunc.emv_issuer_referral_process = emv_issuer_referral_process;
    	tEMVCallBackFunc.emv_multi_language_selection = emv_multi_language_selection;

	tEMVCallBackFunc.emv_check_revocation_certificate = emvCheckCRL;
	tEMVCallBackFunc.emv_check_exception_file = emvCheckExceptPAN;
	tEMVCallBackFunc.emv_get_transaction_log_amount = emvGetTransLogAmount;

	iRet = emv_init_callback_func(&tEMVCallBackFunc);
	if( iRet!=EMV_RET_SUCCESS )
	{
		return iRet;
	}

	emv_delete_all_terminal_aids();
	emv_delete_all_ca_public_keys();

	return EMV_RET_SUCCESS;
}

void AppFirstRun(void)
{
	int		i;

	memset(&stTransCtrl, 0, sizeof(struct _TRANS_CTRL_PARAM));

	stTransCtrl.ucPosStatus = WORK_STATUS;
	strcpy((char *)stTransCtrl.szLastTransDate, "99999999");
	stTransCtrl.ucClearLog = FALSE;
	stTransCtrl.ucClearCheckLog = FALSE;
	stTransCtrl.bEmvCapkLoaded = FALSE;
	stTransCtrl.bEmvAppLoaded  = FALSE;
	stTransCtrl.ucCurSamNo = -1;
	ClearAllTxnStatus();
	fileMakeDir(STR_ELECSIGNATURE_FILE_PATH);
	SaveCtrlParam();

	memset(&operStrc, 0, MAX_OPER_NUM*sizeof(struct _oper_structure));
	sprintf(operStrc[0].szTellerNo, SUP_TELLER_NO);
	sprintf(operStrc[0].szPassword, SUP_TELLER_PWD);

	for(i=1; i<6; i++)
	{
		sprintf(operStrc[i].szTellerNo, "%03d", i);
		sprintf(operStrc[i].szPassword, "0000");
	}

	for(i=6; i<MAX_OPER_NUM; i++)
	{
		sprintf(operStrc[i].szTellerNo, TELLER_NOBODY);
		sprintf(operStrc[i].szPassword, "0000");
	}
	WriteOperFile();

	dial_stat.dials = 0;
	dial_stat.fails = 0;
	dial_stat.hours = 0;
	dial_stat.suc_rate = 0;
	if( write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta")!=OK )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "FILE ERROR");
		lcdFlip();
		kbGetKey();
		return ;
	}
	if( write_stat_time()!=OK )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "FILE ERROR");
		lcdFlip();
		kbGetKey();
		return ;
	}

	memset(&stLimitTotal, 0, sizeof(struct _limit_total_strc));
	SaveLimitTotal();
	memset(&stTotal, 0, TOTAL_STRC_SIZE);
	strcpy((char *)stTotal.sDateTime, "00000000000000");
	SaveLastTotal();

	InitTestApps();
	InitTestKeys();
}


uint8_t CheckSupport(int iTransNo)
{
	switch( iTransNo )
	{
	case POS_QUE://1
		if( stPosParam.sTransOpen[0] & 0x80 )
		{
			return TRUE;
		}
		break;

	case POS_PREAUTH://2
		if( stPosParam.sTransOpen[0] & 0x40 )
		{
			return TRUE;
		}
		break;

	case POS_PREAUTH_VOID://3
		if( stPosParam.sTransOpen[0] & 0x20 )
		{
			return TRUE;
		}
		break;

	case POS_AUTH_CM://4
		if( stPosParam.sTransOpen[0] & 0x10 )
		{
			return TRUE;
		}
		break;

	case POS_AUTH_VOID://5
		if( stPosParam.sTransOpen[0] & 0x08 )
		{
			return TRUE;
		}
		break;

	case POS_SALE://6
		if( stPosParam.sTransOpen[0] & 0x04 )
		{
			return TRUE;
		}
		break;

	case POS_SALE_VOID://7
		if( stPosParam.sTransOpen[0] & 0x02 )
		{
			return TRUE;
		}
		break;

	case POS_REFUND://8
		if( stPosParam.sTransOpen[0] & 0x01 )
		{
			return TRUE;
		}
		break;

	case OFF_SALE://9
		if( stPosParam.sTransOpen[1] & 0x80 )
		{
			return TRUE;
		}
		break;

	case OFF_ADJUST://10
		if( stPosParam.sTransOpen[1] & 0x40 )
		{
			return TRUE;
		}
		break;

	case POS_OFF_CONFIRM:  //11
		if( stPosParam.sTransOpen[1] & 0x20 )
		{
			return TRUE;
		}
		break;

	case UPLOAD_SCRIPT_RESULT:  //12
		if( stPosParam.sTransOpen[1] & 0x10 )
		{
			return TRUE;
		}
		break;

	case EC_NORMAL_SALE://13
		if(( stPosParam.sTransOpen[1] & 0x08 )&&( stPosParam.sTransOpen[4] & 0x80 ))
		{
			return TRUE;
		}
		break;

	case EC_QUICK_SALE://13
		if(( stPosParam.sTransOpen[1] & 0x08 )&&( stPosParam.sTransOpen[4] & 0x40 ))//�������������ı�־�Լ��ֹ����õı�־��֧��
		{
			return TRUE;
		}
		break;

	case POS_PREAUTH_ADD: //14
		if( stPosParam.sTransOpen[1] & 0x04 )
		{
			return TRUE;
		}
		break;

	case POS_INSTALLMENT: //16
		if( stPosParam.sTransOpen[1] & 0x01 )
		{
			return TRUE;
		}
		break;

	case POS_VOID_INSTAL: //17
		if( stPosParam.sTransOpen[2] & 0x80 )
		{
			return TRUE;
		}
		break;

	case EC_TOPUP_SPEC: //20
		if(( stPosParam.sTransOpen[2] & 0x10 )&&( stPosParam.sTransOpen[3] & 0x40 ))
		{
			return TRUE;
		}
		break;

	case EC_TOPUP_NORAML: //20
		if(( stPosParam.sTransOpen[2] & 0x10 )&&( stPosParam.sTransOpen[3] & 0x20 ))
		{
			return TRUE;
		}
		break;

	case EC_TOPUP_CASH: //20
		if(( stPosParam.sTransOpen[2] & 0x10 )&&( stPosParam.sTransOpen[3] & 0x10 ))
		{
			return TRUE;
		}
		break;

	case EC_VOID_TOPUP: //20
		if(( stPosParam.sTransOpen[2] & 0x10 )&&( stPosParam.sTransOpen[3] & 0x08 ))
		{
			return TRUE;
		}
		break;

	case EC_REFUND:
		if( stPosParam.sTransOpen[3] & 0x04 )
		{
			return TRUE;
		}
		break;

	default:
		return TRUE;
	}

	return FALSE;
}

void DispMainScreen(void)
{
	if( DirectIntoMenu == 1)  //ֱ�ӽ���˵�����ʾ������
	{
		return;
	}
	lcdCls();
	if( stTransCtrl.ucLogonFlag==LOGGEDOFF )
	{
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " CHINA UNIONPAY ");
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "NOT LOGON");
		DispMulLanguageString(0, 4, DISP_CFONT, NULL, "PRESS ANY KEY");
		lcdFlip();
		kbFlush();
		kbGetKey();
	}
	else
	{
		if (gstPosCapability.uiScreenType)
			lcdDrLogo(70, 40, Bmp_Display);
		else
			lcdDrLogo(5, 0, Bmp_Display_NoColor);
		if(gstPosCapability.uiScreenType)
		{
//			DispMulLanguageString(170, 1, DISP_CFONT, NULL, "UNIONPAY");
//			lcdDisplay(170, 3, DISP_CFONT, gstPosVersion.szVersion);
			DispMulLanguageString(0, 7, DISP_HFONT16|DISP_MEDIACY, NULL, "Bienvenido");
			DispMulLanguageString(0, 8, DISP_HFONT16|DISP_MEDIACY, NULL, "Oprima ENTER");
			DispMulLanguageString(0, 9, DISP_HFONT16|DISP_MEDIACY, NULL, gstPosVersion.szVersion);
			lcdFlip();
			if( stPosParam.ucEmvSupport==PARAM_OPEN )
			{
//				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "SWIPE/INSERT");
			}
			else
			{
//				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "PLS SWIPE-->");
			}
		}
		else
		{
//			DispMulLanguageString(58, 0, DISP_CFONT, NULL, "UNIONPAY");
//			lcdDisplay(58, 2, DISP_CFONT, gstPosVersion.szVersion);

			if( stPosParam.ucEmvSupport==PARAM_OPEN )
			{
//				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SWIPE/INSERT");
			}
			else
			{
//				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PLS SWIPE-->");
			}
		}
		lcdFlip();
	}
}



uint8_t CheckLimitTotal(void)
{
	if( memcmp(stLimitTotal.sFrnCreditAmt, "\x99\x99\x00\x00\x00\x00", 6)>=0 ||
		memcmp(stLimitTotal.sFrnDebitAmt,  "\x99\x99\x00\x00\x00\x00", 6)>=0 ||
		memcmp(stLimitTotal.sRmbCreditAmt, "\x99\x99\x00\x00\x00\x00", 6)>=0 ||
		memcmp(stLimitTotal.sRmbDebitAmt,  "\x99\x99\x00\x00\x00\x00", 6)>=0 )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "AMOUNT EXCEED");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "SETTLE NOW!");
		lcdFlip();
		ErrorBeep();
		kbGetKey();
		return FALSE;
	}
	return TRUE;
}

void SetParaFunc(void)
{
	int iRet;

	iRet = CheckParamPass();
	if( iRet!=1 )
	{
		return ;
	}
	lcdCls();
	SetupPosPara();

	return ;
}

uint8_t DispTermInfo(void)
{
    uint8_t	szBuff[200];
#ifdef _POS_TYPE_8210
	int     iRet=0;
#endif


	lcdCls();
	sprintf((char *)szBuff, SRC_VERSION);
	lcdDisplay(0, 0, DISP_CFONT, (char *)szBuff);
	sprintf((char *)szBuff, gstPosVersion.szVersion);
	lcdDisplay(0, 2, DISP_CFONT, (char *)szBuff);
	lcdDisplay(0, 4, DISP_CFONT, (char *)stPosParam.szUnitNum);
	lcdDisplay(0, 6, DISP_CFONT, (char *)stPosParam.szPosId);
#ifdef _POS_TYPE_8210
	if (gstPosCapability.uiScreenType)
	{
		memset(szBuff, 0, sizeof(szBuff));
		wc_get_libversion((char*)szBuff);
		lcdDisplay(0, 8, DISP_CFONT, "libwnet:%s",(char*)szBuff+12);//Sep 19 2014_1.0.0
		memset(szBuff, 0, sizeof(szBuff));
		iRet = wc_get_wnetversion((char*)szBuff);
		if(iRet == 0)
			lcdDisplay(0, 10, DISP_CFONT, "wnet:%s",(char*)szBuff+12);//Sep 19 2014_1.0.0
	}
	else
	{
		lcdFlip();
		kbGetKeyMs(5000);
		memset(szBuff, 0, sizeof(szBuff));
		wc_get_libversion((char*)szBuff);
		lcdDisplay(0, 0, DISP_CFONT, "libwnet:%s",(char*)szBuff+12);//Sep 19 2014_1.0.0
		memset(szBuff, 0, sizeof(szBuff));
		iRet = wc_get_wnetversion((char*)szBuff);
		if(iRet == 0)
			lcdDisplay(0, 2, DISP_CFONT, "wnet:%s",(char*)szBuff+12);//Sep 19 2014_1.0.0
	}
#endif
	lcdFlip();
	kbGetKeyMs(5000);
	return NO_DISP;
}


int TermCheck(void)
{
	if( strcmp((char *)stTransCtrl.szNowTellerNo, TELLER_LOCK)==0 )
	{
		TellerRunLock();
		return 1;
	}
	return 0;
}


int CheckManagePass(void)
{
	uint8_t ManagePass[16], passWd[16];
	int     iRet;

	memset(ManagePass, 0, sizeof(ManagePass));
	memcpy(ManagePass, stPosParam.szManagePwd, 6);

	while(1)
	{   
		
//		lcdCls();
		lcdClrLine(2, 7);
		memset(passWd, 0, sizeof(passWd));
//		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   CONFIGURACION   ");
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "CLAVE DE ACCESO:");
		lcdFlip();
		lcdGoto(56, 6);
		iRet = kbGetString(KB_EN_CRYPTDISP+KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6,
			(stPosParam.ucOprtLimitTime*1000), (char *)passWd);
		if( iRet==6 )
		{
			if( strcmp((char *)passWd, (char *)ManagePass)==0 || 
				(stPosParam.TmsAdminPwdSwitch == PARAM_OPEN && 
				strcmp((char *)passWd, (char *)stPosParam.TmsManagePwdTemp)==0))
			{
				return 1;
			}
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR!");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);

		}
		else
		{
			return 0;
		}
	}
}


int CheckParamPass(void)
{
	uint8_t sysPass[16], passWd[16];
	int     iRet;

	memset(sysPass, 0, sizeof(sysPass));
	memcpy(sysPass, stPosParam.szParamsPwd, 6);

	while(1)
	{   
		
		lcdCls();
		memset(passWd, 0, sizeof(passWd));
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   CONFIGURACION   ");
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "CLAVE DE ACCESO:");
		lcdFlip();
		lcdGoto(56, 6);
		iRet = kbGetString(KB_EN_CRYPTDISP+KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6,
			(stPosParam.ucOprtLimitTime*1000), (char *)passWd);
		if( iRet==6 )
		{
			if( strcmp((char *)passWd, (char *)sysPass)==0 || 
				(stPosParam.TmsAdminPwdSwitch == PARAM_OPEN && 
				strcmp((char *)passWd, (char *)stPosParam.TmsAdminPwdTemp)==0))
			{
				return 1;
			}
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR!");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);

		}
		else
		{
			return 0;
		}
	}
}

uint8_t DispLogon(void)
{
//	int ucRet;
	int key;

	const char *LoginMenu[] =
	{
		"MENU PRINCIPAL",
		"1-LOGIN",
		"2-CONFIGURACION"
	};
	
	lcdCls();
	if( stPosParam.szLogonFlag==LOGGEDOFF )
	{
		key = DispMenu3(LoginMenu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
		switch(key)
		{
		case KEY1:
			stTemp.iTransNo = POS_LOGON;
			return OK;
			break;
		case KEY2:
			SetParaFunc();
			return OK;
			break;
		default:
			return NO_DISP;
			break;	
		}
	}
	
	return OK;
}
int  DispVenta(void)
{
	int iRet;
	int key;

	const char *	VentaMenu[] =
	{
		"VENTA TARJETA",
		"1-VENTA NORMAL",
		"2-MONEDERO"
	};
	
	lcdCls();

	key = DispMenu3(VentaMenu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
	switch(key)
	{
		case KEY1:
			stTemp.iTransNo = POS_SALE;
			break;
		case KEY2:		
			InitMenu(MENU_MODE_1, " VENTA MONEDERO");
	
			MainMenuAddMenuItem(PARAM_OPEN,   PURSE_SODEXO,   "SODEXO            ",         NULL);
			MainMenuAddMenuItem(PARAM_OPEN,   PURSE_PUNTO,     "PUNTO CLAVE        ",         NULL);
			MainMenuAddMenuItem(PARAM_OPEN,   PURSE_TODITO,    "TODITO        ",         NULL);			
			MainMenuAddMenuItem(PARAM_OPEN,   PURSE_EDENRED,    "EDENRED        ",         NULL);

			//monedero:Ǯ�����ף�����֪���������̺ͱ���
			iRet = DispDynamicMenu(1);
			if( iRet == NO_TRANS )		//NO_TRANS
			{
				return NO_TRANS;
			}
			else
			{
				PosCom.stTrans.TransFlag = iRet;
				stTemp.iTransNo = PURSE_SALE;
			}
			break;
		default:
			return NO_TRANS;
			break;	
	}
	return OK;
}


int  DispCheck_Balance(void)
{
	int iRet;
	int key;

	const char *	BalanceMenu[] =
	{
		"VENTA MONEDERO",
		"1-PUNTO CLAVE",
		"2-EDENRED"
	};
	
	lcdCls();

	key = DispMenu3(BalanceMenu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
	switch(key)
	{
		case KEY1:
			stTemp.iTransNo = PURSE_GETBALANCE;
			PosCom.stTrans.TransFlag = PURSE_GETRULE;
			break;
		case KEY2:		
			stTemp.iTransNo = PURSE_GETBALANCE;
			PosCom.stTrans.TransFlag = PURSE_GETBALANCE;
			break;
		default:
			return NO_DISP;
			break;	
	}
	return OK;
}

uint8_t ProcEvent(void)
{
	int 	ucRet, ucEvent;

	PosCom.ucPBOCFlag = 0;
	if( DirectIntoMenu == 1 )   // NETPAY Ҫ�󷵻����˵�
	{
		ucEvent = KEY_ENTER;
		DirectIntoMenu = 0;   //��ֱ�ӽ������˵���־ȡ��
		
	}
	else
	{
		ucEvent = WaitEvent();
	}
	GetPosTime(stTemp.szDate, stTemp.szTime);
	strcpy((char *)PosCom.stTrans.szDate, (char *)stTemp.szDate);
	strcpy((char *)PosCom.stTrans.szTime, (char *)stTemp.szTime);	
	switch( ucEvent )
	{
	case OK:
		return OK;
	case KEY_MENU:
	case KEY_ENTER:	
		ucRet = SelectTrans();
		if( ucRet!=OK )
		{
			return ucRet;
		}
		break;
	case KEY_FN:
		SetParaFunc();
		break;

	case KEY_CANCEL:
		return E_APP_EXIT;

	case E_APP_EXIT:
	case E_APP_RESET:
	    CommHangUp(FALSE);
	    return ucEvent;	
	default:
		return NO_DISP;
	}

	return OK;
}

uint CheckTransValid(void)
{
	uint8_t	ucRet;

	if( CheckSupport(stTemp.iTransNo)==FALSE )
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " NOT SUPPORT ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);
		return NO_DISP;
	}
	if(( HaveMacData(stTemp.iTransNo)==TRUE)&&(stPosParam.ucPINMAC == 1))
	{
		lcdCls();
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " LOGON AGAIN ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);
		return NO_DISP;
	}
	if (ChkIfSaveLog() || stTemp.iTransNo==POS_SETT)
	{
		if(stTransCtrl.ucPosStatus==SETTLE_STATUS && stTemp.iTransNo!=POS_SETT)
		{
			lcdCls();
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   SETTLEMENT   ");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LAST SETTLE HALT");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "    CONTINUE?   ");
			lcdFlip();
			ErrorBeep();
			if (kbGetKeyMs(30000) != KEY_ENTER)
			{
				return NO_DISP;
			}
			ucRet = PosSettleTran();
			if( ucRet==OK )
			{
				ClearAllTranLog();
			}
			return NO_DISP;
		}
		else if( stTransCtrl.ucPosStatus==BATCH_STATUS )
		{
			lcdCls();
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     UPLOAD      ");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "LAST UPLOAD HALT");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "CONTINUE");
			lcdFlip();
			ErrorBeep();
			if (kbGetKeyMs(30000) != KEY_ENTER)
			{
				return NO_DISP;
			}

			ucRet = PosBatchSendTran();
			if( ucRet==OK )
			{
				ClearAllTranLog();
			}
			return NO_DISP;
		}

		if(stTemp.iTransNo!=POS_SETT)
		{
			if( (stTransCtrl.iTransNum>=stPosParam.iMaxTransTotal) || (stTransCtrl.iTransNum>=MAX_TRANLOG))
			{
				lcdCls();
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SAVE LOG EXCEED ");
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "SETTLE FIRST");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(2000);
				return NO_DISP;
			}
			if(CheckLimitTotal()==FALSE)
				return NO_DISP;
		}
	}

	return 0;
}

uint8_t ProcSelectTran(void)
{
	uint8_t	ucRet  = 0;

	PosCom.stTrans.iTransNo = stTemp.iTransNo;

	//Ĭ�ϵ�����£���������Ϊ'1'
	PosCom.stTrans.szInputType ='1'; 
	
	if( CheckTransValid() != 0 )
		return NO_DISP;

	PrintDebug("%s%d", "test PrintDebug stTemp.iTransNo:",stTemp.iTransNo);
	switch( stTemp.iTransNo )
	{
	//����Ǯ������
	case PURSE_SALE:
		ucRet =Electronic_wallet_online();
		break;
	case POS_SALE:
		ucRet = OnlineSale();
		break;
	case PURSE_GETBALANCE:
		ucRet = GetBalance_FromBankPlat(PosCom.stTrans.TransFlag);
		break;
	case POS_PREAUTH:
		ucRet = PreAuth();
		break;

	case POS_AUTH_CM:
		ucRet = OnlineAuthConfirm();
		break;

	case POS_OFF_CONFIRM:
		ucRet = OfflineAuthConfirm();
		break;

	case POS_PREAUTH_VOID:
		ucRet = VoidPreAuth();
		break;

	case POS_SALE_VOID:
	case POS_AUTH_VOID:
		ucRet = VoidSaleAuth();
		break;

	case POS_REFUND:
		ucRet = RefundTran();
		break;

	case POS_PREAUTH_ADD:
		ucRet = PreAuthAdd();
		break;

	case POS_QUE:
		ucRet = QueryBalance();
		break;

	case POS_DOWNLOAD_PARAM:
		ucRet = PosDownLoadParam();
		break;

	case POS_UPLOAD_PARAM:
		ucRet = PosUpLoadParam();
		break;

	case RESPONSE_TEST:
		ucRet = TestResponse();
		break;

	case DOWNLOAD_EMV_CAPK:
		ucRet = DownLoadCAPK(FALSE);
		break;

	case DOWNLOAD_EMV_PARAM:
		ucRet = DownLoadEmvParam(FALSE);
		break;

	case OFF_SALE:
		ucRet = OfflineSale();
		break;

	case POS_LOGON:
		ucRet = PosOnlineLogon(1);
		break;

	case POS_LOGOFF:
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
		ucRet = PosOnlineLogoff(1);
		break;

	case PRT_ANY:
		ucRet = reprintTrans(1);
		break;
	case PRT_DETAIL:
		ucRet = PrintDetail(TRUE,FALSE);

		break;
	case PRT_LAST:
		ucRet = reprintTrans(0);
		break;
	case PRT_TIP:
		PrintTipDetail(TRUE,FALSE);
		break;

	case PRT_LAST_TOTAL:
		ucRet = ReadLastTotal();
		if( ucRet!=OK )
		{
			return NO_DISP;
		}
		ucRet = PrtTransTotal(0, 0);
		break;

	case PRT_TOTAL:
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   PRINT TOTAL  ");
		lcdFlip();
		
		ucRet = PrintTransTotal(1, stPosParam.lNowBatchNum);
		break;

	case SETUP_EXTNUM:
		PabxSetup(0);
		ucRet = NO_DISP;
		break;

	case QUE_BATCH_MX:
		ucRet = queryDetail(1);
		break;

	case QUE_BATCH_ANY:
		ucRet = queryTrans();
		break;

	case QUE_BATCH_TOTAL:
		ucRet = QueryTransTotal();
		if( ucRet==OK )
		{
			ucRet = NO_DISP;
		}
		break;

	case OPER_QUE_TERM_INFO:
		ucRet = DispTermInfo();
		break;

	case POS_SETT:
		ucRet = PosSettleTran();

		if( ucRet==OK )
		{
			ClearAllTranLog();
			
			ucRet = NO_DISP;
		}
		if( stTemp.sPacketHead[5]=='6' )
		{
			stTransCtrl.ucDownAppFlag = 1;
			SaveCtrlParam();
		}
		break;
	case ECHO_TEST:
		ucRet = EchoTest();
		break;

	case PRE_VOL_CTL:
		ucRet =PrintCtlvolume(1);
		break;
	default:
		ucRet = E_NO_TRANS;
		break;
	} // switch

	return ucRet;
}

void PabxSetup(uint8_t ucMode)
{
	char  buf[16];
	int iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "SETUP PAX ");

	if (0 == ucMode)
	{
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT MANAGE PWD");
		lcdFlip();
		lcdGoto(0, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6,
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet!=6 )
			return ;

		if( strcmp(buf, operStrc[0].szPassword)!=0x00 )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			return ;
		}
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT PABX:");
	strcpy(buf, (char *)stPosParam.szExtNum);
	iRet = PubGetString( ECHO_IN + ALPHA_IN, 0, 9,(uint8_t *)buf,stPosParam.ucOprtLimitTime,0 );
	if(iRet == 0x00)
	{
		strcpy((char *)stPosParam.szExtNum, buf);
		SaveAppParam();
	}

	return;
}


void GetPosCapablity(void)
{
	char    szPosConfigInfo[140];
	int     iRet;
	int     iScreenwidth, iScreenheight;

	memset(szPosConfigInfo, 0, sizeof(szPosConfigInfo));
	iRet = sysReadConfig((uint8_t *)szPosConfigInfo);
	if( iRet<=0 )
	{
		return;
	}

	memset((char*)&gstPosCapability, 0, sizeof(gstPosCapability));
	gstPosCapability.uiUnContactlessCard = szPosConfigInfo[12];
	gstPosCapability.uiPrnType = 'T';
	gstPosCapability.uipostsScreen = 0;
	gstPosCapability.uiScreenType = SCREEN_TYPE_128_64; //not Color screen
	lcdGetSize(&iScreenwidth, &iScreenheight);
	if (!(iScreenheight==64 && iScreenwidth == 128))
	{
		gstPosCapability.uipostsScreen = szPosConfigInfo[13];
		gstPosCapability.uiScreenType = SCREEN_TYPE_320_240;  //Color screen
	}

	gstPosCapability.uiPosType = szPosConfigInfo[0];
#ifdef _POS_TYPE_8110
	if (gstPosCapability.uiPosType == 1) //8110
	{
		strcpy(gstPosVersion.szVersion,"OXG19");
		strcpy(gstPosVersion.szCertificate,"123136");
	}
	else if (gstPosCapability.uiPosType == 2) //6110
	{
		strcpy(gstPosVersion.szVersion,"V311204");
		strcpy(gstPosVersion.szCertificate,"123125");
	}
	gstPosCapability.uiPortType= PORT_COM1;
#else
	gstPosCapability.uiPrnType = szPosConfigInfo[1];

	if (gstPosCapability.uiPosType == 1) // 8110P/8110PS
	{
		strcpy(gstPosVersion.szVersion,"OXG19");
		strcpy(gstPosVersion.szCertificate,"123136");
		gstPosCapability.uiPortType = PORT_COM1;
	}
	else if (gstPosCapability.uiPosType == 4)	//6210
	{
		strcpy(gstPosVersion.szVersion,"OXG19");
		strcpy(gstPosVersion.szCertificate,"123221");
		gstPosCapability.uiPortType = PORT_XX;
	}
	else if (gstPosCapability.uiPosType == 5)	//7210
	{
		strcpy(gstPosVersion.szVersion,"OXG19");
		strcpy(gstPosVersion.szCertificate,"123226");
		gstPosCapability.uiPortType = PORT_XX;
	}
	else  //8210
	{
		strcpy(gstPosVersion.szVersion,"OXG19");
		strcpy(gstPosVersion.szCertificate,"123209");
		gstPosCapability.uiPortType = PORT_COM1;
	}

#endif

#ifdef _POS_TYPE_7110
	strcpy(gstPosVersion.szVersion,"V311310");
	strcpy(gstPosVersion.szCertificate,"123217");
#endif

	gstPosCapability.uiSecuType = szPosConfigInfo[5];

	gstPosCapability.uiCommType |= CT_RS232;

	if (szPosConfigInfo[2]!=0)
	{
		gstPosCapability.uiCommType |= CT_MODEM;
		gstPosCapability.uiCommType |= CT_MODEMPPP;
	}

	if (gstPosCapability.uiPosType == 4 || szPosConfigInfo[8]!=0)
	{
		gstPosCapability.uiCommType |= CT_TCPIP;
	}

	if (szPosConfigInfo[9]!=0)
	{
		gstPosCapability.uiCommType |= CT_GPRS;
	}

	if (szPosConfigInfo[10]!=0)
	{
		gstPosCapability.uiCommType |= CT_CDMA;
	}

	if (szPosConfigInfo[11]!=0)
	{
		gstPosCapability.uiCommType |= CT_WIFI;
	}

	switch (szPosConfigInfo[3])
	{
	case 1:		gstPosCapability.uiMaxSyncPSTNRate = 1200;		break;
	case 2:		gstPosCapability.uiMaxSyncPSTNRate = 2400;		break;
	case 3:		gstPosCapability.uiMaxSyncPSTNRate = 9600;		break;
	case 4:		gstPosCapability.uiMaxSyncPSTNRate = 14400;		break;
	default:		break;
	}

	switch (szPosConfigInfo[4])
	{
	case 1:		gstPosCapability.uiMaxAsynPSTNRate = 1200;		break;
	case 2:		gstPosCapability.uiMaxAsynPSTNRate = 2400;		break;
	case 3:		gstPosCapability.uiMaxAsynPSTNRate = 4800;		break;
	case 4:		gstPosCapability.uiMaxAsynPSTNRate = 7200;		break;
	case 5:		gstPosCapability.uiMaxAsynPSTNRate = 9600;		break;
	case 6:		gstPosCapability.uiMaxAsynPSTNRate = 12000;		break;
	case 7:		gstPosCapability.uiMaxAsynPSTNRate = 14400;		break;
	case 8:		gstPosCapability.uiMaxAsynPSTNRate = 19200;		break;
	case 9:		gstPosCapability.uiMaxAsynPSTNRate = 24000;		break;
	case 10:	gstPosCapability.uiMaxAsynPSTNRate = 26400;		break;
	case 11:	gstPosCapability.uiMaxAsynPSTNRate = 28800;		break;
	case 12:	gstPosCapability.uiMaxAsynPSTNRate = 31200;		break;
	case 13:	gstPosCapability.uiMaxAsynPSTNRate = 33600;		break;
	case 14:	gstPosCapability.uiMaxAsynPSTNRate = 48000;		break;
	case 15:	gstPosCapability.uiMaxAsynPSTNRate = 56000;		break;
	default:
		break;
	}
}


