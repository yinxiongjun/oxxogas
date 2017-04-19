#include "PosLib.h"
#include "posapi.h"
#include "MathLib.h"
#include <ctype.h>
#include "Display.h"
#include "posmacro.h"
#include "glbvar.h"
#include "tranfunclist.h"

/********************** Internal macros declaration ************************/
#ifdef _POS_DEBUG
	#define LEN_DBGDATA         1024   
	#define MAX_CHARS           5       
	#define CHARS_PERLINE       21     
	#define MAX_LOG_MSG         (((LEN_DBGDATA+MAX_CHARS-1)/MAX_CHARS)*CHARS_PERLINE)
#endif /* _POS_DEBUG */

#define LEN_GETSTRING		512	// for PubGetString()

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
#ifdef _POS_DEBUG
static void GetBaseName(uint8_t *pszFullPath, uint8_t *pszBaseName);
#endif

static void MapChar2Index(uint8_t ch, int *piRow, int *piCol);
static void ShowWorkBuf(uint8_t ucMode, uint8_t *pszWorkBuff, int iLen);

/********************** Internal variables declaration *********************/
#ifdef _POS_DEBUG
static uint8_t glszDbgInfo[MAX_LOG_MSG+128];
#endif

static uint8_t sgMinLineNo;
static char *pszAlphaTbl[10] =
{
	"0.,*# ~`!@$%^&-+=(){}[]<>_|\\:;\"\'?/",
	"1QZqz ", "2ABCabc", "3DEFdef",
	"4GHIghi", "5JKLjkl", "6MNOmno",
	"7PRSprs", "8TUVtuv", "9WXYwxy",
};

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
/*********************************************************************************************/
/*********************************************************************************************/
void PubBeepOk(void)
{
	int	  iCnt;

	for (iCnt=0; iCnt<3; iCnt++)
	{
		sysBeef(6, 60);
		sysDelayMs(80);
	}
}

void PubBeepErr(void)
{
	sysBeef(1, 200);
	sysDelayMs(200);
}

void PubLongBeep(void)
{
	sysBeef(6, 800);
}

int PubChkKeyInput(void)
{
	if (kbhit()==YES)
	{
		return TRUE;
	}

	return FALSE;
}

int PubWaitKey(ulong ulWaitTime)
{
	int   iKey;

	kbFlush();
	if (ulWaitTime>0)
	{
		iKey = kbGetKeyMs((int)(ulWaitTime*1000));
	}
	else
	{
		iKey = kbGetKey();
	}
		
	return iKey;
}

uint8_t PubYesNo(ulong ulWaitTime)
{
	int   iKeyValue;
	
	while (1)
	{
		iKeyValue = PubWaitKey(ulWaitTime);
		if (iKeyValue==KEY_ENTER)
		{
			return  TRUE;
		}
		if (iKeyValue==KEY_CANCEL || iKeyValue==KEY_TIMEOUT)
		{
			return  FALSE;
		}
	}
}

void PubHalt(uint8_t *pszfile, uint32_t uiLine)
{
	int iMode,iErr;

	lcdCls();
	sysGetLastError(&iMode,&iErr);

	lcdDisplay(0, 0, DISP_ASCII, "File:%s,\nLine:%d,\nRet:%d", (char *)pszfile, uiLine, iErr);
	lcdDisplay(0, 4, DISP_CFONT, "PLS RECORD");
	lcdDisplay(0, 4, DISP_CFONT, "THEN RESTART POS");
	lcdFlip();

	while (1)
	{
	};
}

void PubDisplayTitle(int IfClrScr, char *pszTitle)
{
	int ilen,iBegindex;
	char  buffer[16+1];

	PubASSERT(pszTitle!=NULL);
	if (pszTitle==NULL)
	{
		return;
	}

	if (IfClrScr)
	{
		lcdCls();
	}

	memset(buffer,0,sizeof(buffer));
	ilen = strlen(pszTitle);
	if (ilen>=16)
	{
		ilen = 16;
	}

	iBegindex = (16-ilen)/2;
	memset(buffer,' ',sizeof(buffer)-1);
	memcpy(buffer+iBegindex,pszTitle,ilen);

	lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,"%-16.16s", buffer);

	return;
}

void PubDisplayMsg(uint8_t ucLine, uint8_t *pszMsg)
{
	int		iLen;
	uint8_t	ucMode;

	PubASSERT(pszMsg!=NULL);
	if (pszMsg==NULL)
	{
		return;
	}

	ucMode =  (ucLine & DISP_INVLINE) | DISP_CFONT;
	ucLine &= ~DISP_INVLINE;
	lcdClrLine(ucLine, (uint8_t)(ucLine+1));

	iLen = strlen((char *)pszMsg);
	if (iLen > NUM_MAXZHCHARS)
	{
		iLen = NUM_MAXZHCHARS;
	}

	lcdDisplay((uint8_t)((NUM_MAXCOLS-8*iLen)/2), ucLine, ucMode,
			"%.*s", iLen, (char *)pszMsg);

	return;
}


void PubShowTwoMsg(uint8_t ucLine, uint8_t *pszMsg1, uint8_t *pszMsg2)
{
	if (ucLine>4)
	{
		ucLine = 4;
	}
	PubDisplayMsg(ucLine, pszMsg1);
	PubDisplayMsg((uint8_t)(ucLine+2), pszMsg2);
}

void PubDispString(void *pszStr, uint8_t ucPosition)
{
	uint8_t	mode, x, y, ucLen, buffer[60];

	mode  = ucPosition & 0xf0;
	y     = ucPosition & 0x0f;
	ucLen = strlen(pszStr);
	if (ucLen > NUM_MAXZHCHARS)
	{
		ucLen = NUM_MAXZHCHARS;
	}

	switch(mode)
	{
		case DISP_SCR_CENTER:
			lcdCls();
			x = (64 - ucLen * 4);
			y = 3;
			break;

		case DISP_LINE_CENTER:
			lcdClrLine(y, (uint8_t) (y + 1));	
			x = (64 - ucLen * 4);
			break;

		case DISP_LINE_RIGHT:
			x = (128 - ucLen * 8);
			break;

		case DISP_LINE_LEFT:
		default:
			x = 0;
			break;
	}

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, pszStr, ucLen);
	lcdClrLine(y, (uint8_t)(y+1));
	lcdDisplay(x, y, DISP_CFONT, "%s", (char *)buffer);

	return;
}



uint8_t PubConvAmount(uint8_t *pszPrefix, uint8_t *pszIn, uint8_t ucDeciPos, uint8_t *pszOut, uint8_t ucMisc)
{
	int		ii, iInLen, iIntegerLen;
	uint8_t	bSeparator, bNegative, szBuff[40+1], szTemp[40];
	uint8_t	*pRead, *pWr;

	PubASSERT(pszIn!=NULL && pszOut!=NULL && ucDeciPos<4);

	sprintf((char *)szBuff, "%.40s", pszIn);
	PubTrimHeadChars((char *)szBuff, '0');
	iInLen = strlen((char *)szBuff);
	pRead  = szBuff;
	pWr = pszOut;

	bSeparator = ucMisc & GA_SEPARATOR;		//whether use ',' to separate huge amount
	bNegative  = ucMisc & GA_NEGATIVE;		//whether display negative amount

	if (pszPrefix!=NULL && *pszPrefix!=0)
	{
		pWr += sprintf((char *)pWr, "%.3s ", pszPrefix);
	}
	if (bNegative)
	{
		*(pWr++) = '-';
	}

	if (iInLen>ucDeciPos)
	{
		iIntegerLen = iInLen - ucDeciPos;
		ii = iIntegerLen;
		while (ii--)
		{
			*(pWr++) = *(pRead++);
			if (bSeparator && (ii%3==0) && ii)
			{
				*(pWr++) = ',';
			}
		}
	}
	else
	{
		*(pWr++) = '0';
	}

	if (ucDeciPos!=0)
	{
		sprintf((char *)szTemp, "%.*s", ucDeciPos, pRead);
		PubAddHeadChars((char *)szTemp, ucDeciPos, '0');
		sprintf((char *)pWr, ".%s", szTemp);
		pWr += (ucDeciPos+1);
	}
	else
	{
		*(pWr++) = 0;
	}

	return 0;
}


// display menu
int PubGetMenu(uint8_t *pszPrompt, MenuItem *pstMenu, uint8_t ucMode, uint32_t nTimeOut)
{
	static uint8_t *pszDefPrompt = (uint8_t *)"PLS INPUT:";
	int     iKey,iLen, IsChnString, ucDispPosition,line;
	uint8_t   ucMaxLen, ucLen, ucCol, ucKeepChars, ucMenuFont, ucRever;
	uint8_t   ucStartLineNo, ucLineNo, ucMenuLines, ucEndLineNo;
	uint8_t   ucMenuNum, ucCurItem, ucPageNum, ucPageNo;
	uint8_t	bOneKeySel, bAutoSno, szInput[2+1], ucInputCnt, ucMenuCnt;
	uint8_t	ucSelNo, bBottomPrompt, bUseSno, ucCnt, ucShowItemNum;
	uint8_t	szMenuNo[2+1], bMenuOk, ucActNum;
	char  *pDotSign;
	short	iCnt;

	PubASSERT( pstMenu!=NULL );
	kbFlush();

	if (gstPosCapability.uiScreenType)
	{
		ucDispPosition = 260;
		line = 7;
	}
	else
	{
		ucDispPosition = 112;
		line = 6;
	}

	IsChnString =  FALSE;
	ucCurItem = 0;
	// Analyze menu content
	if( pstMenu==NULL )
	{
		return -1;
	}
	bUseSno = (ucMode & MENU_USESNO);
	for(ucShowItemNum=ucMaxLen=ucMenuNum=0; pstMenu[ucMenuNum].szMenuName[0]; ucMenuNum++)
	{
		ucLen	 = strlen((char *)pstMenu[ucMenuNum].szMenuName);

		if (!IsChnString)
		{
			IsChnString = PubCheckCHNString((char *)pstMenu[ucMenuNum].szMenuName,ucLen);
		}

		ucMaxLen = MAX(ucLen, ucMaxLen);
		if( pstMenu[ucMenuNum].bVisible )
		{
			ucShowItemNum++;
		}
		if( !bUseSno )
		{
			continue;
		}
		pDotSign = strchr(pstMenu[ucMenuNum].szMenuName, '.');
		if( pDotSign==NULL )
		{
			return -2;
		}
		iLen = pDotSign - pstMenu[ucMenuNum].szMenuName;
		if( iLen ==0  || iLen > 2  )
		{
			return -2;
		}
	}
	if( ucMenuNum==0 || ucShowItemNum==0 )
	{
		return -3;
	}

	// process options
	ucMenuNum     = MIN(ucMenuNum, (uint8_t)99);
	bBottomPrompt = bUseSno  ? TRUE  : (ucMode & MENU_PROMPT);
	bAutoSno      = bUseSno  ? FALSE : (ucMode & MENU_AUTOSNO);
	bOneKeySel    = bAutoSno ? (ucMenuNum<=9) : FALSE;
	ucKeepChars   = 0;
	if( ucMode & MENU_AUTOSNO )
	{
		ucKeepChars = (ucMenuNum<=9) ? 2 : 3;
	}

	// analyze menu font
	ucMenuFont = (ucMaxLen+ucKeepChars>NUM_MAXZHCHARS) ? DISP_ASCII: DISP_CFONT;
	if( (ucMode & MENU_CFONT) || (ucMode & MENU_ASCII) )
	{
		ucMenuFont = (ucMode & MENU_CFONT) ? DISP_CFONT : DISP_ASCII;
	}
	if (IsChnString)
	{
		ucMenuFont = DISP_CFONT;
	}

	// calculate max. chars per lines and the start column
	if( ucMenuFont== DISP_ASCII )
	{
		ucMaxLen = MIN(ucMaxLen, (uint8_t)(NUM_MAXCHARS-ucKeepChars));
		ucCol    = (ucMode & MENU_CENTER) ? (6*(NUM_MAXCHARS-ucKeepChars-ucMaxLen)/2)   : 0;
	}
	else
	{
		ucMaxLen = MIN(ucMaxLen, (uint8_t)(NUM_MAXZHCHARS-ucKeepChars));
		ucCol    = (ucMode & MENU_CENTER) ? (8*(NUM_MAXZHCHARS-ucKeepChars-ucMaxLen)/2) : 0;
	}

	// analyze menu start/end lines
	ucStartLineNo = 0;
	if( pszPrompt==NULL || *pszPrompt==0 )
	{
		if( bUseSno )
		{
			pszPrompt   = pszDefPrompt;
			ucMenuLines = (ucMenuFont==DISP_ASCII) ? NUM_MAXLINES-2 : NUM_MAXZHLINES-1;
		}
		else if( bAutoSno )
		{
			if( ucMenuFont==DISP_ASCII )
			{
				ucMenuLines = bOneKeySel ? NUM_MAXLINES : NUM_MAXLINES-2;
			}
			else
			{
				ucMenuLines = bOneKeySel ? NUM_MAXZHLINES : NUM_MAXZHLINES-1;
			}
			pszPrompt = bOneKeySel ? NULL : pszDefPrompt;
		}
		else
		{
			ucMenuLines = (ucMenuFont==DISP_ASCII) ? NUM_MAXLINES : NUM_MAXZHLINES;
			pszPrompt   = NULL;
		}
		if( pszPrompt!=NULL )
		{
			ucStartLineNo = bBottomPrompt ? 0 : 2;
		}
	}
	else
	{
		if( !bBottomPrompt )
		{
			ucStartLineNo = (bOneKeySel || !bAutoSno) ? 2 : 0;
		}
		ucMenuLines = (ucMenuFont==DISP_ASCII) ? NUM_MAXLINES-2 : NUM_MAXZHLINES-1;
	}
	ucEndLineNo = (ucMenuFont==DISP_ASCII) ? (ucStartLineNo+ucMenuLines-1) : (ucStartLineNo+ucMenuLines*2-1);
	ucPageNum   = (ucShowItemNum+ucMenuLines-1)/ucMenuLines;

	// show title/prompt message
	if( ucStartLineNo>0 )
	{
		PubDisplayTitle(TRUE, (char *)pszPrompt);
	}
	else if( pszPrompt!=NULL )
	{	
		lcdDisplay(0,line,DISP_CFONT|DISP_CLRLINE,(char *)pszPrompt);
	}

	for(ucMenuCnt=0; ucMenuCnt<ucMenuNum; ucMenuCnt++)
	{
		if( pstMenu[ucMenuCnt].bVisible )
		{
			break;
		}
	}
	ucSelNo = ucMenuCnt;
	// Do menu loops
	ucPageNo = ucInputCnt = 0;
	memset(szInput, 0, sizeof(szInput));
	while( 1 )
	{
		// Light/Close up & down icon
		if( bUseSno || bAutoSno )
		{
			lcdSetIcon(ICON_UP,   (UINT8)(ucPageNo>0		   ? OPENICON : CLOSEICON));
			lcdSetIcon(ICON_DOWN, (UINT8)(ucPageNo<ucPageNum-1 ? OPENICON : CLOSEICON));
		}
		else
		{
			lcdSetIcon(ICON_UP,   (UINT8)(ucSelNo>0		      ? OPENICON : CLOSEICON));
			lcdSetIcon(ICON_DOWN, (UINT8)(ucSelNo<ucMenuNum-1 ? OPENICON : CLOSEICON));
		}

		if( ucStartLineNo==0 && pszPrompt!=NULL )
		{
			lcdDisplay(ucDispPosition, line, DISP_CFONT, "%2s", szInput);
		}

		for(ucActNum=ucMenuCnt=0; ucMenuCnt<ucMenuNum; ucMenuCnt++)
		{
			if( !pstMenu[ucMenuCnt].bVisible )
			{
				continue;
			}
			if( ucActNum>=ucPageNo*ucMenuLines )
			{
				ucCurItem = ucMenuCnt;
				break;
			}
			ucActNum++;
		}
		// Display menu
		lcdClrLine(ucStartLineNo, ucEndLineNo+1);
		for(ucMenuCnt=0,ucLineNo=ucStartLineNo; ucMenuCnt<ucMenuLines;)
		{
			if( pstMenu[ucCurItem].bVisible )
			{
				if( bAutoSno )
				{
					lcdDisplay(ucCol, ucLineNo, ucMenuFont, "%*d.%.*s",
							(int)(ucKeepChars>0 ? ucKeepChars-1 : 0),
							(int)(ucCurItem+1), ucMaxLen, pstMenu[ucCurItem].szMenuName);
				}
				else
				{
					ucRever = 0;
					if( !bUseSno )
					{
						ucRever = (ucCurItem==ucSelNo) ? DISP_INVLINE: 0;
					}
					lcdDisplay(ucCol, ucLineNo, (UINT8)(ucMenuFont|ucRever), "%.*s",
							ucMaxLen, pstMenu[ucCurItem].szMenuName);
				}
				ucLineNo += ((ucMenuFont==DISP_ASCII) ? 1 : 2);
				ucMenuCnt++;
			}
			ucCurItem++;
			if( ucCurItem>=ucMenuNum )
			{
				break;
			}
		}

		lcdFlip();
		iKey = PubWaitKey(nTimeOut);		// Waiting for user input
		lcdSetIcon(ICON_UP,   CLOSEICON);
		lcdSetIcon(ICON_DOWN, CLOSEICON);
		if( iKey==KEY_CANCEL || iKey==KEY_INVALID || iKey == KEY_TIMEOUT)
		{
			return -4;
		}
		else if( iKey==KEY_UP )
		{
			if( bUseSno || bAutoSno )
			{
				ucPageNo = (ucPageNo==0) ? (ucPageNum-1) : (ucPageNo-1);
			}
			else
			{
				ucSelNo  = (ucSelNo>0) ? ucSelNo-1 : ucMenuNum-1;
				for(iCnt=(int)ucSelNo; iCnt>=0; iCnt--)
				{
					if( pstMenu[iCnt].bVisible )
					{
						break;
					}
				}
				if( iCnt<0 )
				{
					for(iCnt=(int)(ucMenuNum-1); iCnt>=0; iCnt--)
					{
						if( pstMenu[iCnt].bVisible )
						{
							break;
						}
					}
				}
				ucSelNo = (uint8_t)iCnt;
				for(ucActNum=ucMenuCnt=0; ucMenuCnt<ucSelNo; ucMenuCnt++)
				{
					if( pstMenu[ucMenuCnt].bVisible )
					{
						ucActNum++;
					}
				}
				ucPageNo = ucActNum/ucMenuLines;
			}
		}
		else if( iKey==KEY_DOWN )
		{
			if( bUseSno || bAutoSno )
			{
				ucPageNo = (ucPageNo+1>=ucPageNum) ? 0 : ucPageNo+1;
			}
			else
			{
				ucSelNo  = (ucSelNo+1<ucMenuNum) ? ucSelNo+1 : 0;
				for(ucMenuCnt=ucSelNo; ucMenuCnt<ucMenuNum; ucMenuCnt++)
				{
					if( pstMenu[ucMenuCnt].bVisible )
					{
						break;
					}
				}
				if( ucMenuCnt>=ucMenuNum )
				{
					for(ucMenuCnt=0; ucMenuCnt<ucMenuNum; ucMenuCnt++)
					{
						if( pstMenu[ucMenuCnt].bVisible )
						{
							break;
						}
					}
				}
				ucSelNo = ucMenuCnt;
				for(ucActNum=ucMenuCnt=0; ucMenuCnt<ucSelNo; ucMenuCnt++)
				{
					if( pstMenu[ucMenuCnt].bVisible )
					{
						ucActNum++;
					}
				}
				ucPageNo = ucActNum/ucMenuLines;
			}
		}
		else if( (bUseSno || bAutoSno) && iKey>=KEY0 && iKey<=KEY9 )
		{
			if( bUseSno )
			{
				if( ucInputCnt<2 )
				{
					szInput[ucInputCnt++] = iKey;
					szInput[ucInputCnt]   = 0;
				}
			}
			else if( bOneKeySel )
			{
				if( iKey>=KEY1 && iKey<ucMenuNum+KEY1 )
				{
					ucSelNo = (uint8_t)(iKey - KEY1);
					break;
				}
			}
			else if( ucInputCnt<(uint8_t)(ucKeepChars-1) )
			{
				szInput[ucInputCnt++] = iKey;
				szInput[ucInputCnt]   = 0;
				ucSelNo = (uint8_t)atoi((char *)szInput);
				if( ucSelNo>ucMenuNum )
				{
					szInput[--ucInputCnt] = 0;
				}
			}
		}
		else if( (bUseSno || bAutoSno) && iKey==KEY_CLEAR )
		{
			szInput[0] = 0;
			ucInputCnt = 0;
		}
		else if( (bUseSno || bAutoSno) && iKey==KEY_BACKSPACE )
		{
			if( ucInputCnt>0 )
			{
				szInput[--ucInputCnt] = 0;
			}
		}
		else if( iKey==KEY_ENTER )
		{
			if( !bAutoSno && !bUseSno )
			{
				break;
			}

			if( bOneKeySel || ucInputCnt==0 )
			{
				ucPageNo = (ucPageNo>=ucPageNum-1) ? 0 : ucPageNo+1;
			}
			else if( ucInputCnt>0 )
			{
				ucSelNo = (uint8_t)atoi((char *)szInput);
				if( bUseSno )
				{
					for(bMenuOk=FALSE,ucCnt=0; ucCnt<ucMenuNum; ucCnt++)
					{
						pDotSign = strchr((char *)pstMenu[ucCnt].szMenuName, '.');
						PubASSERT( pDotSign!=NULL );
						sprintf((char *)szMenuNo, "%.*s",
								(int)MIN(2, pDotSign-pstMenu[ucCnt].szMenuName),
								pstMenu[ucCnt].szMenuName);
						if( atoi((char *)szMenuNo)==(int)ucSelNo )
						{
							ucSelNo = ucCnt;
							bMenuOk = TRUE;
							break;
						}
					}
					if( bMenuOk )
					{
						break;
					}
					szInput[0] = 0;
					ucInputCnt = 0;
				}
				else
				{
					if( ucSelNo>0 && ucSelNo<=ucMenuNum )
					{
						ucSelNo--;
						break;
					}
				}
			}
		}
	}	// end of while( 1

	if( pstMenu[ucSelNo].pfMenuFunc!=NULL )
	{
		(*pstMenu[ucSelNo].pfMenuFunc)();
	}

	return (int)ucSelNo;
}

//GA_SEPARATOR:	0x80
uint8_t PubGetAmount(uint8_t *pszPrefix, uint8_t ucDeciPos,
				  uint8_t ucMinLen, uint8_t ucMaxLen, uint8_t *pszData,
				  uint8_t ucTimeout, uint8_t ucMisc)
{
	uint8_t	*pszNowChar, bSeparator, bNegative;
	uint8_t	szBuff[20], szDispBuf[20], szDispBuf2[30];
	int		iKeyVal , iTotalLen;
	uint32_t  uiOldTime, uiNewTime;

	if (ucDeciPos>5 ||
		ucMinLen>ucMaxLen || ucMaxLen>12 ||
		pszData==NULL)
	{
		return 0xFE;		//parameter illegal
	}

	memset(szBuff, 0, sizeof(szBuff));
	memset(szDispBuf, 0, sizeof(szDispBuf));
	memset(szDispBuf2, 0, sizeof(szDispBuf2));

	bSeparator = ucMisc & GA_SEPARATOR;		//whether use ',' to separate huge amount
	bNegative  = ucMisc & GA_NEGATIVE;		//whether display negative amount
	pszNowChar = szBuff;

	while (1)
	{
		memset(szDispBuf2, 0, sizeof(szDispBuf2));
		if (pszPrefix!=NULL && strlen((char *)pszPrefix)>0)
		{
			sprintf((char *)szDispBuf2, "%.4s", (char *)pszPrefix);
		}

		PubConvAmount(NULL, szBuff, ucDeciPos, szDispBuf, ucMisc);

		lcdClrLine(6, 7);
		iTotalLen = strlen((char *)szDispBuf) + strlen((char *)szDispBuf2);
		if (iTotalLen < 17)
		{
			strcat((char *)szDispBuf2, (char *)szDispBuf);
		}

		uiOldTime = sysGetTimerCount();
		while(1)
		{
			if (iTotalLen < 17)
			{
				lcdDisplay(0, 6, DISP_CFONT, "%16.16s", szDispBuf2);
			}
			else
			{
				lcdDisplay(0, 6, DISP_CFONT, "%s..%s", szDispBuf2, &szDispBuf[iTotalLen-16+2]);
			}
			lcdFlip();
			iKeyVal = kbGetKeyMs(100);
			if (iKeyVal != KEY_TIMEOUT )
			{
				break;
			}
			uiNewTime = sysGetTimerCount();
			if (uiOldTime + ucTimeout*1000 < uiNewTime)
			{
				break;
			}
			
			DrawRect(120,62,127,63);
			lcdFlip();
			sysDelayMs(150);
		}
		switch(iKeyVal)
		{
			case KEY0:
				if (pszNowChar==szBuff || pszNowChar>=szBuff+ucMaxLen)
				{
					PubBeepErr();
				}
				else
				{
					*pszNowChar++ = iKeyVal;
					*pszNowChar   = 0;
				}
				break;
			case KEY1:
			case KEY2:
			case KEY3:
			case KEY4:
			case KEY5:
			case KEY6:
			case KEY7:
			case KEY8:
			case KEY9:
				if (pszNowChar>=szBuff+ucMaxLen)
				{
					PubBeepErr();
				}
				else
				{
					*pszNowChar++ = iKeyVal;
					*pszNowChar   = 0;
				}
				break;

			case KEY_CLEAR: 
				pszNowChar  = szBuff;
				*pszNowChar = 0;
				break;

			case KEY_BACKSPACE:
				if (pszNowChar<=szBuff)
				{
					PubBeepErr();
				}
				else
				{
					*(--pszNowChar) = 0;
				}
				break;

			case KEY_ENTER:
				if (pszNowChar>=szBuff+ucMinLen)
				{
					*pszData = 0;
					strcpy((char *)pszData, (char *)szBuff);
					if (strlen((char *)szBuff)==0)
					{
						strcpy((char *)pszData, "0");
					}
					return 0;
				}
				else
				{
					PubBeepErr();
				}
				break;
			case KEY_CANCEL:
			case KEY_TIMEOUT:
				return 0xFF;
			default:
				PubBeepErr();
				break;
		}
	}
}

void MapChar2Index(uint8_t ch, int *piRow, int *piCol)
{
	int		i;
	char	*p;

	for(i=0; i<sizeof(pszAlphaTbl)/sizeof(pszAlphaTbl[0]); i++)
	{
		for(p=pszAlphaTbl[i]; *p; p++)
		{
			if( *p==toupper(ch) )
			{
				*piRow = i;
				*piCol = p-pszAlphaTbl[i];
				break;
			}
		}
	}
}

void ShowWorkBuf(uint8_t ucMode, uint8_t *pszWorkBuff, int iLen)
{
	int		iLineNum, iCnt, iLeftBytes;
	uint8_t	ucLineNo, sTemp[NUM_MAXZHCHARS];

	iLineNum = (iLen+NUM_MAXZHCHARS-1)/NUM_MAXZHCHARS;
	if (iLineNum<=1)
	{
		iLineNum = 1;
	}
	if( iLineNum>=NUM_MAXZHLINES )
	{
		lcdCls();	
	}
	else
	{
		sgMinLineNo = MIN(sgMinLineNo, (uint8_t)(NUM_MAXLINENO-iLineNum*2+1));
		lcdClrLine(sgMinLineNo, NUM_MAXLINENO);
	}

	memset(sTemp, '*', sizeof(sTemp));
	ucLineNo = NUM_MAXLINENO-1;
	for(iCnt=1; iCnt<=3; iCnt++)
	{
		if( (iCnt*NUM_MAXZHCHARS)<=iLen )
		{
			iLeftBytes = NUM_MAXZHCHARS;
			if( ucMode & PASS_IN )
			{
				lcdDisplay(0, ucLineNo, DISP_CFONT, "%.*s", iLeftBytes, sTemp);
			}
			else
			{
				lcdDisplay(0, ucLineNo, DISP_CFONT, "%.*s", iLeftBytes, pszWorkBuff + iLen - iCnt*NUM_MAXZHCHARS);
			}
		}
		else
		{
			iLeftBytes = iLen - (iCnt-1)*NUM_MAXZHCHARS;
			if( iLeftBytes<=0 && iCnt>1 )
			{
				break;
			}
			if( ucMode & PASS_IN )
			{
				lcdDisplay(0, ucLineNo, DISP_CFONT, "%*.*s", NUM_MAXZHCHARS, iLeftBytes, sTemp);
			}
			else
			{
				lcdDisplay(0, ucLineNo, DISP_CFONT, "%*.*s", NUM_MAXZHCHARS, iLeftBytes, pszWorkBuff);
			}
			break;
		}
		ucLineNo -= 2;
	}
}

uint8_t PubGetString(uint8_t ucMode, uint8_t ucMinlen, uint8_t ucMaxlen, uint8_t *pszOut, uint8_t ucTimeOut, uint8_t ucEnter)
{
	uint8_t	bClearInData, szWorkBuff[LEN_GETSTRING+1];
	int		iLen, iIndex, iLastKey,iKeyValue,iCnt,iKeyAlpha;
	uint32_t  uiOldTime, uiNewTime;

	if (gstPosCapability.uiPosType == 4)
		iKeyAlpha = KEY_FN;
	else
		iKeyAlpha = KEY_ALPHA;

	iIndex = iLastKey = 0;
	PubASSERT( (ucMode & NUM_IN) || (ucMode & PASS_IN) || (ucMode & ALPHA_IN) || (ucMode & HEX_IN) );
	PubASSERT( !((ucMode & CARRY_IN) && (ucMode & ECHO_IN)) );
	if( (ucMode & CARRY_IN) || (ucMode & ECHO_IN) )
	{
		sprintf((char *)szWorkBuff, "%.*s", LEN_GETSTRING, pszOut);
		if (ucMode & HEX_IN)
		{
			iLen = strlen((char *)szWorkBuff);
			for (iCnt=0; iCnt<iLen;iCnt++)
			{
				if ( ( ('0' <= szWorkBuff[iCnt]) && (szWorkBuff[iCnt] <= '9') ) ||
					 ( ('A' <= szWorkBuff[iCnt]) && (szWorkBuff[iCnt] <= 'F') ) || 
					 ( ('a' <= szWorkBuff[iCnt]) && (szWorkBuff[iCnt] <= 'f') ) )
				{
				   continue;	
				}
				else
				{
					memset(szWorkBuff, 0, sizeof(szWorkBuff));
				}
			}
		}
	}
	else
	{
		memset(szWorkBuff, 0, sizeof(szWorkBuff));
	}

	iLen = strlen((char *)szWorkBuff);
	if( iLen>0 )
	{
		MapChar2Index(szWorkBuff[iLen-1], &iLastKey, &iIndex);
	}
	else
	{
		iLastKey = -1;
		iIndex   = 0;
	}

	sgMinLineNo  = 0xFF;
	bClearInData = (ucMode & ECHO_IN);
	while( 1 )
	{
		uiOldTime = sysGetTimerCount();
		while(1)
		{
			ShowWorkBuf(ucMode, szWorkBuff, iLen);
			lcdClrRect(150,144,170,145);
			lcdFlip();
			iKeyValue = kbGetKeyMs(100);
			if (iKeyValue != 0 )
			{
				break;
			}

			uiNewTime = sysGetTimerCount();
			if (uiOldTime + ucTimeOut*1000 < uiNewTime)
			{
				break;
			}

			lcdDrawFrame(150,144,170,145,ON);
			lcdFlip();
			sysDelayMs(150);
		}
		if( iKeyValue==KEY_CANCEL ||iKeyValue==KEY_TIMEOUT )
		{
			return 1;
		}
		else if( iKeyValue==KEY_ENTER )
		{
			bClearInData = FALSE;
			if(iLen==0 && ucEnter==1)
			{
			    sprintf((char *)pszOut, "%s", szWorkBuff);
				return 0;

			}
			if( iLen<(int)ucMinlen )
			{
				continue;
			}
			sprintf((char *)pszOut, "%s", szWorkBuff);
			break;
		}
		else if( iKeyValue>=KEY0 && iKeyValue<=KEY9 )
		{
			if( bClearInData )
			{	// clear in buffer
				szWorkBuff[0] = 0;
				iLen          = 0;
				iLastKey      = -1;
				bClearInData  = FALSE;
			}
			// save key in data
			if( iLen<(int)ucMaxlen )
			{
				szWorkBuff[iLen++] = iKeyValue;
				szWorkBuff[iLen]   = 0;
				iLastKey = iKeyValue - KEY0;
				iIndex   = 0;
			}
		}
		else if( iKeyValue==KEY_CLEAR )
		{
			szWorkBuff[0] = 0;
			iLen          = 0;
			iLastKey      = -1;
		}
		else if( iKeyValue==KEY_BACKSPACE )	// backspace
		{
			bClearInData = FALSE;
			if( iLen<=0 )
			{
				continue;
			}
			szWorkBuff[--iLen] = 0;
			if( iLen>0 )
			{
				MapChar2Index(szWorkBuff[iLen-1], &iLastKey, &iIndex);
			}
			else
			{
				iLastKey = -1;
				iIndex   = 0;
			}
		}
		else if( iKeyValue==iKeyAlpha )
		{
			bClearInData = FALSE;
			if( !( (ucMode & ALPHA_IN) || (ucMode & HEX_IN)) || iLastKey<0 || iLen<1 )
			{
				continue;
			}
			iIndex = (iIndex+1)%strlen(pszAlphaTbl[iLastKey]);
			if (ucMode&HEX_IN)
			{
				if ( ( ('0' <= pszAlphaTbl[iLastKey][iIndex]) && (pszAlphaTbl[iLastKey][iIndex] <= '9') ) ||
					 ( ('A' <= pszAlphaTbl[iLastKey][iIndex]) && (pszAlphaTbl[iLastKey][iIndex] <= 'F') ) || 
					 ( ('a' <= pszAlphaTbl[iLastKey][iIndex]) && (pszAlphaTbl[iLastKey][iIndex] <= 'f') ))
				{
					szWorkBuff[iLen-1] = pszAlphaTbl[iLastKey][iIndex];
				}
				else
				{
					PubBeepErr();
					continue;
				}
			}
			else
			{
				szWorkBuff[iLen-1] = pszAlphaTbl[iLastKey][iIndex];
			}
		}
	}

	return 0;
}

void PubDes(uint8_t ucMode, uint8_t *psData, uint8_t *psKey, uint8_t *psResult)
{
	PubASSERT(ucMode==ONE_ENCRYPT  || ucMode==ONE_DECRYPT ||
		ucMode==TRI_ENCRYPT  || ucMode==TRI_DECRYPT ||
		ucMode==TRI_ENCRYPT3 || ucMode==TRI_DECRYPT3);
	
	switch(ucMode)
	{
	case ONE_ENCRYPT:
		TDEA(psData, psResult, psKey, 8, TDEA_ENCRYPT);
		break;
		
	case ONE_DECRYPT:
		TDEA(psData, psResult, psKey, 8, TDEA_DECRYPT);
		break;
		
	case TRI_ENCRYPT:
		TDEA(psData, psResult, psKey, 16, TDEA_ENCRYPT);
		break;
		
	case TRI_DECRYPT:
		TDEA(psData, psResult, psKey, 16, TDEA_DECRYPT);
		break;
		
	case TRI_ENCRYPT3:
		TDEA(psData, psResult, psKey, 24, TDEA_ENCRYPT);
		break;
		
	case TRI_DECRYPT3:
		TDEA(psData, psResult, psKey, 24, TDEA_DECRYPT);
		break;
	}

	return ;
}


void PubCalcMac(uint8_t ucMode, uint8_t *psKey, uint8_t *psMsg, uint32_t uiLen, uint8_t *psMac)
{
	uint8_t   sOutMac[8];
	uint32_t	uiOffset, i;

	PubASSERT(ucMode==MAC_FAST || ucMode==MAC_ANSIX99);
	memset(sOutMac, 0, sizeof(sOutMac));
	uiOffset = 0;

	while(uiLen>uiOffset)
	{
		if (uiLen-uiOffset<=8)
		{
			for (i=0; i<uiLen-uiOffset; i++)
			{
				sOutMac[i] ^= psMsg[uiOffset+i];
			}
			break;
		}
		for (i=0; i<8; i++)
		{
			sOutMac[i] ^= psMsg[uiOffset+i];
		}
		if (ucMode==MAC_ANSIX99)
		{
			PubDes(ONE_ENCRYPT, sOutMac, psKey, sOutMac);
		}
		uiOffset += 8;
	}

	PubDes(ONE_ENCRYPT, sOutMac, psKey, psMac);
}


int PubCheckCHNString(char *psString, int iLen)
{
	while(iLen > 0 && psString != NULL)
	{
		if (*psString & 0x80)
		{
			return TRUE;
		}

		iLen--;
		psString++;
	}
	
	return FALSE;
}

/*********************************************************************************************/
/*********************************************************************************************/
/***************************************************************************************
    For Debug use
***************************************************************************************/


#ifdef _POS_DEBUG
void DispHexMsg(uint8_t *pszTitle, uint8_t *psMsg, uint32_t uiMsgLen, short nTimeOut)
{
	uint32_t	i, iLineNum, iPageNo, iPageNum, iDispLine;
	uint8_t	*pszBuff, ucStartLine;
	int    iKeyValue;

	// Format message
	uiMsgLen = MIN(uiMsgLen, LEN_DBGDATA);
	glszDbgInfo[0] = 0;
	for (pszBuff=glszDbgInfo,i=0; i<uiMsgLen; i+=MAX_CHARS)
	{
		if (uiMsgLen-i<MAX_CHARS)
		{
			pszBuff += DispHexLine(pszBuff, i, psMsg+i, uiMsgLen-i);
		}
		else
		{
			pszBuff += DispHexLine(pszBuff, i, psMsg+i, MAX_CHARS);
		}
	}   // end of for (pszBuff=

	// Display message
	if (pszTitle!=NULL && *pszTitle!=0)
	{
		PubDisplayTitle(TRUE, pszTitle);
		iDispLine   = NUM_MAXLINES-2;
		ucStartLine = 2;
	}
	else
	{
		iDispLine   = NUM_MAXLINES;
		ucStartLine = 0;
	}

	iLineNum = (strlen((char *)glszDbgInfo)+CHARS_PERLINE-1)/CHARS_PERLINE;
	iPageNum = (iLineNum+iDispLine-1)/iDispLine;
	iPageNo  = 0;

	while(1)
	{
		// Light/Close up & down icon
		lcdSetIcon(ICON_UP,   (uint8_t)(iPageNo>0		  ? OPENICON : CLOSEICON));
		lcdSetIcon(ICON_DOWN, (uint8_t)(iPageNo<iPageNum-1 ? OPENICON : CLOSEICON));

		lcdClrLine(ucStartLine, NUM_MAXLINENO);
		lcdSetIcon(0, ucStartLine, DISP_ASCII, "%.*s", CHARS_PERLINE*iDispLine,
				 &glszDbgInfo[CHARS_PERLINE*iDispLine*iPageNo]);

		lcdFlip();
		iKeyValue = PubWaitKey(nTimeOut);
		lcdSetIcon(ICON_UP,   CLOSEICON);
		lcdSetIcon(ICON_DOWN, CLOSEICON);

		if (iKeyValue==KEY_UP)
		{
			iPageNo = (iPageNo==0) ? iPageNum-1 : iPageNo-1;
		}
		else if (iKeyValue==KEY_DOWN || iKeyValue==KEY_ENTER)
		{
			iPageNo = (iPageNo>=iPageNum-1) ? 0 : iPageNo+1;
		}
		else if (iKeyValue==KEY_CANCEL || iKeyValue==KEY_TIMEOUT)
		{
			return;
		}
	}   // end of while(1
}

// print a line as hexadecimal format
int DispHexLine(uint8_t *pszBuff, uint32_t uiOffset, uint8_t *psMsg, uint32_t uiMsgLen)
{
	uint32_t	i;
	uint8_t	*p = pszBuff;

	// Print line information
	pszBuff += sprintf((char *)pszBuff, "%04Xh:", uiOffset);

	for (i=0; i<uiMsgLen; i++)
	{
		pszBuff += sprintf((char *)pszBuff, " %02X", psMsg[i]);
	}
	for (; i<MAX_CHARS; i++)
	{   // append blank spaces, if needed
		pszBuff += sprintf((char *)pszBuff, "   ");
	}

	return (pszBuff-p);
}

void DispAssert(uint8_t *pszFileName, ulong ulLineNo)
{
	uint8_t	szFName[30];

	PubDisplayTitle(TRUE, (uint8_t *)"Assert Failure");
	GetBaseName(pszFileName, szFName);
	lcdDisplay(0, 2, DISP_CFONT, "FILE:%.11s", szFName);
	lcdDisplay(0, 4, DISP_CFONT, "LINE:%ld", ulLineNo);
	PubDisplayMsg(6, (uint8_t *)"PRESS ENTER EXIT");
	PubLongBeep();
	lcdFlip();

	while(!PubYesNo(60));
}

// get basename of a full path name
void GetBaseName(uint8_t *pszFullPath, uint8_t *pszBaseName)
{
	uint8_t	*pszTmp;
	
	*pszBaseName = 0;
	if (!pszFullPath || !*pszFullPath)
	{
		return;
	}
	
	pszTmp = &pszFullPath[strlen((char *)pszFullPath)-1];
	while( pszTmp>=pszFullPath && *pszTmp!='\\' && *pszTmp!='/' )
	{
		pszTmp--;
	}
	sprintf((char *)pszBaseName, "%s", (char *)(pszTmp+1));
}



#else /* _POS_DEBUG */

#endif /* _POS_DEBUG */

