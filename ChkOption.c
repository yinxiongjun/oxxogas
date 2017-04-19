
#include "ChkOption.h"
#include "glbvar.h"
#include "posmacro.h"
#include "tranfunclist.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

uint8_t ChkIfSaveLog(void)
{
	if( stTemp.iTransNo==POS_SALE        || stTemp.iTransNo==POS_SALE_VOID    ||
		stTemp.iTransNo==POS_PREAUTH     || stTemp.iTransNo==POS_PREAUTH_VOID ||
		stTemp.iTransNo==POS_AUTH_VOID    ||stTemp.iTransNo==POS_PREAUTH_ADD || 
		stTemp.iTransNo==POS_REFUND       || stTemp.iTransNo==POS_OFF_CONFIRM ||	
		stTemp.iTransNo==ICC_OFFSALE     || stTemp.iTransNo==OFF_SALE         ||
		stTemp.iTransNo==ADJUST_TIP      ||
		(stTemp.iTransNo>=EC_QUICK_SALE && stTemp.iTransNo<=EC_VOID_TOPUP) ||       
		stTemp.iTransNo==POS_INSTALLMENT || stTemp.iTransNo==POS_VOID_INSTAL ||
		(stTemp.iTransNo==OFF_ADJUST && stTemp.bSendId==TRUE) ||
		stTemp.iTransNo == PRE_TIP_SALE ||stTemp.iTransNo==INTERESES_SALE   ||
		stTemp.iTransNo==PHONE_TOPUP || 
		stTemp.iTransNo==CHECK_IN || stTemp.iTransNo==CHECK_OUT ||
		stTemp.iTransNo==CASH_ADVACNE || stTemp.iTransNo==NETPAY_REFUND ||
		stTemp.iTransNo==PURSE_SALE)  // NETPAY
	{
		return TRUE;
	}

	return FALSE;
}


uint8_t ChkIfUpdateLog(void)
{
	if( stTemp.iTransNo==POS_SALE_VOID || stTemp.iTransNo==POS_AUTH_VOID ||
		stTemp.iTransNo==OFF_ADJUST    || stTemp.iTransNo==ADJUST_TIP ||
		stTemp.iTransNo== EC_VOID_TOPUP || stTemp.iTransNo==POS_VOID_INSTAL ||
		stTemp.iTransNo==POS_REFUND || stTemp.iTransNo==POS_AUTH_CM ||
		stTemp.iTransNo==CHECK_OUT || stTemp.iTransNo==NETPAY_ADJUST )
	{
		return TRUE;
	}

	return FALSE;
}

uint8_t ChkIfDebitTxn(int iTransType)
{
	if( iTransType==POS_AUTH_CM || iTransType==POS_OFF_CONFIRM	||
		iTransType==OFF_SALE 	|| iTransType==OFF_ADJUST		||
		iTransType==ADJUST_TIP	|| iTransType == EC_NORMAL_SALE	||
		iTransType == EC_QUICK_SALE	|| iTransType == QPBOC_ONLINE_SALE||
		iTransType == POS_INSTALLMENT)
	{
		return TRUE;
	}

	return FALSE;
}

uint8_t ChkIfCreditTxn(int iTransType)
{
	if( iTransType==POS_SALE_VOID || iTransType==POS_REFUND || 
		iTransType==POS_AUTH_VOID || iTransType== EC_TOPUP_CASH ||
		iTransType== POS_VOID_INSTAL ||
        iTransType==EC_REFUND )
	{
		return TRUE;
	}

	return FALSE;
}

uint8_t ChkIfSaleTxn(int iTransType)
{
	if( iTransType ==POS_SALE || iTransType==ICC_OFFSALE || 
		iTransType ==ADJUST_TIP || iTransType == EC_NORMAL_SALE ||
		iTransType == EC_QUICK_SALE || iTransType == QPBOC_ONLINE_SALE ||
		iTransType == POS_INSTALLMENT )
	{
		return TRUE;
	}
	
	return FALSE;
}


uint8_t ChkIfPrtDetill(int iTransType)
{
	if( iTransType ==POS_SALE ||stTemp.iTransNo==PURSE_SALE)
	{
		return TRUE;
	}

	return FALSE;
}


uint8_t ChkIfBlkCard(char *pszCardNo)
{
	int  iRet, iCnt, iCardlen;
	ST_BLACKLIST_TOTAL stBlkCard;
	char  szBuffer[30];

	if (fileExist(FILE_BLKCARD_LOG) < 0 )
	{
		return FALSE;
	}

	memset((char*)&stBlkCard,0,sizeof(stBlkCard));
	iRet = LoadBlkCard(&stBlkCard);
	if (iRet != 0)
	{
		return FALSE;
	}

	memset(szBuffer,0,sizeof(szBuffer));
	PubAsc2Bcd(pszCardNo,strlen(pszCardNo),szBuffer);

	for (iCnt=0; iCnt<stBlkCard.ulBlackListNum; iCnt++)
	{
		iCardlen = stBlkCard.szBlackList[iCnt][0];
		if ((iCardlen%2))
		{
			szBuffer[(iCardlen/2)] |= 0xF0;
		}

		if (iCardlen != 0 && memcmp(szBuffer,&stBlkCard.szBlackList[iCnt][1],(iCardlen+1)/2) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}


uint8_t ChkAcceptTxnCode(char *pszRespcode)
{
	if (memcmp(pszRespcode,"00",2) == 0)
	{
		return TRUE;
	}

	return FALSE;
}


uint8_t ChkOnlyEcCard(uint8_t *sAppAid)
{
	if (memcmp(sAppAid,"A000000333010106",16) == 0)
	{
		return TRUE;
	}

	return FALSE;
}


uint8_t ChkExTxnLog(void)
{
	int  iFilesize;

	iFilesize = fileSize(ICC_FAILUR_TXN_FILE);
	if (iFilesize <=0 )
	{
		return 0;
	}

	iFilesize = iFilesize/COM_SIZE;

	return iFilesize;
}


uint8_t ChkIfElecSignature(void)
{
	if (stPosParam.ucSupportElecSign==PARAM_OPEN && gstPosCapability.uipostsScreen 
		&& PosCom.stTrans.PinCheckSuccessFlag != 1)
	{
		if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==PAYMENT_WITH_CARD ||
			stTemp.iTransNo==PRE_TIP_SALE ||stTemp.iTransNo==INTERESES_SALE || 
			stTemp.iTransNo==CHECK_IN ||stTemp.iTransNo==PURSE_SALE)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// end of file
