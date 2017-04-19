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

#define FILE_PUB_APP_INFO  "pub_app.info"  //全部应用共用信息文件
#define	FILE_SAPP_INFO	   "PB_SAPP.INFO"   //add wgz 共用的文件，放在根目录下
#define FILE_TMS_LOG      "HF_Tms_Ctrl.log"	//add wgz
#define FILE_POS_TMS_LOG  "SysParam.log"
#define FILE_SWITCH_LOG   "HF_Switch.log"	//add wgz
#define FILE_TMP_MANAGE_COMM   "Temp_comm.info" //子应用下载的主控通讯临时文件

extern const APPINFO AppInfo;
extern struct _NEWPOS_PARAM_STRC stPosParam;  
extern  char gszParamFileName[50];      //  参数文件名

#define SWITCH_TYPE_APP            0x01		//从一个应用切换别的应用
#define SWITCH_TYPE_MANAGE         0x02		//从应用切换主控
//子应用交易触发切换,pos收到FF则自动更新
#define SWITCH_TYPE_AUTO_UPDATE    0x04		
#define SWITCH_TYPE_UPDATEPARAM    0x08		//切换到子应用更新参数
#define SWITCH_TYPE_REGISTE        0x10		//切换到主控进行补登

//tms 需要处理的东西
#define TMS_NEED_UPDATE             0x01     //需要更新
#define TMS_NEED_DOWNCONFIRM        0x02     //需要下载确认
#define TMS_NEED_REGIST             0x03     //需要补登
#define TMS_NEED_REDOWN                0x04    //需要重新下载
#define TMS_NEED_GOTO_MANAGE         0x05     //需要进入主控
//end

#define	LEN_REPORT_DATETIME	     14

#define TMS_TIMER_NO                9
#define TMS_TIMER_MS	            5*60*1000
#define E_NEED_INTO_MANAGE         53  //需要进入主控

#ifndef ERR_BASE
#define ERR_BASE			0x10000
#endif

#define ERR_FILEOPER		(ERR_BASE+0x100)
#define ERR_NOT_FILE		(ERR_BASE+0x101)


//tms下下来的标签参数 
#define TMS_DOWN_LABLE		         "TMSDOWNLABLE"			
struct _MULTI_APP_PARAM {
	char tag[10];
	char value[82];
	char rfu[20];
}; 

//tms主控参数
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
	//add wgz	分开存贮，考虑到tms上 lan，gprs，cdma和wifi地址有可能配置不一样
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
	
	uint8_t szExtNum[10];	 //外线号码
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
	uint8_t ucDefaltTxn;// 默认交易类型
	uint8_t ucVoidSwipe;	
	uint8_t ucAuthVoidSwipe;	
	uint8_t ucConfirm;	
	uint8_t ucVoidPin;	
	uint8_t ucPreVoidPin;
	uint8_t ucPreComVoidPin;
	uint8_t ucPreComPin;  
	uint8_t ucKeyMode;    
	uint8_t ucqPbocSupport;      // 是否支持Qpboc
	uint8_t ucPartPaySupport;    // 支持部分扣款
	uint8_t ucSelectCardType;	
	uint8_t szParamsPwd[9];   
	uint8_t szSafepwd[9];//系统管理员密码
	uint8_t szTpdu[11];       
	uint8_t szTransCtl[9]; 
	uint8_t bPreDial;       // 是否预拨号
	uint8_t szDownloadTel[20]; 
	uint8_t szDownLoadTID[9]; 
	uint8_t ucDetailPrt;   
	uint8_t szMerBankId[5];
	uint8_t szLocalId[5];
	uint8_t ucPrnEng; 
	uint8_t ucMaxOfflineTxn;  // 满足自动上送的累计笔数
	uint8_t ucOfflineSettleFlag;  // 小额代授权的结算是是否否开放
	uint8_t ucPrnTitleFlag; 
	uint8_t szPrnTitle[60];       // 打印票据的抬头
	uint8_t szAsyncTpdu[11];
	uint8_t ucTrackEncrypt;   // 是否加密磁道
	uint8_t szMaxRefundAmt[13];
	uint8_t szMaxTransAmt[13];  //最大交易金额

	
	int  uiRever;     // 为了对齐保留
	int ucTicketNum;	
	int ucOprtLimitTime;  
	int ucCommWaitTime; 
	int ucAdjustPercent;	
	int ucResendTimes;	      // 冲正重发次数
	int ucNotifyTimes;	      // 通知重发次数
	int ucGprsOnLine;
	int ucCheckTimes;	      // 通知重发次数
	int ucDialRetryTimes;	
	int ucKeyIndex;		
	int iMaxTransTotal;  
	int lNowTraceNo; 	
	int lNowBatchNum;	

	int iEcMaxAmount;       // 电子终端电子现金交易限额
	int iqPbocMinAmt;      // 非接触式脱机最低限额
	int iqPbocMaxAmt;      // 非接触式交易限额
	
	int iDispLanguage;   // 显示语言
	int iCommType;       // 1-脱机 2-串口 3-联机无线
	
	ST_COMM_CONFIG		stTxnCommCfg;	        // 终端通讯配置 －－
	ST_WIFI_AP_INFO     stWIFIAPInfo;           // wifi AP 配置信息 目前只考虑一台机器只连接一个热点
	ST_TMS_MANAGE_PARA  stTmsManageInfo;

	uint8_t ucSupPinpad;   // 是否支持PINPAD
	uint8_t ucSupQpass;    //1表示外置非接读卡器，0表示内置
	uint8_t ucPinpadID[2]; 
	uint8_t ucTelIndex;    // 当前首选电话号码的索引 初始化为0, 如果在实际使用中，某个索引的电话号码拨成功了，那么记录下来索引，下一次从这个开始
	uint8_t ucTCPIndex;    // 当前首选IP的索引 初始化为0, 如果在实际使用中，某个索引的IP拨成功了，那么记录下来索引，下一次从这个开始
	uint8_t szREFU[200];   //备用，由于远程下载的时候为了保证参数不变，不能改结构体，所以加一些字节备用

	UINT8 szOrganNum[8 + 1];			//tms机构号码 add wgz
	uint8_t bIfDownAppSuccess;			//add wgz 
	struct _MULTI_APP_PARAM staTagValue[40];
	
}POS_PARAM_STRC;
#endif

typedef struct _UPDATE_TRANS_OPEN_FLAG_
{
	uint8_t EcQuickSaleFlag;		//是否支持小额支付
	uint8_t EcQueFlag;				//是否支持电子现金查余
	uint8_t EcTopUpSpecFlag;		//是否支持指定帐户圈存
	uint8_t EcTopUpNormalFlag;		//是否支持非指定帐户圈存
	uint8_t EcTopUpCashFlag;		//是否支持现金充值 
	uint8_t PosQueFlag;				//是否支持查询
	uint8_t PosAuthFlag;			//是否支持预授权
	uint8_t PosAuthVoidFlag;		//是否支持预授权撤销
	uint8_t PosAuthCmpOnlineFlag;	//是否支持完成联机
	uint8_t PosAuthCmpVoidFlag;		//是否支持完成撤销
	uint8_t PosSaleFlag;			//是否支持消费
	uint8_t PosSaleVoidFlag;		//是否支持消费撤销
	uint8_t PosRefundFlag;			//是否支持退货
	uint8_t PosOffSaleFlag;			//是否支持离线结算
	uint8_t PosOffAdjustFlag;		//是否支持离线结算调整
	uint8_t PosAuthCmpOfflineFlag;	//是否支持预授权完成离线
	uint8_t PosAuthAddFlag;			//预授权追加
}_UPDATE_TRANS_OPEN_FLAG_;

//lilt 2014.08.18 11:4 中汇TMS参数模板解析用结构
typedef struct _UPDATE_TMS_PARAMS_
{
	uint8_t szTransOpenFlag[20+1];			//交易控制开关
	uint8_t szTransPinFlag[4+1];			//交易是否输卡密码,1是，0否,第一位：消费撤销,第二位：预授权撤销,第三位：预授权完成撤销,第四位：预授权完成（请求),其他交易均需要输卡密码
	uint8_t szTransVoidSwipe[2+1];				//是否需要刷卡,1是，0否,第一位：消费撤销,第二位：预授权完成撤销
	uint8_t szSettleCtrl[2+1];					//结算相关控制,1是，0否,第一位：自动签退,第二位：打印明细
	uint8_t szOffLineCrtl[2+1];				//离线交易相关控制,第一位：上送方式,1联机前，0结算前,第二位：上送次数(1-9)
	uint8_t ucAuthMaskPan;				//预授权交易是否屏蔽卡号
	uint8_t szManualInput[3+1];				//是否手输卡号（1允许，0禁止）。第一位:消费类；第二位：预授权类；第三位：退货。
	
}_UPDATE_TMS_PARAMS_;
struct _HF_SAPP_INFO_
{
	uint8_t bSettled;		//已经结算标志, TRUE-已经结算, FLASE-未结算
	uint8_t   merchantId[16];	//商户号
	uint8_t   termId[9];			//终端号
	//以下参数非必须填充
	//填充后
	//应用名与版本号同样为可配置信息
	uint8_t   appName[16];		//应用名,该应 用名为空的话, mapp将取应用信息做为该应用名
	uint8_t   appVer[16];		//应用版本,该版本号为空时, mapp将取应用版本
	uint8_t   AppParam[630];		//增加一个子应用的参数 add wgz
	uint16_t  AppParamLen;			//子应用参数的长度 add wgz
	uint8_t   AppParam2[630];//第二组子应用的参数 
    uint16_t  AppParamLen2;			//第二组参数长度
	uint8_t   AppParam3[630];				//
	uint16_t  AppParamLen3;					//
	uint8_t   AppParam4[630];				//保留，暂未处理3个数据包情况
	uint16_t  AppParamLen4;					//保留，暂未处理3个数据包情况
//	  uint8_t	ucRegisterFlag; //是否补登成功标志,PARAM_OPEN---补登成功,PARAM_COLSE---未补登成功
//	uint8_t   iRegisterNum; 	  //补登次数
	uint8_t   AutoReconnectTime[4+1];		//自动重拨时间间隔
	uint8_t   ucUpdateParamFlag;			//参数变动,是否需要参数上送标志,PARAM_CLOSE--不需要,PARAM_OPEN--需要
	int       iUpdateParamNum;              //参数上送次数	
	uint8_t   szUpdateParamTime[14+1];		//上一次参数上送时间

	uint8_t   Rfu[500];

};

typedef struct ST_TMP_MANAGE_COMM_PARAM
{//lilt 2014.08.31 11:26 中汇支付，按银联修改，其中
 //26格式由N8变为AN10
 //30格式由 当编码27：下载内容标志为00时，格式为N6变为ANS15
	uint8_t szTMSIdNum[10+1];        					//26 TMS后台运营机构的标识码
	uint8_t szDowaLoadContentFlag[2+1];      //27 下载内容标志
	uint8_t szDowaLoadTaskId[4+1];  				//28 下载任务标识
	uint8_t szLimitTime[8+1];  							//29 限制日期
	uint8_t szAppVersion[50+1];  						//30 应用版本号
	uint8_t szDownLoadTimeFlag[4+1];				//31 POS下载时机标志	1001-批结后提示下载	9001-立即下载（强制先进行批结算）
	uint8_t szPhoneNum1[20+1];      					//TMS同步电话号码1
	uint8_t szPhoneNum2[20+1];      					//TMS同步电话号码2
	uint8_t szPhoneNum3[20+1];      					//TMS异步电话号码1
	uint8_t szPhoneNum4[20+1];      					//TMS异步电话号码2
	uint8_t szTcpParam[30+1];       					//34 TMS的IP和端口号1
	uint8_t szTcpParam2[30+1];       					//35 TMS的IP和端口号1
	uint8_t szGprsParam[60+1];      					//36 TMS的GPRS参数
	uint8_t szCdmaParam[60+1];     					//37 TMS的CDMA接入方式
	uint8_t szTaskCheckCode[32+1];     			//38 下载任务校验码
	uint8_t szAutoReconnectTime[4+1];			//39 自动重拨间隔时间
	uint8_t szTaskInformation[30+1];     			//40 任务提示信息
	uint8_t szTpdu[10+1];									//41 TPDU
	
}ST_TMP_MANAGE_COMM_PARAM;


typedef struct _TMS_PARAM_ {
//自动信息上送失败总共重发次数,
//和已经发送的次数
    uint8_t  ResendTotalTimes;
	uint8_t  ReportCurrentSendTimes;
	uint8_t  DownCurrentSendTimes;
	uint8_t  ReportFailFlag;
	uint8_t  DownFailFlag;
	ulong ulAutoRedialTime;      //add andy.wang 自动重拨间隔时间 
	
	//以下变量为新增变量
	uint8_t  szReportStartTime[LEN_REPORT_DATETIME+1];	//信息上送开始时间:YYYYMMDDHHMMSS
	uint8_t  szReportEndTime[LEN_REPORT_DATETIME+1];	//信息上送截止时间:YYYYMMDDHHMMSS

	uint8_t  szReportStartLinkTime[LEN_REPORT_DATETIME+1]; //信息上送后台计划实际开始时间:YYYYMMDDHHMMSS
	uint8_t  szReportEndLinkTime[LEN_REPORT_DATETIME+1];	 //信息上送后台计划实际结束时间:YYYYMMDDHHMMSS
	
	uint8_t  szDownloadStartTime[LEN_REPORT_DATETIME+1];//联机下载开始时间:YYYYMMDDHHMMSS
	uint8_t  szDownloadEndTime[LEN_REPORT_DATETIME+1];	  //联机下载截止时间:YYYYMMDDHHMMSS
	
	uint8_t  szUpdateTime[LEN_REPORT_DATETIME+1];	
	uint32_t uiReportPeriod;	  //间隔周期,单位:天
	uint8_t	 bNeedDownloadConfirm;  //下载完成后需要发送下载确认报文
	uint8_t	 bNeedUpdateApp;		  //下载完成后需要更新应用
	uint8_t  bNeedRegistTerm;		  //是否需要进行补登交易
	uint8_t  bNeedSappLogout;			//更新完毕后,需要子应用签退状态  add andy.wang 2013-10-7 10:28:33
	uint8_t  ucretrytimes;			  //错误重试的次数

}	TMS_PARAM;

//主控参数结构体
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
	//add wgz	分开存贮，考虑到tms上 lan，gprs，cdma和wifi地址有可能配置不一样
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
	
	uint8_t szExtNum[10];	 //外线号码
	uint8_t sTpdu[6];		  	
	uint8_t szParamsPwd[9];   
		
	int ucOprtLimitTime;  
	int ucCommWaitTime; 	
	int ucDialRetryTimes;		 
	
	int iDispLanguage;   // 显示语言
	int iCommType;       // 1-脱机 2-串口 3-联机无线
	
	ST_COMM_CONFIG		stTxnCommCfg;	        // 终端通讯配置 －－
	ST_WIFI_AP_INFO     stWIFIAPInfo;           // wifi AP 配置信息 目前只考虑一台机器只连接一个热点
//	ST_TMS_MANAGE_PARA  stTmsManageInfo;

	uint8_t ucTelIndex;    // 当前首选电话号码的索引 初始化为0, 如果在实际使用中，某个索引的电话号码拨成功了，那么记录下来索引，下一次从这个开始
	uint8_t ucTCPIndex;    // 当前首选IP的索引 初始化为0, 如果在实际使用中，某个索引的IP拨成功了，那么记录下来索引，下一次从这个开始
	uint8_t szREFU[200];   //备用，由于远程下载的时候为了保证参数不变，不能改结构体，所以加一些字节备用

	uint8_t szOrganNum[8 + 1];			//tms机构号码 add wgz
	struct _MULTI_APP_PARAM staTagValue[40];
	
}POS_TMS_PARAM_STRC;

#define  STR_PARAMS(A)   (uint8_t*)A,	sizeof(A)
#define  CHAR_PARAMS(A)  (uint8_t*)&A,	sizeof(char)
#define  INT_PARAMS(A)	  (uint8_t*)&A,	sizeof(int)
#define  LONG_PARAMS(A)  (uint8_t*)&A,	sizeof(int)
#define  NOTUSER_PARAM	  NULL,   0,   0

#define  SPECIAL_PARAM   NULL,   1,   0

#define  STRING_ITEM	  0x00	//保证最后一位为0x00;
#define  ASSCII_ITEM     0x01	
#define  HEX_ITEM	      0x02	
#define  BIN_ITEM	      0x03
#define  BCD_ITEM	      0x04
#define  OPTION_ITEM     0x08    //add andy.wang 2013-9-26 13:42:38 for tms

#define SYNC_COMMTYPE     	0x04          //同步类型
#define SYNC_COMMAPN      	0x02
#define SYNC_COMMIP1	 	0x01
#define SYNC_COMMIP2		0x08
#define SYNC_ALL			0x0F			//全部同步

//add andy.wang 2013-9-26 14:17:57 for tms
#define TMS_CHK_SMALL_FONT    			"0380"
#define TMS_CHK_SALE_MANUL    			"0101"
#define TMS_CHK_VOID_MANUL    			"0104"
#define TMS_CHK_VOID_PWD	    		"0002"
#define TMS_CHK_ENABLE_AUTH_PSW  		"0404"
#define TMS_CHK_ENABLE_GPRS_NOTONLINE	"0408"  // GPRS长连接--默认为长连接

/*重复定义，在PosParams.h中已定义
typedef struct
{
	uint8_t name[40];
	void *ini_ptr;
	uint8_t len;
	uint8_t type;
}PARAMS;
*/


// tms一些参数
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
