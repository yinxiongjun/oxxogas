#include <openssl/sha.h>

#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include "emvtranproc.h"
#include "Menu.h"
#include "otherTxn.h"
#include "EMVCallBack.h"

#include "SappTms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STX             0x02
#define ETX             0x03
#define ENQ             0x05
#define ACK             0x06
#define NAK             0x15

#define LEN_MAX_RS232   1024

extern int(*__ICCARD_EXCHANGE_APDU)(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdat);

int time_diff(STAT_TIME *old_time, STAT_TIME *new_time);
extern uint32_t	GetEpoch(uint8_t YYYYMMDDhhmmss[14]);
extern uint CheckTransValid(void);	//zyl+
extern void LongToStr(uint32_t ldat, uint8_t *str);

extern int gl_SupEn;  // IC?????????
extern int gl_SupCN;  // IC?????????

#define STAR_YEAR 1950
uint8_t IsLeapYear(uint32_t inyear);

int CheckCapkAid(void);
extern int gl_FirstRunSelApp;

#define UICC_MUTE_MAXLEN 2048
typedef struct _uicc_mute_t {
    int len;
    u8 buf[UICC_MUTE_MAXLEN];
}uicc_mute_t;

/* 
	????????????
	????1980?1?1?????
	
??: ???YYYYMMDD
??:  ??

in： YYYYMMDD
out:   The number of days
*/
const uint16_t monthday[12]=
{
	0,
	31,		//1
	31+28,		//2
	31+28+31,		//3
	31+28+31+30,		//4
	31+28+31+30+31,		//5
	31+28+31+30+31+30,		//6
	31+28+31+30+31+30+31,		//7
	31+28+31+30+31+30+31+31,		//8
	31+28+31+30+31+30+31+31+30,		//9
	31+28+31+30+31+30+31+31+30+31,		//10
	31+28+31+30+31+30+31+31+30+31+30,		//11
};


int half_chn(uint8_t *info, int len)
{
	int i;
	for(i=0; i<len; )
	{
		if(info[i] >= 0x80)
			i += 2;
		else
			i++;
	}

	if(i == len)
		return 0;
	else
		return 1;
}
void scroll_disp(int line_no, char *info, int start_pos)
{
	int len,end;
	static int sPos=0;
	uint8_t msg[256];

	strcpy((char*)msg, info);
	if(start_pos==0)
		sPos = 0;
	len = strlen(info);
	if(len > 21)
	{
		if(half_chn(&msg[sPos], 21)==1)
			end = sPos+20;
		else
			end = sPos+21;
		msg[end]=0;

		lcdDisplay(0, line_no, DISP_CFONT|DISP_FLIP|DISP_CLRLINE, (char*)&msg[sPos]);

		if(len-sPos >= 21)
		{
			sPos++;
			if(half_chn(msg,sPos)==1)
				sPos++;
		}
		else
			sPos = 0;
	}
	else
		lcdDisplay(0, line_no, DISP_CFONT|DISP_MEDIACY|DISP_FLIP, (char*)msg);
}

uint8_t AppGetAmount(int length, uint8_t flag)
{
	uint8_t	buf[17], szAmount[16],szOriAmount[16],szTipAmount[16];
	int	amt, iRet;
	uint32_t	tmp;
	uint32_t    lpreAuthTipAmount;
	uint32_t    orignal_amt =0;

	if( flag==3 && (PosCom.ucSwipedFlag==CARD_INSERTED || PosCom.bSkipEnterAmt) )
	{
		return OK;
	}
	
	//lcdClrLine(2, 7);
	 if(stTemp.iTransNo == POS_SALE || stTemp.iTransNo == POS_PREAUTH ||stTemp.iTransNo == PURSE_SALE)
	{ 
		lcdCls();
		//lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VENTA");
		lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, TitelName);
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "INDIQUE CANTIDAD");
		lcdFlip();
	}
	else if(stTemp.iTransNo == NETPAY_FORZADA)
	{
		lcdCls();
		lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VENTA FORZADA ");
		lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "INDIQUE CANTIDAD");
		lcdFlip();
	}
	memset(szAmount, 0, 16);
    	lcdFlip();
	memset(buf, 0, sizeof(buf));
	do
	{
		lcdGoto(120, 7);
		lcdDisplay(0, 7, DISP_CFONT, "           $");
		lcdFlip();
		iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
		{
			return E_TRANS_CANCEL;
		}		
		amt = atol((char *)buf);
		break;
	}while( /*amt==0 &&*/ flag!=TIP_AMOUNT );  // NETPAY 要求金额等于0提示错误

    	if( stTemp.iTransNo == POS_PREAUTH && PosCom.stTrans.TransFlag == POS_PREAUTH)
	{
		tmp = amt;   //NETPAY	
		PosCom.stTrans.lPreAuthOriAmount = amt;// save preauth ori amount(LONG)
		lpreAuthTipAmount = tmp * stPosParam.ucTipper/100;
		tmp += lpreAuthTipAmount;
		if( amt>tmp )
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "INVALID AMOUNT");
			DispMulLanguageString(0, 6, DISP_HFONT16|DISP_MEDIACY, NULL, "EXCEED FEE");
			lcdFlip();
			ErrorBeep();
			kbGetKeyMs(2000);
			return NO_DISP;

		}
		sprintf((char *)szOriAmount,"%012d",amt);//预授权原始金额
		AscToBcd(PosCom.stTrans.preAuthOriAmount, szOriAmount, 12);
		
		sprintf((char*)szTipAmount,"%012d",lpreAuthTipAmount);
		AscToBcd(PosCom.stTrans.preAuthTipAmount, szTipAmount, 12);
		
		sprintf((char *)szAmount, "%012d", tmp);
		AscToBcd(PosCom.stTrans.sAmount, szAmount, 12);// 预授权叠加总金额
	}
	else 
	{
		sprintf((char *)szAmount, "%012d", amt);
		if(stPosParam.stVolContrFlg ==PARAM_OPEN)
		{
			orignal_amt = BcdToLong(PosCom.stTrans.sAmount, 6);	
			if(amt > orignal_amt)
			{
				return 	E_REINPUT_AMOUNT;
			}
		}
		AscToBcd(PosCom.stTrans.sAmount, szAmount, 12);
	}

	if( memcmp(PosCom.stTrans.sAmount,"\x00\x00\x00\x00\x00\x00",6) == 0 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "MONTO INVALIDO");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);
		return NO_DISP;
	}
	
	return(OK);
}

/********************
***输入小费金额
*********************/
uint8_t AppGetTip()
{
	uint8_t	buf[17], szAmount[16],szOriAmount[16],szTipAmount[16];
	int	amt, iRet,ucRet;
	uint32_t	tmp;
	uint32_t    lpreAuthTipAmount;
	char *temp =NULL;
	uint8_t tmp_hightip[12] ={0};
	uint8_t tmp_lowtip[5] ={0};
	int c8tip,tiplen;

INPUTTIP:		
	lcdClrLine(2, 7);

	lcdCls();
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VENTA ");
	lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "INDIQUE PROPINA");
	lcdFlip();

	memset(szAmount, 0, 16);
	memset(buf, 0, sizeof(buf));
	do{
		lcdGoto(120, 7);
		lcdDisplay(0, 7, DISP_CFONT, "           $");
		lcdFlip();
		iRet = kbGetString(KB_EN_NUM+KB_EN_FLOAT+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet<0 )
		{
			return E_TRANS_CANCEL;
		}		
		amt = atol((char *)buf);
		break;
	}while( 1 );  // NETPAY 要求金额等于0提示错误

	
	temp= strchr(PosCom.stTrans.max_tip,'.');
	if(temp!=NULL)
	{
		//printf("stTemp.max_tip:%s\n",PosCom.stTrans.max_tip);
		PrintDebug("%s %s", "PosCom.stTrans.max_tip:",PosCom.stTrans.max_tip);
		tiplen = temp -PosCom.stTrans.max_tip;
		memcpy(tmp_hightip,PosCom.stTrans.max_tip,tiplen);
		memcpy(tmp_lowtip,PosCom.stTrans.max_tip+tiplen+1,strlen(PosCom.stTrans.max_tip)-tiplen);
		c8tip = atol((char*)tmp_hightip)*100 +atol((char*)tmp_lowtip);
	}
	else
	{
		c8tip =9999;
	}
	if(amt > c8tip)
	{
		lcdCls();
		//lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");//DISP_HFONT16
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "MONTO PROPINA EXCEDIDA");
		lcdFlip();
		for(ucRet=0;ucRet <3;ucRet++)
		{
			sysBeep();
			sysDelayMs(500);
		}
		goto INPUTTIP;
	}
	else
	{
		sprintf((char *)szAmount, "%012d", amt);
		AscToBcd(PosCom.stTrans.sTipAmount, szAmount, 12);
	}

	
	if( memcmp(PosCom.stTrans.sTipAmount,"\x00\x00\x00\x00\x00\x00",6) == 0 )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 4, DISP_HFONT16|DISP_MEDIACY, NULL, "MONTO INVALIDO");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(2000);
		return NO_DISP;
	}

	return(OK);
}
/************************
***重新的输入NIP
*************************/
int AppInputNip()
{
	int  iRet,ucRet;
	uint8_t buf[10];
	uint8_t buf2[10];
	uint8_t len =0;
	while(1)
	{	
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");//DISP_HFONT16

		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NUEVO NIP");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdGoto(120, 5);
		iRet = kbGetString(KB_BIG_PWD, 0, 4, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return NO_DISP;
		}
		if(iRet !=4)
		{
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");//DISP_HFONT16
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "INGRESE 4 DIGITOS");
			lcdFlip();
			for(ucRet=0;ucRet <3;ucRet++)
			{
				sysBeep();
				sysDelayMs(500);
			}
			continue;
		}

		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "CONFIRME NIP");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdGoto(120, 5);
		iRet = kbGetString(KB_BIG_PWD, 0, 4, (stPosParam.ucOprtLimitTime*1000), (char *)buf2);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return NO_DISP;
		}
		len = strlen(buf);
		if(strcmp(buf,buf2) !=0 || len!=4)
		{
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "DATO INCORRECTO");
			lcdFlip();
			for(ucRet=0;ucRet <3;ucRet++)
			{
				sysBeep();
				sysDelayMs(500);
			}
			continue;
		}
		else
		{
			memcpy(PosCom.stTrans.nnip,buf,len);
			return OK;
		}

	}
	return OK;
}

/************************
***输入driver num and nip
*************************/
uint8_t AppInputDrivernum_nip(int flag)
{
	uint8_t iRet;
	uint8_t buf[12];
	uint8_t buf2[12];
	uint8_t len =0;
	
	lcdCls();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");//DISP_HFONT16

	DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NUEVO DRIVER NUM");
	lcdFlip();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	lcdGoto(120, 5);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
	{
		return NO_DISP;
	}

	len = strlen(buf);
	memcpy(PosCom.stTrans.Driver_num,buf,len);
	

	if(flag ==1)
	{
		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");//DISP_HFONT16

		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "NUEVO DRIVER NIP");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdGoto(120, 5);
		memset(buf,0,sizeof(buf));
		memset(buf2,0,sizeof(buf2));
		
		iRet = kbGetString(KB_BIG_PWD, 0, 4, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return NO_DISP;
		}

		lcdCls();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "CONFIRME DRIVER NIP");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdGoto(120, 5);
		iRet = kbGetString(KB_BIG_PWD, 0, 4, (stPosParam.ucOprtLimitTime*1000), (char *)buf2);
		if( iRet==KB_CANCEL || iRet==KB_TIMEOUT )
		{
			return NO_DISP;
		}
		len = strlen(buf);
		if(strcmp(buf,buf2) !=0)
		{
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "INPUT DRIVER NUM ERROR");

			if (kbGetKey() == KEY_CANCEL)
			{
				return NO_DISP;
			}
		}
		else
		{
			memcpy(PosCom.stTrans.Driver_nip,buf,len);
		}
	}

	return OK;
}

/************************
***输入bomba
*************************/
uint8_t AppGetBomba()
{
	int  iRet;
	uint8_t buf[10];
	uint8_t buf2[100];
	
	
	if( PosCom.stTrans.TransFlag == PAYMENT_WITH_CARD )
	{
		sprintf((char*)buf2,"PAGO A NETPAY CON TARJETA %s",PosCom.stTrans.AscCardType);
	}
	else if( stTemp.iTransNo==NETPAY_REFUND )
	{
		sprintf((char*)buf2,"DEVOLUCION %s",PosCom.stTrans.AscCardType);
	}
	else if( stTemp.iTransNo==NETPAY_FORZADA )
	{
		sprintf((char*)buf2,"VENTA FORZADA %s",PosCom.stTrans.AscCardType);
	}
	else
	{
		sprintf((char*)buf2,"VENTA %s",PosCom.stTrans.AscCardType);
	}
	
	lcdCls();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
	DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Numero de Bomba");
	lcdFlip();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	lcdGoto(120, 5);
	iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 4, (stPosParam.ucOprtLimitTime*1000), (char *)buf);
	if( iRet < 0)
	{
		return NO_DISP;
	}
	else
	{
		memset(PosComconTrol.szBomba,0,sizeof(PosComconTrol.szBomba));
		strcpy(PosComconTrol.szBomba,(char*)buf);
	}
	return OK;

}

/**************************************************
输入控制量，比如:kilomiter、placas、nip
***************************************************/
uint8_t AppGetCtlvalue(uint8_t flag)
{
	uint8_t iRet,ucRet;
	uint8_t buf[20]={0};
	uint8_t buf2[100];
	uint8_t old_transid =0;
	uint8_t old_transflag =0;
	char* Temp2;
	char menuflag1,menuflag2,menuflag3;
	
	if(flag !=PURSE_PUNTO && flag != PURSE_SODEXO && flag !=PURSE_TODITO&& flag !=PURSE_EDENRED)
	{	
		return OK;
	}
	
	if(stPosParam.stVolContrFlg == PARAM_CLOSE)
	{
		//choice produce
			Temp2 = strchr(PosCom.stTrans.oil_id,'1');
			if(Temp2 !=NULL)
			{
				menuflag1 ='1';
			}
			Temp2 = strchr(PosCom.stTrans.oil_id,'2');
			if(Temp2 !=NULL)
			{
				menuflag2 ='1';
			}
			Temp2 = strchr(PosCom.stTrans.oil_id,'3');
			if(Temp2 !=NULL)
			{
				menuflag3 ='1';
			}
			if(menuflag1 ==0 && menuflag2 ==0 && menuflag3 ==0)
			{
				printf("GetRules_FromBankPlat->Y1 ERROR!!!\n");
				return NO_DISP;
			}
			lcdCls();
			InitMenu(MENU_MODE_1, "SELECIONE PRODUCTO");
			MainMenuAddMenuItem(menuflag1,    1,       		   "MAGNA              ",         NULL);		
			MainMenuAddMenuItem(menuflag2,    2,       		   "PREMIUM             ",         NULL);
			MainMenuAddMenuItem(menuflag3,    3,       		   "DIESEL",         NULL);
		

		    	iRet = DispDynamicMenu(1);
			
			if( iRet == 255 )		//NO_TRANS
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.oil_id,0,sizeof(PosCom.stTrans.oil_id));
				switch(iRet)
				{
					case 1:
						PosCom.stTrans.oil_id[0] = '1';
						break;
					case 2:
						PosCom.stTrans.oil_id[0] = '2';
						break;
					case 3:
						PosCom.stTrans.oil_id[0] = '3';
						break;
					default:
						break;
				}
			}
	}
	
	switch(PosCom.stTrans.TransFlag)
	{
		case PURSE_PUNTO:
			old_transid = stTemp.iTransNo;
			stTemp.iTransNo= PURSE_GETRULE;
			iRet = GetRules_FromBankPlat(stTemp.iTransNo);
			stTemp.iTransNo = old_transid;
			
			if(iRet !=OK)
			{
				return iRet;
			}
		#if 1
			//input placas
		INPUT_PLACE:
		
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Ingrese Placas:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
			iRet = PubGetString(ALPHA_IN|ECHO_IN,0,8,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.plate,0,sizeof(PosCom.stTrans.plate));
				strcpy(PosCom.stTrans.plate,(char*) buf);
				if(strcmp(PosCom.stTrans.origi_plate,PosCom.stTrans.plate) !=0)
				{
					memset(PosCom.stTrans.plate,0,sizeof(PosCom.stTrans.plate));
					memset(buf,0,sizeof(buf));

					lcdCls();
					lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
					DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");//DISP_HFONT16
					DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "DATO INCORRECTO");
					lcdFlip();
					for(ucRet=0;ucRet <3;ucRet++)
					{
						sysBeep();
						sysDelayMs(500);
					}
					goto INPUT_PLACE;
				}
			}
			//input kilometer
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Kilometraje:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
			iRet = PubGetString(NUM_IN,0,6,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.kilometer,0,sizeof(PosCom.stTrans.kilometer));
				strcpy(PosCom.stTrans.kilometer,(char*) buf);
			}

			//Ingrese NIP
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Ingrese NIP:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);

			iRet = PubGetString(NUM_IN,0,4,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.nip,0,sizeof(PosCom.stTrans.nip));
				strcpy(PosCom.stTrans.nip,(char*) buf);
			}

		#endif
			break;
		case PURSE_SODEXO:
			#if 1
			//input placas
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Ingrese Placas:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
	
			iRet = PubGetString(ALPHA_IN|ECHO_IN,0,8,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.plate,0,sizeof(PosCom.stTrans.plate));
				strcpy(PosCom.stTrans.plate,(char*) buf);
			}



			//input kilometer
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Kilometraje:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
			iRet = PubGetString(ALPHA_IN|ECHO_IN,0,6,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.kilometer,0,sizeof(PosCom.stTrans.kilometer));
				strcpy(PosCom.stTrans.kilometer,(char*) buf);
			}


			//Ingrese NIP
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Ingrese NIP:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);
			//iRet = kbGetString(KB_EN_NUM+KB_EN_BIGFONT+KB_EN_REVDISP+KB_EN_SHIFTLEFT, 0, 10, (stPosParam.ucOprtLimitTime*1000), (char *)buf);

			iRet = PubGetString(PASS_IN,0,4,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.nip,0,sizeof(PosCom.stTrans.nip));
				strcpy(PosCom.stTrans.nip,(char*) buf);
			}
		#endif
			break;
		case PURSE_TODITO:
			//Ingrese NIP
			memset(buf,0,sizeof(buf));
			lcdCls();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			DispMulLanguageString(0, 0, DISP_HFONT16|DISP_MEDIACY|DISP_INVLINE, NULL, (char*)"VENTA");
			DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Ingrese Nip:");
			lcdFlip();
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			lcdGoto(120, 5);

			iRet = PubGetString(PASS_IN,0,4,buf,stPosParam.ucOprtLimitTime,0);
			if( iRet!=0 )
			{
				return NO_DISP;
			}
			else
			{
				memset(PosCom.stTrans.nip,0,sizeof(PosCom.stTrans.nip));
				strcpy(PosCom.stTrans.nip,(char*) buf);
			}
			break;
		case PURSE_EDENRED:
			old_transflag = PosCom.stTrans.iTransNo;
			old_transid = stTemp.iTransNo;
			
			PosCom.stTrans.iTransNo= PURSE_GETBALANCE;
			stTemp.iTransNo= PURSE_GETBALANCE;

			iRet = GetBalance_FromBankPlat(PosCom.stTrans.TransFlag);
			PosCom.stTrans.iTransNo = old_transflag;
			stTemp.iTransNo = old_transid;
			if(iRet !=OK)
			{
				return iRet;
			}
			break;
		default:
			break;
	}	
	
	return OK;
}
uint8_t AppConfirmAmount(void)
{
	uint8_t	buf[17];
	int	iKey;

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "CONFIRM AMOUNT:");
	memset(buf, 0, sizeof(buf));
	ConvBcdAmount(PosCom.stTrans.sAmount, buf);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	lcdDisplay(0, 6, DISP_CFONT, "[CANCEL]MODIFY");
	lcdFlip();
	while(1)
	{
		iKey = kbGetKeyMs(stPosParam.ucOprtLimitTime*1000);
		if( iKey==KEY_ENTER )
			return(OK);
		else if( iKey==KEY_CANCEL )
			return E_TRANS_CANCEL;
		else if( iKey==KEY_TIMEOUT )
			return E_TRANS_FAIL;
	}

	return(OK);
}

void GetPosTime(uint8_t *pszDate, uint8_t *pszTime)
{
	uint8_t sDateTime[8];

	sysGetTime(sDateTime);

	memcpy(pszDate, "20", 2);

	BcdToAsc0(&pszDate[2], sDateTime, 6);
	BcdToAsc0(pszTime, &sDateTime[3], 6);
}

int  WaitEvent(void)
{
	uint8_t	ucRet=0, ucReadCnt,ucMode;
	uint8_t	szLastTime[6+1],szSignTime[4+1],sDispBuf1[30],sDispBuf2[30];
	uint8_t	szDate[8+1];
	uint8_t	szTime[6+1];
	char szDateTime[128];
	uint8_t   ucRet1;
	int   iKey=0,ierrno;
	int   iCnt = 0;
	int   iOldTime;
	int   ibacklight_set_time_bak;
	int   iRet;
	int   InsertTime = 0;

	AppMagClose();
	AppMagOpen();//8110 close magnetic when contactless
	magReset();
	iOldTime = sysGetTimerCount();
	if (stPosParam.ucqPbocSupport == PARAM_OPEN && imif_fd >=0)
	{
		contactless_close(imif_fd);
		imif_fd = -1;
	}
	ledSetStatus(LED_IDLE);

	iCnt = ucReadCnt = 0;
	kbFlush();
	sprintf((char *)szLastTime, "000000");
	sprintf((char *)szSignTime, "0000");
	while( 1 )
	{
		if( kbhit()==YES )
		{
			iKey = kbGetKey();
			if(PosCom.ucFallBack)
			{
				if(iKey==KEY_CANCEL)
				{
					PosCom.ucFallBack = FALSE;
					return NO_DISP;
				}
				else
				{
					continue;
				}
			}
			else
			{
				return iKey;
			}
		}

		if( magSwiped()==YES )
		{
			//Sleep can not solve
			sysGetbacklighttime(&ibacklight_set_time_bak);
			sysSetbacklighttime(-1);
			sysSetbacklighttime(ibacklight_set_time_bak);

			ucRet = SwipeCardProc(TRUE);
			if( ucRet!=OK )
			{
				if( ucRet==E_ERR_SWIPE )
				{
					DispResult(E_ERR_SWIPE);
					return NO_DISP;
				}
				if( ucRet==E_NEED_INSERT )
				{
					ucMode = CARD_INSERTED;
REINSERT:
					ShowICCardScreen(ucMode,sDispBuf1,sDispBuf2);
					ucRet1 = DetectCardEvent(ucMode,sDispBuf1,sDispBuf2,&ierrno);
					if( ucRet1==CARD_KEYIN )
					{
						iKey = kbGetKey();
						if( iKey==KEY_CANCEL )
						{
							return E_TRANS_CANCEL;
						}
						else
						{
							goto REINSERT;
						}
					}
					else if( ucRet1==CARD_INSERTED )
					{
						CheckCapkAid();
						if( ucRet1==E_NEED_FALLBACK )
						{
							PosCom.ucFallBack = TRUE;
							strcpy((char *)sDispBuf1,"READ ERR");
							strcpy((char *)sDispBuf2,"SWIPE-->");
							ucRet1 = DetectCardEvent(CARD_SWIPED,sDispBuf1,sDispBuf2,&ierrno);
							if (ucRet1 != CARD_SWIPED)
							{
								return NO_DISP;
							}

							continue;
						}
						else if( ucRet1==E_CARD_BLOCKER )
						{
							lcdClrLine(2, 7);
							lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "CARD LOCKED");
							lcdFlip();
							ErrorBeep();
							kbGetKeyMs(2000);

							DispRemoveICC();
							return NO_DISP;
						}
						else if( ucRet1==E_APP_BLOCKED )
						{
							lcdClrLine(2, 7);
							lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "APP LOCKED");
							lcdFlip();
							ErrorBeep();
							kbGetKeyMs(2000);

							DispRemoveICC();
							return NO_DISP;
						}
						else if( ucRet1==E_TRANS_FAIL )
						{
							lcdClrLine(2, 7);
							lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "TRAN REFUSE");
							lcdFlip();
							ErrorBeep();
							kbGetKeyMs(2000);

							DispRemoveICC();
							return NO_DISP;
						}
						else
						{
							return ucRet1;
						}
					}
				}
				return ucRet;
			}
			else if( ucRet==OK )
			{
				start_from_manager =1;
				iRet =GetCardType();
				switch(PosCom.stTrans.CardType)
				{
					case 4:
						PosCom.stTrans.TransFlag = PURSE_PUNTO;
						stTemp.iTransNo = PURSE_SALE;
						break;
					case 5:
						PosCom.stTrans.TransFlag = PURSE_SODEXO;
						stTemp.iTransNo = PURSE_SALE;
						break;
					case 6:
						PosCom.stTrans.TransFlag =PURSE_TODITO;
						stTemp.iTransNo = PURSE_SALE;
						break;
					case 7:
						PosCom.stTrans.TransFlag = PURSE_EDENRED;
						stTemp.iTransNo = PURSE_SALE;
						break;
					default:
						stTemp.iTransNo = POS_SALE;
						break;
				}
			
				ucRet = ProcSelectTran();	// process transaction
				if( ucRet==E_NEED_FALLBACK )
				{
					CommOnHook(FALSE);
					lcdClrLine(2,7);
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "INSERT ERROR");
					DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL, "PLS SWIPE");
					lcdFlip();
					WaitRemoveICC();
					PosCom.ucFallBack    = TRUE;
					PosCom.bSkipEnterAmt = TRUE;
					PosCom.ucSwipedFlag  = NO_SWIPE_INSERT;
					ucRet = PosGetCard(CARD_SWIPED);
					if( ucRet==OK )
					{
						ucRet = ProcSelectTran();
					}
				}			
				DispResult(ucRet);	// 显示交易结果 
				return NO_DISP;
			}
		}
RE_INSERT:
		if( (stPosParam.ucEmvSupport==PARAM_OPEN) && (PosCom.ucFallBack==FALSE) &&
			(iccDetect(ICC_USERCARD)==ICC_SUCCESS) )
		{
			// Sleep can not solve
			start_from_manager =1;

			sysGetbacklighttime(&ibacklight_set_time_bak);
			sysSetbacklighttime(-1);
			sysSetbacklighttime(ibacklight_set_time_bak);

			CheckCapkAid();
			
			ucRet = 0;
			ucRet = Direct_InsertCardProc(TRUE);

			if( ( InsertTime<2 ) && ( ucRet==E_NEED_FALLBACK ) )
			{
				PosCom.ucFallBack = FALSE;
				InsertTime++;
				goto RE_INSERT;
			}
			if( ucRet==E_NEED_FALLBACK )
			{
				ucReadCnt++;
				PosCom.ucFallBack = TRUE;
				lcdFlip();
				kbGetKeyMs(1000);
				if (ucReadCnt > 2)
				{
					ucReadCnt = 0;
					return NO_DISP;
				}
				if( stPosParam.EmvFallBack != PARAM_OPEN )
				{
					return NO_DISP;
				}
				else
				{
					continue;
				}
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
				DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " TRANS REJECT");
				lcdFlip();
				ErrorBeep();
				kbGetKeyMs(2000);

				DispRemoveICC();
				return NO_DISP;
			}
			ucRet = ProcSelectTran();	// process transaction
			if( ucRet==E_NEED_FALLBACK )
			{
				CommOnHook(FALSE);
				lcdClrLine(2,7);
				DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "INSERT ERROR");
				DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, NULL, "PLS SWIPE");
				lcdFlip();
				WaitRemoveICC();
				PosCom.ucFallBack    = TRUE;
				PosCom.bSkipEnterAmt = TRUE;
				PosCom.ucSwipedFlag  = NO_SWIPE_INSERT;
				ucRet = PosGetCard(CARD_SWIPED);
				if( ucRet==OK )
				{
					ucRet = ProcSelectTran();
				}
			}

			DispResult(ucRet);	// 显示交易结果 	
			return NO_DISP;
		}

		if( (sysGetTimerCount() - iOldTime) >= (uint32_t)(stPosParam.ucOprtLimitTime * 1000) )
		{
			return NO_DISP;
		}

		if(PosCom.ucFallBack == TRUE)
		{
			sysDelayMs(10); // Avoid CPU resources occupation
			continue;
		}

		GetPosTime(szDate, szTime);
		if( memcmp(szLastTime, szTime, 6)!=0 )
		{
			sprintf(szDateTime, "%.2s/%.2s/%.4s    %.2s:%.2s:%.2s",szDate+6, szDate+4, szDate, szTime, szTime+2, szTime+4);
			if (gstPosCapability.uiScreenType)	//8210
			{
				lcdDisplay(0, 10, DISP_ASCII|DISP_MEDIACY|DISP_FLIP, szDateTime);
			}
			else
			{
				lcdDisplay(0, 6, DISP_ASCII|DISP_MEDIACY|DISP_FLIP, szDateTime);
			}

			sprintf((char *)szLastTime, "%.6s", szTime);
		}
		if ( memcmp(szSignTime, szTime, 4)!=0 )
		{
			DispWirelessSignal();
			sprintf((char *)szSignTime, "%.4s", szTime);
		}
		//暂时不用，需要的时候再用 (63域 返回 ZY00001 1! 1检测更新，0 不检测更新)
		//PrintDebug("%s:%s", "update_flag",PosComconTrol.AutoUpdateFlag);
		if( atoi((char*)PosComconTrol.AutoUpdateFlag) == 1 )
		{
			if(sysGetTimerCount()-iOldTime > 5000)  //5s
			{
				iOldTime = sysGetTimerCount();
				ucRet = DealTmsFunc(0);
				//每收到一次自动更新标志，只向tms后台请求更新一次
		    		//memset(PosComconTrol.AutoUpdateFlag,0,sizeof(PosComconTrol.AutoUpdateFlag));
				if(ucRet==E_APP_EXIT || ucRet==E_APP_RESET)
				{
					return ucRet;
				}
			}
		}
		

		sysDelayMs(10); // Avoid CPU resources occupation
	}
}

#if 0
uint8_t SelectTrans(void)
{
    int key;
	int keySub;

	const char *mainMenu[] =
	{
		" MENU PRINCIPAL",
		"1-TARJ.CREDITO/DEBITO",
		"2-PBA.COMUNIC VENTA TIEMPO AIRE ",
		"3-CONFIGURACION ",
		"4-REPORTES ",
	};

	const char *cashAdvacneMenu[] =
	{
		"TARJ.CREDITO/DEBITO",
		"1-RETIRO   ", 
		"2-CIERRE   ",        // settlement    
	};

	#if 0
	const char *saleMenu[] =
	{
		"TARJ.CREDITO/DEBITO",
		"1-CHECK IN    ", 
		"2-CHECK OUT   ", 
		"3-VENTA        ",          // sale
		"4-CIERRE  ",        // settlement    
	};

	const char *PreAuthMenu[] =
	{
		"TARJ.CREDITO/DEBITO",
		"1-CHECK IN    ", 
		"2-CHECK OUT   ", 
		"3-VENTA       ",          // sale
		"4-CIERRE       ",         // settlement
		"5-CONFIRMACION PROPINA"
	};
	#endif

	const char *PayMenu[] =
	{
		"MI CUENTA",
		"1-Consultar Saldo        ",          // balance check
		"2-Pago a netpay con ventas ",         // pay with balance
		"3-Pago a netpay con tarjeta ",         // pay with card
		"4-Consulta movs. linea "  				// query the last movement
	};

	#if 0
	const char* manMenu[] =
	{
		"1.LOGON 2.LOGOFF",
		"3.QUERY 4.OPER",
		"5.PABX  6.SETTLE",
		"7.LOCK  8.VERSION"
	};
	#endif

	#if 0
	const char *query_menu[] =
	{
		"TRANS QUERY",
		"1.QUERY LIST",
		"2.QUERY TOTAL",
		"3.INVOICE QUERY"
	};
	#endif

/*	const char *off_menu[] =
	{
		"OFFLINE",
		"1.OFF-SETTLE",
		"2.OFF-ADJUST"
	};
*/
	const char *else_menu[] =
	{
		"1.E-CASH",
		"2.INSTALMENT",
		"3.BALANCE",
		"4.BALANCE CHECK",
		"5.LAST MOVEMENT"
	};

/*	const char* pre_menu[] =
	{
		"1.PREAUTH",
		"2.PREAUTH COMP",
		"3 PREAUTH OFFL",
		"3.VOID PREAUTH",
		"4.VOID PRE-COMP",
		
	};
*/
	while(1)
	{
		key = DispMenu2(mainMenu, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		switch(key)
		{
		#if 0
		case KEY1:
			if(stPosParam.szpreAuth == PARAM_OPEN)
			{
				keySub = DispMenu(PreAuthMenu, 6, 1, KEY1, KEY5, stPosParam.ucOprtLimitTime);
				if( keySub==KEY1 )
					stTemp.iTransNo = CHECK_IN;
				else if( keySub==KEY2 )
					stTemp.iTransNo = CHECK_OUT;
				else if( keySub==KEY3 )
					stTemp.iTransNo = POS_PREAUTH;
				#if 0
				else if( keySub==KEY4 )
					stTemp.iTransNo = POS_REFUND;
				#endif
				else if( keySub==KEY4 )
					stTemp.iTransNo = POS_SETT;
				else if( keySub==KEY5 )
					stTemp.iTransNo = POS_AUTH_CM;
				else
					continue;
				return OK;
				break;
			}
			else
			{
				keySub = DispMenu3(saleMenu, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
				if( keySub==KEY1 )
					stTemp.iTransNo = CHECK_IN;
				else if( keySub==KEY2 )
					stTemp.iTransNo = CHECK_OUT;
				else if( keySub==KEY3 )
					stTemp.iTransNo = POS_SALE;
				#if 0
				else if( keySub==KEY4 )
					stTemp.iTransNo = POS_REFUND;
				#endif
				else if( keySub==KEY4 )
					stTemp.iTransNo = POS_SETT;
				else
					continue;
				return OK;
				break;
			}

			#endif
		case KEY1:
			keySub = DispMenu4(cashAdvacneMenu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = CASH_ADVACNE;
			else if( keySub==KEY2 )
				stTemp.iTransNo = POS_SETT;
			else
				continue;
			return OK;
			break;
		
		case KEY9:
			stTemp.iTransNo = PHONE_TOPUP;
			return OK;
			break;
		case KEY6:
			keySub = DispMenu(PayMenu, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = BALANCE_CHECK;
			else if( keySub==KEY2 )
				stTemp.iTransNo = PAYMENT_WITH_BALANCE_CHECK;
			else if( keySub==KEY3 )
				stTemp.iTransNo = PAYMENT_WITH_CARD_CHECK;
			else if( keySub==KEY4 )
				stTemp.iTransNo = LAST_MOVEMENT;
			else
				continue;
			return OK;
			break ;
		case KEY2:
			stTemp.iTransNo = ECHO_TEST;
/*			keySub = DispMenu(pre_menu, 5, 0, KEY1, KEY5, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = POS_PREAUTH;
			else if( keySub==KEY2 )
				stTemp.iTransNo = POS_AUTH_CM;
			else if( keySub==KEY3 )
				stTemp.iTransNo = POS_OFF_CONFIRM;
			else if( keySub==KEY4 )
				stTemp.iTransNo = POS_PREAUTH_VOID;
			else if( keySub==KEY5 )
				stTemp.iTransNo = POS_AUTH_VOID;
			else
				continue;
*/			return OK;
			break;
		case KEY3:
			SetParaFunc();
			return NO_DISP;
/*			keySub = DispMenu(off_menu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = OFF_SALE;
			else if( keySub==KEY2 )
				stTemp.iTransNo = OFF_ADJUST;
			else
				continue;
			return OK;
			break;
*/		case KEY4:
			if( SelectPrintFunc() )
				continue;
			else
				return OK;
			break;
		case KEY5:
			stTemp.iTransNo = POS_REFUND;
			return OK;
		#if 0
		case KEY7:
			keySub = DispMenu(manMenu, 4, 0, KEY1, KEY8, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
			{
				stTemp.iTransNo = POS_PREAUTH;
			}
			else if( keySub==KEY2 )
			{
/*				if( stTransCtrl.iTransNum!=0 )
				{
					lcdCls();
					DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     LOGOUT     ");
					DispMulLanguageString(0, 2, DISP_CFONT, NULL, " SETTLE FIRST");
					DispMulLanguageString(0, 4, DISP_CFONT, NULL, " LOGOUT LATE ");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);
					return NO_DISP;
				}
*/				stTemp.iTransNo = POS_AUTH_CM;
			}
			else if( keySub==KEY3 )
			{
				keySub = DispMenu(query_menu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
				if( keySub==KEY1 )
					stTemp.iTransNo = QUE_BATCH_MX;
				if( keySub==KEY2 )
					stTemp.iTransNo = QUE_BATCH_TOTAL;
				if( keySub==KEY3 )
					stTemp.iTransNo = QUE_BATCH_ANY;
			}
			else if( keySub==KEY4 )
			{
				TellerManageMenu();
				continue;
			}
			else if( keySub==KEY5 )
			{
				stTemp.iTransNo = SETUP_EXTNUM;
			}
			else if( keySub==KEY6 )
				stTemp.iTransNo = POS_SETT;
			else if( keySub==KEY7 )
			{
				strcpy((char *)stTransCtrl.szOldTellerNo, (char *)stTransCtrl.szNowTellerNo);
				strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_LOCK);
				SaveCtrlParam();
				stTemp.iTransNo = NO_TRANS_SELECT;
				lcdSetIcon(ICON_LOCK, OPENICON);
			}
			else if( keySub==KEY8 )
			{
				stTemp.iTransNo = OPER_QUE_TERM_INFO;
			}
			else
				continue;
			return OK;
		#endif
		case KEY8:
			keySub = DispMenu(else_menu, 5, 0, KEY1, KEY5, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
			{
				if( SelectEcFunc() )
					continue;
				else
					return OK;
			}
			else if( keySub==KEY2 )
			{
				if( SelectInstalFunc() )
					continue;
				else
					return OK;
			}
			else if( keySub==KEY3 )
			{
				stTemp.iTransNo = POS_QUE;
			}
			else if( keySub==KEY4 )
			{
				stTemp.iTransNo = BALANCE_CHECK;
			}
			else if( keySub==KEY5 )
			{
				stTemp.iTransNo = LAST_MOVEMENT;
			}
			
			else
				continue;
			return OK;

			break;
		case KEY_ENTER: //1~2
			break;
		default:
			return NO_DISP;
			break;
		}
	}
	return(NO_DISP);
}

#endif



#if 1
int SelectTrans(void)
{
    	int key;
	int keySub;
	int  iRet;

	const char *mainMenu[] =
	{
		" MENU PRINCIPAL",
		"1-Venta",
		"2-Cierre ",
		"3-Reportes ",
		"4-Config ",
		"5-Prueba Comm ",
		"6-Consultar Saldo ",
		"7-Rpt. Control Gas ",
	};

	const char *PayMenu[] =
	{
		"MI CUENTA",
		"1-VENTA NORMAI ",          // normal sale
		"2-MONEDERO",         // wallet consume
	};

	const char *else_menu[] =
	{
		"1.E-CASH",
		"2.INSTALMENT",
		"3.BALANCE",
		"4.BALANCE CHECK",
		"5.LAST MOVEMENT"
	};

	while(1)
	{
		key = DispMenu2(mainMenu, 8, 1, KEY1, KEY7, stPosParam.ucOprtLimitTime);
		switch(key)
		{
		case KEY1:
			iRet = DispVenta();
			if(iRet ==OK)
			{
				printf("transno: %d;stTrans.TransFlag:%d\n",stTemp.iTransNo,PosCom.stTrans.TransFlag);
				return OK;
			}
			else 
			{
				return NO_DISP;
			}
			break;
		case KEY9:
			stTemp.iTransNo = PHONE_TOPUP;
			return OK;
			break;

		case KEY2:
			stTemp.iTransNo = POS_SETT;
			return OK;
			break;
		case KEY3:
			InitMenu(MENU_MODE_1, "SELECCIONE REPORTE");
		
			MainMenuAddMenuItem(PARAM_OPEN,   PRT_TOTAL,	   "REPORTE DE TOTALES            ",         NULL);
			MainMenuAddMenuItem(PARAM_OPEN,   PRT_DETAIL,      "REPORTE DE DETALLES        ",         NULL);
			MainMenuAddMenuItem(PARAM_OPEN,   PRT_ANY,     	   "REIMPRESION        ",         NULL);


			iRet = DispDynamicMenu(1);
			printf("iRet =%d\n",iRet);
			if( iRet == NO_TRANS )		//NO_TRANS
			{
				return NO_DISP;
			}
			else
			{
				stTemp.iTransNo = iRet;
				return OK;	
			}
			break;
		case KEY4:
			SetParaFunc();
			//return OK;
			break;
		case KEY5:
			stTemp.iTransNo = ECHO_TEST;
			return OK;
			break;
		case KEY6:
			iRet =DispCheck_Balance();
			if(iRet ==OK)
			{
				printf("transno: %d;stTrans.TransFlag:%d\n",stTemp.iTransNo,PosCom.stTrans.TransFlag);
				return OK;
			}
			else 
			{
				return NO_DISP;
			}
			//return OK;
			break;
		case KEY7:
			stTemp.iTransNo = PRE_VOL_CTL;
			return OK;
			break;
		case KEY_ENTER: //1~2
			break;
		default:
			return NO_DISP;
			break;
		}
	}
	return(NO_DISP);
}
#else

uint8_t SelectTrans(void)
{
    int key;
	int keySub;

	const char *mainMenu[] =
	{
		" MENU PRINCIPAL",
		"1-TARJ.CREDITO/DEBITO",
		"2-VENTA TIEMPO AIRE ",
		"3-MI CUENTA",
		"4-PBA.COMUNIC",
		"5-CONFIGURACION",
		"6-REPORTES"
	};

	const char *saleMenu[] =
	{
		"TARJ.CREDITO/DEBITO",
		"1-VENTA        ",          // sale
		"2-CANCELACION ",         // cancle
		"3-CIERRE       "         // settlement
	};

	const char *PreAuthMenu[] =
	{
		"TARJ.CREDITO/DEBITO",
		"1-VENTA       ",          // sale
		"2-CANCELACION ",         // cancle
		"3-CIERRE       ",         // settlement
		"4-CONFIRMACION PROPINA"
	};


	const char *PayMenu[] =
	{
		"MI CUENTA",
		"1-Consultar Saldo        ",          // balance check
		"2-Pago a netpay con ventas ",         // pay with balance
		"3-Pago a netpay con tarjeta ",         // pay with card
		"4-Consulta movs. linea "  				// query the last movement
	};
	
	const char* manMenu[] =
	{
		"1.LOGON 2.LOGOFF",
		"3.QUERY 4.OPER",
		"5.PABX  6.SETTLE",
		"7.LOCK  8.VERSION"
	};

	const char *query_menu[] =
	{
		"TRANS QUERY",
		"1.QUERY LIST",
		"2.QUERY TOTAL",
		"3.INVOICE QUERY"
	};

/*	const char *off_menu[] =
	{
		"OFFLINE",
		"1.OFF-SETTLE",
		"2.OFF-ADJUST"
	};
*/
	const char *else_menu[] =
	{
		"1.E-CASH",
		"2.INSTALMENT",
		"3.BALANCE",
		"4.BALANCE CHECK",
		"5.LAST MOVEMENT"
	};

/*	const char* pre_menu[] =
	{
		"1.PREAUTH",
		"2.PREAUTH COMP",
		"3 PREAUTH OFFL",
		"3.VOID PREAUTH",
		"4.VOID PRE-COMP",
		
	};
*/
	while(1)
	{
		key = DispMenu2(mainMenu, 7, 1, KEY1, KEY6, stPosParam.ucOprtLimitTime);
		switch(key)
		{
		case KEY1:
			if(stPosParam.szpreAuth == PARAM_OPEN)
			{
				keySub = DispMenu(PreAuthMenu, 5, 1, KEY1, KEY6, stPosParam.ucOprtLimitTime);
				if( keySub==KEY1 )
					stTemp.iTransNo = POS_PREAUTH;
				else if( keySub==KEY2 )
					stTemp.iTransNo = POS_REFUND;
				else if( keySub==KEY3 )
					stTemp.iTransNo = POS_SETT;
				else if( keySub==KEY4 )
					stTemp.iTransNo = POS_AUTH_CM;
				else
					continue;
				return OK;
				break;
			}
			else
			{
				keySub = DispMenu3(saleMenu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
				if( keySub==KEY1 )
					stTemp.iTransNo = POS_SALE;
				else if( keySub==KEY2 )
					stTemp.iTransNo = POS_REFUND;
				else if( keySub==KEY3 )
					stTemp.iTransNo = POS_SETT;
				else
					continue;
				return OK;
				break;
			}
			
			
		case KEY2:
			stTemp.iTransNo = PHONE_TOPUP;
			return OK;
			break;
		case KEY3:
			keySub = DispMenu(PayMenu, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = BALANCE_CHECK;
			else if( keySub==KEY2 )
				stTemp.iTransNo = PAYMENT_WITH_BALANCE_CHECK;
			else if( keySub==KEY3 )
				stTemp.iTransNo = PAYMENT_WITH_CARD_CHECK;
			else if( keySub==KEY4 )
				stTemp.iTransNo = LAST_MOVEMENT;
			else
				continue;
			return OK;
			break ;
		case KEY4:
			stTemp.iTransNo = ECHO_TEST;
/*			keySub = DispMenu(pre_menu, 5, 0, KEY1, KEY5, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = POS_PREAUTH;
			else if( keySub==KEY2 )
				stTemp.iTransNo = POS_AUTH_CM;
			else if( keySub==KEY3 )
				stTemp.iTransNo = POS_OFF_CONFIRM;
			else if( keySub==KEY4 )
				stTemp.iTransNo = POS_PREAUTH_VOID;
			else if( keySub==KEY5 )
				stTemp.iTransNo = POS_AUTH_VOID;
			else
				continue;
*/			return OK;
			break;
		case KEY5:
			SetParaFunc();
			return NO_DISP;
/*			keySub = DispMenu(off_menu, 3, 1, KEY1, KEY2, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = OFF_SALE;
			else if( keySub==KEY2 )
				stTemp.iTransNo = OFF_ADJUST;
			else
				continue;
			return OK;
			break;
*/		case KEY6:
			if( SelectPrintFunc() )
				continue;
			else
				return OK;
			break;
		case KEY7:
			keySub = DispMenu(manMenu, 4, 0, KEY1, KEY8, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
			{
				stTemp.iTransNo = POS_PREAUTH;
			}
			else if( keySub==KEY2 )
			{
/*				if( stTransCtrl.iTransNum!=0 )
				{
					lcdCls();
					DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     LOGOUT     ");
					DispMulLanguageString(0, 2, DISP_CFONT, NULL, " SETTLE FIRST");
					DispMulLanguageString(0, 4, DISP_CFONT, NULL, " LOGOUT LATE ");
					lcdFlip();
					ErrorBeep();
					kbGetKeyMs(2000);
					return NO_DISP;
				}
*/				stTemp.iTransNo = POS_AUTH_CM;
			}
			else if( keySub==KEY3 )
			{
				keySub = DispMenu(query_menu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
				if( keySub==KEY1 )
					stTemp.iTransNo = QUE_BATCH_MX;
				if( keySub==KEY2 )
					stTemp.iTransNo = QUE_BATCH_TOTAL;
				if( keySub==KEY3 )
					stTemp.iTransNo = QUE_BATCH_ANY;
			}
			else if( keySub==KEY4 )
			{
				TellerManageMenu();
				continue;
			}
			else if( keySub==KEY5 )
			{
				stTemp.iTransNo = SETUP_EXTNUM;
			}
			else if( keySub==KEY6 )
				stTemp.iTransNo = POS_SETT;
			else if( keySub==KEY7 )
			{
				strcpy((char *)stTransCtrl.szOldTellerNo, (char *)stTransCtrl.szNowTellerNo);
				strcpy((char *)stTransCtrl.szNowTellerNo, TELLER_LOCK);
				SaveCtrlParam();
				stTemp.iTransNo = NO_TRANS_SELECT;
				lcdSetIcon(ICON_LOCK, OPENICON);
			}
			else if( keySub==KEY8 )
			{
				stTemp.iTransNo = OPER_QUE_TERM_INFO;
			}
			else
				continue;
			return OK;

		case KEY8:
			keySub = DispMenu(else_menu, 5, 0, KEY1, KEY5, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
			{
				if( SelectEcFunc() )
					continue;
				else
					return OK;
			}
			else if( keySub==KEY2 )
			{
				if( SelectInstalFunc() )
					continue;
				else
					return OK;
			}
			else if( keySub==KEY3 )
			{
				stTemp.iTransNo = POS_QUE;
			}
			else if( keySub==KEY4 )
			{
				stTemp.iTransNo = BALANCE_CHECK;
			}
			else if( keySub==KEY5 )
			{
				stTemp.iTransNo = LAST_MOVEMENT;
			}
			
			else
				continue;
			return OK;

			break;
		case KEY_ENTER: //1~2
			break;
		default:
			return NO_DISP;
			break;
		}
	}
	return(NO_DISP);
}
#endif

#if 0
uint8_t SelectPrintFunc(void)
{
	int keySub;
/*	const char* print_menu[] = {
		"1.PRINT LAST    ",
		"2.PRINT ANY     ",
		"3.PRINT DETAIL  ",
		"4.PRINT TOTAL   ",
		"5.PRINT LAST SETTLE"
	};
*/	const char* print_menu[] = {
		"SELECCIONE REPORTE",
		"1.Reporte de totales",
		"2.Reporte de detalles",
		"3.Reporte check in",
		"4.Reporte check out",
		"5.Reimpresion",
		"6.Reporte por mesero"
	};

	if( stPosParam.szpreAuth == PARAM_OPEN || 
		stPosParam.szpreTip == PARAM_OPEN)
	{
		keySub = DispMenu1(print_menu, 7, 1, KEY1, KEY6, stPosParam.ucOprtLimitTime);
		if( keySub==KEY1 )
			stTemp.iTransNo = PRT_TOTAL ;
		else if( keySub==KEY2 )
			stTemp.iTransNo = PRT_DETAIL;
		else if( keySub==KEY3 )
			stTemp.iTransNo = PRT_CHECK_IN ;
		else if( keySub==KEY4 )
			stTemp.iTransNo = PRT_CHECK_OUT;
		else if( keySub==KEY5 )
			stTemp.iTransNo = PRT_ANY ;
		else if( keySub==KEY6 )
			stTemp.iTransNo = PRT_TIP;
		else
			return NO_DISP;
	}
	else
	{
		keySub = DispMenu1(print_menu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
		if( keySub==KEY1 )
			stTemp.iTransNo = PRT_TOTAL;
		else if( keySub==KEY2 )
			stTemp.iTransNo = PRT_DETAIL;
		else if( keySub==KEY3 )
			stTemp.iTransNo = PRT_ANY ;
		else if( keySub==KEY4 )
			stTemp.iTransNo = PRT_TIP;
		else
			return NO_DISP;
	}

	
	return OK;
}

#else

uint8_t SelectPrintFunc(void)
{
//	int keySub;
	uint8_t iRet = 0;
/*	const char* print_menu[] = {
		"1.PRINT LAST    ",
		"2.PRINT ANY     ",
		"3.PRINT DETAIL  ",
		"4.PRINT TOTAL   ",
		"5.PRINT LAST SETTLE"
	};
	const char* print_menu[] = {
		"SELECCIONE REPORTE",
		"1.Reporte de totales",
		"2.Reporte de detalles",
		"3.Reimpresion",
		"4.Reporte por mesero"
	};*/

	InitMenu(MENU_MODE_1, "SELECCIONE REPORTE");

	MesesAddMenuItem(PARAM_OPEN,   PRT_TOTAL,	"REPORT DE TOTALES             ",         NULL);
	MesesAddMenuItem(PARAM_OPEN,   PRT_DETAIL,  "REPORT DE DETALLES        ",         NULL);
	MesesAddMenuItem(PARAM_OPEN,   PRT_ANY,  	"REIMPRESION         ",         NULL);

	iRet = DispDynamicMenu(1);
	if( iRet == 255 )		//NO_TRANS
		return NO_DISP;
	else
		stTemp.iTransNo = iRet;
	return OK;	

	#if 0
	if( stPosParam.szpreAuth == PARAM_OPEN || 
		stPosParam.szpreTip == PARAM_OPEN)
	{
		keySub = DispMenu1(print_menu, 5, 1, KEY1, KEY4, stPosParam.ucOprtLimitTime);
		if( keySub==KEY1 )
			stTemp.iTransNo = PRT_TOTAL ;
		else if( keySub==KEY2 )
			stTemp.iTransNo = PRT_DETAIL;
		else if( keySub==KEY3 )
			stTemp.iTransNo = PRT_ANY ;
		else if( keySub==KEY4 )
			stTemp.iTransNo = PRT_TIP;
		else
			return NO_DISP;
	}
	else
	{
		keySub = DispMenu1(print_menu, 4, 1, KEY1, KEY3, stPosParam.ucOprtLimitTime);
		if( keySub==KEY1 )
			stTemp.iTransNo = PRT_TOTAL;
		else if( keySub==KEY2 )
			stTemp.iTransNo = PRT_DETAIL;
		else if( keySub==KEY3 )
			stTemp.iTransNo = PRT_ANY ;
		else if( keySub==KEY4 )
			stTemp.iTransNo = PRT_TIP;
		else
			return NO_DISP;
	}
	#endif
	
	return OK;
}
#endif

uint8_t SelectKeyManageFunc(void)
{
    const char *mainmenu[] = {
        "Nmx Setup",
        "1.Edit App ID",
        "2.Smart RKI Download",
        "3.Smart TLE Download",
        "4.Change PIN",
        "5.Manual Key Entry",
        "6.Destroy Keys"
    };

	int key;
	key = DispMenu(mainmenu, 7, 1, KEY1, KEY6, stPosParam.ucOprtLimitTime);
    switch(key){
        case KEY1:
            break;
        case KEY2:
            stTemp.iTransNo = RKI_INJECTION;
            break;
        case KEY3:
            break;
        case KEY4:
            break;
        case KEY5:
            break;
        case KEY6:
            break;
    	default:
		    return NO_DISP;
    }
    return OK;
}
uint8_t SelectPosManFunc(void)
{
	const char *manmenu[]={
		"TERM MANAGE",
		"1.ECHO TEST",
		"2.PARA DOWNLOAD",
		"3.UPLOAD STATUS",
		"4.CAPK DOWNLOAD",
		"5.EMV PARA DOWN",
		"6.SYS SETUP",
		"7.EMV RESULT",
		"8.B_LIST DOWN",
		"9.FAIL LIST",
	};
	int key,iRet;


	key = DispMenu(manmenu, 10, 1, KEY0, KEY9, stPosParam.ucOprtLimitTime);
	switch(key) {
	case KEY1:
		stTemp.iTransNo = RESPONSE_TEST;
		break;
	case KEY2:
		if( CheckParamPass() != 1)
		{
			return NO_DISP;
		}
		stTemp.iTransNo = POS_DOWNLOAD_PARAM;
		break;
	case KEY3:
		stTemp.iTransNo = POS_UPLOAD_PARAM;
		break;
	case KEY4:
		if( CheckParamPass() != 1)
		{
			return NO_DISP;
		}
		stTemp.iTransNo = DOWNLOAD_EMV_CAPK;
		break;
	case KEY5:
		if( CheckParamPass() != 1)
		{
			return NO_DISP;
		}
		stTemp.iTransNo = DOWNLOAD_EMV_PARAM;
		break;
	case KEY6:
		SetParaFunc();
		return NO_DISP;
		break;
	case KEY7:
		if( CheckParamPass() != 1)
		{
			return NO_DISP;
		}
		ViewEmvResult();
		return NO_DISP;
		break;
	case KEY8:
		if( CheckParamPass() != 1)
		{
			return NO_DISP;
		}
		TransDownloadBlkCard();
		return NO_DISP;
		break;
	case KEY9:
		ViewIccFailureLog();
		return NO_DISP;
		break;
	case KEY0:
		iRet = ExitApplication();
		break;
	default:
		return NO_DISP;
	}
	return OK;
}

void dispRetCode(uint8_t ucFlag)
{
	int i;
	uint8_t tmpbuf[60], dispStr1[20];

	kbFlush();
	lcdClrLine(2, 7);
	if( (memcmp(PosCom.szRespCode, "00", 2)==0) )
	{
		return;
	}

	for(i=0; strlen((char*)sRetErrInfo[i].ucRetCode)!=0; i++)
	{
		if( memcmp(sRetErrInfo[i].ucRetCode, (char*)PosCom.szRespCode, 2)==0 )
			break;
	}
	memset(dispStr1,0,sizeof(dispStr1));
	memset(tmpbuf,0,sizeof(tmpbuf));
	if( strlen((char*)sRetErrInfo[i].ucRetCode)==0 )
	{
		strcpy((char *)dispStr1, (char *)"TRAN FAIL");
		memcpy(tmpbuf, "ERRCODE:", 8);
		sprintf((char *)(tmpbuf+8), "[%.2s]", PosCom.szRespCode);
		tmpbuf[12] = 0;
	}
	else
	{
		strcpy((char *)dispStr1, (char *)sRetErrInfo[i].szMessage);
		strcpy((char *)tmpbuf, (char *)sRetErrInfo[i].szOperate);
	}

	if (!PosCom.bGetHostResp)
	{
		strcpy((char *)dispStr1, (char *)"TRAN FAIL");
		memset((char *)tmpbuf,0,sizeof(tmpbuf));
	}

	lcdDisplay(0, 2, DISP_MEDIACY|DISP_CFONT, (char *)dispStr1);
	lcdDisplay(0, 4, DISP_MEDIACY|DISP_CFONT, (char *)tmpbuf);
	lcdFlip();
	sysBeef(3, 600);

	if(PosCom.stTrans.szEntryMode[1]=='5' && iccDetect(ICC_USERCARD)==ICC_SUCCESS)
	{
		lcdDisplay(0, 6, DISP_MEDIACY|DISP_CFONT, "RETIRE TARJETA");
		while( iccDetect(ICC_USERCARD)==ICC_SUCCESS )
		{
			sysBeep();
			sysDelayMs(500);
		}
		CommHangUp(FALSE);
		return;
	}

	if(ChkAcceptTxnCode((char*)PosCom.szRespCode) == TRUE)
		ucFlag = 2;

	if(ucFlag==1)
	{
		CommHangUp(FALSE);
		kbGetKey();
	}
	else if(ucFlag==2)
	{
		CommHangUp(FALSE);
		kbGetKeyMs(2000);
	}
	else
	{
		SetQuitTimeout();
	}

	return;
}

//失败结果显示
void NetpaydispRetCode(uint8_t ucFlag)
{
	int i  = 0;
	uint8_t tmpbuf[60], dispStr1[20];

	kbFlush();
	lcdClrLine(2, 7);
	if( (memcmp(PosCom.szRespCode, "00", 2)==0) )
	{
		return;
	}
	/*
	for(i=0; strlen((char*)sRetErrInfo[i].ucRetCode)!=0; i++)
	{
		if( memcmp(sRetErrInfo[i].ucRetCode, (char*)PosCom.szRespCode, 2)==0 )
			break;
	}*/
	memset(dispStr1,0,sizeof(dispStr1));
	memset(tmpbuf,0,sizeof(tmpbuf));
	if( strlen((char*)sRetErrInfo[i].ucRetCode)==0 )
	{
		strcpy((char *)dispStr1, (char *)"TRAN FAIL");
		memcpy(tmpbuf, "ERRCODE:", 8);
		sprintf((char *)(tmpbuf+8), "[%.2s]", PosCom.szRespCode);
		tmpbuf[12] = 0;
	}
	else
	{
		strcpy((char *)dispStr1, (char *)PosCom.szRespCode);
		strcpy((char *)tmpbuf, PosComconTrol.szRM);
	}

	if (!PosCom.bGetHostResp)
	{
		strcpy((char *)dispStr1, (char *)"TRAN FAIL");
		memset((char *)tmpbuf,0,sizeof(tmpbuf));
	}

	lcdDisplay(0, 4, DISP_MEDIACY|DISP_HFONT16,"%s %s", (char *)dispStr1,(char *)tmpbuf);
	lcdFlip();
	sysBeef(3, 600);

	if(PosCom.stTrans.szEntryMode[1]=='5' && iccDetect(ICC_USERCARD)==ICC_SUCCESS)
	{
		lcdDisplay(0, 6, DISP_MEDIACY|DISP_CFONT, "RETIRE TARJETA");
		while( iccDetect(ICC_USERCARD)==ICC_SUCCESS )
		{
			sysBeep();
			sysDelayMs(500);
		}
		CommHangUp(FALSE);
		return;
	}

	if(ChkAcceptTxnCode((char*)PosCom.szRespCode) == TRUE)
		ucFlag = 2;

	if(ucFlag==1)
	{
		CommHangUp(FALSE);
		kbGetKey();
	}
	else if(ucFlag==2)
	{
		CommHangUp(FALSE);
		kbGetKeyMs(2000);
	}
	else
	{
		SetQuitTimeout();
	}

	return;
}


void ConvErrCode(uint8_t errCode, char *szErrInfo)
{
	switch(errCode)
	{
	case	0:
		strcpy((char *)szErrInfo, "APROBADO");
		break;
	case	1:
		strcpy((char *)szErrInfo, "CANCELAR");
		break;
	case	2:
		strcpy((char *)szErrInfo, "ERROR EN TRANSACCION");//failed
		break;
	case	3:
		strcpy((char *)szErrInfo, "NO HAY TRANSACCIONES");// no trans
		break;
	case	4:
		strcpy((char *)szErrInfo, "ERROR EN PAQUETE");//PACK ERROR
		break;
	case	5:
		strcpy((char *)szErrInfo, "FALLO EN CONEXION");//CONNECT FAIL
		break;
	case	6:
		strcpy((char *)szErrInfo, "FALLO EN ENVIO");//SEND FAIL
		break;
	case	7:
		strcpy((char *)szErrInfo, "FALLO EN RECEPCION");//RECV FAIL
		break;
	case	8:
		strcpy((char *)szErrInfo, "FALLO EN DESEMPAQUETADO");//UNPACK FAIL
		break;
	case	9:
		strcpy((char *)szErrInfo, "FALLO EN REVERSO");// REVERSAL FAIL
		break;
	case	10:
		strcpy((char *)szErrInfo, "NO ENCONTRADO");
		break;
	case	11:
		strcpy((char *)szErrInfo, "OPERACION ANULADA");
		break;
	case	12:
		strcpy((char *)szErrInfo, "SWIPE ERROR");
		break;
	case	13:
		strcpy((char *)szErrInfo, "FILE OPER ERR");
		break;
	case	14:
		strcpy((char *)szErrInfo, "KEY ERROR");
		break;
	case	15:
		strcpy((char *)szErrInfo, "OVER FEE LIMIT");
		break;
	case	16:
		strcpy((char *)szErrInfo, "TIP NOT OPEN");
		break;
	case	17:
		strcpy((char *)szErrInfo, "TRANS ADJUSTED");
		break;
	case	18:
		strcpy((char *)szErrInfo, "OPEN FILE ERROR");
		break;
	case	19:
		strcpy((char *)szErrInfo, "FILE LOC ERROR");
		break;
	case	20:
		strcpy((char *)szErrInfo, "READ FILE ERR");
		break;
	case	21:
		strcpy((char *)szErrInfo, "WRITE FILE ERR");
		break;
	case	22:
		strcpy((char *)szErrInfo, "MAC CHK ERR");
		break;
	case	23:
		strcpy((char *)szErrInfo, "NO CHIP CARD");
		break;
	case    25:
		strcpy((char *)szErrInfo, "transaccion denegada");
		break;
	case    26:
		strcpy((char *)szErrInfo, "CONNECT ISSUER");
		break;
	case   27:
		strcpy((char *)szErrInfo, "SEND SCRIPT FAIL");
		break;
	case    30:
		strcpy((char *)szErrInfo, "PRODUCTO INVALIDO");
		break;
	case    53:
		strcpy((char *)szErrInfo, "CHECK OUT FINISH");
		break;
	default	:
		strcpy((char *)szErrInfo, "OTHER ERR！");
		break;
	}
	return;
}

void GetTransName(int tCode, uint8_t *szName)
{
	switch(tCode)
	{
	case POS_SALE:
	case ICC_OFFSALE:
	case QPBOC_ONLINE_SALE:
		strcpy((char *)szName, "VENTA");
		break;
	case POS_PREAUTH:
		strcpy((char *)szName, "PREAUTH");
		break;
	case POS_AUTH_CM:
		strcpy((char *)szName, "AUTH COMP");
		break;
	case POS_OFF_CONFIRM:
		strcpy((char *)szName, "AUTH OFFL");
		break;
	case POS_PREAUTH_VOID:
		strcpy((char *)szName, "VOID PREAUTH");
		break;
	case POS_AUTH_VOID:
		strcpy((char *)szName, "VOID AUTH COMP");
		break;
	case POS_SALE_VOID:
		strcpy((char *)szName, "VOID SALE");
		break;
	case POS_PREAUTH_ADD:
		strcpy((char *)szName, "PREAUTH ADD");
		break;
	case POS_REFUND:
		strcpy((char *)szName, "CANCELACION");
		break;
	case POS_QUE:
		strcpy((char *)szName, "BALANCE");
		break;
	case OFF_SALE:
		strcpy((char *)szName, "OFF-SETTLE");
		break;
	case ADJUST_TIP:
	case OFF_ADJUST:
		strcpy((char *)szName, "OFF-ADJUST");
		break;

	case EC_QUICK_SALE:
	case EC_NORMAL_SALE:
		strcpy((char *)szName, "EC SALE");
		break;

	case EC_TOPUP_CASH:
		strcpy((char *)szName, "EC TOPUP CASH");
		break;

	case EC_TOPUP_SPEC:
		strcpy((char *)szName, "EC TOPUP SPEC");
		break;

	case EC_TOPUP_NORAML:
		strcpy((char *)szName, "EC TOPUP NORMAL");
		break;

	case EC_VOID_TOPUP:
		strcpy((char *)szName, "EC VOID TOPUP");
		break;

	case EC_REFUND:
		strcpy((char *)szName, "EC REFUND");
		break;

	case POS_INSTALLMENT:
		strcpy((char *)szName, "INSTALLMENT");
		break;

	case POS_VOID_INSTAL:
		strcpy((char *)szName, "VOID INSTAL");
		break;

	default	:
		strcpy((char *)szName, "    ");
		break;
	}
	return;
}


void init_dial_statics(void)
{
	dial_stat.dials = 0;
	dial_stat.fails = 0;
	dial_stat.hours = 0;
	dial_stat.suc_rate = 0;
	write_stat_time();
}

uint8_t calculate_dial_statics(void)
{
	STAT_TIME stat_time;
	STAT_TIME now_time;
	uint8_t buf[9], B_date[9], B_time[7];

	if( read_data(&dial_stat, sizeof(DIAL_STAT), "dial.sta")!=OK )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "DIAL ERROR");
		lcdFlip();
		kbGetKey();
		return 1;

	}

	if( read_data(&stat_time, sizeof(STAT_TIME), "stat.tm")!=OK )
	{
		lcdClrLine(2, 7);
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "TIME ERROR");
		lcdFlip();
		kbGetKey();
		return 1;
	}

	GetPosTime(B_date, B_time);

	memcpy(buf, B_date, 4);
	buf[4] = 0;
	now_time.year = atoi((char*)buf);

	memcpy(buf, B_date+4, 2);
	buf[2] = 0;
	now_time.month = atoi((char*)buf);

	memcpy(buf, B_date+6, 2);
	buf[2] = 0;
	now_time.day = atoi((char*)buf);

	memcpy(buf, B_time, 2);
	buf[2] = 0;
	now_time.hour = atoi((char*)buf);

	dial_stat.hours = time_diff(&stat_time, & now_time);

	if( dial_stat.hours<=0 )
		dial_stat.hours = 1;
	if( dial_stat.hours>9999 )
		dial_stat.hours = 9999;

	if( dial_stat.dials==0 )
	{
		dial_stat.dials = 0;
		dial_stat.fails = 0;
		dial_stat.suc_rate = 100;
		return OK;
	}
	else
	{
		dial_stat.suc_rate = (int) ((dial_stat.dials - dial_stat.fails) * 100 / dial_stat.dials);
	}
	if( dial_stat.suc_rate>100 )
		dial_stat.suc_rate = 100;
	return OK;

}

uint8_t write_stat_time(void)
{
	uint8_t B_date[9], B_time[7];
	uint8_t buf[5];
	STAT_TIME stat_time;

	GetPosTime(B_date, B_time);

	memcpy(buf, B_date, 4);
	buf[4] = 0;
	stat_time.year = atoi((char*)buf);

	memcpy(buf, B_date + 4, 2);
	buf[2] = 0;
	stat_time.month = atoi((char*)buf);

	memcpy(buf, B_date + 6, 2);
	buf[2] = 0;
	stat_time.day = atoi((char*)buf);

	memcpy(buf, B_time, 2);
	buf[2] = 0;
	stat_time.hour = atoi((char*)buf);

	return write_data(&stat_time, sizeof(STAT_TIME), "stat.tm");

}

uint32_t GetDays(uint8_t *szDate)
{
	int16_t  usyear, mymonth, mydate, nowyear;
	uint32_t rsl;

	nowyear = (int16_t)AscToLong(&szDate[0], 4);

	usyear = nowyear - STAR_YEAR;

	rsl = 0;

	rsl = (nowyear/4)-(STAR_YEAR/4);
	rsl = rsl+(STAR_YEAR/100)-(nowyear/100);
	rsl = rsl+(nowyear/400)-(STAR_YEAR/400);

	rsl = usyear*365l+rsl;
	mymonth = (int16_t) AscToLong(&szDate[4], 2);

	mydate = monthday[mymonth-1];
	rsl += mydate;
	if( IsLeapYear((uint32_t)nowyear) )
	{
		if( IsLeapYear((uint32_t)STAR_YEAR) )
		{
			if( mymonth>=3 )
			{
				/*current year is Leap year*/
				rsl++;
			}
		}
		else
		{
			if( mymonth<3 )
			{
				/*current year is Leap year,but the initial years is not leap year*/
				rsl--;
			}

		}
	}
	else if( IsLeapYear((uint32_t)STAR_YEAR) )
	{
		/*current year is Leap year,but the initial years is not leap year*/
		rsl++;
	}

	mydate = (int16_t)AscToLong(&szDate[6], 2);
	mydate--;
	rsl += mydate;
	return rsl;
}

/*
To obtain the current relative to the number of seconds
Start with the January 1, 1980 0 count
*/
uint32_t GetEpoch(uint8_t *szToDateTime)
{
	uint32_t rsl;
	uint32_t mydate;

	rsl = GetDays(szToDateTime);
	rsl *= 24;

	mydate = AscToLong(&szToDateTime[8], 2);
	rsl += mydate;
	/*Minute*/
	rsl *= 60l;

	mydate = AscToLong(&szToDateTime[10], 2);
	rsl += mydate;
	/*second*/
	rsl *= 60l;

	mydate = AscToLong(&szToDateTime[12], 2);
	rsl += mydate;
	return rsl;
}

/*
judge the leap year
*/
uint8_t IsLeapYear(uint32_t inyear)
{
	if( (inyear%400)==0 )
	{
		return 1;
	}
	else if( (inyear%100)==0 )
	{
		return 0;
	}
	else if( (inyear%4)==0 )
	{
		return 1;
	}
	return 0;
}


/************************************************************************/
/* The time difference between old_time and new_time,In hours */
/************************************************************************/
int time_diff(STAT_TIME * old_time, STAT_TIME * new_time)
{
	uint8_t old_time_buf[15];
	uint8_t new_time_buf[15];
	int t_old, t_new;
	int diff;

	sprintf((char*) old_time_buf, "%04d%02d%02d%02d0000",
		old_time->year, (int)old_time->month, (int)old_time->day, (int)old_time->hour);
	sprintf((char*) new_time_buf, "%04d%02d%02d%02d0000",
		new_time->year, (int)new_time->month, (int)new_time->day, (int)new_time->hour);

	t_old = GetEpoch(old_time_buf);
	t_new = GetEpoch(new_time_buf);

	diff = t_new - t_old;

	if( diff<0 )
		diff = 0 - diff;

	diff /= 3600;

	return diff;
}



uint8_t AmountIsZero(uint8_t *sBcdAmt, uint8_t ucAmtLen)
{
	uint8_t	i;

	for(i=0; i<ucAmtLen; i++)
	{
		if( sBcdAmt[i]!=0 )
		{
			return FALSE;
		}
	}

	return TRUE;
}

uint8_t SwipeCardProc(uint8_t bCheckICC)
{
	uint8_t	ucRet;

	PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
	//memset(PosCom.szTrack2,0,sizeod(PosCom.szTrack2));
	//memset(PosCom.szTrack3,0,sizeod(PosCom.szTrack3));
	//memset(PosCom.stTrans.szCardNo,0,sizeod(PosCom.stTrans.szCardNo));

	ucRet = GetTrackData(PosCom.szTrack2, PosCom.szTrack3, PosCom.stTrans.szCardNo);
	printf("GetTrackData:%d;%s\n",ucRet,PosCom.stTrans.szCardNo);
	
	if( ucRet!=OK )
	{
		return ucRet;
	}

	if( !bCheckICC || stPosParam.ucEmvSupport!=PARAM_OPEN )
	{
		PosCom.ucSwipedFlag = CARD_SWIPED;
		return OK;
	}

	if( PosCom.ucFallBack==TRUE && IsEmvCard(PosCom.szTrack2) )
	{
		PosCom.ucSwipedFlag = FALLBACK_SWIPED;
		return OK;
	}
	if( !IsEmvCard(PosCom.szTrack2) )
	{
		PosCom.ucFallBack   = FALSE;
		PosCom.ucSwipedFlag = CARD_SWIPED;
		return OK;
	}

	return E_NEED_INSERT;
}

void DispFallback(void)
{
	if(iccDetect(ICC_USERCARD)!=ICC_SUCCESS)
	{
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		lcdClrLine(3, 7);
		lcdDisplay(0, 5, DISP_CFONT|DISP_MEDIACY, "NO SE PUEDE LEER CHIP");
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		ErrorBeep();
		return;
	}

	lcdClrLine(2, 7);
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
	lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "NO SE PUEDE LEER CHIP");
	lcdDisplay(0, 8, DISP_CFONT|DISP_MEDIACY, "RETIRE TARJETA");
	lcdFlip();
	lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
	ErrorBeep();
	WaitRemoveICC();
}

extern int(*__ICCARD_EXCHANGE_APDU)(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdat);
uint8_t Direct_InsertCardProc(uint8_t bShowDefTranName)
{
	int		iRet, iErrNo,ucRet;
	uint32_t    uiLen;
	char        Atr[500];
	uint8_t  uLen, sBuff[10];

	#if 1
	if( PosCom.ucFallBack==TRUE )
	{
		return E_NEED_FALLBACK;
	}
	#endif
	
	PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
	stTemp.iTransNo = POS_SALE; //给一个默认值

	if( bShowDefTranName )
	{
		lcdCls();		
		DispTransName();
	}

	DispICCProcessing();
	lcdFlip();
	kbFlush();

	if((stTemp.iTransNo != POS_SALE)&&(stTemp.iTransNo != PURSE_SALE))
	{
		emv_pboc_ec_enable(0);
	}

	emv_get_mck_configure(&tConfig);
	tEMVCoreInit.transaction_type = EMV_GOODS;
	
	if(  stTemp.iTransNo == POS_SALE|| stTemp.iTransNo == PURSE_SALE)
	{
		tConfig.support_forced_online_capability = 1;
	}
	else
	{
		tConfig.support_forced_online_capability = 0;
	}

	SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);

	emv_set_mck_configure(&tConfig);
	switch(stTemp.iTransNo)
	{
	case EC_TOPUP_CASH:
	case EC_VOID_TOPUP:
		tEMVCoreInit.transaction_type = 0x63;
		break;
	case EC_TOPUP_SPEC:
		tEMVCoreInit.transaction_type = 0x60;
		break;
	case EC_TOPUP_NORAML:
		tEMVCoreInit.transaction_type = 0x62;
		break;
	default:
		tEMVCoreInit.transaction_type = EMV_GOODS;
		break;
	}
	emv_set_core_init_parameter(&tEMVCoreInit);
    	gl_FirstRunSelApp = 0x01;
	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_ISO);
	iRet = iccPowerUp(ICC_USERCARD, &uiLen, Atr);
	if( iRet )
	{
		DispFallback();
		return E_NEED_FALLBACK;
	}
	
	icc_fd = iccGetSlotFd(ICC_USERCARD);
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	
	iRet = emv_application_select(icc_fd, stPosParam.lNowTraceNo);
	iErrNo = iRet?errno:0;
	
	// ADVT case 26
	if( iErrNo==EMV_ERRNO_DATA || iErrNo==EMV_ERRNO_NO_APP ||iErrNo==EMV_ERRNO_SW )
	{
		PosCom.ucFallBack = TRUE;
		DispFallback();
		return E_NEED_FALLBACK;
	}
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo==EMV_ERRNO_CARD_BLOCKED )
	{
		return E_CARD_BLOCKER;
	}
	if( iErrNo==EMV_ERRNO_APP_BLOCKED )
	{
		return E_APP_BLOCKED;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	
	DispICCProcessing();
	lcdFlip();
	sysDelayMs(100);

	iRet = emv_read_app_data(icc_fd);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	
	SetEmvTranType();	// set emv transaction type, tag 9C

	emv_set_data_element("\x9F\x53", 2, 1, "\x52");
	// The complete EMV process
	if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==PURSE_SALE)
	{
		iRet = emv_offline_data_authentication(icc_fd);
		iErrNo = iRet?errno:0;
		if( iErrNo!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}

		if (stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.ucTransferFlag)
		{
			uLen = 0;
			memset(sBuff,0,sizeof(sBuff));
			emv_get_data_element("\x95", 1, &uLen, sBuff);
			if (sBuff[0]&0x48)
			{
				return E_TRANS_FAIL;
			}
		}
	}
	// Read Track 2 and/or Pan
	iRet = GetEmvTrackData();
	if( iRet<0 )
	{
		return E_TRANS_FAIL;
	}
	
	PosCom.ucSwipedFlag = CARD_INSERTED;

	DispCardNo(PosCom.stTrans.szCardNo);

	switch(PosCom.stTrans.CardType)
	{
		case 4:
			PosCom.stTrans.TransFlag = PURSE_PUNTO;
			stTemp.iTransNo = PURSE_SALE;
			break;
		case 5:
			PosCom.stTrans.TransFlag = PURSE_SODEXO;
			stTemp.iTransNo = PURSE_SALE;
			break;
		case 6:
			PosCom.stTrans.TransFlag =PURSE_TODITO;
			stTemp.iTransNo = PURSE_SALE;
			break;
		case 7:
			PosCom.stTrans.TransFlag = PURSE_EDENRED;
			stTemp.iTransNo = PURSE_SALE;
			break;
		default:
			stTemp.iTransNo = POS_SALE;
			break;
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
			iRet = ComfirAmount(PosCom.stTrans.sAmount);
			if(iRet == RE_SELECT)
			{
				continue;
			}
			else if(NO_DISP==iRet)
			{
				return iRet;
			}
			else
			{
				break;
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
			iRet = ConfirmTipAmount();
			if(iRet == RE_SELECT)
			{
				continue;
			}
			else  if(iRet ==NO_DISP)
			{
				return iRet;
			}	
			else
			{
				break;
			}
		}			
	}
#if 1
	emv_set_core_init_parameter(&tEMVCoreInit);
    	gl_FirstRunSelApp = 0x01;
	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_ISO);
	iRet = iccPowerUp(ICC_USERCARD, &uiLen, Atr);
	if( iRet )
	{
		DispFallback();
		return E_NEED_FALLBACK;
	}
	
	icc_fd = iccGetSlotFd(ICC_USERCARD);
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	
	iRet = emv_application_select(icc_fd, stPosParam.lNowTraceNo);
	iErrNo = iRet?errno:0;
	
	// ADVT case 26
	if( iErrNo==EMV_ERRNO_DATA || iErrNo==EMV_ERRNO_NO_APP ||iErrNo==EMV_ERRNO_SW )
	{
		PosCom.ucFallBack = TRUE;
		DispFallback();
		return E_NEED_FALLBACK;
	}
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo==EMV_ERRNO_CARD_BLOCKED )
	{
		return E_CARD_BLOCKER;
	}
	if( iErrNo==EMV_ERRNO_APP_BLOCKED )
	{
		return E_APP_BLOCKED;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	
	DispICCProcessing();
	lcdFlip();
	sysDelayMs(100);

	iRet = emv_read_app_data(icc_fd);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	SetEmvTranType();	// set emv transaction type, tag 9C

	emv_set_data_element("\x9F\x53", 2, 1, "\x52");
	// The complete EMV process
	if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==PURSE_SALE)
	{
		iRet = emv_offline_data_authentication(icc_fd);
		iErrNo = iRet?errno:0;
		if( iErrNo!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}
		if (stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.ucTransferFlag)
		{
			uLen = 0;
			memset(sBuff,0,sizeof(sBuff));
			emv_get_data_element("\x95", 1, &uLen, sBuff);
			if (sBuff[0]&0x48)
			{
				return E_TRANS_FAIL;
			}
		}
	}
	
#endif

	return OK;
}
uint8_t InsertCardProc(uint8_t bShowDefTranName)
#if 1
{
	int		iRet, iErrNo,ucRet;
	uint32_t    uiLen;
	char        Atr[500];
	uint8_t  uLen, sBuff[10];

	#if 1
	if( PosCom.ucFallBack==TRUE )
	{
		return E_NEED_FALLBACK;
	}
	#endif
	
	PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
	if( bShowDefTranName )
	{
		lcdCls();		
		DispTransName();
	}

	DispICCProcessing();
	lcdFlip();
	kbFlush();

	if((stTemp.iTransNo != POS_SALE)&&(stTemp.iTransNo != PURSE_SALE))
	{
		emv_pboc_ec_enable(0);
	}

	emv_get_mck_configure(&tConfig);
	tEMVCoreInit.transaction_type = EMV_GOODS;
	
	if( stTemp.iTransNo==POS_QUE || stTemp.iTransNo==POS_PREAUTH || stTemp.iTransNo == POS_SALE|| 
		stTemp.iTransNo == EC_TOPUP_CASH || stTemp.iTransNo == EC_TOPUP_SPEC||stTemp.iTransNo == EC_TOPUP_NORAML ||
		stTemp.iTransNo==EC_VOID_TOPUP || stTemp.iTransNo==CHECK_IN || stTemp.iTransNo==CHECK_OUT ||
		stTemp.iTransNo == CASH_ADVACNE || stTemp.iTransNo == NETPAY_REFUND || stTemp.iTransNo == PURSE_SALE)
	{
		tConfig.support_forced_online_capability = 1;
	}
	else
	{
		tConfig.support_forced_online_capability = 0;
	}

	SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);

	emv_set_mck_configure(&tConfig);
	switch(stTemp.iTransNo)
	{
	case EC_TOPUP_CASH:
	case EC_VOID_TOPUP:
		tEMVCoreInit.transaction_type = 0x63;
		break;
	case EC_TOPUP_SPEC:
		tEMVCoreInit.transaction_type = 0x60;
		break;
	case EC_TOPUP_NORAML:
		tEMVCoreInit.transaction_type = 0x62;
		break;
	default:
		tEMVCoreInit.transaction_type = EMV_GOODS;
		break;
	}
#if 1
	emv_set_core_init_parameter(&tEMVCoreInit);
    	gl_FirstRunSelApp = 0x01;
	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_ISO);
	iRet = iccPowerUp(ICC_USERCARD, &uiLen, Atr);
	if( iRet )
	{
		DispFallback();
		return E_NEED_FALLBACK;
	}
	
	icc_fd = iccGetSlotFd(ICC_USERCARD);
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	
	iRet = emv_application_select(icc_fd, stPosParam.lNowTraceNo);
	iErrNo = iRet?errno:0;
	
	// ADVT case 26
	if( iErrNo==EMV_ERRNO_DATA || iErrNo==EMV_ERRNO_NO_APP ||
		iErrNo==EMV_ERRNO_SW )
	{
		PosCom.ucFallBack = TRUE;
		DispFallback();
		return E_NEED_FALLBACK;
	}
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo==EMV_ERRNO_CARD_BLOCKED )
	{
		return E_CARD_BLOCKER;
	}
	if( iErrNo==EMV_ERRNO_APP_BLOCKED )
	{
		return E_APP_BLOCKED;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	
	DispICCProcessing();
	lcdFlip();
	sysDelayMs(100);

	iRet = emv_read_app_data(icc_fd);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	SetEmvTranType();	// set emv transaction type, tag 9C

	emv_set_data_element("\x9F\x53", 2, 1, "\x52");
	// The complete EMV process
	if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==POS_QUE ||
		stTemp.iTransNo==POS_PREAUTH || stTemp.iTransNo==CHECK_IN||
		stTemp.iTransNo==CHECK_OUT|| stTemp.iTransNo == CASH_ADVACNE ||
		stTemp.iTransNo==NETPAY_REFUND || stTemp.iTransNo == NETPAY_ADJUST ||
		stTemp.iTransNo==PURSE_SALE||stTemp.iTransNo==PURSE_GETBALANCE||
		(stTemp.iTransNo>=EC_NORMAL_SALE && stTemp.iTransNo <= EC_TOPUP_NORAML) )
	{
		iRet = emv_offline_data_authentication(icc_fd);
		iErrNo = iRet?errno:0;
		if( iErrNo!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}

		if (stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.ucTransferFlag)
		{
			uLen = 0;
			memset(sBuff,0,sizeof(sBuff));
			emv_get_data_element("\x95", 1, &uLen, sBuff);
			if (sBuff[0]&0x48)
			{
				return E_TRANS_FAIL;
			}
		}
	}

#endif
	// Read Track 2 and/or Pan
	iRet = GetEmvTrackData();
	if( iRet<0 )
	{
		return E_TRANS_FAIL;
	}

	if (ChkOnlyEcCard(PosCom.stTrans.szAID))
	{
		if(stTemp.iTransNo==POS_SALE || stTemp.iTransNo==EC_NORMAL_SALE || stTemp.iTransNo==EC_QUICK_SALE
        		|| stTemp.iTransNo==EC_TOPUP_CASH || stTemp.iTransNo==EC_TOPUP_SPEC || stTemp.iTransNo==EC_REFUND
			|| stTemp.iTransNo==EC_VOID_TOPUP || (stTemp.iTransNo==EC_TOPUP_NORAML && !PosCom.ucTransferFlag))
		{
			PosCom.ucSwipedFlag = CARD_INSERTED;
			return DispCardNo(PosCom.stTrans.szCardNo);
		}
		else
		{
			lcdClrLine(2,7);
			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PURE E-CASH CARD");
			lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"NOT SUPPORT");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(5);
			return E_TRANS_FAIL;
		}
	}

	//先下电
	//iccPowerDown(ICC_USERCARD);
	
	PosCom.ucSwipedFlag = CARD_INSERTED;

		//余额查询直接退出
	if(stTemp.iTransNo==PURSE_GETBALANCE ||stTemp.iTransNo ==PURSE_GETRULE)
	{
		return OK;
	}
	
	if(CheckCardType()!= OK)
	{
		return E_FILE_SEEK;
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
			iErrNo = ComfirAmount(PosCom.stTrans.sAmount);
			if(iErrNo != RE_SELECT)
			{
				break;;
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
		if(stPosParam.szpreTip == PARAM_CLOSE || stTemp.iTransNo ==PURSE_SALE)
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

#if 1
	emv_set_core_init_parameter(&tEMVCoreInit);
    	gl_FirstRunSelApp = 0x01;
	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_ISO);
	iRet = iccPowerUp(ICC_USERCARD, &uiLen, Atr);
	if( iRet )
	{
		DispFallback();
		return E_NEED_FALLBACK;
	}
	
	icc_fd = iccGetSlotFd(ICC_USERCARD);
	__ICCARD_EXCHANGE_APDU = iccCardExchangeAPDU;
	
	iRet = emv_application_select(icc_fd, stPosParam.lNowTraceNo);
	iErrNo = iRet?errno:0;
	
	// ADVT case 26
	if( iErrNo==EMV_ERRNO_DATA || iErrNo==EMV_ERRNO_NO_APP ||
		iErrNo==EMV_ERRNO_SW )
	{
		PosCom.ucFallBack = TRUE;
		DispFallback();
		return E_NEED_FALLBACK;
	}
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo==EMV_ERRNO_CARD_BLOCKED )
	{
		return E_CARD_BLOCKER;
	}
	if( iErrNo==EMV_ERRNO_APP_BLOCKED )
	{
		return E_APP_BLOCKED;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	
	DispICCProcessing();
	lcdFlip();
	sysDelayMs(100);

	iRet = emv_read_app_data(icc_fd);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}
	SetEmvTranType();	// set emv transaction type, tag 9C

	emv_set_data_element("\x9F\x53", 2, 1, "\x52");
	// The complete EMV process
	if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==POS_QUE ||
		stTemp.iTransNo==POS_PREAUTH || stTemp.iTransNo==CHECK_IN||
		stTemp.iTransNo==CHECK_OUT|| stTemp.iTransNo == CASH_ADVACNE ||
		stTemp.iTransNo==NETPAY_REFUND || stTemp.iTransNo == NETPAY_ADJUST ||
		stTemp.iTransNo==PURSE_SALE||stTemp.iTransNo==PURSE_GETBALANCE||
		(stTemp.iTransNo>=EC_NORMAL_SALE && stTemp.iTransNo <= EC_TOPUP_NORAML) )
	{
		iRet = emv_offline_data_authentication(icc_fd);
		iErrNo = iRet?errno:0;
		if( iErrNo!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}

		if (stTemp.iTransNo == EC_TOPUP_NORAML && PosCom.ucTransferFlag)
		{
			uLen = 0;
			memset(sBuff,0,sizeof(sBuff));
			emv_get_data_element("\x95", 1, &uLen, sBuff);
			if (sBuff[0]&0x48)
			{
				return E_TRANS_FAIL;
			}
		}
	}
	
#endif
	return DispCardNo(PosCom.stTrans.szCardNo);
}
#endif
int GetEmvTrackData(void)
{
	int	iRet, iErrNo;
	uint8_t	sTemp[30], szCardNo[20+1], iLength;
	int	i, bReadTrack2, bReadPan;

	// Read Track 2 Equivalent Data
	bReadTrack2 = FALSE;
	memset(sTemp, 0, sizeof(sTemp));

	iRet = emv_get_data_element("\x5F\x20",2, &iLength, (char*)sTemp);
	if(iRet == OK){
		strcpy((char*)PosCom.stTrans.CardHolderName,(char*)sTemp);
//		BcdToAsc0(PosCom.szTrack1, sTemp, iLength*2);
	}
	
	memset(sTemp, 0, sizeof(sTemp));
	iRet = emv_get_data_element("\x57", 1, &iLength, sTemp);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_RET_SUCCESS )
	{
		bReadTrack2 = TRUE;
		BcdToAsc0(PosCom.szTrack2, sTemp, iLength*2);
		RemoveTailChars(PosCom.szTrack2, 'F');	// erase padded 'F' chars
		for(i=0; PosCom.szTrack2[i]!='\0'; i++)		// convert 'D' to '='
		{
			if( PosCom.szTrack2[i]=='D' )
			{
				PosCom.szTrack2[i] = '=';
				break;
			}
		}
	}

	// read PAN
	bReadPan = FALSE;
	memset(sTemp, 0, sizeof(sTemp));
	iRet = emv_get_data_element("\x5A", 1, &iLength, sTemp);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_RET_SUCCESS )
	{
		BcdToAsc0(szCardNo, sTemp, iLength*2);
		RemoveTailChars(szCardNo, 'F');		// erase padded 'F' chars
		if( bReadTrack2 && !MatchTrack2AndPan(PosCom.szTrack2, szCardNo) )
		{
			return -1;
		}
		sprintf((char *)PosCom.stTrans.szCardNo, "%.19s", szCardNo);
		bReadPan = TRUE;
	}
	else if( !bReadTrack2 )
	{
		return -2;
	}
	if( !bReadPan )
	{
		iRet = GetCardNoFromTrack(PosCom.stTrans.szCardNo, PosCom.szTrack2, (uint8_t *)"");
		if( iRet!=OK )
		{
			return -3;
		}
	}

	// read PAN sequence number
	PosCom.stTrans.bPanSeqNoOk = FALSE;
	iRet = emv_get_data_element("\x5F\x34", 2, &iLength, &PosCom.stTrans.ucPanSeqNo);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_RET_SUCCESS )
	{
		PosCom.stTrans.bPanSeqNoOk = TRUE;
	}

	// read Application Expiration Date
	if( bReadPan )
	{
		memset(sTemp, 0, sizeof(sTemp));
		iRet = emv_get_data_element("\x5F\x24", 2, &iLength, sTemp);
		iErrNo = iRet?errno:0;
		if( iErrNo==EMV_RET_SUCCESS )
		{
			BcdToAsc0(PosCom.stTrans.szExpDate, sTemp, 4);
		}
	}

	// read other data for print slip
	emv_get_data_element("\x50", 1, &iLength, PosCom.stTrans.szAppLable);	// application label
	iRet = emv_get_data_element("\x4F", 1, &iLength, sTemp);	// AID
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_RET_SUCCESS )
	{
		BcdToAsc0(PosCom.stTrans.szAID, sTemp, iLength*2);
		RemoveTailChars(PosCom.stTrans.szAID, 'F');
	}

	// EMV simple process
	if( stTemp.iTransNo!=POS_SALE && stTemp.iTransNo!=POS_QUE &&
		stTemp.iTransNo!=POS_PREAUTH && stTemp.iTransNo!=POS_INSTALLMENT &&
		stTemp.iTransNo!=POS_INSTALLMENT&&
		!(stTemp.iTransNo>=EC_NORMAL_SALE && stTemp.iTransNo <= EC_TOPUP_NORAML))
	{
		iRet = SetIssuerName();
		if( iRet!=OK )
		{
			return -4;
		}
	}

	return OK;
}

void RemoveTailChars(uint8_t *pszString, uint8_t ucRemove)
{
	int	i, iLen;

	iLen = strlen((char *)pszString);
	for(i=iLen-1; i>0; i--)
	{
		if( pszString[i]!=ucRemove )
		{
			break;
		}
		pszString[i] = 0;
	}
}

uint8_t MatchTrack2AndPan(uint8_t *pszTrack2, uint8_t *pszPan)
{
	int	i;
	uint8_t	szTemp[19+1];

	for(i=0; i<19 && pszTrack2[i]!='\0'; i++)
	{
		if( pszTrack2[i]=='=' )
		{
			break;
		}
		szTemp[i] = pszTrack2[i];
	}
	szTemp[i] = 0;

	if( strcmp((char *)szTemp, (char *)pszPan)==0 )
	{
		return TRUE;
	}

	return FALSE;
}

void ModifyTermCapForApp(void)
{
	int	iRet, iCnt, iErrNo;
	uint8_t	sAid[16], iLength;

	emv_get_mck_configure(&tConfig);
	memset(sAid, 0, sizeof(sAid));
	iRet = emv_get_data_element("\x9F\x06", 2,  &iLength, sAid);	// should use this
	iErrNo = iRet?errno:0;
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		memcpy(tConfig.terminal_capabilities, "\xE0\xF9\xC8", 3);	// use default value
		emv_set_data_element("\x9F\x33", 2, 3, tConfig.terminal_capabilities);	// core will not modify this tag
		SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);

		emv_set_mck_configure(&tConfig);
		return;
	}

	memcpy(tConfig.terminal_capabilities, "\xE0\xF9\xC8", 3);	// set default value
	for(iCnt=0; iCnt<glTermAidNum; iCnt++)
	{
		if( glTermAidList[iCnt].ucAidLen==(uint8_t)iLength &&
			memcmp(glTermAidList[iCnt].sAID, sAid, iLength)==0 )
		{
			break;
		}
	}
	if( iCnt<glTermAidNum && glTermAidList[iCnt].bOnlinePin==0 )
	{
		tConfig.terminal_capabilities[1] &= ~0x40;
	}

	if (stTemp.iTransNo == EC_TOPUP_CASH)
	{
		tConfig.terminal_capabilities[1] &= ~0x40;
	}

	SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);

	emv_set_mck_configure(&tConfig);
	emv_set_data_element("\x9F\x33", 2, 3, tConfig.terminal_capabilities);	// core will not modify this tag
}

void DispTransName(void)
{
	//return;
	lcdCls();
	switch( stTemp.iTransNo )
	{
	case POS_SALE:
	case ICC_OFFSALE:
	case QPBOC_ONLINE_SALE:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "      VENTA     ");
		break;

	case POS_PREAUTH:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    PREAUTH     ");
		break;

	case POS_LOGON:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     LOGON      ");
	    break;

	case POS_LOGOFF:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     LOGOUT     ");
	    break;

	case POS_SETT:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     CIERRE     ");
		break;

	case POS_BATCH_UP:
	case POS_REFUND_BAT:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     UPLOAD     ");
		break;

	case POS_SALE_VOID:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    VOID SALE   ");
	    break;

	case POS_AUTH_VOID:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  VOID PRE-COMP ");
	    break;

	case POS_PREAUTH_VOID:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  VOID PREAUTH  ");
		break;

	case POS_REFUND:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     REFUND     ");
		break;

	case POS_AUTH_CM:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  PREAUTH COMP  ");
		break;

	case POS_PREAUTH_ADD:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   PREAUTH ADD  ");
		break;

	case POS_OFF_CONFIRM:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  PREAUTH OFFL  ");
		break;

	case POS_QUE:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     BALANCE    ");
		break;

	case RESPONSE_TEST:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    RSP TEST    ");
		break;

	case OFF_SALE:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   OFF-SETTLE   ");
		break;

	case OFF_ADJUST:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   OFF-ADJUST   ");
		break;

	case DOWNLOAD_EMV_CAPK:
        DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   DOWN PUBKEY  ");
        break;
    case DOWNLOAD_EMV_PARAM:
        DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " DOWN EMV PARAM ");
        break;
	case POS_DOWNLOAD_PARAM:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DOWNLOAD PARA ");
		break;
	case QUERY_EMV_PARAM:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " QUERY EMV PARA ");
		break;
	case QUERY_EMV_CAPK:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " QUERY EMV KEY  ");
		break;
	case END_EMV_PARAM:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DOWN PARA END ");
		break;
	case END_DOWNLOADBLK:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " DOWN BLACKCARD ");
		break;
	case END_EMV_CAPK:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DOWN KEY END  ");
		break;
	case DOWNLOAD:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, " REMOTE DOWNLOAD");
		break;
	case EC_NORMAL_SALE:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     E-CASH     ");
		break;
	case EC_QUICK_SALE:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "     QPBOC      ");
		break;
	case EC_TOPUP_CASH:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    CASH LOAD   ");
		break;
	case EC_TOPUP_SPEC:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   ACCT LOAD    ");
		break;
	case EC_TOPUP_NORAML:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  NO-ACCT LOAD  ");
		break;
	case EC_REFUND:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   EC REFUND    ");
		break;
	case EC_VOID_TOPUP:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   VOID CASH    ");
		break;
	case POS_INSTALLMENT:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   INSTALLMENT  ");
		break;
	case POS_VOID_INSTAL:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   VOID INSTAL  ");
		break;
	case POS_ELEC_SIGN:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "UPLOAD ELEC SIGN");
		break;
	case CHECK_IN:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CHECK IN");
		break;
	case CHECK_OUT:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "CHECK OUT");
		break;
	case CASH_ADVACNE:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  RETIRO  ");
		break;
	case NETPAY_REFUND:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  DEVOLUCION  ");
		break;
	case NETPAY_ADJUST:
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  AJUSTE  ");
		break;
	default:
	    break;
	}
}

void SetEmvTranType(void)
{
	switch( stTemp.iTransNo )
	{
	case POS_SALE:
	case PURSE_SALE:
	case EC_NORMAL_SALE:
	case EC_QUICK_SALE:
    	case QPBOC_ONLINE_SALE:
	case POS_INSTALLMENT:
	case CHECK_IN:
	case CHECK_OUT:	
	case CASH_ADVACNE:
	case NETPAY_REFUND:
	case NETPAY_ADJUST:	
		emv_set_data_element("\x9C", 1, 1, "\x00");
		break;

	case POS_PREAUTH:
		emv_set_data_element("\x9C", 1, 1, "\x03");
		break;

	case POS_QUE:
		emv_set_data_element("\x9C", 1, 1, "\x31");
		break;

	case EC_TOPUP_CASH:
	case EC_VOID_TOPUP:
		emv_set_data_element("\x9C", 1, 1, "\x63");
		break;

	case EC_TOPUP_NORAML:
		emv_set_data_element("\x9C", 1, 1, "\x62");
		break;

	case EC_TOPUP_SPEC:
		emv_set_data_element("\x9C", 1, 1, "\x60");
		break;

	default:
		break;
	}
}


void ViewEmvResult(void)
{
	uint8_t	sTVR[5], sTSI[2], iLength;

	memset(sTVR, 0, sizeof(sTVR));
	emv_get_data_element("\x95", 1, &iLength, sTVR);
	memset(sTSI, 0, sizeof(sTSI));
	emv_get_data_element("\x9B", 1, &iLength, sTSI);

	lcdCls();
	lcdDisplay(0, 0, DISP_ASCII, "TVR: %02X %02X %02X %02X %02X",
			sTVR[0], sTVR[1], sTVR[2], sTVR[3], sTVR[4]);
	lcdDisplay(0, 1, DISP_ASCII, "TSI: %02X %02X", sTSI[0], sTSI[1]);
	lcdFlip();
	kbGetKeyMs(60*1000);
}

uint16_t uiPortRecvBuf(uint8_t ucPort, uint8_t *psBuff, uint16_t nLength, uint32_t nTimeOut)
{
	uint8_t   ucRet;
#define TIMER_TEMPORARY	4

	sysTimerSet(TIMER_TEMPORARY, (uint16_t)(nTimeOut));
	while( nLength>0 )
	{
		if( sysTimerCheck(TIMER_TEMPORARY)==0 )
		{
			return 0xFF;
		}
		ucRet = portRecv(ucPort, psBuff, 4000);
		if( ucRet==PORT_RET_OK )
		{
			psBuff++;
			nLength--;
		}
		else if( ucRet!=0xFF )
		{
			return 1;
		}
	}   // end of while( nLength>0

	return 0;
}

//1234 --> 0x12 0x34
void vLong2Bcd(uint32_t ulSource, uint16_t uiTCnt, uint8_t *psTarget)
{
	char    szFmt[30], szBuf[30];

	sprintf(szFmt, "%%0%dlu", uiTCnt*2);
	sprintf(szBuf, szFmt, ulSource);
	if( psTarget!=NULL )
	{
		vTwo2One((uint8_t *)szBuf, uiTCnt*2, psTarget);
	}
}

void vOne2Two(uint8_t *psIn, uint16_t nLength, uint8_t *psOut)
{
	static const uint8_t ucHexToChar[16] = {"0123456789ABCDEF"};
	uint16_t   nCnt;

	for(nCnt=0; nCnt<nLength; nCnt++)
	{
		psOut[2*nCnt]   = ucHexToChar[(psIn[nCnt]>>4)];
		psOut[2*nCnt+1] = ucHexToChar[(psIn[nCnt]&0x0F)];
	}
}

void vOne2Two0(uint8_t *psIn, uint16_t nLength, uint8_t *psOut)
{
	vOne2Two(psIn, nLength, psOut);
	psOut[2*nLength] = 0;
}

//"12AB"-->0x12AB
void vTwo2One(uint8_t *psIn, uint16_t nLength, uint8_t *psOut)
{
	uint8_t   tmp;
	uint16_t  i;

	for(i=0; i<nLength; i+=2)
	{
		tmp = psIn[i];
		if( tmp>'9' )
		{
			tmp = toupper(tmp) - 'A' + 0x0A;
		}
		else
		{
			tmp &= 0x0F;
		}
		psOut[i/2] = (tmp<<4);

		tmp = psIn[i+1];
		if( tmp>'9' )
		{
			tmp = toupper(tmp) - 'A' + 0x0A;
		}
		else
		{
			tmp &= 0x0F;
		}
		psOut[i/2] |= tmp;
	}
}

void vBcd2Long(uint8_t *psSource, uint16_t uiSCnt, uint32_t *pulTarget)
{
	uint8_t   szBuf[30];

	vOne2Two0(psSource, uiSCnt, szBuf);
	if( pulTarget!=NULL )
	{
		*pulTarget = atol((char *)szBuf);
	}
}

/****************************************************************************
Function:      Send data from com port.
Param In:
ucPort      Port number to read data. valid value: COM1, COM2.
Port must be opened before send data.
psBuff      Data to be send.
nLength     Bytes of data.
Param Out:
none
Return Code:
none
Protocol:      STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
****************************************************************************/
void vNacSend(uint8_t ucPort, uint8_t *psBuff, uint16_t nLength)
{
	uint8_t   ucLRC, sBuff[2];
	uint16_t  i;
	int   iRet;
	uint8_t   szBuf[1024];

	// prepare data
	vLong2Bcd((uint32_t)nLength, 2, sBuff);
	ucLRC = sBuff[0] ^ sBuff[1] ^ ETX;
	for(i=0; i<nLength; i++)
	{
		ucLRC ^= psBuff[i];
	}

	portClose(gstPosCapability.uiPortType);
	portOpen(gstPosCapability.uiPortType, (char *)"9600, 8, n, 1");

	memset(szBuf, 0, sizeof(szBuf));
	szBuf[0] = STX;
	szBuf[1] = sBuff[0];
	szBuf[2] = sBuff[1];
	memcpy(&szBuf[3], psBuff, nLength);
	szBuf[3+nLength] = ETX;
	szBuf[4+nLength] = ucLRC;
	iRet = portSends(gstPosCapability.uiPortType, szBuf, nLength+5);
	if( iRet!=PORT_RET_OK )
	{
		lcdClrLine(2, 7);
		lcdDisplay(0, 2, DISP_CFONT, "Send error!");
		return;
	}
	portClose(gstPosCapability.uiPortType);


}

uint16_t uiNacRecv(uint8_t ucPort, uint8_t *psBuff, uint32_t *puiLen, uint32_t nTimeOut)
{
	uint8_t   ucLRC, ucSTX, sLength[2];
	uint16_t  i, uiRet;
	uint32_t  ulLen;

	portClose(gstPosCapability.uiPortType);
	portOpen(gstPosCapability.uiPortType, (char *)"9600, 8, n, 1");

	// get STX
	while(1)
	{
		uiRet = uiPortRecvBuf(ucPort, &ucSTX, 1, nTimeOut);
		if( uiRet!=0 )
		{
			return uiRet;
		}
		if( ucSTX==STX )
		{
			break;
		}
	}

	// get L1 & L2
	uiRet = uiPortRecvBuf(ucPort, sLength, 2, nTimeOut);
	if( uiRet!=0 )
	{
		return uiRet;
	}
	ucLRC = sLength[0] ^ sLength[1];
	vBcd2Long(sLength, 2, &ulLen);
	if( ulLen>LEN_MAX_RS232 )
	{
		return 0xFE;
	}
	*puiLen = (uint32_t)ulLen;
	ulLen   += 2;


	// get body & ETX & LRC
	uiRet = uiPortRecvBuf(ucPort, psBuff, (uint16_t)ulLen, nTimeOut);
	if( uiRet!=0 )
	{
		return uiRet;
	}

	if( psBuff[ulLen-2]!=ETX )
	{
		return 0xFE;
	}

	for(i=0; i<ulLen; i++)
	{
		ucLRC ^= psBuff[i];
	}
	if( ucLRC!=0 )
	{
		return 0xFE;
	}

	portClose(gstPosCapability.uiPortType);

	return 0;
}

void AfterProcQpboc(void)
{
	EMV_APP_TYPE AppType;
	uint8_t  szBuffer[200];
	uint8_t  uLen;
	uint32_t Amount;

	ledSetStatus(LED_TRANS_OK );
	sysDelayMs(750);
	_root_beep(0,0);
	memset(szBuffer,0,sizeof(szBuffer));
	memset(PosCom.stTrans.szCardNo,0,sizeof(PosCom.stTrans.szCardNo));
	emv_get_data_element("\x5A", 1, &uLen, szBuffer);
	PubBcd2Asc((char*)szBuffer,uLen,(char*)PosCom.stTrans.szCardNo);
	PubTrimTailChars((char*)PosCom.stTrans.szCardNo,'F');

	memset(szBuffer,0,sizeof(szBuffer));
	emv_get_data_element("\x4F", 1, &uLen, szBuffer);	// AID
	BcdToAsc0(PosCom.stTrans.szAID, (uint8_t*)szBuffer, uLen*2);
	RemoveTailChars(PosCom.stTrans.szAID, 'F');

	lcdClrLine(2,7);
	lcdDisplay(0, 2, DISP_MEDIACY | DISP_CFONT, "PLS REMOVE CARD");

	memset(szBuffer,0,sizeof(szBuffer));
	if (0 == emv_get_data_element("\x9F\x5D", 2, &uLen, szBuffer))
	{
		Amount = PubBcd2Long(szBuffer,uLen,NULL);
		lcdDisplay(0, 4, DISP_CFONT, "BALANCE:");
		lcdDisplay(0, 6, DISP_MEDIACY|DISP_CFONT,"%d.%02d", Amount/100, Amount%100);
	}
	lcdFlip();
	PubWaitKey(1);
	contactless_poweroff(imif_fd);
	contactless_close(imif_fd);
	imif_fd = -1;

	emv_qpboc_get_app_type(&AppType);
	if (EMV_MSD == AppType)
	{
		emv_set_data_element("\x9F\x39", 2, 1, "\x91");
	}
	else if (EMV_QPBOC == AppType)
	{
		emv_set_data_element("\x9F\x39", 2, 1, "\x07");
	}
	else if (EMV_PBOC == AppType)
	{
		emv_set_data_element("\x9F\x39", 2, 1, "\x98");
	}

	PosCom.ucSwipedFlag = CARD_PASSIVE;
	PosCom.stTrans.ucSwipeFlag = PosCom.ucSwipedFlag;
	PosCom.bSkipEnterAmt = TRUE;
	GetAndSetQpbocElement();
}

uint8_t SelectTransTypeAmount(void)
{
	uint8_t iRet;
	uint8_t ucRet;
	
	if((stTemp.iTransNo == POS_SALE && PosCom.stTrans.TransFlag == PRE_TIP_SALE )
		|| (stTemp.iTransNo == POS_PREAUTH && PosCom.stTrans.TransFlag == POS_PREAUTH)
		|| (stTemp.iTransNo == POS_SALE && PosCom.stTrans.TransFlag == NORMAL_SALE) ){

RESELECT1:
	
		iRet = SelectSaleType();
			if( iRet == 1)
			{	
				iRet = InputWaiterNo();
				if(iRet != OK){
					return iRet;
				}
			}
			else if(iRet == 2)
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
			if(stTemp.iTransNo == POS_PREAUTH && PosCom.stTrans.TransFlag == POS_PREAUTH)
				ucRet = ComfirAmount(PosCom.stTrans.preAuthOriAmount);  //预授权金额只显示预授权部分，不显示超过的部分		
			else
				ucRet = ComfirAmount(PosCom.stTrans.sAmount);
			if(ucRet == RE_SELECT){
				goto RESELECT1;
			}
			else if( ucRet!=OK )
			{
				return ucRet;
			}
		}
	return OK;
}

extern int(*__ICCARD_EXCHANGE_APDU)(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdat);
uint8_t ContackCardProc(void)
{
	int ierrorno = EMV_QPBOC_OK,iRet=0;
    EMV_APP_TYPE AppType;
	uint32_t iReadCnt;
	int  iNeedSaveLog;
	uint8_t uLen,CID[8];

	AppMagClose();//magstripe Effect of offline transaction speed

	if (CheckSupport(EC_QUICK_SALE)==FALSE)
	{
		ledSetStatus(LED_TRANS_FAIL);
		lcdClrLine(2,7);
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, " NOT SUPPORT ");
		lcdFlip();
		QPBOCErrorBeep();
		kbGetKeyMs(5000);
		return NO_DISP;
	}

	emv_set_data_element("\x9F\x39",2,1, "\x07");
	sprintf((char *)PosCom.szRespCode, "00");
	PosCom.bOnlineTxn = FALSE;
	iReadCnt = 0;
	while(1)
	{
		iReadCnt++;
		if (iReadCnt >= 3)
		{
			DisplayQpbocError(ierrorno);
			return NO_DISP;
		}
		ierrorno = emv_qpboc_read_data(imif_fd);
		if (EMV_QPBOC_NEED_RETRY == ierrorno)
		{
			continue;
		}
		break;
	}
	if(ierrorno == EMV_QPBOC_OK)
	{
#ifndef _POS_TYPE_8110
		_root_beep(1000, 200);//8210,7110Non blocking buzzer
#endif
		iRet = emv_qpboc_complete();
		if ( iRet != EMV_QPBOC_OK)
		{
			_root_beep(0,0);

			GetAndSetQpbocElement();
			CID[0] = 0x00;
			emv_get_data_element("\x9F\x27", 2, &uLen, CID);
			iNeedSaveLog = emv_qpboc_need_save_failed_log();
			if (iNeedSaveLog || CID[0]==0x40)
			{
				if (PosCom.bOnlineTxn)
				{
					SaveIccFailureTranData(FALSE);
				}
				else
				{
					SaveIccFailureTranData(TRUE);
				}
				SaveIccFailureFile();
			}

			DisplayQpbocError(iRet);

			if(stPosParam.ucReversalTime == PARAM_OPEN)
			{
				iRet = ReverseTranProc();
				if (iRet==0)
				{
					lcdClrLine(2,7);
					lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"REVERSAL SUCCESS");
					lcdFlip();
					PubBeepOk();
					PubWaitKey(3);
				}
			}
			return NO_DISP;
		}

		if( !PosCom.bOnlineTxn )
		{
			AfterProcQpboc();
		}
		return 0;
	}
	else if (ierrorno == EMV_QPBOC_CARD_IN_BLACK_LIST)
	{
		ledSetStatus(LED_TRANS_FAIL);
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"BLACK CARD");
		lcdFlip();
		QPBOCErrorBeep();
		PubWaitKey(5);
		return NO_DISP;
	}
	else
	{
		emv_qpboc_get_app_type(&AppType);
		if (EMV_MSD == AppType)
		{
			emv_set_data_element("\x9F\x39", 2, 1, "\x91");
		}
		else if (EMV_QPBOC == AppType)
		{
			emv_set_data_element("\x9F\x39", 2, 1, "\x07");
		}
		else if (EMV_PBOC == AppType)
		{
			emv_set_data_element("\x9F\x39", 2, 1, "\x98");
		}
		iNeedSaveLog = emv_qpboc_need_save_failed_log();
		if (iNeedSaveLog)
		{
			PosCom.stTrans.ucSwipeFlag = PosCom.ucSwipedFlag = CARD_PASSIVE;
			GetAndSetQpbocElement();
			SaveIccFailureTranData(TRUE);
			SaveIccFailureFile();
		}

		DisplayQpbocError(ierrorno);
		return NO_DISP;
	}
}


int read_uicc_mutecard(void)
{
    int cnt=0, var_i, dataLen, fd, ret;
    uicc_mute_t tmp_mute;
    
    fd = fileOpen(UICC_MUTE_FILE, O_RDWR);
    if ( fd < 0 ) {
        return -1;
    }
    
    dataLen = fileSeek(fd, (int)0, SEEK_END);
    cnt = dataLen/sizeof(tmp_mute);
    
    ret = fileSeek(fd, (int)0, SEEK_SET);
    if ( ret < 0 ) {
        fileClose(fd);
        return -1;
    }
    
    for ( var_i=0; var_i<cnt; var_i++ ) {
        fileRead(fd, (u8*)&tmp_mute, sizeof(tmp_mute));
        ret = uicc_qpboc_load_mute_record((u8*)tmp_mute.buf, tmp_mute.len);
    }
    
    return 0;
}


int ClpbocPre(void)
{
  int ierrorno = EMV_QPBOC_OK;
  struct emv_core_init Init;
  uint8_t TransType;
  struct qpboc_parameters  gstQpbocParamters;

  PosCom.ucSwipedFlag = NO_SWIPE_INSERT;
  PosCom.stTrans.ucQPBOCorPBOC = 1;

  TransType = 0x00;
  memcpy(gstQpbocParamters.m_TransactionProperty, "\x46\x00\x00\x80", 4);//走非接PBOC

  gstQpbocParamters.m_StatusCheckSupported = 0;
  ierrorno = emv_qpboc_set_parameter(&gstQpbocParamters);
  if (EMV_QPBOC_OK != ierrorno)
  {
    //ledSetStatus(LED_TRANS_FAIL); //交易失败指示灯
    return E_SET_PARAM_FAIL;
  }

  TransType = 0x00;
  emv_get_core_init_parameter(&Init);
  switch (Init.transaction_type)
  {
    case EMV_CASH:
      TransType = 0x01;
      break;
    case EMV_CASHBACK:
      TransType = 0x09;
      break;
    case EMV_GOODS:
      TransType = 0x00;
      break;
    case EMV_SERVICE:
      TransType = 0x00;
      break;
    case EMV_INQUIRY:
      TransType = 0x10;
      break;
    case EMV_TRANSFER:
      TransType = 0x11;
      break;
    case EMV_PAYMENT:
      TransType = 0x12;
      break;
    case EMV_ADMIN:
      TransType = 0x13;
      break;
  }
  __ICCARD_EXCHANGE_APDU = contactless_exchangeapdu;
  return 0;
}

int ClpbocSelApp(void)
{
  int   iRet, iErrNo;

  DispICCProcessing();
  lcdFlip();
  kbFlush();

  emv_set_data_element("\x9F\x66", 2, 4, "\x46\x00\x00\x80");

  icc_fd = imif_fd;

  emv_get_mck_configure(&tConfig);
  tEMVCoreInit.transaction_type = EMV_GOODS;

  if ( stTemp.iTransNo==POS_QUE || stTemp.iTransNo==POS_PREAUTH || stTemp.iTransNo == POS_SALE
       || stTemp.iTransNo == EC_TOPUP_CASH || stTemp.iTransNo == EC_TOPUP_SPEC
       || stTemp.iTransNo == EC_TOPUP_NORAML || stTemp.iTransNo==EC_VOID_TOPUP)
  {
    tConfig.support_forced_online_capability = 1;
  }
  else
  {
    tConfig.support_forced_online_capability = 0;
  }

  SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);

  emv_set_mck_configure(&tConfig);
  switch (stTemp.iTransNo)
  {
    case EC_TOPUP_CASH:
    case EC_VOID_TOPUP:
      tEMVCoreInit.transaction_type = 0x63;
      break;
    case EC_TOPUP_SPEC:
      tEMVCoreInit.transaction_type = 0x60;
      break;
    case EC_TOPUP_NORAML:
      tEMVCoreInit.transaction_type = 0x62;
      break;
    default:
      tEMVCoreInit.transaction_type = EMV_GOODS;
      break;
  }
  emv_set_core_init_parameter(&tEMVCoreInit);
  gl_FirstRunSelApp = 0x01;

  __ICCARD_EXCHANGE_APDU = contactless_exchangeapdu;
  iRet = emv_application_select(icc_fd, stPosParam.lNowTraceNo);
  iErrNo = iRet?errno:0;
  DispTransName();

  // ADVT case 26
  if ( iErrNo==EMV_ERRNO_DATA || iErrNo==EMV_ERRNO_NO_APP ||
       iErrNo==EMV_ERRNO_SW )
  {
    PosCom.ucFallBack = TRUE;
    DispFallback();
    return E_NEED_FALLBACK;
  }
  if ( iErrNo==EMV_ERRNO_CANCEL )
  {
    return E_TRANS_CANCEL;
  }
  if ( iErrNo==EMV_ERRNO_CARD_BLOCKED )
  {
    return E_CARD_BLOCKER;
  }
  if ( iErrNo==EMV_ERRNO_APP_BLOCKED )
  {
    return E_APP_BLOCKED;
  }
  if ( iErrNo!=EMV_RET_SUCCESS )
  {
    return E_TRANS_FAIL;
  }

  DispICCProcessing();
  lcdFlip();
  sysBeef(3, 100);
  sysDelayMs(100);
  return OK;
}

uint8_t ClpbocGetCard(void)
{
  if (ClpbocSelApp() != OK)
    return E_TRANS_FAIL;

  return ContactlessPBOCProc();
}
// YYYYMMDDhhmmss
void GetDateTime(uint8_t *pszDateTime)
{
	uint8_t	sCurTime[7];

	sysGetTime(sCurTime);
	sprintf((char *)pszDateTime, "%02X%02X%02X%02X%02X%02X%02X",
			0x20, sCurTime[0], sCurTime[1],
			sCurTime[2], sCurTime[3], sCurTime[4], sCurTime[5]);
}

uint8_t ContactlessPBOCProc(void)
{
	int		iRet, iErrNo;
	uint8_t  uLen, sBuff[40];

	icc_fd = imif_fd;
	emv_pboc_ec_enable(0);

	emv_get_mck_configure(&tConfig);
	tEMVCoreInit.transaction_type = EMV_GOODS;

	if( stTemp.iTransNo==POS_QUE || stTemp.iTransNo==POS_PREAUTH || stTemp.iTransNo == POS_SALE
		|| stTemp.iTransNo == EC_TOPUP_CASH || stTemp.iTransNo == EC_TOPUP_SPEC
		|| stTemp.iTransNo == EC_TOPUP_NORAML || stTemp.iTransNo == EC_VOID_TOPUP)
	{
		tConfig.support_forced_online_capability = 1;
	}
	else
	{
		tConfig.support_forced_online_capability = 0;
	}

	SHA1((uint8_t*)&tConfig, sizeof(struct terminal_mck_configure)-20, tConfig.checksum);

	emv_set_mck_configure(&tConfig);
	switch(stTemp.iTransNo)
	{
	case EC_TOPUP_CASH:
	case EC_VOID_TOPUP:
		tEMVCoreInit.transaction_type = 0x63;
		break;
	case EC_TOPUP_SPEC:
		tEMVCoreInit.transaction_type = 0x60;
		break;
	case EC_TOPUP_NORAML:
		tEMVCoreInit.transaction_type = 0x62;
		break;
	default:
		tEMVCoreInit.transaction_type = EMV_GOODS;
		break;
	}
	emv_set_core_init_parameter(&tEMVCoreInit);
	sysDelayMs(100);

	iRet = emv_read_app_data(icc_fd);
	iErrNo = iRet?errno:0;
	if( iErrNo==EMV_ERRNO_CANCEL )
	{
		return E_TRANS_CANCEL;
	}
	if( iErrNo!=EMV_RET_SUCCESS )
	{
		return E_TRANS_FAIL;
	}

	SetEmvTranType();	// set emv transaction type, tag 9C
	emv_set_data_element("\x9F\x53", 2, 1, "\x52");
	memset(sBuff,0,sizeof(sBuff));
	uLen = 0;
	emv_get_data_element("\x4F", 1, &uLen, sBuff);	// AID
	BcdToAsc0(PosCom.stTrans.szAID, sBuff, uLen*2);

	if( stTemp.iTransNo==POS_SALE || stTemp.iTransNo==POS_QUE ||
		stTemp.iTransNo==POS_PREAUTH ||
		(stTemp.iTransNo>=EC_NORMAL_SALE && stTemp.iTransNo <= EC_TOPUP_NORAML) )
	{
		iRet = emv_offline_data_authentication(icc_fd);
		iErrNo = iRet?errno:0;
		if( iErrNo!=EMV_RET_SUCCESS )
		{
			return E_TRANS_FAIL;
		}

		if (stTemp.iTransNo == EC_TOPUP_NORAML)
		{
			uLen = 0;
			memset(sBuff,0,sizeof(sBuff));
			emv_get_data_element("\x95", 1, &uLen, sBuff);
			if (sBuff[0]&0x48)
			{
				return E_TRANS_FAIL;
			}
		}
	}

	// Read Track 2 and/or Pan
	iRet = GetEmvTrackData();
	if( iRet<0 )
	{
		return E_TRANS_FAIL;
	}

	if (ChkOnlyEcCard(PosCom.stTrans.szAID) &&
		(stTemp.iTransNo==POS_PREAUTH || stTemp.iTransNo==POS_QUE))
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PURE E-CASH");
		lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"NOT SUPPORT");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return E_TRANS_FAIL;
	}

	if (ChkOnlyEcCard(PosCom.stTrans.szAID) && (stTemp.iTransNo==POS_SALE))
	{
		iRet = JudgeEcCard();
		if (iRet != 0)
		{
			return E_TRANS_FAIL;
		}
	}

	if (ChkOnlyEcCard(PosCom.stTrans.szAID) && stTemp.iTransNo == EC_TOPUP_SPEC)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"PURE E-CASH");
		lcdDisplay(0,5,DISP_CFONT|DISP_MEDIACY,"NOT SUPPORT");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return E_TRANS_FAIL;
	}

	PosCom.ucSwipedFlag = CARD_PASSIVE;

	return DispCardNo(PosCom.stTrans.szCardNo);
}

uint8_t SelectPBOCTrans(void)
{
    int key;
	int keySub;

	const char *mainMenu[] =
	{
		"1.CONTACT SALE  ",
		"2 LOAD          ",
	};
	const char *Ec_Topup_menu[] =
	{
		"1.CASH LOAD    ",
		"2.ACCT LOAD    ",
		"3.NO-ACCT LOAD ",
		"4.VOID CASH    "
	};

	while(1)
	{
		key = DispMenu(mainMenu, 2, 0, KEY1, KEY2, stPosParam.ucOprtLimitTime);
		switch(key)
		{
		case KEY1:
			stTemp.iTransNo = POS_SALE;
			PosCom.ucPBOCFlag = 2;
			return OK;
			break;
		case KEY2:
			keySub = DispMenu(Ec_Topup_menu, 4, 0, KEY1, KEY4, stPosParam.ucOprtLimitTime);
			if( keySub==KEY1 )
				stTemp.iTransNo = EC_TOPUP_CASH;
			else if( keySub==KEY2 )
				stTemp.iTransNo = EC_TOPUP_SPEC;
			else if( keySub==KEY3 )
				stTemp.iTransNo = EC_TOPUP_NORAML;
			else if( keySub==KEY4 )
				stTemp.iTransNo = EC_VOID_TOPUP;
			return OK;
			break;
		case KEY_ENTER: //1~2
			break;
		default:
			return NO_DISP;
			break;
		}
	}
	return(NO_DISP);
}
// end file

// 按格式显示金额 1,000,000.00
void FormAmountFormmat(uint8_t *BcdAmount,uint8_t *AscAmount)
{	
	int szAmountLen = 0;
	long szlAmount;
	char szAscAmountTemp[30];

	szlAmount = atol((char*)BcdAmount);
	sprintf(szAscAmountTemp,"%ld%02ld",szlAmount/100,szlAmount%100);	
	szAmountLen = strlen(szAscAmountTemp);
	

	if( szAmountLen <= 5 )
	{
		sprintf((char*)AscAmount,"%ld.%02ld",szlAmount/100,szlAmount%100);
	}
	else if( szAmountLen == 6 )
	{
		sprintf((char*)AscAmount,"%.1s,%.3s.%.2s",(char*)szAscAmountTemp,(char*)szAscAmountTemp+1,(char*)szAscAmountTemp+szAmountLen-2);
	}
	else if( szAmountLen == 7 )
	{
		sprintf((char*)AscAmount,"%.2s,%.3s.%.2s",(char*)szAscAmountTemp,(char*)szAscAmountTemp+2,(char*)szAscAmountTemp+szAmountLen-2);
	}
	else if( szAmountLen == 8 )
	{
		sprintf((char*)AscAmount,"%.3s,%.3s.%.2s",(char*)szAscAmountTemp,(char*)szAscAmountTemp+3,(char*)szAscAmountTemp+szAmountLen-2);
	}
	else if( szAmountLen == 9 )
	{
		sprintf((char*)AscAmount,"%.1s,%.3s,%.3s.%.2s",(char*)szAscAmountTemp,(char*)szAscAmountTemp+1,(char*)szAscAmountTemp+4,(char*)szAscAmountTemp+szAmountLen-2);
	}
	else if( szAmountLen == 10 )
	{
		sprintf((char*)AscAmount,"%.2s,%.3s,%.3s.%.2s",(char*)szAscAmountTemp,(char*)szAscAmountTemp+2,(char*)szAscAmountTemp+5,(char*)szAscAmountTemp+szAmountLen-2);
	}
}

static int GetEndKey(void)
{
    int iEndKey;

    iEndKey = (gstDynamicMenu.ucExsitNum + KEY0);

    return iEndKey;
}

static int Key2Value(int iKey)
{
    int iValue;

    iValue = (iKey-KEY1);

    return iValue;
}

void InitMenu(uint8_t ucMode, const char * pszTitle)
{
    int i;

    memset(&gstDynamicMenu, 0, sizeof(T_Dynamic_Menu));
    gstDynamicMenu.ucExsitNum = 0;
    gstDynamicMenu.ucTotalNum = MAX_ITEM_MENU;
    gstDynamicMenu.bTitle = FALSE;
    gstDynamicMenu.ucExistLines=0;
    gstDynamicMenu.bLastMenu = FALSE;
    gstDynamicMenu.pLastMenu = NULL;
    gstDynamicMenu.ucExistHideMenu = 0;
    gstDynamicMenu.ucExistHideItem = 0;

    gstDynamicMenu.ucMenuMode = (ucMode == MENU_MODE_1)? MENU_MODE_1 : MENU_MODE_2;

#if 1
    if(pszTitle)
    {
        gstDynamicMenu.bTitle = TRUE;
        sprintf(gstDynamicMenu.szTitle, "%s", pszTitle);
    }
#endif

    for(i=0; i<MAX_ITEM_MENU+1; i++)
    {
        gstDynamicMenu.pSubMenuFunc[i] = NULL;

        if(i < MAX_HIDE_MENU)
            gstDynamicMenu.stHideMenu[i].pHideMenu = NULL;
    }
}

void AddMenuTitle(char * pszTitle)
{
    if(pszTitle)
    {
        gstDynamicMenu.bTitle = TRUE;
        sprintf(gstDynamicMenu.szTitle, "%s", pszTitle);
    }
    else
    {
        gstDynamicMenu.bTitle = FALSE;
        memset(gstDynamicMenu.szTitle, 0, sizeof(gstDynamicMenu.szTitle));
    }
}

void AddMenuItem(uint8_t bVisible, int iTransNo, char *pszName, SubMenuFunc subMenu)
{
    uint8_t bExist;

    bExist = (bVisible == PARAM_OPEN)? TRUE : FALSE;

    if(bExist == FALSE)
        return ;

    if(gstDynamicMenu.ucExsitNum >= gstDynamicMenu.ucTotalNum)
        return ;

    gstDynamicMenu.szTrans[gstDynamicMenu.ucExsitNum] = iTransNo;

    sprintf(gstDynamicMenu.szMenuItem[gstDynamicMenu.ucExsitNum], "%d.%s", gstDynamicMenu.ucExsitNum+1, pszName);

    gstDynamicMenu.pSubMenuFunc[gstDynamicMenu.ucExsitNum] = subMenu;

    gstDynamicMenu.ucExsitNum += 1;

#ifdef __DEBUG
    //printf("\nszTrans    = %d\n", gstDynamicMenu.szTrans[gstDynamicMenu.ucExsitNum-1]);
    //printf("szMenuItem = %s\n", gstDynamicMenu.szMenuItem[gstDynamicMenu.ucExsitNum-1]);
    //printf("ucExsitNum = %d\n\n", gstDynamicMenu.ucExsitNum);
#endif
}

void AddLastMenu(SubMenuFunc LastMenu)
{
    if(LastMenu)
    {
        gstDynamicMenu.bLastMenu = TRUE;
        gstDynamicMenu.pLastMenu = LastMenu;
    }
    else
    {
        gstDynamicMenu.bLastMenu = FALSE;
        gstDynamicMenu.pLastMenu = NULL;
    }
}

int FindHideMenu(int iKey)
{
    int i=0;

    if(gstDynamicMenu.ucExistHideMenu == 0)
        return -1;

    for(i=0; i<gstDynamicMenu.ucExistHideMenu; i++)
    {
        if(iKey == gstDynamicMenu.stHideMenu[i].iHideMenuKey)
            break;
    }
    if(i == gstDynamicMenu.ucExistHideMenu)
        return -1;

    if(gstDynamicMenu.stHideMenu[i].pHideMenu == NULL)
        return -1;

    return i;
}

void AddHideMenu(int iKey, SubMenuFunc HideMenu)
{
    if(gstDynamicMenu.ucExistHideMenu >= MAX_HIDE_MENU)
        return ;

    if(!HideMenu)
    {
        return ;
    }

    gstDynamicMenu.stHideMenu[gstDynamicMenu.ucExistHideMenu].iHideMenuKey = iKey;
    gstDynamicMenu.stHideMenu[gstDynamicMenu.ucExistHideMenu].pHideMenu = HideMenu;
    gstDynamicMenu.ucExistHideMenu += 1;
}

void AddHideItem(int iKey, int iTransNo)
{
    if(iTransNo <= 0)
    {
        return ;
    }

    gstDynamicMenu.stHideItem[gstDynamicMenu.ucExistHideItem].iHideItemkey = iKey;
    gstDynamicMenu.stHideItem[gstDynamicMenu.ucExistHideItem].iTransNo = iTransNo;
    gstDynamicMenu.ucExistHideItem += 1;
}

int FindHideItem(int iKey)
{
    int i=0;

    if(gstDynamicMenu.ucExistHideItem == 0)
        return -1;

    for(i=0; i<gstDynamicMenu.ucExistHideItem; i++)
    {
        if(iKey == gstDynamicMenu.stHideItem[i].iHideItemkey)
            break;
    }
    if(i == gstDynamicMenu.ucExistHideItem)
        return -1;

    return i;
}

static int AdjustPerLine(const char *pszMenu1, const char *pszMenu2, char *pszOutBuf)
{
    int iLen1, iLen2;
    int iHalf;
    char szBuf[MAX_CHARS_PERLINE+1];

    if(!pszMenu1 || pszMenu1[0] == 0)
        return -2;

    if(!pszMenu2 || pszMenu2[0] == 0)
        return -1;

    iHalf = (MAX_CHARS_PERLINE-1)/2;
    iLen1 = strlen((char *)pszMenu1);
    iLen2 = strlen((char *)pszMenu2);

    if( (gstDynamicMenu.ucMenuMode == MENU_MODE_2) ||
        ((iLen1 + iLen2 + 1) > MAX_CHARS_PERLINE) ||
        ((iLen1 + 1) > iHalf) ||
        (iLen2 > iHalf) )
    {
        return -1;
    }

    memset(szBuf, 0, sizeof(szBuf));
    memcpy(szBuf, pszMenu1, iLen1);
    memset(&szBuf[iLen1], ' ', iHalf-iLen1+1);
    memcpy(&szBuf[iHalf+1], pszMenu2, iLen2);

    sprintf(pszOutBuf, "%s", szBuf);

    //memset(szBuf, ' ', MAX_CHARS_PERLINE);
    //memcpy(szBuf, pszMenu1, iLen1);
    //memcpy(szBuf+iHalf+1, pszMenu2, iLen2);

    //memcpy(pszOutBuf, szBuf, MAX_CHARS_PERLINE);
    //pszOutBuf[MAX_CHARS_PERLINE]=0;

    return 0;
}

static int FlushMenu(void)
{
    int i, j;
    int iRet;

    memset(gstDynamicMenu.szDynamicMenu, 0, sizeof(gstDynamicMenu.szDynamicMenu));

    if(gstDynamicMenu.bTitle == TRUE)
    {
        sprintf(gstDynamicMenu.szDynamicMenu[0], "%s", gstDynamicMenu.szTitle);
        j = 1;
    }
    else
    {
        j = 0;
    }

    i = 0;
    while(1)
    {
        if(gstDynamicMenu.szMenuItem[i][0] == 0)
            break;

        if(gstDynamicMenu.szMenuItem[i+1][0] == 0)
        {
            sprintf(gstDynamicMenu.szDynamicMenu[j], "%s", gstDynamicMenu.szMenuItem[i]);
            j += 1;
            i += 1;
            break;
        }

        iRet = AdjustPerLine(gstDynamicMenu.szMenuItem[i],
            gstDynamicMenu.szMenuItem[i+1], gstDynamicMenu.szDynamicMenu[j]);
        if(iRet == -2)
        {
            break;
        }
        else if(iRet == -1)
        {
            sprintf(gstDynamicMenu.szDynamicMenu[j], "%.*s", MAX_CHARS_PERLINE, gstDynamicMenu.szMenuItem[i]);
            j += 1;
            i += 1;

            if(gstDynamicMenu.szMenuItem[i][0] == 0)
                break;
        }
        else if(iRet == 0)
        {
            i += 2;
            j += 1;
        }


        if(i >= gstDynamicMenu.ucExsitNum || j >= MAX_ITEM_MENU)
            break;
    }

    if( j == 0 || (gstDynamicMenu.bTitle && j==1) )
        return -1;


    gstDynamicMenu.ucExistLines = j;

    #if 0 //YuanZheng 20161030 14:30
    for(i=0; i<j; i++)
    {
        //gstDynamicMenu.szDynamicMenu[i+gstDynamicMenu.bTitle] = szMenu[i];
        printf("FlushMenu, szDynamicMenu[%d] = %s\n", i,gstDynamicMenu.szDynamicMenu[i]);
    }
    #endif

    return 0;
}

static int DispFlushMenu(int iStartKey, int iEndKey, int iTimeOut)
{
    int screens;
    int start_line;
    int start_item;
    int line_per_screen,max_line_screen;
    int cur_screen;
    int i;
    int t;
    int key;

    uint8_t bHideFunc=FALSE;
    uint8_t bHideItem=FALSE;

//#ifdef __DEBUG
#if 0
    lcdClrLine(start_line, 7);
    for(i=0; i<gstDynamicMenu.ucExistLines; i++)
    {
        printf("DisplayMenu[%d] = %s\n", i,gstDynamicMenu.szDynamicMenu[i]);
        lcdDisplay(0, i*2, DISP_CFONT, (char *)gstDynamicMenu.szDynamicMenu[i]);
    }
    lcdFlip();
    kbGetKey();
#endif

    if(gstDynamicMenu.ucExistHideMenu > 0)
    {
        bHideFunc = TRUE;
    }

    if(gstDynamicMenu.ucExistHideItem > 0)
    {
        bHideItem = TRUE;
    }

    if (gstPosCapability.uiScreenType)
    {
        max_line_screen = 6;//6
    }
    else
    {
        max_line_screen = 4;
    }

    lcdClrLine(4,10);
    if( gstDynamicMenu.bTitle )
    {
        start_item = 1;
        screens = (gstDynamicMenu.ucExistLines-2)/(max_line_screen-1) + 1;
        start_line = 6;
        line_per_screen = max_line_screen-1;
        lcdDisplay(0, 0, DISP_HFONT16|DISP_INVLINE|DISP_MEDIACY, (char *)gstDynamicMenu.szDynamicMenu[0]);/////
        //printf("++++++++++ %s +++++++++\n", gstDynamicMenu.szDynamicMenu[0]);
    }
    else
    {
        start_item = 0;
        screens = (gstDynamicMenu.ucExistLines-1)/max_line_screen + 1;//////
        start_line = 6;
        line_per_screen = max_line_screen;
    }

    cur_screen = 0;

    while(1)
    {
        if( cur_screen<screens-1 )
            lcdSetIcon(ICON_DOWN, OPENICON);
        else
            lcdSetIcon(ICON_DOWN, CLOSEICON);
        if( cur_screen>0 )
            lcdSetIcon(ICON_UP, OPENICON);
        else
            lcdSetIcon(ICON_UP, CLOSEICON);

        lcdClrLine(4, 14);
        //lcdClrLine(start_line, 7);
        for(i=0; i<line_per_screen; i++)
        {
            t = i + line_per_screen*cur_screen + start_item;

            if( t<gstDynamicMenu.ucExistLines )
            {
				lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
                lcdDisplay(0, (i*2+start_line), DISP_MEDIACY|DISP_CFONT/*DISP_HFONT16*/, gstDynamicMenu.szDynamicMenu[t]);
				lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
			}
        }
        lcdFlip();
        key = kbGetKeyMs(iTimeOut*1000);
        if( key==KEY_CANCEL || key == KEY_TIMEOUT )
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
            lcdSetIcon(ICON_UP,   CLOSEICON);
            return key;
        }
        else if( key==KEY_ENTER || key==KEY_DOWN )
        {
            if( cur_screen>=screens-1 )
                cur_screen = 0;
            else
                cur_screen++;
        }
        else if( key==KEY_UP )
        {
            if( cur_screen<=0 )
                cur_screen = screens-1;
            else
                cur_screen--;
        }
        else if( key>=iStartKey && key<=iEndKey )
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
            lcdSetIcon(ICON_UP,   CLOSEICON);
            return key;
        }
        else if(bHideFunc || bHideItem)
        {
            if( bHideFunc && (FindHideMenu(key) >= 0) )
                return key;

            if( bHideItem && (FindHideItem(key) >= 0) )
                return key;
        }
    }
}


int DispDynamicMenu(int flag)
{
    int iRet, iHideRet;
    int iSelect;


    iRet = FlushMenu();
    if(iRet < 0)
    {
        return iRet;
    }

    while(1)
    {
    	if(flag == 0)
	{
		iRet = DispFlushMenu(KEY1, GetEndKey(), stPosParam.ucOprtLimitTime);
	}
	else 
	{
        	iRet = MainMenuDispFlushMenu(KEY1, GetEndKey(), stPosParam.ucOprtLimitTime,2);

	}

        if(iRet == KEY_CANCEL)
        {
            if(gstDynamicMenu.bLastMenu && gstDynamicMenu.pLastMenu)
            {
                iRet = (*gstDynamicMenu.pLastMenu)();
               // return iRet;
            }
            return NO_TRANS;
        }
        else if(iRet == KEY_TIMEOUT)
        {
            return NO_TRANS;
        }
        else if( iRet>=KEY1 && iRet<=GetEndKey() )
        {
            iSelect = Key2Value(iRet);
            if( gstDynamicMenu.pSubMenuFunc[iSelect] != NULL )// &&
            {
                iRet = (*gstDynamicMenu.pSubMenuFunc[iSelect])();
                return iRet;
            }
            else
            {
                break;
            }
        }
        else
        {
            iHideRet = FindHideMenu(iRet);
            if(iHideRet >= 0)
            {
                iRet = (*gstDynamicMenu.stHideMenu[iHideRet].pHideMenu)();
                //printf("stHideMenu, iRet=%d\n", iRet);
                return iRet;
            }

            iHideRet = FindHideItem(iRet);
            if(iHideRet >= 0)
            {
                //printf("stHideItem, iTransNo=%d\n", gstDynamicMenu.stHideItem[iHideRet].iTransNo);
                return gstDynamicMenu.stHideItem[iHideRet].iTransNo;
            }
        }
    }

    return gstDynamicMenu.szTrans[iSelect];
}

int DispNoSupport(void)
{
    lcdCls();
    lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "SORRY, THIS TRANSACTION");
    lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "NOT REALIZE");
    lcdDisplay(0, 8, DISP_CFONT|DISP_MEDIACY, "WE WILL UPDATE IT LATER");
    lcdFlip();
    ErrorBeep();
    kbGetKeyMs(5000);

    return 0;
}

int DispError(void)
{
    lcdCls();
    lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "NOT TRANSACTION SUPPORT");
    lcdFlip();
    ErrorBeep();
    kbGetKeyMs(5000);

    return 0;
}


void MesesAddMenuItem(uint8_t bVisible, int iTransNo, char *pszName, SubMenuFunc subMenu)
{
    uint8_t bExist;

    bExist = (bVisible == PARAM_OPEN)? TRUE : FALSE;

    if(bExist == FALSE)
        return ;

    if(gstDynamicMenu.ucExsitNum >= gstDynamicMenu.ucTotalNum)
        return ;

    gstDynamicMenu.szTrans[gstDynamicMenu.ucExsitNum] = iTransNo;

    sprintf(gstDynamicMenu.szMenuItem[gstDynamicMenu.ucExsitNum], "%d) %s", gstDynamicMenu.ucExsitNum+1, pszName);

    gstDynamicMenu.pSubMenuFunc[gstDynamicMenu.ucExsitNum] = subMenu;

    gstDynamicMenu.ucExsitNum += 1;

#ifdef __DEBUG
    //printf("\nszTrans    = %d\n", gstDynamicMenu.szTrans[gstDynamicMenu.ucExsitNum-1]);
    //printf("szMenuItem = %s\n", gstDynamicMenu.szMenuItem[gstDynamicMenu.ucExsitNum-1]);
    //printf("ucExsitNum = %d\n\n", gstDynamicMenu.ucExsitNum);
#endif
}

int MesesMenu(void)
{
    int iRet;
   
    InitMenu(MENU_MODE_1, "MAIN    MENU");

	if( stPosParam.Costom_Promotions == PARAM_CLOSE )
	{
		MesesAddMenuItem(PARAM_OPEN,    3,       "3 meses sin intereses",         NULL);
		MesesAddMenuItem(PARAM_OPEN,    6,       "6 meses sin intereses",         NULL);
		MesesAddMenuItem(PARAM_OPEN,    9,       "9 meses sin intereses",         NULL);
		MesesAddMenuItem(PARAM_OPEN,    12,       "12 meses sin intereses",         NULL);
		MesesAddMenuItem(PARAM_OPEN,    18,       "18 meses sin intereses",         NULL);
	}
	else 
	{
		MesesAddMenuItem(stPosParam.MonthFlag3,    3,       "3 meses sin intereses",         NULL);
		MesesAddMenuItem(stPosParam.MonthFlag6,    6,       "6 meses sin intereses",         NULL);
		MesesAddMenuItem(stPosParam.MonthFlag9,    9,       "9 meses sin intereses",         NULL);
		MesesAddMenuItem(stPosParam.MonthFlag12,    12,       "12 meses sin intereses",         NULL);
		MesesAddMenuItem(stPosParam.MonthFlag18,    18,       "18 meses sin intereses",         NULL);
	}

   
  
    iRet = DispDynamicMenu(0);
	
    return iRet;
}




void MainMenuAddMenuItem(uint8_t bVisible, int iTransNo, char *pszName, SubMenuFunc subMenu)
{
    uint8_t bExist;

    bExist = (bVisible == PARAM_OPEN)? TRUE : FALSE;

    if(bExist == FALSE)
        return ;

    if(gstDynamicMenu.ucExsitNum >= gstDynamicMenu.ucTotalNum)
        return ;

    gstDynamicMenu.szTrans[gstDynamicMenu.ucExsitNum] = iTransNo;

    sprintf(gstDynamicMenu.szMenuItem[gstDynamicMenu.ucExsitNum], "               %d-%s", gstDynamicMenu.ucExsitNum+1, pszName);

    gstDynamicMenu.pSubMenuFunc[gstDynamicMenu.ucExsitNum] = subMenu;

    gstDynamicMenu.ucExsitNum += 1;

#ifdef __DEBUG
    //printf("\nszTrans    = %d\n", gstDynamicMenu.szTrans[gstDynamicMenu.ucExsitNum-1]);
    //printf("szMenuItem = %s\n", gstDynamicMenu.szMenuItem[gstDynamicMenu.ucExsitNum-1]);
    //printf("ucExsitNum = %d\n\n", gstDynamicMenu.ucExsitNum);
#endif
}


/***
****根据flag 的值，纵坐标不同的偏移
***/
int MainMenuDispFlushMenu(int iStartKey, int iEndKey, int iTimeOut,int flag)
{
    int screens;
    int start_line;
    int start_item;
    int line_per_screen,max_line_screen;
    int cur_screen;
    int i;
    int t;
    int key;

    uint8_t bHideFunc=FALSE;
    uint8_t bHideItem=FALSE;

//#ifdef __DEBUG
#if 0
    lcdClrLine(start_line, 7);
    for(i=0; i<gstDynamicMenu.ucExistLines; i++)
    {
        printf("DisplayMenu[%d] = %s\n", i,gstDynamicMenu.szDynamicMenu[i]);
        lcdDisplay(0, i*2, DISP_CFONT, (char *)gstDynamicMenu.szDynamicMenu[i]);
    }
    lcdFlip();
    kbGetKey();
#endif

    if(gstDynamicMenu.ucExistHideMenu > 0)
    {
        bHideFunc = TRUE;
    }

    if(gstDynamicMenu.ucExistHideItem > 0)
    {
        bHideItem = TRUE;
    }

    if (gstPosCapability.uiScreenType)
    {
        max_line_screen = 5;//9
    }
    else
    {
        max_line_screen = 5;
    }

    //lcdClrLine(4,10);
    lcdClrLine(4,14);
    if( gstDynamicMenu.bTitle )
    {
        start_item = 1;
        screens = (gstDynamicMenu.ucExistLines-2)/(max_line_screen-1) + 1;
        start_line = 8;
        line_per_screen = max_line_screen-1;
        //lcdDisplay(0, 0, DISP_HFONT16|DISP_INVLINE|DISP_MEDIACY, (char *)gstDynamicMenu.szDynamicMenu[0]);/////
        //printf("++++++++++ %s +++++++++\n", gstDynamicMenu.szDynamicMenu[0]);DISP_CFONT
         lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)gstDynamicMenu.szDynamicMenu[0]);/////

    }
    else
    {
        start_item = 0;
        screens = (gstDynamicMenu.ucExistLines-1)/max_line_screen + 1;//////
        start_line = 8;
        line_per_screen = max_line_screen;
    }

    cur_screen = 0;

    while(1)
    {
        if( cur_screen<screens-1 )
            lcdSetIcon(ICON_DOWN, OPENICON);
        else
            lcdSetIcon(ICON_DOWN, CLOSEICON);
        if( cur_screen>0 )
            lcdSetIcon(ICON_UP, OPENICON);
        else
            lcdSetIcon(ICON_UP, CLOSEICON);

        lcdClrLine(4, 14);
        //lcdClrLine(start_line, 7);
        for(i=0; i<line_per_screen; i++)
	 {
            t = i + line_per_screen*cur_screen + start_item;

            if( t<gstDynamicMenu.ucExistLines )
            {
            		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
			if(flag ==1)
			{
					lcdDisplay(50, (i*2+start_line), /*DISP_MEDIACY|*/DISP_CFONT, gstDynamicMenu.szDynamicMenu[t]);
			}
			else
			{
					lcdDisplay(10, (i*2+start_line), /*DISP_MEDIACY|*/DISP_CFONT, gstDynamicMenu.szDynamicMenu[t]);
			}
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
            }
        }
/*        {
            t = i + line_per_screen*cur_screen + start_item;

            if( t<gstDynamicMenu.ucExistLines )
            {
            		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
                	lcdDisplay(50, (i*2+start_line),DISP_CFONT, gstDynamicMenu.szDynamicMenu[t]);
			lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
            }
        }
*/
        lcdFlip();
        key = kbGetKeyMs(iTimeOut*1000);
        if( key==KEY_CANCEL || key == KEY_TIMEOUT )
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
            lcdSetIcon(ICON_UP,   CLOSEICON);
            return key;
        }
        else if( key==KEY_ENTER || key==KEY_DOWN )
        {
            if( cur_screen>=screens-1 )
                cur_screen = 0;
            else
                cur_screen++;
        }
        else if( key==KEY_UP )
        {
            if( cur_screen<=0 )
                cur_screen = screens-1;
            else
                cur_screen--;
        }
        else if( key>=iStartKey && key<=iEndKey )
        {
            lcdSetIcon(ICON_DOWN, CLOSEICON);
            lcdSetIcon(ICON_UP,   CLOSEICON);
            return key;
        }
        else if(bHideFunc || bHideItem)
        {
            if( bHideFunc && (FindHideMenu(key) >= 0) )
                return key;

            if( bHideItem && (FindHideItem(key) >= 0) )
                return key;
        }
    }
}




// end file


