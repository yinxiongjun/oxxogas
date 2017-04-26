
#ifndef _NEWPOS_TERSTRUC_H
#define _NEWPOS_TERSTRUC_H

#include "posapi.h"
#include "posmacro.h"
#include "st8583.h"
#include "Commlib.h"

typedef struct _POS_8583PACKET_STRC {
	int  iTransNo;			    
	uint8_t  szSevPointCode[2+1];		 
	uint8_t  szMessTypeCode[2+1];		    
	uint8_t  szNetManInfoCode[3+1];	
} NEWPOS_8583PACKET_STRC;

#define   TS_NOT_RECV    0x02    // sent but no receive
#define   TS_NOT_CODE    0x03    // response code is not 00
#define   TS_INST_SPAC   0x01    // initial instal not print space
#define   TS_FEE_SPAC    0x02    // each instal not print space

#define   TS_ELECSIGN_HAVE   0x20    // have elecsign
#define   TS_ELECSIGN_UP     0x02    // need to upload

#define  SCREEN_TYPE_128_64         0x0000
#define  SCREEN_TYPE_320_240        0x0001

typedef struct   POS_LOG_STRUCT{
	uint8_t	ucBatchSendFlag;       
	uint8_t   ucVoidId; 
	uint8_t 	ucAuthCmlId;   // Ԥ��Ȩ��ɱ�־
	uint8_t	ucSendFlag;
	uint8_t	ucSendFail;
	uint8_t	ucAdjusted;	
	uint8_t 	ucChgFrmAdjust; 

	uint8_t   ucInstalAmtFlag;
	uint8_t   sAmount[6];
	uint8_t   sLiters[6];//����
	long      lTipAmount;
	uint8_t   sTipAmount[6];
	uint8_t   sPreAddAmount[6];
	uint8_t   sLoyaltAmount[6];

	int   lTraceNo;
	int	lOldTraceNo; 
	int	lBatchNumber;
	int	lOldBatchNumber;    
	int   iTransNo;        
	int   iOldTransNo;
	uint32_t    uiTxnRecIndex;       // transaction index

	uint8_t   ucInstallmentNum;     
	uint8_t   ucInstalPayMode; 
	uint8_t   szInstalCode[30+1]; 
	uint8_t   szInstalCurrCode[3+1];
	uint8_t   szPosId[9];	
	uint8_t   szCardNo[21];
	uint8_t   szTransDate[9];
	uint8_t   szTransTime[7];
	uint8_t   szDate[9];
	uint8_t   szTime[7];
	uint8_t   szSysReferNo[13]; 
	uint8_t   szAuthNo[10];     
	uint8_t   szExpDate[5];
	uint8_t   szOldTxnDate[7]; 
	uint8_t   szOldTxnTime[7]; 
	uint8_t   szEntryMode[4];
	uint8_t	szTellerNo[4];
	uint8_t	szCardUnit[4];  
	uint8_t	szAuthMode[3]; 
	uint8_t	szAuthUnit[12]; 
	uint8_t	szIssuerResp[21];
	uint8_t	szCenterResp[21];
	uint8_t	szRecvBankResp[21];
	uint8_t	szIssuerBankId[12];
	uint8_t	szRecvBankId[12];
	uint8_t	szCenterId[9];
	uint8_t	szPrtOldPosSer[7]; 
	uint8_t	szPrtOldAuthNo[7]; 
	uint8_t	szPrtOldHostSer[13]; 
	uint8_t 	ucKeyIndex;
	uint8_t   ucKeyVersion;
	uint8_t   sAfterBalanceAmt[4+1];    
	uint8_t   szTranferCardNo[21];      
	uint8_t   ucEcTopOldSwipeFlag;      
	uint8_t   ucQPBOCorPBOC;           

	// EMV data
	uint8_t	sIccData[255];
	uint8_t	iIccDataLen;
	uint8_t	bPanSeqNoOk;		// ADVT case 43 
	uint8_t	ucPanSeqNo;			// App. PAN sequence number
	uint8_t	sAppCrypto[8];		// app. cryptogram
	uint8_t	sAuthRspCode[2];
	uint8_t	sTVR[5];
	uint8_t	szAID[32+1];
	uint8_t	szAppLable[16+1];
	uint8_t	sTSI[2];
	uint8_t	sATC[2];
	uint8_t	ucFallBack;		
	uint8_t	ucSwipeFlag;
	uint8_t 	ucNeedSignature; 
	uint8_t 	ucIfECTrans;
	uint8_t 	ucElecSignatureflag;   
	uint8_t	szElecfilename[40];  
	uint8_t 	szSettDate[5];      
	uint8_t 	szMobilePhone[21];   
	uint8_t 	szCurrencyCode[4]; 
	uint8_t 	sExtAmount[6];       //balance
	uint8_t 	szREFU[400];   //retain for tms,the length of struct is fixed
	uint8_t 	sARQC[8];      //NETPAY CANCEL ��ӡʱ��Ҫ�õ�

	//NETPAY
	long    lpreTipOriAmount;   // ���ѽ���ԭʼ���
	long    lpreTipAmount;		//С�ѽ��
	
	uint8_t preTipAmount[6];   // С�ѽ���С�ѽ��
	uint8_t preTipOriAmount[6];//С�ѽ���ԭ���

	long    lPreAuthTipAmount; //Ԥ��Ȩ�������(С�ѽ��)
	long    lPreAuthOriAmount; //Ԥ��Ȩ���
	uint8_t preAuthTipAmount[6];   // Ԥ��Ȩ�������
	uint8_t preAuthOriAmount[6];   // Ԥ��Ȩ���
	
	int       waiterNo;			// ����Ա���
	int       szintereseMonth;  //����Ϣ����

	int        CardType;        // 1 visa 2 master 3 AMEX 4 PCLAVE 5 SODEXO 6 TODITO 7 EDENRED
	uint8_t  StIssuerID;     //��ȡ������ʱ���õ���bin�б������һ���е�11λ
	uint8_t  AscCardType[20];;
	uint8_t  CardHolderName[50];    //�ֿ�������

	//�����Ҫ���Ӳ������������￪ʼ���ӣ�
	//Ȼ���ȥ���ݿռ�(������һ��60�ռ�ģ����ԭʼ��1000-60,�豣֤���²��ּ�����==1000)
	uint8_t  PinCheckSuccessFlag;
	uint8_t  ucCheckOutId;   // CheckOut��ɱ�־
	uint8_t  szCheckOutCardNo[21];	//����check out card no
	uint8_t  szMoneyType;			
	uint8_t  szInputType;      //�˴����ڱ�ʾ�������ǽ��
	uint8_t  ReadCardType;	
	uint8_t  szAC[30];
	int        iSelectOldTransNo;
	uint8_t  ucNetpayAdjusted;	
	uint8_t  sNetpayAdjustedAmount[6];
	uint8_t TransFlag;
	uint8_t  isCtlplat;//ÿһ���Ƿ������������ƽ̨	
	uint8_t  CtlFlag ;//���������Ƿ�����������ƽ̨

	//����ƽ̨����
	char oil_filed_60[100];//60�򣬺�̨���صģ�ȷ�Ͻ��ʱԭ������
	char oil_field_62[100];//62�򣬺�̨���ص�,�����������
	char oil_liters[10];//����c2
	char oil_price[10];//�͵���c3
	char oil_amount[10];//���ܼ�c4
	uint8_t oil_id[4+1]; //������c5
	uint8_t nato[4+1];//nato,��ӡ��ʱ���õ�c6
	uint8_t folio[10];//��ӡ��ʱ���õ�c7
	char max_tip[10];//���С�ѽ�c8
	char pemex[15];//��ӡ�õ�c9
	char cp[20]; //��ӡ�õ�CP
	char cf[40];//��ӡ��ʱ���õ�cf

	//electronic wallet used
	uint8_t Amount[13];//����ƽ̨���ص�ȷ�Ͻ�
	char nip[6]; //value of NIP
	char kilometer[8]; //KILOMETER
	char plate[12+1]; //placas
	char product[10];//get from C5 :oil id 
	char counter[4];//counter
	char serialno[10];//pos serial number

	//getrules and balance
	char saletype[4+1];//amount or liters
	char isnip[4+1] ;//Nip or not
	char nnip[4+2]; //new nip
	char origi_kilometer[8];
	char origi_plate[8];
	char RRN[15+1];//37 field
	char DriverRule[4+1];
	char Horometer[4+1];//E2 value
	char Liters[4+1];
	char Driver_num[10+1];
	char Driver_nip[4+1];
	uint8_t Trans_Id[6+1];
	
	//char productid[4];//Product Id
		
	char stpromptmsg[20];
	uint8_t Balance_Amount[13];//pre-auth balance

	//GetAuth Return;
	//SODEXO
	char Prt_SR[50];
	char Prt_ST[10];
	char Prt_SB[12];
	char SA[10];
	
	//uint8_t  OtherTemp[1000-1-6-1-21-1-1-30-4-1-6-1-1-1-1-6];   // ��Ҫ���Ӳ�����ʱ��������ȥ
} NEWPOS_LOG_STRC;	
#define LOG_RECORD_LEN sizeof(struct POS_LOG_STRUCT)

typedef struct NEWPOS_COM_STRUCT {    
	uint8_t  szRespCode[2+1];       
	uint8_t  szUnitNum[15+1];		
	uint8_t  sPIN[8];
	char  szTrack1[TRACK1_LEN+1];
	char  szTrack2[TRACK2_LEN+1];
	char  szTrack3[TRACK3_LEN+1];
	NEWPOS_LOG_STRC stTrans;		
	NEWPOS_8583PACKET_STRC stTrnTbl;  
	uint8_t  ucFallBack;	
	uint8_t  ucSwipedFlag;
	uint8_t  ucPinEntry;	
	uint8_t  sReversalDE55[128];	
	int  iReversalDE55Len;
	uint8_t  sScriptDE55[128];
	int  iScriptDE55Len;
	uint8_t  bOnlineTxn;	
	uint8_t  szCertData[2+20+1];
	uint8_t  bIsFirstGAC;
	uint8_t  bSkipEnterAmt;	
	uint8_t  ucTransferFlag;  // transfer card or into the card
	uint8_t  ucConnectCnt;
	uint8_t  ucPBOCFlag;  // main menu selcet 2 is PBOC
	STISO8583 stOrgPack;
	uint8_t  bGetHostResp;
}POS_COM;
#define COM_SIZE  sizeof(struct NEWPOS_COM_STRUCT)

typedef struct   _REFUND_TRACK_STRUCT {
	int    lTraceNo;   
	uint8_t    szTrack2[TRACK2_LEN+1];
	uint8_t	 szTrack3[TRACK3_LEN+1];
	uint8_t	 szOldDate[5];	
} REFUND_TRACK_STRC;
#define REFUND_LOG_STRC_LEN sizeof(struct _REFUND_TRACK_STRUCT)

typedef struct _total_strc {	
	int iRmbCreditNum;
	int iRmbDebitNum;	
	int iRmbSaleNum;		
	int iRmbConfirmNum;	
	int iRmbOffConfirmNum;
	int iRmbRefundNum;	
	int iRmbOffNum;	
	int iRmbEcNum;

	int iFrnCreditNum;	
	int iFrnDebitNum;		
	int iFrnSaleNum;	
	int iFrnConfirmNum;
	int iFrnOffConfirmNum;
	int iFrnRefundNum;
	int iFrnOffNum;	
	int iTopUpNum;
	int iFrnEcNum;

	uint8_t sRmbDebitAmt[6];
	uint8_t sRmbCreditAmt[6];
	uint8_t sFrnCreditAmt[6];	
	uint8_t sFrnDebitAmt[6];	
	uint8_t sRmbSaleAmt[6];	
	uint8_t sRmbConfirmAmt[6];	
	uint8_t sRmbOffConfirmAmt[6];	
	uint8_t sRmbRefundAmt[6];	
	uint8_t sRmbOffAmt[6];	
	uint8_t sFrnSaleAmt[6];	
	uint8_t sFrnConfirmAmt[6];	
	uint8_t sFrnOffConfirmAmt[6];	
	uint8_t sFrnRefundAmt[6];	
	uint8_t sFrnOffAmt[6];	
	uint8_t sTopUpAmt[6];
	uint8_t sRmbEcAmt[6];
	uint8_t sFrnEcAmt[6];

	int lBatchNum; 
	uint8_t sDateTime[15]; 
}TOTAL_STRC;

#define TOTAL_STRC_SIZE  sizeof(struct _total_strc)

typedef struct _oper_structure {
	char szTellerNo[4];
	char szPassword[13];	
}OPER_STRC;
#define OPER_STRC_SIZE sizeof(struct _oper_structure)

typedef struct _NEWPOS_PARAM_STRC {
	uint8_t szPhoneNum1[20];
	uint8_t szPhoneNum2[20];
	uint8_t szPhoneNum3[20];
	uint8_t szMPhone[20];
	uint8_t szExtNum[10];
	uint8_t szPOSRemoteIP[15+1];
	uint8_t szPOSRemotePort[5+1];
	uint8_t szPOSRemoteIP2[15+1];
	uint8_t szPOSRemotePort2[5+1];
	uint8_t szUnitNum[16];
	uint8_t szPosId[9];
	uint8_t ucSendOffFlag;	
	uint8_t sTpdu[6];
	uint8_t ucTipOpen;		
	uint8_t sTransOpen[8];
	uint8_t ucAutoLogoff;	
	uint8_t ucManualInput;
	uint8_t ucErrorPrt;	
	uint8_t szUnitChnName[41];
	uint8_t szEngName[41];
	uint8_t ucTestFlag; 	
	uint8_t ucVoidSwipe;	
	uint8_t ucAuthVoidSwipe;	
	uint8_t ucConfirm;	
	uint8_t ucVoidPin;	
	uint8_t ucPreVoidPin;
	uint8_t ucPreComVoidPin;
	uint8_t ucPreComPin;  
	uint8_t ucKeyMode;    
	uint8_t ucEmvSupport;
	uint8_t ucqPbocSupport;      
	uint8_t szParamsPwd[7];   
	uint8_t szSafepwd[9];
	uint8_t szTpdu[11];       
	uint8_t szTransCtl[17]; 
	uint8_t bPreDial;       
	uint8_t szDownloadTel[20]; 
	uint8_t szDownLoadTID[9]; 
	uint8_t ucDetailPrt;   
	uint8_t ucMaxOfflineTxn;  
	uint8_t ucOfflineSettleFlag;  
	uint8_t ucPrnTitleFlag; 
	uint8_t szPrnTitle[60];   
	uint8_t szAsyncTpdu[11];

	uint8_t ucTrackEncrypt;  
	uint8_t ucDefaltTxn; 
	uint8_t ucEnterSupPwd;    
	uint8_t ucReversalTime;   
	uint8_t ucPrnNegative;    
	uint8_t ucDownBlkCard;   
	uint8_t ucLongConnection; 
	uint8_t ucSetFastKey;     // hot key
	uint8_t ucSupportElecSign;
	
	int ucTicketNum;	
	int ucOprtLimitTime;  
	int ucCommWaitTime; 
	int ucTipper;		
	int ucAdjustPercent;	
	int ucResendTimes;	      
	int ucOfflineSendTimes;   
	int ucDialRetryTimes;	
	int ucKeyIndex;		
	int iMaxTransTotal;  
	int lNowTraceNo; 	
	int lNowBatchNum;	
	int lMaxRefundAmt;
	int iEcMaxAmount;              
	int iElecSignReSendTimes; 
	
	int iDispLanguage;   // display lanaguage
	int iPPPAuth;        // ppp Authentication
	int iElecSignTimeOut;     

	ST_COMM_CONFIG		stTxnCommCfg;	        // terminal communication config
	ST_COMM_CONFIG		stDownParamCommCfg;		// terminal tms config		
	ST_WIFI_AP_INFO     stWIFIAPInfo;           // wifi AP config

	uint8_t ucSupPinpad;   
	uint8_t ucTelIndex;    // current Telphone
	uint8_t ucTCPIndex;    // current IP
	uint8_t ucReprint;     //reprint for reboot
	uint8_t ucPINMAC;

	uint8_t ucPreAuthMaskPan;

	uint8_t szDialNum[32];
	uint8_t ucCommHostFlag;
	uint8_t ucWnetMode;        //wireless mode
	uint8_t ucSalePassive;     // sale menu support tape

	
	char szStreet[100];
	char szMerchantName[100];
	char szCity[100];
	uint8_t No_Promotions;
	uint8_t Costom_Promotions;
	uint8_t MonthFlag3;
	uint8_t MonthFlag6;
	uint8_t MonthFlag9;
	uint8_t MonthFlag12;
	uint8_t MonthFlag18;
	uint8_t ucReCheckprint;     //reprint for reboot
	uint8_t szManagePwd[7]; 
	uint8_t TmsManagePwdTemp[7];     //TmsAdminPwdSwitch == 1ʱ��������������������ϵͳ����
	uint8_t szAF[60];

	//NETPAY
	char szUser[20];
	char szPassword[20];
	char szMerchantName1[60];	// ���ã��ĳɱ��ÿռ�ʹ��
	char szSN[20];	// pos SN
	char szPR[20];
	char szST[20];   // store ID
	char szLG[20];	
	char szCB[20];   //card first 6 num	
	char szStreet1[100];		// ���ã��ĳɱ��ÿռ�ʹ��
	char szCity1[30];			// ���ã��ĳɱ��ÿռ�ʹ��
	int  szLogonFlag;     // ��¼�ɹ���־
	uint8_t  szpreTip;			// ����ǰС��
	uint8_t  szpreAuth;         // Ԥ��Ȩ
	int  szPostTip;         // ���Ѻ�С��
	int  waiterNo;			// ����Ա���
	int  percent;			// ���Ѱٷֱ�
	uint8_t open_internet;      //�Ƿ�򿪻���������

	// TMS
	int     SettleSign;  // ��Ҫ����ı�־(false  ��Ҫ����  true����Ҫ)
	int	  BatchSign[25];	
	char    szLastDownload[14+1];
	uint8_t	BatchTime[25][4+1];	//batch time
	uint8_t	BatchNum;			//batch num
	uint8_t setDownloadtimer;

	int     TmsAdminPwdSwitch;     //TMS �����������뿪��
	uint8_t TmsAdminPwdTemp[7];     //TmsAdminPwdSwitch == 1ʱ��������������������ϵͳ����
	int     EmvFallBack;     		//EMV fall back switch
	
	int iMaxMuteNum;  		      //�����������
	int iMuteLogTimeouts;		  //������¼�ɴ���ʱ��
	uint8_t ucCommDNS;
	int iMuteRetryTimeouts;	      //����������ˢ����ʱ��
	uint8_t ucSignInputMobile;     //ǩ�������ֻ�����
	uint8_t ucSupportSM;      //�Ƿ�֧�ֹ����㷨
	uint8_t ucCurrWIFIId;     //��ǰ����WIFI�ǵڼ���WIFI�ļ�
	uint8_t ucCommModule;      //ͨѶģ����Ϣ
	uint8_t ucContactnessMethod;  //�ǽӽ���ͨ��
	uint8_t ucSecondIpFlag;    //�ڶ���IP��־
	uint8_t CheckInFlag;			
	uint8_t CheckOutFlag;
	//gas platform
	uint8_t stVolContrFlg;            //�������ƿ���

	//��һ��û���ڱ��ÿռ��м�ȥ
	uint8_t stSodexoTId[9+1]; //Sodexo Terminal Id
	uint8_t stPCSerial[11+1]; //PuntoClave Serial Number
	uint8_t stPCPid[8+1]; //PuntoClave Part Id
	uint8_t stPCpass[4+1];//PuntoClave Password
	uint8_t stETId[5+1]; //Edenred Terminal Id
	uint8_t stETerm[50+1]; //Edenred Terminal Token
	//print head
	uint8_t stHeader1[50];
	uint8_t stAddress1[50];
	uint8_t stAddress2[50];
	uint8_t stAddress3[50];
	uint8_t stAffId[7+1];
	uint8_t stTerminalId[15+1];
	uint8_t szMechId[9];

	//end

	//tms ����
	uint8_t Netpay_RefundFlag;
	uint8_t Netpay_AdjustFlag;
	uint8_t Switch_Retail;
	uint8_t Switch_Hotel;
	uint8_t Switch_Restaurant;
	uint8_t Switch_USD;
	uint8_t Switch_Branch;
	uint8_t Netpay_CancelFlag;
	uint8_t szSupportMultMoneyFlag;		//֧�ֶ����
	uint8_t szMoneyTypeFlag;		//֧�ֶ����
	uint8_t Netpay_ForzadaFlag;
	uint8_t szREFU[2000-1-1-1-1-1-1-1-1-1-1-1];       //retain for tms,the length of struct is fixed

}POS_PARAM_STRC;


#define  MAX_TRANLOG			   1000		     
#define  INV_TXN_INDEX			   0xFF000000	 
#define TS_NEEDTC			    0x4000		
#define TS_NEEDUPLOAD			0x2000		
#define TS_TXN_OFFLINE			0x1000		
#define TS_ICC_OFFLINE          0x0800      
#define TS_ELECSIGN_TXN         0x0400     
#define TS_ELECSIGN_FAIL_TXN    0x0200     
typedef struct _TRANS_CTRL_PARAM {
	uint8_t szNowTellerNo[4];	
	uint8_t szNowTellerPwd[13];
	uint8_t szOldTellerNo[4];
	uint8_t ucPosStatus; 
	uint8_t ucBatchUpStatus; 	 
	uint8_t ucLoadTask;  
	uint8_t szLastTransDate[9]; 
	uint8_t ucLogonFlag;	
	uint8_t ucClearLog;	
	uint8_t ucDownAppFlag;   
	uint8_t bEmvCapkLoaded;	
	uint8_t bEmvAppLoaded;	
	uint8_t ucRmbSettSaveRsp;
	uint8_t ucFrnSettSaveRsp;
	uint8_t ucCurSamNo;
	int     iTransNum;       
	uint32_t sTxnRecIndexList[MAX_TRANLOG];
	uint8_t szLoginTellerNo[4];
	int     iCheckTransNum;  
	uint8_t szLastCheckTransDate[9]; 
	uint8_t ucClearCheckLog;	
	uint8_t ucLastTransType;     //check in and check out is 1
	uint8_t szREFU[500-4-9-1-1];   //retain for tms,the length of struct is fixed

}TRANS_CTRL_PARAM;


typedef struct errinfostru{
	uint8_t ucRetCode[3];	
	uint8_t szMessage[17]; 
	uint8_t szOperate[33]; 
}ERR_INFO_STRC;

typedef struct DIAL_STAT_STRUC {
	int hours;		
	int dials;		
	int fails;		
	int  suc_rate;	
}DIAL_STAT;


typedef struct STAT_TIME_STRUC {
	int	year;
	char	month;
	char	day;
	char	hour;
}STAT_TIME;


typedef struct _limit_total_strc {	
	uint8_t sRmbDebitAmt[6];	
	uint8_t sRmbCreditAmt[6];
	uint8_t sFrnCreditAmt[6];	
	uint8_t sFrnDebitAmt[6];	
}LIMIT_TOTAL_STRC;

typedef struct temp_global_var_strc {
	int iTransNo;			
	int iLogFilePostion;	 	      //Log file location
	uint8_t ucPinEnterTime;             //PIN times
	uint8_t ucRmbSettRsp;	             
	uint8_t ucFrnSettRsp;	            
	uint8_t szDate[9];
	uint8_t szTime[7];
	uint8_t sPacketHead[7];           
	uint8_t bSendId; 
	uint8_t bCheckPrdial; 
	uint8_t ucPowerOnFlg;				//�ϵ���
/*
	//gasplatform used
	//bank card used
	char oil_filed_60[100];//60�򣬺�̨���صģ�ȷ�Ͻ��ʱԭ������
	char oil_field_62[100];//62�򣬺�̨���ص�,�����������
	char oil_liters[10];//����c2
	char oil_price[10];//�͵���c3
	char oil_amount[10];//���ܼ�c4
	uint8_t oil_id[4+1]; //������c5
	uint8_t nato[4+1];//nato,��ӡ��ʱ���õ�c6
	uint8_t folio[10];//��ӡ��ʱ���õ�c7
	char max_tip[10];//���С�ѽ�c8
	char pemex[15];//��ӡ�õ�c9
	char cp[20]; //��ӡ�õ�CP
	char cf[40];//��ӡ��ʱ���õ�cf

	//electronic wallet used
	uint8_t Amount[13];//����ƽ̨���ص�ȷ�Ͻ�
	char nip[6]; //value of NIP
	char kilometer[8]; //KILOMETER
	char plate[12+1]; //placas
	char product[10];//get from C5 :oil id 
	char counter[4];//counter
	char serialno[10];//pos serial number

	//getrules and balance
	char saletype[4+1];//amount or liters
	char isnip[4+1] ;//Nip or not
	char nnip[4+2]; //new nip
	char origi_kilometer[8];
	char origi_plate[8];
	char RRN[15+1];//37 field
	char DriverRule[4+1];
	char Horometer[4+1];//E2 value
	char Liters[4+1];

	char Driver_num[10+1];
	char Driver_nip[4+1];
	
	uint8_t Trans_Id[6+1];
	
	//char productid[4];//Product Id
		
	char stpromptmsg[20];
	uint8_t Balance_Amount[13];//pre-auth balance

	//GetAuth Return;
	//SODEXO
	char Prt_SR[50];
	char Prt_ST[10];
	char Prt_SB[12];
	char SA[10];
*/
} STRUCT_TEMP_VAR;

typedef  struct _tagTermAidList 
{
	uint8_t	ucAidLen;			
	uint8_t	sAID[17];			
	uint8_t	bOnlinePin;		   
	uint32_t    m_EcMaxTxnAmount;       // 9F7B
	uint32_t 	m_ReaderMaxTransAmount; // DF20
	uint32_t 	m_FloorLimiteAmount;    // DF19
	uint32_t 	m_CVMAmount;            // DF21
}TermAidList;

typedef struct _tagCapkInfo 
{
	uint8_t	bNeedDownLoad;
	uint8_t	sRid[5];
	uint8_t	ucKeyID;
	uint8_t	sExpDate[4];
}CapkInfo;

#define    MAX_BLACKLIST_NUM    40000
#define    ST_BLACKLIST_USED         0x01   
#define    ST_BLACKLIST_NEEDDOWN     0x02   
typedef struct _tag_BLACKLIST_TOTAL
{
	uint8_t            ucFlag;          
	char               szVersion[10+1]; 
	uint32_t           ulBlackListNum;  
	char               szBlackList[MAX_BLACKLIST_NUM][8];  // 1len + 7 data
}ST_BLACKLIST_TOTAL;

typedef struct _tag_POS_CONFIG_INFO
{
	uint32_t   uiPosType;            
	uint32_t   uiSecuType;           
	uint32_t   uiPrnType;            
	uint32_t   uiCommType;           
	uint32_t   uiMaxSyncPSTNRate;   
	uint32_t   uiMaxAsynPSTNRate;    
	uint32_t   uiUnContactlessCard;  
	uint32_t   uipostsScreen;       
	uint32_t   uiScreenType;        
	uint32_t   uiPortType;           
}ST_POS_CONFIG_INFO;

typedef struct _tag_POS_VERSION
{
	char       szVersion[15];            // version
	char       szCertificate[10];        // Certificate No.
}ST_POS_VERSION;

//Synchronous trace
typedef struct _st_UPLTRACENO_PAIR {
  int lNowTraceNo;
  int lUplFirstTraceNo;
}ST_UPLTRACENO_PAIR;

#define MAX_UPL_PATCH (MAX_TRANLOG/8)

typedef struct _st_UPLTRACENO_SYNC {
  int lCurUplNo;
  ST_UPLTRACENO_PAIR stUplTracePair[MAX_UPL_PATCH];
}ST_UPLTRACENO_SYNC;

// NETPAY params
typedef struct _NETYAY
{
	char szUser[20];
	char szPassword[20];
	char szSN[20];
	char szST[20];
	char szLG[20];
}NETYAY;

struct _NETYAY Netpay;

typedef struct _POS_COM_CONTROL
{
	char szAmount[20];
	char szAvailablebalance[20];
	char szAproNo[7];
	char szMent1[80];
	char szMent2[80];
	char szMent3[80];
	char szMent4[80];
	char szMent5[80];
	char szMent6[80];
	char szMent7[80];
	char szMent8[80];
	char szMent9[80];
	char szMent10[80];
	char szCommissionPercen[10];
	long szTotalTaxAmount;
	long szTotalAmount;
	char szSKU[20];
	char szAF[60];
	char szRM[200];        // fail reason
	char szCV[20];
	char szBomba[5]; //GASplatform 
	int szTopupAmount;
	int TransFlag;
	uint8_t  ErrorReason[200];
	uint8_t  AutoUpdateFlag[5];       // �Զ����±�־
}POS_COM_CONTROL;

struct _POS_COM_CONTROL PosComconTrol;

typedef struct NetpayPhOneTopup {
	int      iCompanyNo;			    
	int      szTopupAmount;		 
	char     szSKU[13+1];		    
} NETPAYPHONETOPUP;

typedef struct Total_Trans {
	int      iTransNo;			    //��������
	char     szTransName[30];
	long     lTotalTransAmount;		 //�����ܽ��
	int      iTotalTransNo;  //�����ܱ���		    
} TOTAL_TRANS;

typedef struct _tagST_MANAGE_TCP
{
	uint8_t		szPPPAPN[64+1];
	uint8_t		szPPPUID[64+1];
	uint8_t		szPPPPwd[16+1];
	uint8_t		szRemoteIP[20+1];
	uint8_t		szRemotePort[5+1];
	uint8_t		szRemoteIP2[20+1];
	uint8_t		szRemotePort2[5+1];
}ST_MANAGE_TCP_INFO;

typedef struct {
  uint8_t task_flag;    //
  uint8_t down_falg;
  uint8_t last_logon[6];    //YYMMDDhhmmss
  int     retry_cnt;
  char    ver_info[256];
} TMS_INFO_ST;

extern TMS_INFO_ST tms_info;

#define TASK_FLAG_NONE  0   //û����
#define TASK_FLAG_FORCE 1   //ǿ�Ƹ���
#define TASK_FLAG_OPT   2   //ѡ�����
#define TASK_FLAG_AUTO  3   //�Զ�����
#define TASK_FLAG_RST  0x80 //����

#define DOWN_FALG_NONE  0   //��������
#define DOWN_FALG_QUERY 1   //���ڷ�Ѳ���
#define DOWN_FALG_READY 2   //��Ѳ��
#define DOWN_FALG_DOING 3   //��������
#define DOWN_FALG_DONE  4   //��������ɣ�������

int get_tms_file(TMS_INFO_ST *ptms_info);

//++end
#endif
