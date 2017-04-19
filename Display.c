
/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
#include "Display.h"
#include "glbvar.h"
#include "tranfunclist.h"

/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


void   Display2Strings(char *pszString1, char *pszString2)
{
	lcdClrLine(2, 7);	
	PubDisplayMsg(3, (uint8_t *)pszString2);
	DrawRect(0, 17, 127, 63);
}

void   Display2StringInRect(char *pszString1,  char *pszString2)
{
	Display2Strings(pszString1, pszString2);
	DrawRect(0, 17, 127, 63);
}

void DisplayProcessing(void)
{
	lcdClrLine(2, 7);
	PubDispString(" PROCESSING....", 3|DISP_LINE_LEFT);
	DrawRect(0, 17, 127, 63);
}

void DisplayConnectCenter(void)
{
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "CONNECT BANK");
}

void DispBalAmt(uint8_t *pBalance1,uint8_t *pBalance2)
{
	uint8_t szBuf[17];
	int     iLen;
	
	lcdClrLine(2,7);
	if(stPosParam.ucSupPinpad==PARAM_OPEN)
	{
		DispMulLanguageString(0, 2, DISP_MEDIACY|DISP_CFONT, NULL, " QUERY SUCCESS  ");
		DispMulLanguageString(0, 4, DISP_CLRLINE|DISP_MEDIACY|DISP_CFONT, NULL, "   SEE  PINPAD  ");
		if (PosCom.ucSwipedFlag == CARD_INSERTED || PosCom.ucSwipedFlag == CARD_PASSIVE)
		{
			DispMulLanguageString(0, 6, DISP_CLRLINE|DISP_MEDIACY|DISP_CFONT, "  Çë°Î¿¨»òÄÃ¿¨  ", "PLS REMOVE CARD");
		}
		lcdFlip();
		memset(szBuf, 0x20, 14);
		iLen = strlen((char *)pBalance1);
		memcpy(&szBuf[14-iLen-1], pBalance1, iLen);
		szBuf[13] = 0;
		
		EppLight(2,0);
		EppClearScreen();
		EppDisplayLogo(0,0,Bmp_Show_Balance);
		EppDisplayString(0, 2, 16, szBuf, strlen((char *)szBuf));
		EppDisplayLogo(strlen((char *)szBuf)*8, 16, Bmp_Show_CurrName);
		kbFlush();
		kbGetKeyMs(5000);
		EppLight(3,0);
		EppRestoreDefaultIdleLogo();
	}
	else
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_INVLINE|DISP_MEDIACY|DISP_CFONT, NULL, " QUERY SUCCESS  ");
		memset(szBuf, 0x20, 16);
		if (PosCom.ucSwipedFlag == CARD_INSERTED || PosCom.ucSwipedFlag == CARD_PASSIVE)
		{
			DispMulLanguageString(0, 6, DISP_CLRLINE|DISP_MEDIACY|DISP_CFONT, "  Çë°Î¿¨»òÄÃ¿¨  ", "PLS REMOVE CARD");
		}
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "BALANCE:");
		strcpy((char *)(szBuf+14),"Ôª");		
		iLen = strlen((char *)pBalance1);
		memcpy(&szBuf[14-iLen], pBalance1, iLen);
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, (char *)szBuf);
		lcdFlip();
		PubBeepOk();
		kbFlush();
		kbGetKeyMs(5000);
	}
}



// END OF FILE



