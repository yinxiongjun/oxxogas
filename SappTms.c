#include "posapi.h"
#include "MathLib.h"
#include "terstruc.h"
#include "sapptms.h"
#include "tranfunclist.h"
#include "seos.h"
#include "posparams.h"
#include "showDebug/showDebug.h"
#include "ini.h"
#include "libsbTms.h"

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>

/****************************************************************************
NAME
    SappTms.h - ���������ļ������ĺ���

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    Carel.Cheng     2013.08.24      - created
****************************************************************************/
extern int giWriteFd, giReadFd;
extern char s_gszAppName[50];   //add andy.wang 2013-9-13 0:07:02
char gszManageVersion[256];

TMS_APP_PARAMS TmsParam[] = 
{
	{"01000001",		STR_PARAMS(stPosParam.szUnitNum),	      	STRING_ITEM},   //�̻��� 
	{"01000002",		STR_PARAMS(stPosParam.szUnitChnName),     	STRING_ITEM}, 	//�̻�����
	{"01000003",		STR_PARAMS(stPosParam.szEngName),				STRING_ITEM},//Ӣ���̻���		
	{"01000005",		STR_PARAMS(stPosParam.szPosId),				STRING_ITEM},  //�ն˺�
	{"01000082",		LONG_PARAMS(stPosParam.lNowTraceNo),		BIN_ITEM},	//��ˮ��
	{"01000083",		LONG_PARAMS(stPosParam.lNowBatchNum),		BIN_ITEM},  //���κ�
	{"01000048",		INT_PARAMS(stPosParam.ucKeyIndex),				BIN_ITEM},//����Կ����
	{"01000042",		CHAR_PARAMS(stPosParam.ucTicketNum),		ASSCII_ITEM},  //������ӡ����
	{"01000088",		INT_PARAMS(stPosParam.ucResendTimes),		BIN_ITEM},//�����ط�����  	
	{"01000049",		STR_PARAMS(stPosParam.szTpdu),								STRING_ITEM},//TPDU
	{"01000008",		INT_PARAMS(stPosParam.ucCommWaitTime),				BIN_ITEM},//ͨѶ��ʱʱ��,��
	{"01000011",		INT_PARAMS(stPosParam.ucDialRetryTimes),					BIN_ITEM},//ͨѶ���Դ���,1-3																			//����ͨѶ���糣����
	{"01000067",		STR_PARAMS(stPosParam.szPOSRemoteIP), 		STRING_ITEM},
	{"01000063",		STR_PARAMS(stPosParam.szPOSRemotePort), 		STRING_ITEM},
	{"01000064",		STR_PARAMS(stPosParam.szPOSRemoteIP2),			STRING_ITEM},
	{"01000065",		STR_PARAMS(stPosParam.szPOSRemotePort2),		STRING_ITEM},																		//�������ĺ���,������GPRS��CDMA,ATD*99***1#
	{"01000009",		CHAR_PARAMS(stPosParam.bPreDial), 					ASSCII_ITEM},//�Ƿ�Ԥ����
	{"01000012",		STR_PARAMS(stPosParam.szPhoneNum1), 					STRING_ITEM},
	{"01000013",		STR_PARAMS(stPosParam.szPhoneNum2), 					STRING_ITEM}, 
	{"01000014",		STR_PARAMS(stPosParam.szPhoneNum3), 					STRING_ITEM}, 
	{"0100LD01",		NOTUSER_PARAM},//�������ع���˵�����
	{"0100LD03",		NOTUSER_PARAM},//�Ƿ�֧��С�����Ȩ
	{"0100LD05",		NOTUSER_PARAM},//�ѻ��ն� ���ױ������ѷ�ֵ
	{"0100LD06",		NOTUSER_PARAM},//�ѻ��ն� ���ʽ���趨
	{"********", NULL, 0,	}
};


static void convert_option(uint8_t *lpIn, uint8_t *lpout)
{
	uint8_t i;
	uint8_t uctemp,uct = 0;
	uint8_t ucOptIndex, ucOptValue;
	uint16_t iOptNum = 0;

	for(i = 0 ; i < 4; i ++)
	{
		uctemp = toupper(lpIn[i]);
		if(uctemp >= 'A' && uctemp <= 'F')
		{
			uct = uctemp - 0x37;
			iOptNum = uct + iOptNum * 16;
		}
		else if(uctemp >= '0' && uctemp <= '9')
		{
			uct = uctemp - 0x30;
			iOptNum = uct + iOptNum * 16;
		}
		else
		{
			iOptNum += 0;
		}
	}
	ucOptIndex = iOptNum >> 8;
	ucOptValue = iOptNum & 0xFF;
	if(stPosParam.sTransOpen[ucOptIndex] & ucOptValue)
	{
		lpout[0] = '1';
	}
	else
	{
		lpout[0] = '0';
	}
}


//������ת��ΪASCILL��
static void convert_bin(uint8_t *dest, void *srcStr, uint8_t len)
{
	int templen;
	
	switch(len)
	{
		case sizeof(char):
			templen = *(uint8_t *)srcStr;
			break;

		case sizeof(int):
			templen = *(int *)srcStr;
			break;

		default:
			templen = 0;
			break;
	}
	sprintf((char *)dest, "%d", templen);
}


int GetCurrentDir(char *PszCurrentDir)
{
	char temp[256];
	uid_t uid;
	struct passwd *user;

	if(PszCurrentDir == NULL)
	{
		return -1;
	}
	memset(temp,0,sizeof(temp));
	uid = getuid();
	user = getpwuid(uid);
	if(user == NULL)
	{
		return -1;
	}
	else
	{
		snprintf(temp,sizeof(temp),"%s",user->pw_dir);
		strcpy(PszCurrentDir,temp);
		return 0;
	}

}


//Flag:TRUE��ʾ�еڶ���������
//Flag:FLASE��ʾֻ��һ��������
void TmsMakeAppParam(uint8_t *lpOut,uint16_t *OutLen, int *BeginNum, int *Flag)
{
	uint8_t ParamNum = 0, szTemp[128];
	int len = 3, templen=0;
	uint8_t i=0;
	uint8_t TmpNum = 0;
	int LastLen = 0;
	
	for(i = *BeginNum; TmsParam[i].Tag[0] != '*'; i++)
	{
		LastLen = len;
		TmpNum = ParamNum;
		
		if(TmsParam[i].tms_ptr == NULL && TmsParam[i].len == 1)    //special Tmsparam
		{
			if(strcmp((char *)TmsParam[i].Tag, (char *)"04000001") == 0)  //ͨѶ��ʽ
			{
				memcpy(&lpOut[len], TmsParam[i].Tag, 8);
				len += 8;
				memcpy(&lpOut[len], "001", 3);
				len += 3;
				switch(stPosParam.stTxnCommCfg.ucCommType)  //0Nac,1Gprs,2Cdma,3Rs232,4TCP/IP
				{
					case CT_MODEM:
						lpOut[len] = '1';
						break;
					case CT_GPRS:
						lpOut[len] = '2';
						break;
					case CT_CDMA:
						lpOut[len] = '3';
						break;
					case CT_TCPIP:
						lpOut[len] = '4';
						break;
					case CT_WIFI:
						lpOut[len] = '5';
						break;
					case CT_RS232:
						lpOut[len] = '6';
						break;
					case CT_MODEMPPP:
						lpOut[len] = '7';
						break;

					default:
						lpOut[len] = '1';
						break;
				}
				len += 1;
				ParamNum ++;
			}
			else
			{
				continue;
			}
		}
		else if(TmsParam[i].tms_ptr == NULL && TmsParam[i].len == 0)    //û�õ��Ĳ���
		{
			continue;
		}
		else
		{
			switch(TmsParam[i].type)
			{
				case STRING_ITEM:
					memcpy(&lpOut[len], TmsParam[i].Tag, 8);
					len += 8;
					templen = strlen((char *)TmsParam[i].tms_ptr);
					sprintf((char *)&lpOut[len], "%03d", templen);
					len += 3;
					strcpy((char *)&lpOut[len], TmsParam[i].tms_ptr);
					len += templen;
					ParamNum ++;
					break;

				case BIN_ITEM:
					memcpy(&lpOut[len], TmsParam[i].Tag, 8);
					len += 8;
					memset(szTemp, 0, sizeof(szTemp));
					convert_bin(szTemp, TmsParam[i].tms_ptr, TmsParam[i].len);
					templen = strlen((char *)szTemp);
					sprintf((char *)&lpOut[len], "%03d", templen);
					len += 3;
					strcpy((char *)&lpOut[len], (char *)szTemp);
					len += templen;
					ParamNum ++;
					break;

				case ASSCII_ITEM:
					memcpy(&lpOut[len], TmsParam[i].Tag, 8);
					len += 8;
					memcpy(&lpOut[len], "001", 3);
					len += 3;
					lpOut[len] = *(char *)TmsParam[i].tms_ptr;
					len += 1;
					ParamNum ++;
					break;

				case BCD_ITEM:
					memcpy(&lpOut[len], TmsParam[i].Tag, 8);
					len += 8;
					templen = (TmsParam[i].len - 1)*2;
					sprintf((char *)&lpOut[len], "%03d", templen);
					len += 3;
					PubBcd2Asc((char *)TmsParam[i].tms_ptr, TmsParam[i].len - 1, (char *)&lpOut[len]);
					len += templen;
					ParamNum ++;
					break;

			    case OPTION_ITEM:		//add andy.wang 2013-9-26 18:08:35 for tms
			    	memcpy(&lpOut[len], TmsParam[i].Tag, 8);
					len += 8;
					memcpy(&lpOut[len], "001", 3);
					len += 3;
					convert_option((uint8_t *)TmsParam[i].tms_ptr, (uint8_t *)&lpOut[len]);
					if(strcmp((char *)TmsParam[i].Tag, "02000008") == 0)
					{
						lpOut[len] += 1;
					}
					else if(strcmp((char *)TmsParam[i].Tag, "04000004") == 0)
					{
						if(lpOut[len] == '1') lpOut[len] = '0';
						else if(lpOut[len] == '0') lpOut[len] = '1';
						else lpOut[len] = '0';
						
					}
					len += 1;
					ParamNum ++;
					break;
					
				default:
					break;
					
			}
		}
		
		//������ȴ���620ʱ,������������
		if(len > 620)
		{
			*Flag = TRUE;
			*BeginNum = i;
			ParamNum = TmpNum;
			len = LastLen;
#ifdef _ZYL_DEBUG_
	printf("��ȡ��Ӧ�ø���:%d\n", i-1);
#endif
			break;
		}

	}
	memset(szTemp, 0, sizeof(szTemp));
	sprintf((char *)szTemp, "%03d", ParamNum);
	memcpy(lpOut, szTemp, 3);
	*OutLen = len;
}

int LoadAppTmsFile(void)
{
	int iRet;
	uint8_t ucFileName[256];
	uint8_t ucAppdeskFileName[256];
	char szcurrentdir[256];
	uint8_t tmpVer[16];
	char sn_num[9];
	
	void *file;
	const char *value;
	const char *section = "Desktop Entry";
	struct _HF_SAPP_INFO_ Sapp;
	
	memset(&Sapp, 0, sizeof(Sapp));
	memset((char *)tmpVer, 0, sizeof(tmpVer));
	memset((char *)ucFileName, 0, sizeof(ucFileName));
	memset((char *)ucAppdeskFileName, 0, sizeof(ucAppdeskFileName));

	GetCurrentDir(szcurrentdir);
	sprintf((char *)ucAppdeskFileName, "%s/MainAppDesk/%s.desk", szcurrentdir, s_gszAppName);
	sprintf((char *)ucFileName, "%s/SubInfo/%s.info", szcurrentdir, s_gszAppName);

	snprintf(ParamPath, sizeof(ParamPath), "%s", ucFileName);;//���油���ļ�·��
#ifdef FY_DEBUG
	printf("��Ӧ��ucAppdeskFileName:%s\n",ucAppdeskFileName);
	printf("��Ӧ��·��ParamPath:%s\n",ParamPath);
	printf("��Ӧ��·��ucFileName:%s\n", ucFileName);
#endif
	
	file = ini_open((char *)ucAppdeskFileName, O_RDONLY);
	
	if(file)
   	{
	   ini_load(file);
	   value = ini_get(file, section, "Version");
	   if(value)
	   {
	       snprintf((char *)tmpVer, sizeof(tmpVer), "%s", value);
	   }
	}
	ini_close(file);

//	strcpy((char *)Sapp.appName,(char *)AppInfo.AppName);
	snprintf((char *)Sapp.appName, sizeof(Sapp.appName), "%s", s_gszAppName);
//	snprintf((char *)Sapp.appVer, sizeof(Sapp.appVer), "%s", SRC_VERSION);
#ifdef FY_DEBUG
	printf("��Ӧ������:%s\n", Sapp.appName);
	printf("��Ӧ�ð汾:%s\n", tmpVer);
#endif

	if(fileExist((char *)ucFileName)<0)
	{
		memcpy(Sapp.merchantId, stPosParam.szUnitNum, 15);
		memcpy(Sapp.termId, stPosParam.szPosId, 8);		
		strcpy((char *)Sapp.appVer, (char *)tmpVer);
		Sapp.ucUpdateParamFlag = PARAM_CLOSE;
		Sapp.iUpdateParamNum = 0;

		PubWritemanageFile(ParamPath, (uint8_t *)&Sapp, 0, sizeof(struct _HF_SAPP_INFO_));
	}
	else
	{
		memset(&Sapp, 0, sizeof(Sapp));
		iRet = PubReadManageFile(ParamPath, (uint8_t *)&Sapp, 0, sizeof(struct _HF_SAPP_INFO_));
		if(iRet != OK)
		{
			DispFileErrInfo();
			return (E_MEM_ERR);
		}
	
		strcpy((char *)Sapp.appVer, (char *)tmpVer);
	
		SaveAppParam();
		PubWritemanageFile(ParamPath, (uint8_t *)&Sapp, 0, sizeof(struct _HF_SAPP_INFO_));	
	}

	return OK;
}

int ReadPubInfoFile(void)
{
	int iRet;
	int fd;
	int iReadLen = 0;
	char szcurrentdir[256];
	char szsappinfodir[256];

	memset(szcurrentdir, 0, sizeof(szcurrentdir));
	GetCurrentDir(szcurrentdir);
	snprintf(szsappinfodir, sizeof(szsappinfodir), "%s/%s", szcurrentdir, FILE_PUB_APP_INFO);

	iReadLen = sizeof(stPubAppInfo);
	memset(&stPubAppInfo, 0, iReadLen);

	fd = fileOpen((char *)szsappinfodir, O_RDWR);
	if(fd<0)
	{
		DispFileErrInfo();
		return E_MEM_ERR;
	}

	iRet = fileSeek(fd, 0, SEEK_SET);
	if( iRet < 0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}

	iRet = fileRead(fd, &stPubAppInfo, iReadLen);
	fileClose(fd);
	if(iRet != iReadLen)
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}

	return OK;
}


int WritePubInfoFile(void)
{
	int iRet;
	int fd;
	int iReadLen = 0;
	char szcurrentdir[256];
	char szsappinfodir[256];

	memset(szcurrentdir, 0, sizeof(szcurrentdir));
	GetCurrentDir(szcurrentdir);
	snprintf(szsappinfodir, sizeof(szsappinfodir), "%s/%s", szcurrentdir, FILE_PUB_APP_INFO);

	iReadLen = sizeof(stPubAppInfo);

	fd = fileOpen((char *)szsappinfodir, O_RDWR);
	if(fd<0)
	{
		DispFileErrInfo();
		return E_MEM_ERR;
	}

	iRet = fileSeek(fd, 0, SEEK_SET);
	if( iRet < 0 ) 
	{
		DispFileErrInfo();
		fileClose(fd);
		return (E_MEM_ERR);
	}

	iRet = fileWrite(fd, &stPubAppInfo, iReadLen);
	fileClose(fd);
	if(iRet != iReadLen)
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}


	return OK;
}


//���油��������,�̻����ն˺�
int SaveAppTmsParam(void)
{
	int iRet;
	int Flag = FALSE;
	int ParamNum = 0;
	char buf[5];
	uint8_t ParamBuf[1024];
	uint16_t ParamLen = 0;
	struct _HF_SAPP_INFO_ Sapp;

	memset(&Sapp, 0, sizeof(Sapp));
	memset(buf, 0, sizeof(buf));
	memset((char *)ParamBuf, 0, sizeof(ParamBuf));
	
	iRet = PubReadManageFile(ParamPath, (uint8_t *)&Sapp, 0, sizeof(struct _HF_SAPP_INFO_));
	if(iRet != OK)
	{
		DispFileErrInfo();
		return (E_MEM_ERR);
	}
	
	memcpy(Sapp.merchantId, stPosParam.szUnitNum, 15);
	memcpy(Sapp.termId, stPosParam.szPosId, 8);

	//���������Ϊ0
	if(stTransCtrl.iTransNum == 0)
	{
		Sapp.bSettled = TRUE;		//�Ѿ�����
	}
	else
	{
		Sapp.bSettled = FALSE;		//δ����
	}

	//���һ�������
	TmsMakeAppParam(ParamBuf, &ParamLen, &ParamNum, &Flag);
	Sapp.AppParamLen = ParamLen;
	memcpy((char *)Sapp.AppParam, (char *)ParamBuf, Sapp.AppParamLen);
#ifdef _ZYL_DEBUG_
	printf("ParamNum:%d\n", ParamNum);
#endif

	memset((char *)Sapp.AppParam2, 0, sizeof(Sapp.AppParam2));
	Sapp.AppParamLen2 = 0;
	if(Flag == TRUE)
	{
		//��ڶ��������
		Flag = FALSE;
		memset((char *)ParamBuf, 0, sizeof(ParamBuf));
		ParamLen = 0;
		TmsMakeAppParam(ParamBuf, &ParamLen, &ParamNum, &Flag);
		Sapp.AppParamLen2 = ParamLen;
		memcpy((char *)Sapp.AppParam2, (char *)ParamBuf, Sapp.AppParamLen2);
	}
	
	memset((char *)Sapp.AppParam3, 0, sizeof(Sapp.AppParam3));
	Sapp.AppParamLen3 = 0;
	if(Flag == TRUE)
	{
		//������������
		Flag = FALSE;
		memset((char *)ParamBuf, 0, sizeof(ParamBuf));
		ParamLen = 0;
		TmsMakeAppParam(ParamBuf, &ParamLen, &ParamNum, &Flag);
		Sapp.AppParamLen3 = ParamLen;
		memcpy((char *)Sapp.AppParam3, (char *)ParamBuf, Sapp.AppParamLen3);
	}
	
	memset((char *)Sapp.AppParam4, 0, sizeof(Sapp.AppParam4));
	Sapp.AppParamLen4 = 0;
	if(Flag == TRUE)
	{
		//������������
		Flag = FALSE;
		memset((char *)ParamBuf, 0, sizeof(ParamBuf));
		ParamLen = 0;
		TmsMakeAppParam(ParamBuf, &ParamLen, &ParamNum, &Flag);
		Sapp.AppParamLen4 = ParamLen;
		memcpy((char *)Sapp.AppParam4, (char *)ParamBuf, Sapp.AppParamLen4);
	}
#ifdef _ZYL_DEBUG_
	printf("��Ӧ�ó����в���2�ĳ���ֵ:%d\n", Sapp.AppParamLen2);
	printf("��Ӧ�ó����в���3�ĳ���ֵ:%d\n", Sapp.AppParamLen3);
	printf("��Ӧ�ó����в���4�ĳ���ֵ:%d\n", Sapp.AppParamLen4);
#endif
	return  PubWritemanageFile(ParamPath, (uint8_t *)&Sapp, 0, sizeof(struct _HF_SAPP_INFO_));
}

static int OpenAndSeek(const uint8_t *filename, int offset)
{
	int fd = -1;

	fd = fileOpen((char *)filename, O_RDWR|O_CREAT);
	if(fd<0)
	{
		return fd;
	}

	if( fileSeek(fd, offset, SEEK_SET) == offset )
	{
		return fd;
	}

	fileClose(fd);
	return -1;
}

static int WriteAndClose(int fd, uint8_t *buf, int len)
{
	int cnt=0;
	int ret=0;
	
	while(cnt<len)
	{
		ret = fileWrite(fd, buf+cnt, len-cnt);
		if(ret == -1)
		{
			fileClose(fd);
			return ret;
		}

		cnt += ret;
	}

	fileClose(fd);
	return OK;
}


static int ReadAndClose(int fd, uint8_t *buf, int len)
{
	int cnt=0;
	int ret=0;
	
	while(cnt<len)
	{
		ret = fileRead(fd, buf+cnt, len-cnt);
		if(ret == -1)
		{
			fileClose(fd);
			return ret;
		}

		cnt += ret;
	}

	fileClose(fd);
	return OK;
}

int PubWritemanageFile(const char *filename, uint8_t *buf, int offset, int len)
{
	int fd = -1;

	//check params
	if(filename==NULL || offset<0 || len<0)
	{
		return -1;
	}
	if(len==0)
		return OK;

	fd = OpenAndSeek((UINT8 *)filename, offset);
	if(fd < 0)
		return fd;
	return WriteAndClose(fd, buf, len);
}


int PubReadManageFile(const char *filename, uint8_t *buf, int offset, int len)
{
	int fd = -1;

	//check params
	if(filename==NULL || offset<0 || len<0)
	{
		return -1;
	}
	if(len==0)
		return OK;

	fd = OpenAndSeek((UINT8*)filename, offset);
	if(fd < 0)
		return fd;

	return ReadAndClose(fd, buf, len);
}

//GB�����ж�
static int IsCnChar(unsigned char ch,unsigned char ch2)
{
	if(	((ch >= 0xA1 && ch <= 0xA9) && (ch2 >= 0xA1 && ch2 <= 0xFE) ) || //GBK1
		((ch >= 0xB0 && ch <= 0xF7) && (ch2 >= 0xA1 && ch2 <= 0xFE) ) || //GBK2 6763
		((ch >= 0x81 && ch <= 0xA0) && (ch2 >= 0x40 && ch2 <= 0xFE) ) || //GBK3
		((ch >= 0xAA && ch <= 0xFE) && (ch2 >= 0x40 && ch2 <= 0xA0) ) || //GBK4
		((ch >= 0xA8 && ch <= 0xA9) && (ch2 >= 0x40 && ch2 <= 0xA0) ))   //GBK5
	{
		return 1;	//����
	}
	else if(((ch >= 0xA1 && ch <= 0xA9) && (ch2 < 0xA1 || ch2 > 0xFE) ) || //����GBK1
		((ch >= 0xB0 && ch <= 0xF7) && (ch2 < 0xA1 || ch2 > 0xFE) ) || //����GBK2 6763
		((ch >= 0x81 && ch <= 0xA0) && (ch2 < 0x40 || ch2 > 0xFE) ) || //����GBK3
		((ch >= 0xAA && ch <= 0xFE) && (ch2 < 0x40 || ch2 > 0xA0) ) || //����GBK4
		((ch >= 0xA8 && ch <= 0xA9) && (ch2 < 0x40 || ch2 > 0xA0) ))   //����GBK5
	{
		return 0;	//��һ���ֽ��Ǻ��ֵ�ǰһ�룬�ڶ����ֽڲ��Ǻ��ֵ���һ��
	}
	else
		return 2;
}


#define LINE_CHAR 32
int format_info(char *in, char out[][LINE_CHAR])
{
    int i,j,cnt,len;
    
    if(in==NULL || out==NULL)
        return -1;
    
    len = strlen(in);
    if(len == 0)
        return 0;
    
    cnt = 0;
    j = 0;
    for(i=0; i<len; i++)
    {
        if(in[i]=='\r' || in[i]=='\n')
        {
            while(in[i+1]=='\r' || in[i+1]=='\n') i++;  //�������������
            cnt++;
            j = 0;
            continue;
        }
        else if(IsCnChar((uint8_t)in[i],(uint8_t)in[i+1])==1)   //����
        {
            if(j+1 >= LINE_CHAR)     //һ�������ַ�
            {
                cnt++;
                j = 0;
            }
            out[cnt][j++] = in[i++];
            out[cnt][j++] = in[i];
            continue;
        }
        else if(j+1 >= LINE_CHAR)     //һ�������ַ�
        {
            cnt++;
            j = 0;
        }
        out[cnt][j++] = in[i];
    }
    
    if(j > 0)
        cnt++;
    return cnt;
}

uint8_t SelectDownloadExit(int flag)
{
    int i,cnt,iKey,page,arrow;
    uint8_t ret;
    
    char ver_info[32][LINE_CHAR];
    
    memset(ver_info, 0, sizeof(ver_info));
    cnt = format_info(tms_info.ver_info, ver_info);
    if(cnt < 0)
        return 0;
    
    #ifdef ALIPAY_BUG
    printf("format_info()=%d\n",cnt);
    for(i=0; i<cnt;i++)
    {
        printf("%s\n",ver_info[i]);
    }
    #endif
    
    page = (cnt+3)/4;
    
    //if(strlen(tms_info.ver_info) > 0)   //�а汾��Ϣ
    //{
        lcdCls();
        lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VERSION UPDATE");
        lcdDisplay(0, 10, DISP_CFONT|DISP_MEDIACY, "PRESS [ENTER]");
        lcdFlip();
    //}
    /*else
    {
        lcdCls();
        lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "�������");
        lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "Ӧ�ó�����Ҫ����");
        lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "[ȷ��]�� ����");
        if(flag==0)
            lcdDisplay(0, 8, DISP_CFONT|DISP_MEDIACY, "[ȡ��]�� �˳�");
        lcdFlip();
    }*/
    
    i = 0;
    arrow = 1;  //����
    while(1)
    {
        lcdClrLine(2,9);
        lcdDisplay(0, 2, DISP_CFONT, ver_info[i]);
        lcdDisplay(0, 4, DISP_CFONT, ver_info[i+1]);
        lcdDisplay(0, 6, DISP_CFONT, ver_info[i+2]);
        lcdDisplay(0, 8, DISP_CFONT, ver_info[i+3]);
        if(i > 0)
        {
            lcdSetIcon(ICON_UP, OPENICON);
        }
        else
        {
            lcdSetIcon(ICON_UP, CLOSEICON);
        }
        
        if(i+4 < cnt)
        {
            lcdSetIcon(ICON_DOWN, OPENICON);
        }
        else
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
        }
        
        lcdFlip();
        kbFlush();
        iKey = kbGetKeyMs(60000);//1����
        if(iKey == KEY_ENTER)
        {
            ret = E_APP_RESET;
            break;
        }
        else if(flag==0 && iKey==KEY_CANCEL)
        {
            //DispMainScreenForAlipay();
            ret = NO_DISP;
            break;
        }
        else if(iKey==KEY_DOWN)
        {
            arrow = 1;
            if(i+4 < cnt)
                i += 4;
            else
                ;
        }
        else if(iKey==KEY_UP)
        {
            arrow = 0;
            if(i>=4)
                i -= 4;
        }
    }
    
    lcdSetIcon(ICON_DOWN, CLOSEICON);
    lcdSetIcon(ICON_UP,   CLOSEICON);
    return ret;
}

uint8_t DealTmsFunc(int update_now)
{
	uint8_t	ucRet;
	int ret=0;

    ret = tms_process(update_now);
//  printf("******** tms_process(%d)=%d\n",update_now,ret);
    
    if(ret <= 0)
        ucRet = NO_DISP;
	else
	    ucRet = ret;
	return ucRet;
}

//�ж��Ƿ���Ҫ�������ط����ն˲���
//����ֵ: E_APP_EXIT - ��Ҫ���ǣ��˳���Ӧ��  0- ����Ҫ����
uint8_t CheckManageRegesit(void)
{
    if(stPubAppInfo.cRegistRslt != SUCC)            //�ж��Ƿ��Ѳ��ǳɹ�
    {
        //if(stPosParam.ucRegesitedFlag == FALSE)   //�ж��Ƿ���Ӧ�õ��ù�����
        {
            //stPosParam.ucRegesitedFlag = TRUE;    //���𲹵ǣ����õ��ò��Ǳ�־λΪTRUE����Ϊ�ڳɹ�ǩ�����û�
            //SaveAppParam();
            stPubAppInfo.cRegistRslt = UNEXE;       //�ù����ļ��в��ǽ��ΪUNEXE,��־Ҫ������ִ�в��ǣ���λ����������λ
            WritePubInfoFile();
            return E_APP_EXIT;                      //�˳���Ӧ�ã���������
        }
    }
    return 0;
}

void TmsDealExitManageAppComm(void)
{
    int iRspLen = 0;
	int iLen;
    char szRspData[256];

	if(stPosParam.stTxnCommCfg.ucCommType != CT_CDMA  
		&& stPosParam.stTxnCommCfg.ucCommType != CT_GPRS)
	{
		CommOnHook(TRUE);
	}
	//����ͨѶ״̬������
	szRspData[iRspLen++] = 0xA5;
	szRspData[iRspLen++] = 0xF0;
	iLen = strlen((char *)stPosParam.stTxnCommCfg.stWirlessPara.szAPN)+2;
	sprintf(&szRspData[iRspLen], "%03d", iLen);
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
	memcpy(&szRspData[iRspLen], stPosParam.stTxnCommCfg.stWirlessPara.szAPN, strlen((char *)stPosParam.stTxnCommCfg.stWirlessPara.szAPN));
	iRspLen += strlen((char *)stPosParam.stTxnCommCfg.stWirlessPara.szAPN);
	write(giWriteFd, szRspData, iRspLen);
}

void TmsWriteFdAndExit(char *pszResp, int iRespLen)
{
	close(giReadFd);
	SaveAppTmsParam();
	TmsDealExitManageAppComm();
	exit(0);

}

int param_change_allowed(void)
{
	return 2;
}

// end of file
