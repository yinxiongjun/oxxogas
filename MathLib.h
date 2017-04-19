
/****************************************************************************
NAME
	Mathlib.h - Arithmetic operations

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	pyming		2008.03.14		- add precompile process
****************************************************************************/

#ifndef _MATHLIB_H
#define _MATHLIB_H

#include "glbvar.h"

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define INI_VECTORLo 0x00
#define INI_VECTORHi 0x00
#define POLYNOMIALHi 0x80
#define POLYNOMIALLo 0x05

#define OFFSET(type, member)    ( (unsigned long)(&((type *)0)->member) )


#ifndef  MAX
#define MAX(a, b)       ( (a)>=(b) ? (a) : (b) )
#endif

#ifndef  MIN
#define MIN(a, b)       ( (a)<=(b) ? (a) : (b) )
#endif


#ifndef  ABS
#define  ABS(a)      ( (a)>=0 ? (a) : (-1)*(a) )
#endif

void PubXor(char *psSource1, char *psSource2, int iLength);
void PubBcd2Asc(char *psIHex, int iHexLen, char *psOAsc);
void PubBcd2Asc0(uint8_t *psIHex, int iLength, char *pszOAsc);
void PubAsc2Bcd(char *psIAsc, int iAscLen, char *psOBcd);
ulong PubAsc2Long(char *psString, int iStrLen, unsigned long *pulOut);
void PubLong2Char(unsigned long ulSource, int iAsclen, char *psOAsc);
unsigned long PubChar2Long(uint8_t *psString, int iStrLen, unsigned long *pulOut);
void PubLong2Bcd(unsigned long ulSource, int iBcdLen, char *psOBcd);
unsigned long PubBcd2Long(uint8_t *psBcd, int iBcdLen, unsigned long *pulOut);
void PubStrUpper(char *pszString);
void PubStrLower(char *pszString);
void PubTrimStr(char *pszString);
void PubTrimSpcStr(char *pszString, char ucSpcChar);
void PubTrimTailChars(char *pszString, char ucRemoveChar);
void PubTrimHeadChars(char *pszString, char ucRemoveChar);
int PubStrNoCaseCmp(char *pszStr1, char *pszStr2);
void PubAddHeadChars(char *pszString, int uiTargetLen, char ChAddChar);
void PubStrReverse(char *pszString);
char PubHigh4Bit(char ChInChar);
char PubLow4Bit(char ChInChar);
void PubAscMul(char *pszFaciend, char *pszMultiplier, char *pszProduct);
void PubAscAdd(char *psAddend1, char *psAddend2, int iLen, char *pszSum);
#define SafeAscAdd(a,b,c)	PubAscAdd(a,b,c,a)

void PubAscSub(char *psMinuend, char *psSubtrahend, int iLen, char *pszResult);
#define SafeAscSub(a,b,c)	PubAscSub(a,b,c,a)

char PubAscDec(char *psAscStr, int iStrLen);
char PubAscInc(char *psAscStr, int iStrLen);
void PubBcdAdd(char *psAddend1, char *psAddend2, int iLen, char *psResult);
#define SafeBcdAdd(a,b,c)		PubBcdAdd(a, b, c, a)

int PubBcdSub(char *psMinuend, char *psSubtrahend, int iLen, char *psResult);
#define SafeBcdSub(a,b,c)		PubBcdSub(a, b, c, a)

char PubBcdDec(char *psBcdStr, int iStrLen);
char PubBcdInc(unsigned char *psBcdStr, int iStrLen);
char PubCalcLRC(char *psData, unsigned long ulLength, char ChInit);
unsigned short   PubGetCRC(char *psData, unsigned long ulLength);
void showHex(char *pTitle, uint8_t *pHex, int iLen);
long PubBcdStrtol(int len, const char *nptr,char **endptr,int base);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _UTIL_H */
