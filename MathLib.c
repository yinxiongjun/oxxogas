
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "MathLib.h"
#include "glbvar.h"

void PubXor(char *psSource1, char *psSource2, int iLength)
{
    int   iCnt;

    for(iCnt = 0; iCnt < iLength; iCnt++)
    {
        psSource1[iCnt] ^= psSource2[iCnt];
    }
}

//0x12AB-->"12AB"
void PubBcd2Asc(char *psIHex, int iHexLen, char *psOAsc)
{
    static const char szMapTable[17] = {"0123456789ABCDEF"};
    int   iCnt,index;
    unsigned char  ChTemp;

    for(iCnt = 0; iCnt < iHexLen; iCnt++)
    {
		ChTemp = (unsigned char)psIHex[iCnt];
		index = (ChTemp / 16) & 0x0F;
        psOAsc[2*iCnt]   = szMapTable[index];
		ChTemp = (unsigned char) psIHex[iCnt];
		index = ChTemp & 0x0F;
        psOAsc[2*iCnt + 1] = szMapTable[index];
    }
}

// '\0'
void PubBcd2Asc0(uint8_t *psIHex, int iHexLen, char *pszOAsc)
{
    PubBcd2Asc((char *)psIHex, iHexLen, pszOAsc);
    pszOAsc[2*iHexLen] = 0;
}

//"12AB"-->0x12AB
void PubAsc2Bcd(char *psIAsc, int iAscLen, char *psOBcd)
{
    char   Chtmp,ChBcd;
    int    iCnt;

    for(iCnt = 0; iCnt < iAscLen; iCnt += 2)
    {
        Chtmp = psIAsc[iCnt];
		if( Chtmp >= 'A' )
        {
			Chtmp = (char)toupper((int)Chtmp) - 'A' + 0x0A;
        }
        else
        {
            Chtmp &= 0x0F;
        }
		ChBcd = (Chtmp << 4); 
        Chtmp = psIAsc[iCnt+1];
        if( Chtmp >= 'A' )
        {
            Chtmp = (char)toupper((int)Chtmp) - 'A' + 0x0A;
        }
		else
        {
            Chtmp &= 0x0F;
        }
		ChBcd |= Chtmp;

        psOBcd[iCnt/2] = ChBcd;
    }
}

//atol()
unsigned long PubAsc2Long(char *psString, int iStrLen, ulong *pulOut)
{
    char    szBuff[15+1];
	unsigned long    ulTmp;

    sprintf(szBuff, "%.*s", (iStrLen <= 15 ? iStrLen : 15 ), psString);
	ulTmp =  (unsigned long)atol(szBuff);

    if (pulOut != NULL)
	{
		*pulOut = ulTmp;
	}
    return ulTmp;
}

void PubLong2Char(unsigned long ulSource, int iAsclen, char *psOAsc)
{
    int    iCnt;

    for(iCnt = 0; iCnt < iAsclen; iCnt++)
    {
        psOAsc[iCnt] = (char)(ulSource >> (8 * (iAsclen - iCnt - 1)));
    }
}

unsigned long PubChar2Long(uint8_t *psString, int iStrLen, unsigned long *pulOut)
{
    int     iCnt;
	unsigned long   ulTmp,ulValue;

    ulTmp = 0L;
    for(iCnt = 0; iCnt < iStrLen; iCnt++)
    {
		ulValue = (unsigned long)psString[iCnt] & 0xFF;
        ulTmp |= ( ulValue << 8 * (iStrLen - iCnt - 1));
    }

	if (pulOut != NULL)
	{
		*pulOut = ulTmp;
	}
    return ulTmp;
}

//1234 --> 0x12 0x34
void PubLong2Bcd(unsigned long ulSource, int iBcdLen, char *psOBcd)
{
    char    szFmt[30], szBuf[30];

    sprintf(szFmt, "%%0%dlu", iBcdLen*2);
    sprintf(szBuf, szFmt, ulSource);
    if( psOBcd != NULL )
    {
        PubAsc2Bcd(szBuf, iBcdLen * 2, psOBcd);
    }
}

unsigned long PubBcd2Long(uint8_t *psBcd, int iBcdLen, unsigned long *pulOut)
{
    char   szBuf[30];
	unsigned long   ulTmp;

	memset(szBuf,0,sizeof(szBuf));
    PubBcd2Asc0(psBcd, iBcdLen, szBuf);
	ulTmp = (unsigned long)atol((char *)szBuf);
    if( pulOut != NULL )
    {
        *pulOut = ulTmp;
    }
    return ulTmp;
}

//To convert a string to uppercase
void PubStrUpper(char *pszString)
{
    while( *pszString )
    {
        *pszString = toupper((char)*pszString);
        pszString++;
    }
}

//To convert a string to lowercase
void PubStrLower(char *pszString)
{
    while( *pszString )
    {
        *pszString = (char)tolower((int)*pszString);
        pszString++;
    }
}

//Remove the string in the blank character
void PubTrimStr(char *pszString)
{
#define ISSPACE(ch) ( ((ch) == ' ')  || ((ch) == '\t') || \
                      ((ch) == '\n') || ((ch) == '\r') )

    char *p, *q;

    if( !pszString || !*pszString )
    {
        return;
    }

    p = &pszString[strlen((char*)pszString) - 1];
    while(( p > pszString) && ISSPACE(*p) )
    {
        *p-- = 0;
    }
    if( (p == pszString) && ISSPACE(*p) )  *p = 0;

    for(p = pszString; *p && ISSPACE(*p); p++);
    if( p != pszString )
    {
        q = pszString;
        while( *p )   *q++ = *p++;
        *q = 0;
    }
#undef ISSPACE
}


//Remove both ends of the specific character string
void PubTrimSpcStr(char *pszString, char ChSpcChar)
{
	char *p, *q;

	if( !pszString || !*pszString )
	{
		return;
	}

	p = &pszString[strlen((char*)pszString) - 1];
	while( (p > pszString) && ((*p) == ChSpcChar))
	{
		*p-- = 0;
	}
	if( p == pszString && ((*p) == ChSpcChar) )  *p = 0;

	for(p = pszString; *p && ((*p) == ChSpcChar); p++);
	if( p != pszString )
	{
		q = pszString;
		while( *p )   *q++ = *p++;
		*q = 0;
	}
}


//Remove the string on the right side of the specific character of continuous
void PubTrimTailChars(char *pszString, char ChRemoveChar)
{
	int		i, iLen;

	if( !pszString || !*pszString )
    {
        return;
    }

	iLen = strlen((char *)pszString);
	for(i=iLen-1; i>=0; i--)
	{
		if( pszString[i]!=ChRemoveChar )
		{
			break;
		}
		pszString[i] = 0;
	}
}

// Remove leading character
void PubTrimHeadChars(char *pszString, char ChRemoveChar)
{
	char	*p;

	if( !pszString || !*pszString )
    {
        return;
    }

	for(p=pszString; *p && *p==ChRemoveChar; p++);
	if( p!=pszString )
	{
		while( (*pszString++ = *p++) );
	}
}

// Fill the leading character
void PubAddHeadChars( char *pszString, int iTargetLen, char ChAddChar )
{
	int	iLen;

	iLen = strlen((char *)pszString);
	if( iLen>=iTargetLen )
	{
		return;
	}

	memmove(pszString+iTargetLen-iLen, pszString, iLen+1);
	memset(pszString, ChAddChar, iTargetLen-iLen);
}

//Compare two strings without regard to capital or lowercase
int PubStrNoCaseCmp(char *pszStr1, char *pszStr2)
{
    if( !pszStr1 || !pszStr2 )
    {
        return -1;
    }

    while( *pszStr1 && *pszStr2 )
    {
        if( toupper((char)*pszStr1) != toupper((char)*pszStr2) )
        {
            return (toupper((char)*pszStr1) - toupper((char)*pszStr2));
        }
        pszStr1++;
        pszStr2++;
    }
    if( !*pszStr1 && !*pszStr2 )
    {
        return 0;
    }
    if( !*pszStr1 )
    {
        return -1;
    }

    return 1;
}

//Reverse string
void PubStrReverse(char *pszString)
{
    int     i, j, iLength;
    char    ucTmp;

    iLength = strlen((char*)pszString);
    for(i = 0,j = iLength - 1; i < iLength / 2; i++,j--)
    {
        ucTmp        = pszString[i];
        pszString[i] = pszString[j];
        pszString[j] = ucTmp;
    }
}


//get the high 4 bit of the byte
char PubHigh4Bit(char ChInChar)
{
	return ((ChInChar/16) & 0x0F);
}


//get the low 4 bit of the byte
char PubLow4Bit(char ChInChar)
{
	return (ChInChar & 0x0F);
}


//Two ASC string multiplication
void PubAscMul(char *pszFaciend, char *pszMultiplier, char *pszProduct)
{
	char	*p, ucTemp, ucCarryBit, szBuff[100+1];
	int	    iFaciLen, iMulLen, iProdPos, iCnt;

	iFaciLen = strlen((char *)pszFaciend);
	iMulLen  = strlen((char *)pszMultiplier);

	sprintf((char *)szBuff, "%0*ld", iFaciLen+iMulLen, 0L);

	for(iProdPos=0; iFaciLen>0; iFaciLen--,iProdPos++)
	{
		ucCarryBit = 0;
		p = &szBuff[iProdPos];

		for(iCnt=iMulLen; iCnt>0; iCnt--)
		{
			ucTemp = (pszFaciend[iFaciLen-1] & 0x0F) * (pszMultiplier[iCnt-1] & 0x0F) +
					 ucCarryBit + (*p & 0x0F);
			*p++   = (ucTemp % 10) + '0';
			ucCarryBit = ucTemp / 10;
		}
		if( ucCarryBit!=0 )
		{
			*p++ = ucCarryBit + '0';
		}
	}
	PubTrimTailChars(szBuff, '0');
	PubStrReverse(szBuff);

	if( szBuff[0]==0 )
	{
		strcpy((char *)szBuff, "0");
	}
	if( pszProduct!=NULL )
	{
		sprintf((char *)pszProduct, "%s", szBuff);
	}
}


void PubAscAdd(char *psAddend1, char *psAddend2, int iLen, char *pszSum)
{
	char	*pszResult, ChCarryBit, ChTemp, szBuff[100];

	ChCarryBit = 0;
	pszResult  = szBuff;
	while( iLen>0 )
	{
		ChTemp = (psAddend1[iLen-1] & 0x0F) + (psAddend2[iLen-1] & 0x0F) + ChCarryBit;
		*pszResult++ = (ChTemp % 10) + '0';
		ChCarryBit   = (ChTemp>9) ? 1 : 0;
		iLen--;
	}
	if( ChCarryBit!=0 )
	{
		*pszResult++ = '1';
	}
	*pszResult = 0;

	PubStrReverse(szBuff);
	if( pszSum!=NULL )
	{
		sprintf((char *)pszSum, "%s", szBuff);
	}
}


//(psMinuend>psSubtrahend)
void PubAscSub(char *psMinuend, char *psSubtrahend, int iLen, char *pszResult)
{
	char	*pszOut, ChCarryBit, ChTemp, szBuff[100];

	pszOut     = szBuff;
	ChCarryBit = 0;
	while( iLen>0 )
	{
		ChTemp = (psMinuend[iLen-1] & 0x0F) - (psSubtrahend[iLen-1] & 0x0F) - ChCarryBit + 10;
		*pszOut++  = (ChTemp % 10) + '0';
		ChCarryBit = (psMinuend[iLen-1]<psSubtrahend[iLen-1]+ChCarryBit) ? 1 : 0;
		iLen--;
	}
	*pszOut = 0;

	PubStrReverse(szBuff);
	if( pszResult!=NULL )
	{
		sprintf((char *)pszResult, "%s", szBuff);
	}
}

//Minus 1
char PubAscDec(char *psAscStr, int iStrLen)
{
	psAscStr += iStrLen - 1;
	while(--(*psAscStr) < '0')
	{
		*psAscStr-- = '9';
		if(--iStrLen == 0)
			return 1;
	}

	return 0;
}


//plus 1
char PubAscInc(char *psAscStr, int iStrLen)
{
	psAscStr += iStrLen - 1;
	while(++(*psAscStr) > '9')
	{
		*psAscStr-- = '0';
		if(--iStrLen == 0) 
			return 1;
	}

	return 0;
}


void PubBcdAdd(char *psAddend1, char *psAddend2, int iLen, char *psResult)
{
	char	sAdd1[100+1], sAdd2[100];

	PubBcd2Asc(psAddend1, iLen, sAdd1);
	PubBcd2Asc(psAddend2, iLen, sAdd2);
	PubAscAdd(sAdd1, sAdd2, iLen*2, sAdd1);
	PubAsc2Bcd(sAdd1, iLen*2, psResult);
}


int PubBcdSub(char *psMinuend, char *psSubtrahend, int iLen, char *psResult)
{
    int     iCarry;
	char	sMinuend[100+1], sSubtrahend[100];
    
	PubBcd2Asc(psMinuend, iLen, sMinuend);
	PubBcd2Asc(psSubtrahend, iLen, sSubtrahend);

    if(memcmp(sMinuend, sSubtrahend, iLen*2) < 0)
    {
		PubAscSub(sSubtrahend, sMinuend, iLen*2, sMinuend);
        iCarry = 1;
    }
    else
    {
		PubAscSub(sMinuend, sSubtrahend, iLen*2, sMinuend);
        iCarry = 0;
    }
        
	PubAsc2Bcd(sMinuend, iLen*2, psResult);
    return iCarry;
}


//Minus 1
char PubBcdDec(char *psBcdStr, int iStrLen)
{
	psBcdStr += iStrLen - 1;
	while((char)--(*psBcdStr) < 0)
	{
		*psBcdStr = (char)0x99;
		psBcdStr--;
		if(--iStrLen == 0)
			return 1;
	}

	if(((*psBcdStr) & 0x0f) > 9)
		*psBcdStr -= 0x06;

	return 0;
}


//plus 1
char PubBcdInc(unsigned char *psBcdStr, int iStrLen)
{
	psBcdStr += iStrLen - 1;
	while(++(*psBcdStr) > 0x99)
	{
		*psBcdStr = 0;
		psBcdStr--;
		if((--iStrLen) == 0)
			return 1;
	}

	if(((*psBcdStr) & 0x0f) > 9)
		*psBcdStr += 0x06;

	return 0;
}

char PubCalcLRC(char *psData, unsigned long ulLength, char ChInit)
{
	while( ulLength>0 )
	{
		ChInit ^= *psData++;
		ulLength--;
	}
	
	return ChInit;
}

unsigned short   PubGetCRC(char *psData, unsigned long ulLength)
{
	int  crc_return;                                                                    
	unsigned char CRC16Lo=INI_VECTORLo;                                                            
	unsigned char CRC16Hi=INI_VECTORHi;                                                           
	unsigned char SaveHi,SaveLo;                                                                  
	unsigned int i, j;                                                                            

	for (i=0; i<ulLength; i++)
	{                                                              
		CRC16Lo = CRC16Lo ^ *psData++;
		
		for (j=0; j<8; j++)
		{                                                                          
			SaveHi = CRC16Hi;                                                                           
			SaveLo = CRC16Lo;                                                                           
			CRC16Hi = CRC16Hi >>1;                                                                      
			CRC16Lo = CRC16Lo >>1;       
			
			if ((SaveHi & 0x1) == 0x1)  
			{
				CRC16Lo = CRC16Lo | 0x80;       
			}
			
			if ((SaveLo & 0x1) == 0x1)
			{                                                                 
				CRC16Hi = CRC16Hi ^ POLYNOMIALHi;                                                         
				CRC16Lo = CRC16Lo ^ POLYNOMIALLo;                                                         
			}                                                                                           
		}                                                                                             
	}    

	crc_return=CRC16Hi*256 + CRC16Lo;                                                             
	return (crc_return);
}

void showHex(char *pTitle, uint8_t *pHex, int iLen)
{	
	int i;
	printf("%s:\n",pTitle);
	
	for(i = 0; i < iLen; i++){
		printf("%02x ",pHex[i]);
	}
	
	printf("\n\n");
}


long PubBcdStrtol(int len, const char *nptr,char **endptr,int base)
{
	long lTmp;
	char *pTmp = NULL;
	pTmp = (char *)malloc(len*2);
	if (pTmp == NULL) {
		return -1;
	}
	
	PubBcd2Asc((char *)nptr, len, pTmp);
		
	lTmp = strtol(pTmp, endptr, base);
	free(pTmp);

	return lTmp;
}


// end of file

