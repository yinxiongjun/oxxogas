#ifndef _KEY_DOWNLOAD_H
#define _KEY_DOWNLOAD_H


#include "posmacro.h"
#include "tranfunclist.h"
#include "showDebug/showDebug.h"

/*! \enum KeyInjectExitCode
    \brief Enumeration constant values to specify exit codes returned
*/
typedef enum
{
    KEY_INJECT_SUCCESS = 0,
    KEY_INJECT_GETCARD_ERR = -1,
    KEY_INJECT_SECURE_CHANNEL_ERR = -2,
    KEY_INJECT_GET_APPKEY_ERR = -3,
    KEY_INJECT_KCV_ERR = -4,
    KEY_INJECT_INIT_AUTH_ERR = -5,
    KEY_INJECT_AUTH_ERR = -6,
    KEY_INJECT_INPUTPIN_ERR = -7,
    KEY_INJECT_VERIFY_PIN_ERR = -8,
    KEY_INJECT_REQ_SESSTION_ERR = -9,
    KEY_INJECT_SEND_REQ_ERR = -10,
    KEY_INJECT_INPUTPARA_ERR = -99,
}KeyInjectExitCode;



int MutualAuth(uint8_t *AcqID,uint8_t *VenderID,uint8_t *Apk);
int InjectRKIKey(void);



#endif
