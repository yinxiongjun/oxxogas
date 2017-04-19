
/****************************************************************************
NAME
    Disp.h

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    pyming     2008.04.10      - created
****************************************************************************/

#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "posapi.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void   Display2Strings(char *pszString1, char *pszString2);
void   Display2StringInRect(char *pszString1,  char *pszString2);
void   DrawRect(uint32_t uiBegRow, uint32_t uiBegCol, uint32_t uiEndRow, uint32_t uiEndCol);
void   DisplayProcessing(void);
void DisplayConnectCenter(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _UITL_H

// end of file
