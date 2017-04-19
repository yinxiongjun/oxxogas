
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "wireless.h"
#include "emvapi.h"
#include "tmslib.h"
#include "New8210.h"


//TMS
#include "sapptms.h"
#include "libsbTms.h"

int SetTpdu(void);

// set system param
void SetupPosPara(void)
{
	int iKey;

/*	const char *setupMenu[] =
	{
		"CONFIGURACION",
		"1.MERCHANT PARAM",
		"2.TRANS PARAM",
		"3.SYSTEM PARAM",
		"4.COMMU PARAM",
		"5.Salir",
		"6.PASSWORD",
		"7.OTHERS PARAM",
	};
	const char *setupMenu[] =
	{
		"CONFIGURACION",
		"1.Parametros del comercio",
		"2.Parametros de la transaccion",
		"3.Parametros del sistema",
		"4.Parametros de conexion",
		"5.Salir",
		"6.Contrasena",
		"7.Otros parametros",
	};
*/
	const char *setupMenu[] =
	{
		"CONFIGURACION",
		"1-PARAMETROS",
		"2-CAMBIAR MEDIO",
		"3-CONF BROWSER",
		"4-REP CONFIG",
	};

	while(1)
	{
		//iKey = DispMenu(setupMenu, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		iKey = DispMenu2(setupMenu, 5, 1,KEY1, KEY4,stPosParam.ucOprtLimitTime);
		switch(iKey)
		{
		case KEY1:
			SetupTerminalPara();
			break;
		case KEY2:
			SetupAppSwitch();
			SetupSysPara();
			break;
		case KEY3:
			SetCommParam(TRANS_COMMTYPE);
			break;
		case KEY4:
			SetLogonOff();
//			SetSupportPinpad();
//			SetMasterKeys();
			
			break;
		case KEY5:
			ChangePwdFunc();
			break;
		case KEY6:
			OtherFunc();
			break;
		case KEY7:
//			SetOtherParam();
			break;
		case KEY_CANCEL:
		case KEY_TIMEOUT:
			return;
		case KEY_UP:
		case KEY_ENTER:
			break;
		default:
			continue;
		}
		SaveAppParam();
		SaveCtrlParam();
	}
}

void SetupTranFunc(void)
{
	int   iKey;
	const char *setupMenu[] =
	{
		"Para de la trans",
		"1.TRANS PIN",
		"2.SWIPE ON/OFF",
		"3.SETTLE OPT",
		"4.OFFLINE OPT",
		"5.OTHERS OPT",
		"6.ELEC SIGN"
	};

	while(1)
	{
		iKey = DispMenu1(setupMenu, 7, 1, KEY1, KEY6, stPosParam.ucOprtLimitTime);
		switch(iKey)
		{
		case KEY1:
			SetVoidPin();
			break;
		case KEY2:
			SetVoidSwipe();
			break;
		case KEY3:
			SetSettleSwitch();
			break;
		case KEY4:
			SetOfflineSwitch();
			break;
		case KEY5:
			SetOtherSwitch();
			break;
#ifndef _POS_TYPE_8110
		case KEY6:
			SetElecSignSwitch();
			break;
#endif
		case KEY_CANCEL:
		case KEY_TIMEOUT:
			return;
		case KEY_UP:
		case KEY_ENTER:
			break;
		default:
			continue;
		}
	}
}

void ChangePwdFunc(void)
{
	int   iKey;
	const char *setupMenu[] =
	{
		"Contrasena",
		"1.MANAGER PWD",
		"2.SAFE PWD",
		"3.CHG MANAG PWD"
	};


	while(1)
	{
		iKey = DispMenu3(setupMenu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
		switch(iKey)
		{
		case KEY1:
			ChangeSysPwd();
			break;
		case KEY2:
			ChangeSafePwd();
			break;
		case KEY3:
			SupChgPwd();
			break;
		case KEY_CANCEL:
			return;
		case KEY_UP:
		case KEY_ENTER:
			break;
		default:
			continue;
		}
	}
}

void setPrnTitle(void)
{
	int     iKey;
	int     iRet;
	uint8_t buf[61],  buf2[2];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " SETUP PRN TITLE ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "Receipt Title");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "0-ENG 1-LOGO");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucPrnTitleFlag;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucPrnTitleFlag = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucPrnTitleFlag = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	if (stPosParam.ucPrnTitleFlag == PARAM_CLOSE)
	{
		kbFlush();
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " SETUP PRN TITLE ");
		DispMulLanguageString(0, 2, DISP_CFONT, (char *)stPosParam.szPrnTitle, (char *)stPosParam.szPrnTitle);
		DispMulLanguageString(0, 4, DISP_CFONT, NULL, "0-INPUT 1-DEFAULT");
		lcdFlip();
		do {
			iKey = kbGetKey();
		} while( iKey!=KEY_CANCEL && iKey!=KEY_ENTER && iKey!=KEY0 && iKey!=KEY1);
		if( iKey==KEY0 )
		{
			kbFlush();
			memset(buf, 0, sizeof(buf));
			lcdGoto(0, 2);
			iRet = kbGetHZ(0, 20, 0, (char *)buf);
			if( iRet>0 && strcmp((char *)stPosParam.szPrnTitle, (char *)buf) )
			{
				strcpy((char *)stPosParam.szPrnTitle, (char *)buf);
			}
		}
		else if (iKey == KEY1)
		{
			strcpy((char *)stPosParam.szPrnTitle,"POS SLIP");
		}
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " SETUP PRN TITLE ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "SERVICE No.:  ");
	strcpy((char *)buf, (char *)stPosParam.szMPhone);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(0, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 14,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	if( iRet>0 )
	{
		strcpy((char *)stPosParam.szMPhone, (char *)buf);
	}
}

void DownLoadFunction(void)
{
	const char *manmenu[]={
			"1.ECHO TEST",
			"2.DOWN PARA",
			"3.DOWN CAPK",
			"4.DOWN EMV PARA"
	};
	int key;
	uint8_t ucRet;


	while(1)
	{
		key = DispMenu(manmenu, 4, 0, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		switch(key) {
		case KEY1:
			TestResponse();
			break;
		case KEY2:
			ucRet = PosDownLoadParam();
			CommHangUp(FALSE);
			DispResult(ucRet);
			break;
		case KEY3:
			ucRet = DownLoadCAPK(FALSE);
			CommHangUp(FALSE);
			DispResult(ucRet);
			break;
		case KEY4:
			ucRet = DownLoadEmvParam(FALSE);
			CommHangUp(FALSE);
			DispResult(ucRet);
			break;
		case KEY_CANCEL:
			return;
		case KEY_UP:
		case KEY_ENTER:
			break;
		default:
			continue;
		}
	}
}

void OtherFunc(void)
{
	int iKey;
/*	const char *setupMenu[] =
	{
			"Otros parametros",
			"1.CLEAR SETUP",
			"2.RECEIPT",
			"3.DOWNLOAD FUNC",
			"4.PRINT PARA",
			"5.HW TEST",
			"6.POS STATUS",
			"7.REMOTE DOWNDOLA"
	};
*/
	const char *setupMenu[] =
	{
			"Otros parametros",
			"1.CLEAR SETUP",
			"2.RECEIPT",
			"3.PRINT PARA",
			"4.HW TEST",
			"5.POS STATUS",
			"6.REMOTE DOWNDOLA"
	};

	const char *downloadMenu[] = 
	{
		"    REMOTE DOWNDOLA   ",
		"1 PARAM SETUP      ",
		"2 VERSION UPDATE      "
	};

	while(1)
	{
		iKey = DispMenu1(setupMenu, 7, 1, KEY0, KEY6, stPosParam.ucOprtLimitTime);
		switch(iKey) 
		{
		case KEY1:
			SetClear();
			break;
		case KEY2:
			setPrnTitle();
			break;
/*		case KEY3:
			DownLoadFunction();
			break;
*/		case KEY3:
			PrintParam(1);
			break;
		case KEY4:
			DeviceTest();
			break;
		case KEY5:
			SetTestStatus();
			break;
		/*case KEY8:
			SetTransFlow();
			break;*/
		case KEY6:
			iKey = DispMenu1(downloadMenu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
			if( iKey==KEY1 )
				SetRemoteCommParam();
			else if( iKey==KEY2 )
				down_now();
			else 
				continue;
			break;	
		case KEY0:
			SetTransFlow();
			SetEMVQPBOC();
			break;
		case KEY_CANCEL:
		case KEY_ENTER:
			return;
		case KEY_UP:
			break;
		default:
			continue;
		}
	}
}

void SetupTerminalPara(void)
{
	int     iRet;
	uint8_t buf[61];
	uint8_t szpwd[10];

	kbFlush();
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, NULL, "Paras del comercio");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS INPUT MER NO");
	strcpy((char *)buf, (char *)stPosParam.szUnitNum);
	lcdDisplay(8, 4, DISP_CFONT, (char*)buf);
	iRet = PubGetString(ALPHA_IN,15, 15,buf,stPosParam.ucOprtLimitTime,1);
	if(strlen((char*)buf)>0 && strcmp((char *)stPosParam.szUnitNum, (char *)buf))
	{
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "INPUT SAFE PWD");
		lcdFlip();
		iRet = PubGetString(NUM_IN|PASS_IN,6,6,szpwd,stPosParam.ucOprtLimitTime,0);
		if( iRet==0 )
		{
			if( strcmp((char *)szpwd, (char *)stPosParam.szSafepwd)!=0 )
			{
				lcdClrLine(2, 7);
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PWD ERROR!");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(1000);
			}
			else
			{
				if( stTransCtrl.iTransNum!=0 || fileExist(REVERSAL_FILE)>=0 || fileSize(ICC_FAILUR_TXN_FILE)>0 || fileExist(ICC_SCRIPT_FILE)>=0)
				{
					lcdClrLine(2, 7);
					DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "SETTLE NOW!");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000);
					return;
				}
				strcpy((char *)stPosParam.szUnitNum, (char *)buf);
				strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
				stTransCtrl.ucLogonFlag=LOGGEDOFF;
/*				stTransCtrl.bEmvAppLoaded = FALSE;
				stTransCtrl.bEmvCapkLoaded=FALSE;
				emv_delete_all_terminal_aids();
				emv_delete_all_ca_public_keys();
				fileRemove("emv.aid");
				fileRemove(TERM_AIDLIST);
				fileRemove("emv.capk");
*/			}
		}
	}

	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS INPUT TEM NO");
	strcpy((char *)buf, (char *)stPosParam.szMechId);
	lcdDisplay(64, 4, DISP_CFONT, (char *)buf);
	iRet = PubGetString(ALPHA_IN,8, 8,buf,stPosParam.ucOprtLimitTime,1);
	if(strlen((char *)buf)>0 && strcmp((char *)stPosParam.szMechId, (char *)buf))
	{
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "INPUT SAFE PWD");
		lcdFlip();
		iRet = PubGetString(NUM_IN|PASS_IN,6,6,szpwd,stPosParam.ucOprtLimitTime,0);
		if( iRet==0 )
		{
			if( strcmp((char *)szpwd, (char *)stPosParam.szSafepwd)!=0 )
			{
				lcdClrLine(2, 7);
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "PWD ERROR!");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(1000);
			}
			else
			{
				if( stTransCtrl.iTransNum!=0 || fileExist(REVERSAL_FILE)>=0 || fileSize(ICC_FAILUR_TXN_FILE)>0 || fileExist(ICC_SCRIPT_FILE)>=0)
				{
					lcdClrLine(2, 7);
					lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "SETTLE NOW!");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(3000);
					return;
				}
				strcpy((char *)stPosParam.szMechId, (char *)buf);
				strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
				stTransCtrl.ucLogonFlag=LOGGEDOFF;
				stTransCtrl.bEmvAppLoaded = FALSE;
				stTransCtrl.bEmvCapkLoaded=FALSE;
				emv_delete_all_terminal_aids();
				emv_delete_all_ca_public_keys();
				fileRemove("emv.aid");
				fileRemove(TERM_AIDLIST);
				fileRemove("emv.capk");
			}
		}
	}

	strcpy((char *)buf, (char *)stPosParam.szEngName);
	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT MER NAME");
	lcdFlip();
	lcdGoto(0, 4);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT+KB_EN_PRESETSTR, 0, 30,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	if( iRet>0 && strcmp((char *)stPosParam.szEngName, (char *)buf))
	{
		strcpy((char *)stPosParam.szEngName, (char *)buf);
		emv_get_mck_configure(&tConfig);
		memcpy(tEMVCoreInit.merchant_name_loc, stPosParam.szEngName, 28);
		emv_set_core_init_parameter(&tEMVCoreInit);
	}
}

void SetLogonOff(void)
{
	int     iRet;
	uint8_t buf2[2];
	
	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT, "Salir");
	lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
    memset(buf2, 0, sizeof(buf2));
//	buf2[0] = stPosParam.szLogonFlag;
	iRet = SelectSetTwo(buf2);
	if( iRet==1 )
	{
		stPosParam.szLogonFlag = LOGGEDOFF;
		memset(stPosParam.szUser,0,sizeof(stPosParam.szUser));
		memset(stPosParam.szPassword,0,sizeof(stPosParam.szPassword));
		memset(stPosParam.szST,0,sizeof(stPosParam.szST));
		SaveAppParam();
	}
	else if( iRet<0 )
	{
		return ;
	}

	return ;
}


void SetupAppSwitch(void)
{
	int     iRet;
	uint8_t buf2[2];
	
	lcdCls();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, NULL, "Configuracion del sitema");
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);


	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "Retail");
	lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.Switch_Retail;
	iRet = SelectSetTwo(buf2);
	if( iRet==1 )
	{
		stPosParam.Switch_Retail = PARAM_OPEN;
		stPosParam.Switch_Hotel = PARAM_CLOSE;
		stPosParam.Switch_Restaurant = PARAM_CLOSE;

		stPosParam.szpreAuth = PARAM_CLOSE;
		stPosParam.szpreTip = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet==0 )
	{
		stPosParam.Switch_Retail = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet<0 )
	{
		return ;
	}

	

	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "Hotel");
	lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.Switch_Hotel;
	iRet = SelectSetTwo(buf2);
	if( iRet==1 )
	{
		stPosParam.Switch_Hotel = PARAM_OPEN;
		stPosParam.Switch_Retail = PARAM_CLOSE;
		stPosParam.Switch_Restaurant = PARAM_CLOSE;

		stPosParam.szpreAuth = PARAM_CLOSE;
		stPosParam.szpreTip = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet==0 )
	{
		stPosParam.Switch_Hotel = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet<0 )
	{
		return ;
	}

	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "Restaurant");
	lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.Switch_Restaurant;
	iRet = SelectSetTwo(buf2);
	if( iRet==1 )
	{
		stPosParam.Switch_Restaurant = PARAM_OPEN;
		stPosParam.Switch_Hotel = PARAM_CLOSE;
		stPosParam.Switch_Retail = PARAM_CLOSE;

		stPosParam.szpreAuth = PARAM_CLOSE;
		stPosParam.szpreTip = PARAM_OPEN;
		SaveAppParam();
	}
	else if( iRet==0 )
	{
		stPosParam.Switch_Restaurant = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet<0 )
	{
		return ;
	}

	#if 0
	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "USD");
	lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.Switch_USD;
	iRet = SelectSetTwo(buf2);
	if( iRet==1 )
	{
		stPosParam.Switch_USD = PARAM_OPEN;
		stPosParam.Switch_Hotel = PARAM_CLOSE;
		stPosParam.Switch_Restaurant = PARAM_CLOSE;
		stPosParam.Switch_Retail = PARAM_CLOSE;
		stPosParam.Switch_Branch = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet==0 )
	{
		stPosParam.Switch_USD = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet<0 )
	{
		return ;
	}
	#endif

	#if 0
	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "Branch");
	lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.Switch_Retail;
	iRet = SelectSetTwo(buf2);
	if( iRet==1 )
	{
		stPosParam.Switch_Branch  = PARAM_OPEN;
		stPosParam.Switch_Hotel = PARAM_CLOSE;
		stPosParam.Switch_Restaurant = PARAM_CLOSE;
		stPosParam.Switch_USD = PARAM_CLOSE;
		stPosParam.Switch_Retail = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet==0 )
	{
		stPosParam.Switch_Retail = PARAM_CLOSE;
		SaveAppParam();
	}
	else if( iRet<0 )
	{
		return ;
	}
	#endif

	
}


void SetupSysPara(void)
{
	int     iRet;
	uint8_t buf[61], buf2[2];
	uint8_t szTime[15];
	uint8_t szDate[9];
	uint8_t szDateTime[7];
	long    lTmpLong;

	lcdCls();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, NULL, "Configuracion del sitema");
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "NUMERO DE CARGO:");
	sprintf((char *)buf, "%06d", stPosParam.lNowTraceNo);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	
	lcdGoto(72, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	lTmpLong = atol((char *)buf);
	if( iRet>0 && stPosParam.lNowTraceNo!=lTmpLong
			&& lTmpLong>0)
	{			
		stPosParam.lNowTraceNo = lTmpLong;
	}


	//容量控制
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 2, DISP_CFONT, "VOLUM CTR");
		lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
	    	memset(buf2, 0, sizeof(buf2));
		buf2[0] = stPosParam.stVolContrFlg;
		iRet = SelectSetTwo(buf2);
		if( iRet==1 )
		{
			stPosParam.stVolContrFlg = PARAM_OPEN;
			//stPosParam.szpreTip = PARAM_OPEN;
			SaveAppParam();
		}
		else if( iRet==0 )
		{
			stPosParam.stVolContrFlg = PARAM_CLOSE;
			stPosParam.szpreTip = PARAM_CLOSE;
			SaveAppParam();
		}
		else if( iRet<0 )
		{
			return ;
		}
	}
	if( stPosParam.stVolContrFlg==PARAM_OPEN  )
	{
		lcdClrLine(2,7);
		lcdDisplay(0, 2, DISP_CFONT, "PRE TIP");
		lcdDisplay(0, 4, DISP_CFONT, "0-No 1-Yes");
	    	memset(buf2, 0, sizeof(buf2));
		buf2[0] = stPosParam.szpreTip;
		iRet = SelectSetTwo(buf2);
		if( iRet==1 )
		{
			stPosParam.szpreTip = PARAM_OPEN;
			SaveAppParam();
		}
		else if( iRet==0 )
		{
			stPosParam.szpreTip = PARAM_CLOSE;
			SaveAppParam();
		}
		else if( iRet<0 )
		{
			return ;
		}
	}
	
	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "NUMERO DE LOTE:");
	sprintf((char *)buf, "%06d", stPosParam.lNowBatchNum);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(72, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	lTmpLong = atol((char *)buf);
	if( iRet>0 && stPosParam.lNowBatchNum!=lTmpLong
			&& lTmpLong>0)
	{
		stPosParam.lNowBatchNum = lTmpLong;
	}

	GetPosTime(szDate, szTime);
	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT YEAR");
	lcdDisplay(0, 4, DISP_CFONT, "(YYYY)");
	memset(buf, 0, sizeof(buf));
	iRet = PubGetString(NUM_IN,4, 4,buf,stPosParam.ucOprtLimitTime,1);
	if( iRet!=0 )
	{
		return ;
	}
  	else if (4 == strlen((char *)buf))
	{
		memcpy(szDate, buf, 4);
		szDate[8] = 0x00;
		if( CheckDate((char *)szDate)!=0 )
		{
			FailBeep();
			lcdClrLine(4, 7);
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "DATE FORMAT ERR");
			lcdFlip();
			kbGetKeyMs(2000);
			return;
		}

		AscToBcd(szDateTime, &szDate[2], 6);
		AscToBcd(szDateTime+3, szTime, 6);
		sysSetTime(szDateTime);
		write_stat_time();
		memset(&dial_stat, 0, sizeof(DIAL_STAT));
		write_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta");
	}

	iRet = SetPrintTicketNum();
	if (iRet!=0)
	{
		return;
	}

	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "SSL");
	lcdDisplay(0, 4, DISP_CFONT, "   0-NO    1-SI ");
    	memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.open_internet;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.open_internet = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.open_internet = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

#ifndef _POS_TYPE_8110
	SetElecSignSwitch();
#endif

	iRet = SetTranResendTime();
	if (iRet!=0)
	{
		return;
	}
	iRet = SetTranMaxNumber();
	if (iRet!=0)
	{
		return;
	}
	return ;
}

//切换通讯必须下电
int SetCurCommType(int iMode)
{
	uint8_t  ucCommType, ucCommTypeBak,DispPosition,line_no;
	int  iKey,iRet;
#ifndef _POS_TYPE_8110
	uint8_t  ucCommBak;
#endif

	if (gstPosCapability.uiScreenType)
	{
		DispPosition = 160;
		line_no = 2;
	}
	else
	{
		DispPosition = 48;
		line_no = 0;
	}

	if (iMode == DOWNLAOD_COMMTYPE)
	{
		ucCommType = stPosParam.stDownParamCommCfg.ucCommType;
	}
	else
	{
		ucCommType = stPosParam.stTxnCommCfg.ucCommType;
	}
	if( !(ucCommType & CT_ALLCOMM) || ucCommType>CT_ALLCOMM )
	{
		ucCommType = gstPosCapability.uiCommType;
		ucCommType = ucCommType & ~CT_RS232;
	}

	ucCommTypeBak = ucCommType;  // 先备份原来的通讯类型

#ifndef _POS_TYPE_8110
PRESET_COMM:
#endif
	while(1)
	{
		if (iMode == DOWNLAOD_COMMTYPE)
		{
			if (ucCommType == CT_MODEMPPP)
			{
				ucCommType = CT_MODEM;
			}
		}

		lcdCls();
		lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "COMM SELECT");
		line_no = 2;
		lcdDisplay(0, line_no, (CT_MODEM == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "1.DIAL");
		lcdDisplay(DispPosition, line_no, (CT_GPRS == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "2.GPRS");
		line_no += 2;
		lcdDisplay(0, line_no, (CT_CDMA == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "3.CDMA");
		lcdDisplay(DispPosition, line_no, (CT_TCPIP == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "4.TCP");
		line_no += 2;
		lcdDisplay(0, line_no, (CT_RS232 == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "5.SERIAL");
		lcdDisplay(DispPosition, line_no, (CT_WIFI == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "6.WIFI");
		lcdFlip();
		iKey = kbGetKeyMs(30000);
		switch(iKey)
		{
		case KEY_ENTER:
			return 0;
		case KEY_CANCEL:
		case KEY_TIMEOUT:
			return -1;
		case KEY1:
			ucCommType = CT_MODEM;
			break;
		case KEY2:
			ucCommType = CT_GPRS;
			break;
		case KEY3:
			ucCommType = CT_CDMA;
			break;
		case KEY4:
			ucCommType = CT_TCPIP;
			break;
		case KEY5:
			ucCommType = CT_RS232;
			break;
		case KEY6:
			ucCommType = CT_WIFI;
			break;
		default:
			continue;
		}

		if(( gstPosCapability.uiCommType & ucCommType ) == 0)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 2, DISP_CFONT, "Modual Non-Exist");
			lcdDisplay(0, 4, DISP_CFONT, "Re-Choose");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(3000);
			ucCommType = ucCommTypeBak;
			continue;
		}
		break;
	}

	if(ucCommType==CT_MODEM && iMode==DOWNLAOD_COMMTYPE)
	{
		iRet = SetRemoteModemType();
		if (iRet != 0)
		{
			return iRet;
		}

		if (stPosParam.stDownParamCommCfg.stPSTNPara.ucSendMode == MODEM_COMM_SYNC)
			ucCommType = CT_MODEMPPP;
		else
			ucCommType = CT_MODEM;
	}

	// hangup the orginal comm
	if (ucCommTypeBak != ucCommType)
	{
		if(ucCommType==CT_GPRS || ucCommType==CT_CDMA)
		{
#ifdef _POS_TYPE_8110
			CommOnHookMode(ucCommType); //powen down
			if ( gstPosCapability.uiCommType & CT_WIFI)
			{
				WnetSelectModule(0);
			}
#else
			if (ucCommTypeBak!=CT_GPRS && ucCommTypeBak!=CT_CDMA)
			{
				lcdClrLine(2, 7);
				lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "module info...");
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "pls waiting");
				lcdFlip();
				CommOnHookMode(ucCommTypeBak); //wireless need to power on,so orginal comm poer down at first
				if(stPosParam.stTxnCommCfg.ucCommType!=CT_GPRS && stPosParam.stTxnCommCfg.ucCommType!=CT_CDMA)
				CommOnHookMode(stPosParam.stTxnCommCfg.ucCommType);//tms for modemppp,transaction for modem
			}
			if ( gstPosCapability.uiCommType & CT_WIFI)
			{
				WnetSelectModule(0);
			}

			CheckWireless();//get module,GPRS or CDMA
			if(( gstPosCapability.uiCommType & ucCommType ) == 0)//no this module,wireless power down
			{
				if (ucCommTypeBak!=CT_GPRS && ucCommTypeBak!=CT_CDMA)
				{
					if (iMode == DOWNLAOD_COMMTYPE)
					{
						ucCommBak = stPosParam.stDownParamCommCfg.ucCommType;
						stPosParam.stDownParamCommCfg.ucCommType = ucCommType;
						CommInitModule(&stPosParam.stDownParamCommCfg);//power on then power down
						stPosParam.stDownParamCommCfg.ucCommType = ucCommBak;
					}
					else
					{
						ucCommBak = stPosParam.stTxnCommCfg.ucCommType;
						stPosParam.stTxnCommCfg.ucCommType = ucCommType;
						CommInitModule(&stPosParam.stTxnCommCfg);
						stPosParam.stTxnCommCfg.ucCommType = ucCommBak;
					}
					CommOnHookMode(ucCommType); //下电
				}
				lcdClrLine(2, 7);
				lcdDisplay(0, 2, DISP_CFONT, "Modual Non-Exist");
				lcdDisplay(0, 4, DISP_CFONT, "Re-Choose");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				ucCommType = ucCommTypeBak;
				goto PRESET_COMM;
			}
#endif
		}
		else if(ucCommType==CT_WIFI)
		{
			CommOnHookMode(ucCommTypeBak); //下电
			if ( (gstPosCapability.uiCommType & CT_GPRS)
				|| (gstPosCapability.uiCommType & CT_CDMA))
			{
				WnetSelectModule(1);
				InitWiFiModule(TRUE);
			}
		}
		else
		{
			CommOnHookMode(ucCommTypeBak); //下电
		}
#ifdef _POS_TYPE_8210
		if (wifi_power_control_force)
		{
			if(ucCommTypeBak==CT_WIFI && ucCommType!=CT_WIFI)	//close wifi
			{
				wifi_power_control_force(0);
				wifi_status = 0;
			}
			else if(ucCommTypeBak!=CT_WIFI && ucCommType==CT_WIFI)
			{
				wifi_power_control_force(1);
				wifi_status = 1;
			}
		}
#endif

		if (iMode == DOWNLAOD_COMMTYPE)
		{
			stPosParam.stDownParamCommCfg.ucCommType = ucCommType;
			CommInitModule(&stPosParam.stDownParamCommCfg);//modem ppp not allow open modem
		}
		else
		{
			stPosParam.stTxnCommCfg.ucCommType = ucCommType;
			CommInitModule(&stPosParam.stTxnCommCfg);
		}
		SaveAppParam();
	}
	WritePubComm();//???????????
	return ucCommType;
}

void Manage_Pwd(char *pPwdInput)
{
	POS_PARAM_STRC_MAIN TmpMngParam;
	if ( 0 == Get_Ser_Comm_Param(&TmpMngParam))
	{
		snprintf((char *)TmpMngParam.szParamsPwd, sizeof(TmpMngParam.szParamsPwd), "%s", pPwdInput);
//		strcpy((char *)TmpMngParam.szParamsPwd,"869637");
		Set_Ser_Comm_Param(&TmpMngParam);
	}
}


int SetCommParam(int iMode)
{
	uint8_t  ucCommType,DispPosition,line_no;
	int  iKey,iRet;
	ST_COMM_CONFIG  stCommCfgBak;
	POS_PARAM_STRC_MAIN pSaveParam;

	if (iMode == DOWNLAOD_COMMTYPE)
	{
		ucCommType = stPosParam.stDownParamCommCfg.ucCommType;
	}
	else
	{
		ucCommType = stPosParam.stTxnCommCfg.ucCommType;
	}
	if( !(ucCommType & CT_ALLCOMM) || ucCommType>CT_ALLCOMM )
	{
		ucCommType = gstPosCapability.uiCommType;
		ucCommType = ucCommType & ~CT_RS232;
	}
	while(1)
	{
		lcdCls();
		if (gstPosCapability.uiScreenType)
		{
			DispPosition = 160;
			line_no = 2;
			lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "Para de conexion");
		}
		else
		{
			DispPosition = 48;
			line_no = 0;
		}

		if (iMode == DOWNLAOD_COMMTYPE)
		{
			if (ucCommType == CT_MODEMPPP)
			{
				ucCommType = CT_MODEM;
			}
		}

		lcdDisplay(0, line_no, DISP_CFONT,"1.TPDU");
		lcdDisplay(DispPosition, line_no,DISP_CFONT, "2.TYPE");
		line_no += 2;
		lcdDisplay(0, line_no, (CT_MODEM == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "3.DIAL");
		lcdDisplay(DispPosition, line_no, (CT_GPRS == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "4.GPRS");
		line_no += 2;
		lcdDisplay(0, line_no, (CT_CDMA == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "5.CDMA");
		lcdDisplay(DispPosition, line_no, (CT_TCPIP == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "6.TCP");
		line_no += 2;
		lcdDisplay(0, line_no, (CT_RS232 == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "7.SERIAL");
		lcdDisplay(DispPosition, line_no, (CT_WIFI == ucCommType)?(DISP_CFONT|DISP_REVERSE):(DISP_CFONT), "8.WIFI");
		lcdFlip();

		iKey = kbGetKeyMs(30000);

		if(iKey == KEY_ENTER)
		{
			if(CT_MODEM == ucCommType)
				iKey = '3';
			else if(CT_GPRS == ucCommType)
				iKey = '4';
			else if(CT_CDMA == ucCommType)
				iKey = '5';
			else if(CT_TCPIP == ucCommType)
				iKey = '6';
			else if(CT_RS232 == ucCommType)
				iKey = '7';
			else if(CT_WIFI == ucCommType)
				iKey = '8';
		}

		switch(iKey)
		{
		case KEY_CANCEL:
		case KEY_TIMEOUT:
			return -1;
		case KEY1:
			SetTpduOutTime();
			break;
		case KEY2:
			iRet = SetCurCommType(iMode);
			if(iRet > 0)
				ucCommType = iRet;
			break;

		case KEY3:
			if (iMode == TRANS_COMMTYPE)
			{
				SetAcqTransTelNo(iMode);
				SetModemParam();
			}
			else
			{
//				iRet = SetDownModem();
			}
			break;
		case KEY4:
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stCommCfgBak = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stPosParam.stDownParamCommCfg;
			}
			Get_Ser_Comm_Param(&pSaveParam);
			iRet = SetWirelessParam(iMode,CT_GPRS,&pSaveParam);
			Set_Ser_Comm_Param(&pSaveParam);
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stPosParam.stDownParamCommCfg = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stCommCfgBak;
			}
			break;
		case KEY5:
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stCommCfgBak = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stPosParam.stDownParamCommCfg;
			}
			Get_Ser_Comm_Param(&pSaveParam);
			iRet = SetWirelessParam(iMode,CT_CDMA,&pSaveParam);
			Set_Ser_Comm_Param(&pSaveParam);
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stPosParam.stDownParamCommCfg = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stCommCfgBak;
			}
			break;
		case KEY6:
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stCommCfgBak = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stPosParam.stDownParamCommCfg;
			}
			iRet = SetTcpIpParam(iMode);
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stPosParam.stDownParamCommCfg = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stCommCfgBak;
			}
			break;
		case KEY7:
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stCommCfgBak = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stPosParam.stDownParamCommCfg;
			}
			SetRS232Param();
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stPosParam.stDownParamCommCfg = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stCommCfgBak;
			}
			break;
		case KEY8:
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stCommCfgBak = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stPosParam.stDownParamCommCfg;
			}
			iRet = SetWIFIParam(iMode);
			if (iMode == DOWNLAOD_COMMTYPE)
			{
				stPosParam.stDownParamCommCfg = stPosParam.stTxnCommCfg;
				stPosParam.stTxnCommCfg = stCommCfgBak;
			}
			break;
		case KEY0:
			iRet = SetWnetMode();
			break;
		}
	}
	return 0;
}


int SetTpdu(void)
{
	uint8_t buf[32], buf1[32];
	int  iRet;

	memset(buf1, 0, sizeof(buf1));
	BcdToAsc(buf1, stPosParam.sTpdu, 10);
	buf1[10] = 0;
	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "COMM SETUP");
	lcdDisplay(0, 2, DISP_CFONT, "TPDU");
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf1);
	lcdFlip();
	lcdGoto(40, 6);
	iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return -1;
	}
	if( iRet==10 && strcmp((char *)buf1, (char *)buf) )
	{
		AscToBcd(stPosParam.sTpdu, buf, 10);
	}
	return 0;
}

void SetTpduOutTime(void)
{
	uint8_t buf[32], buf1[32];
	int     iTmpInt, iRet;

	if(SetTpdu()!=0)
		return;

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "COMM SETUP");
	lcdDisplay(0, 2, DISP_CFONT, "PRE Dial?");
	lcdDisplay(0, 4, DISP_CFONT, "0-No  1-Yes");
	memset(buf, 0, sizeof(buf));
	buf[0] = stPosParam.bPreDial;
	iRet = SelectSetTwo(buf);
	if( iRet==0 )
	{
		stPosParam.bPreDial = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.bPreDial = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	memset(buf1, 0, sizeof(buf1));
	sprintf((char *)buf1, "(20~90S): %d", stPosParam.ucCommWaitTime);

	lcdClrLine(2, 7);
	lcdDisplay(0, 2, DISP_CFONT, "COMM TIMEOUT ");
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf1);
	lcdFlip();
	lcdGoto(104, 6);
	iRet = kbGetString(KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 2, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	iTmpInt = atoi((char *)buf);
	if( iRet>0 && iTmpInt>=20 && iTmpInt<=90 && stPosParam.ucCommWaitTime!=iTmpInt )
	{
		stPosParam.ucCommWaitTime = iTmpInt;
	}

	lcdClrLine(2, 7);
	lcdDisplay(0, 2, DISP_CFONT, "   REDAIL TIMES ");
	sprintf((char *)buf, "%d", stPosParam.ucDialRetryTimes);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdFlip();
	lcdGoto(112, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 1, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	iTmpInt = atoi((char *)buf);
	if( iRet==1 && iTmpInt<=6 && iTmpInt>=1 && stPosParam.ucDialRetryTimes!=iTmpInt )
	{
		stPosParam.ucDialRetryTimes = iTmpInt;
	}
}

void ClearAllTranLog(void)
{
	stTransCtrl.iTransNum = 0;
	ClearAllTxnStatus();
	ClearElecSignaturefile();
	fileMakeDir(STR_ELECSIGNATURE_FILE_PATH);  // 创建用于保存电子签名图片的目录,没有就创建
	fileRemove(POS_LOG_FILE);
	fileRemove(REFUND_TRACK_FILE);
	fileRemove(ICC_FAILUR_TXN_FILE);
	stTransCtrl.ucClearLog = FALSE;
	stTransCtrl.ucPosStatus = WORK_STATUS;
	SaveCtrlParam();
}

void CleaCheckInTranLog(void)
{
	stTransCtrl.iCheckTransNum = 0;
	ClearAllTxnStatus();
	ClearElecSignaturefile();
	fileMakeDir(STR_ELECSIGNATURE_FILE_PATH);  // 创建用于保存电子签名图片的目录,没有就创建
	fileRemove(POS_LOG_CHECK_FILE);
	fileRemove(ICC_FAILUR_TXN_FILE);
	stTransCtrl.ucClearCheckLog = FALSE;
	stTransCtrl.ucPosStatus = WORK_STATUS;
	SaveCtrlParam();
}

void ClearFile(void)
{
	int iRet;

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "CLEAR BATCH");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "1.Clear 0.Cancel");
	lcdFlip();
	iRet = kbGetKey();
	if( iRet==KEY1 )
	{
		ClearAllTranLog();
		CleaCheckInTranLog();
		lcdClrLine(2,7);
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY, "Cleared");
		lcdFlip();
		PubWaitKey(3);
	}
	return ;
}

void ClearReversal(void)
{
	int iRet;

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "CLEAR REVERSAL");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "1.Clear 0.Cancel");
	lcdFlip();
	iRet = kbGetKey();
	if( iRet==KEY1 )
	{
		fileRemove(REVERSAL_FILE);
		lcdClrLine(2,7);
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"Cleared");
		lcdFlip();
		PubWaitKey(3);

	}
}

void ClearScript(void)
{
	int iRet;

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "CLEAR SCRIPT");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "1.Clear 0.Cancel");
	lcdFlip();
	iRet = kbGetKey();
	if( iRet==KEY1 )
	{
		fileRemove(ICC_SCRIPT_FILE);
		lcdClrLine(2,7);
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"Cleared");
		lcdFlip();
		PubWaitKey(3);
	}
}

void ClearBlkCard(void)
{
	int iRet;

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "CLEAR B-LIST");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "1.Clear 0.Cancel");
	lcdFlip();
	iRet = kbGetKey();
	if( iRet==KEY1 )
	{
		fileRemove(FILE_BLKCARD_LOG);
		lcdClrLine(2,7);
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"Cleared");
		lcdFlip();
		PubWaitKey(3);
	}
}

void SetClear(void)
{
	int iKey;
	const char *clearMenu[] =
	{
			"1.CLEAR REVERSAL",
			"2.CLEAR BATCH",
			"3.CLEAR SCRIPT",
			"4.CLEAR B-LIST",
			"5.DISP FREE MEM"
	};


	while(1)
	{
		iKey = DispMenu1(clearMenu, 5, 0, KEY1, KEY5, stPosParam.ucOprtLimitTime);
		switch(iKey)
		{
		case KEY1:
			ClearReversal();
			break;
		case KEY2:
			ClearFile();
			break;
		case KEY3:
			ClearScript();
			break;
		case KEY4:
			ClearBlkCard();
			break;
		case KEY5:
			DispRemainSpace();
			break;
		case KEY_CANCEL:
			return;
		case KEY_ENTER:
		case KEY_UP:
			break;
		default:
			continue;
		}
	}
}

void SetEMVQPBOC(void)
{
	int iRet;
	uint8_t buf2[2];

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "SUPPORT EMV TRAN");
	lcdDisplay(0, 2, DISP_CFONT, "SUPPORT EMV?");
	lcdDisplay(0, 4, DISP_CFONT, "0-OFF 1-ON");
    	memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucEmvSupport;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucEmvSupport = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucEmvSupport = PARAM_OPEN;
	}
	else if(iRet<0)
    {
        return;
    }

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "SUPPORT QPBOC");
	lcdDisplay(0, 2, DISP_CFONT, "SUPPORT QPBOC?");
	lcdDisplay(0, 4, DISP_CFONT, "0-OFF 1-ON");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucqPbocSupport;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucqPbocSupport = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucqPbocSupport = PARAM_OPEN;
	}
	else if(iRet<0)
    {
        return;
    }

	return ;
}


void SetTestStatus(void)
{
	uint8_t key1;

	key1 = OptionsSelect("    TEST STATUS    ", stPosParam.ucTestFlag==PARAM_CLOSE,
		"CURR STATUS:NORMAL", "CURR STATUS:TEST", "[1].NORMAL", "[2].TEST", stPosParam.ucOprtLimitTime);
	if( (stTransCtrl.iTransNum!=0) && (key1==KEY1 || key1==KEY2) )
	{
		lcdClrLine(2, 7);
		lcdDisplay(0, 4, DISP_CFONT, "  SETTLE FIRST  ");
		lcdFlip();
		sysBeef(3, 1000);
		kbGetKeyMs(1000);
		return;
	}

	if( key1==KEY2 && stPosParam.ucTestFlag==PARAM_CLOSE )
	{
		stPosParam.ucTestFlag = PARAM_OPEN;
		strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
		stTransCtrl.ucLogonFlag = LOGGEDOFF;
		stTransCtrl.ucPosStatus = TEST_STATUS;
	}
	else if( key1==KEY1 && stPosParam.ucTestFlag==PARAM_OPEN )
	{
		stPosParam.ucTestFlag = PARAM_CLOSE;
		strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_NOBODY);
		stTransCtrl.ucLogonFlag = LOGGEDOFF;
		stTransCtrl.ucPosStatus = WORK_STATUS;
	}
	SaveCtrlParam();
	return ;

}

void SetMasterKeys(void)
{
	uint8_t buf[64];
	uint8_t buf1[64];
	uint8_t ucTmpKeyId;
	uint8_t ucLen,ucMode;
	int iTmpInt, iRet, iKey;
	const char* KeyMenu[] =
	{
		"KEY MANAGE      ",
		"1.IN MASTER KEY ",
		"2.SET ALGORITHM ",
		"3.TRAN KEY INDEX"
	};

	while(1)
	{
		iKey = DispMenu(KeyMenu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
		switch(iKey)
		{
		case KEY1:
			if( stPosParam.ucKeyMode==KEY_MODE_DES )
			{
				ucLen = 16;
				ucMode = 0x01;
			}
			else  ///3DES
			{
				ucLen = 32;
				ucMode = 0x03;
			}
			lcdCls();
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " MASTER KEY ");
			DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT KEY INDEX:");
			DispMulLanguageString(0, 4, DISP_CFONT, NULL, "INPUT:(00--99)");
			lcdFlip();
			lcdGoto(104, 6);
			memset(buf,0,sizeof(buf));
			iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 2, (stPosParam.ucOprtLimitTime*1000),
				(char *)buf);
			if( iRet>0 )
			{
				iTmpInt = atoi((char *)buf);
				if( iTmpInt>99 || iTmpInt<0 )
				{
					lcdClrLine(6, 7);
					DispMulLanguageString(0, 6, DISP_CFONT, NULL, "KEY INDEX ERROR");
					lcdFlip();
					kbGetKeyMs(1000);
					break;
				}
				ucTmpKeyId = iTmpInt+1;
			}
			else
				break;
			lcdCls();
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " MASTER KEY ");
			DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT KEY DATA:");
			lcdFlip();
			lcdGoto(0, 4);
			memset(buf,0,sizeof(buf));
			iRet = PubGetString(ALPHA_IN,ucLen,ucLen,(uint8_t*)buf,stPosParam.ucOprtLimitTime, 0);
			if( iRet!=0 )
				break;

			if( stPosParam.ucKeyMode==KEY_MODE_DES )
			{
				AscToBcd(buf1, buf, ucLen);
				memcpy(buf1+8, buf1, 8);
			}
			else
			{
				AscToBcd(buf1, buf, ucLen);
			}

			iRet = PedWriteMasterKey(PARITY_NONE+TDEA_NONE, 1, ucTmpKeyId, 16, buf1);
			if( iRet!=PED_RET_OK )
			{
				lcdClrLine(2, 7);
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " LOAD KEY ERROR ");
				lcdDisplay(0, 6, DISP_CFONT, "iRet=%d", iRet);
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				break;
			}

			if( stPosParam.ucSupPinpad==PARAM_OPEN )
			{
				iRet = CheckConnectPinPad(1);
				if (iRet!=0)
				{
					break;
				}
				iRet = EppDownloadAppKey(0x01, PARITY_NONE+TDEA_NONE, ucTmpKeyId, buf1, 16, ucTmpKeyId);
				EppLight(3,60*1000);
				if( iRet!=EPP_SUCCESS )
				{
				    if(iRet==EPP_RECV_TIMEOUT)
	       			{
						lcdClrLine(2, 7);
						DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CHECK:");
						DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL, "PINPAD CONNECTED");
						lcdFlip();
						ErrorBeep();
						kbGetKeyMs(3000);
						break;
		   			}
				    else
				    {
						lcdClrLine(2, 7);
						DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " LOAD KEY ERROR ");
						lcdDisplay(0, 6, DISP_CFONT, "iRet=%d", iRet);
						lcdFlip();
						ErrorBeep();
						kbGetKeyMs(3000);
						break;
				    }
				}
			}

            PubBeepOk();
            lcdClrLine(2, 7);
            lcdDisplay(0, 3, DISP_CFONT, "SUCCEED");
			lcdFlip();
            kbGetKeyMs(3000);
			break;

		case KEY2:
			lcdCls();
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "SETUP KEY LENGTH");
			lcdDisplay(0, 2, DISP_CFONT, "1-SINGLE DES");
			lcdDisplay(0, 4, DISP_CFONT, "0-TRIPLE DES");
			memset(buf, 0, sizeof(buf));
			buf[0] = stPosParam.ucKeyMode;
			iRet = SelectSetTwo(buf);
			if( iRet==0 )
			{
				stPosParam.ucKeyMode = KEY_MODE_3DES;
			}
			else if( iRet==1 )
			{
				stPosParam.ucKeyMode = KEY_MODE_DES;
			}
			break;

		case KEY3:
			lcdCls();
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   KEY INDEX    ");
			iTmpInt = stPosParam.ucKeyIndex;
			sprintf((char *)buf, "KEY INDEX:%02d", iTmpInt);
			lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
			lcdDisplay(0, 4, DISP_CFONT, "INPUT:(00--99)  ");
			sprintf((char *)buf, "%02d", iTmpInt);
			lcdFlip();
			lcdGoto(104, 6);
			memset(buf,0,sizeof(buf));
			iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 1, 2, (stPosParam.ucOprtLimitTime*1000),
				(char *)buf);
			if( iRet>0 && (iTmpInt!=atoi((char *)buf)) )
			{
				iTmpInt = atoi((char *)buf);
				if( iTmpInt>99 || iTmpInt<0 )
				{
					lcdClrLine(6, 7);
					DispMulLanguageString(0, 6, DISP_CFONT, NULL, "KEY INDEX ERROR");
					lcdFlip();
					kbGetKeyMs(1000);
					break;
				}
				stPosParam.ucKeyIndex = iTmpInt;
			}
			break;

		default:
			return;
		}
	}
}

int SetTranResendTime(void)
{
	uint8_t buf[32];
	int     iTmpInt;
	int     iRet;

	lcdClrLine(2,7);
	iTmpInt = stPosParam.ucResendTimes;
	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf, "RESEND TIME:%d", iTmpInt);
	lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
	lcdFlip();
	sprintf((char *)buf, "%d", iTmpInt);
	lcdGoto(112, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 1, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return NO_DISP;
	}
	if( iRet==1 && iTmpInt!=atoi((char *)buf) )
	{
		iTmpInt = atoi((char *)buf);
		stPosParam.ucResendTimes = iTmpInt;
	}
	return 0;
}

int SetOfflineSendTime(void)
{
	uint8_t buf[32];
	int     iTmpInt;
	int     iRet;

	lcdClrLine(2,7);
	iTmpInt = stPosParam.ucOfflineSendTimes;
	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf, "OFF SEND TIMES:%d", iTmpInt);
	lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
	lcdFlip();
	sprintf((char *)buf, "%d", iTmpInt);
	lcdGoto(112, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 1, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return NO_DISP;
	}
	if( iRet==1 && iTmpInt!=atoi((char *)buf) )
	{
		iTmpInt = atoi((char *)buf);
		stPosParam.ucOfflineSendTimes = iTmpInt;
	}
	return 0;
}


void SetTransFlow(void)
{
	uint8_t buf2[2];
	int iRet,iKey;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  PRE-CMP MODE  ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "0 BOTH          ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "1 PRE-CMP ONLINE");
	DispMulLanguageString(0, 6, DISP_CFONT, NULL, "2 PRECMP OFFLINE");
	kbFlush();
	lcdFlip();
	iKey = kbGetKey();
	if( iKey==KEY0 )
		stPosParam.ucConfirm = '0';
	else if( iKey==KEY1 )
		stPosParam.ucConfirm = '1';
	else if( iKey==KEY2 )
		stPosParam.ucConfirm = '2';
	if( stPosParam.ucConfirm=='0' )
	{
		MaskTrans(POS_AUTH_CM, 4, "PRE-CMP ONLINE", KEY2);
		MaskTrans(POS_OFF_CONFIRM, 11, "PRECMP OFFLINE", KEY2);
	}
	else if( stPosParam.ucConfirm=='1' )
	{
		MaskTrans(POS_AUTH_CM, 4, "PRE-CMP ONLINE", KEY2);
		MaskTrans(POS_OFF_CONFIRM, 11, "PRECMP OFFLINE", KEY1);
	}
	else if( stPosParam.ucConfirm=='2' )
	{
		MaskTrans(POS_AUTH_CM, 4, "PRE-CMP ONLINE", KEY1);
		MaskTrans(POS_OFF_CONFIRM, 11, "PRECMP OFFLINE", KEY2);
	}

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "ENCRYPT TRACK");
	lcdDisplay(0, 2, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucTrackEncrypt;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucTrackEncrypt = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucTrackEncrypt = PARAM_OPEN;
	}
}

int TipperSetup(void)
{
	int     iRet, iTmpInt;
	uint8_t dispbuf[20], buf[20];


	lcdClrLine(2,7);
	sprintf((char *)dispbuf, "TIP PERCENT(%d%%)", stPosParam.ucTipper);
	lcdDisplay(0, 2, DISP_CFONT, "%s", dispbuf);
	sprintf((char *)buf, "%d", stPosParam.ucTipper);
	lcdFlip();
	lcdGoto(104, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 2, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if (iRet==KB_TIMEOUT || iRet==KB_CANCEL)
	{
		return NO_DISP;
	}
	iTmpInt = atoi((char *)buf);
	if( iRet>0 && iTmpInt!=stPosParam.ucTipper )
	{
		stPosParam.ucTipper = iTmpInt;
	}
	return 0;
}

void SetVoidPin(void)
{
	int     iRet;
	uint8_t buf2[2];

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE, "VOID SALE");
	lcdDisplay(0, 2, DISP_CFONT|DISP_INVLINE, "NEED PIN");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucVoidPin;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucVoidPin = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucVoidPin = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE, "VOID PREAUTH");
	lcdDisplay(0, 2, DISP_CFONT|DISP_INVLINE, "NEED PIN");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucPreVoidPin;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucPreVoidPin = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucPreVoidPin = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE, "VOID AUTH_COMP");
	lcdDisplay(0, 2, DISP_CFONT|DISP_INVLINE, "NEED PIN");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucPreComVoidPin;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucPreComVoidPin = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucPreComVoidPin = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}


	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE, "PREAUTH COMP");
	lcdDisplay(0, 2, DISP_CFONT|DISP_INVLINE, "NEED PIN");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucPreComPin;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucPreComPin = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucPreComPin = PARAM_OPEN;
	}
	else if( iRet<0)
	{
		return ;
	}

	return ;
}

void SetVoidSwipe(void)
{
	int     iRet;
	uint8_t buf2[2];

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VOID SALE");
	lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "NEED SWIPE");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucVoidSwipe;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucVoidSwipe = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucVoidSwipe = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE, "VOID AUTH_COMP");
	lcdDisplay(0, 2, DISP_CFONT|DISP_INVLINE, "NEED SWIPE");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucAuthVoidSwipe;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucAuthVoidSwipe = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucAuthVoidSwipe = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}
	return;
}

void SetSettleSwitch(void)
{
	int     iRet;
	uint8_t buf2[2];

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "SETTLE TRAN");
	lcdDisplay(0, 2, DISP_CFONT, "LOGOFF AFTER SETTLE");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucAutoLogoff;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucAutoLogoff = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucAutoLogoff = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	lcdDisplay(0, 2, DISP_CFONT, "PRINT SETTLE DETAIL");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
    buf2[0] = stPosParam.ucDetailPrt;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucDetailPrt = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucDetailPrt = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}
	return;
}

void SetOfflineSwitch(void)
{
	int     iRet;
	uint8_t buf2[2];

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "OFFLINE TRAN");
	lcdDisplay(0, 2, DISP_CFONT, "UPLOAD METHOD");
	lcdDisplay(0, 4, DISP_CFONT, "1.ONLINE 0.SETTLE");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucSendOffFlag;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucSendOffFlag = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucSendOffFlag = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	iRet = SetOfflineSendTime();
	if (iRet!=0)
	{
		return;
	}
	SetMaxOfflineNums();
	return;
}

void SetOtherSwitch(void)
{
	int     iRet;
	uint8_t buf2[20],buf1[20];

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE, "VOID/REFUND");
	lcdDisplay(0, 2, DISP_CFONT|DISP_INVLINE, "NEED MANAGER PWD");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO 1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucEnterSupPwd;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucEnterSupPwd = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucEnterSupPwd = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "OTHER SET");
	lcdDisplay(0, 2, DISP_CFONT, "MANUAL CARD");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO 1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucManualInput;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucManualInput = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucManualInput = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

/*	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_CFONT, "DEFAULT TRANS");
	lcdDisplay(0, 4, DISP_CFONT, "0-PREAUTH 1-SALE");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucDefaltTxn;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucDefaltTxn = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucDefaltTxn = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}*/

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "MAX REFUND AMT");
	lcdDisplay(0, 4, DISP_CFONT, "INPUT MAX AMT");
	LongToAmount(buf2, stPosParam.lMaxRefundAmt);
	sprintf((char *)buf1, "FORMER:%s", buf2);
	lcdDisplay(0, 2, DISP_CFONT, (char *)buf1);
	lcdFlip();
	lcdGoto(48, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_FLOAT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 8,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf2);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return ;
	}
	if( iRet>0 && stPosParam.lMaxRefundAmt!=atol((char *)buf2) )
	{
		stPosParam.lMaxRefundAmt= atol((char *)buf2);
	}

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "OTHER SET");
	lcdDisplay(0, 2, DISP_CFONT, "SET HOT KEY");
	lcdDisplay(0, 4, DISP_CFONT, "0-NO 1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucSetFastKey;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucSetFastKey = PARAM_CLOSE;
	}
	else if( iRet==1 )
	{
		stPosParam.ucSetFastKey = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	return;
}

int SetTranMaxNumber(void)
{
	uint8_t buf[18];
	int iTmpInt, iRet;

	lcdClrLine(2,7);
	sprintf((char *)buf, "MAX TRAN NUM:%d", stPosParam.iMaxTransTotal);
	lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
	while(1)
	{
		lcdClrLine(4, 7);
		sprintf((char *)buf, "%d", stPosParam.iMaxTransTotal);
		lcdFlip();
		lcdGoto(96, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 3, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if (iRet==KB_TIMEOUT || iRet==KB_CANCEL)
		{
			return NO_DISP;
		}
		else if( iRet==0 )
			return 0;
		iTmpInt = atoi((char *)buf);
		if( iTmpInt>500 || iTmpInt==0 )
		{
			lcdClrLine(6, 7);
			lcdDisplay(0, 6, DISP_CFONT, "   NUM ERROR   ");
			lcdFlip();
			ErrorBeep();
			continue;
		}
		if( iRet>0 && iTmpInt!=stPosParam.iMaxTransTotal )
		{
			stPosParam.iMaxTransTotal = iTmpInt;
			break;
		}
		else
			return 0;
	}
	return 0;

}


void  SetMaxOfflineNums(void)
{
	uint8_t buf[18];
	int iTmpInt, iRet;

	lcdClrLine(2,7);
	sprintf((char *)buf, "UPLOAD NUM:%d", stPosParam.ucMaxOfflineTxn);
	lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
	while(1)
	{
		lcdClrLine(4, 7);
		sprintf((char *)buf, "%d", stPosParam.ucMaxOfflineTxn);
		lcdFlip();
		lcdGoto(104, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 2, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<=0 )
			return;
		iTmpInt = atoi((char *)buf);
		if( iTmpInt>99 || iTmpInt==0 )
		{
			lcdClrLine(6, 7);
			lcdDisplay(0, 6, DISP_CFONT, "   NUM ERROR   ");
			lcdFlip();
			ErrorBeep();
			continue;
		}
		if( iRet>0 && iTmpInt!=stPosParam.ucMaxOfflineTxn )
		{
			stPosParam.ucMaxOfflineTxn = iTmpInt;
			break;
		}
		else
			return;
	}
}

void PrnFontData(int vai)
{
  int  iRet;
  if (vai == 2)
    iRet = prnSetFont("/usr/share/fonts/PosSong-24.ttf", "GB18030", 0, 24, SET_PRIMARY_FONT);
  else if (vai == 1)
    iRet = prnSetFont("/usr/share/fonts/PosSong-16.ttf", "GB18030", 0, 16, SET_PRIMARY_FONT);
  else
    iRet = prnSetFont("/usr/share/fonts/PosSong-8.ttf", "GB18030", 0, 8, SET_PRIMARY_FONT);
  if ( iRet!=OK )
  {
    lcdClrLine(2, 7);
    DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
    lcdFlip();
    kbGetKey();
    return ;
  }
  return ;
}

void TestPrint(void)
{
	int iRet;

  if( OneTwoSelect("PRINT TEST", "ENTER", "CANCEL")!=KEY1 )
    return;
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
  PrnFontData(2);
  prnPrintf("\n\n\n\n");
  prnPrintf("24Pixels\n");
  PrnFontData(1);
  prnPrintf("\n\n\n\n");
  prnPrintf("16Pixels\n");
  PrnFontData(0);
  prnPrintf("ASCII TEST 1234567890 abcdef\n\n\n\n");
  PrintData();
}


uint8_t MaskTrans(int iTransId, int iMaskBit, char *pFuncName, uint8_t ucInKey)
{
	uint8_t ucKey;
	uint8_t ucEnv_buf[16];
	uint8_t ucMask = 0x80;
	int iIndex = 0;

	if( iMaskBit>8 )
		iIndex = 1;

	ucMask = ucMask >> (iMaskBit - 1) % 8;
	if( ucInKey )
		ucKey = ucInKey;
	else
	{
		ucKey = OptionsSelect(pFuncName, CheckSupport(iTransId), "CURRENT:ON", "CURRENT:OFF", "[1].OFF" ,
			"[2].ON", stPosParam.ucOprtLimitTime);
	}
	switch(ucKey)
	{
	case KEY2:
		stPosParam.sTransOpen[iIndex] |= ucMask;
		BcdToAsc(ucEnv_buf, stPosParam.sTransOpen, 8);
		ucEnv_buf[8] = 0;
		return KEY2;
		break;
	case KEY1:
		stPosParam.sTransOpen[iIndex] &= ~ucMask;
		BcdToAsc(ucEnv_buf, stPosParam.sTransOpen, 8);
		ucEnv_buf[8] = 0;
		return KEY1;
		break;
	case KEY_CANCEL:
		return KEY_CANCEL;
	}
	return OK;
}

void DispMulLanguageString(int Col, int Line, uint32_t Mode, char *pstChBuf, char *pstEnBuf)
{
	if( stPosParam.iDispLanguage==1 )
	{
		lcdDisplay(Col, Line, Mode, pstChBuf);
	}
	else
	{
		lcdDisplay(Col, Line, Mode, pstEnBuf);
	}
}

int SetPrintTicketNum(void)
{
	uint8_t  buf[32];
	int  tmpInt, iRet;
	int  key;

	while(1)
	{
		lcdClrLine(2,7);
		tmpInt = stPosParam.ucTicketNum - '0';
		sprintf((char *)buf, "PRINT NUM: %d", tmpInt);
		lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
		lcdFlip();
		lcdGoto(112, 6);
		iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 1, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if (iRet==KB_TIMEOUT || iRet==KB_CANCEL)
		{
			return NO_DISP;
		}
		else if(iRet==0)
		{
			return 0;
		}
		tmpInt = atoi((char *)buf);
		if(tmpInt>3 || tmpInt<1)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 4, DISP_CFONT | DISP_MEDIACY, "NUM ERROR");
			lcdFlip();
			PubWaitKey(3);
			continue;
		}
		break;
	}
	key = 0x30 + tmpInt;
	if( iRet==1 && tmpInt<=3 && tmpInt>=1 && key!=stPosParam.ucTicketNum )
	{
		stPosParam.ucTicketNum = key;
	}

	return 0;
}

void SetErrPrnTicketNum(void)
{
	int  key;

	key = OptionsSelect(" ERR TRANS PRINT ", stPosParam.ucErrorPrt==PARAM_OPEN,
		"CURRENT:ON", "CURRENT:OFF",	"[1].OFF", "[2].ON", stPosParam.ucOprtLimitTime);

	switch(key)
	{
	case KEY2:
		stPosParam.ucErrorPrt = PARAM_OPEN;
		break;
	case KEY1:
		stPosParam.ucErrorPrt = PARAM_CLOSE;
		break;
	case KEY_CANCEL:
		return;
	}
}


void PrintParam(uint8_t prtFlag)
{
   	int	i,iRet;
    uint8_t   buf[60];

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, NULL, "   PIRNT PARA   ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "WAITTING...");
	lcdFlip();
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
	prnPrintf("Parm Info\n");
	prnPrintf("Major Version:   %s\n", gstPosVersion.szVersion);
	prnPrintf("Minor Version:   %s\n", SRC_VERSION);
#ifdef _POS_TYPE_8210
	memset(buf, 0, sizeof(buf));
	wc_get_libversion((char*)buf);
	prnPrintf("libwnet     : %s\n",(char*)buf);//Sep 19 2014_1.0.0
	memset(buf, 0, sizeof(buf));
	iRet = wc_get_wnetversion((char*)buf);
	if(iRet == 0)
		prnPrintf("wnet        : %s\n",(char*)buf);//Sep 19 2014_1.0.0
#endif
	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf, "Merchant name:%s\n", stPosParam.szEngName);

    if (gstPosCapability.uiPrnType == 'T')
      PrnSmallFontData();
	prnPrintf((char *)buf);
	prnPrintf("MID: %s\n", stPosParam.szUnitNum);
	prnPrintf("TID: %s\n", stPosParam.szPosId);
	memset(buf, 0 , sizeof(buf));
	sysReadSN(buf);
	prnPrintf("SN  : %s\n", buf);
	prnPrintf("Redial num: %d\n", stPosParam.ucDialRetryTimes);
	BcdToAsc(buf, stPosParam.sTpdu, 10);
	buf[10] = 0x00;
	prnPrintf("TPDU: %s\n", buf);
	prnPrintf("Resend times: %d\n", stPosParam.ucResendTimes);
	i = stPosParam.ucTicketNum - '0';
	prnPrintf("Print num: %d\n", i);


	prnPrintf("Comm timeout: %d秒\n", stPosParam.ucCommWaitTime);
	prnPrintf("Batch no.:    %ld\n", stPosParam.lNowBatchNum);
	prnPrintf("Trace no.:    %ld\n", stPosParam.lNowTraceNo);
	prnPrintf("Max trans num: %d笔\n", stPosParam.iMaxTransTotal);

	i = stPosParam.ucKeyIndex;
	prnPrintf("key index: %d\n", i);

	prnPrintf("\n\n\n");
	PrintData();

	if( stPosParam.ucEmvSupport==PARAM_OPEN )
		PrnEmvParam();	// 打印EMV参数
}


void DeviceTest(void)
{
	int ikey;

	const char * test_main[] =
	{
		"  HW TEST       ",
		"1.Printer       ",
		"2.Mag Reader    ",
		"3.Keyboard      ",
		"4.Echo Test     "
	};

	while(1)
	{
		ikey = DispMenu(test_main, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		switch(ikey) {
		case KEY1:
			TestPrint();
			break;
		case KEY2:
			MagTest(1);
			break;
		case KEY3:
			KeyboardTest();
			kbSetSlipFW(ON);
			break;
		case KEY4:
			TestResponse();
			break;

		default:
			return;
		}
	}
}

void ChangeSysPwd(void)
{
	uint8_t buf[16], buf1[16];
	int iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " CHG SYSTEM PWD ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT OLD PWD:");
	lcdFlip();
	lcdGoto(56, 6);
	iRet = kbGetString(KB_EN_CRYPTDISP+KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==6 )
	{
		if( strcmp((char *)buf, (char *)stPosParam.szParamsPwd)!=0 )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR!");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			return ;
		}
	}
	else
	{
		return ;
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NEW PWD:");
	lcdFlip();
	lcdGoto(56, 6);
	iRet = kbGetString(KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==6 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT PWD AGAIN");
		lcdFlip();
		lcdGoto(56, 6);
		iRet = kbGetString(KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6,
			(stPosParam.ucOprtLimitTime*1000), (char *)buf1);
		if( iRet==6 )
		{
			if( strcmp((char *)buf, (char *)buf1)==0 )
			{
				memset(stPosParam.szParamsPwd,0,sizeof(stPosParam.szParamsPwd));
				memcpy(stPosParam.szParamsPwd,buf1,6);
				SaveAppParam();
			}
			else
			{
				lcdClrLine(2, 7);
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT THE SAME");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				return ;
			}
		}
		else
		{
			return ;
		}
	}
	else
	{
		return ;
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "SUCCESS");
	lcdFlip();
	kbGetKeyMs(3000);

	return ;
}

void ChangeSafePwd(void)
{
	uint8_t buf[16], buf1[16];
	int iRet;

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " CHG SAFE PWD ");
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT OLD PWD:");
	lcdFlip();
	lcdGoto(56, 6);
	iRet = kbGetString(KB_EN_CRYPTDISP+KB_EN_BIGFONT+KB_EN_NUM+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==6 )
	{
		if( strcmp((char *)buf, (char *)stPosParam.szSafepwd)!=0 )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PWD ERROR!");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(1000);
			return ;
		}
	}
	else
	{
		return ;
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT NEW PWD:");
	lcdFlip();
	lcdGoto(56, 6);
	iRet = kbGetString(KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==6 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "INPUT PWD AGAIN");
		lcdFlip();
		lcdGoto(56, 6);
		iRet = kbGetString(KB_EN_BIGFONT+KB_EN_CRYPTDISP+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 6, 6,
			(stPosParam.ucOprtLimitTime*1000), (char *)buf1);
		if( iRet==6 )
		{
			if( strcmp((char *)buf, (char *)buf1)==0 )
			{
				memcpy(stPosParam.szSafepwd,buf1,6);
				SaveAppParam();
			}
			else
			{
				lcdClrLine(2, 7);
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "NOT THE SAME");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(3000);
				return ;
			}
		}
		else
		{
			return ;
		}
	}
	else
	{
		return ;
	}

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "SUCCESS");
	lcdFlip();
	kbGetKeyMs(3000);

	return ;
}

int    SelectSetTwo(uint8_t *buf)
{
    int iRet;
    while(1)
    {
        lcdClrLine(6,6);
		lcdFlip();
    	lcdGoto(120, 6);
		iRet = PubGetString(NUM_IN|ECHO_IN, 0, 1, buf, stPosParam.ucOprtLimitTime,0);
    	if(iRet==0)
        {
			if (strlen((char *)buf)>0)
				return atoi((char *)buf);
        }
		else
			return -1;
    }
}

int    NetpaySelectSetTwo(uint8_t *buf)
{
    int iRet;
    while(1)
    {
        lcdClrLine(9,9);
		lcdFlip();
    	lcdGoto(120, 9);
		iRet = PubGetString(NUM_IN|ECHO_IN, 0, 1, buf, stPosParam.ucOprtLimitTime,0);
    	if(iRet==0)
        {
			if (strlen((char *)buf)>0)
				return atoi((char *)buf);
        }
		else
			return -1;
    }
}

void DispRemainSpace(void)
{
	int iFreeLen;

	iFreeLen = fileFreeSpace();
	lcdCls();
	lcdDisplay(0,0,DISP_CFONT|DISP_INVLINE|DISP_MEDIACY,"FREE MEMORY");
	lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%d",iFreeLen);
	lcdFlip();
	PubWaitKey(20);
}

int SetElecSignSendTime(void)
{
	uint8_t buf[32];
	int     iTmpInt;
	int     iRet;

	lcdClrLine(2,7);
	iTmpInt = stPosParam.iElecSignReSendTimes;
	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf, "SIGN RESEND TIMES:%d", iTmpInt);
	lcdDisplay(0, 2, DISP_CFONT, (char *)buf);
	lcdFlip();
	sprintf((char *)buf, "%d", iTmpInt);
	lcdGoto(112, 6);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 1, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return NO_DISP;
	}
	if( iRet==1 && iTmpInt!=atoi((char *)buf) )
	{
		iTmpInt = atoi((char *)buf);
		stPosParam.iElecSignReSendTimes = iTmpInt;
	}
	return 0;
}

void SetElecSignSwitch(void)
{
	int     iRet,iTmpInt;
	uint8_t buf2[4],buf1[4];

	if (!gstPosCapability.uipostsScreen)
	{
		lcdCls();
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, NULL, "SIGNATURE");
		lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "NO SUPPORT SIGN");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return;
	}

	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, NULL, "SIGNATURE");
	lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "SUPPORT SIGN");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "0-NO  1-YES");
    memset(buf2, 0, sizeof(buf2));
	buf2[0] = stPosParam.ucSupportElecSign;
	iRet = SelectSetTwo(buf2);
	if( iRet==0 )
	{
		stPosParam.ucSupportElecSign = PARAM_CLOSE;
		return;
	}
	else if( iRet==1 )
	{
		stPosParam.ucSupportElecSign = PARAM_OPEN;
	}
	else if( iRet<0 )
	{
		return ;
	}

	iRet = SetElecSignSendTime();
	if (iRet!=0)
	{
		return;
	}

	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "TIMEOUT");
	sprintf((char *)buf1,"%d",stPosParam.iElecSignTimeOut);
	iRet = PubGetString(ECHO_IN + NUM_IN, 1, 3,buf1,stPosParam.ucOprtLimitTime,0);
	iTmpInt = atoi((char *)buf1);
	if(iRet == 0x00 &&stPosParam.iElecSignTimeOut != iTmpInt )
	{
		stPosParam.iElecSignTimeOut = iTmpInt;
		if(stPosParam.iElecSignTimeOut < 20) stPosParam.iElecSignTimeOut = 500;
	}
	return;
}


//end of line


