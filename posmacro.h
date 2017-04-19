
#ifndef _POSMACRO_H
#define _POSMACRO_H

//#define __DEBUG

//#define _POS_TYPE_8110
//#define _POS_TYPE_7110
#define _POS_TYPE_8210

//#define _GET_BASETATION


#define _EMV_TEST_VERSION		// PBOC test
//#define _OFFLINE_TEST_VERSION	// offline test
//#define CL_LED_SUPPORT 		//the lamp for contactless
//#define PIN_TEST_VERSION		//pin auth

//#define  SSL_CONNECT  



#define SRC_VERSION "V1.0.160629"

#define STR_ELECSIGNATURE_FILE_PATH   "./image"

#define  APP_NUM         10
#define  APP_NAME        17

#define  NUM_PER_PAGE	 50


#define  NO_TRANS_SELECT 	999

//NETPAY PHONE TOPUP company code
#define  IUSACEL        120
#define  MOVISTAR        121
#define  NEXTEL        122
#define  TELCEL        123
#define  UNEFON        124

enum{
	POS_SALE=1,
	POS_PREAUTH,
	POS_AUTH_CM,
	POS_PREAUTH_VOID,
	POS_AUTH_VOID,
	POS_SALE_VOID,
	POS_REFUND,
	POS_PREAUTH_ADD,
	POS_QUE,
	POS_REVERSE,

	POS_LOGON,
	POS_LOGOFF,
	POS_SETT,
	POS_BATCH_UP,
	POS_DOWNLOAD_PARAM,
	POS_UPLOAD_PARAM,
	RESPONSE_TEST,
	OFF_SALE,
	OFF_ADJUST,
	ADJUST_TIP,
	POS_OFF_CONFIRM,
	OFF_SALE_BAT,
	OFF_ADJUST_BAT,
	ADJUST_TIP_BAT,
	POS_REFUND_BAT,
	OFF_CONFIRM_BAT,
	QUE_TOTAL,
	QUE_BATCH_TOTAL,
	QUE_BATCH_MX,
	QUE_BATCH_ANY,
	PRT_LAST,
	PRT_TIP,
	PRT_ANY,
	PRT_DETAIL,
	PRT_TOTAL,
	PRT_LAST_TOTAL,
	TELLER_LOGON,
	TELLER_LOGOFF,
	TELLER_ADD,
	TELLER_DEL,
	TELLER_DISP,
	TELLER_CHGPWD,
	SETUP_EXTNUM,
	OPER_QUE_TERM_INFO,

	DOWNLOAD_EMV_CAPK,
	DOWNLOAD_EMV_PARAM,
	QUERY_EMV_CAPK,
	QUERY_EMV_PARAM,
	END_EMV_CAPK,
	END_EMV_PARAM,
	UPLOAD_SCRIPT_RESULT,
	ICC_OFFSALE,
	ICC_BATCHUP,
	DOWNLOAD,

    	EC_QUICK_SALE,
	QPBOC_ONLINE_SALE,
	EC_NORMAL_SALE,
	EC_TOPUP_CASH,
	EC_TOPUP_SPEC,
	EC_TOPUP_NORAML,
	EC_REFUND,
	EC_VOID_TOPUP,
	EC_BALANCE,
	EC_QUE_TXN,
	EC_QUE_TOPUP,

	POS_INSTALLMENT,
	POS_VOID_INSTAL,

	POS_DOWNLOADBLK,
	END_DOWNLOADBLK,
	ICC_FAIL_BATCHUP,
	POS_ELEC_SIGN,

    	RKI_INJECTION,
    	BALANCE_CHECK,
    	CREDIT_MESSAGE,
    	LAST_MOVEMENT,
    	PAYMENT_WITH_BALANCE_CHECK,
    	PAYMENT_WITH_BALANCE,
    	PAYMENT_WITH_CARD_CHECK,
    	PAYMENT_WITH_CARD,
    	PHONE_TOPUP,
    	ECHO_TEST,
    	INTERESES_SALE,
    	PRE_TIP_SALE,
    	PRE_VOL_CTL,
    	NORMAL_SALE,
    	POS_MAGTOPUP_ACCT,           //磁条卡帐户充值
    	CHECK_IN,
    	CHECK_OUT,
    	PRT_CHECK_IN,
    	PRT_CHECK_OUT,
    	CASH_ADVACNE,
    	NETPAY_REFUND,
    	NETPAY_ADJUST,
    	NETPAY_FORZADA,
    	PRT_ANY_CHECK_IN,
    	PRT_ANY_CHECK_OUT,

//gasplat transaction
//	RQC_VOL_SALE,
	PURSE_AUTH,
	PURSE_SALE,
	PURSE_SODEXO,
	PURSE_PUNTO,
	PURSE_TODITO,
	PURSE_EDENRED,

	PURSE_GETRULE,//获取交易规则
	PURSE_GETBALANCE //获取余额
};

#define  TRUE    1
#define  FALSE   0

#define  E_TRANS_CANCEL   1
#define  E_TRANS_FAIL     2
#define  E_NO_TRANS       3
#define  E_MAKE_PACKET    4
#define  E_ERR_CONNECT    5
#define  E_SEND_PACKET    6
#define  E_RECV_PACKET    7
#define  E_RESOLVE_PACKET 8
#define  E_REVERSE_FAIL   9
#define  E_NO_OLD_TRANS   10
#define  E_TRANS_VOIDED   11
#define  E_ERR_SWIPE      12
#define  E_MEM_ERR        13
#define  E_PINPAD_KEY     14
#define  E_TIP_AMOUNT	  15
#define  E_TIP_NO_OPEN	  16
#define  E_TRANS_HAVE_ADJUESTED 17
#define  E_FILE_OPEN	  18
#define  E_FILE_SEEK	  19
#define  E_FILE_READ	  20
#define  E_FILE_WRITE	  21
#define  E_CHECK_MAC_VALUE 22
#define  E_CARD_BLOCKER    23
#define  E_APP_BLOCKED     24
#define  E_TRAN_DEFUSED    25
#define  E_CARD_DEFUSED    26
#define  E_SCRIPT_FAIL     27
#define  E_SET_PARAM_FAIL  28
#define  E_PRE_PROCESS_FAIL  29
#define  LITER_WITHOUT          30

#define  NO_DISP          36
#define  E_APP_EXIT       40
#define  RE_SELECT        41
#define  E_APP_RESET      42   //重启应用
#define  E_REINPUT        43
#define  E_REINPUT_AMOUNT 44  //重输金额(check out)
#define  E_RESELECT       45

#define  E_NEED_FALLBACK	51
#define  E_NEED_INSERT		52
#define  E_CHECK_OUT_FINISH 53


#define  POS_LOG_FILE		    "pos.log"
#define  POS_LOG_CHECK_FILE		"pos_check.log"
#define  REVERSAL_FILE			"dup_file"
#define  REFUND_TRACK_FILE		"refnd.trk"
#define  TERM_AIDLIST			"termaid.lst"
#define  TERM_CAPK_FILE         "termcapk.lst"
#define  ICC_SCRIPT_FILE		"iccscript"
#define  ICC_FAILUR_TXN_FILE    "ICC_FAILUR.LOG"
#define  FILE_BLKCARD_LOG       "BLKCARD.DAT"
#define  UICC_MUTE_FILE 		"mutefile"

#define NO_SWIPE_INSERT		0x00
#define CARD_SWIPED			0x01
#define CARD_INSERTED		0x02
#define CARD_KEYIN			0x04
#define FALLBACK_SWIPED		0x08
#define CARD_PASSIVE        0x20
#define SKIP_CHECK_ICC		0x80

#define  PARAM_OPEN		'1'
#define  PARAM_CLOSE	'0'

#define  PARAM_PESEO	'1'
#define  PARAM_DOLARES	'2'

#define KEY_MODE_DES      '1'
#define KEY_MODE_3DES     '0'

#define  PIN_KEY_ID		2
#define  MAC_KEY_ID		3
#define  TEST_KEY_ID	4
#define  TDK_KEY_ID     5

#define  EMV_PIN_KEY_ID  31

#define  DOUBLE_PIN_KEY_ID  6
#define  DOUBLE_MAC_KEY_ID  8
#define  DOUBLE_TDK_KEY_ID  10

#define  HAVE_INPUT_PIN	  '1'
#define  NOT_INPUT_PIN	  '2'

#define  TRACK1_LEN        79
#define  TRACK2_LEN        40
#define  TRACK3_LEN        107

#define  NORMAL            0
#define  REPRINT           1
#define  REELECSIGN        3

#define  MAX_PACKET_LEN 2048*5+10


#define  MAX_OPER_NUM 		101

#define  SYS_TELLER_NO	"099"
#define  SUP_TELLER_NO	"000"
#define  SUP_TELLER_PWD "567347"

#define  TELLER_NOBODY	"\x01\x01\x01"
#define  TELLER_LOCK	"\x02\x01\x01"

#define  LOGGEDOFF		0
#define  LOGGEDON		1

#define	 RMBLOG				1
#define  FRNLOG				2
#define  ALLLOG				3
#define  EMVLOG				4

#define  WORK_STATUS         '0'
#define  TEST_STATUS         '1'
#define  BATCH_STATUS		 '2'
#define  SETTLE_STATUS		 '3'

#define  INITSIGNAL   1

#define ICC_USER	0

#define TRAN_AMOUNT		0
#define TIP_AMOUNT		1
#define ADJUST_AMOUNT	2
#define IC_AMOUNT       3

#define MAX_APP_NUM       32         //EMV
#define MAX_KEY_NUM       64         //EMV
#define PART_MATCH        0x01       //Partial matching
#define FULL_MATCH        0x00       //Complete matching


#define ERR_USERCANCEL  1
#define ERR_TRAN_FAIL	2

#define STR_PBOC_AID    "A000000333"

//子应用调用补登
#define    TMSFAIL		-1
#define		SUCC		1
#define    UNEXE        2
#endif
