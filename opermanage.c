
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "modem_iface.h"
#include "libsbTms.h"

/*static void   RemoteDownloadMenu(void);*/
//static int    SetRemoteCommParam(void);
//static void   ResetSysAfterRemoteDown(void);
//static void   DeleteRemoteTask(void);
/*static*/ void   SetupRemoteParam(void);

uint8_t szRemoteDownLoadTID[20]={0};  //代替stPosParam.szDownLoadTID，因为stPosParam.szDownLoadTID仅仅有9个字节，不能兼容12位的序列号

uint8_t TellerLogon(uint8_t flag,uint8_t ucOnline)
{
	uint8_t  buf[20], buf1[20],buf2[20]/*, nowOper[4]*/,DispPosition;
//	int      j;
	int      iRet;
	
	if (gstPosCapability.uiScreenType)
	{
		DispPosition = 115;
	}
	else
	{
		DispPosition = 72;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     LOGIN      ");
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "COMERCIO");

	lcdFlip();
	kbFlush();
	lcdGoto(DispPosition, 6);	
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT , 1, 15, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf1);
	if( iRet<0 ) 
	{
		return NO_DISP;
	}
	strcpy(stPosParam.szST,(char*)buf1);
	
/*	sprintf((char *)nowOper, "0%2s", buf1);
	if( strcmp((char *)nowOper, operStrc[0].szTellerNo)==0 )
	{
		SupTellerFunc();
		return NO_DISP;
	}
	if( strcmp((char *)nowOper, SYS_TELLER_NO)==0 )
	{
		SetParaFunc();
		return NO_DISP;
	}
	
	for(j=0; j<MAX_OPER_NUM; j++) 
	{
		if( strcmp((char *)nowOper, operStrc[j].szTellerNo)==0x00 ) 
		{
			break;
		}
	}
	if( j==MAX_OPER_NUM )
	{ 		
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "OPER NO EXIST");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return NO_DISP;
	}

#ifndef _POS_TYPE_8110
	if (ucOnline && ( (flag==0 && stTransCtrl.ucLogonFlag==LOGGEDOFF) || flag==1 ))
	{
		PreDial();
	}
#endif	
*/	
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "USUARIO");
	lcdFlip();
	lcdGoto(DispPosition-22, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 15, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet<0 )
	{
		return NO_DISP;
	}
	strcpy(stPosParam.szUser,(char*)buf);

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "CLAVE");
	lcdFlip();
	lcdGoto(DispPosition, 6);
	iRet = kbGetString(KB_EN_CRYPTDISP+KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 1, 15,
		/*kbGetString(KB_EN_REVDISP+KB_EN_SHIFTLEFT+KB_SMALL_PWD+KB_EN_CRYPTDISP, 1, 15,*/ 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf2);
	if( iRet<0 )
	{
		return NO_DISP;
	}
	strcpy(stPosParam.szPassword,(char*)buf2);
	
/*	if( strcmp((char *)buf, operStrc[j].szPassword) ) 
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "  PWD ERROR!  ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return NO_DISP;
	}
	strcpy((char *)stTransCtrl.szNowTellerNo, (char *)nowOper);
	strcpy((char *)stTransCtrl.szNowTellerPwd, (char *)operStrc[j].szPassword);
*/	
	return OK;
}


void TellerAdd()
{
	int  j;
	int  iRet;
	uint8_t  buf[20], buf1[20], buf2[20], buf3[20];
	uint8_t  szDisp[17],DispPosition;

	if (gstPosCapability.uiScreenType)
	{
		DispPosition = 160;
	}
	else
	{
		DispPosition = 72;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, "    ADD OPER    ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "INPUT MANAGE PWD");
	lcdFlip();
	lcdGoto(DispPosition, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=6 )
		return ;

	if( strcmp((char *)buf, operStrc[0].szPassword)!=0x00 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return ;
	}
		
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "OPER NO:");
	lcdFlip();
	kbFlush();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 2, 2, (stPosParam.ucOprtLimitTime*1000), (char *)buf1);
	if( iRet!=2 ) 
	{
		return ;
	}
	sprintf((char *)buf, "0%2s", buf1);

	if( strcmp(operStrc[0].szTellerNo, (char *)buf)==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SAME WITH MANAGE");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);

		return;
	}
	if( strcmp(SYS_TELLER_NO, (char *)buf)==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SAME WITH SYSTEM");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);

		return;
	}

	for(j=1; j<MAX_OPER_NUM; j++) 
	{
		if( strcmp(operStrc[j].szTellerNo, (char *)buf)==0 )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "OPER EXIST");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			return;
		}
	}

	sprintf((char *)szDisp, "OPER: %.2s", buf1);
	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, (char *)szDisp);
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "INPUT PWD:");
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf2);
	if( iRet!=4 )
		return ;

	sprintf((char *)szDisp, "OPER: %.2s", buf1);
	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, (char *)szDisp);
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "INPUT PWD:");
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf3);
	if( iRet!=4 )
		return;

	if( memcmp(buf2, buf3, 4)!=0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT, NULL, "NOT THE SAME");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}

	for(j=1; j<MAX_OPER_NUM; j++) 
	{
		if( strcmp(operStrc[j].szTellerNo, TELLER_NOBODY)==0 ) 
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT, NULL, "  ADD SUCC!  ");
			lcdFlip();
			OkBeep();
			strcpy(operStrc[j].szTellerNo, (char *)buf);
			strcpy(operStrc[j].szPassword, (char *)buf2);
			kbGetKeyMs(3000);
			WriteOperFile();
			return;
		}
	}

	if( j==MAX_OPER_NUM ) 
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "TOO MANY OPER ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);

		return;
	}

	return;	
}

void TellerDel()
{
	int j;
	int iRet;
	uint8_t buf[20], buf1[20];
	uint8_t szDisp[17],DispPosition;

	if (gstPosCapability.uiScreenType)
	{
		DispPosition = 160;
	}
	else
	{
		DispPosition = 72;
	}

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, "    DEL OPER    ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "INPUT MANAGE PWD");
	lcdFlip();
	lcdGoto(DispPosition, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=6 )
		return ;

	if( strcmp((char *)buf, operStrc[0].szPassword)!=0x00 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return ;
	}

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, "DELETE OPER");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "OPER NO:");
	lcdFlip();
	kbFlush();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 2, 2, (stPosParam.ucOprtLimitTime*1000), (char *)buf1);
	if( iRet!=2 ) 
	{
		return;
	}
	sprintf((char *)buf, "0%.2s", buf1);
	if( strcmp(SUP_TELLER_NO, (char *)buf)==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT DEL SUPER");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}
	else if( strcmp(SYS_TELLER_NO, (char *)buf)==0)
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT DEL ADMIN");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}
	
	if( strcmp((char *)stTransCtrl.szNowTellerNo, (char *)buf)==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " LOGONED ");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "DELETE FORBID");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}
	for(j=1; j<MAX_OPER_NUM; j++) 
	{
		if( strcmp(operStrc[j].szTellerNo, (char *)buf)==0 ) 
		{
			lcdCls();	
			DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, "    DEL OPER    ");
			sprintf((char *)szDisp, "OPER: %.2s", buf1);
			lcdDisplay(0, 2, DISP_CFONT, (char *)szDisp);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "DELETE?");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "ENTER CANCEL");
			lcdFlip();
			while(1)
			{
				iRet = kbGetKeyMs(30000);
				if( iRet==KEY_CANCEL || iRet==KEY_TIMEOUT )
					return;
				else if( iRet==KEY_ENTER )
					break;
			}
			strcpy(operStrc[j].szTellerNo, TELLER_NOBODY);
			strcpy((char *)operStrc[j].szPassword, "0000");
			WriteOperFile();
			break;
		}
	}

	if( j==MAX_OPER_NUM ) 
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "OPER NO EXIST");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);

		return;
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "DELETE SUCC");
	lcdFlip();
	OkBeep();
	kbGetKeyMs(3000);
	return;

}

void TellerShow()
{
	int  j, operNum=0;
	
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, " VIEW OPER INFO ");
	lcdFlip();
	lcdGoto(0, 2);

	for(j=1; j<MAX_OPER_NUM; j++) 
	{
		if( strcmp(operStrc[j].szTellerNo, TELLER_NOBODY)==0 ) 
		{
			continue;
		}
		lcdPrintf(" %.2s ", operStrc[j].szTellerNo+1); 
		operNum++;
		if( operNum%4==0 ) 
			lcdPrintf("\n");
		
		if( operNum%8==0 )
		{
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "MORE PRESS KEY");
			lcdFlip();
			kbGetKey();
			lcdClrLine(2, 7);	
			lcdGoto(0, 2);
		}
	}
	
	lcdFlip();
	kbGetKey();
	return;
}

void TellerManageMenu(void)
{
	int ikey;	
	const char *teller_man_main[] = 
	{
		"OPER MANAGE ",
		"1.CHG MANAG PWD",
		"2.ADD  3.DEL",
		"4.INQ  5.CHG PWD"
	};

	while(1) 
	{
		ikey = DispMenu(teller_man_main, 4, 1, KEY1, KEY5, stPosParam.ucOprtLimitTime);
		switch(ikey) 
		{
		case KEY1:
			SupChgPwd();
			break;
		case KEY2:
			TellerAdd();
			break;
		case KEY3:
			TellerDel();
			break;
		case KEY4:
			TellerShow();
			break;
		case KEY5:
			TellerChgPwd();
			break;

		default:
			return ;
			break;
		}
	}//while
	
	return ;
}

void TellerChgPwd()
{
	
	int     j;
	uint8_t buf[20], buf1[20], nowOper[4];	
	int iRet;
	uint8_t szDisp[17];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, "OPER CHANGE PWD ");
	sprintf((char *)szDisp, "OPER: %.2s", &stTransCtrl.szNowTellerNo[1]);
	lcdDisplay(0, 2, DISP_CFONT, (char *)szDisp);
	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "ENTER CANCEL");
	lcdFlip();
	while(1)
	{
		iRet = kbGetKeyMs(30000);
		if( iRet==KEY_ENTER )
			break;
		else if( iRet==KEY_CANCEL || iRet==KEY_TIMEOUT )
			return;
	}
	

	sprintf((char *)nowOper, "0%2s", &stTransCtrl.szNowTellerNo[1]);
	for(j=1; j<MAX_OPER_NUM; j++) 
	{
		if( strcmp((char *)nowOper, operStrc[j].szTellerNo)==0x00 ) 
		{
			break;
		}
	}
	if( j==MAX_OPER_NUM ) 
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " OPER NO EXIST ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "ORIGINAL PWD:");
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=4 )
		return;
	
	if( strcmp((char *)buf, operStrc[j].szPassword) ) 
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "   PWD ERROR! ");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);

		return;
	}
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NEW PWD:");
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=4 )
		return ;
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT AGAIN:");
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf1);
	if( iRet!=4 )
		return;
	if( strcmp((char *)buf, (char *)buf1) )  
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT THE SAME");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}
	strcpy((char *)operStrc[j].szPassword ,(char *)buf);
	WriteOperFile();
	if( strcmp((char *)stTransCtrl.szNowTellerNo, (char *)nowOper)==0 )
	{
		strcpy((char *)stTransCtrl.szNowTellerPwd, (char *)buf);
		SaveCtrlParam();
	}
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "  CHANGED SUCC! ");
	lcdFlip();
	OkBeep();
	kbGetKeyMs(3000);
	return;
	
}

void TellerRunLock()
{
	uint8_t buf[20], pass[20], nowOper[4];
	int iRet;

	kbFlush();
	lcdCls();
	DispMulLanguageString(0, 0, DISP_MEDIACY|DISP_INVLINE|DISP_CFONT, NULL, "LOCK THE SCREEN ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT SUPER");
	lcdFlip();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 2, 2, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=2 ) 
	{
		return ;
	}

	memset(nowOper, 0, sizeof(nowOper));
	sprintf((char *)nowOper, "0%2s", buf);
	if( strcmp((char *)nowOper, operStrc[0].szTellerNo)==0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT SUPER PWD:");
		lcdFlip();
		lcdGoto(72, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6, 
			(stPosParam.ucOprtLimitTime*1000), (char *)pass);
		if( iRet!=6 )
			return ;

		if( (memcmp((char *)pass, operStrc[0].szPassword, 6)!=0x00) ) 
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			return;
		}
	}
	else if( strcmp((char *)nowOper, (char *)stTransCtrl.szOldTellerNo)==0 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT OPER PWD:");
		lcdFlip();
		lcdGoto(0, 4);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
			(stPosParam.ucOprtLimitTime*1000), (char *)pass);
		if( iRet!=4 )
			return ;

		if( (strcmp((char *)pass, (char *)stTransCtrl.szNowTellerPwd)!=0x00) ) 
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);

			return;
		}

	}
	else
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NO CURRENT OPER");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return ;

	}


	if( (strcmp((char *)nowOper, operStrc[0].szTellerNo)==0) )
	{
		strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
		SaveCtrlParam();		
	}
	else
	{
		strcpy((char *)stTransCtrl.szNowTellerNo, (char *)stTransCtrl.szOldTellerNo);
		SaveCtrlParam();
		lcdSetIcon(ICON_LOCK, CLOSEICON);	
	}
	
	return;
}

uint8_t CheckSupPwd(uint8_t ucFlag)
{
	char  buf[20];
	int iRet;

	if ((stPosParam.ucEnterSupPwd==PARAM_CLOSE) && (ucFlag==0))
	{
		return OK;
	}
	while(1)
	{	
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT MANAGE PWD");
		lcdFlip();
		lcdGoto(72, 6);  
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet!=6 )
			return NO_DISP;

		if( stPosParam.TmsAdminPwdSwitch != PARAM_OPEN )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "CAN'T ENTER");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			return NO_DISP;
		}

		if( strcmp(buf, operStrc[0].szPassword)!=0x00 )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			continue;
		}
		return OK;
	}	
}

uint8_t CheckTellerPwd()
{
	char  buf[20];
	int iRet;
	
	while(1)
	{		
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT OPER PWD:");
		lcdFlip();
		lcdGoto(88, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 4, 4, 
			(stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet!=4 )
			return NO_DISP;
		
		if( strcmp((char *)buf, (char *)stTransCtrl.szNowTellerPwd)!=0x00 )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			continue;
		}
		return OK;
	}
}


void SupTellerFunc(void)
{
	uint8_t ret;
	int ikey;

	const char * teller_man_main[] = 
	{
		"  MANAGE FUNC   ",
		"1.ADD  2.DEL",
		"3.INQ 4.CHG PWD",
		"5.PABX     "
	};

	ret = CheckSupPwd(1);
	if( ret )
		return;
	
	while(1) 
	{
		ikey = DispMenu(teller_man_main, 4, 1, KEY1, KEY8, stPosParam.ucOprtLimitTime);
		switch(ikey) 
		{
		case KEY1:
			TellerAdd();
			break;
		case KEY2:
			TellerDel();
			break;
		case KEY3:
			TellerShow();
			break;
		case KEY4:
			SupChgPwd();
			break;
		case KEY5:
			PabxSetup(1);
			break;
		case KEY8:
//			RemoteDownloadMenu();
			break;
			
		default:
			return ;
			break;
		}
	}//while
	
	return ;
}

void SupChgPwd()
{
	uint8_t  buf[20], tmp[20];
	int  iRet;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " CHANGE MAG PWD ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT ORG PWD");
	lcdFlip();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=6 )
		return ;

	if( strncmp((char *)buf, (char *)stPosParam.szManagePwd,6)!=0x00 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return ;
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NEW PWD:");
	lcdFlip();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet!=6 )
		return;

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "INPUT PWD AGAIN");
	lcdFlip();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6, 
		(stPosParam.ucOprtLimitTime*1000), (char *)tmp);
	if( iRet!=6 )
		return;
	
	if( strcmp((char *)tmp, (char *)buf)!=0 )
	{
		lcdClrLine(2, 7);	
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT THE SAME");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(1000);
		return;
	}
	strcpy((char *)stPosParam.szManagePwd, (char *)buf);
	WriteOperFile();
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "CHG PWD SUCC");
	lcdFlip();
	kbGetKeyMs(2000);

	return;
}
#if 0
void RemoteDownloadMenu(void)
{
	int     iKey;
	const char *stRemoteMenu[] = 
	{
		"1.REMOTE UPDATE",
		"2.PARAM SET    ",
		"3.UPDATE APP   ",
		"4.DEL TASK     "			
	};
	
	while(1) 
	{
		iKey = DispMenu(stRemoteMenu, 4, 0, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		switch(iKey) 
		{
		case KEY1:
			if(stPosParam.stDownParamCommCfg.ucCommType!=stPosParam.stTxnCommCfg.ucCommType)
			{
#ifdef _POS_TYPE_8210
				if (wifi_power_control_force)
				{
					if(stPosParam.stTxnCommCfg.ucCommType==CT_WIFI && stPosParam.stDownParamCommCfg.ucCommType!=CT_WIFI)
					{
						wifi_power_control_force(0);
					}
					else if(stPosParam.stTxnCommCfg.ucCommType!=CT_WIFI && stPosParam.stDownParamCommCfg.ucCommType==CT_WIFI)
					{
						wifi_power_control_force(1);
					}
				}
#endif				
				CommOnHookMode(stPosParam.stTxnCommCfg.ucCommType);
				CommInitModule(&stPosParam.stDownParamCommCfg);
			}
			RemoteDownLoad();
			CommOnHook(FALSE);
			break;
		case KEY2:
			SetupRemoteParam();
			break;
		case KEY3:
//			ResetSysAfterRemoteDown();
			break;
		case KEY4:
//			DeleteRemoteTask();
			break;
		case KEY_CANCEL:
			if(stPosParam.stDownParamCommCfg.ucCommType!=stPosParam.stTxnCommCfg.ucCommType)
			{
#ifdef _POS_TYPE_8210
				if (wifi_power_control_force)
				{			
					if(stPosParam.stDownParamCommCfg.ucCommType==CT_WIFI && stPosParam.stTxnCommCfg.ucCommType!=CT_WIFI)
					{
						wifi_power_control_force(0);
					}
					else if(stPosParam.stDownParamCommCfg.ucCommType!=CT_WIFI && stPosParam.stTxnCommCfg.ucCommType==CT_WIFI)
					{
						wifi_power_control_force(1);
					}
				}
#endif					
				CommOnHookMode(stPosParam.stDownParamCommCfg.ucCommType);
				CommInitModule(&stPosParam.stTxnCommCfg);
			}	
			return;
		case KEY_UP:
		case KEY_ENTER:
			break;
		default:
			continue;
		}
	}	
}


void  ResetSysAfterRemoteDown(void)
{
	int iRet,iUpdateNum,iRemainNum;
	
	sysSetPowerKey(0);
	PubDisplayTitle(TRUE,"SYSTEM UPDATE");
	lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "UPDATING");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "NOT POWER OFF");
	lcdFlip();
	iRet =  tmsUpdate(TMS_PROMPT_CHN,&iUpdateNum,&iRemainNum);
	sysSetPowerKey(1500);
	if (iRet==0 && iUpdateNum >=1)
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "UPDATE (%d) TASK",iUpdateNum);
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PLS REBOOT");
		lcdFlip();
		PubWaitKey(1);
		sysReset();
	}
	else
	{
		if (iRet == -TMS_ERROR_NOTASK || iUpdateNum==0)
		{
			lcdClrLine(2,7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "NO TASK");
			lcdFlip();
			PubWaitKey(5);
			return ;
		}
		
		if (iRet < 0 )
		{
			lcdClrLine(2,7);
			lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "UPDATE FAIL");
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PLS RETRY");
			lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "ERR CODE:%d",iRet);
			lcdFlip();
			PubWaitKey(5);
			return ;
		}
	}
	
	return ;
}

void DeleteRemoteTask(void)
{
	int iRet;
	
	kbFlush();
	PubDisplayTitle(TRUE,"DELETE TASK");
	lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "DETELE DOWNLOAD");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "TASK?");
	lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "[CANCEL] [ENTER]");
	lcdFlip();
	if(!PubYesNo(0))
	{
		return ;
	}
	
	iRet = tmsRemoveTask(TMS_PROMPT_CHN);
	if (iRet < 0 )
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "DELETE FAIL");
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PLS RETRY");
		lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "ERR CODE:%d",iRet);
		lcdFlip();
		PubWaitKey(5);
	}
	else
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "DELETE SUCCEED");
		lcdFlip();
		PubWaitKey(5);
	}
	
	return;
}


void SetupRemoteParam(void)
{
	int iRet;
	char   szTID[17]; 
			
	lcdCls();
	lcdDisplay(0,0,DISP_CFONT|DISP_INVLINE|DISP_MEDIACY,"PARA SET");
	PubDispString("INPUT SN:",      DISP_LINE_CENTER|4);
	memset(szTID,0,sizeof(szTID));
	if (strlen((char *)stPosParam.szDownLoadTID) == 8)
		sprintf(szTID,"%s",stPosParam.szDownLoadTID);
	else
		sysReadSN(szTID);
	iRet = PubGetString(NUM_IN|ECHO_IN, 0, 8, (uint8_t *)szTID, stPosParam.ucOprtLimitTime,0);
	if (iRet != 0)
	{
		return;
	}
	if (strlen(szTID)==8)
	{
		sprintf((char *)stPosParam.szDownLoadTID,"%s",szTID);
	}
	else
	{
		memset(szTID,0,sizeof(szTID));
		sysReadSN(szTID);
		sprintf((char *)stPosParam.szDownLoadTID,"%s",szTID);
	}	
	SaveAppParam();
	
	lcdCls();
	lcdDisplay(0,0,DISP_CFONT|DISP_INVLINE|DISP_MEDIACY,"REMOTE DOWNLOAD");
	lcdFlip();
	iRet = SetRemoteCommParam();
	return;
}


void RemoteDownLoad(void)
{
	int iRet,ibacklight_set_time_bak;
	tmsctrl_t stDownCtrl;
	char   sTpdu[10]; 

	iRet = TransInit(DOWNLOAD);
	if( iRet!=0 )
	{
		return iRet;
	}

	lcdCls();
	lcdDisplay(0,0,DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "REMOTE DOWNLOAD ");
	
	memset(&stDownCtrl, 0x00, sizeof(stDownCtrl));
	stDownCtrl.protocol = TMS_PROTO_NEWTMS;
	stDownCtrl.prompt = TMS_PROMPT_CHN;
	stDownCtrl.enbreak = YES;
	stDownCtrl.specifiedtmk = NO;
	if (strlen((char *)stPosParam.szDownLoadTID) == 8)
		sprintf((char*)stDownCtrl.termID,"%s",stPosParam.szDownLoadTID);
	else
		sysReadSN(stDownCtrl.termID);
	lcdCls();
	lcdDisplay(0,0,DISP_CFONT|DISP_INVLINE|DISP_MEDIACY,"REMOTE DOWNLOAD");
	lcdFlip();
	switch (stPosParam.stDownParamCommCfg.ucCommType)
	{
	case CT_RS232:
		stDownCtrl.commtype = TMS_COMM_UART;
		stDownCtrl.commport = stPosParam.stDownParamCommCfg.stRS232Para.ucPortNo;
		break;
	case CT_GPRS:
	case CT_CDMA:
		if (stPosParam.stDownParamCommCfg.ucCommType==CT_CDMA)
			stDownCtrl.commtype = TMS_COMM_CDMA;
		else
			stDownCtrl.commtype = TMS_COMM_GPRS;		
		break;
	case CT_TCPIP:
	case CT_WIFI:
			stDownCtrl.commtype = TMS_COMM_GPRS;		
		break;
	case CT_MODEM:
		stDownCtrl.commtype = TMS_COMM_MDMASY; 
		memset(sTpdu,0,sizeof(sTpdu));
		PubAsc2Bcd((char *)stPosParam.szAsyncTpdu,strlen((char *)stPosParam.szAsyncTpdu),sTpdu);
		tmsSetModemHead(sTpdu,strlen((char *)stPosParam.szAsyncTpdu)/2);
		break;
	case CT_MODEMPPP:
		stDownCtrl.commtype = TMS_COMM_PPP; 
		break;
	default:
		lcdClrLine(2,7);
		Display2StringInRect(NULL, "NOT SUPPORT!");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(3);
		return;
		break;
	}
	
	PosCom.stTrans.iTransNo = DOWNLOAD;
	stTemp.iTransNo = DOWNLOAD;
	iRet = ConnectHost();
	if (iRet != 0)
	{
		lcdClrLine(2,7);
		Display2StringInRect(NULL, "CONNECT TMS ERR");
		lcdFlip();
		PubWaitKey(5);
		return;
	}

	if (stPosParam.stDownParamCommCfg.ucCommType != CT_RS232 )
	{
		stDownCtrl.commport = GetCurSocket();
	}

#ifdef _POS_TYPE_8210
	if(stPosParam.stDownParamCommCfg.ucCommType==CT_GPRS || stPosParam.stDownParamCommCfg.ucCommType==CT_CDMA)
	{
		stDownCtrl.commport = GetCurWirelessSocket();
	}
	else if(stPosParam.stDownParamCommCfg.ucCommType==CT_WIFI|| stPosParam.stDownParamCommCfg.ucCommType==CT_TCPIP)
	{
		stDownCtrl.commport = GetCurTcpSocket();
	}
#endif	

	sysGetbacklighttime(&ibacklight_set_time_bak);
	sysSetbacklighttime(-1);
	start_timer(30, 0);
    iRet = tmsDownload(&stDownCtrl);
	stop_timer();
	sysSetbacklighttime(ibacklight_set_time_bak);
	if (iRet<0)
	{
		lcdClrLine(2,7);
		Display2StringInRect(NULL, "DOWNLOAD FAILED");
		lcdFlip();
		PubWaitKey(5);
		return;
	}
	else
	{
		lcdClrLine(2,7);
		PubDispString("REMOTE SUCC", 3|DISP_LINE_CENTER);
		lcdFlip();
		PubWaitKey(1);
	}
//	ResetSysAfterRemoteDown();

	return ;
}
#endif

int  SetRemoteCommParam(void)//设置主控的参数，并且同步到主控
{
	POS_PARAM_STRC_MAIN pSaveParam;

	Get_Ser_Comm_Param(&pSaveParam);
	if(stPosParam.stTxnCommCfg.ucCommType==CT_MODEM)
	{
		SetModemPPPParam(&pSaveParam);
	}
	else if(stPosParam.stTxnCommCfg.ucCommType==CT_GPRS || stPosParam.stTxnCommCfg.ucCommType==CT_CDMA
		||stPosParam.stTxnCommCfg.ucCommType==CT_WIFI || stPosParam.stTxnCommCfg.ucCommType==CT_TCPIP)
	{
		SetManageTcpIpParam(&pSaveParam);
	}
	Set_Ser_Comm_Param(&pSaveParam);

	return OK;
}

int    SetRemoteModemType(void)
{
	int iKey, ucFlag;

	ucFlag = 1;
	while(1)
	{
		PubDisplayTitle(TRUE, (char *)"通讯类型");
		if (stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode == MODEM_COMM_ASYNC)
		{
			lcdDisplay(0, 2, DISP_CFONT, "=>Modem ASYNC");
			lcdDisplay(0, 4, DISP_CFONT, "  Modem PPP");
		}
		else
		{
			lcdDisplay(0, 2, DISP_CFONT, "  Modem ASYNC");
			lcdDisplay(0, 4, DISP_CFONT, "=>Modem PPP");
		}
		lcdFlip();
		
		iKey = PubWaitKey(30);
		switch (iKey)
		{
		case KEY_CANCEL:			
			return ERR_USERCANCEL;			
			break;
		case KEY_ENTER:	
			ucFlag = 0;
			break;
		case KEY1 :			       
			stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_ASYNC;
			break;
		case KEY2 :		
			stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_SYNC;
			break;			       
		case KEY_UP :
		case KEY_DOWN :
			if(stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode == MODEM_COMM_ASYNC)
			{
				stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_SYNC;
			}
			else
			{
				stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode = MODEM_COMM_ASYNC;
			}
			break;
		case KEY_TIMEOUT:
			return ERR_USERCANCEL;
			break;	
		default:
			break;
		}
		
		if (ucFlag==0)
		{
			break;
		}
	}
	
	return 0;
}


int    SetModemAsyncParam(void)
{
	int  iRet, iTemp, iCnt, iKey;
	char	szBuffer[20],buf[100];
	static	MenuItem stBateRateMenu[20];
	int   sBateRateList[20];
	
	memset(buf,0,sizeof(buf));
	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "COMM SETUP");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT PHONE:");
	strcpy((char *)buf, (char *)stPosParam.szDownloadTel);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_CHAR+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 20, 
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet>0 && strcmp((char *)stPosParam.szDownloadTel, (char *)buf))
	{
		strcpy((char *)stPosParam.szDownloadTel, (char *)buf);
	}
	
	PubDisplayTitle(TRUE, (char *)"COMM SETUP");
	memset(szBuffer,0,sizeof(szBuffer));
	if (strlen((char *)stPosParam.szAsyncTpdu)==0)
	{
		strcpy((char *)stPosParam.szAsyncTpdu,"6000030000");
	}
	strcpy(szBuffer,(char *)stPosParam.szAsyncTpdu);
	PubDispString("TPDU:", DISP_LINE_LEFT|2);
	iRet = PubGetString(ECHO_IN, 10, 10, (uint8_t*)szBuffer, stPosParam.ucOprtLimitTime,0);
	if(iRet !=0)
	{
		return E_TRANS_CANCEL;
	}
	strcpy((char *)stPosParam.szAsyncTpdu,szBuffer);

    stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.connect_mode = MODEM_COMM_ASYNC;
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.dial_mode = MODEM_DAIL_DTMF;
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.dial_pause = 1;
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.extension_chk = 1;
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.region = ModemRegion(USA);
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.dtmf_level = 10;
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.t_answertone = 10;
	stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.idle_timeo = 60;

	iTemp = stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.connect_speed;
	if (iTemp == 0)
	{
		iTemp = MODEM_CONNECT_33600BPS;
	}
	sBateRateList[14] = MODEM_CONNECT_1200BPS;		
	sBateRateList[13] = MODEM_CONNECT_2400BPS;
	sBateRateList[12] = MODEM_CONNECT_4800BPS;
	sBateRateList[11] = MODEM_CONNECT_7200BPS;
	sBateRateList[10] = MODEM_CONNECT_9600BPS;
	sBateRateList[9] = MODEM_CONNECT_12000BPS;
	sBateRateList[8] = MODEM_CONNECT_14400BPS;
	sBateRateList[7] = MODEM_CONNECT_16800BPS;
	sBateRateList[6] = MODEM_CONNECT_19200BPS;
	sBateRateList[5] = MODEM_CONNECT_21600BPS;
	sBateRateList[4] = MODEM_CONNECT_24000BPS;
	sBateRateList[3] = MODEM_CONNECT_26400BPS;
	sBateRateList[2] = MODEM_CONNECT_28800BPS;
	sBateRateList[1] = MODEM_CONNECT_31200BPS;
	sBateRateList[0] = MODEM_CONNECT_33600BPS;
	memset(stBateRateMenu,0,sizeof(stBateRateMenu));
	for (iCnt=0; iCnt<= 14; iCnt++)
	{
		stBateRateMenu[iCnt].bVisible = TRUE;
		if (sBateRateList[iCnt] == iTemp)
			strcpy(szBuffer,"=>");
		else
			strcpy(szBuffer,"  ");
		
		sprintf(stBateRateMenu[iCnt].szMenuName,"%s%d",szBuffer,sBateRateList[iCnt]);
		stBateRateMenu[iCnt].pfMenuFunc = NULL;
	}
	stBateRateMenu[iCnt].bVisible = FALSE;
	stBateRateMenu[iCnt].pfMenuFunc = NULL;
	stBateRateMenu[iCnt].szMenuName[0] = 0;
	sprintf(szBuffer,"BAUD RATE(%d)",iTemp);
	iKey = PubGetMenu((uint8_t *)szBuffer, stBateRateMenu, MENU_AUTOSNO|MENU_ASCII, stPosParam.ucOprtLimitTime);
	if((iKey>=0 && iKey < iCnt))
	{
		stPosParam.stDownParamCommCfg.stPSTNPara.stDialPara.connect_speed = sBateRateList[iKey];
		return 0;
	}
	
	return 0;
}

int    SetOtherParam(void)
{
	uint8_t buf[32], buf1[32];
	int     iTmpInt, iRet, iCnt;
	static	MenuItem stBateRateMenu[20];
	int   sBateRateList[20];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CONNECT TIME");
	sprintf((char *)buf,"%ld",stPosParam.stTxnCommCfg.ulSendTimeOut);
	sprintf((char *)buf1,"(20~90S): %ld",stPosParam.stTxnCommCfg.ulSendTimeOut);
	PubDispString(buf1,2|DISP_LINE_LEFT);
	iRet = PubGetString(ECHO_IN + NUM_IN, 1, 2,buf,stPosParam.ucOprtLimitTime,0  );
	iTmpInt = atoi((char *)buf);
	if(iRet == 0x00 && iTmpInt >= 20 && iTmpInt <= 90 &&stPosParam.stTxnCommCfg.ulSendTimeOut != iTmpInt )
	{
		stPosParam.stTxnCommCfg.ulSendTimeOut = iTmpInt;
		if(stPosParam.stTxnCommCfg.ulSendTimeOut < 30) stPosParam.stTxnCommCfg.ulSendTimeOut = 30;
	}

	if(gstPosCapability.uiCommType & (CT_GPRS|CT_CDMA))
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "PPP AUTHENTIC");
		iTmpInt = stPosParam.iPPPAuth;
		while(1)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"PPP AUTHENTIC");
			sprintf((char *)buf, "(%s)", iTmpInt==PPP_ALG_CHAP? "CHAP" : "PAP");
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,(char *)buf);
			lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"1.CHAP  0.PAP");
			lcdFlip();
			iRet = kbGetKeyMs(60000);
			if( iRet==KEY_CANCEL || iRet==KEY_INVALID || iRet == KEY_TIMEOUT )
			{
				return ERR_USERCANCEL;
			}
			else if( iRet== KEY_ENTER )
			{
				break;
			}
			else if ( iRet==KEY0 )
			{
				iTmpInt = PPP_ALG_PAP;
			}
			else if ( iRet==KEY1 )
			{
				iTmpInt = PPP_ALG_CHAP;
			}
		}
		stPosParam.iPPPAuth = iTmpInt;
	}
	
	if(( gstPosCapability.uiCommType & CT_MODEM )==0)
		return 0;
	
	PubDisplayTitle(TRUE,"MODEM PARA");

	iTmpInt = stPosParam.stTxnCommCfg.stPSTNPara.ucSendMode;
	while(1)
	{
		lcdClrLine(2, 7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"COMM MODE");
		sprintf((char *)buf, "(%s)", iTmpInt ? "SYNC" : "ASYNC");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,(char *)buf);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"1.SYNC  0.ASYNC");
		lcdFlip();
		iRet = kbGetKeyMs(60000);
		if( iRet==KEY_CANCEL || iRet==KEY_INVALID || iRet == KEY_TIMEOUT )
		{
			return ERR_USERCANCEL;
		}
		else if( iRet== KEY_ENTER )
		{
			break;
		}
		else if ( iRet==KEY0 )
		{
			iTmpInt = MODEM_COMM_ASYNC;
		}
		else if ( iRet==KEY1 )
		{
			iTmpInt = MODEM_COMM_SYNC;
		}
	}
	stPosParam.stTxnCommCfg.stPSTNPara.ucSendMode = iTmpInt;

	PubDisplayTitle(TRUE,"MODEM PARA");
	iTmpInt = stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_speed;
	if (iTmpInt == 0)
	{
		iTmpInt = MODEM_CONNECT_1200BPS;
	}
	sBateRateList[0] = MODEM_CONNECT_1200BPS;		
	sBateRateList[1] = MODEM_CONNECT_2400BPS;
	sBateRateList[2] = MODEM_CONNECT_4800BPS;
	sBateRateList[3] = MODEM_CONNECT_7200BPS;
	sBateRateList[4] = MODEM_CONNECT_9600BPS;
	sBateRateList[5] = MODEM_CONNECT_12000BPS;
	sBateRateList[6] = MODEM_CONNECT_14400BPS;
	sBateRateList[7] = MODEM_CONNECT_16800BPS;
	sBateRateList[8] = MODEM_CONNECT_19200BPS;
	sBateRateList[9] = MODEM_CONNECT_21600BPS;
	sBateRateList[10] = MODEM_CONNECT_24000BPS;
	sBateRateList[11] = MODEM_CONNECT_26400BPS;
	sBateRateList[12] = MODEM_CONNECT_28800BPS;
	sBateRateList[13] = MODEM_CONNECT_31200BPS;
	sBateRateList[14] = MODEM_CONNECT_33600BPS;
	memset(stBateRateMenu,0,sizeof(stBateRateMenu));
	for (iCnt=0; iCnt<= 14; iCnt++)
	{
		stBateRateMenu[iCnt].bVisible = TRUE;
		if (sBateRateList[iCnt] == iTmpInt)
			strcpy((char *)buf,"=>");
		else
			strcpy((char *)buf,"  ");
		
		sprintf(stBateRateMenu[iCnt].szMenuName,"%s%d",buf,sBateRateList[iCnt]);
		stBateRateMenu[iCnt].pfMenuFunc = NULL;
	}
	stBateRateMenu[iCnt].bVisible = FALSE;
	stBateRateMenu[iCnt].pfMenuFunc = NULL;
	stBateRateMenu[iCnt].szMenuName[0] = 0;
	sprintf((char *)buf,"BAUD RATE(%d)",iTmpInt);
	iRet = PubGetMenu((uint8_t *)buf, stBateRateMenu, MENU_AUTOSNO|MENU_ASCII, stPosParam.ucOprtLimitTime);
	if((iRet>=0 && iRet < iCnt))
	{
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.connect_speed = sBateRateList[iRet];
	}
	
	PubDisplayTitle(TRUE,"MODEM PARA");
	iTmpInt = stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_mode;
	while(1)
	{
		lcdClrLine(2, 7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"DAIL TYPE");
		sprintf((char *)buf, "(%s)", iTmpInt ? "PULSE" : "DTMF");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,(char *)buf);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"0.DTMF 1.PULSE");
		lcdFlip();
		iRet = kbGetKeyMs(60000);
		if( iRet==KEY_CANCEL || iRet==KEY_INVALID || iRet == KEY_TIMEOUT )
		{
			return ERR_USERCANCEL;
		}
		else if( iRet== KEY_ENTER )
		{
			break;
		}
		else if ( iRet==KEY0 )
		{
			iTmpInt = MODEM_DAIL_DTMF;
		}
		else if ( iRet==KEY1 )
		{
			iTmpInt = MODEM_DAIL_PULSE;
		}
	}
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_mode = iTmpInt;

	while(1)
	{
		lcdClrLine(2, 7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"PAUSE TIME:");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"  %ld",stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_pause);
		lcdFlip();
		lcdGoto(80, 6);
		memset(buf,0,sizeof(buf));
		iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 3, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return (E_TRANS_CANCEL);
		iTmpInt = atoi((char *)buf);
		if (iTmpInt ==0 )
		{
			break;
		}
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_pause = iTmpInt;
		break;
	}

	while( 1 )
	{
		lcdClrLine(2, 7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TIMEOUT");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"  %ld",stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_timeo);
		lcdFlip();
		lcdGoto(80, 6);
		memset(buf,0,sizeof(buf));
		iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 3, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return (E_TRANS_CANCEL);
		iTmpInt = atoi((char *)buf);
		if (iTmpInt ==0 )
		{
			break;
		}
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.dial_timeo = iTmpInt;
		break;
	}

	while(1)
	{
		lcdClrLine(2, 7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"FREE TIMEOUT");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"  %ld",stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.idle_timeo);
		lcdFlip();
		lcdGoto(80, 6);
		memset(buf,0,sizeof(buf));
		iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 3, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
			return (E_TRANS_CANCEL);
		iTmpInt = atoi((char *)buf);
		if (iTmpInt ==0 )
		{
			break;
		}
		stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.idle_timeo = iTmpInt;
		break;
	}

	iTmpInt = stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.extension_chk;
	while( 1 )
	{
		lcdClrLine(2, 7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"CHECK LINE");
		sprintf((char *)buf, "(%s)", iTmpInt ? "YES" : "NO");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,(char *)buf);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"0.YES   1.NO");
		lcdFlip();
		iRet = kbGetKeyMs(60000);
		if( iRet==KEY_CANCEL || iRet==KEY_INVALID || iRet == KEY_TIMEOUT  )
		{
			return ERR_USERCANCEL;
		}
		else if( iRet== KEY_ENTER )
		{
			break;
		}
		else if ( iRet==KEY0 )
		{
			iTmpInt = 1;
			break;
		}
		else if ( iRet==KEY1 )
		{
			iTmpInt = 0;
			break;
		}
	}
	stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.extension_chk = iTmpInt;


	iTmpInt = stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.region;
	memset(buf, 0, sizeof(buf));
	lcdClrLine(2, 7);
	PubDispString("COUNTRY CODE", DISP_LINE_CENTER|2);
	sprintf((char *)buf, "(%s)", iTmpInt ? "YES" : "NO");
	PubDispString(buf,       DISP_LINE_CENTER|4);
	PubDispString("1.YES  0.NO", DISP_LINE_CENTER|6);
	lcdFlip();
	while(1)
	{
		iRet = PubWaitKey(stPosParam.ucOprtLimitTime);
		if( iRet==KEY_CANCEL || iRet==KEY_INVALID || iRet == KEY_TIMEOUT  )
		{
			return ERR_USERCANCEL;
		}
		else if( iRet==KEY_ENTER )
		{
			break;
		}
		else if ( iRet==KEY0 )
		{
			stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.region = 0;
			break;
		}
		else if ( iRet==KEY1 )
		{
			stPosParam.stTxnCommCfg.stPSTNPara.stDialPara.region = ModemRegion(USA);
			break;
		}
	}

	return 0;
}

void    SetQuitTimeout(void)
{
	int iKeyVal,iCnt=0;

	while(1)
	{
		iKeyVal = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if (iKeyVal==KEY_TIMEOUT || iKeyVal==KEY_INVALID)
		{
			if (iCnt==0)
			{
				CommHangUp(FALSE);
				iCnt++;
			}
			else
			{
				continue;
			}
		}
		else
		{
			return;
		}
	}
}
// end of file
