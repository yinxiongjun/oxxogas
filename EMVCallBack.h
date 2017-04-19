#ifndef EMVCALLBACK_H_
#define EMVCALLBACK_H_

void  emv_multi_language_selection(void);
int emvInputTransAmount(uint32_t *pTransAmount);
int emvInputCashBackAmount(uint32_t *pCashbackAmount);
int emv_candidate_apps_selection(void);
int emvGetOnlinePIN(void);
int emv_check_online_pin(void);
int emv_check_certificate(void);
int emv_online_transaction_process(uint8_t *RspCode,
                           uint8_t *AuthCode, int *AuthCodeLen,
                           uint8_t  *IAuthData,int *IAuthDataLen,
                           uint8_t *script, int *ScriptLen, int *online_result);
int emv_issuer_referral_process(void);
int emv_advice_process(int Flg);
int emv_get_pin(void *CardPin);
int emv_return_pin_verify_result(uint8_t TryCounter);
int emv_get_amount(uint32_t *pTransAmount, uint32_t *pOtherAmount);
int emvCheckExceptPAN(uint8_t PANLen, const void *PAN, uint8_t PANSn);
int emvGetTransLogAmount(uint8_t PanLen, const void *PAN, uint8_t PanSn);

#endif

