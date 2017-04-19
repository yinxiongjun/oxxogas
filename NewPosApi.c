
#include "posapi.h"
#include "posappapi.h"
#include "glbvar.h"
#include "tranfunclist.h"
#include <ctype.h>

void apiConvAmount(uint8_t *ou, uint8_t *ascamt, uint8_t len)
{
	uint16_t	ii, jj, tt;
	uint8_t	ch;
	uint8_t	aa;
	uint8_t	buf[3];

	jj = 0;
	tt = 0;

	ou[jj++] = ' ';
	for(ii=0; ii<len; ii++)
	{
		ch = ascamt[ii];
		if( (tt==0) && (ch=='C') )
		{
			tt = 1;
		}
		else if( (tt==0) && (ch=='D') )
		{
			tt = 1;
			ou[jj++] = '-';
		}
		else if( !isdigit(ch) )
		{
			break;
		}
	}

	len = ii;
	aa = 0;
	for(ii=tt; (ii+3)<len; ii++)
	{
		ch = ascamt[ii];
		if( (ch=='0') && (aa==0) )
		{
			continue;
		}

		if( isdigit(ch) )
		{
			aa = 1;
			ou[jj++] = ch;
		}
	}

	tt = ii;
	len = len - ii;
	buf[0] = '0', buf[1] = '0', buf[2] = '0';
	for(ii=0; ii<len; ii++)
	{
		buf[3-len+ii] = ascamt[tt++];
	}

	ou[jj++] = buf[0];
	ou[jj++] = '.';
	ou[jj++] = buf[1];
	ou[jj++] = buf[2];
	ou[jj++] = '\0';
}

/****************************************************************************
 功能描述: 显示菜单
****************************************************************************/
int DispMenu(const char * menu[], int iLines, int iTitle, int iStartKey, int iEndKey, int iTimeOut)
{
	int screens;
	int start_line;
	int start_item;
	int line_per_screen,max_line_screen;
	int cur_screen;
	int i;
	int t;
	int key;
	char  szBuffer[100];

#ifdef CUP_UMS_VERSION
	iTimeOut = 60;		//银商所有菜单超时为60秒
#endif

	if (gstPosCapability.uiScreenType)
	{
		max_line_screen = 7;
	}
	else
	{
		max_line_screen = 4;
	}

	if( iTitle ) 
	{
		start_item = 1;
//		screens = (iLines-2)/ 3 + 1;
		screens = (iLines-2)/(max_line_screen-1) + 1;
		start_line = 2;
//		line_per_screen = 3;
		line_per_screen = max_line_screen-1;
		lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)menu[0]);
	}
	else 
	{
		start_item = 0;
//		screens = (iLines-1)/4 + 1;
		screens = (iLines-1)/max_line_screen + 1;
		start_line = 0;
//		line_per_screen = 4;
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

		lcdClrLine(start_line, 7);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		for(i=0; i<line_per_screen; i++) 
		{
			t = i + line_per_screen*cur_screen + start_item;
			if( t<iLines ) 
			{
				memset(szBuffer,0,sizeof(szBuffer));
				if (max_line_screen == 7)
				{
					AdjustMenuContext(menu[t],szBuffer);
					lcdDisplay(30, (i*2+3+start_line), DISP_CFONT, (char *)szBuffer);
				}
				else
				{
					lcdDisplay(30, (i*2+start_line), DISP_CFONT, (char *)menu[t]);
				}
			}				
		}
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		key = kbGetKeyMs(iTimeOut*1000);
//		if( key==KEY_CANCEL ) 
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
		//else 
			//sysBeep();
	}
}


/****************************************************************************
 功能描述: 显示菜单
****************************************************************************/
int DispMenu4(const char * menu[], int iLines, int iTitle, int iStartKey, int iEndKey, int iTimeOut)
{
	int screens;
	int start_line;
	int start_item;
	int line_per_screen,max_line_screen;
	int cur_screen;
	int i;
	int t;
	int key;
	char  szBuffer[100];

#ifdef CUP_UMS_VERSION
	iTimeOut = 60;		//银商所有菜单超时为60秒
#endif

	if (gstPosCapability.uiScreenType)
	{
		max_line_screen = 7;
	}
	else
	{
		max_line_screen = 4;
	}

	if( iTitle ) 
	{
		start_item = 1;
//		screens = (iLines-2)/ 3 + 1;
		screens = (iLines-2)/(max_line_screen-1) + 1;
		start_line = 7;
//		line_per_screen = 3;
		line_per_screen = max_line_screen-1;
		lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)menu[0]);
	}
	else 
	{
		start_item = 0;
//		screens = (iLines-1)/4 + 1;
		screens = (iLines-1)/max_line_screen + 1;
		start_line = 5;
//		line_per_screen = 4;
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

		lcdClrLine(2, 7);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		for(i=0; i<line_per_screen; i++) 
		{
			t = i + line_per_screen*cur_screen + start_item;
			if( t<iLines ) 
			{
				memset(szBuffer,0,sizeof(szBuffer));
				if (max_line_screen == 7)
				{
					AdjustMenuContext(menu[t],szBuffer);
					lcdDisplay(180, (i*2+3+start_line), DISP_CFONT, (char *)szBuffer);
				}
				else
				{
					lcdDisplay(180, (i*2+start_line), DISP_CFONT, (char *)menu[t]);
				}
			}				
		}
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		key = kbGetKeyMs(iTimeOut*1000);
//		if( key==KEY_CANCEL ) 
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
		//else 
			//sysBeep();
	}
}


/****************************************************************************
 功能描述: 显示菜单 (NETPAY 要求在右下部显示，增加此函数显示简短的)
****************************************************************************/
int DispMenu1(const char * menu[], int iLines, int iTitle, int iStartKey, int iEndKey, int iTimeOut)
{
	int screens;
	int start_line;
	int start_item;
	int line_per_screen,max_line_screen;
	int cur_screen;
	int i;
	int t;
	int key;
	char  szBuffer[100];

#ifdef CUP_UMS_VERSION
	iTimeOut = 60;		//银商所有菜单超时为60秒
#endif

	if (gstPosCapability.uiScreenType)
	{
		max_line_screen = 7;
	}
	else
	{
		max_line_screen = 4;
	}

	if( iTitle ) 
	{
		start_item = 1;
//		screens = (iLines-2)/ 3 + 1;
		screens = (iLines-2)/(max_line_screen-1) + 1;
		start_line = 2;
//		line_per_screen = 3;
		line_per_screen = max_line_screen-1;
		lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)menu[0]);
	}
	else 
	{
		start_item = 0;
//		screens = (iLines-1)/4 + 1;
		screens = (iLines-1)/max_line_screen + 1;
		start_line = 0;
//		line_per_screen = 4;
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

		lcdClrLine(start_line, 7);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		for(i=0; i<line_per_screen; i++) 
		{
			t = i + line_per_screen*cur_screen + start_item;
			if( t<iLines ) 
			{
				memset(szBuffer,0,sizeof(szBuffer));
				if (max_line_screen == 7)
				{
					AdjustMenuContext(menu[t],szBuffer);
					lcdDisplay(90, (i*2+3+start_line), DISP_CFONT, (char *)szBuffer);
				}
				else
				{
					lcdDisplay(80, (i*2+start_line), DISP_CFONT, (char *)menu[t]);
				}
			}				
		}
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		key = kbGetKeyMs(iTimeOut*1000);
//		if( key==KEY_CANCEL ) 
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
		
		//else 
			//sysBeep();
	}
}



/****************************************************************************
 功能描述: 显示菜单 (NETPAY 要求在右下部显示，增加此函数显示简短的)
****************************************************************************/
int DispMenu2(const char * menu[], int iLines, int iTitle, int iStartKey, int iEndKey, int iTimeOut)
{
	int screens;
	int start_line;
	int start_item;
	int line_per_screen,max_line_screen;
	int cur_screen;
	int i;
	int t;
	int key;
	char  szBuffer[100];

#ifdef CUP_UMS_VERSION
	iTimeOut = 60;		//银商所有菜单超时为60秒
#endif

	if (gstPosCapability.uiScreenType)
	{
		max_line_screen = 4;
	}
	else
	{
		max_line_screen = 4;
	}

	if( iTitle ) 
	{
		start_item = 1;
		screens = (iLines-2)/(max_line_screen-1) + 1;
		start_line = 3;
		line_per_screen = max_line_screen-1;
		lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)menu[0]);
	}
	else 
	{
		start_item = 0;
		screens = (iLines-1)/max_line_screen + 1;
		start_line = 3;
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

//		lcdClrLine(start_line, 9);
		lcdClrLine(2, 12);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		for(i=0; i<line_per_screen; i++) 
		{
			t = i + line_per_screen*cur_screen + start_item;
			if( t<iLines ) 
			{
				memset(szBuffer,0,sizeof(szBuffer));
				if (max_line_screen == 4)
				{
					AdjustMenuContext(menu[t],szBuffer);
					lcdDisplay(100, (i*2+3+start_line), DISP_CFONT, (char *)szBuffer);
				}
				else
				{
					lcdDisplay(100, (i*2+start_line), DISP_CFONT, (char *)menu[t]);
				}
			}				
		}
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		key = kbGetKeyMs(iTimeOut*1000);
//		if( key==KEY_CANCEL ) 
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
		
		//else 
			//sysBeep();
	}
}




/****************************************************************************
 功能描述: 显示菜单 (NETPAY 要求在右下部显示，增加此函数显示简短的)
****************************************************************************/
int DispMenu3(const char * menu[], int iLines, int iTitle, int iStartKey, int iEndKey, int iTimeOut)
{
	int screens;
	int start_line;
	int start_item;
	int line_per_screen,max_line_screen;
	int cur_screen;
	int i;
	int t;
	int key;
	char  szBuffer[100];

#ifdef CUP_UMS_VERSION
	iTimeOut = 60;		//银商所有菜单超时为60秒
#endif

	if (gstPosCapability.uiScreenType)
	{
		max_line_screen = 7;
	}
	else
	{
		max_line_screen = 4;
	}

	if( iTitle ) 
	{
		start_item = 1;
//		screens = (iLines-2)/ 3 + 1;
		screens = (iLines-2)/(max_line_screen-1) + 1;
		start_line = 2;
//		line_per_screen = 3;
		line_per_screen = max_line_screen-1;
		lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)menu[0]);
	}
	else 
	{
		start_item = 0;
//		screens = (iLines-1)/4 + 1;
		screens = (iLines-1)/max_line_screen + 1;
		start_line = 0;
//		line_per_screen = 4;
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

		lcdClrLine(start_line, 7);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		for(i=0; i<line_per_screen; i++) 
		{
			t = i + line_per_screen*cur_screen + start_item;
			if( t<iLines ) 
			{
				memset(szBuffer,0,sizeof(szBuffer));
				if (max_line_screen == 7)
				{
					AdjustMenuContext(menu[t],szBuffer);
					lcdDisplay(110, (i*2+6+start_line), DISP_CFONT, (char *)szBuffer);
				}
				else
				{
					lcdDisplay(80, (i*2+start_line), DISP_CFONT, (char *)menu[t]);
				}
			}				
		}
		lcdFlip();
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		key = kbGetKeyMs(iTimeOut*1000);
//		if( key==KEY_CANCEL ) 
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
		
		//else 
			//sysBeep();
	}
}


/*
int DispMenu(const char * menu[], int iLines, int iTitle, int iStartKey, int iEndKey, int iTimeOut)
{
	int screens;
	int start_line;
	int start_item;
	int line_per_screen,max_line_screen;
	int cur_screen;
	int i;
	int t;
	int key;
	char  szBuffer[100];

	if (gstPosCapability.uiScreenType)
	{
		max_line_screen = 6;
	}
	else
	{
		max_line_screen = 4;
	}

	if( iTitle ) 
	{
		start_item = 1;
		screens = (iLines-2)/(max_line_screen-1) + 1;
		start_line = 2;
		line_per_screen = max_line_screen-1;
		lcdDisplay(0, 0, DISP_CFONT|DISP_INVLINE|DISP_MEDIACY, (char *)menu[0]);
	}
	else 
	{
		start_item = 0;
		screens = (iLines-1)/max_line_screen + 1;
		start_line = 0;
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

		lcdClrLine(start_line-1, 7);
		for(i=0; i<line_per_screen; i++) 
		{
			t = i + line_per_screen*cur_screen + start_item;
			if( t<iLines ) 
			{
				memset(szBuffer,0,sizeof(szBuffer));
				if (max_line_screen == 6)
				{
					AdjustMenuContext(menu[t],szBuffer);
									
					lcdDisplay(0, (i*2+start_line), DISP_CFONT, (char *)szBuffer);
				}
				else
				{					
					lcdDisplay(0, (i*2+start_line), DISP_CFONT, (char *)menu[t]);
				}
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
	}
}
*/
uint8_t OptionsSelect(char * szTitle,	int iCondition, char* szTruePrmpt, char* szFalsePrmpt,	
					char* option1, char* option2, uint8_t ucTimeOut)
{
	int key;

	lcdCls();	
	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, szTitle);
	if( iCondition ) 
	{
		lcdDisplay(0, 2, DISP_CFONT, szTruePrmpt);
	}
	else 
	{
		lcdDisplay(0, 2, DISP_CFONT, szFalsePrmpt);
	}
	lcdDisplay(0, 4, DISP_CFONT, "%s", option1);
	lcdDisplay(0, 6, DISP_CFONT, "%s", option2);
	lcdFlip();

	do {
		key = kbGetKeyMs(ucTimeOut*1000);
	} while( key!=KEY1 && key!=KEY2 && key!=KEY_ENTER && key!=KEY_CANCEL && key!=KEY_INVALID );

	if( key==KEY_INVALID ) 
		key = KEY_CANCEL;
	
	return key;

}

void ErrorBeep(void)
{
	sysBeef(6, 200);
	sysDelayMs(200);
}

uint8_t OneTwoSelect(const char *szItem, const char *choice1, const char *choice2)
{
	int key;
	uint8_t buf[20];

	kbFlush();
	lcdClrLine(2, 7);	
	lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, (char *)szItem);
	sprintf((char *)buf, "[1].%s", choice1);
	lcdDisplay(0, 4, DISP_CFONT, (char *)buf);
	sprintf((char *)buf, "[2].%s", choice2);
	lcdDisplay(0, 6, DISP_CFONT, (char *)buf);
	lcdFlip();
	
	do {
		key = kbGetKey();
	} while( key!=KEY_ENTER && key!=KEY_CANCEL && key!=KEY1 && key!=KEY2 );
	
	return key;
}

void OkBeep(void)
{
	uint8_t i;
	
	for(i=0; i<3; i++)
	{
		sysBeef(3, 100);
		sysDelayMs(100);
	}
}

void FailBeep(void)
{
	sysBeef(6, 700);
}

static uint8_t abcd_to_asc(uint8_t ucBcd)
{
	uint8_t ucAsc;
	
	ucBcd &= 0x0f;
	if( ucBcd<=9 )
		ucAsc = ucBcd + '0';
	else
		ucAsc = ucBcd + 'A' - 10;
	return (ucAsc);
}

void BcdToAsc(uint8_t * sAscBuf, uint8_t * sBcdBuf, int iAscLen)
{
	int i, j;

	j = 0;
	for(i=0; i<iAscLen/2; i++) 
	{
		sAscBuf[j] = (sBcdBuf[i] & 0xf0) >> 4;
		sAscBuf[j] = abcd_to_asc(sAscBuf[j]);
		j++;
		sAscBuf[j] = sBcdBuf[i] & 0x0f;
		sAscBuf[j] = abcd_to_asc(sAscBuf[j]);
		j++;
	}
	if( iAscLen%2 ) 
	{
		sAscBuf[j] = (sBcdBuf[i] & 0xf0) >> 4;
		sAscBuf[j] = abcd_to_asc(sAscBuf[j]);
	}
}

void BcdToAsc0(uint8_t * sAscBuf, uint8_t * sBcdBuf, int iAscLen)
{
	BcdToAsc(sAscBuf, sBcdBuf, iAscLen);
	sAscBuf[iAscLen] = 0;
}

static uint8_t aasc_to_bcd(uint8_t ucAsc)
{
	uint8_t ucBcd;

	if( (ucAsc>='0') && (ucAsc<='9') )
		ucBcd = ucAsc - '0';
	else if( (ucAsc>='A') && (ucAsc<='F') )
		ucBcd = ucAsc - 'A' + 10;
	else if( (ucAsc>='a') && (ucAsc<='f') )
		ucBcd = ucAsc - 'a' + 10;
	else if( (ucAsc>0x39) && (ucAsc<=0x3f) )
		ucBcd = ucAsc - '0';
	else 
 		ucBcd = 0x0f;
	
	return ucBcd;
}

void AscToBcd(uint8_t * sBcdBuf, uint8_t * sAscBuf, int iAscLen)
{
	int   i, j;

	j = 0;

	for(i=0; i<(iAscLen+1)/2; i++) 
	{
		sBcdBuf[i] = aasc_to_bcd(sAscBuf[j++]) << 4;
		sBcdBuf[i] |= (j >= iAscLen) ? 0x00 : aasc_to_bcd(sAscBuf[j++]);
	}
}

static uint8_t bcd_to_byte(uint8_t ucBcd)
{
	return (((ucBcd >> 4) & 0x0f) * 10 + (ucBcd & 0x0f));
}


int BcdToLong(uint8_t  *sBcdBuf, int iBcdLen)
{
	int 	lValue = 0;

	if( iBcdLen<=0 )
		return 0;
		
	while( iBcdLen-- > 0)
		lValue = lValue * 100 + bcd_to_byte(*sBcdBuf++);
	return lValue;
}

int  LongToAmount(uint8_t *szAmount, int lAmount)
{
	sprintf((char *)szAmount, "%d.%02d", lAmount/100, lAmount%100);
	
	return (strlen((char *)szAmount));
}

uint8_t LongToDec(uint8_t * szAmount)
{
	uint8_t tmp[16];
	uint8_t len1;

	memcpy(tmp, "0.00", 4);
	tmp[4] = 0;

	len1 = strlen((char*)szAmount);
	if( len1==0 ) 
	{
		memcpy(szAmount, tmp, 4);
		szAmount[4] = 0;
		return (4);
	}
	if( len1==1 ) 
	{
		tmp[3] = szAmount[0];
		memcpy(szAmount, tmp, 4);
		szAmount[4] = 0;
		return (4);
	}
	if( len1==2 ) 
	{
		tmp[2] = szAmount[0];
		tmp[3] = szAmount[1];
		memcpy(szAmount, tmp, 4);
		szAmount[4] = 0;
		return (4);
	}
	memcpy(tmp, szAmount, len1 - 2);
	tmp[len1-2] = '.';
	memcpy(tmp+len1-1, szAmount+len1-2, 2);
	tmp[len1+1] = 0;
	memcpy(szAmount, tmp, len1+1);
	szAmount[len1+1] = 0;
	return (len1+1);
}

int CheckDate(char *sDate)
{
	int  	y, m, d;
	char 	chLeapFlag;
	char 	szYear[5];
	char 	szMonth[3];
	char 	szDay[3];
	int  	i;
	
	for(i=0; i<8; i++) 
	{
		if( sDate[i]<'0' || sDate[i]>'9' )
			return (1);
	}
	
	sprintf(szYear, "%.4s", sDate);
	sprintf(szMonth, "%.2s", sDate+4);
	sprintf(szDay, "%.2s", sDate+6);
	
	y = atoi(szYear);
	m = atoi(szMonth);
	d = atoi(szDay);

	if( m>12 || m<=0 )
		return (1);
	if( d>31 || d<=0 )
		return (1);
	if( (m==4 || m==6 || m==9 || m==11) && d==31 )
		return (1);
	
	if( m==2 ) 
	{
		chLeapFlag = 0;
		if( y%400==0 )
			chLeapFlag = 1;
		else if( y%4==0 && y%100 != 0)
			chLeapFlag = 1;
		if( chLeapFlag!=1 && d>28 )
			return (1);
		if( chLeapFlag==1 && d>29 )
			return (1);
	}
	
	return 0;
}

int CheckTime(char *sTime)
{
	int   h, m, s, i;
	char  szBuf[3];

	for(i=0; i<6; i++) 
	{
		if( sTime[i]<'0' || sTime[i]>'9' )
			return (1);
	}

	sprintf(szBuf, "%.2s", sTime);
	h = atoi(szBuf);
	if( h<0 || h>23 )
		return (1);
		
	sprintf(szBuf, "%.2s", sTime+2);
	m = atoi(szBuf);
	if( m<0 || m>59 )
		return (1);
		
	sprintf(szBuf, "%.2s", sTime+4);
	s = atoi(szBuf);
	if( s<0 || s>59 )
		return (1);
		
	return 0;
}

int CheckYYMM(char *sYYMM)
{
	int     m;
	int     i;
	char    szMonth[3];
	
	for(i=0; i<4; i++) 
	{
		if( sYYMM[i]<'0' || sYYMM[i]>'9' )
			return (1);
	}

	sprintf(szMonth, "%.2s", sYYMM+2);
	m = atoi(szMonth);

	if( m>12 || m<=0 )
		return (1);
		
	return 0;
}


uint16_t ByteToShort(uint8_t *buf)
{
	return (uint16_t)((buf[0]<<8) | buf[1]);
}


void ShortToByte(uint8_t *buf, uint16_t shortvar)
{
	buf[0] = (uint8_t)(shortvar>>8);
	buf[1] = (uint8_t)(shortvar & 0xFF);
}

uint16_t dat_asclen(uint8_t *ptr, uint16_t len)
{
	uint16_t rlen;
	for(rlen=0; rlen<len; rlen++)
	{
		if( !isdigit(ptr[rlen]) )
		{
			break;
		}
	}
	return rlen;
}

uint8_t BcdAdd(uint8_t *augend, uint8_t *addend, uint16_t len) 
{
	uint8_t carry,tmp_result;
	carry = 0x66;
	while( len-->0 ) 
	{
		tmp_result = augend[len] + addend[len]+ carry;
		carry = 0x67;
		if( DataLow4Bit(tmp_result)>DataLow4Bit(augend[len]) )
		{
			tmp_result -= 0x06;
		}
		if( DataHigh4Bit(tmp_result)>DataHigh4Bit(augend[len]) ) 
		{
			tmp_result -= 0x60;
			carry = 0x66;
     	}
     	augend[len] = tmp_result;
	}
   	return (carry-0x66);
}	// BcdAdd

uint8_t ConvBcdAmount(uint8_t *bcdAmt, uint8_t *amount_ptr)
{
	uint8_t  buffer[16], amtLen;
	int      i;
	char AmountTemp[20] = {0};

	memset(buffer, 0, 16);
	BcdToAsc(buffer, bcdAmt, 12);
	buffer[12] = 0x00;
	for(i=0; i<12; i++)
	{
		if( buffer[i]!='0' )
		{
			amtLen = LongToDec(buffer+i);
			strcpy((char *)AmountTemp, (char *)buffer+i);
			

			if( amtLen <= 6 )
			{
				sprintf((char*)amount_ptr,"%s",AmountTemp);
			}
			else if( amtLen == 7 )
			{
				sprintf((char*)amount_ptr,"%.1s,%.6s",(char*)AmountTemp,(char*)AmountTemp+1);
			}
			else if( amtLen == 8 )
			{
				sprintf((char*)amount_ptr,"%.2s,%.6s",(char*)AmountTemp,(char*)AmountTemp+2);
			}
			else if( amtLen == 9 )
			{
				sprintf((char*)amount_ptr,"%.3s,%.6s",(char*)AmountTemp,(char*)AmountTemp+3);
			}
			else if( amtLen == 10 )
			{
				sprintf((char*)amount_ptr,"%.1s,%.3s,%.6s",(char*)AmountTemp,(char*)AmountTemp+1,(char*)AmountTemp+4);
			}
			else if( amtLen == 11 )
			{
				sprintf((char*)amount_ptr,"%.2s,%.3s,%.6s",(char*)AmountTemp,(char*)AmountTemp+2,(char*)AmountTemp+5);
			}
			return amtLen;
		}
	}
	strcpy((char *)amount_ptr, "0.00");
	return 4;
}

uint32_t AscToLong(uint8_t *sAsc, uint8_t ucLen)
{
    uint32_t result;
	uint8_t  ii;
    result = 0;
	
	ucLen = (uint8_t)(dat_asclen(sAsc, ucLen));
	
	for(ii=0;ii<ucLen;ii++) 
   	{
		result = result*10 + DataLow4Bit(sAsc[ii]);
   	}
	return result;
}	// AscToLong


uint8_t DataHigh4Bit(uint8_t inchar) 
{
	return (inchar/16);
}

uint8_t DataLow4Bit(uint8_t inchar) 
{
	return (inchar & 0x0f);
}



