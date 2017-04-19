
/****************************************************************************
NAME
    pack8583.c 
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "posapi.h"
#include "pack8583.h"

static int iPackElement(FIELD_ATTR *pAttr, uint8_t *pusIn, uint8_t *pusOut, uint16_t *puiOutLen);
static int iUnPackElement(FIELD_ATTR *pAttr, uint8_t *pusIn, uint8_t *pusOut, uint16_t *puiInLen);

int iPack8583(FIELD_ATTR *pMsgAttr, FIELD_ATTR *pDataAttr, void *pSt8583, uint8_t *pusOut, uint16_t *puiOutLen)
{
    int     iRet, i;
    uint16_t    iMsgLen, iDataLen, iFieldLen, iMsgNum, uiBitmap;
    uint8_t     *pusI, *pusO, *pusB;

    pusI        = (uint8_t *)pSt8583;
    pusO        = pusOut;
    iMsgLen     = 0;
    iDataLen    = 0;
    *puiOutLen  = 0;

    iMsgNum = 0;
    while( (pMsgAttr+iMsgNum)->eElementAttr!=Attr_Over ) 
		iMsgNum++;
    for(i=0; i<iMsgNum; i++)
	{
        if( (pMsgAttr+i)->eElementAttr==Attr_UnUsed )   
			continue;
        iRet = iPackElement(pMsgAttr+i, pusI, pusO, (uint16_t *)&iFieldLen);
        if( iRet<=0 )
		{
            return ((-1)*(i+1));
        }
        pusI    += ((pMsgAttr+i)->uiLength + 2);
        pusO    += iFieldLen;
        iMsgLen += iFieldLen;
    }

    uiBitmap =  pDataAttr->uiLength;
    pusI     += (uiBitmap*2);
    pusB     =  pusO;

    pusO     += (uiBitmap*2);
    iDataLen =  uiBitmap*2;
    memset(pusB, 0, uiBitmap*2);

    for(i=1; i<uiBitmap*2*8; i++)
	{
        if( (pDataAttr+i)->eElementAttr==Attr_Over )
		{
            break;
        }
        if( (pDataAttr+i)->eElementAttr==Attr_UnUsed )
		{
            continue;
        }
        iRet = iPackElement(pDataAttr+i, pusI, pusO, (uint16_t *)&iFieldLen);
        if( iRet<0 )
		{
            return ((-1)*(1000+(i+1)));
        }
        if( iRet>0 )
		{
            pusO     += iFieldLen;
            iDataLen += iFieldLen;
            *(pusB+(i/8)) |= (0x80>>(i%8));
        }
        pusI+= ((pDataAttr+i)->uiLength + 2);
    }
    if( i%((pDataAttr+0)->uiLength*8) )
	{
        return ((-1)*(2000+(i+1)));
    }

    for(i=uiBitmap*2-1; i>=0; i--)
	{
        if(*(pusB+i))   break;
    }
    if( i>=8 )
	{
        *pusB |= 0x80;
    }
	else
	{
        memmove(pusB+uiBitmap, pusB+uiBitmap*2, iDataLen-uiBitmap*2);
        iDataLen -= uiBitmap;
    }

    *puiOutLen = iMsgLen+iDataLen;

    return 0;
}


int iUnPack8583(FIELD_ATTR *pMsgAttr, FIELD_ATTR *pDataAttr, uint8_t *pusIn, uint16_t uiInLen, void *pSt8583)
{
    int     iRet;
    uint16_t    i, iLen, iMsgNum, iFieldLen, uiBitmap;
    uint8_t     *pusI, *pusO, *pusB;

    pusI = pusIn;
    pusO = (uint8_t *)pSt8583;
    iLen = 0;

    iMsgNum = 0;
    while( (pMsgAttr+iMsgNum)->eElementAttr!=Attr_Over )   
		iMsgNum++;

    for(i=0; i<iMsgNum; i++)
	{
        if( (pMsgAttr+i)->eElementAttr==Attr_UnUsed )
		{
            continue;
        }
        iRet = iUnPackElement(pMsgAttr+i, pusI, pusO, (uint16_t *)&iFieldLen);
		if( iRet<0 )
		{
            return ((-1)*(i+1));
        }
        pusI += iFieldLen;
        pusO += ((pMsgAttr+i)->uiLength + 2);
        iLen += iFieldLen;
    }

    /*** generate data and bitmap of 8583 struct ***/
    pusB = pusI;
    if( pusB[0] & 0x80 )
	{
        uiBitmap = (pDataAttr+0)->uiLength*2;
    }
	else
	{
        uiBitmap= (pDataAttr+0)->uiLength;
    }
    memcpy(pusO, pusB, uiBitmap);
    pusI += uiBitmap;
    pusO += (pDataAttr+0)->uiLength*2;
    iLen += uiBitmap;

    for(i=1; i<(uiBitmap*8); i++)
	{
        if( (pDataAttr+i)->eElementAttr==Attr_Over )
		{
            return ((-1)*(2000+(i+1)));
        }
        if( *(pusB+i/8) & (0x80>>i%8) )
		{
            if( (pDataAttr+i)->eElementAttr==Attr_UnUsed )
			{
                return ((-1)*(2000+(i+1)));
            }
            iRet = iUnPackElement(pDataAttr+i, pusI, pusO, (uint16_t *)&iFieldLen);
			if( iRet<0 )
			{
                return ((-1)*(1000+(i+1)));
            }
            pusI += iFieldLen;
            iLen += iFieldLen;
        }

        if( (pDataAttr+i)->eElementAttr==Attr_UnUsed )
		{
            continue;
        }
        pusO += ((pDataAttr+i)->uiLength + 2);
    }   /*** for ***/

    if( uiInLen!=iLen )
	{
        return ERR_UNPACK_LEN;
    }

    return 0;
}


int iPackElement(FIELD_ATTR *pAttr, uint8_t *pusIn, uint8_t *pusOut, uint16_t *puiOutLen)
{
   uint16_t    i, j, iInLen, iOutLen=0;
   uint8_t     ucTemp;

    *puiOutLen = 0;

    if( pAttr->eElementAttr!=Attr_b )
	{
        iInLen = strlen((char *)pusIn);
    }
	else
	{
        iInLen = ((uint16_t)pusIn[0]<<8) | (uint16_t)pusIn[1];
        pusIn += 2;
    }

    if( iInLen<=0 )
	{
        return 0;
    }
    if( iInLen>pAttr->uiLength )
	{
        return -1;
    }

    switch( pAttr->eLengthAttr )
	{
    case Attr_fix:
         iOutLen = pAttr->uiLength;
         break;
    case Attr_var1:
         pusOut[0] = ((iInLen/10)<<4) | (iInLen%10);
         pusOut++;
         iOutLen = 1+iInLen;
         break;
    case Attr_var2:
         pusOut[0] = iInLen/100;
         pusOut[1] = (((iInLen%100)/10)<<4) | ((iInLen%100)%10);
         pusOut += 2;
         iOutLen = 2+iInLen;
         break;
    }  

    switch( pAttr->eElementAttr )
	{
    case Attr_n:
        switch( pAttr->eLengthAttr )
		{
        case Attr_fix:
            iOutLen= (pAttr->uiLength+1)/2;

            memset(pusOut, 0, iOutLen);
            for(i=0,j=0; i<iInLen; i+=2,j++)
			{
                if( i==iInLen-1 )
				{
                    if( toupper(pusIn[iInLen-i-1])<'A' )
					{
                        pusOut[iOutLen-j-1] = (pusIn[iInLen-i-1]&0x0F);
                    }
					else
					{
						pusOut[iOutLen-j-1] =
						toupper(pusIn[iInLen-i-1])-'A'+0x0A;
                    }
				}
				else
				{
                    if( toupper(pusIn[iInLen-i-2])<'A' )
					{
                        pusOut[iOutLen-j-1] = (pusIn[iInLen-i-2]&0x0F)<<4;
                    }
					else
					{
                        pusOut[iOutLen-j-1] =
                        (toupper(pusIn[iInLen-i-2])-'A'+0x0A)<<4;
                    }

                    if( toupper(pusIn[iInLen-i-1])<'A' )
					{
                        pusOut[iOutLen-j-1] |= (pusIn[iInLen-i-1]&0x0F);
                    }
					else
					{
                        pusOut[iOutLen-j-1] |=
                           (toupper(pusIn[iInLen-i-1])-'A'+0x0A);
                    }
                 }
            }   /*** for(i=0; ***/
            break;

        case Attr_var1:
        case Attr_var2:
            iOutLen = iOutLen - iInLen + (iInLen+1)/2;

            for(i=0; i<(iInLen+1)/2; i++)
			{
                if( toupper(pusIn[2*i])<'A' )
				{
                    pusOut[i] = (pusIn[2*i]&0x0F)<<4;
                }
				else
				{
                     pusOut[i] =
                        (toupper(pusIn[2*i])-'A'+0x0A)<<4;
                }

                if( toupper(pusIn[2*i+1])<'A'){
                     pusOut[i] |= (pusIn[2*i+1]&0x0F);
                }
				else
				{
                     pusOut[i] |= (toupper(pusIn[2*i+1])-'A'+0x0A);
                }
            }


            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_z:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iOutLen = (pAttr->uiLength+1)/2;

            memset( pusOut, 0, iOutLen );
            for(i=0,j=0; i<iInLen; i+=2,j++)
			{
                if( i==iInLen-1 )
				{
                    pusOut[iOutLen-j-1] = (pusIn[iInLen-i-1]&0x0F);
                }
				else
				{
                    pusOut[iOutLen-j-1] = ((pusIn[iInLen-i-2]&0x0F)<<4) |
                                    (pusIn[iInLen-i-1]&0x0F);
                }
            }   /*** for ***/
            break;

        case Attr_var1:
        case Attr_var2:
            iOutLen = iOutLen - iInLen + (iInLen+1)/2;
            for(i=0; i<(iInLen+1)/2; i++)
			{
				if( (pusIn[2*i] >= 'A' && pusIn[2*i] <= 'F') || (pusIn[2*i] >= 'a' && pusIn[2*i] <= 'f'))
				{
					ucTemp = ((pusIn[2*i]-'A'+0x0A)<<4);
				}
				else
				{
					ucTemp = ((pusIn[2*i]&0x0F)<<4);
				}

				if(  (pusIn[2*i+1] >= 'A' && pusIn[2*i+1] <= 'F') || (pusIn[2*i+1] >= 'a' && pusIn[2*i+1] <= 'f'))
				{
					pusOut[i] = ucTemp | (pusIn[2*i+1]-'A'+0x0A);
				}
				else
				{
					pusOut[i] = ucTemp | (pusIn[2*i+1]&0x0F);
				}
            }
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_b:
        switch( pAttr->eLengthAttr )
		{
        case Attr_fix:
            memset(pusOut, 0, iOutLen);
            memcpy(pusOut, pusIn, iInLen);
            break;

        case Attr_var1:
        case Attr_var2:
            memcpy(pusOut, pusIn, iInLen);
            break;
        }
        break;

    case Attr_a:
        switch( pAttr->eLengthAttr )
		{
        case Attr_fix:
            memset(pusOut, ' ', iOutLen);
            memcpy(pusOut, pusIn, iInLen);
            break;

        case Attr_var1:
        case Attr_var2:
            memcpy(pusOut, pusIn, iInLen);
            break;
        }
        break;
   }    /*** switch(pAttr->eElementAttr) ***/

   *puiOutLen = iOutLen;

   return 1;
}

int iUnPackElement(FIELD_ATTR *pAttr, uint8_t *pusIn, uint8_t *pusOut,
                   uint16_t *puiInLen)
{
    uint16_t    i, j, iInLen=0, iTmpLen=0;

    memset(pusOut, 0, pAttr->uiLength);
    *puiInLen = 0;

    switch( pAttr->eLengthAttr ){
    case Attr_fix:
        iInLen  = pAttr->uiLength;
        iTmpLen = iInLen;
        break;

    case Attr_var1:
        iTmpLen= (pusIn[0]>>4)*10 + (pusIn[0]&0x0F);
        pusIn++;
        iInLen= 1+iTmpLen;
        break;

    case Attr_var2:
        iTmpLen = (pusIn[0]&0x0F)*100 + (pusIn[1]>>4)*10 + (pusIn[1]&0x0F);
        pusIn += 2;
        iInLen = 2+iTmpLen;
        break;
    }   /*** switch(pAttr->eLengthAttr ***/

    if( iTmpLen>pAttr->uiLength ){
        return -1;
    }
    if( pAttr->eElementAttr==Attr_b ){
        pusOut[0] = (uint8_t)(iTmpLen>>8);
        pusOut[1] = (uint8_t)iTmpLen;
        pusOut   += 2;
    }

    switch( pAttr->eElementAttr ){
    case Attr_n:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen= (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                if( (pusIn[iInLen-j-1]&0x0F)<0x0A ){
                    pusOut[pAttr->uiLength-i-1] = (pusIn[iInLen-j-1]&0x0F)|0x30;
                }else{
                    pusOut[pAttr->uiLength-i-1] =
                        (pusIn[iInLen-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[iInLen-j-1]>>4)<0x0A ){
                        pusOut[pAttr->uiLength-i-2] =
                           (pusIn[iInLen-j-1]>>4)|0x30;
                    }else{
                        pusOut[pAttr->uiLength-i-2]=
                           (pusIn[iInLen-j-1]>>4)-0x0A+'A';
                    }
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

            for(i=0; i<(iTmpLen/2); i++){
                if( (pusIn[i]>>4)<0x0A ){
                    pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) -0x0A + 'A';
                }

                if( (pusIn[i]&0x0F)<0x0A ){
                     pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
                }else{
                     pusOut[2*i+1] = (pusIn[i]&0x0F) - 0x0A + 'A';
                }
            }

            if( iTmpLen%2 ){
                if( (pusIn[i]>>4)<0x0A ){
                     pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) - 0x0A + 'A';
                }
            }

            break;
        }   
        break;

    case Attr_z:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen = (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                pusOut[pAttr->uiLength-i-1]= (pusIn[iInLen-j-1]&0x0F)|0x30;
                if( i!=pAttr->uiLength-1 ){
                     pusOut[pAttr->uiLength-i-2]= (pusIn[iInLen-j-1]>>4)|0x30;
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

            for(i=0; i<(iTmpLen/2); i++){
                pusOut[2*i]   = (pusIn[i]>>4) | 0x30;
                pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
            }
            if( iTmpLen%2 ){
                pusOut[2*i]= (pusIn[i]>>4) | 0x30;
            }
            break;
        }   
        break;

    case Attr_b:
        memcpy(pusOut, pusIn, iTmpLen);
        break;

    case Attr_a:
        memcpy(pusOut, pusIn, iTmpLen);
        break;
    }    

    *puiInLen = iInLen;

    return 0;
}

// end of file
