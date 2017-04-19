#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "tranfunclist.h"

void GetKeyDescription(int ucKey, char *buf);

void PosDeviceDetect(void)
{
	int iRet;

	lcdCls();	
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "Revision de terminal");

	iRet = prnInit();
  	if ( iRet!=OK )
  	{
  		lcdClrLine(2, 7);
		if(iRet==-1 && errno==ENOTCONN && gstPosCapability.uiPosType == 4)
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "PLS CONNECT BASE");
		else
    		DispMulLanguageString(0, 3, DISP_CFONT, NULL, "INIT Impresora FAIL");
    	lcdFlip();
    	sysDelayMs(3000);
		return;
  	}
	iRet = prnStatus();
	switch(iRet) 
	{
	case PRN_PAPEROUT:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Impresora...NO PAPER");
		lcdFlip();
		ErrorBeep();
		kbGetKey();
		break;
	case PRN_OK:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Impresora");
		DispMulLanguageString(0, 6, DISP_CFONT|DISP_MEDIACY, NULL, "detectada OK");
		lcdFlip();
		sysDelayMs(1000);
		break;
	case PRN_NOBATTERY:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Impresora...ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKey();
		break;	
	default:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PRINT...ERROR");
		lcdFlip();
		ErrorBeep();
		kbGetKey();
		break;
	}

	prnClose();
}

void GetKeyDescription(int ucKey, char *buf)
{
	switch(ucKey)
	{
	case KEY1:
		strcpy(buf, "KEY: 1");
		break;
	case KEY2:
		strcpy(buf, "KEY: 2");
		break;
	case KEY3:
		strcpy(buf, "KEY: 3");
		break;
	case KEY4:
		strcpy(buf, "KEY: 4");
		break;
	case KEY5:
		strcpy(buf, "KEY: 5");
		break;
	case KEY6:
		strcpy(buf, "KEY: 6");
		break;
	case KEY7:
		strcpy(buf, "KEY: 7");
		break;
	case KEY8:
		strcpy(buf, "KEY: 8");
		break;
	case KEY9:
		strcpy(buf, "KEY: 9");
		break;
	case KEY0:
		strcpy(buf, "KEY: 0");
		break;
	case KEY_ALPHA:
		strcpy(buf, "KEY: ALPHA");
		break;
	case KEY_ENTER:
		strcpy(buf, "KEY: ENTER");
		break;
	case KEY_BACKSPACE:
		strcpy(buf, "KEY: BACKSPACE");
		break;
	case KEY_CLEAR:
		strcpy(buf, "KEY: CLEAR");
		break;		
	case KEY_MENU:
		strcpy(buf, "KEY: MENU");
		break;
	case KEY_FN:
		strcpy(buf, "KEY: FUNCT");
		break;
	case KEY_UP:
		strcpy(buf, "KEY: UP");
		break;
	case KEY_DOWN:
		strcpy(buf, "KEY: DOWN");
		break;
	case KEY_PRNUP:
		strcpy(buf, "KEY: FEED");
		break;
	}
}

int KeyboardTest(void)
{	
	int iKey;
	uint8_t buf[20];
	
	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "TEST KEY,[ENTER]");
	lcdFlip();
	kbSetSlipFW(OFF);
	while(1)
	{	
		iKey = kbGetKey();
		if( iKey==KEY_CANCEL )
		{
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT, NULL, "TEST NOT OVER");
			DispMulLanguageString(0, 4, DISP_CFONT, NULL, "CANCEL-EXIST");
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "ENTER-CONTINUE");
			lcdFlip();
			if( kbGetKey()==KEY_CANCEL ) 
				return 0;
			lcdClrLine(2, 7);	
			DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PRESS KEY");
			lcdFlip();
			continue;
		}
		GetKeyDescription(iKey, (char *)buf);
		lcdClrLine(6, 7);
		lcdDisplay(0, 6, DISP_CFONT, (char *)buf);
		lcdFlip();
	}
}

int LcdTest(int flag)
{
	int i, x, y;
	INT32 iKey,iScreenwidth,iScreenheight;

	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "   LCD TEST     ");

	lcdClrLine(2, 7);
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "LCD TEST");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "LITTLE CHAR");
	lcdFlip();
	iKey = kbGetKeyMs(1000);
	if( iKey==KEY_CANCEL )	
		return 0;

	lcdClrLine(2, 7);
	lcdDisplay(0, 2, DISP_ASCII, "abcdefghijklmnopqrstuvwxyz!@#$%&*()_-+=[]{},;.<>|\\/?1234567890ABCDEFGHIJKLMNOPQRSTUVWXWXYZ");
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;

	lcdClrLine(2, 7);	
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "LCD TEST");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "BIG CHAR");
	lcdFlip();
	iKey = kbGetKeyMs(1000);
	if( iKey==KEY_CANCEL )	
		return 0;

	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT, "abcdefghijklmnopqrstuvwxyz!@#$%&*()_-+=[]{},;.<>");
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;
	
	lcdClrLine(2, 7);
	lcdDisplay(0, 2, DISP_CFONT, "|\\/?1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;
	
	
	lcdClrLine(2, 7);
	lcdLight(LIGHT_OFF_MODE);
	DispMulLanguageString(40, 4, DISP_CFONT, NULL, "CLOSE LIGHT");
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;
	
	lcdClrLine(2, 7);
	lcdLight(LIGHT_ON_MODE);
	DispMulLanguageString(40, 4, DISP_CFONT, NULL, "OPEN LIGHT");
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;

	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "LIGHT ALL ICON");
	for(i=1; i<10; i++)
	{
		if( i==2 ) 
			lcdSetIcon(i, INITSIGNAL+5);
		else  
			lcdSetIcon(i, OPENICON);
	}
	lcdSetIcon(1, CLOSEICON);
	lcdSetIcon(2, INITSIGNAL);
	sysDelayMs(300);
	lcdSetIcon(2, INITSIGNAL+1);
	sysDelayMs(300);
	lcdSetIcon(2, INITSIGNAL+2);
	sysDelayMs(300);
	lcdSetIcon(2, INITSIGNAL+3);
	sysDelayMs(300);
	lcdSetIcon(2, INITSIGNAL+4);
	sysDelayMs(300);
	lcdSetIcon(2, INITSIGNAL+5);
	sysDelayMs(300);
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;
	
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "  BACK SCREEN   ");
	lcdFlip();
	if (gstPosCapability.uiScreenType)
	{
		iScreenwidth = 320;
		iScreenheight = 240;
	}
	else
	{
		iScreenwidth = 128;
		iScreenheight = 64;
	}

	for(x=0; x<iScreenwidth; x++)
	  for(y=0; y<iScreenheight; y++)
		 lcdStipple(x, y, ON);
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "    LCD TEST    ");
	DispMulLanguageString(0, 4, DISP_CFONT, NULL, "CLOSE ALL ICON");
	for(i=1; i<10; i++) 
		lcdSetIcon(i, CLOSEICON);
    lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;
	
	DispMulLanguageString(0, 6, DISP_CFONT, NULL, "TEST END");
	lcdFlip();
	iKey = kbGetKeyMs(2000);
	if( iKey==KEY_CANCEL )	
		return 0;

	OkBeep();
	return 0;
}

void MagTest(int ShowFlag)
{
	uint8_t ucRet;
	uint8_t Track1[100], Track2[50], Track3[120];

	while(1)
	{
		lcdCls();	
		DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, NULL, "  MAGCARD TEST  ");
		DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS SWIPE<<<");
		lcdFlip();
		while(1)
		{
			if( magSwiped()==YES ) 
				break;
			if( kbhit()==YES )
			{
				if( kbGetKey()==KEY_CANCEL )
				{
					return;
				}
			}

			sysDelayMs(10); // Avoid CPU resources occupation
		}
		memset(Track1, 0, sizeof(Track1));
		memset(Track2, 0, sizeof(Track2));
		memset(Track3, 0, sizeof(Track3));
		ucRet = magRead(Track1, Track2, Track3);
		if( (ucRet&0x70)!=0x00 )
		{
			ErrorBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "READ ERROR!");
		}
		if( ucRet==0x06 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "2/3 TRACK OK");
		}
		if( ucRet==0x03 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "1/2 TRACK OK");
		}
		if( ucRet==0x01 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "1 TRACK OK");
		}
		if( ucRet==0x02 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "2 TRACK OK");
		}
		if( ucRet==0x04 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "3 TRACK OK");
		}
		if( ucRet==0x07 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "1/2/3 TRACK OK");
		}
		if( ucRet==0x05 )
		{
			OkBeep();
			DispMulLanguageString(0, 6, DISP_CFONT, NULL, "1/3 TRACK OK");
		}
		lcdFlip();
		kbGetKey();
		
		if( ShowFlag )
		{
			if( ucRet & 0x01 )
			{
				lcdClrLine(2, 7);
				lcdDisplay(0, 2, DISP_ASCII, (char *)Track1);
				lcdFlip();
				kbGetKey();
			}
			if( ucRet & 0x02 )
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 2, DISP_ASCII, (char *)Track2);
				lcdFlip();
				kbGetKey();
			}
			if( ucRet & 0x04 )
			{
				lcdClrLine(2, 7);	
				lcdDisplay(0, 2, DISP_ASCII, (char *)Track3);
				lcdFlip();
				kbGetKey();
			}
		}
	}
}

