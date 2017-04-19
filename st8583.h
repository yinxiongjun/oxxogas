
#ifndef _ST8583_H
#define _ST8583_H

#include "posapi.h"
#include "pack8583.h"

#define LEN_MSG_CODE			4
#define LEN_BITMAP				8
#define LEN_PAN					19
#define LEN_PROC_CODE			6
#define LEN_TRAN_AMT			12
#define LEN_POS_STAN			6
#define LEN_LOCAL_TIME			6
#define LEN_LOCAL_DATE			4
#define LEN_EXP_DATE			4
#define LEN_SETTLE_DATE			4
#define LEN_ENTRY_MODE			4	
#define LEN_PAN_SEQ_NO			3
#define LEN_NII			        4
#define LEN_SVR_CODE			2
#define LEN_CAPTURE_CODE		2
#define LEN_ACQUIRER_ID			11
#define LEN_TRACK1				79
#define LEN_TRACK2				37
#define LEN_TRACK3				104
#define LEN_RRN					12
#define LEN_AUTH_CODE			6
#define LEN_RSP_CODE			2
#define LEN_TERM_ID				8
#define LEN_MERCHANT_ID			15
#define LEN_ADD_RSP_DATA		25
#define LEN_FIELD48				999
#define LEN_CURRENCY_CODE		3
#define LEN_FIELD50				999
#define LEN_PIN_DATA			8
#define LEN_SECURITY_INFO		16
#define LEN_EXT_AMOUNT			12
#define LEN_ICC_DATA			2500
#define LEN_EP_DATA				100
#define LEN_FIELD59				999
#define LEN_FIELD60				999
#define LEN_ORIGINAL_MSG		999
#define LEN_FIELD62				1000
#define LEN_FIELD63				999
#define LEN_MAC_DATA			8

#define LEN_FIELD65			200  
#define LEN_FIELD71			200  
#define LEN_FIELD72			200   
#define LEN_FIELD73			200
#define LEN_FIELD74			1000
#define LEN_FIELD75			1000
#define LEN_FIELD76			200
#define LEN_FIELD77			200
#define LEN_FIELD78			200
#define LEN_FIELD79			200
#define LEN_FIELD80			200
#define LEN_FIELD81			200
#define LEN_FIELD82			200
#define LEN_FIELD83			200
#define LEN_FIELD84			200
#define LEN_FIELD88			200
#define LEN_FIELD89			200
#define LEN_FIELD95			200
#define LEN_FIELD96			999
#define LEN_FIELD97			999
#define LEN_FIELD98			999
#define LEN_FIELD99			999
#define LEN_FIELD104		200
#define LEN_FIELD105		400
#define LEN_FIELD106		999
#define LEN_FIELD107		999
#define LEN_FIELD108		200
#define LEN_FIELD109		200
#define LEN_FIELD110		200
#define LEN_FIELD111		200
#define LEN_FIELD112		200
#define LEN_FIELD113		200
#define LEN_FIELD114		200
#define LEN_FIELD115		256
#define LEN_FIELD116		256
#define LEN_FIELD117		512
#define LEN_FIELD118		256
#define LEN_FIELD119		256
#define LEN_FIELD120		999
#define LEN_FIELD128		8

#define LEN_EXTERM 			200

typedef struct _tagSTISO8583{
	uint8_t	szMsgCode[LEN_MSG_CODE+2];
	uint8_t	sBitMap[2*LEN_BITMAP];
	uint8_t	szPan[LEN_PAN+2];
	uint8_t	szProcCode[LEN_PROC_CODE+2];
	uint8_t	szTranAmt[LEN_TRAN_AMT+2];
	uint8_t	szSTAN[LEN_POS_STAN+2];
	uint8_t	szLocalTime[LEN_LOCAL_TIME+2];
	uint8_t	szLocalDate[LEN_LOCAL_DATE+2];
	uint8_t	szExpDate[LEN_EXP_DATE+2];
	uint8_t	szSettleDate[LEN_SETTLE_DATE+2];
	uint8_t	szEntryMode[LEN_ENTRY_MODE+2];
	uint8_t	szPanSeqNo[LEN_PAN_SEQ_NO+2];
	uint8_t 	szNII[LEN_NII+2];
	uint8_t	szSvrCode[LEN_SVR_CODE+2];
	uint8_t	szCaptureCode[LEN_CAPTURE_CODE+2];
	uint8_t	szAcquirerID[LEN_ACQUIRER_ID+2];
	uint8_t	szTrack2[LEN_TRACK2+2];
	uint8_t	szTrack3[LEN_TRACK3+2];
	uint8_t	szRRN[LEN_RRN+2];
	uint8_t	szAuthCode[LEN_AUTH_CODE+2];
	uint8_t	szRspCode[LEN_RSP_CODE+2];
	uint8_t	szTermID[LEN_TERM_ID+2];
	uint8_t	szMerchantID[LEN_MERCHANT_ID+2];
	uint8_t	szAddRspData[LEN_ADD_RSP_DATA+2];
	uint8_t 	szField45[LEN_TRACK1+2];
	uint8_t	szField48[LEN_FIELD48+2];
	uint8_t	szCurrencyCode[LEN_CURRENCY_CODE+2];
	uint8_t	sField50[LEN_FIELD50+2];
	uint8_t	sPINData[LEN_PIN_DATA+2];
	uint8_t	szSecurityInfo[LEN_SECURITY_INFO+2];
	uint8_t	szExtAmount[LEN_EXT_AMOUNT+2];//改成变长
	uint8_t	sICCData[LEN_ICC_DATA+2];
	uint8_t	szEPData[LEN_EP_DATA+2];
	uint8_t szField59[LEN_FIELD59+2];
	uint8_t	szField60[LEN_FIELD60+2];
	uint8_t	szOrginalMsg[LEN_ORIGINAL_MSG+2];
	uint8_t	sField62[LEN_FIELD62+2];
	uint8_t	szField63[LEN_FIELD63+2];
	uint8_t	sMacData[LEN_MAC_DATA+2];
	// NETPAY MOBILE PHONE TOPUP ADD 128 FIELD
	
	uint8_t szField65[LEN_EXTERM + 2];	
	uint8_t szField66[LEN_EXTERM + 2];	
	uint8_t szField67[LEN_EXTERM + 2];	
	uint8_t szField68[LEN_EXTERM + 2];	
	uint8_t szField69[LEN_EXTERM + 2];	
	
	uint8_t szField70[LEN_EXTERM + 2];	
	uint8_t szField71[LEN_EXTERM + 2];	
	uint8_t szField72[LEN_EXTERM + 2];	
	uint8_t szField73[LEN_EXTERM + 2];	
	uint8_t szField74[LEN_EXTERM + 2];	
	uint8_t szField75[LEN_EXTERM + 2];	
	uint8_t szField76[LEN_EXTERM + 2];	
	uint8_t szField77[LEN_EXTERM + 2];	
	uint8_t szField78[LEN_EXTERM + 2];	
	uint8_t szField79[LEN_EXTERM + 2];
	
	uint8_t szField80[LEN_EXTERM + 2];	
	uint8_t szField81[LEN_EXTERM + 2];	
	uint8_t szField82[LEN_EXTERM + 2];	
	uint8_t szField83[LEN_EXTERM + 2];	
	uint8_t szField84[LEN_EXTERM + 2];	
	uint8_t szField85[LEN_EXTERM + 2];	
	uint8_t szField86[LEN_EXTERM + 2];	
	uint8_t szField87[LEN_EXTERM + 2];	
	uint8_t szField88[LEN_EXTERM + 2];	
	uint8_t szField89[LEN_EXTERM + 2];
	
	uint8_t szField90[LEN_EXTERM + 2];	
	uint8_t szField91[LEN_EXTERM + 2];	
	uint8_t szField92[LEN_EXTERM + 2];	
	uint8_t szField93[LEN_EXTERM + 2];	
	uint8_t szField94[LEN_EXTERM + 2];	
	uint8_t szField95[LEN_EXTERM + 2];	
	uint8_t szField96[LEN_EXTERM + 2];
	uint8_t szField97[LEN_EXTERM + 2];	
	uint8_t szField98[LEN_EXTERM + 2];	
	uint8_t szField99[LEN_EXTERM + 2];	
	
	uint8_t szField100[LEN_EXTERM + 2];	
	uint8_t szField101[LEN_EXTERM + 2];	
	uint8_t szField102[LEN_EXTERM + 2];	
	uint8_t szField103[LEN_EXTERM + 2];	
	uint8_t szField104[LEN_EXTERM + 2];	
	uint8_t szField105[LEN_EXTERM + 2];	
	uint8_t szField106[LEN_EXTERM + 2];	
	uint8_t szField107[LEN_EXTERM + 2];
	uint8_t szField108[LEN_EXTERM + 2];	
	uint8_t szField109[LEN_EXTERM + 2];
	
	uint8_t szField110[LEN_EXTERM + 2];	
	uint8_t szField111[LEN_EXTERM + 2];	
	uint8_t szField112[LEN_EXTERM + 2];	
	uint8_t szField113[LEN_EXTERM + 2];	
	uint8_t szField114[LEN_EXTERM + 2];	
	uint8_t szField115[LEN_EXTERM + 2];	
	uint8_t szField116[LEN_EXTERM + 2];	
	uint8_t szField117[LEN_EXTERM + 2];	
	uint8_t szField118[LEN_EXTERM + 2];	
	uint8_t szField119[LEN_EXTERM + 2];
	
	uint8_t szField120[LEN_FIELD120 + 2];	
	uint8_t szField121[LEN_EXTERM + 2];	
	uint8_t szField122[LEN_EXTERM + 2];	
	uint8_t szField123[LEN_EXTERM + 2];	
	uint8_t szField124[LEN_EXTERM + 2];	
	uint8_t szField125[LEN_EXTERM + 2];
	uint8_t szField126[LEN_EXTERM + 2];	
	uint8_t szField127[LEN_EXTERM + 2];	
	uint8_t szField128[LEN_FIELD128+ 2];
}STISO8583;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern FIELD_ATTR	glMsg0[];
extern FIELD_ATTR	glData0[];
extern STISO8583	glSendPack, glRecvPack;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _ST8583_H */


//-lsbTms

// end of file
