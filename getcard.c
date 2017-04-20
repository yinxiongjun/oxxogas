#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "posappapi.h"
#include "tranfunclist.h"
#include "Menu.h"
#ifdef CL_LED_SUPPORT
#include "dfb_led.h"
#endif

int CheckCapkAid(void);

/*=================================================================*/
/*  description：judge emv card								           */
/*  return  ：    TRUE --> EMV card								     */
/*                      FALSE--> no EMV card						           */
/*=================================================================*/
uint8_t IsEmvCard(uint8_t *pszTrack2Data)
{
	char	*pszSeperator;
	
	if( *pszTrack2Data==0 )
	{
		return FALSE;
	}
	
	pszSeperator = strchr((char *)pszTrack2Data, '=');
	if( pszSeperator==NULL )
	{
		return FALSE;
	}
	if( (pszSeperator[5]=='2') || (pszSeperator[5]=='6') )
	{
		return TRUE;
	}
	
	return FALSE;
}

static int JudgeContactless(int iPreProcessFlag)
{
	if(iPreProcessFlag != 0)
	{
		if(iPreProcessFlag == E_SET_PARAM_FAIL)
		{
			return E_SET_PARAM_FAIL;
		}
		return E_PRE_PROCESS_FAIL;
	}
	else 
	{
		return CARD_PASSIVE; 
	}
		
}

/****************************************************************************
description:check card event
return:            1: swipe
		 	   2: insert
		 	   4: manual
****************************************************************************/
uint8_t DetectCardEvent(uint8_t ucMode,uint8_t *pDispInfo1,uint8_t *pDispInfo2,int *ierrno)
{
	int  iRet, iCnt,timeout,iPreProcessFlag=0,iPreErrno=0;
	UINT8 ucDisp=0;
	int iOldTimeMs = 0, iNewTimeMs = 0;

	iRet = 0;
	if( ucMode & CARD_SWIPED )
	{
		AppMagClose();
		AppMagOpen();
		magReset();
	}

	lcdDispScreen(90,110,Bmp_passive,ucDisp,pDispInfo1,pDispInfo2,ucMode);
	sysDelayMs(100);
	kbFlush();

	iOldTimeMs = sysGetTimerCount();
	
	*ierrno=0;
	if ( (stPosParam.ucqPbocSupport == PARAM_OPEN) && (ucMode & CARD_PASSIVE))
	{
		AppMagClose();
		AppMagOpen();
		magReset();

		contactless_close(imif_fd);
		imif_fd = -1;
		imif_fd = contactless_open();
#ifdef CL_LED_SUPPORT
    	led_set_nfc_fd(imif_fd);
#endif
    	ledSetStatus(LED_ACTIV_CARD);
		// NETPAY
		if(PosCom.stTrans.iTransNo==POS_SALE || PosCom.stTrans.iTransNo==EC_QUICK_SALE)
		{
			iPreProcessFlag = qpboc_pre_process(&iPreErrno);
			*ierrno = iPreErrno; 
		}
		contactless_poll(imif_fd);
	}

	iCnt = 0;			
	while(1)
	{
		if ( (stPosParam.ucqPbocSupport == PARAM_OPEN) && (ucMode & CARD_PASSIVE))
		{
			iRet = contactless_poweredon(imif_fd);
			if (iRet != 0)
			{           
				if (iRet == MIF_RET_MULTICARD)
				{
					timeout  = sysGetTimerCount() + 1000;
					while ((iRet = contactless_poweredon(imif_fd))) 
					{
						if (iRet == MIF_RET_NOCARD || iRet == MIF_RET_MULTICARD)
						{
							if (sysGetTimerCount() > timeout )
								break;
						}
						sysDelayMs(10);
					}
					if (iRet ==MIF_RET_SUCCESS)
					{
						return JudgeContactless(iPreProcessFlag);  
					}

					if (iRet == MIF_RET_MULTICARD)
					{
						ledSetStatus(LED_TRANS_FAIL);
						contactless_close(imif_fd);
						imif_fd = -1;
						lcdClrLine(2,7);
						lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"MULTI CARDS");
						lcdFlip();
						QPBOCErrorBeep();
						PubWaitKey(5);
						return NO_DISP;
					}
				}
			}
			else
			{
				return JudgeContactless(iPreProcessFlag);   
			}
			
		}	
		if( kbhit()==YES )
		{
			ledSetStatus(LED_NOT_READY);
			if (ucMode & CARD_PASSIVE)
			{
				contactless_close(imif_fd);
				imif_fd = -1;
			}
			return CARD_KEYIN;
		}
		if( (ucMode & CARD_SWIPED) && (magSwiped()==YES) )
		{
			ledSetStatus(LED_NOT_READY);
			if (ucMode & CARD_PASSIVE)
			{
				contactless_close(imif_fd);
				imif_fd = -1;
			}
			return CARD_SWIPED;
		}
		if( (stPosParam.ucEmvSupport==PARAM_OPEN) && (ucMode & CARD_INSERTED) &&
			(iccDetect(ICC_USERCARD)==ICC_SUCCESS) )
		{
			ledSetStatus(LED_NOT_READY);
			if (ucMode & CARD_PASSIVE)
			{
				contactless_close(imif_fd);
				imif_fd = -1;
			}
			return CARD_INSERTED;
		}
		sysDelayMs(10); // Avoid CPU resources occupation
		iNewTimeMs = sysGetTimerCount();
        if( (iNewTimeMs - iOldTimeMs) >= (uint32_t)(stPosParam.ucOprtLimitTime * 1000) ) 
        {
        	ledSetStatus(LED_NOT_READY);
		if (ucMode & CARD_PASSIVE)
		{
			contactless_close(imif_fd);
			imif_fd = -1;
		}
            return KEY_CANCEL;
        }
	}
}


void ShowGetCardScreen(uint8_t ucMode,uint8_t *pDispBuf1,uint8_t *pDispBuf2)
{
	uint8_t	szDispBuf[33],szDispBuf2[33];

	memset(szDispBuf2,0,sizeof(szDispBuf2));
	switch( ucMode & 0x7F )
	{
	case CARD_SWIPED:
		if (PosCom.stTrans.iTransNo == EC_TOPUP_NORAML)
			sprintf((char *)szDispBuf, "Swipe(Transfer)-->");
		else
			sprintf((char *)szDispBuf, "SWIPE-->");
		break;

	case CARD_INSERTED:
		sprintf((char *)szDispBuf, "INSERT-->");
		break;

	case CARD_KEYIN:
		sprintf((char *)szDispBuf, "Cuenta");
	    break;

	case CARD_SWIPED|CARD_KEYIN:
		sprintf((char *)szDispBuf, "Inserte / Deslice");
		sprintf((char *)szDispBuf2, "TARJETA");
	    break;

	case CARD_SWIPED|CARD_INSERTED:
		if (PosCom.stTrans.iTransNo == EC_TOPUP_NORAML)
		{
			sprintf((char *)szDispBuf, "Inserte / Deslice");
			sprintf((char *)szDispBuf2, "TRANSFER CARD");			
		}
		else
		{
			sprintf((char *)szDispBuf, "Inserte / Deslice");
		}
		break;

	case CARD_INSERTED|CARD_KEYIN:
		sprintf((char *)szDispBuf, "Inserte/Manual");
		break;

	case CARD_INSERTED|CARD_KEYIN|CARD_SWIPED:
		sprintf((char *)szDispBuf, "Inserte / Deslice");
		sprintf((char *)szDispBuf2, "TARJETA");
		break;

	case CARD_PASSIVE:
		sprintf((char *)szDispBuf, "TAPE");
		break;

	case CARD_PASSIVE|CARD_INSERTED:
		sprintf((char *)szDispBuf, "Insert Tarjeta");
		break;

	case CARD_INSERTED|CARD_KEYIN|CARD_SWIPED|CARD_PASSIVE:
		sprintf((char *)szDispBuf, "Inserte / Deslice");
		sprintf((char *)szDispBuf2, "TARJETA");
		break;

	case CARD_INSERTED|CARD_SWIPED|CARD_PASSIVE:
		sprintf((char *)szDispBuf, "Inserte / Deslice");
		sprintf((char *)szDispBuf2, "TARJETA");
		break;

	default:
	    return;
	}
	memcpy(pDispBuf1,szDispBuf,strlen((char *)szDispBuf));
	memcpy(pDispBuf2,szDispBuf2,strlen((char *)szDispBuf2));
	pDispBuf1[strlen((char *)szDispBuf)] = 0;
	pDispBuf2[strlen((char *)szDispBuf2)] = 0;

}

void SaleGetCardScreen(uint8_t ucMode,uint8_t *pDispBuf1,uint8_t *pDispBuf2)
{
	uint8_t	 szAmount[15],szTmpBuff[40],szDispBuf1[40],szDispBuf2[40];
	int      iAmount;

	memset(szDispBuf2,0,sizeof(szDispBuf2));

	iAmount = PubBcd2Long(PosCom.stTrans.sAmount,6,NULL);
	if (iAmount==0)
	{
		if (ucMode==CARD_SWIPED)
		{
			strcpy((char *)szDispBuf1,"SWIPE-->");
		}
		else
		{
			strcpy((char *)szDispBuf1,"      Inserte/Deslice      TARJETA");
		}
	}
	else
	{
		memset(szAmount,0,sizeof(szAmount));
//		ConvBcdAmount(PosCom.stTrans.sAmount, szAmount);
		sprintf((char *)szTmpBuff, "%s", szAmount);
		strcpy((char *)szDispBuf1,(char *)szTmpBuff);
		if (ucMode==CARD_SWIPED)
		{
			strcpy((char *)szDispBuf2,"Inserte/Deslice         TARJETA");
			memset(szDispBuf1,0,sizeof(szDispBuf1));
		}
		else
		{
			strcpy((char *)szDispBuf2,"   Inserte/Deslice      TARJETA");
			memset(szDispBuf1,0,sizeof(szDispBuf1));
		}
	}
	memcpy(pDispBuf1,szDispBuf1,strlen((char *)szDispBuf1));
	memcpy(pDispBuf2,szDispBuf2,strlen((char *)szDispBuf2));
	pDispBuf1[strlen((char *)szDispBuf1)] = 0;
	pDispBuf2[strlen((char *)szDispBuf2)] = 0;
}

void ShowICCardScreen(uint8_t ucMode,uint8_t *pDispBuf1,uint8_t *pDispBuf2)
{
	uint8_t szDispBuf1[30],szDispBuf2[30];

	lcdClrLine(2,7);
//	strcpy((char *)szDispBuf1,"IC CARD");
	switch( ucMode & 0x7F )
	{
	case CARD_INSERTED|CARD_PASSIVE:
		strcpy((char *)szDispBuf2,"INSERTE TARJETA");
		break;
	case CARD_INSERTED|CARD_PASSIVE|CARD_KEYIN:
	case CARD_INSERTED|CARD_KEYIN:
		strcpy((char *)szDispBuf2,"INSERT/MANUAL");
		break;
	default:
		strcpy((char *)szDispBuf2,"INSERTE TARJETA");
		break;
	}
	memcpy(pDispBuf1,szDispBuf1,strlen((char *)szDispBuf1));
	memcpy(pDispBuf2,szDispBuf2,strlen((char *)szDispBuf2));
	pDispBuf1[strlen((char *)szDispBuf1)] = 0;
	pDispBuf2[strlen((char *)szDispBuf2)] = 0;
}

#if 0
{
    	uint8_t	ucRet=0, bCheckICC, ucFlag=0,ucSwipe=0;
	int     iRet,iErrNo;
	uint8_t sDispBuf1[30],sDispBuf2[30];
	int ierrno;
	int InsertTime = 0;
	uint8_t mRet;
	
	if( PosCom.ucSwipedFlag!=NO_SWIPE_INSERT )
	{
		return OK;
	}
	
	bCheckICC = !(ucMode & SKIP_CHECK_ICC);
	while(1)
	{
		if (ucFlag == 0)
		{
			if( ucRet==E_NEED_INSERT )
				ShowICCardScreen(ucMode,sDispBuf1,sDispBuf2);
			else if (stTemp.iTransNo==POS_SALE|| 
				stTemp.iTransNo==EC_QUICK_SALE || 
				stTemp.iTransNo==EC_NORMAL_SALE ||
				stTemp.iTransNo==POS_QUE || 
				stTemp.iTransNo==POS_REFUND || 
				stTemp.iTransNo==POS_PREAUTH || 
				stTemp.iTransNo==CHECK_IN ||
				stTemp.iTransNo==CHECK_OUT ||
				stTemp.iTransNo==CASH_ADVACNE ||
				stTemp.iTransNo==NETPAY_REFUND ||
				stTemp.iTransNo==NETPAY_ADJUST )
				SaleGetCardScreen(ucMode,sDispBuf1,sDispBuf2);
			else
				ShowGetCardScreen(ucMode,sDispBuf1,sDispBuf2);
		}
		else
		{
			ucFlag = 0;
		}
RE_INSERT:
		ucRet = DetectCardEvent(ucMode,sDispBuf1,sDispBuf2,&ierrno);
		PrintDebug("%s%d", "DetectCardEvent:",ucRet);
		//printf("DetectCardEvent:%d\n",ucRet);
		if( ucRet==CARD_KEYIN )
		{
			if (ucFlag == 1)
			{	
				if( kbGetKey()== KEY_CANCEL )
				{
					return E_TRANS_CANCEL;
				}
				else
				{
					ucFlag = 0;
					continue;
				}
			}
			if( ucMode & CARD_KEYIN )
			{
				ucRet = ManualEntryCardNo(PosCom.stTrans.szCardNo,ucMode);
				if( ucRet!=OK )
				{
					return ucRet;
				}
				
				PreDial();
				GetCardType();

			FORZADA_REINPUT:				
				ucRet = SelectMoneyType();
				if( ucRet != OK )
				{
					return ucRet;
				}

				if( stTemp.iTransNo==NETPAY_FORZADA && 
					(stPosParam.Switch_Retail==PARAM_OPEN || stPosParam.Switch_Hotel==PARAM_OPEN))
				{
					iRet = SelectSaleType();
					if( iRet == 101)    // 因为E_TRANS_CANCEL = 1,为了避免相等设置其他值
					{	
						iRet = InputWaiterNo();
						if(iRet != OK){
							return iRet;
						}
					}
					else if(iRet == 102)
					{
						iRet = SelectInteresesSaleType();
						if(iRet != OK){
							return iRet;
						}
					}
					else {
						return iRet;
					}
					
			
					iRet = AppGetAmount(9, TRAN_AMOUNT);
					if( iRet!=OK )
					{
						return iRet;
					}
					//确认其他金额
					
					ucRet = ComfirAmount(PosCom.stTrans.sAmount);
					if(ucRet == RE_SELECT){
						goto FORZADA_REINPUT;
					}
					else if( ucRet!=OK )
					{
						return ucRet;
					}
				}
				ucRet = ManualEntryExpDate(PosCom.stTrans.szExpDate);
				if( ucRet != OK )
				{
					return ucRet;
				}

				ucRet = EnterSeguridadCode();
				if( ucRet != OK )
				{
					return ucRet;
				}
				
				return ManualAutorNo();				
			}
			else if( kbGetKey()==KEY_CANCEL )
			{
				contactless_close(imif_fd);
				imif_fd = -1;
				return E_TRANS_CANCEL;
			}

		}
		else if( ucRet==CARD_SWIPED ) 
		{	
			ucRet = SwipeCardProc(bCheckICC);

			if( ucRet==OK )
			{
				//余额查询直接退出
				printf("stTemp.iTransNo:%d;%d\n",stTemp.iTransNo,PURSE_GETBALANCE);
				if(stTemp.iTransNo==PURSE_GETBALANCE ||stTemp.iTransNo ==PURSE_GETRULE)
				{
					printf("return ok\n");
					return OK;
				}
					
				//添加对卡型号的判断
				mRet = CheckCardType();
				if(mRet !=OK)
				{
					goto RE_INSERT;
				}
				
				if(stPosParam.stVolContrFlg ==PARAM_CLOSE)
				{
		RESELECT:
					iRet = AppGetAmount(9, TRAN_AMOUNT);
					if( iRet!=OK )
					{
						return iRet;
					}
					ucRet = ComfirAmount(PosCom.stTrans.sAmount);
					if(ucRet == RE_SELECT)
					{
						goto RESELECT;
					}
					else 
					{
						return ucRet;
					}
					/*
					else if( ucRet!=OK )
					{
						return ucRet;
					}
					*/
				}
				else
				{
					iRet = GetAmount_FromGasPlat();
					if(iRet != OK)
					{
						if(iRet ==RE_SELECT)
						{
							while(1)
							{
								iRet = AppGetAmount(9, TRAN_AMOUNT);
								if( iRet!=OK )
								{
									return iRet;
								}
								ucRet = ComfirAmount(PosCom.stTrans.sAmount);
								if(ucRet == OK)
								{
									break;
								}
								else if(ucRet == NO_DISP)
								{
									return iRet;
	
								}
							}
							
						}
						else
						{
							return iRet;
						}
					}
					if(stPosParam.szpreTip == PARAM_CLOSE)
					{
						return OK;
					}
					//钱包消费不需要输小费
					if(stTemp.iTransNo ==PURSE_SALE)
					{
						return OK;
					}
					//小费输入

			RECELECTTO:
					ucRet = AppGetTip();
					if(ucRet != OK)
					{
						return iRet;
					}
					//确认小费金额
					iRet = ConfirmTipAmount();
					
					if(iRet == RE_SELECT)
					{
						goto RESELECT;
					}
					else 
					{
						return iRet;
					}				
					
				}		
			}
			else if( ucRet==E_ERR_SWIPE )
			{
				strcpy((char *)sDispBuf1,"DESLICE TARJETA");
				if( kbhit()==YES )
				{
					if( kbGetKey()==KEY_CANCEL )
					{
						return NO_DISP;
					}
				}
				ucFlag = 1;
				
			}
			else if( ucRet==E_NEED_INSERT )
			{
				if( (ucMode & CARD_INSERTED)==0 ) // transaction not promise insert
				{
					return E_NEED_INSERT;
				}
				ucMode &= ~CARD_SWIPED;	// delete swipe
				ucSwipe = 1;
			}
			else
			{
				return ucRet;                
			}

		}
		else if( ucRet==CARD_INSERTED )
		{
			CheckCapkAid();
			ucRet = InsertCardProc(FALSE);
			if( ( InsertTime<2 ) && ( ucRet==E_NEED_FALLBACK ) )
			{
				InsertTime++;
				goto RE_INSERT;
			}

			if( ucRet==E_NEED_FALLBACK )
			{
				if( stPosParam.EmvFallBack != PARAM_OPEN )
				{
					lcdClrLine(2, 7);	
					lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
					lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "NO SE LEE EL CHIP");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);
					lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
					return NO_DISP;
				}
				
				// 不允许降级交易
				if(( (ucMode & CARD_SWIPED)==0 )&& ucSwipe!=1)//ucSwipe:1 is fallbacl
				{
					return NO_DISP;
				}
				memset(sDispBuf1,0,sizeof(sDispBuf1));
				strcpy((char *)sDispBuf2,"Deslice tarjeta-->");
				if( kbhit()==YES )
				{
					if( kbGetKey()==KEY_CANCEL )
					{
						return NO_DISP;
					}
				}
				ucFlag = 1;
				ucMode = CARD_SWIPED;
				PosCom.ucFallBack = TRUE;
				bCheckICC = TRUE;
			}
			else if( ucRet==E_CARD_BLOCKER )
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "CARD LOCKED");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(2000);		

				DispRemoveICC();
				return NO_DISP;
			}
			else if( ucRet==E_APP_BLOCKED )
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "APP LOCKED");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(2000);		

				DispRemoveICC();
				return NO_DISP;
			}
			else if( ucRet==E_TRANS_FAIL )
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "TRAN REFUSE");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(2000);		

				DispRemoveICC();
				return NO_DISP;
			}
	#if 0
			else if(ucRet ==OK)
			{
				if(stPosParam.stVolContrFlg ==PARAM_CLOSE)
				{
					while(1)
					{
						iRet = AppGetAmount(9, TRAN_AMOUNT);
						if( iRet!=OK )
						{
							return iRet;
						}
						iErrNo = ComfirAmount(PosCom.stTrans.sAmount);
						if(iErrNo != RE_SELECT)
						{
							return iErrNo;
						}
					}
				}
				else
				{
					iRet = GetAmount_FromGasPlat();
					if(iRet != OK)
					{
						if(iRet ==RE_SELECT)
						{
							while(1)
							{
								iRet = AppGetAmount(9, TRAN_AMOUNT);
								if( iRet!=OK )
								{
									return iRet;
								}
								ucRet = ComfirAmount(PosCom.stTrans.sAmount);
								if(ucRet == OK)
								{
									break;
								}
								else if(ucRet == NO_DISP)
								{
									return iRet;
								}
							}
							
						}
						else
						{
							return iRet;
						}
					}
					
					//钱包消费不需要输小费
					if(stPosParam.szpreTip == PARAM_CLOSE || stTemp.iTransNo ==PURSE_SALE)
					{
						return DispCardNo(PosCom.stTrans.szCardNo);
					}

					//小费输入

					while(1)
					{
						ucRet = AppGetTip();
						if(ucRet != OK)
						{
							return iRet;
						}
						//确认小费金额
						iRet = ConfirmTipAmount();
						
						if(iRet != RE_SELECT)
						{
							return iRet;
						}		
					}			
				}
			}
		#endif
			else
			{
				return ucRet;
			}
		}
		else if (((ucMode & CARD_PASSIVE) && ucRet==CARD_PASSIVE)||(ucRet == E_PRE_PROCESS_FAIL)
			||(ucRet == E_SET_PARAM_FAIL))
		{
			CheckCapkAid();
			ledSetStatus(LED_TRANS_HANDLING);
			if(ucRet == E_PRE_PROCESS_FAIL)
			{
				DisplayQpbocError(ierrno);
				return NO_DISP;
			}
			else if(ucRet == E_SET_PARAM_FAIL)
			{
				ledSetStatus(LED_TRANS_FAIL);
				lcdCls();
				lcdDisplay(0 ,3 , DISP_CFONT | DISP_MEDIACY , "SET PARAM FAIL");
				lcdFlip();
				kbGetKey();
				return NO_DISP;
			}
			else
			{
        		if (PosCom.stTrans.iTransNo == EC_REFUND)
          			ucRet = ClpbocGetCard();
        		else
        		{
          			PosCom.stTrans.iTransNo = EC_QUICK_SALE;
          			stTemp.iTransNo = EC_QUICK_SALE;
          			ucRet = ContackCardProc();
        		}
        		if (ucRet == 0)
        		{
          			return 0;
        		}
			}
			if (ucRet != 0)
			{
				contactless_close(imif_fd);
				imif_fd = -1;
			}
			return ucRet;
		}
		else if(ucRet == E_PRE_PROCESS_FAIL || ucRet == KEY_CANCEL)
			return NO_DISP;
	}

	return ucRet;
}
#endif

uint8_t PosGetCard(uint8_t ucMode)
{
    	uint8_t	ucRet=0, bCheckICC, ucFlag=0,ucSwipe=0;
	int     iRet,iErrNo;
	uint8_t sDispBuf1[30],sDispBuf2[30];
	int ierrno;
	int InsertTime = 0;
	uint8_t mRet;

	bCheckICC = !(ucMode & SKIP_CHECK_ICC);
	while(1)
	{
		if (ucFlag == 0)
		{
			if( ucRet==E_NEED_INSERT )
				ShowICCardScreen(ucMode,sDispBuf1,sDispBuf2);
			else if (stTemp.iTransNo==POS_SALE|| 
				stTemp.iTransNo==EC_QUICK_SALE || 
				stTemp.iTransNo==EC_NORMAL_SALE ||
				stTemp.iTransNo==POS_QUE || 
				stTemp.iTransNo==POS_REFUND || 
				stTemp.iTransNo==POS_PREAUTH || 
				stTemp.iTransNo==CHECK_IN ||
				stTemp.iTransNo==CHECK_OUT ||
				stTemp.iTransNo==CASH_ADVACNE ||
				stTemp.iTransNo==NETPAY_REFUND ||
				stTemp.iTransNo==NETPAY_ADJUST )
				SaleGetCardScreen(ucMode,sDispBuf1,sDispBuf2);
			else
				ShowGetCardScreen(ucMode,sDispBuf1,sDispBuf2);
		}
		else
		{
			ucFlag = 0;
		}

		if(start_from_manager ==1 )
		{
			if(PosCom.ucSwipedFlag ==CARD_INSERTED)
			{
				return OK;
			}
			
			if(stPosParam.stVolContrFlg ==PARAM_CLOSE)
			{
		RRE_SELECT:
				iRet = AppGetAmount(9, TRAN_AMOUNT);
				if( iRet!=OK )
				{
					return iRet;
				}
				ucRet = ComfirAmount(PosCom.stTrans.sAmount);
				if(ucRet == RE_SELECT)
				{
					goto RRE_SELECT;
				}
				else 
				{
					return ucRet;
				}

			}
			else
			{
				PosCom.stTrans.CtlFlag =1;
				iRet = GetAmount_FromGasPlat();
				if(iRet != OK)
				{
					if(iRet ==RE_SELECT)
					{
						while(1)
						{
							iRet = AppGetAmount(9, TRAN_AMOUNT);
							if( iRet!=OK )
							{
								return iRet;
							}
							ucRet = ComfirAmount(PosCom.stTrans.sAmount);
							if(ucRet == OK)
							{
								break;
							}
							else if(ucRet == NO_DISP)
							{
								return iRet;
							}
						}
						
					}
					else
					{
						return iRet;
					}
				}
				//钱包消费不需要输小费
				if(stPosParam.szpreTip == PARAM_CLOSE ||stTemp.iTransNo ==PURSE_SALE)
				{
					return OK;
				}
				//小费输入
				do{
					ucRet = AppGetTip();
					if(ucRet != OK)
					{
						return iRet;
					}
					//确认小费金额
					iRet = ConfirmTipAmount();
					
					if(iRet == RE_SELECT)
					{
						continue;
					}
					else 
					{
						return iRet;
					}				
				}while(1);		
			}		
		
		}
		else
		{
		#if 1
	RE_INSERT:
			ucRet = DetectCardEvent(ucMode,sDispBuf1,sDispBuf2,&ierrno);
			if( ucRet==CARD_KEYIN )
			{
				if (ucFlag == 1)
				{	
					if( kbGetKey()== KEY_CANCEL )
					{
						return E_TRANS_CANCEL;
					}
					else
					{
						ucFlag = 0;
						continue;
					}
				}
				if( ucMode & CARD_KEYIN )
				{
					ucRet = ManualEntryCardNo(PosCom.stTrans.szCardNo,ucMode);
					if( ucRet!=OK )
					{
						return ucRet;
					}
					
					PreDial();
					GetCardType();

				FORZADA_REINPUT:				
					ucRet = SelectMoneyType();
					if( ucRet != OK )
					{
						return ucRet;
					}

					if( stTemp.iTransNo==NETPAY_FORZADA && 
						(stPosParam.Switch_Retail==PARAM_OPEN || stPosParam.Switch_Hotel==PARAM_OPEN))
					{
						iRet = SelectSaleType();
						if( iRet == 101)    // 因为E_TRANS_CANCEL = 1,为了避免相等设置其他值
						{	
							iRet = InputWaiterNo();
							if(iRet != OK){
								return iRet;
							}
						}
						else if(iRet == 102)
						{
							iRet = SelectInteresesSaleType();
							if(iRet != OK){
								return iRet;
							}
						}
						else {
							return iRet;
						}
						
				
						iRet = AppGetAmount(9, TRAN_AMOUNT);
						if( iRet!=OK )
						{
							return iRet;
						}
						//确认其他金额
						
						ucRet = ComfirAmount(PosCom.stTrans.sAmount);
						if(ucRet == RE_SELECT){
							goto FORZADA_REINPUT;
						}
						else if( ucRet!=OK )
						{
							return ucRet;
						}
					}
					ucRet = ManualEntryExpDate(PosCom.stTrans.szExpDate);
					if( ucRet != OK )
					{
						return ucRet;
					}

					ucRet = EnterSeguridadCode();
					if( ucRet != OK )
					{
						return ucRet;
					}
					
					return ManualAutorNo();				
				}
				else if( kbGetKey()==KEY_CANCEL )
				{
					contactless_close(imif_fd);
					imif_fd = -1;
					return E_TRANS_CANCEL;
				}

			}
			else if( ucRet==CARD_SWIPED ) 
			{	
				ucRet = SwipeCardProc(bCheckICC);

				if( ucRet==OK )
				{
					//余额查询直接退出
					PrintDebug("%s%d%d", "stTemp.iTransNo:",stTemp.iTransNo,PURSE_GETBALANCE);

					if(stTemp.iTransNo==PURSE_GETBALANCE ||stTemp.iTransNo ==PURSE_GETRULE)
					{
						return OK;
					}
						
					//添加对卡型号的判断
					mRet = CheckCardType();
					if(mRet !=OK)
					{
						goto RE_INSERT;
					}
					
					if(stPosParam.stVolContrFlg ==PARAM_CLOSE)
					{
						while(1)
						{
							iRet = AppGetAmount(9, TRAN_AMOUNT);
							if( iRet!=OK )
							{
								return iRet;
							}
							ucRet = ComfirAmount(PosCom.stTrans.sAmount);
							if(ucRet == RE_SELECT)
							{
								continue;
							}
							else 
							{
								return ucRet;
							}
						}
					}
					else
					{
						PosCom.stTrans.CtlFlag =1;
						iRet = GetAmount_FromGasPlat();
						if(iRet != OK)
						{
							if(iRet ==RE_SELECT)
							{
								while(1)
								{
									iRet = AppGetAmount(9, TRAN_AMOUNT);
									PrintDebug("%s%d", "AppGetAmount:",iRet);
									
									if( iRet!=OK )
									{
										return iRet;
									}
									ucRet = ComfirAmount(PosCom.stTrans.sAmount);
									if(ucRet == OK)
									{
										break;
									}
									else if(ucRet == NO_DISP)
									{
										return iRet;
		
									}
								}
								
							}
							else
							{
								return iRet;
							}
						}
						if(stPosParam.szpreTip == PARAM_CLOSE)
						{
							return OK;
						}
						//钱包消费不需要输小费
						if(stTemp.iTransNo ==PURSE_SALE)
						{
							return OK;
						}
						//小费输入
						while(1)
						{
							ucRet = AppGetTip();
							if(ucRet != OK)
							{
								return iRet;
							}
							//确认小费金额
							iRet = ConfirmTipAmount();
							
							if(iRet == RE_SELECT)
							{
								continue;
							}
							else 
							{
								return iRet;
							}		
						}						
					}		
				}
				else if( ucRet==E_ERR_SWIPE )
				{
					strcpy((char *)sDispBuf1,"DESLICE TARJETA");
					if( kbhit()==YES )
					{
						if( kbGetKey()==KEY_CANCEL )
						{
							return NO_DISP;
						}
					}
					ucFlag = 1;
					
				}
				else if( ucRet==E_NEED_INSERT )
				{
					if( (ucMode & CARD_INSERTED)==0 ) // transaction not promise insert
					{
						return E_NEED_INSERT;
					}
					ucMode &= ~CARD_SWIPED;	// delete swipe
					ucSwipe = 1;
				}
				else
				{
					return ucRet;                
				}

			}
			else if( ucRet==CARD_INSERTED )
			{
				CheckCapkAid();
				ucRet = InsertCardProc(FALSE);
				if( ( InsertTime<2 ) && ( ucRet==E_NEED_FALLBACK ) )
				{
					InsertTime++;
					goto RE_INSERT;
				}

				if( ucRet==E_NEED_FALLBACK )
				{
					if( stPosParam.EmvFallBack != PARAM_OPEN )
					{
						lcdClrLine(2, 7);	
						lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
						lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "NO SE LEE EL CHIP");
						lcdFlip();
						ErrorBeep();
						kbGetKeyMs(2000);
						lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
						return NO_DISP;
					}
					
					// 不允许降级交易
					if(( (ucMode & CARD_SWIPED)==0 )&& ucSwipe!=1)//ucSwipe:1 is fallbacl
					{
						return NO_DISP;
					}
					memset(sDispBuf1,0,sizeof(sDispBuf1));
					strcpy((char *)sDispBuf2,"Deslice tarjeta-->");
					if( kbhit()==YES )
					{
						if( kbGetKey()==KEY_CANCEL )
						{
							return NO_DISP;
						}
					}
					ucFlag = 1;
					ucMode = CARD_SWIPED;
					PosCom.ucFallBack = TRUE;
					bCheckICC = TRUE;
				}
				else if( ucRet==E_CARD_BLOCKER )
				{
					lcdClrLine(2, 7);	
					lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "CARD LOCKED");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);		

					DispRemoveICC();
					return NO_DISP;
				}
				else if( ucRet==E_APP_BLOCKED )
				{
					lcdClrLine(2, 7);	
					lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "APP LOCKED");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);		

					DispRemoveICC();
					return NO_DISP;
				}
				else if( ucRet==E_TRANS_FAIL )
				{
					lcdClrLine(2, 7);	
					lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "TRAN REFUSE");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);		

					DispRemoveICC();
					return NO_DISP;
				}
				else if(ucRet ==E_FILE_SEEK )
				{
					DispRemoveICC();
					goto RE_INSERT;
				}
				else
				{
					return ucRet;
				}
			}
			else if (((ucMode & CARD_PASSIVE) && ucRet==CARD_PASSIVE)||(ucRet == E_PRE_PROCESS_FAIL)
				||(ucRet == E_SET_PARAM_FAIL))
			{
				CheckCapkAid();
				ledSetStatus(LED_TRANS_HANDLING);
				if(ucRet == E_PRE_PROCESS_FAIL)
				{
					DisplayQpbocError(ierrno);
					return NO_DISP;
				}
				else if(ucRet == E_SET_PARAM_FAIL)
				{
					ledSetStatus(LED_TRANS_FAIL);
					lcdCls();
					lcdDisplay(0 ,3 , DISP_CFONT | DISP_MEDIACY , "SET PARAM FAIL");
					lcdFlip();
					kbGetKey();
					return NO_DISP;
				}
				else
				{
	        		if (PosCom.stTrans.iTransNo == EC_REFUND)
	          			ucRet = ClpbocGetCard();
	        		else
	        		{
	          			PosCom.stTrans.iTransNo = EC_QUICK_SALE;
	          			stTemp.iTransNo = EC_QUICK_SALE;
	          			ucRet = ContackCardProc();
	        		}
	        		if (ucRet == 0)
	        		{
	          			return 0;
	        		}
				}
				if (ucRet != 0)
				{
					contactless_close(imif_fd);
					imif_fd = -1;
				}
				return ucRet;
			}
			else if(ucRet == E_PRE_PROCESS_FAIL || ucRet == KEY_CANCEL)
				return NO_DISP;
		#endif

		}
	}

	return ucRet;
}


uint8_t ManualAutorNo(void)
{
	uint8_t inBuf[6];
	int iRet;

	while(1) 
	{
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		if( stTemp.iTransNo==NETPAY_FORZADA )
		{
			
			lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VENTA FORZADA %s",PosCom.stTrans.AscCardType);
		}

		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "Codigo Autorizacion:");
		lcdFlip();
		
		memset(inBuf, 0, sizeof(inBuf));
		lcdGoto(120, 8);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 6, 
			(stPosParam.ucOprtLimitTime*1000), (char *)inBuf);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

		if( iRet<0 )
			return (E_TRANS_CANCEL);

		if( strlen((char *)inBuf)!=6 )
		{
			lcdClrLine(2, 7);	
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "INGRESE 6 DIGITOS");
			lcdFlip();
			PubWaitKey(3);
			continue;
		}

		
		strcpy((char *)PosCom.stTrans.szAC, (char *)inBuf);
		break;
			
		
	}

	sprintf((char*)PosCom.szTrack2,"%s=%s",(char*)PosCom.stTrans.szCardNo,(char*)PosCom.stTrans.szExpDate);
	return OK;
}


uint8_t ManualEntryExpDate(uint8_t *szExpDate)
{
	uint8_t inBuf[6];
	char  chkDate[9];
	int iRet;

	while(1) 
	{
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		if( stTemp.iTransNo==NETPAY_FORZADA )
		{
			
			lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VENTA FORZADA %s",PosCom.stTrans.AscCardType);
		}

		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "CAT:%s",PosCom.stTrans.szCardNo);
		lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "Fecha Exp(MMAA)",PosCom.stTrans.szCardNo);
		lcdFlip();
		
		memset(inBuf, 0, sizeof(inBuf));
		lcdGoto(120, 10);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 4, 
			(stPosParam.ucOprtLimitTime*1000), (char *)inBuf);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

		if( iRet<0 )
			return (E_TRANS_CANCEL);
		#if 0
		if( strlen((char *)inBuf)==0 )
			break;
		#endif
		if( strlen((char *)inBuf)!=4 )
		{
			lcdClrLine(2, 7);	
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "INGRESE 4 DIGITOS");
			lcdFlip();
			PubWaitKey(3);
			continue;
		}
//		memcpy(chkDate, inBuf, 4);
		memcpy(chkDate, inBuf+2, 2);
		memcpy(chkDate+2, inBuf, 2);
		if( CheckYYMM(chkDate)==0 )
		{
			strcpy((char *)PosCom.stTrans.szExpDate, (char *)inBuf);
			break;
		}	
		else
		{
			FailBeep();
			lcdClrLine(2, 7);	
			lcdDisplay(0, 6, DISP_CFONT, "INVALID FORMAT");
			lcdFlip();
			kbGetKeyMs(1000);
		}			
	}

	sprintf((char*)PosCom.szTrack2,"%s=%s",(char*)PosCom.stTrans.szCardNo,(char*)PosCom.stTrans.szExpDate);
	return OK;
}

uint8_t ManualEntryCardNo(uint8_t *pszCardNo,uint8_t ucMode)
{
	uint8_t	sBuff[21];
	int   iRet;

	while(1)
	{
		if (ucMode==CARD_KEYIN)
		{
			lcdClrLine(4,7);
		}
		else
		{
			lcdClrLine(2,7);
		}
		lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "Cuenta");
		lcdGoto(0, 4);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 16, 
			(stPosParam.ucOprtLimitTime*1000), (char *)sBuff);
		if( iRet<0 )
		{
			return E_TRANS_CANCEL;
		}
		else if( iRet < 15 || iRet > 16)
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "INGRESE ENTRE");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "15 Y 16 DIGITOS");
			lcdFlip();
			FailBeep();
			sysDelayMs(1500);
			continue;
		}
		else
			break;
	}
	
	sprintf((char *)pszCardNo, "%.*s", iRet, sBuff);
	PosCom.ucSwipedFlag = CARD_KEYIN;
	PosCom.stTrans.ucSwipeFlag = CARD_KEYIN;

	return OK;
}

uint8_t GetTrackData(uint8_t *pszTrack2, uint8_t *pszTrack3, uint8_t *pszCardNo)
{
	uint8_t	ucRet;
	uint8_t	ucError;
	int   iCardLen=0;
	uint8_t pszTrack1[100] ={0};

	ucRet = magRead(pszTrack1, pszTrack2, pszTrack3);
	PrintDebug("%s%d", "magRead:",ucRet);
	PrintDebug("%s%s%s", "magRead:",pszTrack1,pszTrack2,pszTrack3);

	if( ((ucRet&0x70)!=0x00) && ((ucRet&0x70)!=0x10))
	{
		 //1磁道校验错误，忽略
		return E_ERR_SWIPE;
	}

	GetCardHolderName(pszTrack1);
	strcpy((char*)PosCom.szTrack1,(char*)pszTrack1);
	
	iCardLen = strlen((char *)pszTrack2);
	//电子钱包punto clave 卡二磁道只有卡号，先屏蔽这个判断
/*
	if( iCardLen<24 || iCardLen>37 )
	{
		return E_ERR_SWIPE;
	}
*/
	return GetCardNoFromTrack(pszCardNo, pszTrack2, pszTrack3);
}

uint8_t GetCardType(void)
{
	char    szAscCardNoTemp[7+1] = {0};
	long    szlCardNo;

	if( strlen((char*)PosCom.stTrans.szCardNo) == 0 )
	{
		return NO_DISP;		//CHECK OUT
	}
	
	//保存卡类型
	memcpy(szAscCardNoTemp,PosCom.stTrans.szCardNo,7);
	memset(PosCom.stTrans.AscCardType,0,sizeof(PosCom.stTrans.AscCardType));

	szlCardNo = atol(szAscCardNoTemp);
	if( (szlCardNo >= 4000000 && szlCardNo <= 4999999 ) ||(szlCardNo >= 2020220 && szlCardNo <= 2869009 ))
	{
		PosCom.stTrans.CardType = 1;  //visa card
		strcpy((char*)PosCom.stTrans.AscCardType,"VISA");
	}
	else if( szlCardNo >= 5000000 && szlCardNo <= 5599999 )
	{
		PosCom.stTrans.CardType = 2;  //master card;
		strcpy((char*)PosCom.stTrans.AscCardType,"MCARD");
	}
	else if( (szlCardNo >= 3400000 && szlCardNo <= 3499999) || 
		(szlCardNo >= 3700000 && szlCardNo <= 3799999) )
	{
		PosCom.stTrans.CardType = 3;  //AMEX card
		strcpy((char*)PosCom.stTrans.AscCardType,"AMEX");
	}
	else if(szlCardNo >=6506850 && szlCardNo <= 6506859)
	{
		PosCom.stTrans.CardType = 4;  //PCLAVE card
		strcpy((char*)PosCom.stTrans.AscCardType,"PCLAVE");
	}
	else if(szlCardNo >= 6275350 && szlCardNo <=6275359)
	{
		PosCom.stTrans.CardType = 5;  //SODEXO card
		strcpy((char*)PosCom.stTrans.AscCardType,"SODEXO");
	}
	else if(szlCardNo >= 6371330 && szlCardNo <= 6371339)
	{
		PosCom.stTrans.CardType = 6;  //TODITO card
		strcpy((char*)PosCom.stTrans.AscCardType,"TODITO");
	}
	else if(szlCardNo >= 6363180 && szlCardNo <= 6363189)
	{
		PosCom.stTrans.CardType = 7;  //EDENRED card
		strcpy((char*)PosCom.stTrans.AscCardType,"EDENRED");
	}
	PrintDebug("%s%d", "PosCom.stTrans.CardType:",PosCom.stTrans.CardType,PosCom.stTrans.AscCardType);
	return OK;
}


/*******************************************
**电子钱包交易要判断卡是否匹配
**
*******************************************/
uint8_t CheckCardType(void)
{
	char    szAscCardNoTemp[7+1] = {0};
	long    szlCardNo;
	uint8_t iRet =NO_DISP;

	if( strlen((char*)PosCom.stTrans.szCardNo) == 0 )
	{
		return NO_DISP;	
	}
	memcpy(szAscCardNoTemp,PosCom.stTrans.szCardNo,7);
	szlCardNo = atol(szAscCardNoTemp);

	if(stTemp.iTransNo == PURSE_SALE)
	{
		switch(PosCom.stTrans.TransFlag)
		{
			case PURSE_SODEXO:
				if(szlCardNo >= 6275350 && szlCardNo <=6275359)
				{
					iRet = OK;
				}
				break;
			case PURSE_PUNTO:
				if(szlCardNo >=6506850 && szlCardNo <= 6506859)
				{
					iRet = OK;
				}
				break;
			case PURSE_TODITO:
				if(szlCardNo >= 6371330 && szlCardNo <= 6371339)
				{
					iRet = OK;
				}
				break;
			case PURSE_EDENRED:
				if(szlCardNo >= 6363180 && szlCardNo <= 6363189)
				{
					iRet = OK;
				}
				break;
			default:
				break;
		}
	}
	else
	{
		iRet = OK;
	}
	if(iRet !=OK)
	{
		lcdCls();
		lcdDisplay(0, 4, DISP_MEDIACY|DISP_HFONT16,"%s", "CARD IS INAVIEBLE");
		lcdFlip();
		sysBeef(3, 600);	
	}
	return iRet;
}

uint8_t DispCardNo(uint8_t *pszCardNo)
{
	uint8_t szTempCardNo[21];
	int	   small_font_size = 28,big_font_size = 30/*,iKey*/;
	char    szAscCardNoTemp[7] = {0};
	long    szlCardNo;
	
	uint8_t iRet ;
	
	small_font_size = 28;
	big_font_size = 30;
	PreDial(); 
	MaskPan(pszCardNo, szTempCardNo);

	//保存卡类型
	iRet = GetCardType();

	if (gstPosCapability.uiScreenType && strlen((char *)pszCardNo)>18)
	{
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0,  small_font_size, 0);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0,  big_font_size, 0);
	}

	PosCom.stTrans.ucSwipeFlag = PosCom.ucSwipedFlag;
	return OK;
}

uint8_t GetCardNoFromTrack(uint8_t *szCardNo, uint8_t *track2, uint8_t *track3)
{
	int   i =0;
	
	track2[37] = 0;
	track3[104] = 0;
	char * pszSeperator ;
	// from start to '＝'
	//电子钱包消费中，有一种卡二磁道只有卡号
	pszSeperator = strchr((char *)track2, '=');
	//if((stTemp.iTransNo ==PURSE_SALE ||stTemp.iTransNo ==PURSE_GETRULE||stTemp.iTransNo ==PURSE_GETBALANCE) && (pszSeperator==NULL)) 
	if(pszSeperator==NULL) 
	{
		i =  strlen((char *)track2);
		if( i!=0 )
		{	
			memcpy(szCardNo, track2, i);	
			memset(PosCom.stTrans.szExpDate,0,sizeof(PosCom.stTrans.szExpDate));
			szCardNo[i] = 0;
			return OK;	
		}
		else
		{
			return E_ERR_SWIPE;
		}
	}
	else
	{		
		// from start to '＝'
		if( strlen((char *)track2)!=0 )
		{	
			i = 0;
			while( track2[i]!='=' )
			{
				if( i>19 ) 
				{
					return E_ERR_SWIPE;
				}
				i++;
			}
			if( i<13 || i>19 )
			{
				return E_ERR_SWIPE;
			}
			memcpy(szCardNo, track2, i);	
			memset(PosCom.stTrans.szExpDate,0,sizeof(PosCom.stTrans.szExpDate));
			memcpy(PosCom.stTrans.szExpDate,track2+i+1,4);
			szCardNo[i] = 0;
			PrintDebug("%s%s", "GetCardNoFromTrack ->szCardNo:",szCardNo);

			return OK;	
		}
		// from second to '='
		else if( strlen((char *)track3)!=0 ) 
		{
			i = 0;
			while( track3[i]!='=' ) 
			{
				if( i>21 )
				{
					return E_ERR_SWIPE;
				}
				i++;
			}			    
			if( i<15 || i>21 )
			{
				return E_ERR_SWIPE;
			}
			memcpy(szCardNo, track3+2 ,i-2);		
			szCardNo[i-2] = 0;
			return OK;	
		}
	}
	
	return E_ERR_SWIPE;		

}


uint8_t GetCardHolderName(uint8_t *track1)
{
	int   i,j;
	
	track1[79] = 0;  //磁道1最大79位
	char * pszSeperator;

	// from start to '＝'
	if( strlen((char *)track1)!=0 )
	{	
			//电子钱包消费中，有一种卡二磁道只有卡号
		pszSeperator = strchr((char *)track1, '^');

		//if((stTemp.iTransNo ==PURSE_SALE) && (pszSeperator==NULL)) 
		if(pszSeperator==NULL) 
		{
			return OK;
		}
		
		i = 0;
		while( track1[i]!='^' )  //第一个标志
		{
			i++;
		}

		j = i+1;

		while( track1[j]!='^' )  //第一个标志
		{
			j++;
		}

		memcpy(PosCom.stTrans.CardHolderName, track1+i+1, j-i-1);	
		return OK;
	}
		
	return OK;
}



void DispRemoveICC(void)
{
	if( stPosParam.ucEmvSupport==PARAM_CLOSE )
	{
		return;
	}

	iccPowerDown(ICC_USERCARD);
	if( iccDetect(ICC_USERCARD)!=ICC_SUCCESS )
	{
		return;
	}

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "RETIRE TARJETA");
	lcdFlip();
	while( iccDetect(ICC_USERCARD)==ICC_SUCCESS )
	{ 
		sysBeep();
		sysDelayMs(500);
	}
}

void WaitRemoveICC(void)
{
	iccPowerDown(ICC_USERCARD);
	while( iccDetect(ICC_USERCARD)==ICC_SUCCESS )
	{ 
		sysBeep();
		sysDelayMs(500);
	}
}

void DispICCProcessing(void)
{
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PROCESANDO...");
}

void MaskPan(uint8_t *pszInPan, uint8_t *pszOutPan)
{
	uint8_t   	szBuff[30];
	int		iCnt, iPanLen;
	
	memset(szBuff, 0, sizeof(szBuff));
	iPanLen = strlen((char *)pszInPan);
	for(iCnt=0; iCnt<iPanLen; iCnt++)
	{
		if( /*(iCnt<6) || */(iCnt>=iPanLen-4 && iCnt<iPanLen) )
		{
			szBuff[iCnt] = pszInPan[iCnt];
		}
		else
		{
			szBuff[iCnt] = '*';
		}
	}
	
	sprintf((char *)pszOutPan, "%.*s", LEN_PAN, szBuff);
}
// end of file
