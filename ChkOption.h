
/****************************************************************************
NAME
    checkopt.h 

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    pyming     2011.08.12      - created
****************************************************************************/

#ifndef _CHECKOPT_H
#define _CHECKOPT_H

#include "posapi.h"
#include "posappapi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint8_t ChkIfSaveLog(void);

uint8_t ChkIfUpdateLog(void);

uint8_t ChkIfDebitTxn(int iTransType);

uint8_t ChkIfCreditTxn(int iTransType);

uint8_t ChkIfSaleTxn(int iTransType);

uint8_t ChkIfPrtDetill(int iTransType);

uint8_t ChkIfBlkCard(char *pszCardNo);

uint8_t ChkAcceptTxnCode(char *pszRespcode);

uint8_t ChkOnlyEcCard(uint8_t *sAppAid);

uint8_t ChkExTxnLog(void);

uint8_t ChkIfElecSignature(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _CHECKOPT_H

// end of file
