#include "posparams.h"

#ifndef _SAPPTMS_H
#define _SAPPTMS_H
#define TMS_DEBUG


#ifdef TMS_DEBUG
	#define PrintDebug(fmt,arg...) printf("[%d,%s,%s]" fmt "\n",__LINE__,__FILE__,__FUNCTION__,##arg)
#else
	#define PrintDebug(fmt,arg...) do {} while(0)
#endif

#define	LEN_REPORT_DATETIME	14

#define FILE_PUB_APP_INFO  "pub_app.info"  //ȫ��Ӧ�ù�����Ϣ�ļ�
#define	FILE_SAPP_INFO	   "PB_SAPP.INFO"   //add wgz ���õ��ļ������ڸ�Ŀ¼��
#define FILE_TMS_LOG      "HF_Tms_Ctrl.log"	//add wgz
#define FILE_POS_TMS_LOG  "SysParam.log"
#define FILE_SWITCH_LOG   "HF_Switch.log"	//add wgz
#define FILE_TMP_MANAGE_COMM   "Temp_comm.info" //��Ӧ�����ص�����ͨѶ��ʱ�ļ�

extern const APPINFO AppInfo;
extern struct _NEWPOS_PARAM_STRC stPosParam;  
extern  char gszParamFileName[50];      //  �����ļ���

#define SWITCH_TYPE_APP            0x01		//��һ��Ӧ���л����Ӧ��
#define SWITCH_TYPE_MANAGE         0x02		//��Ӧ���л�����
//��Ӧ�ý��״����л�,pos�յ�FF���Զ�����
#define SWITCH_TYPE_AUTO_UPDATE    0x04		
#define SWITCH_TYPE_UPDATEPARAM    0x08		//�л�����Ӧ�ø��²���
#define SWITCH_TYPE_REGISTE        0x10		//�л������ؽ��в���

//tms ��Ҫ����Ķ���
#define TMS_NEED_UPDATE             0x01     //��Ҫ����
#define TMS_NEED_DOWNCONFIRM        0x02     //��Ҫ����ȷ��
#define TMS_NEED_REGIST             0x03     //��Ҫ����
#define TMS_NEED_REDOWN                0x04    //��Ҫ��������
#define TMS_NEED_GOTO_MANAGE         0x05     //��Ҫ��������
//end

#define	LEN_REPORT_DATETIME	     14

#define TMS_TIMER_NO                9
#define TMS_TIMER_MS	            5*60*1000
#define E_NEED_INTO_MANAGE         53  //��Ҫ��������

#ifndef ERR_BASE
#define ERR_BASE			0x10000
#endif

#define ERR_FILEOPER		(ERR_BASE+0x100)
#define ERR_NOT_FILE		(ERR_BASE+0x101)


//tms�������ı�ǩ���� 
#define TMS_DOWN_LABLE		         "TMSDOWNLABLE"			
struct _MULTI_APP_PARAM {
	char tag[10];
	char value[82];
	char rfu[20];
}; 

//tms���ز���
typedef struct _tagST_TMS_MANAGE_PARA
{
	int  ulTimeOut;
	int  autoDialTime;
	int ucCommType;
	char szTpdu[13];
	char szModenNum1[80];
	char szModenNum2[80];
	char szTcpIpPara[80];
	char szWifiPara[80];
	char szGprsPara[80];
	char szCdmaPara[80];
	char szPppPara[80];
	char szRs232Para[80];
}ST_TMS_MANAGE_PARA;
#if 0

typedef struct _NEWPOS_PARAM_STRC {
	uint8_t szPhoneNum1[20];
	uint8_t szPhoneNum2[20];
	uint8_t szPhoneNum3[20];
	uint8_t szMPhone[20];


	uint8_t szPOSLocalIP[15+1];
	uint8_t szPOSLocalPort[15+1];
	uint8_t szPOSLocalGate[15+1];
	
	uint8_t szPOSRemoteIP[15+1];
	uint8_t	szPOSRemotePort[5+1];
	uint8_t szPOSRemoteIP2[15+1];
	uint8_t	szPOSRemotePort2[5+1];
	//add wgz	�ֿ����������ǵ�tms�� lan��gprs��cdma��wifi��ַ�п������ò�һ��
	uint8_t szPOSRemoteGprsIp[15+1];
	uint8_t szPOSRemoteGprsPort[5+1];
	uint8_t szPOSRemoteGprsIp2[15+1];
	uint8_t szPOSRemoteGprsPort2[5+1];
	uint8_t szPOSGprsApn[64+1];
	uint8_t szPOSGprsUID[64+1];
	uint8_t szPOSGprsPwd[16+1];

	uint8_t szPOSRemoteCdmaIp[15+1];
	uint8_t szPOSRemoteCdmaPort[5+1];
	uint8_t szPOSRemoteCdmaIp2[15+1];
	uint8_t szPOSRemoteCdmaPort2[5+1];
	uint8_t szPOSCdmaApn[64+1];
	uint8_t szPOSCdmaUID[64+1];
	uint8_t szPOSCdmaPwd[16+1];

	uint8_t szPOSRemoteWifiIp[15+1];
	uint8_t szPOSRemoteWifiPort[5+1];
	uint8_t szPOSRemoteWifiIp2[15+1];
	uint8_t szPOSRemoteWifiPort2[5+1];
	uint8_t szCenterCallNum[41];
	//end wgz
	
	uint8_t szExtNum[10];	 //���ߺ���
	uint8_t szUnitNum[16];
	uint8_t szPosId[9];
	uint8_t ucSendOffFlag;	
	uint8_t sTpdu[6];
	uint8_t ucTipOpen;		
	uint8_t sTransOpen[4];
	uint8_t ucAutoLogoff;	
	uint8_t ucManualInput;
	uint8_t ucErrorPrt;	
	uint8_t szUnitChnName[43];
	uint8_t szEngName[41];
	uint8_t ucTestFlag; 
	uint8_t ucDefaltTxn;// Ĭ�Ͻ�������
	uint8_t ucVoidSwipe;	
	uint8_t ucAuthVoidSwipe;	
	uint8_t ucConfirm;	
	uint8_t ucVoidPin;	
	uint8_t ucPreVoidPin;
	uint8_t ucPreComVoidPin;
	uint8_t ucPreComPin;  
	uint8_t ucKeyMode;    
	uint8_t ucqPbocSupport;      // �Ƿ�֧��Qpboc
	uint8_t ucPartPaySupport;    // ֧�ֲ��ֿۿ�
	uint8_t ucSelectCardType;	
	uint8_t szParamsPwd[9];   
	uint8_t szSafepwd[9];//ϵͳ����Ա����
	uint8_t szTpdu[11];       
	uint8_t szTransCtl[9]; 
	uint8_t bPreDial;       // �Ƿ�Ԥ����
	uint8_t szDownloadTel[20]; 
	uint8_t szDownLoadTID[9]; 
	uint8_t ucDetailPrt;   
	uint8_t szMerBankId[5];
	uint8_t szLocalId[5];
	uint8_t ucPrnEng; 
	uint8_t ucMaxOfflineTxn;  // �����Զ����͵��ۼƱ���
	uint8_t ucOfflineSettleFlag;  // С�����Ȩ�Ľ������Ƿ�񿪷�
	uint8_t ucPrnTitleFlag; 
	uint8_t szPrnTitle[60];       // ��ӡƱ�ݵ�̧ͷ
	uint8_t szAsyncTpdu[11];
	uint8_t ucTrackEncrypt;   // �Ƿ���ܴŵ�
	uint8_t szMaxRefundAmt[13];
	uint8_t szMaxTransAmt[13];  //����׽��

	
	int  uiRever;     // Ϊ�˶��뱣��
	int ucTicketNum;	
	int ucOprtLimitTime;  
	int ucCommWaitTime; 
	int ucAdjustPercent;	
	int ucResendTimes;	      // �����ط�����
	int ucNotifyTimes;	      // ֪ͨ�ط�����
	int ucGprsOnLine;
	int ucCheckTimes;	      // ֪ͨ�ط�����
	int ucDialRetryTimes;	
	int ucKeyIndex;		
	int iMaxTransTotal;  
	int lNowTraceNo; 	
	int lNowBatchNum;	

	int iEcMaxAmount;       // �����ն˵����ֽ����޶�
	int iqPbocMinAmt;      // �ǽӴ�ʽ�ѻ�����޶�
	int iqPbocMaxAmt;      // �ǽӴ�ʽ�����޶�
	
	int iDispLanguage;   // ��ʾ����
	int iCommType;       // 1-�ѻ� 2-���� 3-��������
	
	ST_COMM_CONFIG		stTxnCommCfg;	        // �ն�ͨѶ���� ����
	ST_WIFI_AP_INFO     stWIFIAPInfo;           // wifi AP ������Ϣ Ŀǰֻ����һ̨����ֻ����һ���ȵ�
	ST_TMS_MANAGE_PARA  stTmsManageInfo;

	uint8_t ucSupPinpad;   // �Ƿ�֧��PINPAD
	uint8_t ucSupQpass;    //1��ʾ���÷ǽӶ�������0��ʾ����
	uint8_t ucPinpadID[2]; 
	uint8_t ucTelIndex;    // ��ǰ��ѡ�绰��������� ��ʼ��Ϊ0, �����ʵ��ʹ���У�ĳ�������ĵ绰���벦�ɹ��ˣ���ô��¼������������һ�δ������ʼ
	uint8_t ucTCPIndex;    // ��ǰ��ѡIP������ ��ʼ��Ϊ0, �����ʵ��ʹ���У�ĳ��������IP���ɹ��ˣ���ô��¼������������һ�δ������ʼ
	uint8_t szREFU[200];   //���ã�����Զ�����ص�ʱ��Ϊ�˱�֤�������䣬���ܸĽṹ�壬���Լ�һЩ�ֽڱ���

	UINT8 szOrganNum[8 + 1];			//tms�������� add wgz
	uint8_t bIfDownAppSuccess;			//add wgz 
	struct _MULTI_APP_PARAM staTagValue[40];
	
}POS_PARAM_STRC;
#endif

typedef struct _UPDATE_TRANS_OPEN_FLAG_
{
	uint8_t EcQuickSaleFlag;		//�Ƿ�֧��С��֧��
	uint8_t EcQueFlag;				//�Ƿ�֧�ֵ����ֽ����
	uint8_t EcTopUpSpecFlag;		//�Ƿ�֧��ָ���ʻ�Ȧ��
	uint8_t EcTopUpNormalFlag;		//�Ƿ�֧�ַ�ָ���ʻ�Ȧ��
	uint8_t EcTopUpCashFlag;		//�Ƿ�֧���ֽ��ֵ 
	uint8_t PosQueFlag;				//�Ƿ�֧�ֲ�ѯ
	uint8_t PosAuthFlag;			//�Ƿ�֧��Ԥ��Ȩ
	uint8_t PosAuthVoidFlag;		//�Ƿ�֧��Ԥ��Ȩ����
	uint8_t PosAuthCmpOnlineFlag;	//�Ƿ�֧���������
	uint8_t PosAuthCmpVoidFlag;		//�Ƿ�֧����ɳ���
	uint8_t PosSaleFlag;			//�Ƿ�֧������
	uint8_t PosSaleVoidFlag;		//�Ƿ�֧�����ѳ���
	uint8_t PosRefundFlag;			//�Ƿ�֧���˻�
	uint8_t PosOffSaleFlag;			//�Ƿ�֧�����߽���
	uint8_t PosOffAdjustFlag;		//�Ƿ�֧�����߽������
	uint8_t PosAuthCmpOfflineFlag;	//�Ƿ�֧��Ԥ��Ȩ�������
	uint8_t PosAuthAddFlag;			//Ԥ��Ȩ׷��
}_UPDATE_TRANS_OPEN_FLAG_;

//lilt 2014.08.18 11:4 �л�TMS����ģ������ýṹ
typedef struct _UPDATE_TMS_PARAMS_
{
	uint8_t szTransOpenFlag[20+1];			//���׿��ƿ���
	uint8_t szTransPinFlag[4+1];			//�����Ƿ��俨����,1�ǣ�0��,��һλ�����ѳ���,�ڶ�λ��Ԥ��Ȩ����,����λ��Ԥ��Ȩ��ɳ���,����λ��Ԥ��Ȩ��ɣ�����),�������׾���Ҫ�俨����
	uint8_t szTransVoidSwipe[2+1];				//�Ƿ���Ҫˢ��,1�ǣ�0��,��һλ�����ѳ���,�ڶ�λ��Ԥ��Ȩ��ɳ���
	uint8_t szSettleCtrl[2+1];					//������ؿ���,1�ǣ�0��,��һλ���Զ�ǩ��,�ڶ�λ����ӡ��ϸ
	uint8_t szOffLineCrtl[2+1];				//���߽�����ؿ���,��һλ�����ͷ�ʽ,1����ǰ��0����ǰ,�ڶ�λ�����ʹ���(1-9)
	uint8_t ucAuthMaskPan;				//Ԥ��Ȩ�����Ƿ����ο���
	uint8_t szManualInput[3+1];				//�Ƿ����俨�ţ�1����0��ֹ������һλ:�����ࣻ�ڶ�λ��Ԥ��Ȩ�ࣻ����λ���˻���
	
}_UPDATE_TMS_PARAMS_;
struct _HF_SAPP_INFO_
{
	uint8_t bSettled;		//�Ѿ������־, TRUE-�Ѿ�����, FLASE-δ����
	uint8_t   merchantId[16];	//�̻���
	uint8_t   termId[9];			//�ն˺�
	//���²����Ǳ������
	//����
	//Ӧ������汾��ͬ��Ϊ��������Ϣ
	uint8_t   appName[16];		//Ӧ����,��Ӧ ����Ϊ�յĻ�, mapp��ȡӦ����Ϣ��Ϊ��Ӧ����
	uint8_t   appVer[16];		//Ӧ�ð汾,�ð汾��Ϊ��ʱ, mapp��ȡӦ�ð汾
	uint8_t   AppParam[630];		//����һ����Ӧ�õĲ��� add wgz
	uint16_t  AppParamLen;			//��Ӧ�ò����ĳ��� add wgz
	uint8_t   AppParam2[630];//�ڶ�����Ӧ�õĲ��� 
    uint16_t  AppParamLen2;			//�ڶ����������
	uint8_t   AppParam3[630];				//
	uint16_t  AppParamLen3;					//
	uint8_t   AppParam4[630];				//��������δ����3�����ݰ����
	uint16_t  AppParamLen4;					//��������δ����3�����ݰ����
//	  uint8_t	ucRegisterFlag; //�Ƿ񲹵ǳɹ���־,PARAM_OPEN---���ǳɹ�,PARAM_COLSE---δ���ǳɹ�
//	uint8_t   iRegisterNum; 	  //���Ǵ���
	uint8_t   AutoReconnectTime[4+1];		//�Զ��ز�ʱ����
	uint8_t   ucUpdateParamFlag;			//�����䶯,�Ƿ���Ҫ�������ͱ�־,PARAM_CLOSE--����Ҫ,PARAM_OPEN--��Ҫ
	int       iUpdateParamNum;              //�������ʹ���	
	uint8_t   szUpdateParamTime[14+1];		//��һ�β�������ʱ��

	uint8_t   Rfu[500];

};

typedef struct ST_TMP_MANAGE_COMM_PARAM
{//lilt 2014.08.31 11:26 �л�֧�����������޸ģ�����
 //26��ʽ��N8��ΪAN10
 //30��ʽ�� ������27���������ݱ�־Ϊ00ʱ����ʽΪN6��ΪANS15
	uint8_t szTMSIdNum[10+1];        					//26 TMS��̨��Ӫ�����ı�ʶ��
	uint8_t szDowaLoadContentFlag[2+1];      //27 �������ݱ�־
	uint8_t szDowaLoadTaskId[4+1];  				//28 ���������ʶ
	uint8_t szLimitTime[8+1];  							//29 ��������
	uint8_t szAppVersion[50+1];  						//30 Ӧ�ð汾��
	uint8_t szDownLoadTimeFlag[4+1];				//31 POS����ʱ����־	1001-�������ʾ����	9001-�������أ�ǿ���Ƚ��������㣩
	uint8_t szPhoneNum1[20+1];      					//TMSͬ���绰����1
	uint8_t szPhoneNum2[20+1];      					//TMSͬ���绰����2
	uint8_t szPhoneNum3[20+1];      					//TMS�첽�绰����1
	uint8_t szPhoneNum4[20+1];      					//TMS�첽�绰����2
	uint8_t szTcpParam[30+1];       					//34 TMS��IP�Ͷ˿ں�1
	uint8_t szTcpParam2[30+1];       					//35 TMS��IP�Ͷ˿ں�1
	uint8_t szGprsParam[60+1];      					//36 TMS��GPRS����
	uint8_t szCdmaParam[60+1];     					//37 TMS��CDMA���뷽ʽ
	uint8_t szTaskCheckCode[32+1];     			//38 ��������У����
	uint8_t szAutoReconnectTime[4+1];			//39 �Զ��ز����ʱ��
	uint8_t szTaskInformation[30+1];     			//40 ������ʾ��Ϣ
	uint8_t szTpdu[10+1];									//41 TPDU
	
}ST_TMP_MANAGE_COMM_PARAM;


typedef struct _TMS_PARAM_ {
//�Զ���Ϣ����ʧ���ܹ��ط�����,
//���Ѿ����͵Ĵ���
    uint8_t  ResendTotalTimes;
	uint8_t  ReportCurrentSendTimes;
	uint8_t  DownCurrentSendTimes;
	uint8_t  ReportFailFlag;
	uint8_t  DownFailFlag;
	ulong ulAutoRedialTime;      //add andy.wang �Զ��ز����ʱ�� 
	
	//���±���Ϊ��������
	uint8_t  szReportStartTime[LEN_REPORT_DATETIME+1];	//��Ϣ���Ϳ�ʼʱ��:YYYYMMDDHHMMSS
	uint8_t  szReportEndTime[LEN_REPORT_DATETIME+1];	//��Ϣ���ͽ�ֹʱ��:YYYYMMDDHHMMSS

	uint8_t  szReportStartLinkTime[LEN_REPORT_DATETIME+1]; //��Ϣ���ͺ�̨�ƻ�ʵ�ʿ�ʼʱ��:YYYYMMDDHHMMSS
	uint8_t  szReportEndLinkTime[LEN_REPORT_DATETIME+1];	 //��Ϣ���ͺ�̨�ƻ�ʵ�ʽ���ʱ��:YYYYMMDDHHMMSS
	
	uint8_t  szDownloadStartTime[LEN_REPORT_DATETIME+1];//�������ؿ�ʼʱ��:YYYYMMDDHHMMSS
	uint8_t  szDownloadEndTime[LEN_REPORT_DATETIME+1];	  //�������ؽ�ֹʱ��:YYYYMMDDHHMMSS
	
	uint8_t  szUpdateTime[LEN_REPORT_DATETIME+1];	
	uint32_t uiReportPeriod;	  //�������,��λ:��
	uint8_t	 bNeedDownloadConfirm;  //������ɺ���Ҫ��������ȷ�ϱ���
	uint8_t	 bNeedUpdateApp;		  //������ɺ���Ҫ����Ӧ��
	uint8_t  bNeedRegistTerm;		  //�Ƿ���Ҫ���в��ǽ���
	uint8_t  bNeedSappLogout;			//������Ϻ�,��Ҫ��Ӧ��ǩ��״̬  add andy.wang 2013-10-7 10:28:33
	uint8_t  ucretrytimes;			  //�������ԵĴ���

}	TMS_PARAM;

//���ز����ṹ��
typedef struct _NEWPOS_TMS_PARAM_STRC {
	uint8_t szPhoneNum1[20];
	uint8_t szPhoneNum2[20];
	uint8_t szPhoneNum3[20];
	uint8_t szMPhone[20];


	uint8_t szPOSLocalIP[15+1];
	uint8_t szPOSLocalPort[15+1];
	uint8_t szPOSLocalGate[15+1];
	
	uint8_t szPOSRemoteIP[15+1];
	uint8_t	szPOSRemotePort[5+1];
	uint8_t szPOSRemoteIP2[15+1];
	uint8_t	szPOSRemotePort2[5+1];
	//add wgz	�ֿ����������ǵ�tms�� lan��gprs��cdma��wifi��ַ�п������ò�һ��
	uint8_t szPOSRemoteGprsIp[15+1];
	uint8_t szPOSRemoteGprsPort[5+1];
	uint8_t szPOSRemoteGprsIp2[15+1];
	uint8_t szPOSRemoteGprsPort2[5+1];
	uint8_t szPOSGprsApn[64+1];
	uint8_t szPOSGprsUID[64+1];
	uint8_t szPOSGprsPwd[16+1];

	uint8_t szPOSRemoteCdmaIp[15+1];
	uint8_t szPOSRemoteCdmaPort[5+1];
	uint8_t szPOSRemoteCdmaIp2[15+1];
	uint8_t szPOSRemoteCdmaPort2[5+1];
	uint8_t szPOSCdmaApn[64+1];
	uint8_t szPOSCdmaUID[64+1];
	uint8_t szPOSCdmaPwd[16+1];

	uint8_t szPOSRemoteWifiIp[15+1];
	uint8_t szPOSRemoteWifiPort[5+1];
	uint8_t szPOSRemoteWifiIp2[15+1];
	uint8_t szPOSRemoteWifiPort2[5+1];
	//end wgz
	
	uint8_t szExtNum[10];	 //���ߺ���
	uint8_t sTpdu[6];		  	
	uint8_t szParamsPwd[9];   
		
	int ucOprtLimitTime;  
	int ucCommWaitTime; 	
	int ucDialRetryTimes;		 
	
	int iDispLanguage;   // ��ʾ����
	int iCommType;       // 1-�ѻ� 2-���� 3-��������
	
	ST_COMM_CONFIG		stTxnCommCfg;	        // �ն�ͨѶ���� ����
	ST_WIFI_AP_INFO     stWIFIAPInfo;           // wifi AP ������Ϣ Ŀǰֻ����һ̨����ֻ����һ���ȵ�
//	ST_TMS_MANAGE_PARA  stTmsManageInfo;

	uint8_t ucTelIndex;    // ��ǰ��ѡ�绰��������� ��ʼ��Ϊ0, �����ʵ��ʹ���У�ĳ�������ĵ绰���벦�ɹ��ˣ���ô��¼������������һ�δ������ʼ
	uint8_t ucTCPIndex;    // ��ǰ��ѡIP������ ��ʼ��Ϊ0, �����ʵ��ʹ���У�ĳ��������IP���ɹ��ˣ���ô��¼������������һ�δ������ʼ
	uint8_t szREFU[200];   //���ã�����Զ�����ص�ʱ��Ϊ�˱�֤�������䣬���ܸĽṹ�壬���Լ�һЩ�ֽڱ���

	uint8_t szOrganNum[8 + 1];			//tms�������� add wgz
	struct _MULTI_APP_PARAM staTagValue[40];
	
}POS_TMS_PARAM_STRC;

#define  STR_PARAMS(A)   (uint8_t*)A,	sizeof(A)
#define  CHAR_PARAMS(A)  (uint8_t*)&A,	sizeof(char)
#define  INT_PARAMS(A)	  (uint8_t*)&A,	sizeof(int)
#define  LONG_PARAMS(A)  (uint8_t*)&A,	sizeof(int)
#define  NOTUSER_PARAM	  NULL,   0,   0

#define  SPECIAL_PARAM   NULL,   1,   0

#define  STRING_ITEM	  0x00	//��֤���һλΪ0x00;
#define  ASSCII_ITEM     0x01	
#define  HEX_ITEM	      0x02	
#define  BIN_ITEM	      0x03
#define  BCD_ITEM	      0x04
#define  OPTION_ITEM     0x08    //add andy.wang 2013-9-26 13:42:38 for tms

#define SYNC_COMMTYPE     	0x04          //ͬ������
#define SYNC_COMMAPN      	0x02
#define SYNC_COMMIP1	 	0x01
#define SYNC_COMMIP2		0x08
#define SYNC_ALL			0x0F			//ȫ��ͬ��

//add andy.wang 2013-9-26 14:17:57 for tms
#define TMS_CHK_SMALL_FONT    			"0380"
#define TMS_CHK_SALE_MANUL    			"0101"
#define TMS_CHK_VOID_MANUL    			"0104"
#define TMS_CHK_VOID_PWD	    		"0002"
#define TMS_CHK_ENABLE_AUTH_PSW  		"0404"
#define TMS_CHK_ENABLE_GPRS_NOTONLINE	"0408"  // GPRS������--Ĭ��Ϊ������

/*�ظ����壬��PosParams.h���Ѷ���
typedef struct
{
	uint8_t name[40];
	void *ini_ptr;
	uint8_t len;
	uint8_t type;
}PARAMS;
*/


// tmsһЩ����
typedef struct
{
	uint8_t Tag[9];
	void *tms_ptr;
	uint8_t len;
	uint8_t type;
} TMS_APP_PARAMS;

extern char gszManageVersion[256];
#ifdef __cplusplus
extern "C" {
#endif 


//int  TxtToEnv(void);
uint8_t PrintData(void);
int SaveAppTmsParam(void);
int PubReadManageFile(const char *filename, uint8_t *buf, int offset, int len);
int PubWritemanageFile(const char *filename, uint8_t *buf, int offset, int len);
int ReadPubInfoFile(void);
int WritePubInfoFile(void);
int NeedAutoInspection(void);
int NeedDownloadApp(void);
uint8_t SelectReportExit(void);
uint8_t SelectDownloadExit(int falg);
uint8_t DealTmsFunc(int update_now);
uint8_t CheckManageRegesit(void);
int LoadAppTmsFile(void);
void TmsDealExitManageAppComm(void);
void TmsWriteFdAndExit(char *pszResp, int iRespLen);


#ifdef __cplusplus
}
#endif 

#endif	
// end of file
