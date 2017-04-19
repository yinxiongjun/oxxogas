
#include <openssl/sha.h>
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "emvtranproc.h"
#include "otherTxn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "posparams.h"


void AfterTransBatch(void)
{
	uint8_t	ucFlag,ucRet;
	int   iStanBak;
	uint8_t sPacketHeadBak[7];
	
	memcpy(sPacketHeadBak,stTemp.sPacketHead,sizeof(stTemp.sPacketHead));
	if (stTemp.iTransNo !=POS_LOGON && stTemp.iTransNo !=POS_LOGOFF)
	{
		if (PosCom.ucSwipedFlag == CARD_INSERTED)
		{
			DispRemoveICC();
		}

		kbFlush();
		iStanBak = PosCom.stTrans.lTraceNo;
		ucFlag = FALSE;

		if (GetOfflineTxnNums(TS_TXN_OFFLINE|TS_ICC_OFFLINE)>=stPosParam.ucMaxOfflineTxn)
		{
			ucFlag = TRUE;
		}
		if (PosCom.bOnlineTxn || ucFlag)
		{
			if (ucFlag || stPosParam.ucSendOffFlag == PARAM_OPEN)
			{
				while(GetOfflineTxnNums(TS_TXN_OFFLINE|TS_ICC_OFFLINE))
				{
					ucRet = TrickleFeedOffLineTxn(TRUE);
					if (ucRet != 0)
					{
						break;
					}
					ucRet = TrickleFeedICOffLineTxn(TRUE);
					if (ucRet != 0)
					{
						break;
					}
						
					if( kbGetKeyMs(150) == KEY_CANCEL )
					{
						break;
					}
				}
			}
			
			if (gstPosCapability.uipostsScreen && stPosParam.ucSupportElecSign == PARAM_OPEN)
			{
				if( stTemp.iTransNo==CHECK_IN || stTemp.iTransNo==CHECK_OUT)
				{
					if (GetCheckElecSignTxnNums())
					{
						if (ucFlag)
							ucRet = TrickleFeedElecSignTxn(TRUE,1);
						else 
						{
							ucRet = TrickleFeedElecSignTxn(FALSE,1);
						}
					}
				}
				else
				{
					if (GetElecSignTxnNums())
					{
						if (ucFlag)
							ucRet = TrickleFeedElecSignTxn(TRUE,1);
						else 
						{
							ucRet = TrickleFeedElecSignTxn(FALSE,1);
						}
					}
				}
				
			}
	
			kbFlush();
		}	
		
		PosCom.stTrans.lTraceNo = iStanBak;
	}
	memcpy(stTemp.sPacketHead,sPacketHeadBak,sizeof(stTemp.sPacketHead));
	OnlineDownOrUp(); 
	return;
}
// end of file

