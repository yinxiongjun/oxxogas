#include <openssl/des.h>
#include <openssl/sha.h>
#include "posapi.h"
#include "posmacro.h"
#include "glbvar.h"
#include "st8583.h"
#include "tranfunclist.h"
#include "emvtranproc.h"
#include "emvapi.h"
#include "Menu.h"

static uint8_t sAuthData[16];			// The card issuer authentication data
static uint8_t sIssuerScript[300];		
static uint8_t sIssuerScriptRet[128];

static int   iAuthDataLen, iScriptLen;
static EmvScriptInfo stScriptInfo;
static uint8_t   sScriptBak[300];
static int   iCurScript, bHaveScript, iScriptBakLen, iScriptNums, sIssuerScriptRetlen;
int gl_FirstRunSelApp = 0x01;

const uint8_t g_InputPinLogoBmp[190] = {
    0x42, 0x4D, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x73, 0xF5, 0x33, 0xC0, 0x0F, 0xFF, 0xDF, 0xF7, 0x7B,
    0xF5, 0xBB, 0xDD, 0xEF, 0x7F, 0xAF, 0xF3, 0x03, 0xD5, 0xAB, 0xDD, 0xEF, 0x6F, 0xF7, 0xF5, 0x7B,
    0x84, 0x2B, 0xDD, 0xEF, 0x0F, 0xF7, 0xF7, 0x03, 0xF1, 0xAB, 0xFD, 0xFF, 0x68, 0x17, 0xF7, 0x7B,
    0xF4, 0x2B, 0x88, 0x1F, 0x6F, 0xF7, 0xF7, 0x03, 0xF5, 0xAB, 0xF3, 0xD6, 0x6C, 0x07, 0xF7, 0xFF,
    0x80, 0x3B, 0xD5, 0xD7, 0x6D, 0xDF, 0xC6, 0x00, 0xB7, 0xFF, 0xD6, 0xEF, 0x0D, 0xDF, 0xFF, 0xEF,
    0xD5, 0x09, 0x75, 0x6F, 0xBD, 0xDF, 0xFF, 0x03, 0xDE, 0xF7, 0xBB, 0xB7, 0xBD, 0xDF, 0xF7, 0xEF,
    0x81, 0x6F, 0x80, 0x07, 0xBF, 0xDF, 0xF6, 0x01, 0xEF, 0x9F, 0xFE, 0xFE, 0x04, 0x0F, 0xEF, 0xEF,
    0xEF, 0xBF, 0xFD, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// balance/sale/prauth,field 55 EMV tag
static Def55EmvTag stOnlineTxnTagList[] =
{

	{0x5F2A, DE55_MUST_SET},
	{0x82,   DE55_MUST_SET},
	{0x84,   DE55_OPT_SET},
	{0x95,   DE55_MUST_SET},
	{0x9A,   DE55_MUST_SET},
	{0x9C,   DE55_MUST_SET},
	{0x9F02, DE55_MUST_SET},
	{0x9F03, DE55_MUST_SET},
	{0x9F09, DE55_OPT_SET},	
	{0x9F10, DE55_MUST_SET},
	{0x9F1A, DE55_MUST_SET},
	{0x9F1E, DE55_COND_SET},	
	{0x9F26, DE55_MUST_SET},
	{0x9F27, DE55_MUST_SET},
	{0x9F33, DE55_MUST_SET},
	{0x9F34, DE55_OPT_SET},
	{0x9F35, DE55_OPT_SET},
	{0x9F36, DE55_MUST_SET},
	{0x9F37, DE55_MUST_SET},
	{0x9F41, DE55_OPT_SET},
	{0x9F53, DE55_OPT_SET},
	{0x5F34, DE55_COND_SET},

	//{0x4F, 	 DE55_OPT_SET}, //NETPAY
	//{0x9F63, DE55_COND_SET},
	//{0x9F74, DE55_COND_SET},  // EC

	{NULL_TAG_1},
};
#if 0
{
	{0x9F26, DE55_MUST_SET},
	{0x9F27, DE55_MUST_SET},
	{0x9F10, DE55_MUST_SET},
	{0x9F37, DE55_MUST_SET},
	{0x9F36, DE55_MUST_SET},
	{0x95,   DE55_MUST_SET},
	{0x9A,   DE55_MUST_SET},
	{0x9C,   DE55_MUST_SET},
	{0x9F02, DE55_MUST_SET},
	{0x5F2A, DE55_MUST_SET},
	{0x82,   DE55_MUST_SET},
	{0x9F1A, DE55_MUST_SET},
	{0x9F03, DE55_MUST_SET},
	{0x9F33, DE55_MUST_SET},
	{0x9F34, DE55_OPT_SET},
	{0x9F35, DE55_OPT_SET},
	{0x9F1E, DE55_COND_SET},
	{0x84,   DE55_OPT_SET},
	{0x9F09, DE55_OPT_SET},
	{0x9F41, DE55_OPT_SET},
	{0x4F, 	 DE55_OPT_SET}, //NETPAY
	{0x9F63, DE55_COND_SET},
	{0x9F74, DE55_COND_SET},  // EC
//	{0x5A, DE55_COND_SET},
	{0x5F34, DE55_COND_SET},
	{0x9F53, DE55_OPT_SET},
/*	{0x9F12, DE55_COND_SET},
	{0x50, DE55_COND_SET},
	{0x5F30, DE55_COND_SET},
	{0x5F20, DE55_COND_SET},
	{0x57, DE55_COND_SET},
	{0x9F74, DE55_COND_SET},
	{0x9B, DE55_COND_SET},
	{0x8A, DE55_COND_SET},
	{0x9F07, DE55_COND_SET},
	{0x9F0E, DE55_COND_SET},
	{0x9F0F, DE55_COND_SET},
	{0x9F01, DE55_COND_SET},
	{0x5F25, DE55_COND_SET},
	{0x5F24, DE55_COND_SET},
	{0x9F15, DE55_COND_SET},
	{0x9F16, DE55_COND_SET},
	{0x9F39, DE55_COND_SET},
	{0x9F1C, DE55_COND_SET},
	{0x5F28, DE55_COND_SET},
	{0x9F21, DE55_COND_SET},*/
	{NULL_TAG_1},
};
#endif
// no response,MAC fail,field 55 EMV tag
static Def55EmvTag stReversalTagList[] =
{
	{0x95,   DE55_COND_SET},
	{0x9F1E, DE55_COND_SET},
	{0x9F10, DE55_COND_SET},
//	{0x9F36, DE55_COND_SET},
	{0xDF31, DE55_COND_SET},
	{NULL_TAG_1},
};

// ARPC fail,no ARPC,field 55 EMV tag
static Def55EmvTag stARPCReversalTagList[] =
{
	{0x95,   DE55_COND_SET},
	{0x9F1E, DE55_COND_SET},
	{0x9F10, DE55_COND_SET},
	{0x9F36, DE55_COND_SET},
	{0xDF31, DE55_COND_SET},
	{NULL_TAG_1},
};

// script result send,field 55 EMV tag
static Def55EmvTag stScriptResultTagList[] =
{
	{0x9F33, DE55_COND_SET},
	{0x95,   DE55_MUST_SET},
	{0x9F37, DE55_COND_SET},
	{0x9F1E, DE55_COND_SET},
	{0x9F10, DE55_MUST_SET},
	{0x9F26, DE55_MUST_SET},
	{0x9F36, DE55_MUST_SET},
	{0x82,   DE55_MUST_SET},
	{0xDF31, DE55_MUST_SET},
	{0x9F1A, DE55_MUST_SET},
	{0x9A,   DE55_MUST_SET},
	{NULL_TAG_1},
};

uint8_t gl_TmSupLang[8] = {0x65, 0x6E, 0x45, 0x4E, 0x7A, 0x68, 0x5A, 0x48};
uint8_t szSaveICCFail;

ST_EMVPRINT_INFO gstIccPrintData;

#ifdef _POS_TYPE_8110
int emv_candidate_apps_selection(void)
{
    int  Ret = 0;
    uint8_t CandListNum=0;
    int  j=0, Cursor=0;
    struct candapp CandApp[32];
    struct terminal_mck_configure tMckConfig;

    emv_get_mck_configure(&tMckConfig); 
    emv_get_all_candapps(&CandListNum, CandApp); 
    if( CandListNum==0 )
    {
		errno = EMV_ERRNO_NO_APP;
		return -1;
    }

	// netpay
/*    if( !gl_FirstRunSelApp )
    {
        lcdClrLine(2, 7);	
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "TRY AGAIN?");
		lcdFlip();
        Ret = kbGetKey();
        if( Ret!=KEY_ENTER )
        {
			errno = EMV_ERRNO_CANCEL;
			return -1;
        }
    }
*/
    if( CandListNum==1 )
    {
        if( gl_FirstRunSelApp )
        {
            if( CandApp[0].cFlgAPID )
            {
                if( ((CandApp[0].cAPID & 0x80)==0x80) && tMckConfig.support_cardholder_confirmation )
                {
                    lcdCls();	
					DispTransName();
					lcdDisplay(0, 2, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[0].tCandAppName);
					lcdDisplay(0, 4, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[ENTER]-YES");
					lcdDisplay(0, 6, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[CANCEL]-NO");
					lcdFlip();
                    Ret = kbGetKey();
                    if( Ret!=KEY_ENTER )
                    {
						errno = EMV_ERRNO_CANCEL;
						return -1;
                    }
                }
                else if( ((CandApp[0].cAPID & 0x80)==0x80) && !tMckConfig.support_cardholder_confirmation )
                {
					errno = EMV_ERRNO_NO_APP;
                    return -1;
                }
            }
            gl_FirstRunSelApp = 0;
        }
        else
        {
            lcdCls();
			lcdDisplay(0, 2, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[0].tCandAppName);
			lcdDisplay(0, 4, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[ENTER]-YES");
			lcdDisplay(0, 6, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[CANCEL]-NO");
			lcdFlip();
            Ret = kbGetKey();
			if( Ret!=KEY_ENTER )
			{
				errno = EMV_ERRNO_CANCEL;
				return -1;
            }
        }
        return 0;
    }

    if( !tMckConfig.support_cardholder_confirmation )
    {
        for(j=0; j<CandListNum; j++)
        {
            if( CandApp[j].cFlgAPID && (CandApp[j].cAPID & 0x80) )
            {
                continue;
            }
            else
            {
                gl_FirstRunSelApp = 0;
                return j;
            }
        }
    }

    lcdCls();	
    kbFlush();
    if( CandListNum>1 )
    {
        lcdSetIcon(ICON_DOWN, ON);
    }
    else
    {
        lcdSetIcon(ICON_DOWN, OFF);
    }
    lcdSetIcon(ICON_UP, OFF);
    j = 0;

	
    while(1)
    {
        lcdCls();	
		lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "    APP LIST    ");
        if( j<CandListNum )
        {
            if( (Cursor%3)==0 )
            {
                lcdDisplay(0, 2, DISP_INVLINE|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[j].tCandAppName);
            }
            else
            {
                lcdDisplay(0, 2, DISP_CLRLINE|DISP_CFONT, "%s", CandApp[j].tCandAppName);
            }
        }

        if( (j+1)<CandListNum )
        {
            if( (Cursor%3)==1 )
            {
                lcdDisplay(0, 4, DISP_INVLINE|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[j+1].tCandAppName);
            }
            else
            {
                lcdDisplay(0, 4, DISP_CLRLINE|DISP_CFONT, "%s", CandApp[j+1].tCandAppName);
            }
        }

        if( (j+2)<CandListNum )
        {
            if( (Cursor%3)==2 )
            {
                lcdDisplay(0, 6, DISP_INVLINE|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[j+2].tCandAppName);
            }
            else
            {
                lcdDisplay(0, 6, DISP_CLRLINE|DISP_CFONT, "%s", CandApp[j+2].tCandAppName);
            }
        }
		lcdFlip();
        Ret = kbGetKey();
		sysDelayMs(50);


        switch(Ret)
        {
        case KEY_DOWN:
            if( Cursor < (CandListNum-1) )
            {
                Cursor++;
                lcdSetIcon(ICON_UP, ON);
                if( Cursor==(CandListNum-1) )
                {
                    lcdSetIcon(ICON_DOWN, OFF);
                }
            }

            j = (Cursor/3)*3;
            break;
        case KEY_UP:
            if( Cursor!=0 )
            {
                Cursor--;
                lcdSetIcon(ICON_DOWN, ON);
            }

            if( Cursor==0 )
            {
                lcdSetIcon(ICON_UP, OFF);
            }

            j = (Cursor/3)*3;
            break;
        case KEY_ENTER:
            gl_FirstRunSelApp = 0;
			errno = 0;
            return Cursor;
        case KEY_CANCEL:
			errno = EMV_ERRNO_CANCEL;
            return -1;
        default:
            break;
        }
    }
}
#else
int emv_candidate_apps_selection(void)
{
    int  Ret = 0, iCnt;
    uint8_t CandListNum=0;
    int  j=0, iCupAppNum, iCupAppindex;
    struct candidate_list_app CandApp[MAX_APP_NUM]; 
    struct terminal_mck_configure tMckConfig;
	MenuItem	    stAppMenu[MAX_APP_NUM+1];
	
    emv_get_mck_configure(&tMckConfig);

	memset((char*)&CandApp[0],0,sizeof(CandApp));
    emv_get_candidate_list(&CandListNum, CandApp);
    if( CandListNum==0 )
    {
		errno = EMV_ERRNO_NO_APP;
		return -1;
    }

/*    if( !gl_FirstRunSelApp )
    {
        lcdClrLine(2, 7);	
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "TRY AGAIN?");
		lcdFlip();
        Ret = kbGetKey();
        if( Ret!=KEY_ENTER )
        {
			errno = EMV_ERRNO_CANCEL;
			return -1;
        }
    }
*/
	//according to union,wild card and unionpay card exist at the same time,select unionpay card directly
	iCupAppNum = iCupAppindex = 0;
	for(j=0; j<CandListNum; j++)
	{
		if(memcmp(CandApp[j].AID,"A000000333",10)==0 || memcmp(CandApp[j].AID,"a000000333",10)==0)
		{
			iCupAppNum++;
			iCupAppindex = j;
		}
	}		
	if (iCupAppNum==1)
	{
		CandListNum = 1;
		CandApp[0] = CandApp[iCupAppindex];
	}

    if( CandListNum==1 )
    {
        if( CandApp[0].cFlgAPID )  // cardhodler enter
        {
            if( ((CandApp[0].cAPID & 0x80)==0x80) && tMckConfig.support_cardholder_confirmation )
            {
                lcdCls();	
				DispTransName();
				lcdDisplay(0, 2, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[0].tCandAppName);
				lcdDisplay(0, 4, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[ENTER]-YES");
				lcdDisplay(0, 6, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[CANCEL]-NO");
				lcdFlip();
                Ret = kbGetKey();
                if( Ret!=KEY_ENTER )
                {
					errno = EMV_ERRNO_CANCEL;
					return -1;
                }
            }
            else if( ((CandApp[0].cAPID & 0x80)==0x80) && !tMckConfig.support_cardholder_confirmation )
            {
				errno = EMV_ERRNO_NO_APP;
                return -1;
            }
        }
        gl_FirstRunSelApp = 0;
		if (iCupAppNum==1)
			return iCupAppindex;
		else
			return 0;
    }

	memset(stAppMenu, 0, sizeof(stAppMenu));
	for(iCnt=0; iCnt<CandListNum && iCnt<MAX_APP_NUM; iCnt++)
	{
		stAppMenu[iCnt].bVisible = TRUE;
		stAppMenu[iCnt].pfMenuFunc = NULL;
		sprintf((char *)stAppMenu[iCnt].szMenuName, "%.16s", CandApp[iCnt].tCandAppName);
	}	
	stAppMenu[iCnt].bVisible = FALSE;
	stAppMenu[iCnt].pfMenuFunc = NULL;
	stAppMenu[iCnt].szMenuName[0] = 0;
	
	Ret = PubGetMenu((UINT8 *)"   Select App   ", stAppMenu, MENU_CFONT, 0);
	if( Ret<0 || Ret+1>CandListNum)
	{
		lcdCls();	
		DispTransName();
		errno = EMV_ERRNO_CANCEL;
        return -1;
	}

	gl_FirstRunSelApp = 0; // have chosen
	if( CandApp[Ret].cFlgAPID )
	{
		if( ((CandApp[Ret].cAPID & 0x80)==0x80) && tMckConfig.support_cardholder_confirmation )
		{
			lcdCls();	
			DispTransName();
			lcdDisplay(0, 2, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "%s", CandApp[Ret].tCandAppName);
			lcdDisplay(0, 4, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[ENTER]-YES");
			lcdDisplay(0, 6, DISP_MEDIACY|DISP_CLRLINE|DISP_CFONT, "[CANCEL]-NO");
			lcdFlip();
			Ret = kbGetKey(); 
			if( Ret!=KEY_ENTER )
			{
				errno = EMV_ERRNO_CANCEL;
				return -1;
			}
		}
		else if( ((CandApp[Ret].cAPID & 0x80)==0x80) && !tMckConfig.support_cardholder_confirmation )
		{
			errno = EMV_ERRNO_NO_APP;
			return -1;
		}
	}

	return Ret;
}
#endif

uint8_t EcAppQue(void)//Whether the EC card
{
	uint8_t    ucLen;
	int ret,i;
	uint8_t buf[100];

	ret=emv_get_data_element("\x9F\x38",2,&ucLen,buf);
	if(ret != EMV_RET_SUCCESS) return NO;
	if(ucLen == 0) return NO;
	for(i=0;i<=ucLen-3;i++){
		if(memcmp(buf+i,"\x9f\x7a\x01",3)==0) return YES;
	}
	return NO;
}

void CheckEcCard(void)
{
	uint8_t buf[64];

	if(stTemp.iTransNo != POS_SALE) return ;
	if((EcAppQue() != YES) || (PosCom.ucPBOCFlag==2))
	{
		emv_pboc_ec_enable(0); //close the ec function
		return ; 
	}
	if(PosCom.ucPBOCFlag==1)//ec
	{
		stTemp.iTransNo = EC_NORMAL_SALE;
		PosCom.stTrans.iTransNo = EC_NORMAL_SALE;
		emv_pboc_ec_enable(1);
		DispTransName();
		return;
	}

	PubAsc2Bcd((char *)PosCom.stTrans.szAID,strlen((char *)PosCom.stTrans.szAID),(char *)buf);
	if(memcmp(buf,"\xA0\x00\x00\x03\x33\x01\x01\x06",8) == 0) 
	{
		stTemp.iTransNo = EC_NORMAL_SALE;
		emv_pboc_ec_enable(1);
		DispTransName();
		return ;
	}
	else	//main meun,Priority PBOC
	{
		emv_pboc_ec_enable(0);
		return;
	}
}

void emv_multi_language_selection(void)
{
	uint8_t uLen;
	uint8_t bBuffer[20];
	
	ModifyTermCapForApp();// before GPO
	emv_get_data_element("\x4F", 1, &uLen, bBuffer);	// AID
	BcdToAsc0(PosCom.stTrans.szAID, bBuffer, uLen*2);
	CheckEcCard();
}

int emvCheckCRL(uint8_t index,const void *rid,const void *cert_sn)
{
	errno = EMV_ERRNO_NODATA;
	return -1;
}

int emvCheckExceptPAN(uint8_t PANLen, const void *PAN, uint8_t PANSn)
{	
	char  szCardNo[21];

	if (PosCom.stTrans.iTransNo != EC_QUICK_SALE)
	{
		errno = EMV_ERRNO_NODATA;
		return -1;
	}

	memset(szCardNo,0,sizeof(szCardNo));
	BcdToAsc0((uint8_t*)szCardNo, (uint8_t*)PAN, PANLen*2);
	RemoveTailChars((uint8_t*)szCardNo, 'F');		// erase padded 'F' chars

	if (ChkIfBlkCard((char*)szCardNo))
	{
		return 0;
	}

	errno = EMV_ERRNO_NODATA;
	return -1;
}

int emvGetTransAmount(int PANLen, uint8_t *PAN, uint8_t PANSn, uint32_t *Amount)
{
	int ucRet;    
	int iRecNo=0;
	NEWPOS_LOG_STRC	stLog;
	uint8_t szCardTemp[21];
	int iAmtTmp=0;

	if((PAN == NULL) || (Amount == NULL) || (PANLen > 10))
	{
		errno = EMV_ERRNO_INVAL;
		return -1;
	}

	*Amount = 0;
	if( stTransCtrl.iTransNum==0 )
	{
		errno = EMV_ERRNO_NODATA;
		return -1;
	}
	
	memset(szCardTemp, 0, sizeof(szCardTemp));
	BcdToAsc0(szCardTemp, PAN, PANLen*2);
	for(iRecNo=stTransCtrl.iTransNum-1; iRecNo>=0; iRecNo--)
	{
		memset(&stLog, 0, sizeof(stLog));
		ucRet = LoadTranLog(iRecNo, &stLog);
		if( ucRet!=OK )
		{
			lcdCls();	
			errno = EMV_ERRNO_INVAL;
			return ucRet;
		}
		
		if( PANLen==10 )
		{
			if( memcmp(szCardTemp, stLog.szCardNo, 19)==0 )      
			{
				iAmtTmp = BcdToLong(stLog.sAmount, 6);
				if(stTransCtrl.sTxnRecIndexList[iRecNo]&TS_ICC_OFFLINE)
					*Amount += iAmtTmp;
			}
		}
		else
		{
			if( memcmp(szCardTemp, stLog.szCardNo, PANLen*2)==0 )      
			{
				iAmtTmp = BcdToLong(stLog.sAmount, 6);
				if(stTransCtrl.sTxnRecIndexList[iRecNo]&TS_ICC_OFFLINE)
					*Amount += iAmtTmp;
			}
		}
	}

	if(*Amount != 0)
		return EMV_RET_SUCCESS;
	
	errno = EMV_ERRNO_NODATA;
	
	return -1;
}

int emvGetTransLogAmount(uint8_t PanLen, const void *PAN, uint8_t PanSn)
{
	int Ret;
	uint32_t Amount=0;

	Ret = emvGetTransAmount(PanLen,(uint8_t *)PAN,PanSn,&Amount);
	if( Ret==EMV_RET_SUCCESS )
	{
		return Amount;
	}
	else
	{
		return 0;
	}
}

//input amount
int emvInputTransAmount(uint32_t *pTransAmount)
{
	
	*pTransAmount = (uint32_t)BcdToLong(PosCom.stTrans.sAmount, 6);
	emv_set_data_element("\x9F\x03", 2, 6, "\x00\x00\x00\x00\x00\x00");

	return EMV_RET_SUCCESS;
}

int emvInputCashBackAmount(uint32_t *pCashbackAmount)
{
	
	if( pCashbackAmount!=NULL )
	{
		*pCashbackAmount = 0;
	}
	else
	{	// set CASHBACK amount
		emv_set_data_element("\x9F\x03", 2, 6, "\x00\x00\x00\x00\x00\x00");
		emv_set_data_element("\x9F\x04", 2, 4, "\x00\x00\x00\x00");
	}
	
	return EMV_RET_SUCCESS;
}

void LongToStr(uint32_t ldat, uint8_t *str)
{
    str[0] = (uint8_t)(ldat >> 24);
    str[1] = (uint8_t)(ldat >> 16);
    str[2] = (uint8_t)(ldat >> 8);
    str[3] = (uint8_t)(ldat);
}

void  auxLongToBCD(uint8_t* pStr, int  cLen, uint32_t lLng)
{
	int   i;
	uint8_t   cBR;

	memset(pStr, 0x00, cLen);
	for(i=(cLen-1); i>=0; i--)
	{
		cBR  = (uint8_t)(lLng%10);
		lLng = lLng/10;
		pStr[i] = (uint8_t)(lLng%10);
		pStr[i] = pStr[i]*0x10 + cBR;
		lLng = lLng/10;
	}
	return;
}

int emvGetOnlinePIN(void)
{
	int	iResult;

	// online pin
	iResult = EnterPIN(0x80);
	if( iResult==OK )
	{
		return 0;
	}
	else if( iResult==PED_RET_NOPIN )
	{
		errno = EMV_ERRNO_BYPASS_PIN;
		return -1;
	}
	else if( iResult==E_TRANS_CANCEL )
	{
		errno = EMV_ERRNO_CANCEL;
		return -1;
	}
	else
	{
		errno = EMV_ERRNO_DATA;
		return -1;
	}
}

int emv_check_online_pin(void)
{
    int Ret = EMV_RET_SUCCESS;
	int iErrNo;
  
	if (PosCom.stTrans.iTransNo== EC_TOPUP_SPEC || PosCom.stTrans.iTransNo== EC_TOPUP_NORAML || 
		PosCom.stTrans.iTransNo== EC_VOID_TOPUP)
	{
		if (PosCom.ucPinEntry)
			return 0;
		else{
			errno = EMV_ERRNO_BYPASS_PIN;
			return -1;
		}
	}

    Ret = emvGetOnlinePIN();
	iErrNo = Ret?errno:0;
    switch(iErrNo)
    {
    case PED_RET_APPTSSAKERR:
		errno = EMV_ERRNO_DATE;
        break;
    case PED_RET_NOPIN:
		errno = EMV_ERRNO_BYPASS_PIN;
        break;
    case EMV_ERRNO_CANCEL:
		errno = EMV_ERRNO_CANCEL;
        break;
    case PED_RET_OK:
		return 0;
    default:
		break;
    }

    return -1;
}

uint32_t asc2ulong(uint8_t *asc)
{
    int i, blen;
    uint32_t dwTmp;

    blen = strlen((char *)asc);
    dwTmp = 0;

    for(i=0; i<blen; i++)
    {
        if((asc[i] < '0') || (asc[i] > '9'))
            return -1;
        dwTmp = dwTmp*10 + (asc[i] - 0x30);
    }
    return dwTmp;
}


int JudgeEcCard(void)
{
	uint8_t szBuffer[20];
	int     iCnt;

	if (stTemp.iTransNo == EC_NORMAL_SALE)
	{
		PubAsc2Bcd((char *)PosCom.stTrans.szAID,strlen((char *)PosCom.stTrans.szAID),(char *)szBuffer);
		for (iCnt=0; iCnt< MAX_APP_NUM; iCnt++)
		{
			if (memcmp(szBuffer,glTermAidList[iCnt].sAID,strlen((char *)PosCom.stTrans.szAID)/2) == 0)
			{
				break;
			}
		}
		if (iCnt >= MAX_APP_NUM)
		{
			emv_pboc_set_ec_ttl(stPosParam.iEcMaxAmount); // set max amount
		}
		else
		{	
			emv_pboc_set_ec_ttl(glTermAidList[iCnt].m_EcMaxTxnAmount); // set max amount
			if (glTermAidList[iCnt].m_EcMaxTxnAmount == 0)
			{
				emv_pboc_set_ec_ttl(stPosParam.iEcMaxAmount);
			}
		}
	}
	return 0;
}

uint8_t SelectTipPercent(uint8_t *BcdNormalAmount)
{
	int iRet;
	char buf[10];
	long amt;
	long lNormalAmount;
	long lTipAmount;
	char AscAmount[20];
	char AscNormalAmount[20];
	int  key;
//	long lTotalAmt;

	lcdClrLine(4, 9);
	DispMulLanguageString(0, 4, DISP_HFONT16, NULL, "Elija el porcentaje de propina");
	DispMulLanguageString(210, 5, DISP_HFONT16, NULL, "1-0%%");
	DispMulLanguageString(210, 6, DISP_HFONT16, NULL, "2-10%%");
	DispMulLanguageString(210, 7, DISP_HFONT16, NULL, "3-15%%");
	DispMulLanguageString(210, 8, DISP_HFONT16, NULL, "4-20%%");
	DispMulLanguageString(210, 9, DISP_HFONT16, NULL, "5-Otra");
	lcdFlip();

	stPosParam.percent = 0;
	do {
		key = PubWaitKey(60);
	}while(key != KEY_ENTER && key != KEY_CANCEL && key != KEY_TIMEOUT &&
	key != KEY1 && key != KEY2 && key != KEY3 && key != KEY4 &&
		key != KEY5);
	switch(key)
	{
	case KEY1:
		stPosParam.percent = 0;
		break;
	case KEY2:
		stPosParam.percent = 10;
		PubBcd2Asc0(BcdNormalAmount,6,AscNormalAmount);
		lNormalAmount = atol((char*)AscNormalAmount);
		lTipAmount = lNormalAmount * stPosParam.percent/100;
		PosCom.stTrans.lpreTipAmount = lTipAmount;    //save tip amount
		lNormalAmount += lTipAmount;
		sprintf(AscAmount,"%010ld%02ld\n",lNormalAmount/100,lNormalAmount%100);
		AscToBcd(BcdNormalAmount,(uint8_t*)AscAmount,12);

		memset(AscAmount,0,sizeof(AscAmount));
		memset(PosCom.stTrans.preTipAmount,0,sizeof(PosCom.stTrans.preTipAmount));
		sprintf(AscAmount,"%010ld%02ld\n",lTipAmount/100,lTipAmount%100);
		AscToBcd(PosCom.stTrans.preTipAmount,(uint8_t*)AscAmount,12);

		break;
	case KEY3:
		stPosParam.percent = 15;
		PubBcd2Asc0(BcdNormalAmount,6,AscNormalAmount);
		lNormalAmount = atol((char*)AscNormalAmount);
		lTipAmount = lNormalAmount * stPosParam.percent/100;
		PosCom.stTrans.lpreTipAmount = lTipAmount;    //save tip amount
		lNormalAmount += lTipAmount;
		sprintf(AscAmount,"%010ld%02ld\n",lNormalAmount/100,lNormalAmount%100);
		AscToBcd(BcdNormalAmount,(uint8_t*)AscAmount,12);

		memset(AscAmount,0,sizeof(AscAmount));
		memset(PosCom.stTrans.preTipAmount,0,sizeof(PosCom.stTrans.preTipAmount));
		sprintf(AscAmount,"%010ld%02ld\n",lTipAmount/100,lTipAmount%100);
		AscToBcd(PosCom.stTrans.preTipAmount,(uint8_t*)AscAmount,12);
		break;
	case KEY4:
		stPosParam.percent = 20;
		PubBcd2Asc0(BcdNormalAmount,6,AscNormalAmount);
		lNormalAmount = atol((char*)AscNormalAmount);
		lTipAmount = lNormalAmount * stPosParam.percent/100;
		PosCom.stTrans.lpreTipAmount = lTipAmount;    //save tip amount
		lNormalAmount += lTipAmount;
		sprintf(AscAmount,"%010ld%02ld\n",lNormalAmount/100,lNormalAmount%100);
		AscToBcd(BcdNormalAmount,(uint8_t*)AscAmount,12);

		memset(AscAmount,0,sizeof(AscAmount));
		memset(PosCom.stTrans.preTipAmount,0,sizeof(PosCom.stTrans.preTipAmount));
		sprintf(AscAmount,"%010ld%02ld\n",lTipAmount/100,lTipAmount%100);
		AscToBcd(PosCom.stTrans.preTipAmount,(uint8_t*)AscAmount,12);
		break;
	case KEY5:
		lcdClrLine(4, 10);	
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "INDIQUE PROPINA");
		DispMulLanguageString(0, 5, DISP_HFONT16|DISP_MEDIACY, NULL, "PROPINA NO PUEDE EXCEDER 25%%");
		lcdFlip();

		memset(buf, 0, sizeof(buf));
		PubBcd2Asc0(BcdNormalAmount,6,AscNormalAmount);
		lNormalAmount = atol((char*)AscNormalAmount);
		do{
			lcdGoto(120, 6);
			iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
			if( iRet<0 )
				return E_TRANS_CANCEL;
			amt = atol((char *)buf);
			if(amt > lNormalAmount * 0.25){
				lcdClrLine(4,10);
				DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "PROPINA SUPERIOR");
				DispMulLanguageString(0, 6, DISP_HFONT16|DISP_MEDIACY, NULL, "A LA PERMITIDA");
				lcdFlip();
				PubBeepErr();
				PubWaitKey(3);
				return NO_DISP;
			}
			else{
				lNormalAmount += amt;
				sprintf(AscAmount,"%010ld%02ld\n",lNormalAmount/100,lNormalAmount%100);
				PosCom.stTrans.lpreTipAmount = atol(AscAmount);    //save tip amount
				AscToBcd(BcdNormalAmount,(uint8_t*)AscAmount,12);

				memset(AscAmount,0,sizeof(AscAmount));
				memset(PosCom.stTrans.preTipAmount,0,sizeof(PosCom.stTrans.preTipAmount));
				sprintf(AscAmount,"%010ld%02ld\n",amt/100,amt%100);
				AscToBcd(PosCom.stTrans.preTipAmount,(uint8_t*)AscAmount,12);
				break;
			}
		}while(1);
		
		break;
	
	default:
		return NO_DISP;
		break;	
	}

	return OK;
}

int emv_get_amount(uint32_t *pTransAmount, uint32_t *pOtherAmount)
{
	int iRet=0;
	int iAmount;

	lcdCls();
	DispTransName();
	if (stTemp.iTransNo == EC_NORMAL_SALE)
	{
		if (CheckSupport(EC_NORMAL_SALE)==FALSE)
		{
			lcdClrLine(2,7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " NOT SUPPORT ");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(3);
			errno = E_TRANS_FAIL;
			return -1;
		}
		if(EcAppQue() != YES)
		{
			lcdClrLine(2,7);
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"NO EC CARD");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(3);
			errno = E_TRANS_FAIL;
			return -1;
		}
	}

	if ((stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.ucTransferFlag)) 
	{
		if( pTransAmount )
		{
			*pTransAmount = 0;
		}
		if( pOtherAmount )
		{
			*pOtherAmount = 0;
		}
		return 0;
	}
#if 0
	if( stTemp.iTransNo!=POS_QUE && stTemp.iTransNo!=POS_SALE_VOID &&
		stTemp.iTransNo!=POS_AUTH_VOID && stTemp.iTransNo != EC_REFUND 
		&& stTemp.iTransNo != POS_REFUND && stTemp.iTransNo != POS_OFF_CONFIRM
		&& stTemp.iTransNo != POS_AUTH_CM && stTemp.iTransNo != POS_PREAUTH_VOID &&
		PosCom.stTrans.TransFlag != PRE_TIP_SALE && stTemp.iTransNo != CHECK_IN &&
		stTemp.iTransNo != CHECK_OUT && stTemp.iTransNo != CASH_ADVACNE && 
		stTemp.iTransNo!=NETPAY_REFUND&&stTemp.iTransNo!=NETPAY_ADJUST)
	{
		iAmount = BcdToLong(PosCom.stTrans.sAmount, 6);
		if ((iAmount != 0 &&  gl_FirstRunSelApp == 1) || 
			(PosCom.stTrans.TransFlag == PAYMENT_WITH_CARD && iAmount != 0))   //netpay flow 
		{
			*pTransAmount = 0;
			*pTransAmount = iAmount;
			return JudgeEcCard();
		}

		iRet = AppGetAmount(10, IC_AMOUNT);
		if( iRet!=OK )
		{
			errno = E_TRANS_CANCEL;
			return -1;
		}
	}
#endif
	if( pTransAmount )
	{
		*pTransAmount = 0;
		*pTransAmount = BcdToLong(PosCom.stTrans.sAmount, 6);
	}
	if( pOtherAmount )
	{
		*pOtherAmount = 0;
	}

	return JudgeEcCard();
}


int cEMVUnknowTLVData(uint16_t iTag, uint8_t *psDat, int iDataLen)
{
	switch( iTag )
	{
	case 0x9F53:	 
		*psDat = 0x52;
		break;

	default:
		return -1;
	}

	return EMV_RET_SUCCESS;
}

int emvCertCheck(void)
{
	return EMV_RET_SUCCESS;
}


int emv_issuer_referral_process(void)
{
	errno = EMV_ERRNO_DECLINE;
	return -1;
/*
	uint8_t len;
	uint8_t pan[100], buff[200];
	int rc;

	memset(pan, 0x00, sizeof(pan));
	memset(buff, 0x00, sizeof(buff));
	len = 0;
	emv_get_data_element("\x5A", 1, &len, buff);
	BcdToAsc(pan, buff, len*2);

	lcdCls();	
	if(gl_SupCN == 0) // if no support chinese
	lcdDisplay(0,0,DISP_CFONT|DISP_MEDIACY,"CALL YOUR BANK");
	lcdDisplay(0,3,0,"APP PAN:");
	lcdDisplay(0,5,0,"%s", pan);
	lcdDisplay(0,7,DISP_ASCII,"Enter to Continue");
	lcdFlip();
	kbGetKey();
	lcdCls();	
	lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"ACCEPT TRANS?");

	lcdFlip();
	while(1)
	{
		rc = kbGetKey();
		if(rc == KEY_ENTER)
		{
			return 0;
		}

		if(rc == KEY_CANCEL)
		{
			errno = EMV_ERRNO_DECLINE;
			return -1;  // Decline the transaction
		}
	}
*/
}

// unionpay detect advice by reversal
int emv_advice_process(int Flg)
{
	struct terminal_mck_configure tMckConfig;

	emv_get_mck_configure(&tMckConfig);  // Get current emv kernel MCK configuration

	if(!tMckConfig.support_advices) // the MCK do not support advice
	{
		return 0;
	}


	return 0;
}

//online
int  emv_online_transaction_process(uint8_t *psRspCode,  uint8_t *psAuthCode, int *piAuthCodeLen,
					uint8_t *psAuthData, int *piAuthDataLen,
					uint8_t *psScript,   int *piScriptLen, int *online_result)
{
	uint8_t	ucRet, bOnlineTxnBak;
	int	iLength, PinEntryTimes, iResult;
	uint32_t	ulICCDataLen;
	uint8_t	*psICCData, *psTemp;

	*psRspCode      = 0;
	*piAuthCodeLen  = 0;
	*piAuthDataLen  = 0;
	*piScriptLen    = 0;
	PosCom.bIsFirstGAC = FALSE;

	emv_del_data_element("\x89", 1);
	emv_del_data_element("\x8A", 1);
	emv_del_data_element("\x91", 1);

	if (PosCom.stTrans.iTransNo == EC_QUICK_SALE)
	{
		if (ChkOnlyEcCard(PosCom.stTrans.szAID))// pure ec card not support online
		{
			errno = EMV_ERRNO_DECLINE;
			return -1;
		}
    		ledSetStatus(LED_TRANS_HANDLING);
		AfterProcQpboc();	
		PosCom.stTrans.iTransNo = QPBOC_ONLINE_SALE;
		stTemp.iTransNo = QPBOC_ONLINE_SALE;
		DispTransName();

		iResult = EnterPIN(0);
		if(iResult)
		{
      			ledSetStatus(LED_TRANS_FAIL);
			DispEppPedErrMsg(iResult);
			errno = EMV_ERRNO_DECLINE;
			return -1;
		}

		ledSetStatus(LED_ONLINE_HANDLING);
		SetCommReqField((uint8_t *)"0200",(uint8_t *)"000000");
		
	}

	if (PosCom.stTrans.iTransNo == EC_NORMAL_SALE || PosCom.ucPBOCFlag==2)
	{
		if (ChkOnlyEcCard(PosCom.stTrans.szAID))// pure ec card not support online
		{
			szSaveICCFail = 0; //balance insufficient,not save fail log
			errno = EMV_ERRNO_DECLINE;
			return -1;
		}
    		ledSetStatus(LED_ONLINE_HANDLING);
		PosCom.stTrans.iTransNo = POS_SALE;
		stTemp.iTransNo = POS_SALE;		
		SetCommReqField((uint8_t *)"0200",(uint8_t *)"000000");
	}

	ucRet = SetOnlineTranDE55(stOnlineTxnTagList, &glSendPack.sICCData[2], &iLength);
	PrintDebug("%s%d", "SetOnlineTranDE55:",ucRet);

	
	if( ucRet!=OK )
	{
		errno = EMV_ERRNO_ONLINE_ERROR;
		PrintDebug("%s%d", "SetOnlineTranDE55:errno:",EMV_ERRNO_ONLINE_ERROR);
		return -1;
	}

	if(stPosParam.szpreTip == PARAM_OPEN && stTemp.iTransNo == POS_SALE &&
		PosCom.stTrans.TransFlag == PRE_TIP_SALE)
	{
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.preTipAmount, 12);	
	}
	if( stTemp.iTransNo == NETPAY_ADJUST && PosCom.stTrans.iOldTransNo== PRE_TIP_SALE)
	{
		BcdToAsc0(glSendPack.szExtAmount, PosCom.stTrans.preTipAmount, 12);	
	}

	ShortToByte(glSendPack.sICCData, (uint16_t)iLength);
	PubLong2Char((ulong)iLength, 2, (char *)glSendPack.sICCData);

	memcpy(PosCom.stTrans.sIccData, &glSendPack.sICCData[2], iLength);	// save for batch upload
	PosCom.stTrans.iIccDataLen = (uint8_t)iLength;

	if( stTemp.iTransNo==POS_QUE || stTemp.iTransNo==POS_SALE )
	{	
//		ShortToByte(glSendPack.sField62, (uint16_t)strlen((char *)PosCom.szCertData));
//		sprintf((char *)&glSendPack.sField62[2], "%s", PosCom.szCertData);
	}

	UpdateEntryModeByPIN();	
	if( stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.stTrans.ucEcTopOldSwipeFlag == CARD_SWIPED) //transfer card
	{
		glSendPack.szEntryMode[1] = '2';
	}

	ucRet =SendScriptResult();
	PrintDebug("%s%d", "SendScriptResult:",ucRet);

	if (ucRet == E_SCRIPT_FAIL)//send script fail
	{
		errno = EMV_ERRNO_ONLINE_ERROR;
		return -1;
	}
	ucRet =ReverseTranProc();
	PrintDebug("%s%d", "ReverseTranProc:",ucRet);


	if (ucRet == E_REVERSE_FAIL)//send reversal fail
	{
		errno = EMV_ERRNO_ONLINE_ERROR;
		return -1;
	}
	
//	DispTransName();
	PinEntryTimes = 0;
	while(1)
	{
		bOnlineTxnBak = PosCom.bOnlineTxn;
		ucRet = SendRecvPacket();
	    	PosCom.bOnlineTxn = bOnlineTxnBak;
		if( ucRet!=OK )
		{
			UpdateReversalFile(ucRet); // update reversal file
			errno = EMV_ERRNO_ONLINE_ERROR;
			PosCom.bOnlineTxn = bOnlineTxnBak;
			return -2;
		}

		break;
	}

	memcpy(psRspCode,  glRecvPack.szRspCode,  LEN_RSP_CODE);
	if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0 )
	{
		fileRemove(REVERSAL_FILE);
	}
	PosCom.bOnlineTxn = TRUE;

	iAuthDataLen = iScriptLen = 0;
	iScriptNums = 0;

	Char2Long(glRecvPack.sICCData, 2, &ulICCDataLen);
	psICCData = &glRecvPack.sICCData[2];


	for(psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
	{
		iResult = GetTlvValue(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, TRUE);
		if( iResult<0 )
		{	
			*online_result = 1;
			errno = EMV_ERRNO_DECLINE;
			return -1;
		}
	}
	memcpy(psAuthData, sAuthData, iAuthDataLen);
	*piAuthDataLen = iAuthDataLen;
	AdjustIssuerScript();
	memcpy(psScript, sIssuerScript, iScriptLen);	
	*piScriptLen  = iScriptLen;

	if (stTemp.iTransNo == EC_TOPUP_CASH || stTemp.iTransNo == EC_TOPUP_SPEC 
		|| stTemp.iTransNo == EC_TOPUP_NORAML || stTemp.iTransNo == EC_VOID_TOPUP )
	{
		if (iScriptNums != 1)
		{
			*piScriptLen = 2;
			memcpy(psScript,"\x72\x00",2);	//clear script
			ucRet = E_SCRIPT_FAIL;			
			UpdateReversalFile(ucRet); 		
			errno = EMV_ERRNO_ONLINE_ERROR;
			*online_result = 1;
			return -1;
		}
	}

	if( memcmp(glRecvPack.szRspCode, "05", LEN_RSP_CODE)==0 ||
		memcmp(glRecvPack.szRspCode, "51", LEN_RSP_CODE)==0 )
	{
		*online_result = 1;
	}
	else if( memcmp(glRecvPack.szRspCode, "01", LEN_RSP_CODE)==0 )
	{
		*online_result = 2;		
	}
	else if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0 )
	{
		*online_result = 3;
		errno = EMV_ERRNO_DECLINE;
		return -1;
	}
	else 
	{
		*online_result = 0;
	}
	memcpy(psAuthCode, glRecvPack.szAuthCode, LEN_AUTH_CODE);
	*piAuthCodeLen = strlen((char *)glRecvPack.szAuthCode);

	return EMV_RET_SUCCESS;
}

//如果不需要提示密码验证成功，则直接返回就可以了
void emvDispPINOK(void)
{
	lcdClrLine(2, 7);	
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PIN OK!");
	lcdFlip();
	kbGetKeyMs(1000);
}

//持卡人认证例程
int emv_check_certificate(void)
{
//	int	    iRet, iErrNo;
	int     ucKey;
	uint8_t	ucCertType, szCertNo[50], iLength;

	if( emv_get_data_element("\x9F\x62", 2, &iLength, &ucCertType) )
	{
		errno = EMV_ERRNO_DATA;
		return -1;
	}

	memset(szCertNo, 0, sizeof(szCertNo));
	if( emv_get_data_element("\x9F\x61", 2, &iLength, szCertNo) )
	{
		errno = EMV_ERRNO_DATA;
		return -1;
	}
	if( iLength>40 )
	{
		errno = EMV_ERRNO_DATA;
		return -1;
	}

	lcdCls();	
	lcdDisplay(0, 0, DISP_CFONT, "Check certificate");
	switch( ucCertType )
	{
	case 0x00:
		lcdDisplay(0, 2, DISP_CFONT, "identity card:");
		break;

	case 0x01:
		lcdDisplay(0, 2, DISP_CFONT, "officer ID:");
		break;

	case 0x02:
		lcdDisplay(0, 2, DISP_CFONT, "Passport:");
		break;

	case 0x03:
		lcdDisplay(0, 2, DISP_CFONT, "Immigrant ID:");
		break;

	case 0x04:
		lcdDisplay(0, 2, DISP_CFONT, "Temp identity:");
		break;

	default:
		lcdDisplay(0, 2, DISP_CFONT, "Other");
		break;
	}
	lcdDisplay(0, 4, DISP_ASCII, "%.40s", szCertNo);
	lcdDisplay(0, 6, DISP_CFONT, "0-CANCEL  1-ENTER");
	lcdFlip();

	while(1)
	{
		ucKey = kbGetKey();
		if( (ucKey!=KEY1) && (ucKey!=KEY0) )
		{
			continue;
		}
		break;
	}
	if( ucKey==KEY0 )
	{
		lcdCls();	
		errno = EMV_ERRNO_DATA;
		return -1;

	}
	if( ucKey==KEY1 )
	{
		lcdCls();	
		sprintf((char *)&PosCom.szCertData, "%02X%-20.20s", ucCertType, szCertNo);
		return 0;
	}
	return 0;
}

int emv_get_pin(void *CardPin)
{
#ifdef _POS_TYPE_8210
	uint8_t	 ucDispFlag=0;
	int      iRet;
	do{
		if (stPosParam.ucSupPinpad != PARAM_OPEN)
		{
			iRet = PedGetSensitiveTimer(TYPE_PIN_INTERVAL);
			if( iRet>0 )
			{
				if (ucDispFlag == 0)//only disp one time
				{
					lcdClrLine(2,7);
					lcdDisplay(0, 2, DISP_CFONT|DISP_CLRLINE, "PLS WAITINT..");
					lcdFlip();
					ucDispFlag = 1;
				}
				sysDelayMs(250); // Avoid excessive occupied CPU resource
				continue;			
			}
		}
		lcdClrLine(2,7);
//		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"ENTER PIN:");
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"Ingrese Pin:");
		lcdFlip();
		break;
	}while(1);
	
	return 0;
#else
	int retval;
	uint8_t pin_block[50];
	uint8_t pin[8];
	DES_key_schedule k1;
	DES_key_schedule k2;
	DES_key_schedule k3;
	uint8_t i=0;
	uint8_t pinlen;
	uint8_t *ppin = (uint8_t *)CardPin;
	struct terminal_mck_configure tMCKConfig;

	if (stPosParam.ucSupPinpad == PARAM_OPEN)
	{	
		memset(pin_block,0,sizeof(pin_block));
		memcpy(pin_block,    "\x85\x6C\x82\x02\x8D\xA9\x77\x48", 8);
		memcpy(pin_block+8,  "\x2F\x26\x20\x56\xE2\x64\x4C\x93", 8);
		memcpy(pin_block+16, "\x37\x59\x60\x78\x01\x3C\x2A\xB6", 8);
		DES_set_key((const_DES_cblock *)pin_block, &k1);
		DES_set_key((const_DES_cblock *)&pin_block[8], &k2);
		DES_set_key((const_DES_cblock *)&pin_block[16], &k3);
		
		retval = EppDownloadAppKey(KEY_TYPE_FIXED_PIN,0x00,EMV_PIN_KEY_ID,pin_block,24,1);
		if( retval<0 )
		{		
			errno = EMV_ERRNO_DATA;
			return -1;
		}
		
		retval = EppBeep(2500,200);
		if( retval<0 )
		{			
			EppClearAKey(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID);
			errno = EMV_ERRNO_DATA;
			return -1;
		}
		
		retval = EppLight(2, 0);
		if( retval<0 )
		{			
			EppClearAKey(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID);
			errno = EMV_ERRNO_DATA;
			return -1;
		}
		
		retval = EppClearScreen();
		if( retval<0 )
		{			
			EppClearAKey(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID);
			EppLight(3, 0);
			EppRestoreDefaultIdleLogo();
			errno = EMV_ERRNO_DATA;
			return -1;
		}
	}

	emv_get_mck_configure(&tMCKConfig);

	lcdClrLine(2,7);
	lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"ENTER PIN:");
	lcdFlip();
	if (stPosParam.ucSupPinpad == PARAM_OPEN)
	{
		retval = EppDisplayLogo(0, 0, g_InputPinLogoBmp);
		if( retval<0 )
		{			
			EppClearAKey(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID);
			EppLight(3, 0);
			EppRestoreDefaultIdleLogo();
			errno = EMV_ERRNO_DATA;
			return -1;
		}
		
		memset(pin_block, 0x00, sizeof(pin_block));
		memset(pin, 0x00, sizeof(pin));
		if( tMCKConfig.support_bypass_pin_entry )
		{
			retval = EppGetPin(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID, 0, 1, (uint8_t *)"1234567890123456",
				(uint8_t *)"0456789abc\0", pin_block);
		}
		else
		{
			retval = EppGetPin(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID, 0, 1, (uint8_t *)"1234567890123456", 
				(uint8_t *)"456789abc\0", pin_block);
		}
		
		EppClearAKey(KEY_TYPE_FIXED_PIN, EMV_PIN_KEY_ID);
		EppLight(3, 0);
		EppRestoreDefaultIdleLogo();
		//epp_close(glbEpp);
		
		if( retval<0 )
		{			
			if( retval==-RET_USER_PRESS_CANCEL_EXIT || retval==-EPP_USER_PRESS_CANCEL || retval == -RET_USER_INPUT_TIME_OUT)
			{
				errno = EMV_ERRNO_CANCEL;
				return -1;
			}
			else if( retval==-EPP_NO_PIN )
			{
				errno = EMV_ERRNO_BYPASS_PIN;
				return -1;
			}
			else
			{	
				errno = EMV_ERRNO_DATA;
				return -1;
			}
		}
		DES_ecb3_encrypt((const_DES_cblock *)pin_block, (DES_cblock *)pin, &k1, &k2, &k3, DES_DECRYPT);	
		memset(pin_block, 0x00, sizeof(pin_block));
		BcdToAsc((uint8_t *)pin_block, pin, 16);
		
		pinlen = 0;
		for(i=0; i<16; i++)
		{
			if( (pin_block[i] != 'F') 
				&& (pin_block[i] != 'f') )
			{
				ppin[i] = pin_block[i];
				pinlen++;
			}
		}	
		return pinlen;
	}
	else
	{
		lcdGoto(20,5);
		while(PedGetSensitiveTimer(TYPE_PIN_INTERVAL));
	
		if(tMCKConfig.support_bypass_pin_entry)
			retval = kbGetString(KB_BIG_PWD,0,12,0,CardPin);
		else
			retval = kbGetString(KB_BIG_PWD,4,12,0,CardPin);
		sysDelayMs(50);//Avoid tone piercing
	
		if(retval == KB_CANCEL)
		{
			errno = EMV_ERRNO_CANCEL;
			return -1;
		}
		else if(retval== 0)
		{
			errno = EMV_ERRNO_BYPASS_PIN;
			return -1;			
		}
		return retval;
	}
#endif
}

int emv_return_pin_verify_result(uint8_t TryCounter)
{
	int Ret;

	kbFlush();
	PosCom.stTrans.PinCheckSuccessFlag = 0;
	if( TryCounter==0 )
	{
		PosCom.stTrans.PinCheckSuccessFlag = 1;
		lcdCls();	
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PIN OK");
		lcdFlip();
		return 0;
	}

	lcdCls();	
	if(TryCounter==1)
	{
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"PIN ERROR");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"Remain %d Times",TryCounter);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"TRY AGAIN?");
	}
	else
	{
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"PIN ERROR");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"TRY AGAIN?");
	}
	lcdFlip();
	while(1)
	{
		Ret = kbGetKey();		
		if(Ret == KEY_ENTER)
		{    
			Ret = EMV_RET_SUCCESS;
			break;
		}
		if(Ret == KEY_CANCEL)
		{
			errno = EMV_ERRNO_CANCEL;
			return -1;
		}
	}
	
	return Ret;
}

uint8_t ProcEmvTran(void)
{
	int	iRet, iErrNo, need_online, iDatelen;
	uint16_t  iSw;
	uint8_t   ucNeedonline,uclen;
	uint8_t   ucScripRet = FALSE;
	uint8_t   uLen, iIccDataLenBak;
	uint8_t   szBuffer[10],sAppCryptoBak[100], sIccDataBak[300];

	lcdFlip(); 
	emv_get_mck_configure(&tConfig);
	memset(sIssuerScriptRet,0,sizeof(sIssuerScriptRet));
	sIssuerScriptRetlen = 0;
	szSaveICCFail = 1;

	ucNeedonline = 0;
	if (stTemp.iTransNo != POS_SALE && stTemp.iTransNo != EC_QUICK_SALE
		&& stTemp.iTransNo != EC_NORMAL_SALE)
	{
		ucNeedonline = 1;
	}
	tConfig.support_forced_online_capability = 1;//ucNeedonline;
//	tConfig.support_forced_online_capability = (stTemp.iTransNo!=POS_SALE ? 1 : 0);
//	Hash(&tConfig.terminal_type, sizeof(tConfig)-20, tConfig.checksum);
	SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);
	emv_set_mck_configure(&tConfig);
	
	sprintf((char *)PosCom.szRespCode, "00");
	PosCom.stTrans.iTransNo = stTemp.iTransNo;

	emv_process_restriction();
	
	if (stTemp.iTransNo!=EC_TOPUP_CASH  && stTemp.iTransNo!=EC_TOPUP_SPEC && stTemp.iTransNo!=EC_TOPUP_NORAML
		&& stTemp.iTransNo!=EC_VOID_TOPUP)
	{
		iRet = emv_cardholder_verify(icc_fd);	
		iErrNo = iRet?errno:0;	
		if( iRet<0 )
		{
			goto ERRCODE;
		}
	}

	iRet = emv_terminal_risk_manage(icc_fd);	
	PrintDebug("%s%d", "after emv_terminal_risk_manage:",iRet);

	
	iErrNo = iRet?errno:0;
	if( iRet<0 )
	{
		goto ERRCODE;
	}
	
	PrintDebug("%s", "before emv_terminal_action_analysis");
	iRet = emv_terminal_action_analysis(icc_fd, &need_online);
	PrintDebug("%s%d", "after emv_terminal_action_analysis:",iRet);


	if(!iRet && need_online)
	{
		PrintDebug("%s", "before emv_online_transaction");
		iRet = emv_online_transaction(icc_fd);
		PrintDebug("%s%d", "after emv_online_transaction:",iRet);
	}
	iErrNo = iRet?errno:0;
	PrintDebug("%s%d", "after emv_online_transaction iErrno:",errno);

	// After that do not modify the iRet and iError values
	memset(szBuffer,0,sizeof(szBuffer));
	szBuffer[0] = 0xFF;
	emv_get_data_element("\x9F\x27", 2, &uLen, szBuffer);
	szBuffer[0] &= 0xC0;
	if (need_online==0 && szBuffer[0]==0 && szSaveICCFail==1 && ChkIfSaveLog())
	{
		GetAndSetQpbocElement();
		SaveIccFailureTranData(TRUE);
		//strcpy((char*)PosCom.stTrans.szEntryMode,"052");
		memcpy(PosCom.stTrans.szEntryMode, "550", 3);

		if (PosCom.ucPinEntry)
		{
			//strcpy((char*)PosCom.stTrans.szEntryMode,"051");
			memcpy(PosCom.stTrans.szEntryMode, "550", 3);

		}
		SaveIccFailureFile();
	}

	// Before that do not modify the iRet and iError values
ERRCODE:

	PrintDebug("%s%d", "iErrNo:",iErrNo);
/*	if(PosCom.stTrans.CardType ==7 && iErrNo==EMV_ERRNO_DECLINE)
	{
		return E_TRANS_CANCEL;
	}
*/
	if( iErrNo==EMV_ERRNO_ONLINE_TIMEOUT || iErrNo==EMV_ERRNO_CANCEL ||iErrNo==EMV_ERRNO_ONLINE_ERROR)
	{
		//if(PosCom.stTrans.CardType !=7)
		{
			return E_TRANS_CANCEL;
		}
	}
	//"EDENRED"卡不需要生成TC
	if(PosCom.stTrans.CardType ==7)
	{
		;
	}
	else
	{
		if( iErrNo!=EMV_RET_SUCCESS ) 
		{
			iSw = emv_get_last_iccard_sw();
			if( PosCom.bIsFirstGAC && (iSw==0x6985) )
			{
				return E_NEED_FALLBACK;
			}
			
			UpdateEmvRevFile();
			
			if(need_online && PosCom.bOnlineTxn==0 && stPosParam.ucReversalTime == PARAM_OPEN)
			{
				iRet = ReverseTranProc();
				if (iRet==0)
				{
					lcdClrLine(2,7);
					lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"REVERSE SUCCESS");
					lcdFlip();
					PubBeepOk();
					PubWaitKey(3);
				}
				return NO_DISP;
			}

			if( PosCom.bOnlineTxn && memcmp(glRecvPack.szRspCode, "00", 2)!=0 )
			{
				iRet = AfterCommProc();
				return iRet;
			}
			
			if( PosCom.bOnlineTxn && memcmp(glRecvPack.szRspCode, "00", 2)==0 )	//posp accept,but card refuse
			{
				UpdateARPCEmvRevFile();		
				// check script result
				emv_get_data_element("\x9B", 1, &uclen, PosCom.stTrans.sTSI);
				if(PosCom.stTrans.sTSI[0] & 0x04)		//have script
					ucScripRet = TRUE;
				if (PosCom.stTrans.sTVR[4] & 0x30)		//tag71 or tag72 fail
				{
					ucScripRet = FALSE;
				}
				
				if (PosCom.stTrans.iTransNo== EC_TOPUP_CASH || PosCom.stTrans.iTransNo== EC_TOPUP_SPEC
					|| PosCom.stTrans.iTransNo== EC_TOPUP_NORAML || PosCom.stTrans.iTransNo == EC_VOID_TOPUP)
				{
					if (!ucScripRet ) 
					{
						SaveEmvTranData();  // save script
						UpdateEcRevFile();
					}
				}

				if(stPosParam.ucReversalTime == PARAM_OPEN)
				{
					iRet = ReverseTranProc();
					PrintDebug("%s%d", "ReverseTranProc:",iRet);

					if (iRet==0)
					{
						lcdClrLine(2,7);
						lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"REVERSE SUCCESS");
						lcdFlip();
						PubBeepOk();
						PubWaitKey(3);
						DispTransName();
						return E_CARD_DEFUSED;
					}
					else if (iRet == 1)
					{
						return E_CARD_DEFUSED;
					}
					DispTransName();
					return iRet;
				}
				return E_CARD_DEFUSED;//disp:contact the issuer bank
			}

			if( iErrNo==EMV_ERRNO_DECLINE )
				return E_TRAN_DEFUSED;
			else
				return E_TRANS_FAIL;
		}	
	}	

	emv_get_data_element("\x9B", 1, &uclen, PosCom.stTrans.sTSI);
	if(PosCom.stTrans.sTSI[0] & 0x04)		//have script
		ucScripRet = TRUE;
	
	emv_get_data_element("\x95", 1, &uclen, PosCom.stTrans.sTVR);
	showHex("PosCom.stTrans.sTVR",PosCom.stTrans.sTVR,5);
	if (PosCom.stTrans.sTVR[4] & 0x30)		//tag71 or tag72 fail
	{
		ucScripRet = FALSE;
	}

	//read cardholder name
	emv_get_data_element("\x5F20", 1, &uclen, PosCom.stTrans.CardHolderName);
	PrintDebug("% %s","Tag5F20:",PosCom.stTrans.CardHolderName);
	
	if (PosCom.stTrans.iTransNo== EC_TOPUP_CASH || PosCom.stTrans.iTransNo== EC_TOPUP_SPEC
		|| PosCom.stTrans.iTransNo== EC_TOPUP_NORAML || PosCom.stTrans.iTransNo == EC_VOID_TOPUP)
	{
		if (!ucScripRet)
		{
			SaveEmvTranData();  // save script
			UpdateEcRevFile();
			return E_TRANS_FAIL;
		}

		iSw = emv_get_last_iccard_sw();
		if (iSw != 0x9000)
		{
			SaveEmvTranData();  // save script
			UpdateEcRevFile();			
			return E_TRANS_FAIL;
		}
	}
 
	// ARPCfail,card succeed,need to save fail log
	if (PosCom.stTrans.sTVR[4] & 0x40)//ARPC fail,The issuer authentication failure
	{
		// backup sIccData，SaveIccFailureTranData will modify it
		memcpy(sIccDataBak,PosCom.stTrans.sIccData,PosCom.stTrans.iIccDataLen);
		iIccDataLenBak = PosCom.stTrans.iIccDataLen;
		GetAndSetQpbocElement();
		SaveIccFailureTranData(FALSE);
		memcpy(sAppCryptoBak,PosCom.stTrans.sAppCrypto,sizeof(PosCom.stTrans.sAppCrypto));
		GetARPCTag91((char*)PosCom.stTrans.sAppCrypto,&iDatelen);
		strcpy((char*)PosCom.stTrans.szEntryMode,"052");
		if (PosCom.ucPinEntry)
		{
			strcpy((char*)PosCom.stTrans.szEntryMode,"051");
		}

		iRet = SaveIccFailureFile();
		memcpy(PosCom.stTrans.sAppCrypto,sAppCryptoBak,sizeof(PosCom.stTrans.sAppCrypto));
		PosCom.stTrans.iIccDataLen = iIccDataLenBak;
		memcpy(PosCom.stTrans.sIccData,sIccDataBak,PosCom.stTrans.iIccDataLen);
	}

	if( PosCom.bOnlineTxn)
	{
		fileRemove(REVERSAL_FILE);
	}

	SaveEmvTranData();
	return AfterCommProc();
}

uint8_t SetOnlineTranDE55(Def55EmvTag *pstList, uint8_t *psTLVData, int *piDataLen)
{
	int	i, iRet, iTemp, iErrNo;
	uint8_t	*psTemp, sBuff[128], iLength;
	uint8_t   sTemp[3];

	*piDataLen = 0;
	psTemp     = psTLVData;

	for(i=0; pstList[i].uiTagNum!=NULL_TAG_1; i++)
	{
		if (pstList[i].uiTagNum == 0x9F74)
		{
			if (PosCom.stTrans.iTransNo == EC_TOPUP_CASH || PosCom.stTrans.iTransNo == EC_TOPUP_SPEC 
				|| PosCom.stTrans.iTransNo == EC_TOPUP_NORAML || PosCom.stTrans.iTransNo == POS_PREAUTH
				|| PosCom.stTrans.iTransNo == POS_QUE  || PosCom.stTrans.iTransNo == EC_VOID_TOPUP
				|| PosCom.stTrans.iTransNo == POS_SALE||PosCom.stTrans.iTransNo ==QPBOC_ONLINE_SALE
				|| PosCom.stTrans.iTransNo == POS_INSTALLMENT 
				|| PosCom.stTrans.iTransNo == CHECK_IN 
				|| PosCom.stTrans.iTransNo == CHECK_OUT 
				|| PosCom.stTrans.iTransNo == CASH_ADVACNE 
				||stTemp.iTransNo==NETPAY_REFUND 
				||stTemp.iTransNo==NETPAY_ADJUST
				||stTemp.iTransNo==PURSE_SALE
				||stTemp.iTransNo==PURSE_GETBALANCE
				||stTemp.iTransNo==PURSE_GETRULE)
			{
				continue;
			}
		}

		memset(sBuff, 0, sizeof(sBuff));
		memset(sTemp, 0, sizeof(sTemp));
		if( pstList[i].uiTagNum>0xFF )
		{
			sTemp[0] = (uint8_t)(pstList[i].uiTagNum>>8);
			sTemp[1] = (uint8_t)(pstList[i].uiTagNum);
			iTemp = 2;
		}
		else
		{
			sTemp[0] = (uint8_t)(pstList[i].uiTagNum);
			iTemp = 1;
		}

		if(pstList[i].uiTagNum == 0xDF31) // only reversal have DF31，emv_get_script_result return length is 0
		{
			memset(sBuff, 0, sizeof(sBuff));
			iRet = 0;
			memcpy(sBuff,sIssuerScriptRet,sIssuerScriptRetlen);
			iLength = sIssuerScriptRetlen;
		}
		else
		{
			iRet = emv_get_data_element(sTemp, iTemp, &iLength, sBuff);
		}

		iErrNo = iRet?errno:0;
		if( iErrNo==EMV_RET_SUCCESS )
		{
			MakeTlvString(pstList[i].uiTagNum, sBuff, iLength, &psTemp);
		}
		else if( pstList[i].ucTagOpt==DE55_MUST_SET )
		{
			return E_TRANS_FAIL;
		}
	}
	*piDataLen = (psTemp-psTLVData);

	return OK;
}

int GetTlvValue(uint8_t **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uint8_t bExpandAll)
{
	int		iRet;
	uint8_t		*psTag, *psSubTag;
	uint16_t		uiTag, uiLenBytes;
	uint32_t		lTemp;

	for(psTag=*ppsTLVString; psTag<*ppsTLVString+iMaxLen; psTag++)
	{
		if( (*psTag!=NULL_TAG_1) && (*psTag!=NULL_TAG_2) )
		{
			break;
		}
	}
	if( psTag>=*ppsTLVString+iMaxLen )
	{
		*ppsTLVString = psTag;
		return 0;	
	}

	uiTag = *psTag++;
	if( (uiTag & TAGMASK_FIRSTBYTE)==TAGMASK_FIRSTBYTE )
	{	
		uiTag = (uiTag<<8) + *psTag++;
	}
	if( psTag>=*ppsTLVString+iMaxLen )
	{
		return -1;
	}

	if( (*psTag & LENMASK_NEXTBYTE)==LENMASK_NEXTBYTE )
	{
		uiLenBytes = *psTag & LENMASK_LENBYTES;
		Char2Long(psTag+1, uiLenBytes, &lTemp);
	}
	else
	{
		uiLenBytes = 0;
		lTemp      = *psTag & LENMASK_LENBYTES;
	}
	psTag += uiLenBytes+1;
	if( psTag+lTemp>*ppsTLVString+iMaxLen )
	{
		return -2;
	}
	*ppsTLVString = psTag+lTemp;	

	(*pfSaveData)(uiTag, psTag, (int)lTemp);
	if( !IsConstructedTag(uiTag) || !bExpandAll )
	{
		return 0;
	}

	for(psSubTag=psTag; psSubTag<psTag+lTemp; )
	{
		iRet = GetTlvValue(&psSubTag, psTag+lTemp-psSubTag, pfSaveData, TRUE);
		if( iRet<0 )
		{
			return iRet;
		}
	}

	return 0;
}

int IsConstructedTag(uint16_t uiTag)
{
	int		i;

	for(i=0; (uiTag&0xFF00) && i<2; i++)
	{
		uiTag >>= 8;
	}

	return ((uiTag & TAGMASK_CONSTRUCTED)==TAGMASK_CONSTRUCTED);
}

void Char2Long(uint8_t *psSource, uint16_t uiSCnt, uint32_t *pulTarget)
{
	uint16_t    i;
	
	*pulTarget = 0L;
	for(i=0; i<uiSCnt; i++)
	{
		*pulTarget |= ((uint32_t)psSource[i] << 8*(uiSCnt-i-1));
	}
}

void SaveRspICCData(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	switch( uiTag )
	{
/*
	case 0x91:
		memcpy(sAuthData, psData, MIN(iDataLen, 16));
		iAuthDataLen = MIN(iDataLen, 16);
		break;
*/
	case 0x71:
	case 0x72:
		sIssuerScript[iScriptLen++] = (uint8_t)uiTag;
		if( iDataLen>127 )
		{
			sIssuerScript[iScriptLen++] = 0x81;
		}
		sIssuerScript[iScriptLen++] = (uint8_t)iDataLen;
		memcpy(&sIssuerScript[iScriptLen], psData, iDataLen);
		iScriptLen += iDataLen;
		break;
	
	case 0x9F36:
		break;
		
	default:
		break;
	}
}

uint8_t SetIssuerName(void)
{
	int	iRet, iErrNo;
	uint8_t	sAID[16], iLength;
	
	// Application Identifier (AID) - ICC
	memset(sAID, 0, sizeof(sAID));
	iRet = emv_get_data_element("\x4F", 1, &iLength, sAID);
	iErrNo = iRet?errno:0;
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	if( memcmp(sAID, "\xD1\x56", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "CUP");
	}
	else if( memcmp(PosCom.stTrans.szCardNo, "4", 1)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "VIS");
	}
	else if( memcmp(PosCom.stTrans.szCardNo, "5", 1)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "MCC");
	}
	else if( memcmp(PosCom.stTrans.szCardNo, "34", 2)==0 ||
		memcmp(PosCom.stTrans.szCardNo, "37", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "AMX");
	}
	else if( memcmp(PosCom.stTrans.szCardNo, "30", 2)==0 ||
		memcmp(PosCom.stTrans.szCardNo, "36", 2)==0 ||
		memcmp(PosCom.stTrans.szCardNo, "38", 2)==0 ||
		memcmp(PosCom.stTrans.szCardNo, "39", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "DCC");
	}
	else if( memcmp(PosCom.stTrans.szCardNo, "35", 2)==0 )
	{
		sprintf((char *)PosCom.stTrans.szCardUnit, "JCB");
	}

	return OK;
}

// called only if emvProcessTrans() failed
uint8_t UpdateEmvRevFile(void)
{
	uint8_t	ucRet;
	int	iLength;
	uint8_t	sReversalDE55[128];
	uint8_t   szBuff[10];

	SaveScriptResult();
	if( fileExist(REVERSAL_FILE)<0 )
	{
		return OK;
	}
	memset(sReversalDE55, 0, sizeof(sReversalDE55));
	ucRet = SetOnlineTranDE55(stReversalTagList, sReversalDE55, &iLength);
	if( ucRet!=OK )
	{
		return E_TRANS_FAIL;
	}
	memset(szBuff, 0, sizeof(szBuff));
	if( strlen((char *)PosCom.stTrans.szAuthNo)>0 )
	{
		sprintf((char *)szBuff, "%.6s", PosCom.stTrans.szAuthNo);
	}
	ReadReversalTranData();
	memcpy(PosCom.sReversalDE55, sReversalDE55, iLength);
	PosCom.iReversalDE55Len = iLength;
	if( PosCom.szRespCode[0]==0 || memcmp(PosCom.szRespCode, "00", 2)==0 )
	{
		sprintf((char *)PosCom.szRespCode, "06"); 
	}

	if( strlen((char *)szBuff)>0 )
	{
		sprintf((char *)PosCom.stTrans.szAuthNo, "%.6s", szBuff);
	}
	SaveReversalTranData();
	return OK;
}

uint8_t UpdateARPCEmvRevFile(void)
{
	uint8_t	ucRet;
	int	iLength;
	uint8_t	sReversalDE55[128];
	uint8_t   szBuff[10];
	
	
	if( fileExist(REVERSAL_FILE)<0 )
	{
		return OK;
	}
	
	memset(sReversalDE55, 0, sizeof(sReversalDE55));
	ucRet = SetOnlineTranDE55(stARPCReversalTagList, sReversalDE55, &iLength);
	if( ucRet!=OK )
	{
		return E_TRANS_FAIL;
	}
	
	memset(szBuff, 0, sizeof(szBuff));
	if( strlen((char *)PosCom.stTrans.szAuthNo)>0 )
	{
		sprintf((char *)szBuff, "%.6s", PosCom.stTrans.szAuthNo);
	}
	
	ReadReversalTranData();
	
	memcpy(PosCom.sReversalDE55, sReversalDE55, iLength);
	PosCom.iReversalDE55Len = iLength;
	if( PosCom.szRespCode[0]==0 || memcmp(PosCom.szRespCode, "00", 2)==0 )
	{
		sprintf((char *)PosCom.szRespCode, "06"); 
	}
	
	if( strlen((char *)szBuff)>0 )
	{
		sprintf((char *)PosCom.stTrans.szAuthNo, "%.6s", szBuff);
	}
	
	SaveReversalTranData();
	
	return OK;
}


uint8_t UpdateEcRevFile(void)
{
	uint8_t	ucRet;
	int	iLength;
	uint8_t	sReversalDE55[128];

	if( fileExist(REVERSAL_FILE)<0 )
	{
		return OK;
	}
	
	memset(sReversalDE55, 0, sizeof(sReversalDE55));
	ucRet = SetOnlineTranDE55(stReversalTagList, sReversalDE55, &iLength);
	if( ucRet!=OK )
	{
		return E_TRANS_FAIL;
	}
		
	ReadReversalTranData();
	
	memcpy(PosCom.sReversalDE55, sReversalDE55, iLength);
	PosCom.iReversalDE55Len = iLength;
	
	SaveReversalTranData();
	
	return OK;
}

// process tag
void MakeTlvString(uint16_t uiTagNum, uint8_t *psData, int iLength, uint8_t **ppsOutData)
{
	uint8_t	*psTemp;

	if( iLength<=0 )
	{
		return;
	}

	psTemp = *ppsOutData;
	if( uiTagNum & 0xFF00 )
	{
		*psTemp++ = (uint8_t)(uiTagNum >> 8);
	}
	*psTemp++ = (uint8_t)uiTagNum;	
	if( iLength<=127 )
	{
		*psTemp++ = (uint8_t)iLength;
	}
	else
	{	
		*psTemp++ = 0x81;
		*psTemp++ = (uint8_t)iLength;
	}	
	memcpy(psTemp, psData, iLength);
	psTemp += iLength;	
	*ppsOutData = psTemp;
}

void SaveEmvTranData(void)
{
	int		iLength, iRet;
	uint8_t uLen;
	char    szBuffer[15];
	uint32_t iSendLen, iRecvLen;
	UINT8 szSendData[300], szRecvData[300];

	if( !PosCom.bOnlineTxn && PosCom.stTrans.iTransNo==POS_SALE )
	{
		PosCom.stTrans.iTransNo = ICC_OFFSALE;
		stTemp.iTransNo = ICC_OFFSALE;
		iLength = 0;
		SetOnlineTranDE55(stOnlineTxnTagList, PosCom.stTrans.sIccData, &iLength);
		PosCom.stTrans.iIccDataLen = (uint8_t)iLength;
		IncreaseTraceNo();

		if (memcmp(PosCom.stTrans.szAID,STR_PBOC_AID,10) == 0)
		{
			strcpy((char *)PosCom.stTrans.szCardUnit, "CUP");
		}
	}
	if (!PosCom.bOnlineTxn)
	{
		if (PosCom.stTrans.iTransNo==EC_NORMAL_SALE || PosCom.stTrans.iTransNo==EC_QUICK_SALE)
		{
			stTemp.iTransNo = PosCom.stTrans.iTransNo;
			iLength = 0;
			SetOnlineTranDE55(stOnlineTxnTagList, PosCom.stTrans.sIccData, &iLength);
			PosCom.stTrans.iIccDataLen = (uint8_t)iLength;
			IncreaseTraceNo();
		}

		if (memcmp(PosCom.stTrans.szAID,STR_PBOC_AID,10) == 0)
		{
			strcpy((char *)PosCom.stTrans.szCardUnit, "CUP");
		}
	}

	emv_get_data_element("\x9F\x26", 2, &uLen, PosCom.stTrans.sAppCrypto);
	emv_get_data_element("\x8A", 1, &uLen, PosCom.stTrans.sAuthRspCode);
	emv_get_data_element("\x95", 1, &uLen, PosCom.stTrans.sTVR);
	emv_get_data_element("\x9B", 1, &uLen, PosCom.stTrans.sTSI);
	emv_get_data_element("\x9F\x36", 2, &uLen, PosCom.stTrans.sATC);

	if(PosCom.stTrans.iTransNo==EC_NORMAL_SALE)
	{
		memset(szBuffer,0,sizeof(szBuffer));
		iRet = emv_get_data_element("\x9F\x79", 2, &uLen, szBuffer);
		iLength = PubBcd2Long((uint8_t*)szBuffer,6,NULL);
		PubLong2Char(iLength,4,(char*)PosCom.stTrans.sAfterBalanceAmt);
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"SUCCEED");
		lcdDisplay(0, 4, DISP_MEDIACY|DISP_CFONT, "BALANCE:");
		lcdDisplay(0, 6, DISP_MEDIACY|DISP_CFONT,"%d.%02d", iLength/100, iLength%100);
		lcdFlip();
		sysBeef(6,400);
		PubWaitKey(1);
	}

	if (PosCom.stTrans.iTransNo==EC_TOPUP_CASH || PosCom.stTrans.iTransNo==EC_TOPUP_NORAML ||
		PosCom.stTrans.iTransNo==EC_TOPUP_SPEC  || PosCom.stTrans.iTransNo == EC_VOID_TOPUP)
	{
		uLen = 0;
		if (PosCom.ucSwipedFlag == CARD_PASSIVE)
		{
			iSendLen = iRecvLen = 0;  
			memset(szSendData, 0x00, sizeof(szSendData));
			memset(szRecvData, 0x00, sizeof(szRecvData));	
			memcpy(szSendData,"\x80\xCA\x9F\x79\x00",5);
			iSendLen = 5;
			iRet = contactless_exchangeapdu(icc_fd,iSendLen,szSendData,&iRecvLen,szRecvData);
			memcpy(szBuffer,&szRecvData[3],6);
			uLen = 6;
		}
		else
			iRet = GetEcElementData((uint8_t*)"\x9F\x79", &uLen, (uint8_t*)szBuffer);	// EC BALANCE

		if (iRet == 0)
		{
			iRet = PubBcd2Long((uint8_t*)szBuffer,uLen,NULL);
			PubLong2Char(iRet,4,(char*)PosCom.stTrans.sAfterBalanceAmt);
		}
	}

	SaveScriptResult();
}


uint8_t SaveScriptResult(void)
{
	int	iRet, iCnt, iTemp, iLen, iErrNo;
	uint8_t	*psTemp, sBuff[128];
	uint8_t   sTemp[3], iLength;

	psTemp = PosCom.sScriptDE55;
	for(iCnt=0; stScriptResultTagList[iCnt].uiTagNum!=NULL_TAG_1; iCnt++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		memset(sTemp, 0, sizeof(sTemp));
		if( stScriptResultTagList[iCnt].uiTagNum>0xFF )
		{
			sTemp[0] = (uint8_t)(stScriptResultTagList[iCnt].uiTagNum>>8);
			sTemp[1] = (uint8_t)(stScriptResultTagList[iCnt].uiTagNum);
			iTemp = 2;
		}
		else
		{
			sTemp[0] = (uint8_t)(stScriptResultTagList[iCnt].uiTagNum);
			iTemp = 1;
		}

		if((sTemp[0] == 0xDF) && (sTemp[1] == 0x31))
		{
			sysDelayMs(100);
			
			memset(sBuff, 0, sizeof(sBuff));
			iRet = emv_get_script_result(sBuff, &iLen);			
			if( iRet!=EMV_RET_SUCCESS )
			{
				return OK;
			}

			memcpy(sIssuerScriptRet,sBuff,iLen);
			sIssuerScriptRetlen = iLen;
			MakeTlvString(stScriptResultTagList[iCnt].uiTagNum, sBuff, iLen, &psTemp);			
		}
		else
		{
			iRet = emv_get_data_element(sTemp, iTemp, &iLength, sBuff);
			iErrNo = iRet?errno:0;
			if( iErrNo==EMV_RET_SUCCESS )
			{
				MakeTlvString(stScriptResultTagList[iCnt].uiTagNum, sBuff, iLength, &psTemp);
			}
			else if( stScriptResultTagList[iCnt].ucTagOpt==DE55_MUST_SET )
			{	
				return E_TRANS_FAIL;
			}
		}
	}
	PosCom.iScriptDE55Len = (psTemp-PosCom.sScriptDE55);

	return WriteScriptFile();
}

uint8_t SendScriptResult(void)
{
	uint8_t	ucRet=0;
	uint8_t   ucIndex,ucTryCnt;

	if( fileExist(ICC_SCRIPT_FILE)<0 )
	{
		return OK;
	}
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "Upload Scrip...");
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "Upload Scrip... ");
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PLS WAIT....");
	lcdFlip();
	sysDelayMs(500); 
	
	PosCom.stTrans.iTransNo = stTemp.iTransNo;
	memcpy(&PosCom.stOrgPack, &glSendPack, sizeof(glSendPack));	// backup orignal message
	memcpy(&BackPosCom, &PosCom, COM_SIZE);

	for(ucTryCnt=1; ucTryCnt<stPosParam.ucResendTimes+2; ucTryCnt++) 
	{		
		ucRet = ReadScriptFile();	// read PosCom structure 
		if( ucRet!=OK )
		{
			memcpy( &PosCom, &BackPosCom, COM_SIZE);
			memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));
			return ucRet;
		}
	
		stTemp.iTransNo = UPLOAD_SCRIPT_RESULT;
		memset(&glSendPack, 0, sizeof(STISO8583));
		sprintf((char *)glSendPack.szMsgCode,      "0620");
		sprintf((char *)glSendPack.szPan, "%.19s", PosCom.stTrans.szCardNo);
		sprintf((char *)glSendPack.szProcCode,     "%s",    PosCom.stOrgPack.szProcCode);
		sprintf((char *)glSendPack.szTranAmt,      "%s",    PosCom.stOrgPack.szTranAmt);
		sprintf((char *)glSendPack.szSTAN,         "%0*d", LEN_POS_STAN, stPosParam.lNowTraceNo);
		sprintf((char *)glSendPack.szEntryMode,    "%s",    PosCom.stOrgPack.szEntryMode);
		sprintf((char *)glSendPack.szPanSeqNo,     "%s",    PosCom.stOrgPack.szPanSeqNo);
		sprintf((char *)glSendPack.szAcquirerID,   "%.*s",  LEN_ACQUIRER_ID, PosCom.stTrans.szCenterId);
		sprintf((char *)glSendPack.szRRN,          "%.12s", PosCom.stTrans.szSysReferNo);
		sprintf((char *)glSendPack.szAuthCode,     "%.6s",  PosCom.stTrans.szAuthNo);
		sprintf((char *)glSendPack.szTermID,       "%s",    PosCom.stOrgPack.szTermID);
		sprintf((char *)glSendPack.szMerchantID,   "%s",    PosCom.stOrgPack.szMerchantID);
		sprintf((char *)glSendPack.szCurrencyCode, "%s",    PosCom.stOrgPack.szCurrencyCode);
		ShortToByte(glSendPack.sICCData, (uint16_t)PosCom.iScriptDE55Len);
		memcpy(&glSendPack.sICCData[2], PosCom.sScriptDE55, PosCom.iScriptDE55Len);
		ucIndex = GetTxnTableIndex(PosCom.stTrans.iTransNo);
		sprintf((char *)glSendPack.szField60, "00%06d%s%.2s", 
		stPosParam.lNowBatchNum, "951", &PosCom.stOrgPack.szField60[11]);
		sprintf((char *)glSendPack.szOrginalMsg, "%.6s%.6s%.4s", &PosCom.stOrgPack.szField60[2],
			PosCom.stOrgPack.szSTAN, &PosCom.stTrans.szDate[4]);
		if( HaveMacData(stTemp.iTransNo) )
		{
			ShortToByte(glSendPack.sMacData, LEN_MAC_DATA);
		}

		ucRet = SendRecvPacket();
		if (ucRet==E_MAKE_PACKET || ucRet==E_ERR_CONNECT || ucRet==E_SEND_PACKET)
		{
			return E_SCRIPT_FAIL;
		}
		if(ucRet==0)
		{
			fileRemove(ICC_SCRIPT_FILE);
			break;
		}
	}

	if( ucTryCnt>=stPosParam.ucResendTimes+2 )
	{
		fileRemove(ICC_SCRIPT_FILE);
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SEND SCRIPT FAIL");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);		
	}
	
	memcpy(&PosCom, &BackPosCom, COM_SIZE);
	memcpy(&glSendPack, &PosCom.stOrgPack, sizeof(glSendPack));	// recovery message
	stTemp.iTransNo = PosCom.stTrans.iTransNo;
	PosCom.stTrans.lTraceNo = stPosParam.lNowTraceNo;

	// set bit 11
	if( stTemp.iTransNo!=RESPONSE_TEST      && stTemp.iTransNo!=POS_DOWNLOAD_PARAM &&
		stTemp.iTransNo!=POS_UPLOAD_PARAM   && stTemp.iTransNo!=QUERY_EMV_CAPK     &&
		stTemp.iTransNo!=QUERY_EMV_PARAM    && stTemp.iTransNo!=DOWNLOAD_EMV_CAPK  &&
		stTemp.iTransNo!=DOWNLOAD_EMV_PARAM && stTemp.iTransNo!=END_EMV_CAPK       &&
		stTemp.iTransNo!=END_EMV_PARAM && stTemp.iTransNo!=END_DOWNLOADBLK)    
	{
		sprintf((char *)glSendPack.szSTAN, "%0*d", LEN_POS_STAN,  PosCom.stTrans.lTraceNo);
	}
	return ucRet;
}

void AdjustIssuerScript(void)
{
	int	iRet;
	uint8_t	*psTemp;

	memset(sScriptBak, 0, sizeof(sScriptBak));
	memset(&stScriptInfo, 0, sizeof(stScriptInfo));
	iCurScript = iScriptBakLen = 0;
	bHaveScript  = FALSE;
	for(psTemp=sIssuerScript; psTemp<sIssuerScript+iScriptLen; )
	{
		iRet = GetTlvValue(&psTemp, sIssuerScript+iScriptLen-psTemp, SaveScriptData, TRUE);
		if( iRet<0 )
		{
			return;
		}
	}
	if( bHaveScript && iCurScript>0 )
	{
		PackScriptData();
	}

	memcpy(sIssuerScript, sScriptBak, iScriptBakLen);
	iScriptLen = iScriptBakLen;
}

void  SaveScriptData(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x71:
	case 0x72:
		if( bHaveScript && iCurScript>0 )
		{
			PackScriptData();
		}
		stScriptInfo.uiTag = uiTag;
		bHaveScript = TRUE;
		iScriptNums++;
		break;
		
	case 0x9F18:
		stScriptInfo.iIDLen = MIN(4, iDataLen);
		memcpy(stScriptInfo.sScriptID, psData, MIN(4, iDataLen));
		break;

	case 0x86:
		stScriptInfo.iCmdLen[iCurScript] = iDataLen;
		memcpy(stScriptInfo.sScriptCmd[iCurScript], psData, iDataLen);
		iCurScript++;
		break;

	default:
		break;
	}

}

void PackTLVHead(uint16_t uiTag, uint16_t uiTLVDataLen, uint8_t *psOutData, int *piOutLen)
{
	int		iTagLen, iLen;
	
	PackTLVTag(uiTag, psOutData, &iTagLen);
	PackTLVLength(uiTLVDataLen, psOutData+iTagLen, &iLen);
	*piOutLen = iTagLen + iLen;
}

void PackTLVTag(uint16_t uiTag, uint8_t *psOutData, int *piOutLen)
{
	uint8_t	*psTemp;
	
	psTemp = psOutData;
	if( uiTag & 0xFF00 )
	{
		*psTemp++ = uiTag>>8;
	}
	*psTemp++ = uiTag;
	
	*piOutLen = (psTemp-psOutData);
}

void PackTLVLength(uint16_t uiTLVDataLen, uint8_t *psOutData, int *piOutLen)
{
	uint8_t	*psTemp;
	
	psTemp = psOutData;
	if( uiTLVDataLen<=127 )
	{
		*psTemp++ = (uint8_t)(uiTLVDataLen & 0xFF);
	}
	else
	{
		*psTemp++ = LENMASK_NEXTBYTE|0x01;
		*psTemp++ = (uint8_t)(uiTLVDataLen & 0xFF);
	}
	
	*piOutLen = (psTemp-psOutData);
}

int CalcTLVTotalLen(uint16_t uiTag, uint16_t uiDataLen)
{
	int	iLen, iTempLen;
	uint8_t	sTemp[4];
	
	iLen = 1;
	if( uiTag & 0xFF00 )
	{
		iLen++;
	}
	PackTLVLength(uiDataLen, sTemp, &iTempLen);
	
	return (iLen+iTempLen+uiDataLen);
}

void PackScriptData(void)
{
	int		iCnt, iTotalLen, iTempLen;

	iTotalLen = 0;
	if( stScriptInfo.iIDLen>0 )
	{
		iTotalLen += CalcTLVTotalLen(0x9F18, 4);
	}
	for(iCnt=0; iCnt<iCurScript; iCnt++)
	{
		iTotalLen += CalcTLVTotalLen(0x86, stScriptInfo.iCmdLen[iCnt]);
	}
	PackTLVHead(stScriptInfo.uiTag, iTotalLen, &sScriptBak[iScriptBakLen], &iTempLen);
	iScriptBakLen += iTempLen;
	
	if( stScriptInfo.iIDLen>0 )
	{
		PackTLVHead(0x9F18, 4, &sScriptBak[iScriptBakLen], &iTempLen);
		iScriptBakLen += iTempLen;
		memcpy(&sScriptBak[iScriptBakLen], stScriptInfo.sScriptID, 4);
		iScriptBakLen += 4;
	}
	for(iCnt=0; iCnt<iCurScript; iCnt++)
	{
		PackTLVHead(0x86, stScriptInfo.iCmdLen[iCnt], &sScriptBak[iScriptBakLen], &iTempLen);
		iScriptBakLen += iTempLen;
		memcpy(&sScriptBak[iScriptBakLen], stScriptInfo.sScriptCmd[iCnt], stScriptInfo.iCmdLen[iCnt]);
		iScriptBakLen += stScriptInfo.iCmdLen[iCnt];
	}

	memset(&stScriptInfo, 0, sizeof(stScriptInfo));
	iCurScript = 0;
}


void  SavePrintICCData(uint16_t uiTag, uint8_t *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x82:
		memset(gstIccPrintData.sAip,0,sizeof(gstIccPrintData.sAip));
		memcpy(gstIccPrintData.sAip, psData, MIN(iDataLen, 2));	
		break;
		
	case 0x9F10:
		memset(gstIccPrintData.sIssureAppData,0,sizeof(gstIccPrintData.sIssureAppData));
		memcpy(gstIccPrintData.sIssureAppData, psData, MIN(iDataLen, 32));
		gstIccPrintData.ucIssureAppDatalen = iDataLen;
		if (gstIccPrintData.ucIssureAppDatalen > 32)
		{
			gstIccPrintData.ucIssureAppDatalen = 32;
		}
		break;
		
	case 0x9F33:
		memset(gstIccPrintData.sTerminalCapab,0,sizeof(gstIccPrintData.sTerminalCapab));
		memcpy(gstIccPrintData.sTerminalCapab, psData, MIN(iDataLen, 3));
		break;

	case 0x9F34:	//zyl+
		memset(gstIccPrintData.sCVMR,0,sizeof(gstIccPrintData.sCVMR));
		memcpy(gstIccPrintData.sCVMR, psData, MIN(iDataLen, 3));
		break;
		
	case 0x9F37:
		memset(gstIccPrintData.sTerminalRand,0,sizeof(gstIccPrintData.sTerminalRand));
		memcpy(gstIccPrintData.sTerminalRand, psData, MIN(iDataLen, 4));
		break;
		
	case 0x9F74:
		memset(gstIccPrintData.sEcIssureAuthCode,0,sizeof(gstIccPrintData.sEcIssureAuthCode));
		memcpy(gstIccPrintData.sEcIssureAuthCode, psData, MIN(iDataLen, 6));	
		break;

	case 0x9F26:
		memset(gstIccPrintData.sARQC,0,sizeof(gstIccPrintData.sARQC));
		memcpy(gstIccPrintData.sARQC, psData, MIN(iDataLen, 8));
		memcpy(PosCom.stTrans.sARQC,gstIccPrintData.sARQC,8);
		break;

	case 0x9F36:
		memset(gstIccPrintData.sAppAtc,0,sizeof(gstIccPrintData.sAppAtc));
		memcpy(gstIccPrintData.sAppAtc, psData, MIN(iDataLen, 2));
		break;
		
	case 0x95:
		memset(gstIccPrintData.sTVR,0,sizeof(gstIccPrintData.sTVR));
		memcpy(gstIccPrintData.sTVR, psData, MIN(iDataLen, 5));
		gstIccPrintData.ucTVRoK = TRUE;
		break;
		
	case 0x9F1E:
		memset(gstIccPrintData.sTIDSn,0,sizeof(gstIccPrintData.sTIDSn));
		memcpy(gstIccPrintData.sTIDSn, psData, MIN(iDataLen, 8));
		gstIccPrintData.ucTIDSnoK = TRUE;
		break;

	case 0xDF31:
		memset(gstIccPrintData.sScritptRet,0,sizeof(gstIccPrintData.sScritptRet));
		memcpy(gstIccPrintData.sScritptRet, psData, MIN(iDataLen, 21));
		gstIccPrintData.ucScritptRetlen = iDataLen;
		if (gstIccPrintData.ucScritptRetlen > 21)
		{
			gstIccPrintData.ucScritptRetlen = 21;
		}	
		break;
		
	default:
		break;
	}
}

void  GetARPCTag91(char *pszData, int *iDatalen)
{
	if (pszData!=NULL && iAuthDataLen>=2)
	{
		memcpy(pszData,sAuthData,iAuthDataLen-2);  // after tag91 response code
		*iDatalen = iAuthDataLen-2;
	}
}

// end of file

