
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"

void PrtTranTicket(int flag)
{
  char  buf[200], buf1[200],szPrnTitle[200];
  int   iRet, j;
  uint8_t szCardTemp[20];
  uint32_t	ulICCDataLen;
  uint8_t	*psICCData, *psTemp;

  memset(buf,0,sizeof(buf));
  memset(buf1,0,sizeof(buf1));
  if(flag == REELECSIGN)
  {
  	lcdClrLine(2, 7);
  	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "REIMPRESION ELECSIGN");
  	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
    lcdFlip();
  }
  else if (OFF_ADJUST != stTemp.iTransNo)
  {
    DispTransName();
    DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
    lcdFlip();
  }
  else
  {
    lcdClrLine(2, 7);
    DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "  ADJUST SUCC  ");
    DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "  IMPRIMIENDO...  ");
    memset(buf, 0, sizeof(buf));
    memset(buf1, 0, sizeof(buf1));
    ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t*)buf);
    sprintf(buf1, "%s", buf);
    DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, buf1, buf1);
    lcdFlip();
  }
  
  for (j=0; j<3; j++)
  {
    if ( flag!=2 )
    {
      if (OFF_ADJUST != stTemp.iTransNo)
      {
        lcdClrLine(6, 7);
        DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
        lcdFlip();
      }
      else
      {
        lcdClrLine(2, 7);
        DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "  ADJUST SUCC  ");
        DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "  IMPRIMIENDO...  ");
        memset(buf, 0, sizeof(buf));
        memset(buf1, 0, sizeof(buf1));
        ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t*)buf);
        sprintf(buf1, "%s", buf);
        DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, buf1, buf1);
        lcdFlip();
      }
    }

    if ( j==1 && stPosParam.ucTicketNum=='2' )
      continue;

    if (flag == REELECSIGN && j==1)
    {
      return;
    }

    iRet = prnInit();
    if ( iRet!=OK )
    {
      lcdClrLine(2, 7);
      DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
      lcdFlip();
      kbGetKey();
      return ;
    }
    
    prnStep(2);
    PrnSmallFontData();
    prnSetFontZoom(2, 2);
    if ( flag==2)
    {
      prnPrintf("\n***FAIL TRANSACTION SLIP***\n");
    }

    if ( stPosParam.ucTestFlag==PARAM_OPEN )
    {
      prnPrintf("\n**TESTING,SLIP INVALID**\n");
    }
    prnSetFontZoom(1, 1);

    prnPrintf("-----------------------------------------------\n");
    prnSetFontZoom(2, 2);
    if (stPosParam.ucPrnTitleFlag == PARAM_OPEN)
    {
      newprnLogo(0,0,360,96,(uint8_t *)Bmp_Prn);
    }
    else
    {
      memset(szPrnTitle,0,sizeof(szPrnTitle));
      if (strlen((char*)stPosParam.szPrnTitle)>18)
      {
        prnSetFontZoom(1, 1);
        PrnBigFontChnData();
      }
      sprintf(szPrnTitle,"%s SLIP\n",stPosParam.szPrnTitle);
      prnPrintf(szPrnTitle);
      PrnSmallFontData();
    }
    prnSetFontZoom(1, 1);
    prnPrintf("-----------------------------------------------\n");
    PrnBigFontChnData();
    if ( j==0 )
    {
      prnPrintf("MERCHANT COPY\n");
    }
    else if ( j==1 )
    {
      prnPrintf("BANK COPY\n");
    }
    else if ( j==2 )
    {
      prnPrintf("CARDHOLDER COPY\n");
    }
    PrnSmallFontData();
    prnPrintf("-----------------------------------------------\n");

    PrnBigFontChnData();
    prnPrintf("MERCHANT NAME: \n");
    prnPrintf("%s\n", stPosParam.szEngName);

    prnPrintf("MERCHANT NO: \n");
    prnPrintf("%s\n", PosCom.szUnitNum);

    prnPrintf("TERMINAL NO:\n");
    prnPrintf("%s\n", PosCom.stTrans.szPosId);

    prnPrintf("OPERATOR NO:");
    prnPrintf(" %s\n", stTransCtrl.szNowTellerNo+1);

    PrnSmallFontData();
    prnPrintf("-----------------------------------------------\n");
    PrnBigFontChnData();
    
    prnPrintf("CARD NO:\n");
    memset(buf, 0, sizeof(buf));
    memset(buf1, 0, sizeof(buf1));
    if ( (((PosCom.stTrans.iTransNo==POS_PREAUTH) || (PosCom.stTrans.iTransNo==POS_PREAUTH_ADD))
    	 && (stPosParam.ucPreAuthMaskPan != PARAM_OPEN)) ||	
         (PosCom.stTrans.iTransNo==OFF_SALE) || (PosCom.stTrans.iTransNo==OFF_ADJUST) ||
         (PosCom.stTrans.iTransNo==ICC_OFFSALE) || (PosCom.stTrans.iTransNo==EC_QUICK_SALE) ||
         (PosCom.stTrans.iTransNo==EC_NORMAL_SALE) )
    {
      MakeFormatCardNo((char *)PosCom.stTrans.szCardNo, buf1);
    }
    else
    {
        if (PosCom.stTrans.iTransNo==EC_TOPUP_NORAML)
        {
          memset(szCardTemp, 0, sizeof(szCardTemp));
          MaskPan(PosCom.stTrans.szTranferCardNo, szCardTemp);
          MakeFormatCardNo((char *)szCardTemp, buf1);
        }
        else
        {
            memset(szCardTemp, 0, sizeof(szCardTemp));
            MaskPan(PosCom.stTrans.szCardNo, szCardTemp);
            MakeFormatCardNo((char *)szCardTemp, buf1);
        }
    }
    sprintf(buf, "%s %c\n", buf1, GetEntryMode());
    PrnBlackEngData();
    prnPrintf(buf);
    PrnBigFontChnData();
    prnPrintf("TXN. TYPE:\n");
    memset(buf1,0,sizeof(buf1));
    GetTransPrtName(PosCom.stTrans.iTransNo, buf1);
    PrnSmallFontData();
    prnSetFontZoom(2, 2);
    prnPrintf(buf1);
    prnSetFontZoom(1, 1);
    PrnBigFontChnData();

    if ( strlen((char *)PosCom.stTrans.szExpDate)==4 )
    {
      if ( memcmp(PosCom.stTrans.szExpDate, "0000", 4)!=0 )
      {
        memset(buf1, 0, sizeof(buf1));
        memset(buf, 0, sizeof(buf));
        if ( memcmp(PosCom.stTrans.szExpDate, "50", 2)>=0 )
        {
          memcpy((char*)buf1, "19", 2);
        }
        else
        {
          strcpy((char*)buf1, "20");
        }
        memcpy(buf1+2, PosCom.stTrans.szExpDate, 2);
        buf1[4] = '/';
        memcpy(buf1+5, PosCom.stTrans.szExpDate+2, 2);
        buf1[7] = 0;
        sprintf(buf, "EXP. DATE:   %s\n", buf1);
        prnPrintf(buf);
      }
    }
    PrnSmallFontData();
    prnPrintf("-----------------------------------------------\n");
    PrnBigFontChnData();
    prnPrintf("BATCH NO  :");
    prnPrintf("%06ld\n", PosCom.stTrans.lBatchNumber);

    prnPrintf("VOUCHER NO:");
	PrnSmallFontData();
    prnSetFontZoom(2, 2);
    prnPrintf("%06ld\n", PosCom.stTrans.lTraceNo);
	prnSetFontZoom(1, 1);
    PrnBigFontChnData();

    prnPrintf("AUTH NO   :");

   	PrnSmallFontData();
	prnSetFontZoom(2, 2);
	prnPrintf("%s\n", PosCom.stTrans.szAuthNo);
	prnSetFontZoom(1, 1);
    PrnBigFontChnData();

    prnPrintf("DATE/TIME:\n");
    prnPrintf("   %.4s/%.2s/%.2s %.2s:%.2s:%.2s\n",
              &PosCom.stTrans.szDate[0], &PosCom.stTrans.szDate[4], &PosCom.stTrans.szDate[6],
              &PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

    prnPrintf("REF. NO:");
    prnPrintf("%s\n", PosCom.stTrans.szSysReferNo);

    PrintAmount();
    PrnSmallFontData();
    if ( flag==2 )
    {
      prnPrintf("REVERSAL FAIL,PLS MANUAL PROCESS\n");
      prnPrintf("***FAIL TRANSACTION SLIP***\n\n\n\n");
      prnPrintf("\f");
      PrintData();
      return;
    }
     prnPrintf("REFERENCE\n");
    if ( PosCom.stTrans.iTransNo==POS_SALE_VOID || PosCom.stTrans.iTransNo==POS_VOID_INSTAL
         || PosCom.stTrans.iTransNo==POS_AUTH_VOID || PosCom.stTrans.iTransNo==EC_VOID_TOPUP)
    {
        prnPrintf("ORG VOUCHER:%s\n", PosCom.stTrans.szPrtOldPosSer);
    }
    else if ( PosCom.stTrans.iTransNo==POS_AUTH_CM )
    {
        prnPrintf("ORG AUTH NO:%s\n", PosCom.stTrans.szPrtOldAuthNo);
    }
    else if ( PosCom.stTrans.iTransNo==POS_REFUND)
    {
        prnPrintf("ORG REFER NO: %s\n", PosCom.stTrans.szPrtOldHostSer);
        prnPrintf("ORG DATE: %s\n", PosCom.stTrans.szOldTxnDate);
    }
    else if ( PosCom.stTrans.iTransNo == POS_PREAUTH_VOID )
    {
        prnPrintf("ORG AUTH NO:%s\n", PosCom.stTrans.szPrtOldAuthNo);
    }
    else if (PosCom.stTrans.iTransNo == EC_REFUND)
    {
    	prnPrintf("ORG DATE: %s\n", PosCom.stTrans.szOldTxnDate);
   		prnPrintf("ORG TERMINAL NO: %s\n", PosCom.stTrans.szInstalCode);
    	prnPrintf("ORG BATCH:%6d\n", PosCom.stTrans.lOldBatchNumber);
    	prnPrintf("ORG VOUCHER:%s\n", PosCom.stTrans.szPrtOldPosSer);
    }
    
    else if ( PosCom.stTrans.iTransNo==POS_AUTH_VOID)
    {
        prnPrintf("ORG AUTH NO:%s\n", PosCom.stTrans.szPrtOldAuthNo);
    }

    PrintAddtionalInfo();

	memset(szPrnTitle,0,sizeof(szPrnTitle));
    if ( strlen((char *)PosCom.stTrans.szIssuerResp)!=0 && memcmp(PosCom.stTrans.szIssuerResp, "          ", 10)!=0 )
    	strcat(szPrnTitle,(char *)PosCom.stTrans.szIssuerResp);
    if ( strlen((void *)PosCom.stTrans.szCenterResp)!=0 && memcmp(PosCom.stTrans.szCenterResp, "          ", 10)!=0 )
		strcat(szPrnTitle,(char *)PosCom.stTrans.szCenterResp);
    if ( strlen((void *)PosCom.stTrans.szRecvBankResp)!=0 && memcmp(PosCom.stTrans.szRecvBankResp, "          ", 10)!=0 )
    	strcat(szPrnTitle,(char *)PosCom.stTrans.szRecvBankResp);
	if(strlen(szPrnTitle))
		prnPrintf("%s\n", szPrnTitle);

    if (( memcmp(PosCom.stTrans.szEntryMode, "05", 2)==0 )||(PosCom.stTrans.ucQPBOCorPBOC==1))
    {
      if ( PosCom.stTrans.iTransNo==POS_SALE || PosCom.stTrans.iTransNo==ICC_OFFSALE ||
           PosCom.stTrans.iTransNo==POS_PREAUTH || PosCom.stTrans.iTransNo==POS_INSTALLMENT)
      {
        psICCData = PosCom.stTrans.sIccData;
        ulICCDataLen = PosCom.stTrans.iIccDataLen;
        memset((char*)&gstIccPrintData,0,sizeof(gstIccPrintData));
        for (psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
        {
          iRet = GetTlvValue(&psTemp, psICCData+ulICCDataLen-psTemp, SavePrintICCData, TRUE);
          if ( iRet<0 )
          {
            break;
          }
        }
        BcdToAsc0((uint8_t *)buf, gstIccPrintData.sARQC, 16);
        prnPrintf("ARQC: %s\n", buf);

        BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sTVR, 10);
        prnPrintf("TVR: %s\n", buf);
        prnPrintf("AID: %.32s\n", PosCom.stTrans.szAID);
        if ( PosCom.stTrans.bPanSeqNoOk )
        {	// TIP interoper.04-T02-SC01
          prnPrintf("PAN SN: %03d\n",PosCom.stTrans.ucPanSeqNo);
        }
        BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sTSI, 4);
        prnPrintf("TSI: %s\n", buf);
        BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sATC, 4);
        prnPrintf("ATC: %s\n", buf);
        prnPrintf("APPLAB: %.16s\n", PosCom.stTrans.szAppLable);
      }
    }

    if ( PosCom.stTrans.iTransNo== EC_QUICK_SALE || PosCom.stTrans.iTransNo== EC_NORMAL_SALE
         || PosCom.stTrans.iTransNo == QPBOC_ONLINE_SALE)
    {
      psICCData = PosCom.stTrans.sIccData;
      ulICCDataLen = PosCom.stTrans.iIccDataLen;
      memset((char*)&gstIccPrintData,0,sizeof(gstIccPrintData));
      for (psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
      {
        iRet = GetTlvValue(&psTemp, psICCData+ulICCDataLen-psTemp, SavePrintICCData, TRUE);
        if ( iRet<0 )
        {
          break;
        }
      }

      if (PosCom.stTrans.iTransNo == QPBOC_ONLINE_SALE)
      {
        BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sAppCrypto, 16);
        prnPrintf("ARQC: %s\n", buf);
      }
      else
      {
        BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sAppCrypto, 16);
        prnPrintf("TC: %s\n", buf);
        BcdToAsc0((uint8_t *)buf, gstIccPrintData.sTerminalCapab, 6);
        prnPrintf("TermCap: %s\n", buf);
      }
      BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sTVR, 10);
      prnPrintf("TVR: %s\n", buf);
      prnPrintf("AID: %.32s\n", PosCom.stTrans.szAID);
      BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sTSI, 4);
      prnPrintf("TSI: %s\n", buf);
      BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sATC, 4);
      prnPrintf("ATC: %s\n", buf);
      if ( PosCom.stTrans.bPanSeqNoOk )
      {	// TIP interoper.04-T02-SC01
        prnPrintf("PAN SN: %03d\n",PosCom.stTrans.ucPanSeqNo);
      }
      BcdToAsc0((uint8_t *)buf, gstIccPrintData.sAip, 4);
      prnPrintf("AIP: %s\n",buf);
      BcdToAsc0((uint8_t *)buf, gstIccPrintData.sTerminalRand, 8);
      prnPrintf("UNPRNUM: %s \n",buf);
      if (memcmp(gstIccPrintData.sEcIssureAuthCode,"\x00\x00\x00\x00\x00\x00",6) != 0)
      {
        memset(buf,0,sizeof(buf));
        memcpy(buf,gstIccPrintData.sEcIssureAuthCode,6);
        prnPrintf("ISSUER AUTH CODE %s\n",buf);
      }
      if (gstIccPrintData.ucIssureAppDatalen != 0)
      {
        BcdToAsc0((uint8_t *)buf, gstIccPrintData.sIssureAppData, gstIccPrintData.ucIssureAppDatalen*2);
        prnPrintf("IAD: %s\n", buf);
      }
      prnPrintf("APPLAB: %.16s\n", PosCom.stTrans.szAppLable);
      if (PosCom.stTrans.iTransNo== EC_NORMAL_SALE)
      {
        iRet = PubChar2Long(PosCom.stTrans.sAfterBalanceAmt,4,NULL);
        prnPrintf("BALANCE: %d.%02d\n", (iRet/100), (iRet%100));
      }
      else
      {
        ConvBcdAmount(PosCom.stTrans.sLoyaltAmount,(uint8_t *)buf1);
        sprintf(buf, "BALANCE: %s\n",buf1);
        prnPrintf(buf);
      }
    }

    if ( PosCom.stTrans.iTransNo==EC_TOPUP_CASH || PosCom.stTrans.iTransNo==EC_TOPUP_NORAML ||
         PosCom.stTrans.iTransNo==EC_TOPUP_SPEC || PosCom.stTrans.iTransNo==EC_VOID_TOPUP)
    {
		prnPrintf("AID: %.32s\n", PosCom.stTrans.szAID);
	    BcdToAsc0((uint8_t *)buf, PosCom.stTrans.sATC, 4);
	    prnPrintf("ATC: %s\n", buf);
	    iRet = PubChar2Long(PosCom.stTrans.sAfterBalanceAmt,4,NULL);
	    if(PosCom.stTrans.iTransNo==EC_VOID_TOPUP)
	    	prnPrintf("BALANCE: %d.%02d\n", (iRet/100), (iRet%100));
	    else
	      	prnPrintf("BALANCE: %d.%02d\n", (iRet/100), (iRet%100));
	    if (PosCom.stTrans.iTransNo==EC_TOPUP_NORAML)
	    {
	      prnPrintf("Transfer Card:\n  %s\n", PosCom.stTrans.szCardNo);
	    }
    }

    PrnBigFontChnData();
    if (flag==REPRINT || flag==REELECSIGN)
    {
        prnPrintf("****DUPLICATED****\n");
    }
    if ( stPosParam.ucTestFlag==PARAM_OPEN )
    {
      prnPrintf("***TESTING,SLIP INVALID***\n");
    }
    
    PrnSmallFontData();
    if ( j!=2 )
    {
    	prnPrintf("CARDHOLDER SIGNATURE\n");
    	if (PosCom.stTrans.ucElecSignatureflag & TS_ELECSIGN_HAVE)
	    {
	      strcpy(buf,(char *)PosCom.stTrans.szElecfilename);
	      strcpy(buf1,"/var/tmp/prttemp.png"); //生成临时文件
	      iRet = jbg2png(buf,buf1);
	      if (iRet == 0)
	      {
	        newprnLogo(0,0,0,0,(uint8_t *)buf1);
	      }
	      else
	      {
	        prnPrintf("\n\n\n");
	      }
	      fileRemove(buf1);
	    }
	    else
	    {
	      prnPrintf("\n\n\n");
	    }
	    prnPrintf("-----------------------------------------------\n");
	    prnPrintf("I ACKNOWLEDGE SATISFACTORY RECEIPT OF RELATIVE  GOODS/SERVICE\n");
    }
    
    prnPrintf("\n\n\n");
    if(PrintData() != OK)
    	break;
	
    if ( j==0 && stPosParam.ucTicketNum=='1' )
      break;
    if ( (stPosParam.ucTicketNum=='2' && j==0) || (stPosParam.ucTicketNum=='3' && j==1) )
    {
      lcdClrLine(6, 7);
      DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "[ENTER]-PRINT");
	  lcdFlip();
      kbGetKeyMs(2000);
    }
  }
  return;
}
void NetpayPrtBalance(void)
{
	  char  buf[500], buf1[200],buf2[200],szPrnTitle[200],buf3[200];
	  char tmp_printdata[100];
	  
	  int   iRet, j;
	  uint8_t szCardTemp[20];
	  uint32_t	ulICCDataLen;
	  uint8_t	*psICCData, *psTemp;
	  uint8_t printDate[30];
	  uint8_t szCardType[20] = {0};

	  memset(buf,0,sizeof(buf));
	  memset(buf1,0,sizeof(buf1));
  
  	 for (j=0; j<1; j++)
	 {
		if (OFF_ADJUST != stTemp.iTransNo)
		{
			lcdClrLine(6, 7);
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
			lcdFlip();
		}
		else
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "  ADJUST SUCC  ");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "  IMPRIMIENDO...  ");
			memset(buf, 0, sizeof(buf));
			memset(buf1, 0, sizeof(buf1));
			ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t*)buf);
			sprintf(buf1, "%s", buf);
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, buf1, buf1);
			lcdFlip();
		}
	

		iRet = prnInit();
		if ( iRet!=OK )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
			lcdFlip();
			kbGetKey();
			return ;
		}

		prnStep(2);
		PrnSmallFontData();
		prnSetFontZoom(2, 2);

		prnSetFontZoom(1, 1);

		prnSetFontZoom(2, 2);
		
		printf("stPosParam.ucPrnTitleFlag:%c\n",stPosParam.ucPrnTitleFlag);
		
		if (stPosParam.ucPrnTitleFlag == PARAM_OPEN)
		{
			newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		}
		else
		{
			memset(szPrnTitle,0,sizeof(szPrnTitle));
			if (strlen((char*)stPosParam.szPrnTitle)>18)
			{
				prnSetFontZoom(1, 1);
				PrnBigFontChnData();
			}
			sprintf(szPrnTitle,"%s SLIP\n",stPosParam.szPrnTitle);
			prnPrintf(szPrnTitle);
			PrnSmallFontData();
		}

		prnPrintf("\n");
		prnSetFontZoom(1, 1);
		PrnSmallFontData();

	//打印小票头
		memset(buf,0,sizeof(buf1));
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf1));
		memset(buf3,0,sizeof(buf1));

		sprintf(buf,"               %s\n",stPosParam.stHeader1);
		sprintf(buf1,"                %s\n",stPosParam.stAddress1);
		sprintf(buf2,"                  %s\n",stPosParam.stAddress2);
		sprintf(buf3,"                  %s\n",stPosParam.stAddress3);

		prnPrintf("%s", buf);
		prnPrintf("%s", buf1);  
		prnPrintf("%s", buf2);  
		prnPrintf("%s", buf3);  

		memset(tmp_printdata,0,sizeof(tmp_printdata));
		sprintf(tmp_printdata,"              TERMINAL %s\n",stPosParam.stTerminalId);
		prnPrintf("%s", tmp_printdata);  

		memset(tmp_printdata,0,sizeof(tmp_printdata));
		sprintf(tmp_printdata,"              AFILIACION  %s\n",stPosParam.stAffId);
		prnPrintf("%s", tmp_printdata);  
		prnPrintf("\n");
		
		MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);

		prnPrintf("%s%s           %s%.2s:%.2s:%.2s\n","FECHA: ",printDate, "HORA: ",
			&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);
		prnPrintf("\n");

		PrnBigFontChnData();
	    	memset(szCardTemp, 0, sizeof(szCardTemp));
		
	    	MaskPan(PosCom.stTrans.szCardNo, szCardTemp);
	    	MakeFormatCardNo((char *)szCardTemp, buf1);
		PrnBlackEngData();
		prnPrintf("%s\n",buf1);

		PrnSmallFontData();
		memset(buf1,0,sizeof(buf1));
		prnPrintf("SALDO DISPONIBLE:$     %s\n",PosCom.stTrans.Balance_Amount);
		prnPrintf("\n\n\n");		

		if(PrintData() != OK)
		{
			break;
		}
	}
  return;
}
void PrtPreAuthTicket(int flag)
{
	int iRet;
	//char printDate[30] = {0};
	char szCardTemp[30];
	char  buf[500], buf1[200],buf2[200],buf3[200];
	char tmp_printdata[100];	
	uint32_t	ulICCDataLen;
	uint8_t	*psICCData, *psTemp;
	  
	lcdClrLine(2, 7);
  	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
    	lcdFlip();

	iRet = prnInit();
	if ( iRet!=OK )
    	{
        	lcdClrLine(2, 7);
        	DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
        	lcdFlip();
        	kbGetKey();
        	return ;
    	}
	if (stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
      		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
    	}

	prnPrintf("\n");	
    	PrnBigFontChnData();
    	prnStep(2);


	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf1));
	memset(buf3,0,sizeof(buf1));
	PrnSmallFontData();
	
	sprintf(buf,"               %s\n",stPosParam.stHeader1);
	sprintf(buf1,"                %s\n",stPosParam.stAddress1);
	sprintf(buf2,"                  %s\n",stPosParam.stAddress2);
	sprintf(buf3,"                  %s\n",stPosParam.stAddress3);
	prnPrintf("%s", buf);
	prnPrintf("%s", buf1);  
	prnPrintf("%s", buf2); 
	prnPrintf("%s", buf3); 
	prnPrintf("\n");  


	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              TERMINAL %s\n",stPosParam.stTerminalId);
	prnPrintf("%s", tmp_printdata);  

	memset(tmp_printdata,0,sizeof(tmp_printdata));
	sprintf(tmp_printdata,"              AFILIACION  %s\n",stPosParam.stAffId);
	prnPrintf("%s", tmp_printdata);  
	prnPrintf("\n");

	prnPrintf("%s\n", "               C-L-I-E-N-T-E");

	// 卡有效期

	if ( strlen((char *)PosCom.stTrans.szExpDate)==4 )
	{
		prnPrintf("NO.TARJETA       D&1       EXP:%s\n",PosCom.stTrans.szExpDate);
	}
	else
	{
		prnPrintf("NO.TARJETA       D&1       EXP:%s\n","0000");	
	}

	prnPrintf("\n");

	PrnBigFontChnData();
    	memset(szCardTemp, 0, sizeof(szCardTemp));
	
    	MaskPan(PosCom.stTrans.szCardNo, (uint8_t*)szCardTemp);
    	MakeFormatCardNo((char *)szCardTemp, buf1);
	PrnBlackEngData();
	prnPrintf("%s\n",buf1);

	PrnBlackEngData();
	prnPrintf("     PREAUTORIZACION\n");

	PrnBigFontChnData();
	
	NetpayPrintAmount(1);

	prnPrintf("\n");

	PrnSmallFontData();

	prnPrintf("APPOBACION:%s        CARGO:%06ld\n", PosCom.stTrans.szAuthNo,PosCom.stTrans.lTraceNo);//water NO

	if(PosCom.ucSwipedFlag == CARD_INSERTED)
	{
		psICCData = PosCom.stTrans.sIccData;
		ulICCDataLen = PosCom.stTrans.iIccDataLen;
		memset((char*)&gstIccPrintData,0,sizeof(gstIccPrintData));
		for (psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
		{
			iRet = GetTlvValue(&psTemp, psICCData+ulICCDataLen-psTemp, SavePrintICCData, TRUE);
			if ( iRet<0 )
			{
				break;
			}
		}
	
		memset(buf,0,sizeof(buf));
      	 	BcdToAsc0((uint8_t *)buf, gstIccPrintData.sARQC, 16);
		prnPrintf("REF:  %s     ARQC: %s\n", PosCom.stTrans.szSysReferNo,buf);
	}
	else
	{
		prnPrintf("REF:   %s \n", PosCom.stTrans.szSysReferNo);
	}
	prnPrintf("\n");

	prnPrintf("\n                  %s\n",gstPosVersion.szVersion);

	prnPrintf("\n\n\n\n");
	PrintData();

	return ;
}

void NetpayPrtTranTicket(int flag)
{
	  char  buf[500], buf1[200],buf2[200],szPrnTitle[200],buf3[200];
	  char tmp_printdata[100];
	  
	  int   iRet, j;
	  uint8_t szCardTemp[20];
	  uint32_t	ulICCDataLen;
	  uint8_t	*psICCData, *psTemp;
	  uint8_t printDate[30];
	  //char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
	 // long    szlCardNo;
	  uint8_t szCardType[20] = {0};

	  memset(buf,0,sizeof(buf));
	  memset(buf1,0,sizeof(buf1));
	  
	  if(flag == REELECSIGN)
	  {
	  	lcdClrLine(2, 7);
	  	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "REIMPRIMIENDO");
	  	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
	    	lcdFlip();
	  }
	  else if (OFF_ADJUST != stTemp.iTransNo)
	  {
		lcdCls();
	    	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
	    	lcdFlip();
	  }
	  else
	  {
	    	lcdClrLine(2, 7);
	    	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "  ADJUST SUCC  ");
	    	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "  IMPRIMIENDO...  ");
	    	memset(buf, 0, sizeof(buf));
	    	memset(buf1, 0, sizeof(buf1));
	    	ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t*)buf);
	    	sprintf(buf1, "%s", buf);
	    	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, buf1, buf1);
	    	lcdFlip();
	  }
  
  	 for (j=0; j<3; j++)
	 {
		if ( flag!=2 )
		{
			if (OFF_ADJUST != stTemp.iTransNo)
			{
				lcdClrLine(6, 7);
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
				lcdFlip();
			}
			else
			{
				lcdClrLine(2, 7);
				DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "  ADJUST SUCC  ");
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "  IMPRIMIENDO...  ");
				memset(buf, 0, sizeof(buf));
				memset(buf1, 0, sizeof(buf1));
				ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t*)buf);
				sprintf(buf1, "%s", buf);
				DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, buf1, buf1);
				lcdFlip();
			}
		}

		if ( j==1 && stPosParam.ucTicketNum=='2' )
		{
			continue;
		}

		if (flag == REELECSIGN && j==1)
		{
			return;
		}

		iRet = prnInit();
		if ( iRet!=OK )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
			lcdFlip();
			kbGetKey();
			return ;
		}

		prnStep(2);
		PrnSmallFontData();
		prnSetFontZoom(2, 2);
		if ( flag==2)
		{
			prnPrintf("\n***FAIL TRANSACTION SLIP***\n");
		}

		if ( stPosParam.ucTestFlag==PARAM_OPEN )
		{
			prnPrintf("\n**TESTING,SLIP INVALID**\n");
		}
		prnSetFontZoom(1, 1);

		prnSetFontZoom(2, 2);
		
		printf("stPosParam.ucPrnTitleFlag:%c\n",stPosParam.ucPrnTitleFlag);
		
		if (stPosParam.ucPrnTitleFlag == PARAM_OPEN)
		{
			newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		}
		else
		{
			memset(szPrnTitle,0,sizeof(szPrnTitle));
			if (strlen((char*)stPosParam.szPrnTitle)>18)
			{
				prnSetFontZoom(1, 1);
				PrnBigFontChnData();
			}
			sprintf(szPrnTitle,"%s SLIP\n",stPosParam.szPrnTitle);
			prnPrintf(szPrnTitle);
			PrnSmallFontData();
		}

		prnPrintf("\n");
		prnSetFontZoom(1, 1);
		//PrnBigFontChnData();
		PrnSmallFontData();

	//打印小票头
		memset(buf,0,sizeof(buf1));
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf1));
		memset(buf3,0,sizeof(buf1));

		sprintf(buf,"               %s\n",stPosParam.stHeader1);
		sprintf(buf1,"                %s\n",stPosParam.stAddress1);
		sprintf(buf2,"                  %s\n",stPosParam.stAddress2);
		sprintf(buf3,"                  %s\n",stPosParam.stAddress3);

		prnPrintf("%s", buf);
		prnPrintf("%s", buf1);  
		prnPrintf("%s", buf2);  
		prnPrintf("%s", buf3);  

		memset(tmp_printdata,0,sizeof(tmp_printdata));
		sprintf(tmp_printdata,"              TERMINAL %s\n",stPosParam.stTerminalId);
		prnPrintf("%s", tmp_printdata);  

		memset(tmp_printdata,0,sizeof(tmp_printdata));
		sprintf(tmp_printdata,"              AFILIACION  %s\n",stPosParam.stAffId);
		prnPrintf("%s", tmp_printdata);  
		prnPrintf("\n");
		
		MakeFormatPrintDate(PosCom.stTrans.szDate,printDate);

		prnPrintf("%s%s           %s%.2s:%.2s:%.2s\n","FECHA: ",printDate, "HORA: ",
			&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);
		prnPrintf("\n");

		if(j==0)
		{
			prnPrintf("%s\n", "             C-O-M-E-R-C-I-O");
		}
		else
		{
			prnPrintf("%s\n", "               C-L-I-E-N-T-E");
		}

// 卡有效期

		if ( strlen((char *)PosCom.stTrans.szExpDate)==4 )
		{
			prnPrintf("NO.TARJETA       D&1       EXP:%s\n",PosCom.stTrans.szExpDate);
		}
		else
		{
			prnPrintf("NO.TARJETA       D&1       EXP:%s\n","0000");	
		}

		prnPrintf("\n");

		PrnBigFontChnData();
	    	memset(szCardTemp, 0, sizeof(szCardTemp));
		
	    	MaskPan(PosCom.stTrans.szCardNo, szCardTemp);
	    	MakeFormatCardNo((char *)szCardTemp, buf1);
		PrnBlackEngData();
		prnPrintf("%s\n",buf1);
		
	//end
	#if 1
		memset(buf1,0,sizeof(buf1));
		GetTransPrtName(PosCom.stTrans.iTransNo, buf1);  
		PrnBlackEngData();
		prnPrintf("                 %s\n",buf1);
		memset(buf1,0,sizeof(buf1));
		

		PrnSmallFontData();
		prnPrintf("                    %s\n",PosCom.stTrans.AscCardType);

		if(PosCom.ucSwipedFlag == CARD_INSERTED)
		{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"              AID %s\n",PosCom.stTrans.szAID);
			prnPrintf(buf);
		}
		if(stPosParam.stVolContrFlg ==PARAM_OPEN)
		{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"Nota #:%s",PosCom.stTrans.nato);
			prnPrintf("                   %s\n",buf);

			memset(buf,0,sizeof(buf));
			sprintf(buf,"Folio:%s",PosCom.stTrans.folio);
			prnPrintf("                %s\n",buf);

			memset(buf,0,sizeof(buf));
			sprintf(buf,"Bomba:%s",PosComconTrol.szBomba);
			prnPrintf("                   %s\n",buf);

			memset(buf,0,sizeof(buf));
			sprintf(buf,"Clave SIIC:%s",PosCom.stTrans.pemex);
			prnPrintf("              %s\n",buf);
			
			memset(buf,0,sizeof(buf));
			memset(buf1,0,sizeof(buf1));
			prnPrintf("%s \n","Producto       Cant       Precio      Monto");
			switch(PosCom.stTrans.oil_id[0])
			{
				case '1':
					sprintf(buf1,"%s","MAGNA");
					break;
				case '2':
					sprintf(buf1,"%s","PREMIUM");
					break;
				case '3':
					sprintf(buf1,"%s","DIESEL");
					break;
				default:
					sprintf(buf1,"%s","ERROR");
					break;
			}
			sprintf(buf,"%s       %s       %s      %s",buf1,PosCom.stTrans.oil_liters,PosCom.stTrans.oil_price,PosCom.stTrans.oil_amount);
			prnPrintf("%s\n",buf);

			prnPrintf("\n");
			prnPrintf("%s\n","CLAVE PEMEX:");

			if(strlen(PosCom.stTrans.plate) !=0 && strlen(PosCom.stTrans.kilometer) !=0)
			{
				memset(buf1,0,sizeof(buf1));
				sprintf(buf1,"Vehiculo: %s\n",PosCom.stTrans.plate);
				prnPrintf("%s\n",buf1);
		
				memset(buf1,0,sizeof(buf1));
				sprintf(buf1,"Kilometros: %s\n",PosCom.stTrans.kilometer);
				prnPrintf("%s\n",buf1);
			}
		}
		else
		{
			memset(buf1,0,sizeof(buf1));
			switch(PosCom.stTrans.oil_id[0])
			{
				case '1':
					sprintf(buf1,"%s","MAGNA");
					break;
				case '2':
					sprintf(buf1,"%s","PREMIUM");
					break;
				case '3':
					sprintf(buf1,"%s","DIESEL");
					break;
				default:
					sprintf(buf1,"%s","ERROR");
					break;
			}
			prnPrintf("Producto: %s\n",buf1);
			if(strlen(PosCom.stTrans.nip))
			{
				prnPrintf("Vehiculo: %s\n",PosCom.stTrans.nip);
			}
			if(strlen(PosCom.stTrans.kilometer))
			{
				prnPrintf("Kilometros: %s\n",PosCom.stTrans.kilometer);
			}
			if(strlen(PosCom.stTrans.Prt_ST))
			{
				prnPrintf("Transaccion: %s\n",PosCom.stTrans.Prt_ST);
			}
		}

		PrnBigFontChnData();
		
		NetpayPrintAmount(1);

		prnPrintf("\n");

		PrnSmallFontData();

		prnPrintf("APPOBACION:%s        CARGO:%06ld\n", PosCom.stTrans.szAuthNo,PosCom.stTrans.lTraceNo);//water NO

		if(PosCom.ucSwipedFlag == CARD_INSERTED)
		{
			psICCData = PosCom.stTrans.sIccData;
			ulICCDataLen = PosCom.stTrans.iIccDataLen;
			memset((char*)&gstIccPrintData,0,sizeof(gstIccPrintData));
			for (psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
			{
				iRet = GetTlvValue(&psTemp, psICCData+ulICCDataLen-psTemp, SavePrintICCData, TRUE);
				if ( iRet<0 )
				{
					break;
				}
			}
		
			memset(buf,0,sizeof(buf));
	      	 	BcdToAsc0((uint8_t *)buf, gstIccPrintData.sARQC, 16);
			prnPrintf("REF:  %s     ARQC: %s\n", PosCom.stTrans.szSysReferNo,buf);
		}
		else
		{
			prnPrintf("REF:   %s \n", PosCom.stTrans.szSysReferNo);
		}
		prnPrintf("\n");
		if(PosCom.stTrans.PinCheckSuccessFlag == 1)
		{
			PrnBlackEngData();
			prnPrintf("               PIN VERIFICADO \n");	
		}
		prnPrintf("\n");

		if ( j==0 && stPosParam.ucTicketNum=='2' )
		{
		    PrnSmallFontData();
		    if ( j!=2 )
		    {
		    		if (PosCom.stTrans.ucElecSignatureflag & TS_ELECSIGN_HAVE)
				{
				      strcpy(buf,(char *)PosCom.stTrans.szElecfilename);
				      strcpy(buf1,"/var/tmp/prttemp.png"); //生成临时文件
				      iRet = jbg2png(buf,buf1);
				      if (iRet == 0)
				      {
				        	newprnLogo(0,0,0,0,(uint8_t *)buf1);
				      }
				      else
				      {
				        	prnPrintf("\n\n");
				      }
				      fileRemove(buf1);
				 }
				 else
				 {
				      prnPrintf("\n\n");
				 }
			    prnPrintf("FIRMA_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _\n");
			    //prnPrintf("I ACKNOWLEDGE SATISFACTORY RECEIPT OF RELATIVE  GOODS/SERVICE\n");
			    //Reconozco satisfactoriamente la recepcion de bienes relativos  /  servicios
			    prnPrintf("RECONOZCO SATISFACTORIAMENTE LA RECEPCION DE BIENES RELATIVOS/SERVICIOS");

		    }
		}


		if(PosCom.stTrans.TransFlag !=PURSE_TODITO && PosCom.stTrans.TransFlag!=PURSE_SODEXO)
		{
			if(PosCom.stTrans.CtlFlag ==1)
			{
				prnPrintf("            TIMM CALZADA/GUILLERMO \n");
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%s\n","POR ESTE PAGARE ME OBLIGO INCONDICIONALMENTE A PAGAR A LA ORDEN DEL BANCO O LA INSTITUCION FINANCIERA QUE OTORGA EL CREDITO DEL IMPORTE DE ESTE TITULO.ESTE PAGAREPROCEDE DEL CONTRATO DE APERTURA DE CREDI-TO QUE EL BANCO O LA INSTITUCION FINANCI-ERA QUE OTORGA EL BANCO EL CREDITO Y EL TARJETAHA-BIENTE CELEBRADO.PAGARE NEGOCIABLE UNICAMENTE EN INSTITUCIONES DE CREDITO. ");

				prnPrintf("%s \n",buf);

				prnPrintf("%s \n",PosCom.stTrans.cf);

				prnPrintf("                  %s\n",gstPosVersion.szVersion);

				prnPrintf("\n\n\n\n");
			}
			else
			{
				prnPrintf("%s \n",PosCom.stTrans.Prt_SR);
			}

			if(PrintData() != OK)
			{
				break;
			}
		}
		else
		{
			prnPrintf("                    %s \n","TARJETA\n");
			prnPrintf("                   %s\n",gstPosVersion.szVersion);

			prnPrintf("\n\n\n");
			if(PrintData() != OK)
			{
				break;
			}
		}
		

		if ( (stPosParam.ucTicketNum=='2' && j==0) || (stPosParam.ucTicketNum=='3' && j==1) )
		{
			if(stTemp.iTransNo == PRT_ANY)
			{
				return;
			}
			memset(buf1,0,sizeof(buf1));
			memset(buf2,0,sizeof(buf2));
			memset(buf3,0,sizeof(buf3));
			GetTransPrtName(PosCom.stTrans.iTransNo, buf1); 
			sprintf(buf3,"%s %s",buf1,szCardType);
			sprintf(buf2,"APROBADO    %s",PosCom.stTrans.szAuthNo);
			lcdCls();
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
			lcdFlip();
			usleep(10*1000);

			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			lcdCls();
			DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, buf3);
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, buf2);
			DispMulLanguageString(0, 8, DISP_CFONT|DISP_MEDIACY, NULL, "CORTE EL RECIBO");
			DispMulLanguageString(0, 10, DISP_CFONT|DISP_MEDIACY, NULL, "ENTER PARA CONTINUAR");
			lcdFlip();
			kbGetKeyMs(5000);
			//为了测试方便，去掉两联打印
			if( kbGetKeyMs(5000)==KEY_CANCEL)
			{
				return ;
			}
			
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

		}
	#endif
	}
  return;
}


void MakeFormatCardNo(const char *card, char *fmtCard)
{
	int i;

	memset(fmtCard, ' ', 22);
	fmtCard[22] = 0;

	for(i=0; i<strlen(card); i++)
	{
		if( i%4==0 )
			memcpy(fmtCard+5*(i/4), card+i, 4);
	}
}



void MakeFormatPrintDate(uint8_t *szDate,uint8_t *PrintDate)
{
	int i = 0;
	int temp[2];
	char month[20] = {0};

	memcpy(temp,szDate+4,2);
	i = atoi((char*)temp);
	printf("i = %d\n",i);
	switch(i)
	{
		case 1:
			strcpy(month,"ENE");
			break;
		case 2:
			strcpy(month,"FEB");
			break;
		case 3:
			strcpy(month,"MAR");
			break;
		case 4:
			strcpy(month,"ABR");
			break;
		case 5:
			strcpy(month,"MAY");
			break;
		case 6:
			strcpy(month,"JUN");
			break;
		case 7:
			strcpy(month,"JUL");
			break;
		case 8:
			strcpy(month,"AGO");
			break;
		case 9:
			strcpy(month,"SEP");
			break;
		case 10:
			strcpy(month,"OCT");
			break;
		case 11:
			strcpy(month,"NOV");
			break;
		case 12:
			strcpy(month,"DIC");
			break;			
	}
	
	//sprintf((char*)PrintDate,"%s %.2s, %.2s",month, &PosCom.stTrans.szDate[6], &PosCom.stTrans.szDate[2] );
	sprintf((char*)PrintDate,"%.2s/%s/%.2s",&PosCom.stTrans.szDate[6],month,  &PosCom.stTrans.szDate[2] );
	
	//showHex((char*)"PrintDate",PrintDate,strlen((char*)PrintDate));
}



uint8_t PrtTranDetailHead(void)
{
	if( stPosParam.ucTestFlag==PARAM_OPEN )
	{
		PrnSmallFontData();
		prnSetFontZoom(2, 2);
		prnPrintf("\n**TESTING,SLIP INVALID**\n");
		prnSetFontZoom(1, 1);
	}
	PrnBigFontChnData();
	if( PosCom.stTrans.szMoneyType == PARAM_PESEO )
	{
		prnPrintf("LISTA DE TRANS. MXN\n\n");
	}
	else 
	{
		prnPrintf("LISTA DE TRANS. USD\n\n");
	}
	

	return(OK);
}

uint8_t HotelCalTotalTransNo(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	char    AscTransAmount[20];
	
	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));
	switch( pstLog->iTransNo )
	{
	case CASH_ADVACNE:
		strcpy((char*)ucTranType,"RETIRO USD");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
	
	case POS_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		AllTransTable[4].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[5].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;

	
	case PRE_TIP_SALE:
		return OK;
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);
		
		AllTransTable[1].iTotalTransNo++;        // 消费总笔数
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
		AllTransTable[4].iTotalTransNo++;//总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[4].lTotalTransAmount += atol((char*)AscTransAmount);
//		AllTransTable[7].lTotalTransAmount +=  pstLog->lpreTipAmount;
		return OK;

	case POS_PREAUTH:
		return OK;
		// 统计总金额 [0] total amount
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++;        // 消费总笔数
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
		}

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += pstLog->lPreAuthOriAmount;
		AllTransTable[6].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		return OK;


	case INTERESES_SALE:
		return OK;
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[1].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		
		
	case POS_REFUND:
		strcpy((char*)ucTranType,"Cancel");
		
		if( pstLog->iOldTransNo == CHECK_IN || pstLog->iOldTransNo == CHECK_IN)
		{
			return OK;
		}
		
		AllTransTable[1].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		AllTransTable[4].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	#if 0
	case CHECK_IN:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( pstLog->ucCheckOutId == TRUE )
			{
				return OK;
			}
				
		}
		strcpy((char*)ucTranType,"CHECK IN");
		AllTransTable[7].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);

//		AllTransTable[9].iTotalTransNo++;//总笔数
//		AllTransTable[9].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
		
	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		strcpy((char*)ucTranType,"CHECK OUT");
		AllTransTable[8].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[9].iTotalTransNo++;//总笔数
		AllTransTable[9].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
	#endif

	case NETPAY_REFUND:
		strcpy((char*)ucTranType,"DEVOLUCION");
		AllTransTable[3].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[3].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		AllTransTable[4].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case NETPAY_FORZADA:
		strcpy((char*)ucTranType,"FORZADA");
		AllTransTable[0].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		AllTransTable[4].lTotalTransAmount += atol((char*)AscTransAmount);
		//统计总消费
		AllTransTable[5].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
/*	case POS_AUTH_CM:
		ucTranType = 'P';
		break;
		
	case OFF_SALE:
	case OFF_ADJUST:		
		ucTranType = 'L';
		break;

	case POS_OFF_CONFIRM:
		ucTranType = 'C';
		break;

	case POS_INSTALLMENT:
		ucTranType = 'T';
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:

	case EC_TOPUP_CASH:
	case EC_TOPUP_NORAML:
	case EC_TOPUP_SPEC:
		ucTranType = 'Q';
		break;
*/
	default:
		strcpy((char*)ucTranType,"");
		break;
	}

	return OK;
}



uint8_t HotelSettleCalTotalTransNo(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	char    AscTransAmount[20];
	
	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));
	switch( pstLog->iTransNo )
	{

	case CASH_ADVACNE:
		strcpy((char*)ucTranType,"RETIRO USD");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
	
	case POS_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;

	case PRE_TIP_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);
		
		AllTransTable[1].iTotalTransNo++;        // 消费总笔数
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
		AllTransTable[6].iTotalTransNo++;//总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
//		AllTransTable[7].lTotalTransAmount +=  pstLog->lpreTipAmount;
		break;

	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++;        // 消费总笔数
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
		}

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += pstLog->lPreAuthOriAmount;
		AllTransTable[6].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		break;

	case INTERESES_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[1].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
		
	case POS_REFUND:
		strcpy((char*)ucTranType,"Cancel");
		
		if( pstLog->iOldTransNo == CHECK_IN )
		{
			break;
		}
		
		AllTransTable[3].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		
		AllTransTable[3].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case CHECK_IN:
		return OK;
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( pstLog->ucCheckOutId == TRUE )
			{
				return OK;
			}
				
		}
		strcpy((char*)ucTranType,"CHECK IN");
		AllTransTable[7].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);

//		AllTransTable[9].iTotalTransNo++;//总笔数
//		AllTransTable[9].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
		
	case CHECK_OUT:
		return OK;
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		strcpy((char*)ucTranType,"CHECK OUT");
		AllTransTable[8].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[9].iTotalTransNo++;//总笔数
		AllTransTable[9].lTotalTransAmount += atol((char*)AscTransAmount);
		break;

	case NETPAY_REFUND:
		strcpy((char*)ucTranType,"DEVOLUCION");
		AllTransTable[4].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[4].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case NETPAY_FORZADA:
		strcpy((char*)ucTranType,"FORZADA");
		AllTransTable[0].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
/*	case POS_AUTH_CM:
		ucTranType = 'P';
		break;
		
	case OFF_SALE:
	case OFF_ADJUST:		
		ucTranType = 'L';
		break;

	case POS_OFF_CONFIRM:
		ucTranType = 'C';
		break;

	case POS_INSTALLMENT:
		ucTranType = 'T';
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:

	case EC_TOPUP_CASH:
	case EC_TOPUP_NORAML:
	case EC_TOPUP_SPEC:
		ucTranType = 'Q';
		break;
*/
	default:
		strcpy((char*)ucTranType,"");
		break;
	}

	return OK;
}



uint8_t RestaurantCalTotalTransNo(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	char    AscTransAmount[20];
	
	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));
	switch( pstLog->iTransNo )
	{
	#if 0
	case POS_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount); 
		break;
	#endif

	case PRE_TIP_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);
		
		AllTransTable[1].iTotalTransNo++;        // 消费总笔数
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
		AllTransTable[7].iTotalTransNo++;//总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount); 
		break;

	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++;        // 消费总笔数
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
		}

		AllTransTable[7].iTotalTransNo++;//总笔数
		AllTransTable[7].lTotalTransAmount += pstLog->lPreAuthOriAmount;
		AllTransTable[7].lTotalTransAmount += pstLog->lPreAuthTipAmount;

		AllTransTable[8].lTotalTransAmount += pstLog->lPreAuthOriAmount;
		AllTransTable[8].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		break;

	case INTERESES_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[1].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
		
	case POS_REFUND:
		strcpy((char*)ucTranType,"Cancel");
		
		if( pstLog->iOldTransNo == CHECK_IN )
		{
			return OK;
		}
		
		AllTransTable[4].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		
		AllTransTable[4].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		AllTransTable[7].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case NETPAY_REFUND:
		strcpy((char*)ucTranType,"DEVOLUCION");
		AllTransTable[5].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[5].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		AllTransTable[7].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case NETPAY_FORZADA:
		strcpy((char*)ucTranType,"FORZADA");
		AllTransTable[0].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);
		//统计总消费
		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
/*	case POS_AUTH_CM:
		ucTranType = 'P';
		break;
		
	case OFF_SALE:
	case OFF_ADJUST:		
		ucTranType = 'L';
		break;

	case POS_OFF_CONFIRM:
		ucTranType = 'C';
		break;

	case POS_INSTALLMENT:
		ucTranType = 'T';
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:

	case EC_TOPUP_CASH:
	case EC_TOPUP_NORAML:
	case EC_TOPUP_SPEC:
		ucTranType = 'Q';
		break;
*/
	default:
		strcpy((char*)ucTranType,"");
		break;
	}

	return OK;
}


uint8_t RetailCalTotalTransNo(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	char    AscTransAmount[20];
	
	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));
	switch( pstLog->iTransNo )
	{

	case CASH_ADVACNE:
		strcpy((char*)ucTranType,"RETIRO USD");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
	
	case POS_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;

	case PRE_TIP_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);
		
		AllTransTable[1].iTotalTransNo++;        // 消费总笔数
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
		AllTransTable[6].iTotalTransNo++;//总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
//		AllTransTable[7].lTotalTransAmount +=  pstLog->lpreTipAmount;
		break;

	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++;        // 消费总笔数
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
		}

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += pstLog->lPreAuthOriAmount;
		AllTransTable[6].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		break;

	case INTERESES_SALE:
		strcpy((char*)ucTranType,"VENTA");
		AllTransTable[1].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
		
	case POS_REFUND:
		strcpy((char*)ucTranType,"Cancel");
		
		if( pstLog->iOldTransNo == CHECK_IN )
		{
			break;
		}
		
		AllTransTable[3].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		
		AllTransTable[3].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case CHECK_IN:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( pstLog->ucCheckOutId == TRUE )
			{
				return OK;
			}
				
		}
		strcpy((char*)ucTranType,"CHECK IN");
		AllTransTable[7].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount);

//		AllTransTable[9].iTotalTransNo++;//总笔数
//		AllTransTable[9].lTotalTransAmount += atol((char*)AscTransAmount);
		break;
		
	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		strcpy((char*)ucTranType,"CHECK OUT");
		AllTransTable[8].iTotalTransNo++;        
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[8].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[9].iTotalTransNo++;//总笔数
		AllTransTable[9].lTotalTransAmount += atol((char*)AscTransAmount);
		break;

	case NETPAY_REFUND:
		strcpy((char*)ucTranType,"DEVOLUCION");
		AllTransTable[4].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[4].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	case NETPAY_FORZADA:
		strcpy((char*)ucTranType,"FORZADA");
		AllTransTable[0].iTotalTransNo++;       
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[6].iTotalTransNo++;//总笔数
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);
		//统计总消费
		AllTransTable[7].lTotalTransAmount += atol((char*)AscTransAmount); 

		break;
	default:
		strcpy((char*)ucTranType,"");
		break;
	}

	return OK;
}


uint8_t CalTotalTransNo(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	char    AscTransAmount[20];
	
	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));
	switch( pstLog->iTransNo )
	{
	case POS_SALE:
	case PURSE_SALE:
		if(pstLog->szInputType =='1')
		{
			AllTransTable[0].iTotalTransNo++;        // 金额消费总笔数
			memset(AscTransAmount,0,sizeof(AscTransAmount));
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
			AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

			SIGNALTransTable[0].iTotalTransNo++;
			SIGNALTransTable[0].lTotalTransAmount+= atol((char*)AscTransAmount);
			break;
		}
		else if(pstLog->szInputType =='2')
		{
			if(pstLog->CardType ==7)
			{
				AllTransTable[0].iTotalTransNo++;        // 金额消费总笔数
				memset(AscTransAmount,0,sizeof(AscTransAmount));
				PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
				AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

				SIGNALTransTable[0].iTotalTransNo++;
				SIGNALTransTable[0].lTotalTransAmount+= atol((char*)AscTransAmount);
				
				AllTransTable[1].iTotalTransNo++; //容量总笔数
				memset(AscTransAmount,0,sizeof(AscTransAmount));
				PubBcd2Asc0(pstLog->sLiters,6,AscTransAmount);
				AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);
				
				SIGNALTransTable[1].iTotalTransNo++;
				SIGNALTransTable[1].lTotalTransAmount+= atol((char*)AscTransAmount);
			}
			else
			{
				AllTransTable[1].iTotalTransNo++; //容量总笔数
				memset(AscTransAmount,0,sizeof(AscTransAmount));
				PubBcd2Asc0(pstLog->sLiters,6,AscTransAmount);
				AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

				
				SIGNALTransTable[1].iTotalTransNo++;
				SIGNALTransTable[1].lTotalTransAmount+= atol((char*)AscTransAmount);
			}
			
			break;
		}
	default:
		strcpy((char*)ucTranType,"");
		break;
	}

	return OK;
}



uint8_t NetpayCheckInPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[20];
	uint8_t   szCardTemp[50];
	uint8_t szTransTime[30];
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{
	case POS_REFUND:
		sprintf((char*)ucTranType,"Cancel %s",szCardType);
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
	case CHECK_IN:
		sprintf((char*)ucTranType,"CHECK IN");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
/*	case OFF_SALE:
	case OFF_ADJUST:		
		ucTranType = 'L';
		break;
		
	case POS_AUTH_CM:
		ucTranType = 'P';
		break;

	case POS_OFF_CONFIRM:
		ucTranType = 'C';
		break;

	case POS_INSTALLMENT:
		ucTranType = 'T';
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:

	case EC_TOPUP_CASH:
	case EC_TOPUP_NORAML:
	case EC_TOPUP_SPEC:
		ucTranType = 'Q';
		break;
*/
	default:
		strcpy((char*)ucTranType,"");
		break;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH ||
		(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE))
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
	}
	
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		sprintf((char *)szAmountBuff, "%s",szBuff);
		sprintf((char*)szTransTime,"%.4s/%.2s/%.2s %.2s:%.2s:%.2s",pstLog->szDate, pstLog->szDate+4, pstLog->szDate+6,
            pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE || pstLog->iTransNo==POS_REFUND))
	{
		if( pstLog->iTransNo==INTERESES_SALE )
		{
			prnPrintf("%.19s   %d meses\n",szCardTemp,pstLog->szintereseMonth);
		}
		else
		{
			prnPrintf("%.19s\n",szCardTemp);
		}
		
		prnPrintf("%s   %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s\n",(char *)szAmountBuff);
	}	
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%.19s\n",szCardTemp);     
		prnPrintf("%s   %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%.19s\n",szCardTemp);
		prnPrintf("%s   %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE )
		{
			prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
		}
		else if (pstLog->ucVoidId == TRUE )
		{
			prnPrintf("%s Cancelacion\n",(char *)szAmountBuff);			
		}
		else 
		{
			prnPrintf("%s PENDIENTE POR MESERO\n",(char *)szAmountBuff);
		}
	}
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}



uint8_t HotelCheckOutSettlePrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	char    AscTransAmount[20];
	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	case POS_REFUND:
		sprintf((char*)ucTranType,"Cancel");
		if( pstLog->iOldTransNo != CHECK_OUT )
		{
			return OK;
		}
		
		memset(AscTransAmount,0,sizeof(AscTransAmount));

		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);	
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);
		AllTransTable[1].iTotalTransNo++;//总笔数

		AllTransTable[3].lTotalTransAmount -= atol((char*)AscTransAmount);		
		AllTransTable[3].iTotalTransNo++;//总笔数
		break;

	#if 0
	case CHECK_IN:
		sprintf((char*)ucTranType,"CHECK IN");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[2].iTotalTransNo++;//总笔数
		break;
	#endif
	case NETPAY_REFUND:
		sprintf((char*)ucTranType,"DEVOLUCION");
		if( pstLog->iOldTransNo != CHECK_OUT )
		{
			return OK;
		}
		
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[3].lTotalTransAmount -= atol((char*)AscTransAmount);
		AllTransTable[3].iTotalTransNo++;//总笔数
		break;
	
	case CHECK_OUT:
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[3].lTotalTransAmount += atol((char*)AscTransAmount);
		AllTransTable[3].iTotalTransNo++;//总笔数
		break;
	

	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}

	return OK;
}


uint8_t HotelCheckOutNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[50];
	uint8_t   szCardTemp[50];
	uint8_t   AdjustedAmount[50] = {0};
	uint8_t szTransTime[30];
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	case POS_REFUND:
		sprintf((char*)ucTranType,"Cancel");
		if( pstLog->iOldTransNo != CHECK_OUT )
		{
			return OK;
		}
		
		memset(AscTransAmount,0,sizeof(AscTransAmount));

		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);	
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);
		AllTransTable[1].iTotalTransNo++;//总笔数

		AllTransTable[3].iTotalTransNo++;//总笔数
		break;

	#if 0
	case CHECK_IN:
		sprintf((char*)ucTranType,"CHECK IN");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[2].iTotalTransNo++;//总笔数
		break;
	#endif
	case NETPAY_REFUND:
		sprintf((char*)ucTranType,"DEVOLUCION");
		if( pstLog->iOldTransNo != CHECK_OUT )
		{
			return OK;
		}
		
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[3].iTotalTransNo++;//总笔数
		break;
	
	case CHECK_OUT:
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[3].iTotalTransNo++;//总笔数
		break;
	

	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH ||
		(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE))
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
	}
	
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		memset(szAmountBuff,0,sizeof(szAmountBuff));
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			sprintf((char *)szAmountBuff, "%s MXN PRM %s",szBuff,pstLog->szSysReferNo);
		}
		else
		{
			sprintf((char *)szAmountBuff, "%s USD PRM %s",szBuff,pstLog->szSysReferNo);
		}
		
		sprintf((char*)szTransTime,"%.4s/%.2s/%.2s %.2s:%.2s:%.2s",pstLog->szDate, pstLog->szDate+4, pstLog->szDate+6,
            pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE || pstLog->iTransNo==POS_REFUND || 
		pstLog->iTransNo==CASH_ADVACNE || pstLog->iTransNo==NETPAY_REFUND ||
		pstLog->iTransNo==NETPAY_FORZADA ) )
	{
		if( pstLog->iTransNo==INTERESES_SALE )
		{
			prnPrintf("%.19s   %d meses\n",szCardTemp,pstLog->szintereseMonth);
		}
		else
		{
			prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		}
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s\n",(char *)szAmountBuff);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}

	}	
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);     
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sNetpayAdjustedAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN ",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD ",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE )
		{
			prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
		}
		else if (pstLog->ucVoidId == TRUE )
		{
			prnPrintf("%s Cancelacion\n",(char *)szAmountBuff);			
		}
		else 
		{
			prnPrintf("%s PENDIENTE POR MESERO\n",(char *)szAmountBuff);
		}
	}
	else if(gstPosCapability.uiPrnType == 'T' && 
		(pstLog->iTransNo==CHECK_IN || pstLog->iTransNo==CHECK_OUT ))
	{	
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
			
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->iTransNo==CHECK_IN )
		{
			if(pstLog->ucCheckOutId == TRUE )
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else 
				{
					prnPrintf("%s   CERRADO \n",(char *)szAmountBuff);
				}
				
			}
			else 
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else
				{
					prnPrintf("%s   PENDIENTE\n",(char *)szAmountBuff);
				}
			}
		}
		else
		{
			if( pstLog->ucVoidId == TRUE )
			{
				prnPrintf("%s  Cancelacion %s\n",(char *)szAmountBuff,szCardType);	
			}
			else 
			{
				prnPrintf("%s\n",(char *)szAmountBuff);
			}
			
		}
		
	}	
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}



uint8_t HotelCheckInSettlePrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100]/*,szAmountBuff[50]*/;
	uint8_t   szCardTemp[50];
//	uint8_t   AdjustedAmount[50] = {0};
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	case POS_REFUND:
		sprintf((char*)ucTranType,"Cancel");
		if( pstLog->iOldTransNo != CHECK_IN )
		{
			break;
		}
		
		memset(AscTransAmount,0,sizeof(AscTransAmount));

		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);	
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);
		AllTransTable[1].iTotalTransNo++;//总笔数

		AllTransTable[2].iTotalTransNo++;//总笔数
		AllTransTable[2].lTotalTransAmount -= atol((char*)AscTransAmount);
		break;

	
	case CHECK_IN:
		sprintf((char*)ucTranType,"CHECK IN");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);
		AllTransTable[2].iTotalTransNo++;//总笔数
		break;
	#if 0
	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[6].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	#endif

	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}
	
	

	return OK;
}



uint8_t HotelCheckInNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[50];
	uint8_t   szCardTemp[50];
	uint8_t   AdjustedAmount[50] = {0};
	uint8_t szTransTime[30];
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	case POS_REFUND:
		sprintf((char*)ucTranType,"Cancel");
		if( pstLog->iOldTransNo != CHECK_IN )
		{
			break;
		}
		
		memset(AscTransAmount,0,sizeof(AscTransAmount));

		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);	
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);
		AllTransTable[1].iTotalTransNo++;//总笔数

		AllTransTable[2].iTotalTransNo++;//总笔数
		break;

	
	case CHECK_IN:
		sprintf((char*)ucTranType,"CHECK IN");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[2].iTotalTransNo++;//总笔数
		break;
	#if 0
	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[6].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	#endif

	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH ||
		(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE))
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
	}
	
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		memset(szAmountBuff,0,sizeof(szAmountBuff));
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			sprintf((char *)szAmountBuff, "%s MXN PRM %s",szBuff,pstLog->szSysReferNo);
		}
		else
		{
			sprintf((char *)szAmountBuff, "%s USD PRM %s",szBuff,pstLog->szSysReferNo);
		}
		
		sprintf((char*)szTransTime,"%.4s/%.2s/%.2s %.2s:%.2s:%.2s",pstLog->szDate, pstLog->szDate+4, pstLog->szDate+6,
            pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE || pstLog->iTransNo==POS_REFUND || 
		pstLog->iTransNo==CASH_ADVACNE || pstLog->iTransNo==NETPAY_REFUND ||
		pstLog->iTransNo==NETPAY_FORZADA ) )
	{
		if( pstLog->iTransNo==INTERESES_SALE )
		{
			prnPrintf("%.19s   %d meses\n",szCardTemp,pstLog->szintereseMonth);
		}
		else
		{
			prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		}
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s\n",(char *)szAmountBuff);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}

	}	
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);     
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sNetpayAdjustedAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN ",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD ",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE )
		{
			prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
		}
		else if (pstLog->ucVoidId == TRUE )
		{
			prnPrintf("%s Cancelacion\n",(char *)szAmountBuff);			
		}
		else 
		{
			prnPrintf("%s PENDIENTE POR MESERO\n",(char *)szAmountBuff);
		}
	}
	else if(gstPosCapability.uiPrnType == 'T' && 
		(pstLog->iTransNo==CHECK_IN || pstLog->iTransNo==CHECK_OUT ))
	{	
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
			
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->iTransNo==CHECK_IN )
		{
			if(pstLog->ucCheckOutId == TRUE )
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else 
				{
					prnPrintf("%s   CERRADO \n",(char *)szAmountBuff);
				}
				
			}
			else 
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else
				{
					prnPrintf("%s   PENDIENTE\n",(char *)szAmountBuff);
				}
			}
		}
		else
		{
			if( pstLog->ucVoidId == TRUE )
			{
				prnPrintf("%s  Cancelacion %s\n",(char *)szAmountBuff,szCardType);	
			}
			else 
			{
				prnPrintf("%s\n",(char *)szAmountBuff);
			}
			
		}
		
	}	
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}


uint8_t HotelNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[50];
	uint8_t   szCardTemp[50];
	uint8_t   AdjustedAmount[50] = {0};
	uint8_t szTransTime[30];
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	case POS_SALE:
		sprintf((char*)ucTranType,"VENTA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			PubBcd2Asc0(pstLog->sNetpayAdjustedAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;
		
	case PRE_TIP_SALE:
		sprintf((char*)ucTranType,"VENTA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		memset(AscTransAmount,0,sizeof(AscTransAmount));
		AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
//		AllTransTable[0].lTotalTransAmount += pstLog->lpreTipAmount;

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;
	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		sprintf((char*)ucTranType,"VENTA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
//			AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		}

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;
	case INTERESES_SALE:
		sprintf((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;
		
	case POS_REFUND:
		
		sprintf((char*)ucTranType,"Cancel");

		if( pstLog->iOldTransNo == CHECK_IN )
		{
			break;
		}
		
		AllTransTable[1].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		AllTransTable[1].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;

	case NETPAY_REFUND:
		sprintf((char*)ucTranType,"DEVOLUCION");
		if( pstLog->iOldTransNo == CHECK_IN )
		{
			break;
		}
		
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;

	case NETPAY_FORZADA:	
		sprintf((char*)ucTranType,"VENTA FORZADA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;

	#if 0
	case CHECK_IN:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( pstLog->ucCheckOutId == TRUE )
			{
				return OK;
			}
				
		}
		sprintf((char*)ucTranType,"CHECK IN");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[5].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[5].lTotalTransAmount += atol((char*)AscTransAmount);

//		AllTransTable[7].iTotalTransNo++;//总笔数
		break;

	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[6].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	#endif

	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH ||
		(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE))
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
	}
	
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		memset(szAmountBuff,0,sizeof(szAmountBuff));
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			sprintf((char *)szAmountBuff, "%s MXN PRM %s",szBuff,pstLog->szSysReferNo);
		}
		else
		{
			sprintf((char *)szAmountBuff, "%s USD PRM %s",szBuff,pstLog->szSysReferNo);
		}
		
		sprintf((char*)szTransTime,"%.4s/%.2s/%.2s %.2s:%.2s:%.2s",pstLog->szDate, pstLog->szDate+4, pstLog->szDate+6,
            pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE || pstLog->iTransNo==POS_REFUND || 
		pstLog->iTransNo==CASH_ADVACNE || pstLog->iTransNo==NETPAY_REFUND ||
		pstLog->iTransNo==NETPAY_FORZADA) )
	{
		if( pstLog->iTransNo==INTERESES_SALE )
		{
			prnPrintf("%.19s   %d meses\n",szCardTemp,pstLog->szintereseMonth);
		}
		else
		{
			prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		}
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s\n",(char *)szAmountBuff);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}

	}	
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);     
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sNetpayAdjustedAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN ",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD ",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE )
		{
			prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
		}
		else if (pstLog->ucVoidId == TRUE )
		{
			prnPrintf("%s Cancelacion\n",(char *)szAmountBuff);			
		}
		else 
		{
			prnPrintf("%s PENDIENTE POR MESERO\n",(char *)szAmountBuff);
		}
	}
	else if(gstPosCapability.uiPrnType == 'T' && 
		(pstLog->iTransNo==CHECK_IN || pstLog->iTransNo==CHECK_OUT ))
	{
		//结算完成之后CHECK IN FINISH AND CHECK OUT DONT PRINT
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( ( (pstLog->iTransNo==CHECK_IN) && (pstLog->ucCheckOutId == TRUE) ) ||
				pstLog->iTransNo==CHECK_OUT )
			{
				return OK;
			}
				
		}
		
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
			
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->iTransNo==CHECK_IN )
		{
			if(pstLog->ucCheckOutId == TRUE )
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else 
				{
					prnPrintf("%s   CERRADO \n",(char *)szAmountBuff);
				}
				
			}
			else 
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else
				{
					prnPrintf("%s   PENDIENTE\n",(char *)szAmountBuff);
				}
			}
		}
		else
		{
			if( pstLog->ucVoidId == TRUE )
			{
				prnPrintf("%s  Cancelacion %s\n",(char *)szAmountBuff,szCardType);	
			}
			else 
			{
				prnPrintf("%s\n",(char *)szAmountBuff);
			}
			
		}
		
	}	
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}

uint8_t RestaurantNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[50];
	uint8_t   szCardTemp[50];
	uint8_t   AdjustedAmount[50] = {0};
	uint8_t szTransTime[30];
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	#if 0
	case POS_SALE:
		sprintf((char*)ucTranType,"VENTA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			PubBcd2Asc0(pstLog->sNetpayAdjustedAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;
	#endif
		
	case PRE_TIP_SALE:
		sprintf((char*)ucTranType,"VENTA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		memset(AscTransAmount,0,sizeof(AscTransAmount));
		AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
//		AllTransTable[0].lTotalTransAmount += pstLog->lpreTipAmount;

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		sprintf((char*)ucTranType,"VENTA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
//			AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		}

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
	case INTERESES_SALE:
		sprintf((char*)ucTranType,"VENTA");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
		
	case POS_REFUND:
		sprintf((char*)ucTranType,"Cancel");

		if( pstLog->iOldTransNo == CHECK_IN )
		{
			return OK;
		}
		
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;

	case NETPAY_REFUND:
		sprintf((char*)ucTranType,"DEVOLUCION");
		if( pstLog->iOldTransNo == CHECK_IN )
		{
			return OK;
		}
		
		AllTransTable[3].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		AllTransTable[3].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;

	case NETPAY_FORZADA:	
		sprintf((char*)ucTranType,"VENTA FORZADA ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;

	#if 0
	case CHECK_IN:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( pstLog->ucCheckOutId == TRUE )
			{
				return OK;
			}
				
		}
		sprintf((char*)ucTranType,"CHECK IN");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[5].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[5].lTotalTransAmount += atol((char*)AscTransAmount);

//		AllTransTable[7].iTotalTransNo++;//总笔数
		break;

	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[6].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	#endif

	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH ||
		(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE))
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memcpy(pstLog->sAmount,pstLog->sNetpayAdjustedAmount,sizeof(pstLog->sNetpayAdjustedAmount));
		}
		else
		{
			memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
		}
	}
	
	
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		memset(szAmountBuff,0,sizeof(szAmountBuff));
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			if( pstLog->iTransNo==PRE_TIP_SALE || pstLog->iTransNo==POS_PREAUTH )
			{
				sprintf((char *)szAmountBuff, "%s MXN",szBuff);
			}
			else
			{
				sprintf((char *)szAmountBuff, "%s MXN PRM %s",szBuff,pstLog->szSysReferNo);
			}
			
		}
		else
		{
			if( pstLog->iTransNo==PRE_TIP_SALE || pstLog->iTransNo==POS_PREAUTH )
			{
				sprintf((char *)szAmountBuff, "%s USD",szBuff);
			}
			else
			{
				sprintf((char *)szAmountBuff, "%s USD PRM %s",szBuff,pstLog->szSysReferNo);
			}
		}
		
		sprintf((char*)szTransTime,"%.4s/%.2s/%.2s %.2s:%.2s:%.2s",pstLog->szDate, pstLog->szDate+4, pstLog->szDate+6,
            pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE || pstLog->iTransNo==POS_REFUND || 
		pstLog->iTransNo==CASH_ADVACNE || pstLog->iTransNo==NETPAY_REFUND ||
		pstLog->iTransNo==NETPAY_FORZADA) )
	{
		if( pstLog->iTransNo==INTERESES_SALE )
		{
			prnPrintf("%.19s   %d meses\n",szCardTemp,pstLog->szintereseMonth);
		}
		else
		{
			prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		}
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s\n",(char *)szAmountBuff);
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->sAmount, szBuff);
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN",szBuff);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD",szBuff);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}

	}	
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);     
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			prnPrintf("%s    PROP. %ld.%02ld MXN\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
			prnPrintf("PRM: %s\n",pstLog->szSysReferNo);
		}
		else
		{
			prnPrintf("%s    PROP. %ld.%02ld USD\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
			prnPrintf("PRM: %s\n",pstLog->szSysReferNo);
		}
		
		if( pstLog->ucNetpayAdjusted==TRUE )
		{
			memset(szBuff,0,sizeof(szBuff));
			ConvBcdAmount(pstLog->preTipOriAmount, szBuff);
			
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				sprintf((char *)AdjustedAmount, "%s MXN  PROP. %ld.%02ld MXN",szBuff,
					pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
			}
			else
			{
				sprintf((char *)AdjustedAmount, "%s USD  PROP. %ld.%02ld USD",szBuff,
					pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
			}
			prnPrintf("AJUSTE %s\n",(char *)AdjustedAmount);
		}
		
		
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE )
		{
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				prnPrintf("%s    PROP. %ld.%02ld MXN\n",(char *)szAmountBuff,
					pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
				prnPrintf("PRM: %s\n",pstLog->szSysReferNo);
			}
			else
			{
				prnPrintf("%s    PROP. %ld.%02ld USD\n",(char *)szAmountBuff,
					pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
				prnPrintf("PRM: %s\n",pstLog->szSysReferNo);
			}
			
		}
		else if (pstLog->ucVoidId == TRUE )
		{
			prnPrintf("%s Cancelacion\n",(char *)szAmountBuff);			
		}
		else 
		{
			prnPrintf("%s PENDIENTE POR MESERO\n",(char *)szAmountBuff);
		}
	}
	else if(gstPosCapability.uiPrnType == 'T' && 
		(pstLog->iTransNo==CHECK_IN || pstLog->iTransNo==CHECK_OUT ))
	{
		//结算完成之后CHECK IN FINISH AND CHECK OUT DONT PRINT
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( ( (pstLog->iTransNo==CHECK_IN) && (pstLog->ucCheckOutId == TRUE) ) ||
				pstLog->iTransNo==CHECK_OUT )
			{
				return OK;
			}
				
		}
		
		prnPrintf("%s %.19s\n",szCardType,szCardTemp);
			
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->iTransNo==CHECK_IN )
		{
			if(pstLog->ucCheckOutId == TRUE )
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else 
				{
					prnPrintf("%s   CERRADO \n",(char *)szAmountBuff);
				}
				
			}
			else 
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else
				{
					prnPrintf("%s   PENDIENTE\n",(char *)szAmountBuff);
				}
			}
		}
		else
		{
			if( pstLog->ucVoidId == TRUE )
			{
				prnPrintf("%s  Cancelacion %s\n",(char *)szAmountBuff,szCardType);	
			}
			else 
			{
				prnPrintf("%s\n",(char *)szAmountBuff);
			}
			
		}
		
	}	
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}


uint8_t GasstationPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog,int flag)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];
	uint8_t szTransTime[30];
	char    AscTransAmount[20];
	char    AscTransLiters[20];

	long  szamount =0;
	long szLiters = 0;
	int szret =0;
	
	int		iCnt, iPanLen;
	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	//szret = GetCardType();

	switch( pstLog->iTransNo )
	{	
	case POS_SALE:
	case PURSE_SALE:
		if(pstLog->CardType ==7)
		{
			sprintf((char*)ucTranType,"VENTA ");
			memset(AscTransAmount,0,sizeof(AscTransAmount));
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
			szamount = atol((char*)AscTransAmount);

			if(pstLog->szInputType =='1')
			{
				AllTransTable[0].iTotalTransNo++;        // 金额消费总笔数
				AllTransTable[0].lTotalTransAmount += szamount;
			}
			else 
			{
				memset(AscTransLiters,0,sizeof(AscTransLiters));
				PubBcd2Asc0(pstLog->sLiters,6,AscTransLiters);
				szLiters= atol((char*)AscTransLiters);
				
				AllTransTable[0].iTotalTransNo++;        // 金额消费总笔数
				AllTransTable[0].lTotalTransAmount += szamount;
				AllTransTable[1].iTotalTransNo++;        //容量 消费总笔数
				AllTransTable[1].lTotalTransAmount += szLiters;
			}
		}
		else
		{
			sprintf((char*)ucTranType,"VENTA ");
			memset(AscTransAmount,0,sizeof(AscTransAmount));
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
			szamount = atol((char*)AscTransAmount);
			
			if(pstLog->szInputType =='1')
			{
				AllTransTable[0].iTotalTransNo++;        // 金额消费总笔数
				AllTransTable[0].lTotalTransAmount += szamount;
			}
			else 
			{
				AllTransTable[1].iTotalTransNo++;        //容量 消费总笔数
				AllTransTable[1].lTotalTransAmount += szamount;
			}
		}
		break;
	default:
		strcpy((char*)ucTranType,"");
		return OK;
	}

	if(flag ==1)
	{
		MaskPan(pstLog->szCardNo, szCardTemp);

		memset(szBuf,0,sizeof(szBuf));
		printf("PosCom.stTrans.AscCardType:%s\n",pstLog->AscCardType);
		
		sprintf(szBuf,"%.2s  %s\n",pstLog->AscCardType,szCardTemp);
		prnPrintf("%s ",szBuf);

		memset(szBuf,0,sizeof(szBuf));
		MakeFormatPrintDate(pstLog->szDate,(uint8_t *)szTransTime);
		sprintf((char *)szBuf, "%s       %06d  %s   %.2s%.2s%.2s\n",  ucTranType,pstLog->lTraceNo,szTransTime,
			&pstLog->szTime[0], &pstLog->szTime[2], &pstLog->szTime[4]);
		prnPrintf("%s ",szBuf);

		memset(szBuf,0,sizeof(szBuf));
		sprintf(szBuf,"%d.%02d  %s  %s\n",szamount/100,szamount%100,"APROBACION",pstLog->szAuthNo);		
		prnPrintf("%s ",szBuf);
	}
	else
	{
		//MaskPan(pstLog->szCardNo, szCardTemp);
		
		memset(szBuff, 0, sizeof(szBuff));
		iPanLen = strlen((char *)pstLog->szCardNo);
		for(iCnt=0; iCnt<iPanLen; iCnt++)
		{
			if( (iCnt<6) ||(iCnt>=iPanLen-4 && iCnt<iPanLen) )
			{
				szBuff[iCnt] = pstLog->szCardNo[iCnt];
			}
			else
			{
				szBuff[iCnt] = '*';
			}
		}
		sprintf((char *)szCardTemp, "%.*s", LEN_PAN, szBuff);

		memset(szBuf,0,sizeof(szBuf));
		PrintDebug("%s %s", "PosCom.stTrans.AscCardType:",pstLog->AscCardType);
		
		sprintf(szBuf,"%s  %s\n",szCardTemp,"Registrada");
		prnPrintf("%s ",szBuf);

		memset(szBuf,0,sizeof(szBuf));
		MakeFormatPrintDate(pstLog->szDate,(uint8_t *)szTransTime);
		sprintf((char *)szBuf, "%s   %s  %d.%02d\n", szTransTime,pstLog->szAuthNo,szamount/100,szamount%100);
		prnPrintf("%s ",szBuf);
	}
	return OK;
}


uint8_t NetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[20];
	uint8_t   szCardTemp[50];
	uint8_t szTransTime[30];
	char    AscTransAmount[20];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    	long    szlCardNo;
    	uint8_t szCardType[20] = {0};
	

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{
	case CASH_ADVACNE:	
		sprintf((char*)ucTranType,"RETIRO USD ");
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[4].iTotalTransNo++;//总笔数
		break;
		
	case POS_SALE:
		
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
		
	case PRE_TIP_SALE:
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		memset(AscTransAmount,0,sizeof(AscTransAmount));
		AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;
		//小费也要统计到总金额
//		AllTransTable[0].lTotalTransAmount += pstLog->lpreTipAmount;

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
//			AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		}

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	case INTERESES_SALE:
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
		
	case POS_REFUND:
		
		sprintf((char*)ucTranType,"Cancel %s",szCardType);

		if( pstLog->iOldTransNo == CHECK_IN )
		{
			break;
		}
		
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
	case CHECK_IN:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( pstLog->ucCheckOutId == TRUE )
			{
				return OK;
			}
				
		}
		sprintf((char*)ucTranType,"CHECK IN");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[5].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[5].lTotalTransAmount += atol((char*)AscTransAmount);

//		AllTransTable[7].iTotalTransNo++;//总笔数
		break;

	case CHECK_OUT:
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			return OK;	
		}
		sprintf((char*)ucTranType,"CHECK OUT");
//		sprintf((char*)ucTranType,"CHECK IN %s",szCardType);
		AllTransTable[6].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[6].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[7].iTotalTransNo++;//总笔数
		break;
/*	case OFF_SALE:
	case OFF_ADJUST:		
		ucTranType = 'L';
		break;
		
	case POS_AUTH_CM:
		ucTranType = 'P';
		break;

	case POS_OFF_CONFIRM:
		ucTranType = 'C';
		break;

	case POS_INSTALLMENT:
		ucTranType = 'T';
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:

	case EC_TOPUP_CASH:
	case EC_TOPUP_NORAML:
	case EC_TOPUP_SPEC:
		ucTranType = 'Q';
		break;
*/
	default:
		strcpy((char*)ucTranType,"");
		break;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH ||
		(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE))
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
	}
	
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		sprintf((char *)szAmountBuff, "%s",szBuff);
		sprintf((char*)szTransTime,"%.4s/%.2s/%.2s %.2s:%.2s:%.2s",pstLog->szDate, pstLog->szDate+4, pstLog->szDate+6,
            pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE || pstLog->iTransNo==POS_REFUND || 
		pstLog->iTransNo==CASH_ADVACNE) )
	{
		if( pstLog->iTransNo==INTERESES_SALE )
		{
			prnPrintf("%.19s   %d meses\n",szCardTemp,pstLog->szintereseMonth);
		}
		else
		{
			prnPrintf("%.19s\n",szCardTemp);
		}
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s\n",(char *)szAmountBuff);

	}	
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%.19s\n",szCardTemp);     
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("%.19s\n",szCardTemp);
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE )
		{
			prnPrintf("%s    PROP. %ld.%02ld\n",(char *)szAmountBuff,
				pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
		}
		else if (pstLog->ucVoidId == TRUE )
		{
			prnPrintf("%s Cancelacion\n",(char *)szAmountBuff);			
		}
		else 
		{
			prnPrintf("%s PENDIENTE POR MESERO\n",(char *)szAmountBuff);
		}
	}
	else if(gstPosCapability.uiPrnType == 'T' && 
		(pstLog->iTransNo==CHECK_IN || pstLog->iTransNo==CHECK_OUT ))
	{
		//结算完成之后CHECK IN FINISH AND CHECK OUT DONT PRINT
		if( pstLog->lBatchNumber != stPosParam.lNowBatchNum )
		{
			if( ( (pstLog->iTransNo==CHECK_IN) && (pstLog->ucCheckOutId == TRUE) ) ||
				pstLog->iTransNo==CHECK_OUT )
			{
				return OK;
			}
				
		}
		
		prnPrintf("%.19s\n",szCardTemp);
			
		prnPrintf("%s %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->iTransNo==CHECK_IN )
		{
			if(pstLog->ucCheckOutId == TRUE )
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else 
				{
					prnPrintf("%s   CERRADO \n",(char *)szAmountBuff);
				}
				
			}
			else 
			{
				if( pstLog->ucVoidId == TRUE )
				{
					prnPrintf("%s   Cancelacion %s\n",(char *)szAmountBuff,szCardType);
				}
				else
				{
					prnPrintf("%s   PENDIENTE\n",(char *)szAmountBuff);
				}
			}
		}
		else
		{
			if( pstLog->ucVoidId == TRUE )
			{
				prnPrintf("%s  Cancelacion %s\n",(char *)szAmountBuff,szCardType);	
			}
			else 
			{
				prnPrintf("%s\n",(char *)szAmountBuff);
			}
			
		}
		
	}	
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}



uint8_t NetpayPrtOneTranTip(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType[60], szBuff[100], szBuf[100],szAmountBuff[20];
	uint8_t   szCardTemp[50];
	uint8_t szTransTime[30];
	char    AscTransAmount[20];
	uint8_t printDate[30];

	char    szAscCardNoTemp[7] = {0};  // 卡号前6位，用于判断卡类型 
    long    szlCardNo;
    uint8_t szCardType[20] = {0};

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));

	memcpy(szAscCardNoTemp,pstLog->szCardNo,6);
	szlCardNo = atol(szAscCardNoTemp);	
	if( szlCardNo > 400000 && szlCardNo < 499999 )
	{
		strcpy((char*)szCardType,"VISA");
	}
	else if( szlCardNo > 510000 && szlCardNo < 559999 )
	{
		strcpy((char*)szCardType,"MCARD");
	}
	else if( (szlCardNo > 340000 && szlCardNo < 349999) || 
		(szlCardNo > 370000 && szlCardNo < 379999) )
	{
		strcpy((char*)szCardType,"AMEX");     //美国运通卡
		
	}
	
	switch( pstLog->iTransNo )
	{	
	#if 0
	case POS_SALE:
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
	#endif
		
	case PRE_TIP_SALE:
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->preTipOriAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		memset(AscTransAmount,0,sizeof(AscTransAmount));
		AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
		AllTransTable[1].lTotalTransAmount += pstLog->lpreTipAmount;

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
	case POS_PREAUTH:
		// 统计总金额 [0] total amount
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthOriAmount;
	
		// 统计小费金额 [1] tip total amount
		if(pstLog->ucAuthCmlId == TRUE && pstLog->lPreAuthTipAmount != 0)
		{
			AllTransTable[1].iTotalTransNo++; // 含小费笔数统计
			AllTransTable[1].lTotalTransAmount += pstLog->lPreAuthTipAmount;
			//小费也要统计到总金额
			AllTransTable[0].lTotalTransAmount += pstLog->lPreAuthTipAmount;
		}

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
		
	case INTERESES_SALE:
		sprintf((char*)ucTranType,"VENTA %s",szCardType);
		AllTransTable[0].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		AllTransTable[0].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;
		
	case POS_REFUND:
		sprintf((char*)ucTranType,"CANCELACION %s",szCardType);
		AllTransTable[2].iTotalTransNo++;        // 消费总笔数
		memset(AscTransAmount,0,sizeof(AscTransAmount));
		if(pstLog->iOldTransNo == POS_PREAUTH && 
		pstLog->ucAuthCmlId != TRUE)
		{
			PubBcd2Asc0(pstLog->preAuthOriAmount,6,AscTransAmount);
		}
		else
		{
			PubBcd2Asc0(pstLog->sAmount,6,AscTransAmount);
		}
		
		AllTransTable[2].lTotalTransAmount += atol((char*)AscTransAmount);

		AllTransTable[5].iTotalTransNo++;//总笔数
		break;

	default:
		break;
	}
	
	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	else if( pstLog->iTransNo==POS_PREAUTH )
	{
		memcpy(pstLog->sAmount,pstLog->preAuthOriAmount,sizeof(pstLog->preAuthOriAmount));
	}
	else if( pstLog->iTransNo==PRE_TIP_SALE )
	{
		memcpy(pstLog->sAmount,pstLog->preTipOriAmount,sizeof(pstLog->preTipOriAmount));
	}
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( /*(pstLog->iTransNo==POS_PREAUTH) ||*/ (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, (int)ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%s  %06d",  ucTranType,pstLog->lTraceNo);		
		sprintf((char *)szAmountBuff, "%s",szBuff);

		MakeFormatPrintDate(pstLog->szDate,printDate);
		sprintf((char*)szTransTime,"%s ",printDate/*, 
			pstLog->szTime, pstLog->szTime+2, pstLog->szTime+4*/);
	}
	prnPrintf("---------------------------------------------\n");
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T' && (pstLog->iTransNo==POS_SALE || 
		pstLog->iTransNo==INTERESES_SALE ))
	{
		prnPrintf("%.19s\n",szCardTemp);
		prnPrintf("%s  %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			prnPrintf("%s MXN\n",(char *)szAmountBuff);
		}
		else
		{
			prnPrintf("%s USD\n",(char *)szAmountBuff);
		}
		
	}
	else if(gstPosCapability.uiPrnType == 'T' &&  pstLog->iTransNo==POS_REFUND )
	{
		prnPrintf("%.19s\n",szCardTemp);
		prnPrintf("%s  %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			prnPrintf("%s MXN\n",(char *)szAmountBuff);
		}
		else
		{
			prnPrintf("%s USD\n",(char *)szAmountBuff);
		}
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==PRE_TIP_SALE ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("VI   %.19s %c\n",szCardTemp,GetEntryMode());     
		prnPrintf("%s  %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if( pstLog->szMoneyType == PARAM_PESEO )
		{
			prnPrintf("%s    PROP. %ld.%02ld MXN\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
		}
		else
		{
			prnPrintf("%s    PROP. %ld.%02ld USD\n",(char *)szAmountBuff,
				pstLog->lpreTipAmount/100,pstLog->lpreTipAmount%100);
		}
		
	}
	else if( gstPosCapability.uiPrnType == 'T' && ( pstLog->iTransNo==POS_PREAUTH ) )
	{
		prnPrintf("Mesero:%d\n",pstLog->waiterNo);
		prnPrintf("VI  %.19s %c\n",szCardTemp,GetEntryMode());
		prnPrintf("%s  %s %6.6s\n",(char *)szBuf,szTransTime, pstLog->szAuthNo);
		if(pstLog->ucAuthCmlId == TRUE)
		{
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				prnPrintf("%s    PROP. %ld.%02ld MXN\n",(char *)szAmountBuff,
					pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
			}
			else
			{
				prnPrintf("%s    PROP. %ld.%02ld USD\n",(char *)szAmountBuff,
					pstLog->lPreAuthTipAmount/100,pstLog->lPreAuthTipAmount%100);
			}
					}
		else 
		{
			if( pstLog->szMoneyType == PARAM_PESEO )
			{
				prnPrintf("%s MXN PENDIENTE POR MESERO\n",(char *)szAmountBuff);
			}
			else
			{
				prnPrintf("%s USD PENDIENTE POR MESERO\n",(char *)szAmountBuff);
			}
		}
	}
	
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/	
	return OK;
}



uint8_t PrtOneTranTxn(NEWPOS_LOG_STRC *pstLog)
{
	uint8_t	ucTranType, szBuff[100], szBuf[100];
	uint8_t   szCardTemp[50];

	memset((char*)szCardTemp,0,sizeof(szCardTemp));
	memset((char*)szBuff,0,sizeof(szBuff));
	memset((char*)szBuf,0,sizeof(szBuf));
	switch( pstLog->iTransNo )
	{
	case POS_SALE:
	case ICC_OFFSALE:
	case QPBOC_ONLINE_SALE:
	case ADJUST_TIP: 
		ucTranType = 'S';
		break;

	case POS_AUTH_CM:
		ucTranType = 'P';
		break;

	case POS_REFUND:
    case EC_REFUND:
		ucTranType = 'R';
		break;

	case OFF_SALE:
	case OFF_ADJUST:		
		ucTranType = 'L';
		break;

	case POS_OFF_CONFIRM:
		ucTranType = 'C';
		break;

	case POS_INSTALLMENT:
		ucTranType = 'T';
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:

	case EC_TOPUP_CASH:
	case EC_TOPUP_NORAML:
	case EC_TOPUP_SPEC:
		ucTranType = 'Q';
		break;

	default:
		ucTranType = ' ';
		break;
	}

	if( pstLog->iTransNo==ADJUST_TIP )
	{
		BcdAdd(pstLog->sAmount, pstLog->sTipAmount, 6);
	}
	ConvBcdAmount(pstLog->sAmount, szBuff);

	memset(szBuf, 0, sizeof(szBuf));
	if( (pstLog->iTransNo==POS_PREAUTH) || (pstLog->iTransNo==POS_PREAUTH_ADD) ||
		(pstLog->iTransNo==OFF_SALE) || (pstLog->iTransNo==OFF_ADJUST) ||
		(pstLog->iTransNo==ICC_OFFSALE) || ( pstLog->iTransNo == EC_QUICK_SALE) ||
		( pstLog->iTransNo == EC_NORMAL_SALE))
	{
		strcpy((char *)szCardTemp, (char *)pstLog->szCardNo);
	}
	else
	{
		MaskPan(pstLog->szCardNo, szCardTemp);
	}
	if (pstLog->iTransNo==OFF_SALE || pstLog->iTransNo==OFF_ADJUST || pstLog->iTransNo==ADJUST_TIP)
	{
		{
			sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, ucTranType, pstLog->szAuthNo,
				szBuff);
		}
	}
	else
	{
		sprintf((char *)szBuf, "%06d %c %6.6s %12.12s", pstLog->lTraceNo, ucTranType, pstLog->szAuthNo,
				szBuff);
	}
	
	PrnSmallFontData();
	if(gstPosCapability.uiPrnType == 'T')
		prnPrintf("%s %.19s\n",(char *)szBuf,szCardTemp);
/*	else
	{
		prnPrintf("%s\n",(char *)szBuf);
		prnPrintf("%.19s\n",szCardTemp);
	}
*/
	return OK;
}


uint8_t PrtTransTotal(uint8_t flag, int batchNo)
{
   	int	i, ret;
    uint8_t   buf1[40];
	uint8_t   sRmbTotalSaleAmt[20], szRmbAmout[13],szRmbEcAmount[13];
	
	lcdCls();
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "WAITING...");
	lcdFlip();
	ret = prnInit();
  	if ( ret!=OK )
  	{
  		lcdClrLine(2, 7);
		if(ret==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}

	if( stPosParam.ucTestFlag==PARAM_OPEN )
	{
		PrnSmallFontData();
		prnSetFontZoom(2, 2);
		prnPrintf("\n**TESTING,SLIP INVALID**\n");
		prnSetFontZoom(1, 1);
	}
	else 
	{
		PrnSmallFontData();
		prnPrintf("\n\n");
	}
		
	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();
	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
		prnPrintf("      SETTLE TOTALES\n");
	}
	else
	{
		prnPrintf("      SETTLE TOTALES\n");
	}
	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();

	prnPrintf("MERCHANT NAME:\n%s\n", stPosParam.szMerchantName);	
	prnPrintf("MERCHANT NO:\n%s\n", stPosParam.szUnitNum);	
	prnPrintf("TERMINAL NO:\n%s\n", stPosParam.szPosId);
	if( flag )
	{
		if( batchNo!=0 ) 
		{
			prnPrintf("BATCH NO:%06ld\n", batchNo);
		}
		else 
		{
			prnPrintf("BATCH NO:%06ld\n", stPosParam.lNowBatchNum);
		}
		prnPrintf("DATE/TIME:%.4s/%.2s/%.2s %.2s:%.2s:%.2s\n",
			&stTemp.szDate[0], &stTemp.szDate[4], &stTemp.szDate[6],
			&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);
	}
	else
	{
		prnPrintf("BATCH NO:%06ld\n", stTotal.lBatchNum);		
		prnPrintf("DATE/TIME:%.4s/%.2s/%.2s %.2s:%.2s:%.2s\n",
			&stTotal.sDateTime[0], &stTotal.sDateTime[4], &stTotal.sDateTime[6],
			&stTotal.sDateTime[8], &stTotal.sDateTime[10], &stTotal.sDateTime[12]);
	}
	
	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();
	prnPrintf("TYPE   SUM AMOUNT\n");
	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();
	
	if( stTransCtrl.ucRmbSettSaveRsp=='2' )
		prnPrintf("CUP CARD: NOT BALANCE\n");
	else if( stTransCtrl.ucRmbSettSaveRsp=='3' )
		prnPrintf("CUP CARD: SETTLE ERROR\n");
	else
		prnPrintf("CUP CARD: BALANCE\n");
	
	if(stTotal.iRmbSaleNum > stTotal.iRmbEcNum)
	{
		memset(szRmbAmout,0,sizeof(szRmbAmout));
		memset(szRmbEcAmount,0,sizeof(szRmbEcAmount));
		memset(sRmbTotalSaleAmt,0,sizeof(sRmbTotalSaleAmt));
		PubBcd2Asc0(stTotal.sRmbSaleAmt,6,(char *)szRmbAmout);
		PubBcd2Asc0(stTotal.sRmbEcAmt,6,(char *)szRmbEcAmount);
		PubAscSub((char *)szRmbAmout,(char *)szRmbEcAmount,12,(char *)sRmbTotalSaleAmt);
		PubAsc2Bcd((char *)sRmbTotalSaleAmt,12,(char *)szRmbAmout);
		i = ConvBcdAmount(szRmbAmout, buf1);
	    prnPrintf("SALE        %3d%12s\n", stTotal.iRmbSaleNum-stTotal.iRmbEcNum, buf1);
	}

	if( !AmountIsZero(stTotal.sRmbEcAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sRmbEcAmt, buf1);
		prnPrintf("EC SALE%3d%12s\n", stTotal.iRmbEcNum, buf1);
	}

	if( !AmountIsZero(stTotal.sRmbRefundAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sRmbRefundAmt, buf1);
		prnPrintf("REFUND      %3d%12s\n", stTotal.iRmbRefundNum, buf1);
	}
	if( !AmountIsZero(stTotal.sRmbConfirmAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sRmbConfirmAmt, buf1);
		prnPrintf("AUTH COMP %3d%12s\n", stTotal.iRmbConfirmNum, buf1);
	}
	if( !AmountIsZero(stTotal.sRmbOffConfirmAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sRmbOffConfirmAmt, buf1);
		prnPrintf("AUTH OFFL %3d%12s\n", stTotal.iRmbOffConfirmNum, buf1);
	}
	if( !AmountIsZero(stTotal.sRmbOffAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sRmbOffAmt, buf1);
		prnPrintf("OFFLINE     %3d%12s\n", stTotal.iRmbOffNum, buf1);
	}
	if( !AmountIsZero(stTotal.sTopUpAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sTopUpAmt, buf1);
		prnPrintf("LOAD        %3d%12s\n", stTotal.iTopUpNum, buf1);
	}
	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();

	if( stTransCtrl.ucFrnSettSaveRsp=='2' )
		prnPrintf("FOREIGN CARD:  NOT BALANCE\n");
	else if( stTransCtrl.ucFrnSettSaveRsp=='3' )
		prnPrintf("FOREIGN CARD:  SETTLE ERROR\n");
	else
		prnPrintf("FOREIGN CARD:  BALANCE\n");

	if( !AmountIsZero(stTotal.sFrnSaleAmt, 6) )
	{
		memset(szRmbAmout,0,sizeof(szRmbAmout));
		memset(szRmbEcAmount,0,sizeof(szRmbEcAmount));
		memset(sRmbTotalSaleAmt,0,sizeof(sRmbTotalSaleAmt));
		PubBcd2Asc0(stTotal.sFrnSaleAmt,6,(char *)szRmbAmout);
		PubBcd2Asc0(stTotal.sFrnEcAmt,6,(char *)szRmbEcAmount);
		PubAscSub((char *)szRmbAmout,(char *)szRmbEcAmount,12,(char *)sRmbTotalSaleAmt);
		PubAsc2Bcd((char *)sRmbTotalSaleAmt,12,(char *)szRmbAmout);
		i = ConvBcdAmount(szRmbAmout, buf1);
		prnPrintf("SALE        %3d%12s\n", stTotal.iFrnSaleNum-stTotal.iFrnEcNum, buf1);
	}
	
	if( !AmountIsZero(stTotal.sFrnEcAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sFrnEcAmt, buf1);
			prnPrintf("EC SALE%3d%12s\n", stTotal.iFrnEcNum, buf1);
	}

	if( !AmountIsZero(stTotal.sFrnRefundAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sFrnRefundAmt, buf1);
		prnPrintf("REFUND      %3d%12s\n", stTotal.iFrnRefundNum, buf1);
	}
	if( !AmountIsZero(stTotal.sFrnConfirmAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sFrnConfirmAmt, buf1);
		prnPrintf("AUTH COMP  %3d%12s\n", stTotal.iFrnConfirmNum, buf1);
	}
	if( !AmountIsZero(stTotal.sFrnOffConfirmAmt, 6) )
	{	
		i = ConvBcdAmount(stTotal.sFrnOffConfirmAmt, buf1);
		prnPrintf("AUTH OFFL %3d%12s\n", stTotal.iFrnOffConfirmNum, buf1);
	}
	if( !AmountIsZero(stTotal.sFrnOffAmt, 6) )
	{
		i = ConvBcdAmount(stTotal.sFrnOffAmt, buf1);
		prnPrintf("OFFLINE     %3d%12s\n", stTotal.iFrnOffNum, buf1);
	}

	if( flag==0 ) 
	{
		prnPrintf("****DUPLICATED****\n");
	}

	if( stPosParam.ucTicketNum=='0' ) 
		prnPrintf("\f");
	else 
		prnPrintf("\n\n\n");	

	ret = PrintData();
	return(ret);
}

uint8_t PrtNowTotal(void)
{
   	int	i, ret;
    uint8_t   buf[50], buf1[50];
	uint8_t   amtBuf[20];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   PRINT TOTAL  ");	
	if( stTransCtrl.iTransNum==0 ) 
		return(E_NO_TRANS);
		
	DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PRINT TOTAL..");
	lcdFlip();
	ret = prnInit();
  	if ( ret!=OK )
  	{
  		lcdClrLine(2, 7);
		if(ret==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return NO_DISP;
  	}

	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();

	if(stPosParam.ucPrnTitleFlag == PARAM_OPEN)
	{
		newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
	    prnPrintf("       TRANSACTION TOTAL LIST\n");
	}
	else
	{
		prnPrintf("       TRANSACTION TOTAL LIST\n");
	}

	PrnSmallFontData();	
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();

	memset(buf, 0, sizeof(buf));
	prnPrintf("MERCHANT NAME:\n%s\n", stPosParam.szEngName);
	prnPrintf("MERCHANT NO:\n%s\n", stPosParam.szUnitNum);		
	prnPrintf("TERMINAL NO:\n%s\n", stPosParam.szPosId);	
	prnPrintf("OPERATOR NO:%s\n", stTransCtrl.szNowTellerNo+1);	
	prnPrintf("BATCH NO:%06ld\n", stPosParam.lNowBatchNum);
	
	prnPrintf("DATE/TIME:%.4s/%.2s/%.2s %.2s:%.2s:%.2s\n",
		&stTemp.szDate[0], &stTemp.szDate[4], &stTemp.szDate[6],
		&stTemp.szTime[0], &stTemp.szTime[2], &stTemp.szTime[4]);
	
	PrnSmallFontData();
	prnPrintf("-------------------------------------------\n");
	PrnBigFontChnData();
	prnPrintf("TYPE   SUM AMOUNT\n");
	PrnSmallFontData();
	if (gstPosCapability.uiPrnType == 'T')
		prnPrintf("-------------------------------------------\n");
	else
		prnPrintf("--------------------------------\n");
	PrnBigFontChnData();

	
	memset(buf, ' ', 30);
	memcpy(buf, "CUP DEBIT",9);
	sprintf((char *)buf+15, "%3d", stTotal.iRmbDebitNum);
	prnPrintf((char *)buf);
	memcpy(amtBuf, stTotal.sRmbDebitAmt, 6);
	i = ConvBcdAmount(amtBuf, buf1);
	buf1[i] = '\0';
	prnPrintf("   ");
	prnPrintf((char *)buf1);
	prnPrintf("\n");

	memset(buf, ' ', 30);
	memcpy(buf, "CUP CREDIT",10);
	sprintf((char *)buf+15, "%3d", stTotal.iRmbCreditNum);
	prnPrintf((char *)buf);
	memcpy(amtBuf, stTotal.sRmbCreditAmt, 6);
	i = ConvBcdAmount(amtBuf, buf1);	
	buf1[i] = '\0';
	prnPrintf("   ");
	prnPrintf((char *)buf1);
	prnPrintf("\n");

	memset(buf, ' ', 30);
	memcpy(buf, "FOREIGN DEBIT", 13);
	sprintf((char *)buf+15, "%3d", stTotal.iFrnDebitNum);
	prnPrintf((char *)buf);
	memcpy(amtBuf, stTotal.sFrnDebitAmt, 6);
	i = ConvBcdAmount(amtBuf, buf1);
	buf1[i] = '\0';
	prnPrintf("   ");
	prnPrintf((char *)buf1);
	prnPrintf("\n");
		
	memset(buf, ' ', 30);
	memcpy(buf, "FOREIGN CREDIT", 14);
	sprintf((char *)buf+15, "%3d", stTotal.iFrnCreditNum);
	prnPrintf((char *)buf);
	memcpy(amtBuf, stTotal.sFrnCreditAmt, 6);
	i = ConvBcdAmount(amtBuf, buf1);	
	buf1[i] = '\0';
	prnPrintf("   ");
	prnPrintf((char *)buf1);
	prnPrintf("\n");

	PrnSmallFontData();
	if (gstPosCapability.uiPrnType == 'T')
	{
		prnPrintf("-------------------------------------------\n");
		PrnBigFontChnData();
		if( stPosParam.ucTicketNum=='0' ) 
			prnPrintf("\f");
		else 
			prnPrintf("\n\n\n");
	}
	else
		prnPrintf("--------------------------------\n");
	
	ret = PrintData();
    return(ret);
}

uint8_t PrintAllErrOffTrans(void)
{
	uint8_t ret, ucRet;
	uint8_t hasPrt = 0;
	NEWPOS_LOG_STRC stLog;	
	int  iRecNo, iNumOfOnePage,iRet;
	int fd;

	fd = fileOpen(POS_LOG_FILE, O_RDWR);
	if( fd<0 ) 
	{
		return (E_MEM_ERR);
	}
	fileClose(fd);

	iNumOfOnePage = 0;
	for(iRecNo=0; iRecNo<stTransCtrl.iTransNum; iRecNo++)
	{
		if (stTransCtrl.sTxnRecIndexList[iRecNo] == INV_TXN_INDEX)
		{
			continue;
		}

		if (!(stTransCtrl.sTxnRecIndexList[iRecNo] & TS_TXN_OFFLINE) 	
			&& !(stTransCtrl.sTxnRecIndexList[iRecNo] & TS_ICC_OFFLINE))
		{
			continue;
		}

		memset(&stLog, 0, sizeof(stLog));
		ret = LoadTranLog(iRecNo, &stLog);
		if( ret )
		{
			continue;
		}
		
		if( stLog.ucSendFail != TS_NOT_RECV)
		{
			continue;
		}

		if( hasPrt==0x00 )
		{			
   			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
			prnPrintf((char *)"\n");
			PrnSmallFontData();
			prnPrintf("UNSUCCESSSFUL LIST\n");
			PrtTranDetailHead();
			hasPrt = 0x01;
		}

		ucRet = PrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			return ucRet;
		}
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				prnClose();
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			prnClose();
			return ucRet;
		}
	}
	prnClose();
	
	hasPrt = 0;
	iNumOfOnePage = 0;
	for(iRecNo=0; iRecNo<stTransCtrl.iTransNum; iRecNo++)
	{
		if (stTransCtrl.sTxnRecIndexList[iRecNo] == INV_TXN_INDEX)
		{
			continue;
		}
		
		if (!(stTransCtrl.sTxnRecIndexList[iRecNo] & TS_TXN_OFFLINE) 	
			&& !(stTransCtrl.sTxnRecIndexList[iRecNo] & TS_ICC_OFFLINE))
		{
			continue;
		}
		
		memset(&stLog, 0, sizeof(stLog));
		ret = LoadTranLog(iRecNo, &stLog);
		if( ret )
		{
			continue;
		}
		
		if( stLog.ucSendFail != TS_NOT_CODE)
		{
			continue;
		}
		
		// 打印故障单
		if( hasPrt==0x00 )
		{			
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
			prnPrintf((char *)"\n");
			PrnSmallFontData();
			prnPrintf("DENIED LIST\n");
			PrtTranDetailHead();
			hasPrt = 0x01;
		}
		
		ucRet = PrtOneTranTxn(&stLog);
		if( ucRet!=OK )
		{
			prnClose();
			return ucRet;
		}
		iNumOfOnePage++;	
		if( iNumOfOnePage>=NUM_PER_PAGE )
		{
			ucRet = PrintData();
			if( ucRet!=OK )
			{
				prnClose();
				return ucRet;
			}
			iNumOfOnePage = 0;
			iRet = prnInit();
  			if ( iRet!=OK )
  			{
  				lcdClrLine(2, 7);
				if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
				else
    				DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    			lcdFlip();
    			sysDelayMs(3000);
				return NO_DISP;
  			}
		}
	}
		
	if( iNumOfOnePage>0 )
	{
		if(gstPosCapability.uiPrnType == 'T')
			prnPrintf("\n\n\n\n");
		ucRet = PrintData();
		if( ucRet!=OK )
		{
			prnClose();
			return ucRet;
		}
	}
	else
	{
		prnClose();
	}
	return 0;
}

void DispPrnErrInfo(uint8_t err)
{
	uint8_t dispStr1[20], dispStr2[20];
	
	lcdClrLine(2, 7);	
	strcpy((char *)dispStr1, "PRINT FAIL");
	
	switch(err)
	{
	case PRN_WRONG_PACKAGE:
		strcpy((char *)dispStr2, "DATA FMT ERR");
		break;
	case PRN_FAULT:
		strcpy((char *)dispStr2, "PRINTER ERR");
		break;
	case PRN_TOOHEAT:
		strcpy((char *)dispStr2, "OVERHEATED");
		break;
	case PRN_UNFINISHED:
		strcpy((char *)dispStr2, "INCOMPLETE");
		break;
	case PRN_NOFONTLIB:
		strcpy((char *)dispStr2, "FONT ABSENT");
		break;
	case PRN_OUTOFMEMORY:
		strcpy((char *)dispStr2, "DATA TOO LONG");
		break;
	default:
		strcpy((char *)dispStr2, "OTHER ERR");
		break;
	}
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, (char *)dispStr1);
	lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, (char *)dispStr2);
	lcdFlip();
	sysBeef(3, 1000);

}


uint8_t PrintData(void)
{
	uint8_t ret;
	int key;
	
	while(1)
	{
        ret = prnStart();
		if( ret==PRN_OK )
		{
			prnClose();
			return(OK);
		}
		if( ret!=PRN_OK )
		{
			CommHangUp(FALSE);
			if( ret==PRN_PAPEROUT || ret==PRN_NOBATTERY)
			{
				lcdCls();	
				lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
				if( ret==PRN_PAPEROUT )
					DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, "Impresora sin papel");
				else
					DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, "CONNECT BATTERY");
				DispMulLanguageString(0, 4, DISP_MEDIACY|DISP_CFONT, NULL, "[Enter]-Imprimir");
				DispMulLanguageString(0, 6, DISP_MEDIACY|DISP_CFONT, NULL, "[Cancelar]-Salir");
				lcdFlip();
				lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
				sysBeef(2, 1000);
				do{
					key = kbGetKey();
				}while( key!=KEY_ENTER && key!=KEY_CANCEL );
				if( key==KEY_ENTER )
				{
					lcdCls();
					DispMulLanguageString(0, 4, DISP_MEDIACY|DISP_CFONT, NULL, "IMPRIMIENDO...");
					lcdFlip();
					continue;
				}
				else
				{
					prnClose();	
					return(PRN_PAPEROUT);
				}
			}

			prnClose();				
			DispPrnErrInfo(ret);
			kbGetKey();
			return(NO_DISP);
		}
	}
}

uint8_t GetEntryMode(void)
{
	if( PosCom.stTrans.ReadCardType==CARD_SWIPED)
	{
		PosCom.stTrans.szEntryMode[1]='2';
	}
	if (PosCom.stTrans.iTransNo == EC_TOPUP_NORAML)
	{
		if (PosCom.stTrans.ucEcTopOldSwipeFlag== CARD_INSERTED)
		{
			return 'I';
		}
		else
		{
			return 'S';
		}
	}
	if( PosCom.stTrans.ucFallBack )
	{
		return 'S';
	}
	if( PosCom.stTrans.szEntryMode[1]=='1' )
	{
		return 'M';
	}
	if( PosCom.stTrans.szEntryMode[1]=='2' )
	{
		return 'D';
	}
	if( PosCom.stTrans.szEntryMode[1]=='5' )
	{
		return 'C';
	}
	if (PosCom.stTrans.szEntryMode[1]=='7')
	{
		return 'C';
	}

	if (memcmp(PosCom.stTrans.szEntryMode,"96",2)==0)
	{
		return 'C';
	}

	return ' ';
}

void PrnEmvParam(void)
{
	int		iRet, iCnt;
	uint8_t		szBuff[40];
	struct terminal_aid_info stEmvApp;
	struct issuer_ca_public_key	stEmvCapk;
	int       fd, fileLen, Num;

	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return;
  	}
	PrnBigFontChnData();
	prnPrintf("EMV PARAM INFO\n");
	emv_get_mck_configure(&tConfig);
	memset(szBuff, 0, sizeof(szBuff));
	sprintf((char *)szBuff, "MERCHEANT TYPE: %02X %02X\n", tEMVCoreInit.merchant_cate_code[0], tEMVCoreInit.merchant_cate_code[1]);
	if (gstPosCapability.uiPrnType == 'T')//热敏
		PrnSmallFontData();
	prnPrintf((char *)szBuff);
	prnPrintf("TERMINAL TYPE: %02X\n", tConfig.terminal_type);
	prnPrintf("TERMINAL CABLE: %02X %02X %02X\n", tConfig.terminal_capabilities[0], tConfig.terminal_capabilities[1], 
		tConfig.terminal_capabilities[2]);
	prnPrintf("EXPANSION CABLE: %02X %02X %02X %02X %02X\n", tConfig.additional_terminal_capabilities[0], 
		tConfig.additional_terminal_capabilities[1], tConfig.additional_terminal_capabilities[2], 
		tConfig.additional_terminal_capabilities[3], tConfig.additional_terminal_capabilities[4]);
	prnPrintf("COUNTRY CODE: %02X %02X\n", tEMVCoreInit.terminal_country_code[0], tEMVCoreInit.terminal_country_code[1]);

	prnPrintf("EMV LIST VIEW\n");
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(stEmvApp));
		iRet = emv_get_one_aid_info(iCnt, &stEmvApp);
		if( iRet<0 )
		{
			continue;
		}
		BcdToAsc0(szBuff, stEmvApp.aid, stEmvApp.aid_len*2);
		prnPrintf("AID: %s\n", szBuff);
		prnPrintf("Floor Limit: %d.%02d\n", stEmvApp.terminal_floor_limit/100, stEmvApp.terminal_floor_limit%100);
		BcdToAsc0(szBuff, stEmvApp.terminal_action_code_denial, 10);
		prnPrintf("TAC Denial:  %s\n", szBuff);
		BcdToAsc0(szBuff, stEmvApp.terminal_action_code_online, 10);
		prnPrintf("TAC Online:  %s\n", szBuff);
		BcdToAsc0(szBuff, stEmvApp.terminal_action_code_default, 10);
		prnPrintf("TAC Default: %s\n", szBuff);
		prnPrintf("EC MAX AMT: %d.%02d\n", stEmvApp.cl_ReaderMaxTransAmount/100,stEmvApp.cl_ReaderMaxTransAmount%100);
		prnPrintf("EC Floor limit: %d.%02d\n", stEmvApp.cl_Floor_Limit/100,stEmvApp.cl_Floor_Limit%100);
		prnPrintf("EC CVM Amt: %d.%02d\n\n", stEmvApp.cl_CVM_Amount/100,stEmvApp.cl_CVM_Amount%100);
	}
	
	prnPrintf("\nEMV PUBLIC-KEY LIST\n");
    fd = fileOpen("emv.capk", O_RDWR);
    if( fd>=0 )
    {
        fileLen = fileSize("emv.capk");
        Num = fileLen/sizeof(struct issuer_ca_public_key);

        for(iCnt=0; iCnt<Num; iCnt++)
        {
            	fileSeek(fd, iCnt*sizeof(struct issuer_ca_public_key), SEEK_SET);
            	fileRead(fd, (uint8_t*)&stEmvCapk, sizeof(struct issuer_ca_public_key));
            	BcdToAsc0(szBuff, stEmvCapk.RID, 10);
		prnPrintf("RID: %s, Key ID:%02X, ", szBuff, stEmvCapk.Index);
		BcdToAsc0(szBuff, stEmvCapk.expiration_date, 6);
		prnPrintf("Expiry: 20%.2s-%.2s-%.2s\n", szBuff, &szBuff[2], &szBuff[4]);
        }
        fileClose(fd);
    }


	prnPrintf("\n\n\n");

	PrintData();
}


void  GetTransPrtName(int iTransType, char *pszTransName)
{
	char  szBuffer[100];

	memset(szBuffer,0,sizeof(szBuffer));
	switch(iTransType) 
	{
	case POS_SALE:
	case ICC_OFFSALE:
	case INTERESES_SALE:
	case POS_PREAUTH:
	case PURSE_SALE:
		strcpy(szBuffer,"  VENTA");			
		break;
	case POS_SALE_VOID:
		strcpy(szBuffer,"  CANCELACION");				
		break;
	case POS_AUTH_VOID:
		strcpy(szBuffer,"  VOID AUTH COMP\n");			
		break;
	case POS_PREAUTH_VOID:
		strcpy(szBuffer,"  VOID PREAUTH\n");			
		break;
	case POS_OFF_CONFIRM:
		strcpy(szBuffer,"  AUTH OFFL\n");			
		break;
	case POS_AUTH_CM:
		strcpy(szBuffer,"  AUTH COMP\n");			
		break;
	case POS_REFUND:
		strcpy(szBuffer,"  CANCELACION");				
		break;
/*	case POS_PREAUTH:
		strcpy(szBuffer,"  PREAUTH\n");			
		break;
*/	case POS_PREAUTH_ADD:
		strcpy(szBuffer,"  ADD-AUTH\n");			
		break;
	case OFF_SALE:
		strcpy(szBuffer,"  OFF-SETTLE\n");			
		break;
	case OFF_ADJUST:
	case ADJUST_TIP:
		strcpy(szBuffer,"  OFF-ADJUST");			
		break;

	case POS_INSTALLMENT:
		strcpy(szBuffer,"  INSTALLMENT");				
		break;

	case POS_VOID_INSTAL:
		strcpy(szBuffer,"  VOID INSTALL");			
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:
		strcpy(szBuffer,"  EC SALE");				
		break;

	case QPBOC_ONLINE_SALE:
		strcpy(szBuffer,"  SALE");			
		break;

	case EC_TOPUP_NORAML:
		strcpy(szBuffer,"EC TOPUP NORMAL");				
		break;

	case EC_TOPUP_SPEC:
		strcpy(szBuffer,"EC TOPUP SPEC");				
		break;

	case EC_TOPUP_CASH:
		strcpy(szBuffer,"EC TOPUP CASH");				
		break;
		
	case EC_VOID_TOPUP:
		strcpy(szBuffer,"EC VOID TOPUP");				
		break;

	case EC_REFUND:
		strcpy(szBuffer,"EC REFUND");
		break;
	case LAST_MOVEMENT:
		strcpy(szBuffer,"CONSULTA MOVS.LINEA");
		break;
	case PAYMENT_WITH_CARD:
		strcpy(szBuffer,"PAGO A NETPAY CON TARJETA");
		break;
	case PAYMENT_WITH_BALANCE:
		strcpy(szBuffer,"PAGO A NETPAY CON VENTAS");
		break;
	case POS_SETT:
		strcpy(szBuffer,"TOTAL DEL CARGO");
		break;
	case PHONE_TOPUP:
		strcpy(szBuffer,"TIEMPO AIRE");
		break;
	case BALANCE_CHECK:
		strcpy(szBuffer,"CONSULTA DE SALDO");
		break;
	case CHECK_IN:
		strcpy(szBuffer,"CHECK IN");
		break;
	case CHECK_OUT:
		strcpy(szBuffer,"CHECK OUT");
		break;
	case CASH_ADVACNE:
		strcpy(szBuffer,"RETIRO USD");
		break;
	case NETPAY_REFUND:
		strcpy(szBuffer,"DEVOLUCION");
		break;
	case NETPAY_ADJUST:
		strcpy(szBuffer,"AJUSTE");
		break;
	case NETPAY_FORZADA:
		strcpy(szBuffer,"FORZADA");
		break;
	default:
		if (iTransType >= EC_QUICK_SALE && iTransType <= POS_VOID_INSTAL)
		{
			GetTransName(iTransType,(uint8_t*)szBuffer);
			szBuffer[strlen(szBuffer)] = '\n';
		}
		break;
	}

	if (pszTransName != NULL)
	{
		sprintf(pszTransName,"%s",szBuffer);
	}

	return ;
}


void  NetpayPrintAmount(int flag)
{
	char  buf[200], buf1[200];/*buf2[200],*///buf3[200];
	long  szlOriAmount;
	char  szAscOriAmount[20];
	uint8_t szBcdOriAmount[7];
	char  szAscTipAmount[20];
	long  szlTipAmount;
	char  szAscTotalAmount[20];
	long  szlTotalAmount = 0;
	//long szcount =0;
	
	prnPrintf("\n");
	//switch(stTemp.iTransNo)
	switch(PosCom.stTrans.iTransNo)
	{
		case POS_SALE:
			if(stPosParam.stVolContrFlg ==PARAM_OPEN)
			{
				memset(buf1, 0, sizeof(buf1));
				memset(buf, 0, sizeof(buf));		
				PubBcd2Asc0(PosCom.stTrans.sTipAmount,6,szAscTipAmount);
				szlTipAmount = atol(szAscTipAmount);
				PubBcd2Asc0(PosCom.stTrans.sAmount,6,szAscTotalAmount);
				szlTotalAmount = atol(szAscTotalAmount);
				szlOriAmount = szlTotalAmount - szlTipAmount;
				sprintf(szAscOriAmount,"%010ld%02ld",szlOriAmount/100,szlOriAmount%100);
				AscToBcd(szBcdOriAmount,(uint8_t *)szAscOriAmount,12);
				
				ConvBcdAmount(szBcdOriAmount, (uint8_t *)buf1);		


				sprintf(buf, "MONTO    $            %s\n", buf1);

				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();

				memset(buf1, 0, sizeof(buf1));
				memset(buf, 0, sizeof(buf));
				//if(PosCom.stTrans.sTipAmount !=0)
				if(stPosParam.szpreTip == PARAM_OPEN)
				{
					ConvBcdAmount(PosCom.stTrans.sTipAmount, (uint8_t *)buf1);
					sprintf(buf, "PROP.      $            %s\n", buf1);
					PrnBlackEngData();
					prnPrintf(buf);
					PrnBigFontChnData();

				}
			}
	
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));
			ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);
			sprintf(buf, "TOTAL     $            %s\n", buf1);
			PrnBlackEngData();
			prnPrintf(buf);
			PrnBigFontChnData();
			break;
		case PURSE_SALE:
			if(PosCom.stTrans.TransFlag ==PURSE_TODITO)
			{
				memset(buf1, 0, sizeof(buf1));
				memset(buf, 0, sizeof(buf));
				ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);
				sprintf(buf, "TOTAL M.N.     $           %s\n", buf1);
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
			}
			else if(PosCom.stTrans.TransFlag ==PURSE_SODEXO)
			{	
				memset(buf1, 0, sizeof(buf1));
				memset(buf, 0, sizeof(buf));
				ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);
				sprintf(buf, "TOTAL M.N.     $           %s\n", buf1);
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
				
				//memset(buf1, 0, sizeof(buf1));
				memset(buf, 0, sizeof(buf));

				if(PosCom.stTrans.CtlFlag ==1)
				{
					if(strlen(PosCom.stTrans.Balance_Amount)!=0 )
					{
						sprintf(buf, "SALDO ACT.   $            %s\n", PosCom.stTrans.Balance_Amount);
					}
					else
					{									
						sprintf(buf, "SALDO ACT.   $               0.00\n");
					}
				}
				else
				{
						sprintf(buf, "SALDO ACT.   $         %s\n", PosCom.stTrans.Prt_SB);
				}
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
			}
			else if(PosCom.stTrans.TransFlag ==PURSE_PUNTO)
			{

				memset(buf, 0, sizeof(buf));
				memset(buf1,0,sizeof(buf1));
				ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);

				if(PosCom.stTrans.saletype[0] == '1')
				{
					sprintf(buf, "IMPORTE.                        %s\n",buf1);
				}
				else
				{				
					sprintf(buf, "LITROS.                           %s\n",buf1);
				}
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
				
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "SALDO ACT.      $            %s\n", PosCom.stTrans.Balance_Amount);
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
		
			}
			else if(PosCom.stTrans.TransFlag ==PURSE_EDENRED)
			{

				memset(buf, 0, sizeof(buf));
				memset(buf1,0,sizeof(buf1));
				ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);

				if(PosCom.stTrans.saletype[0] == '1')
				{
					sprintf(buf, "IMPORTE.                      %s\n",buf1);
				}
				else
				{				
					sprintf(buf, "LITROS.                           %s\n",buf1);
				}
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
				
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "SALDO ACT.      $       %s\n", PosCom.stTrans.Balance_Amount);
				PrnBlackEngData();
				prnPrintf(buf);
				PrnBigFontChnData();
		
			}
			break;
		case PURSE_AUTH:
			memset(buf, 0, sizeof(buf));
			memset(buf1,0,sizeof(buf1));
			ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);

			if(PosCom.stTrans.saletype[0] == '1')
			{
				sprintf(buf, "IMPORTE.                    %s\n",buf1);
			}
			else
			{				
				sprintf(buf, "LITROS.                       %s\n",buf1);
			}
			PrnBlackEngData();
			prnPrintf(buf);
			PrnBigFontChnData();
			
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "SALDO ACT.      $       %s\n", PosCom.stTrans.Balance_Amount);
			PrnBlackEngData();
			prnPrintf(buf);
			PrnBigFontChnData();
			break;
		default:
			break;

	}
	
}


void  PrintAmount(void)
{
	char  buf[200], buf1[200];
	long  szlOriAmount;
	char  szAscOriAmount[20];
	uint8_t szBcdOriAmount[7];
//	uint8_t szBcdTipAmount[7];
	char  szAscTipAmount[20];
	long  szlTipAmount;
//	uint8_t szBcdTotalAmount[7];
	char  szAscTotalAmount[20];
	long  szlTotalAmount;
	
//	prnPrintf("AMOUNT:\n");
	
	memset(buf1, 0, sizeof(buf1));
	memset(buf, 0, sizeof(buf));
	ConvBcdAmount(PosCom.stTrans.sAmount, (uint8_t *)buf1);
	if(stPosParam.szpreTip == PARAM_OPEN &&   
		PosCom.stTrans.iTransNo == POS_SALE &&
		!AmountIsZero(PosCom.stTrans.preTipAmount, 6) &&
		PosCom.stTrans.TransFlag==PRE_TIP_SALE)
	{
		memset(buf1,0,sizeof(buf1));
		PubBcd2Asc0(PosCom.stTrans.preTipAmount,6,szAscTipAmount);
		szlTipAmount = atol(szAscTipAmount);
		PubBcd2Asc0(PosCom.stTrans.sAmount,6,szAscTotalAmount);
		szlTotalAmount = atol(szAscTotalAmount);
		szlOriAmount = szlTotalAmount - szlTipAmount;
		sprintf(szAscOriAmount,"%010ld%02ld",szlOriAmount/100,szlOriAmount%100);
		AscToBcd(szBcdOriAmount,(uint8_t *)szAscOriAmount,12);		
		ConvBcdAmount(szBcdOriAmount, (uint8_t *)buf1);
		
	}
	
	if (stPosParam.ucPrnNegative == PARAM_OPEN)
	{
		if (PosCom.stTrans.iTransNo==POS_PREAUTH_VOID      || PosCom.stTrans.iTransNo==POS_AUTH_VOID
			||PosCom.stTrans.iTransNo==POS_SALE_VOID       || PosCom.stTrans.iTransNo==POS_REFUND
			||PosCom.stTrans.iTransNo==EC_REFUND           || PosCom.stTrans.iTransNo==EC_VOID_TOPUP
			||PosCom.stTrans.iTransNo==POS_VOID_INSTAL)
		{
			sprintf(buf, "MONTO  -$               %s\n", buf1);
		}
		else
		{
			sprintf(buf, "MONTO   $                %s\n", buf1);
		}
	}
	else
	{
		sprintf(buf, "TOTAL $               %s\n", buf1);
	}
	PrnBlackEngData();
	prnPrintf(buf);
	PrnBigFontChnData();
	if (PosCom.stTrans.iTransNo == POS_INSTALLMENT || PosCom.stTrans.iTransNo == POS_VOID_INSTAL)
	{
		PrnSmallFontData();
//		prnPrintf("-----------------------------------------------\n");
		return;
	}
	if( PosCom.stTrans.iTransNo==POS_PREAUTH_ADD )
	{
		if( !AmountIsZero(PosCom.stTrans.sPreAddAmount, 6) )
		{
			prnPrintf("TOTAL  :\n");			
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));
			ConvBcdAmount(PosCom.stTrans.sPreAddAmount, (uint8_t *)buf1);
			sprintf(buf, "Importe  $                %s\n", buf1);
			PrnBlackEngData();
			prnPrintf(buf);
			PrnBigFontChnData();
		}
	}
	
	if( !AmountIsZero(PosCom.stTrans.preTipAmount, 6) && PosCom.stTrans.TransFlag==PRE_TIP_SALE)
	{				
		memset(buf1, 0, sizeof(buf1));
		memset(buf, 0, sizeof(buf));
		ConvBcdAmount(PosCom.stTrans.preTipAmount, (uint8_t *)buf1);
		sprintf(buf, "PROP.     $                %s\n", buf1);
		PrnBlackEngData();
		prnPrintf(buf);
		PrnBigFontChnData();
//		prnPrintf("TOTAL :\n");
			
		memcpy(buf1, PosCom.stTrans.sAmount, 6);
//		BcdAdd((uint8_t *)buf1, (uint8_t *)PosCom.stTrans.sAmount, 6);
		ConvBcdAmount((uint8_t *)buf1, (uint8_t *)buf);
		sprintf(buf1, "TOTAL    $               %s\n", buf);
		PrnBlackEngData();
		prnPrintf(buf1);
		PrnBigFontChnData();
	}
	else if( PosCom.stTrans.iTransNo==POS_PREAUTH && PosCom.stTrans.TransFlag==POS_PREAUTH )
	{
		sprintf(buf, "PROP.     $_______________________\n");
		PrnBlackEngData();
		prnPrintf(buf);
		PrnBigFontChnData();
			
		sprintf(buf1, "TOTAL    $_______________________\n");
		PrnBlackEngData();
		prnPrintf(buf1);
		PrnBigFontChnData();
	}

}


void  PrintAddtionalInfo(void)
{
	char  buf[200], buf1[200];

	memset(buf1,0,sizeof(buf1));
	memset(buf,0,sizeof(buf));
	PrnSmallFontData();
	if (PosCom.stTrans.iTransNo == POS_INSTALLMENT || PosCom.stTrans.iTransNo == POS_VOID_INSTAL)
	{
		sprintf(buf,"INSTAL NUM: %d\n",PosCom.stTrans.ucInstallmentNum);
		prnPrintf(buf);
		if (!PosCom.stTrans.ucInstalPayMode)
		{
			prnPrintf("INITIAL AMT: ");
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));
			ConvBcdAmount(PosCom.stTrans.sTipAmount,(uint8_t *)buf1);
			sprintf(buf, "%s\n", buf1);
			prnPrintf(buf);	

			sprintf(buf,"CURRENCY: %3.3s\n",PosCom.stTrans.szInstalCurrCode);
			prnPrintf(buf);

			prnPrintf("FEE: ");
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));
			ConvBcdAmount(PosCom.stTrans.sPreAddAmount,(uint8_t *)buf1);
			sprintf(buf, "%s\n", buf1);
			prnPrintf(buf);
		}
		else
		{
			prnPrintf("INITIAL AMT: ");
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));
			ConvBcdAmount(PosCom.stTrans.sTipAmount,(uint8_t *)buf1);
			sprintf(buf, "%s\n", buf1);
			prnPrintf(buf);
			sprintf(buf,"CURRENCY: %3.3s\n",PosCom.stTrans.szInstalCurrCode);
			prnPrintf(buf);
			prnPrintf("INITIAL FEE:");
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));
			if (!(PosCom.stTrans.ucInstalAmtFlag & TS_INST_SPAC))
			{
				ConvBcdAmount(PosCom.stTrans.sPreAddAmount,(uint8_t *)buf1);
				sprintf(buf, "%s\n", buf1);
				prnPrintf(buf);				
			}
			else
			{
				sprintf(buf, "\n");
				prnPrintf(buf);
			}
			prnPrintf("EACH FEE:");
			memset(buf1, 0, sizeof(buf1));
			memset(buf, 0, sizeof(buf));

			if (!(PosCom.stTrans.ucInstalAmtFlag & TS_FEE_SPAC))
			{
				PubBcd2Asc0(PosCom.stTrans.sLoyaltAmount,6,buf);
				ConvBcdAmount(PosCom.stTrans.sLoyaltAmount,(uint8_t *)buf1);
				sprintf(buf, "%s\n",buf1);
				prnPrintf(buf);	
			}
			else
			{
				sprintf(buf, "\n");
				prnPrintf(buf);
			}
		}

		return;
	}
}

uint8_t PrintAllErrElecSignTrans(void)
{
	int iRecNo,iRet;

	for(iRecNo=0; iRecNo<stTransCtrl.iTransNum; iRecNo++)
	{
		if (stTransCtrl.sTxnRecIndexList[iRecNo] == INV_TXN_INDEX)
		{
			continue;
		}

		if (!(stTransCtrl.sTxnRecIndexList[iRecNo] & TS_ELECSIGN_TXN)) 
			continue;

		InitCommData();
		iRet = LoadTranLog(iRecNo, &PosCom.stTrans);
		if( iRet )
		{
			return iRet;
		}

		PrtTranTicket(REELECSIGN);
	}
	return 0;

}

uint8_t PrintBalanceCheck(char *T0,char *T1,char *T2,char *T3,char *T4,char *T5,char *T6)
{
	char  /*buf[200],*/ szPrnTitle[200],buf1[200];
	int iRet;
//	char szDate1[14+1] = {0};
//	char szDate2[21+1] = {0};
	char printDate[30];
	
	lcdClrLine(2, 7);
  	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
    lcdFlip();

	iRet = prnInit();
    if ( iRet!=OK )
    {
        lcdClrLine(2, 7);
        DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
        lcdFlip();
        kbGetKey();
        return NO_DISP;
    }

	if (stPosParam.ucPrnTitleFlag == PARAM_OPEN)
    {
      newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
    }
    else
    {
      memset(szPrnTitle,0,sizeof(szPrnTitle));
      if (strlen((char*)stPosParam.szPrnTitle)>18)
      {
        prnSetFontZoom(1, 1);
        PrnBigFontChnData();
      }
      sprintf(szPrnTitle,"%s SLIP\n",stPosParam.szPrnTitle);
      prnPrintf(szPrnTitle);
      PrnSmallFontData();
    }

	prnPrintf("\n");	
	GetTransPrtName(PosCom.stTrans.iTransNo, buf1); 
	prnPrintf("TIPO DE TRANSACCION:\n");
	prnPrintf("     %s\n\n",buf1);
	
    PrnBigFontChnData();
    prnStep(2);
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  
	
	PrnBigFontChnData();

	prnPrintf("NUMERO DE COMERCIO : %s\n", stPosParam.szST);  //store ID
	prnPrintf("APROBACION %s\n", PosCom.stTrans.szAuthNo);  //APROBACION
	
	MakeFormatPrintDate(PosCom.stTrans.szDate,(uint8_t *)printDate);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",printDate, 
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

	prnPrintf("TERMINAL: %s\n", PosCom.stTrans.szPosId);
	prnPrintf("CARGO: %06ld\n", PosCom.stTrans.lTraceNo);//water NO
	prnPrintf("RRN:   %s \n", PosCom.stTrans.szSysReferNo);
	
	PrnBlackEngData();
	prnPrintf("       Consulta de saldo     \n");

	PrnBigFontChnData();
	prnPrintf("LIMITE DE CREDITO $\n");	
	PrnBlackEngData();
	if( atol(T6)<0 )
	{
		prnPrintf("               %ld.%02ld\n",atol(T6)/100,atol(T6)*(-1)%100);
	}
	else
	{
		prnPrintf("               %ld.%02ld\n",atol(T6)/100,atol(T6)%100);
	}

	PrnBigFontChnData();
	prnPrintf("SALDO DISPONIBLE   $\n");	
	PrnBlackEngData();
	if( atol(T3)<0 )
	{
		prnPrintf("               %ld.%02ld\n",atol(T3)/100,atol(T3)*(-1)%100);
	}
	else
	{
		prnPrintf("               %ld.%02ld\n",atol(T3)/100,atol(T3)%100);
	}
	
	PrnBigFontChnData();
	prnPrintf("FECHA CORTE        :\n");
	PrnBlackEngData();
	prnPrintf("           %s\n",T0);

	PrnBigFontChnData();
	prnPrintf("FECHA ULTIMO PAGO  :\n");
	PrnBlackEngData();
	prnPrintf("           %s\n",T1);

	PrnBigFontChnData();
	prnPrintf("PAGO MINIMO       $\n");	
	PrnBlackEngData();
	if( atol(T2)<0 )
	{
		prnPrintf("               %ld.%02ld\n",atol(T2)/100,atol(T2)*(-1)%100);
	}
	else
	{
		prnPrintf("               %ld.%02ld\n",atol(T2)/100,atol(T2)%100);
	}
	

	PrnBigFontChnData();
	prnPrintf("SALDO VENTAS      $\n");	
	PrnBlackEngData();
	if( atol(T4)<0 )
	{
		prnPrintf("               %ld.%02ld\n",atol(T4)/100,atol(T4)*(-1)%100);
	}
	else
	{
		prnPrintf("               %ld.%02ld\n",atol(T4)/100,atol(T4)%100);
	}

	PrnBigFontChnData();
	prnPrintf("REFERENCIA         :\n");	
	PrnBlackEngData();
	prnPrintf("       %s\n",T5);

	PrnSmallFontData();
	prnPrintf("\n               %s\n\n",gstPosVersion.szVersion);
	prnPrintf("\n\n\n");
	PrintData();

	return OK;
}


uint8_t PrintFaildTicket(int flag,uint8_t *RSP_CODE,uint8_t *szErrorReason)
{
//	char  szPrnTitle[200];
	int iRet;
	char printDate[30] = {0};
	char szCardTemp[30];
	char buf1[200],buf2[200];
	
	lcdClrLine(2, 7);
  	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "IMPRIMIENDO...");
    lcdFlip();

	iRet = prnInit();
    if ( iRet!=OK )
    {
        lcdClrLine(2, 7);
        DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT PRINT FAIL");
        lcdFlip();
        kbGetKey();
        return NO_DISP;
    }

	if (stPosParam.ucPrnTitleFlag == PARAM_OPEN)
    {
      newprnLogo(85,0,200,65,(uint8_t *)Bmp_Prn);
    }

	prnPrintf("\n");	
    PrnBigFontChnData();
    prnStep(2);
	prnPrintf("%s\n", stPosParam.szMerchantName);
	prnPrintf("%s\n", stPosParam.szStreet);  // 地址
	prnPrintf("%s\n", stPosParam.szCity);  // city
	prnPrintf("\n");  
	
	PrnBigFontChnData();

	prnPrintf("%s\n", stPosParam.szST);  //store ID

	// 卡有效期
	if ( strlen((char *)PosCom.stTrans.szExpDate)==4 )
	{
	    if ( memcmp(PosCom.stTrans.szExpDate, "0000", 4)!=0 )
	    {
	      	memset(buf2, 0, sizeof(buf2));
	        if ( memcmp(PosCom.stTrans.szExpDate, "50", 2)>=0 )
	        {
	          memcpy((char*)buf2, "19", 2);
	        }
	        else
	        {
	          strcpy((char*)buf2, "20");
	        }
	        memcpy(buf2+2, PosCom.stTrans.szExpDate+2, 2);
	        buf2[4] = '/';
	        memcpy(buf2+5, PosCom.stTrans.szExpDate, 2);
	        buf2[7] = 0;
	     }
	}

	
	if( strlen((char*)PosCom.stTrans.szCardNo) == 0)
	{
		memcpy(buf1,"***********",sizeof("***********"));
	}
	else
	{
		memset(szCardTemp, 0, sizeof(szCardTemp));
	    MaskPan(PosCom.stTrans.szCardNo, (uint8_t *)szCardTemp);
	    MakeFormatCardNo((char *)szCardTemp, buf1);
	}

	prnPrintf("NUMERO DE CUENTA \n");  // 账号
	prnPrintf("%s%c %s\n",buf1,GetEntryMode(),buf2+2);  // 账号

	prnPrintf("TIEMPO\n"); 	
	MakeFormatPrintDate(PosCom.stTrans.szDate,(uint8_t *)printDate);
	prnPrintf("%s    %.2s:%.2s:%.2s\n",printDate, 
		&PosCom.stTrans.szTime[0], &PosCom.stTrans.szTime[2], &PosCom.stTrans.szTime[4]);

	GetTransPrtName(PosCom.stTrans.iTransNo, buf1); 

	if( PosCom.stTrans.iTransNo==PHONE_TOPUP )
	{
		prnPrintf("TIPO DE TRANSACCION:\n");
		//prnPrintf("            %s %s\n",buf1,(char*)PosCom.stTrans.PhoneComanyName);
	}
	else
	{
		prnPrintf("TIPO DE TRANSACCION:\n");
		prnPrintf("     %s\n",buf1);
	}
	
	prnPrintf("CARGO %06ld   TERMINAL:%s\n", PosCom.stTrans.lTraceNo,PosCom.stTrans.szPosId);//water NO
	prnPrintf("LOTE NUM %06ld\n",PosCom.stTrans.lBatchNumber);
	

	if( flag == 0 && strlen((char*)szErrorReason) != 0)
	{
		prnPrintf("\n%s %s\n",(char*)RSP_CODE,(char*)szErrorReason);
	}
	else
	{
		prnPrintf("  -1001SIN COMUNICATION  \n");
	}

	prnPrintf("  TRANSACCION DECLINADA  \n");

	PrnSmallFontData();
	prnPrintf("\n                %s\n",gstPosVersion.szVersion);
	prnPrintf("\n\n\n\n");
	PrintData();

	return OK;
}





// end of file
