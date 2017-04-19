
#include "posapi.h"
#include "posmacro.h"
#include "emvapi.h"
#include "glbvar.h"


struct NEWPOS_COM_STRUCT PosCom;
struct NEWPOS_COM_STRUCT BackPosCom;
struct _total_strc stTotal;  
struct _limit_total_strc stLimitTotal; 
struct _oper_structure operStrc[MAX_OPER_NUM];
struct _TRANS_CTRL_PARAM stTransCtrl;
struct _NEWPOS_PARAM_STRC stPosParam;    
struct temp_global_var_strc stTemp;
struct _st_UPLTRACENO_SYNC stUplTraceSync;
struct ST_PUB_APP_INFO stPubAppInfo;


int gUplTrace = 0;

int    iPsam_fd; 
int    imif_fd = -1;  


uint16_t  usSendPacketLen;  
uint16_t  usRecvPacketLen;
uint8_t	sSendPacketBuff[MAX_PACKET_LEN];	
uint8_t	sRecvPacketBuff[MAX_PACKET_LEN];	


struct emv_core_init  tEMVCoreInit;
struct terminal_mck_configure tConfig;
struct emv_callback_func tEMVCallBackFunc;



TermAidList glTermAidList[MAX_APP_NUM];
int		glTermAidNum;
DIAL_STAT   dial_stat;	

int start_from_manager=0;	//zyl+ ???????????????
int g_ums_img;				//zyl+ ??ums??
int wifi_status=0;

uint8_t   gucManageCommType = CT_ALLCOMM;
int       giIfInitWirelessSuccess = FALSE;

int       NoDispTmsPrecessingFlag = FALSE;     

char TitelName[50];//�������

struct errinfostru sRetErrInfo[] = {
	{"01",	"[01] pls contact", "the issuer"},
	{"02",	"[02] pls contact", "the issuer"},
	{"03",	"[03]            ", "invalid merchant"},
	{"04",	"[04]            ", "card picked up"},
	{"05",	"[05]  cardholder", "auth failed"},
	{"06",	"[06] pls contact", "the issuer"},
	{"07",	"[07] pls contact", "the issuer"},
	{"09",	"[09]            ", "try again"},
	{"12",	"[12]    invalid ", "transaction"},
	{"13",	"[13]    invalid ", "amount"},
	{"14",	"[14]    invalid ", "card number"},
	{"15",	"[15]    no card ", "to issuer"},
	{"19",	"[19] pls contact", "the issuer"},
	{"20",	"[20] pls contact", "the issuer"},
	{"21",	"[21]    card    ", "not personalized"},
	{"22",	"[22]  operation ", "error,pls retry"},
	{"23",	"[23] pls contact", "the issuer"},
	{"25",	"[25]no org trans", "contact issuer"},
	{"30",	"[30]            ", "try again"},
	{"31",	"[31]     card   ", "not accepted"},
	{"33",	"[33] pls contact", "the issuer"},
	{"34",	"[34]            ", "fraud card"},
	{"35",	"[35] pls contact", "the issuer"},
	{"36",	"[36] change card", "try again"},
	{"37",	"[37] pls contact", "the issuer"},
	{"38",	"[38]error times ","exceeds limit"},
	{"39",	"[39] pls contact", "the issuer"},
	{"40",	"[40]   trans    ", "no support"},
	{"41",	"[41]            ", "lost card"},
	{"42",	"[42] pls contact", "the issuer"},
	{"43",	"[43]            ", "stolen card"},
	{"44",  "[44] pls contact", "the issuer"},
	{"45",  "[45]            ", "insert card"},
	{"51",	"[51]insufficient", "balance"},
	{"52",  "[52] pls contact", "the issuer"},
	{"53",  "[53] pls contact", "the issuer"},
	{"54",	"[54]            ", "card expired"},
	{"55",	"[55]  key error ", "          "},
	{"56",  "[56] pls contact", "the issuer"},
	{"57",	"[57] pls contact", "the issuer"},
	{"58",	"[58]            ", "invalid terminal"},
	{"59",	"[59]            ", "card crc error"},
	{"60",  "[60] pls contact", "the issuer"},
	{"61",	"[61]     exceeds", "amount limit"},
	{"62",	"[62]            ", "restricted card"},
	{"64",	"[64]  unmatching", "with org card"},
	{"65",	"[65]     exceeds", "sale times"},
	{"66",	"[66]  pls contact", "the acquirer"},
	{"67",	"[67]  pls contact", "the issuer"},
	{"68",	"[68]  timeout    ", "try again"},
	{"75",	"[75]      exceeds", "pin error times"},
	{"77",	"[75]             ", "logon again"},
	{"90",	"[90]             ", "retry later"},
	{"91",	"[91]             ", "retry later"},
	{"92",	"[92]             ", "retry later"},
	{"93",  "[93]  pls contact", "the issuer"},
	{"94",	"[94] repetitive  ", "transaction"},
	{"96",	"[96]    rejected ", "system error"},
	{"97",	"[97]    terminal ", "not registered"},
	{"98",	"[98]    timeout  ", "please retry"},
	{"99",	"[99]  PIN format ", "error"},
	{"A0",  "[A0]  MAC check  ", "error"},
	{"A1",  "[A1]  inconsisent", "trans currency"},
	{"A3",  "[A3]   incorrect ", "account"},
	{"A7",	"[A7]   rejection ", "please retry"},
	{"",	"", ""}

};


NEWPOS_8583PACKET_STRC TrTable[] = {
	{POS_SALE,         "00", "22", "000"},
	{POS_PREAUTH,      "06", "10", "000"},
	{POS_AUTH_CM,      "06", "20", "000"},
	{POS_PREAUTH_VOID, "06", "11", "000"},
	{POS_AUTH_VOID,    "06", "21", "000"},
	{POS_SALE_VOID,    "00", "23", "000"}, 
	{POS_REFUND,       "00", "25", "000"}, 
	{POS_PREAUTH_ADD,  "60", "10", "000"},
	{POS_QUE,          "00", "01", "000"},
	{POS_REVERSE,      "00", "00", "000"}, 
	{POS_LOGON,        "00", "00", "001"}, 
	{POS_LOGOFF,       "00", "00", "002"}, 
	{POS_SETT,         "00", "00", "201"}, 
	{POS_BATCH_UP,     "00", "00", "201"}, 
	{POS_DOWNLOAD_PARAM,   "00", "00", "360"}, 
	{POS_UPLOAD_PARAM, "00", "00", "362"}, 
	{RESPONSE_TEST,    "00", "00", "301"}, 
	{OFF_SALE ,        "00", "30", "000"}, 
	{OFF_ADJUST  ,     "00", "32", "000"}, 
	{ADJUST_TIP,       "00", "34", "000"}, 
	{OFF_SALE_BAT,     "00", "30", "000"}, 
	{OFF_ADJUST_BAT,   "00", "32", "000"}, 
	{ADJUST_TIP_BAT,   "00", "34", "000"}, 
	{POS_REFUND_BAT,   "00", "25", "000"},
	{POS_OFF_CONFIRM,  "06", "24", "000"},
	{OFF_CONFIRM_BAT,  "06", "24", "000"},
	{DOWNLOAD_EMV_CAPK,  "00", "00", "370"},
	{DOWNLOAD_EMV_PARAM, "00", "00", "380"},
	{QUERY_EMV_CAPK,     "00", "00", "372"},
	{QUERY_EMV_PARAM,    "00", "00", "382"},
	{END_EMV_CAPK,    "00", "00", "371"},
	{END_EMV_PARAM,   "00", "00", "381"},
	{ICC_OFFSALE,     "00", "36", "000"},
	{ICC_BATCHUP,     "00", "00", "203"},

	//EC
	{EC_QUICK_SALE,   "00", "36", "000"},         
	{QPBOC_ONLINE_SALE, "00", "22", "000"},      
	{EC_NORMAL_SALE,  "00", "36", "000"},       
	{EC_TOPUP_CASH,   "91", "46", "000"},      
	{EC_TOPUP_SPEC,   "91", "45", "000"},       
	{EC_TOPUP_NORAML, "91", "47", "000"},       
	{EC_VOID_TOPUP,   "91", "51", "000"},       
	{EC_BALANCE,      "00", "22", "000"},       
	{EC_REFUND,       "00", "27", "000"},        
	{EC_QUE_TXN,      "00", "22", "000"},        
	{EC_QUE_TOPUP,    "00", "22", "000"},       

	// INSTALLMENT
	{POS_INSTALLMENT,  "64", "22", "000"},      
	{POS_VOID_INSTAL,  "64", "23", "000"},     

	{POS_DOWNLOADBLK,        "00", "00", "390"},          
	{END_DOWNLOADBLK,        "00", "00", "391"},           
	{ICC_FAIL_BATCHUP,       "00", "00", "204"},           
	{POS_ELEC_SIGN,          "00", "07", "000"},

	//NETPAY ()
	{BALANCE_CHECK,          "00", "00", "000"},
	{CREDIT_MESSAGE,          "00", "00", "000"},
	{LAST_MOVEMENT,           "00", "00", "000"},
	{PAYMENT_WITH_BALANCE,           "00", "00", "000"},
	{PAYMENT_WITH_BALANCE_CHECK,           "00", "00", "000"},
	{PAYMENT_WITH_CARD_CHECK,           "00", "00", "000"},
	{PAYMENT_WITH_CARD,           "00", "00", "000"},
	{PHONE_TOPUP,           "00", "00", "000"},	
	{ECHO_TEST,           "00", "00", "000"},
	{CHECK_IN,           "00", "00", "000"},
	{CHECK_OUT,           "00", "00", "000"},
	{CASH_ADVACNE,           "00", "00", "000"},
	{NETPAY_REFUND,           "00", "00", "000"},
	{NETPAY_ADJUST,           "00", "00", "000"},
	{NETPAY_FORZADA,           "00", "00", "000"},
	{-1,  "", "", ""}           
};

ST_POS_VERSION        gstPosVersion;
ST_POS_CONFIG_INFO    gstPosCapability;     // ???????
ST_COMM_CONFIG		  gstCurCommCfg;		 // ????????
char                  gszParamFileName[50];      //  ?????
//TMS
char      ParamPath[256];//????
char      s_gszAppName[50];   
ST_MANAGE_TCP_INFO    gstManageTcpCfg;

unsigned char Bmp_Show_in_PIN[] = 
{
    0x42, 0x4D, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x7A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFB, 0x9E, 
	0xAC, 0xFF, 0xFF, 0x80, 0x7F, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xDB, 0xDE, 
	0xAE, 0xCF, 0xF3, 0xB7, 0x7B, 0xF5, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xC8, 0x18, 
	0x8A, 0xF7, 0xEF, 0xB7, 0x78, 0xFE, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xDB, 0xDE, 
	0x2A, 0xFB, 0xDF, 0xF7, 0xFA, 0x82, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xD8, 0x1E, 
	0x8A, 0xFB, 0xBE, 0x20, 0xFA, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xDB, 0xDE, 
	0xAA, 0xFD, 0xBF, 0xCE, 0xFA, 0xC0, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xD0, 0x08, 
	0x0E, 0xFD, 0x7F, 0x57, 0xB2, 0xDD, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x1E, 0xFA, 
	0xFF, 0xFD, 0x7F, 0x9B, 0x78, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xF8, 0x1D, 
	0xA2, 0x7E, 0xFF, 0xD5, 0xFB, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFE, 0xF8, 
	0x5D, 0xFE, 0xFE, 0xFF, 0xBD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xD0, 0x0E, 
	0xEB, 0xFE, 0xFE, 0x00, 0x30, 0x7D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xBE, 0xFE, 
	0xF7, 0xF9, 0xFF, 0xF7, 0xFF, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 
};


unsigned char Bmp_Show_Balance[] = 
{
    0x42, 0x4D, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x7A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0x9E, 
	0xFB, 0x3F, 0x9F, 0xF7, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xDF, 
	0x7B, 0xB9, 0xDE, 0xF0, 0xDB, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xDF, 
	0xBB, 0xBE, 0xDD, 0xF6, 0xE7, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0xBB, 0xBF, 0x5B, 0xE6, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xE0, 0xDF, 
	0x80, 0x3F, 0xDF, 0xF0, 0x2D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0xBB, 0xBC, 0x01, 0xEA, 0xAD, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0xBB, 0xBF, 0xDE, 0x75, 0xAD, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0x80, 0x39, 0x05, 0xF8, 0xBD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xE0, 0xDF, 
	0xBB, 0xBE, 0xFB, 0xEB, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xDF, 
	0xBB, 0xBF, 0x77, 0xE0, 0x6F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x07, 
	0xBB, 0xBF, 0xAF, 0xFD, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xEF, 
	0x80, 0x3F, 0xDF, 0xFB, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 
};

unsigned char Bmp_Show_CurrName[] = 
{
    0x42, 0x4D, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x9F, 0x87, 0x00, 0x00, 0xEF, 0x77, 
	0x00, 0x00, 0xF7, 0x77, 0x00, 0x00, 0xFB, 0x7F, 0x00, 0x00, 0xFB, 0x7F, 0x00, 0x00, 0xFB, 0x7F, 
	0x00, 0x00, 0xFB, 0x7F, 0x00, 0x00, 0xC0, 0x07, 0x00, 0x00, 0xFF, 0xEF, 0x00, 0x00, 0xFF, 0xFF, 
	0x00, 0x00, 0xE0, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 
};

unsigned char Bmp_Show_AmtName[] = 
{
    0x42, 0x4D, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x80, 0x0E, 
	0xE7, 0xBF, 0x80, 0x00, 0x00, 0x00, 0xF5, 0x7E, 0x1B, 0x79, 0x80, 0x00, 0x00, 0x00, 0xED, 0xBE, 
	0xDC, 0xF9, 0x80, 0x00, 0x00, 0x00, 0xFD, 0xFC, 0xD5, 0xBF, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x1E, 
	0x05, 0xBF, 0x80, 0x00, 0x00, 0x00, 0xFD, 0xFD, 0x55, 0xB9, 0x80, 0x00, 0x00, 0x00, 0xE0, 0x7E, 
	0xB5, 0xB9, 0x80, 0x00, 0x00, 0x00, 0x9F, 0xCF, 0x17, 0xBF, 0x80, 0x00, 0x00, 0x00, 0xEF, 0xBD, 
	0x60, 0x3F, 0x80, 0x00, 0x00, 0x00, 0xF7, 0x7C, 0x0D, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFA, 0xFF, 
	0xBD, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFD, 0xFF, 0x70, 0x3F, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 
};


unsigned char Bmp_Show_LytBalance[] = 
{
    0x42, 0x4D, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 
	0x00, 0x00, 0x7A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0x9E, 
	0xFB, 0x3E, 0xBF, 0x77, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xDF, 
	0x7B, 0xBE, 0xDF, 0x7B, 0xAF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xDF, 
	0xBB, 0xBA, 0xEE, 0xFD, 0xEF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0xBB, 0xBA, 0xF5, 0xFE, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xE0, 0xDF, 
	0x80, 0x3C, 0xBF, 0xFE, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0xBB, 0xBC, 0x60, 0xEE, 0xEF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0xBB, 0xBE, 0xEE, 0xF4, 0x09, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xEE, 0xDF, 
	0x80, 0x38, 0x2E, 0xFB, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xE0, 0xDF, 
	0xBB, 0xBE, 0xEE, 0xFD, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xDF, 
	0xBB, 0xBE, 0xEE, 0xFD, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x07, 
	0xBB, 0xB8, 0xE0, 0xFE, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xEF, 
	0x80, 0x3F, 0x3F, 0xFE, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 
};

unsigned char Bmp_Show_JiFen[] = 
{
    0x42, 0x4D, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 
    0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xF8, 
    0x00, 0x00, 0xFF, 0xF8, 0x00, 0x00, 0xDC, 0x78, 0x00, 0x00, 0xEF, 0xB8, 0x00, 0x00, 0xF7, 0xB8, 
    0x00, 0x00, 0xF7, 0xB8, 0x00, 0x00, 0xFB, 0xB8, 0x00, 0x00, 0xFB, 0xB8, 0x00, 0x00, 0xA0, 0x28, 
    0x00, 0x00, 0xDF, 0xD8, 0x00, 0x00, 0xEF, 0xB8, 0x00, 0x00, 0xEF, 0xB8, 0x00, 0x00, 0xF7, 0x78, 
    0x00, 0x00, 0xF7, 0x78, 0x00, 0x00, 0xFF, 0xF8, 0x00, 0x00, 0xFF, 0xF8, 0x00, 0x00, 
};

NETPAYPHONETOPUP TELCEL_SKUTable[] = {	
	{TELCEL,        20, "8469766750482"},  
	{TELCEL, 		30, "8469760100306"}, 
	{TELCEL,        50, "8469760100504"}, 
	{TELCEL,        100, "8469760101006"}, 
	{TELCEL,        200, "8469760102003"}, 
	{TELCEL,        300, "8469760103000"},  
	{TELCEL,        500, "8469760105004"}, 
	{-1,  -1, ""}           
};
NETPAYPHONETOPUP MOVISTAR_SKUTable[] = {	
	{MOVISTAR,    	20, "7779860100207"}, 
	{MOVISTAR, 		30, "7779860100306"},
	{MOVISTAR,      50, "7779860100504"},
	{MOVISTAR,  	60, "7779860100607"}, 
	{MOVISTAR,  	100, "7779860101006"}, 
	{MOVISTAR,      150, "7779860101501"}, 
	{MOVISTAR,      200, "7779860102003"}, 
	{MOVISTAR,      300, "7779860103000"}, 
	{MOVISTAR,      500, "7779860105004"},  
	{-1,  -1, ""}         
};


NETPAYPHONETOPUP IUSACEL_SKUTable[] = {
	{IUSACEL,		20, "7385830100302"},
	{IUSACEL,       30, "7385830100306"},
	{IUSACEL,     	50, "7385830100504"},
	{IUSACEL, 	  	100, "7385830101008"},
	{IUSACEL,  	 	200, "7385830102005"},
	{IUSACEL,   	300, "7385830103002"}, 
	{IUSACEL,       500, "7385830105006"}, 
	{-1,  -1, ""}          
};


NETPAYPHONETOPUP UNEFON_SKUTable[] = {	
	{UNEFON, 		30, "8578690100307"}, 
	{UNEFON,        50, "8578690100505"}, 
	{UNEFON,        150, "8578690101502"}, 
	{UNEFON,        200, "8578690102004"}, 
	{UNEFON,        300, "8578690103001"},  
	{UNEFON ,       500, "8578690105005"}, 
	{-1,  -1, ""}         
};

NETPAYPHONETOPUP NEXTEL_SKUTable[] = {	
	{NEXTEL, 		30, "7503004884001"}, 
	{NEXTEL,        50, "7503004884018"}, 
	{NEXTEL,        100, "7503004884025"}, 
	{NEXTEL,        200, "7503004884032"}, 
	{NEXTEL,        500, "7503004884049"}, 
	{-1,  -1, ""}          
};

TOTAL_TRANS AllTransTable[20];
TOTAL_TRANS SIGNALTransTable[5];

int   DirectIntoMenu = 0;
// end file