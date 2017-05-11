
#ifndef _NEWPOS_FUNCLIST_H
#define _NEWPOS_FUNCLIST_H

#include "posapi.h"
#include "posappapi.h"
#include "emvapi.h"
#include "emvtranproc.h"

uint8_t AdjustConfirmTipAmount(void);
uint8_t  CashAdvacne(void);
void Manage_Pwd(char *pPwdInput);
void  InitPos(int iRet);
void  DispResult(uint8_t rej);
void  InitCommData(void);
void  AppFirstRun(void);
uint8_t  CheckLimitTotal(void);
void  InitAllTransData(void);
void  SetParaFunc(void);
void  DispMainScreen(void);
void  OnlineDownOrUp(void);
void  CommHangUp(uint8_t bReleaseAll);
void  MakeReversalPack(void);
void  SaveRspPackData(void);
void  IncreaseTraceNo(void);
void  UpdateParameters(uint8_t *psPosParam, uint16_t iMaxLen);
void  MagTest(int ShowFlag);
void  PabxSetup(uint8_t ucMode);
void  PosDeviceDetect(void);
void  AfterSettleTran(void);
int KeyboardTest(void);
int LcdTest(int flag);
uint8_t CheckSupport(int iTransNo);
int CheckParamPass(void);
int CheckManagePass(void);
int TermCheck(void);
uint8_t DispTermInfo(void);
uint8_t ProcEvent(void);
uint8_t ProcSelectTran(void);
uint8_t OnlineCommProc(void);
uint8_t SendPacket(uint8_t *sSendData, uint16_t PacketLen);
uint8_t RecvPacket(uint8_t *psPacket, uint16_t *usPacketLen, uint8_t psWaitTime);
uint8_t SendRecvPacket(void);
int ConnectHost(void);
uint8_t SaveReversalTranData(void);
uint8_t ReadReversalTranData(void);
uint8_t ReverseTranProc(void);
uint8_t UpdateReversalFile(uint8_t Err);
int MakeReqPacket(void);
int ProcRspPacket(void);
uint8_t TranNeedReversal(int iTranType);
uint8_t TranNeedIncTraceNo(int iTranType);
uint8_t HaveMacData(int iTranType);
uint8_t TrickleFeedOffLineTxn(uint8_t ucAllowBreak);
uint8_t TrickleFeedICOffLineTxn(uint8_t ucAllowBreak);

uint8_t ClpbocGetCard(void);
uint8_t PosOnlineLogon(uint8_t flag);
uint8_t PosOnlineLogoff(uint8_t flag);
uint8_t PosSettleTran(void);
uint8_t PosBatchUp(uint8_t flag);
uint8_t SelectCardUnit(uint8_t *szCardUnit);
uint8_t BatchSendOffTrans1(int *sendLen, uint8_t flag);
uint8_t BatchSendOffTrans2_Refund(int *sendLen, uint8_t flag);
uint8_t BatchSendOffTrans2_CM(int *sendLen, uint8_t flag);

uint8_t SendOffLineTrans(uint8_t flag, int maxSendTimes);
uint8_t PosBatchSendTran(void);
uint8_t GetOldTraceNo(void);
uint8_t GetSysRefNo(void);
uint8_t GetAuthNo(void);
uint8_t GetOrignTxnDate(void);
uint8_t GetTxnTableIndex(int txn);
uint8_t GetAuthType(void);
uint8_t NeedPIN(int iTxnNo);
uint8_t ICNeedPIN(int iTxnNo);
uint8_t PreAuthAddInput(void);
uint8_t GetUpdateInfo(void);
uint8_t CheckAdjustAmt(uint8_t *sAmount, uint8_t *sOldAmount, uint8_t ucPercent);

uint8_t Electronic_wallet_online(void);
uint8_t OnlineSale(void);
uint8_t VoidSaleAuth(void);
uint8_t PreAuth(void);
uint8_t VoidPreAuth(void);
uint8_t RefundTran(void);
uint8_t PreAuthAdd(void);
uint8_t OnlineAuthConfirm(void);
uint8_t OfflineAuthConfirm(void);
uint8_t QueryBalance(void);
uint8_t OfflineSale(void);
uint8_t OfflineAdjust(void);
uint8_t TestResponse(void);
uint8_t AfterCommProc(void);
void  SetCommReqField(uint8_t *pszMsgCode, uint8_t *pszProcCode);
uint8_t DownLoadCAPK(uint8_t ucNeedBak);
uint8_t DownLoadEmvParam(uint8_t ucNeedBak);
uint8_t QueryEmvParam(void);
uint8_t UnpackTlvData(SaveTLVData pfSaveData);
void  SaveOneAid(uint16_t uiTag, uint8_t *psData, int iDataLen);
void  SaveAppParamItem(uint16_t uiTag, uint8_t *psData, int iDataLen);
uint8_t SaveEmvAppParam(TermAidList *pstTermAidList);
uint8_t FinishDownLoad(int iTranNo);
uint8_t QueryEmvCapk(void);
void  SaveCapkInfoItem(uint16_t uiTag, uint8_t *psData, int iDataLen);
uint8_t UpdateCoreCapkInfo(void);
int SearchCapkInfo(uint8_t *psRid, uint8_t ucKeyID);
void  SaveEmvCapkItem(uint16_t uiTag, uint8_t *psData, int iDataLen);
uint8_t SaveNewEmvCapk(struct issuer_ca_public_key *pstNewCapkList);
uint8_t SendICCOffLineTrans(uint8_t flag, int maxSendTimes);
uint8_t BatchMagOnLineTrans(int *piTotalNum, uint8_t ucSendFalg);
uint8_t BatchSendICCOnLineTrans(int *piTotalNum, uint8_t ucSendFlag);
uint8_t BatchICCOffLineTrans(int *piTotalNum, uint8_t ucSendFlag);
void  UpdateEntryModeByPIN(void);

uint8_t TransBatchUpLoadFailureTxn(int *piTotalNum, uint8_t ucSendFlag);
uint8_t TransBatchUpLoadIccFailureTxn(int *piTotalNum, uint8_t ucSendFlag);
uint8_t TransBatchUpLoadIccAACTxn(int *piTotalNum, uint8_t ucSendFlag);

uint8_t SaveLogFile(void);
uint8_t SaveCheckLogFile(void);
uint8_t CheckData(int lPosStan, NEWPOS_LOG_STRC *pstOutLog);
uint8_t UpdateLogFile(void);
uint8_t   UpdateCheckLogFile(void);
uint8_t ComputeTransTotal(int flag);
uint8_t queryDetail(int flag);
uint8_t reprintTrans(uint8_t prt_bz);
uint8_t reprintCheckTrans(uint8_t prt_bz,int lTraceNo);
uint8_t RestaurantPrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t queryTrans(void);
uint8_t QueryTransTotal(void);
void  update_limit_total(void);
void  DispOneTrans(NEWPOS_LOG_STRC *pstLog);
void  GetTransStatusInfo(NEWPOS_LOG_STRC *pstLog,char *pszStatus);

int  emvSaveAIDList(int iNo, const struct terminal_aid_info *info);
int  emvLoadAIDList(int iNo, struct terminal_aid_info *info);
int  emvSaveCAPKList(int iNo, const struct issuer_ca_public_key *info);
int  emvLoadCAPKList(int iNo, struct issuer_ca_public_key *info);


uint8_t SaveCtrlParam(void);
uint8_t ReadCtrlParam(void);
uint8_t ReadOperFile(void);
uint8_t WriteOperFile(void);
void  DispFileErrInfo(void);
uint8_t SaveLimitTotal(void);
uint8_t ReadLimitTotal(void);
uint8_t SaveLastTotal(void);
uint8_t ReadLastTotal(void);
uint8_t read_data(void* pdata, int size, char* filename);
uint8_t write_data(void* pdata, int size, char* filename);
uint8_t SaveRefundDataFile(void);
uint8_t CheckRefundDataFile(int lTrace);
uint8_t ReadTermAidList(void);
uint8_t WriteScriptFile(void);
uint8_t ReadScriptFile(void);
uint8_t LoadTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog);
uint8_t LoadCheckTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog);
uint8_t UpdateTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog);
uint8_t UpdateCheckTranLog(int iRecNo, NEWPOS_LOG_STRC *pstTranLog);
uint8_t SaveUplTraceSync(void);
uint8_t ReadUplTraceSync(void);

uint8_t TellerLogon(uint8_t flag,uint8_t ucOnline);
void  TellerAdd(void);
void  TellerDel(void);
void  TellerShow(void);
void  TellerChgPwd(void);
void  TellerManageMenu(void);

void  TellerRunLock(void);
uint8_t CheckSupPwd(uint8_t ucFlag);
uint8_t CheckTellerPwd(void);
void  SupTellerFunc(void);
void  SupChgPwd(void);

int EnterPIN(uint8_t ucFlag);
uint8_t DispBalance(void);
uint8_t CalMAC(uint8_t * inbuf, int len, uint8_t * outbuf);
uint8_t ExtractPAN(uint8_t* cardno, uint8_t* pan);
uint8_t LogonDivKey(uint8_t *pKeyData,uint8_t ucDispErrInFO);
uint8_t LogonDivKey_EPP(uint8_t *pKeyData,uint8_t ucDispErrInFO);
uint8_t LogonDivKey_PED(uint8_t *pKeyData,uint8_t ucDispErrInFO);

uint8_t PrintData(void);
uint8_t PrtTranDetailHead(void);
uint8_t PrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t PrintAllErrOffTrans(void);
uint8_t PrtTransTotal(uint8_t flag, int batchNo);
uint8_t PrtNowTotal(void);
void  DispPrnErrInfo(uint8_t err);
void  PrtTranTicket(int flag);
void  MakeFormatCardNo(const char *card, char *fmtCard);
void  GetTransPrtName(int iTransType, char *pszTransName);
void  PrintAmount(void);
void  PrintAddtionalInfo(void);
uint8_t GetEntryMode(void);
void  PrnEmvParam(void);
void SetupTranFunc(void);

void  SetupPosPara(void);
void  SetupTerminalPara(void);

void  SetTpduOutTime(void);
void  SetTestStatus(void);
void SetSupportPinpad(void);
void  SetMasterKeys(void);
int   SetTranResendTime(void);
void  SetTransFlow(void);
void  SetTransMask(void);
int  TipperSetup(void);
void ChangePwdFunc(void);

int   SetTranMaxNumber(void);
void  TestPrint(void);
uint8_t MaskTrans(int iTransId, int iMaskBit, char *pFuncName, uint8_t ucInKey);
int  SetPrintTicketNum(void);
void SetErrPrnTicketNum(void);
void  SetMaxOfflineNums(void);

void  PrintParam(uint8_t prtFlag);
void  DeviceTest(void);
void  ChangeSysPwd(void);
void  OtherFunc(void);
void ChangeSafePwd(void);
void ClearFile(void);
void ClearReversal(void);
void SetEMVQPBOC(void);

int  SetCommParam(int iMode);
void  init_dial_statics(void);
uint8_t calculate_dial_statics(void);

uint8_t DetectCardEvent(uint8_t mode,uint8_t *pDispInfo1,uint8_t *pDispInfo2,int *ierrno);
uint8_t DispCardNo(uint8_t *pszCardNo);
uint8_t GetCardNoFromTrack(uint8_t *szCardNo, uint8_t *track2, uint8_t *track3);
uint8_t GetTrackData(uint8_t *pszTrack2, uint8_t *pszTrack3, uint8_t *pszCardNo);
uint8_t PosGetCard(uint8_t ucMode);
uint8_t ManualEntryExpDate(uint8_t *pszExpDate);
uint8_t ManualEntryCardNo(uint8_t *pszCardNo,uint8_t ucMode);
uint8_t IsEmvCard(uint8_t *pszTrack2Data);
void  DispRemoveICC(void);
void  WaitRemoveICC(void);
void  DispICCProcessing(void);
void  MaskPan(uint8_t *pszInPan, uint8_t *pszOutPan);

int   SelectSetTwo(uint8_t *buf);

void ShowGetCardScreen(uint8_t ucMode,uint8_t *pDispBuf1,uint8_t *pDispBuf2);
void PrnBigFontChnData(void);
void PrnSmallFontData(void);

int  WaitEvent(void);
void  GetPosTime(uint8_t *pszDate, uint8_t *pszTime);
void  dispRetCode(uint8_t ucFlag);
int SelectTrans(void);
uint8_t SelectPrintFunc(void);
void  ConvErrCode(uint8_t errCode, char *szErrInfo);
uint8_t AppGetAmount(int length, uint8_t flag);
void  GetTransName(int tCode, uint8_t *szName);
uint8_t SelectAuthFunc(void);
uint8_t write_stat_time(void);
uint8_t AmountIsZero(uint8_t *sBcdAmt, uint8_t ucAmtLen);
uint8_t SelectPosManFunc(void);
uint8_t SelectKeyManageFunc(void);
uint8_t AppConfirmAmount(void);

uint8_t SwipeCardProc(uint8_t bCheckICC);
uint8_t InsertCardProc(uint8_t bShowDefTranName);
uint8_t Direct_InsertCardProc(uint8_t bShowDefTranName);

int GetEmvTrackData(void);
void  RemoveTailChars(uint8_t *pszString, uint8_t ucRemove);
uint8_t MatchTrack2AndPan(uint8_t *pszTrack2, uint8_t *pszPan);
void  ModifyTermCapForApp(void);
uint8_t PosDownLoadParam(void);
uint8_t PosUpLoadParam(void);
int MakeTermStatusData(uint8_t *pStatus);
void  DispTransName(void);
void  SetEmvTranType(void);
void  DispMulLanguageString(int Col, int Line, uint32_t Mode, char *pstChBuf, char *pstEnBuf);
uint8_t ContackCardProc(void); // 非接触式卡处理
void GetDateTime(uint8_t *pszDateTime);


void    ViewEmvResult(void);
void    vTwo2One(uint8_t *psIn, uint16_t nLength, uint8_t *psOut);
void    vOne2Two0(uint8_t *psIn, uint16_t nLength, uint8_t *psOut);
void    vOne2Two(uint8_t *psIn, uint16_t nLength, uint8_t *psOut);
void    vLong2Bcd(uint32_t ulSource, uint16_t uiTCnt, uint8_t *psTarget);
uint16_t  uiPortRecvBuf(uint8_t ucPort, uint8_t *psBuff, uint16_t nLength, uint32_t nTimeOut);
void    vNacSend(uint8_t ucPort, uint8_t *psBuff, uint16_t nLength);
uint16_t  uiNacRecv(uint8_t ucPort, uint8_t *psBuff, uint32_t *puiLen, uint32_t nTimeOut);


int  AdjustCommParam(uint8_t ucCheckWire);
void   AdjustAppParam(void);
void   LoadDefaultAppParam(void);
int    SaveAppParam(void);
int    LoadAppParam(void);
void   ClearAllTxnStatus(void);
void   SetTxnCtrlStatus(POS_COM *pstTxnLog);

void   UpdataTxnCtrlStatus(uint32_t uiTxnIndex,uint32_t uiStatus);
int    GetOfflineTxnNums(uint32_t uiTxnType);
int    GetMinSendTimesTxn(uint32_t uiTxnType);

void  EncrypteFieldTrac23(uint8_t *pszTrack, uint8_t *pszEncrypData);
void  SetCommField60(void);
uint8_t DisplayLoyaltBalance(void);

uint8_t TransDownloadBlkCard(void);
int     SaveBlkCard(ST_BLACKLIST_TOTAL *pstBlkCard);
int     LoadBlkCard(ST_BLACKLIST_TOTAL *pstBlkCard);

int    SetRemoteModemType(void);
int    SetModemAsyncParam(void);

int    iccCardExchangeAPDU(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdata);

int    SetOtherParam(void);
void   DispBalAmt(uint8_t *pBalance1,uint8_t *pBalance2);
int    TransQpbocQueAmt(void);

void   CheckPinPad(void);
int    BootInit(int argc, char *argv[]);
int    CheckConnectPinPad(uint8_t ucFlag);
void   IfSupportPinpad(void);
void   DispEppPedErrMsg(int rej);

void   AdjustMenuContext(const char *pszInStr, char *pszOutStr);
void   ShowICCardScreen(uint8_t ucMode,uint8_t *pDispBuf1,uint8_t *pDispBuf2);
int    TransBatchUploadQpbocFailureTxnLog(int *piTotalNum, uint8_t ucSendFlag);
void   SetQuitTimeout(void);
void   PrnBlackEngData(void);

void   SetSettleSwitch(void);
void   SetOfflineSwitch(void);
void   SetVoidSwipe(void);
void   SetVoidPin(void);
void   SetOtherSwitch(void);
int    SetOfflineSendTime(void);
void   SetupSysPara(void);
void   SetClear(void);
void   newprnLogo(int x, int y, int w, int h, uint8_t *filename);
int    Init_timer(void);
int    start_timer(time_t sec, suseconds_t usec);
int    stop_timer(void);
void   DispRemainSpace(void);
uint8_t ContactlessPBOCProc(void);
void   SetElecSignReqField(int iTransNo);
uint8_t SendOffLineElecSignTrans(uint8_t flag, uint8_t szSettleBatch);
uint8_t SendCheckOffLineElecSignTrans(uint8_t flag, uint8_t szSettleBatch);
uint8_t TrickleFeedElecSignTxn(uint8_t ucAllowBreak,uint8_t szSettleBatch);
uint8_t PrintAllErrElecSignTrans(void);
int jbg_encode_buffer(uint8_t *szPName,uint8_t *psImageBuffer,int *piDataLen);
int    GetElecSignFailTxnNums(void);
int    GetElecSignTxnNums(void);
int    GetCheckElecSignTxnNums(void);
void   ClearElecSignaturefile(void);
void   SetElecSignSwitch(void);
int    CheckElecSignFailTxn(void);
void  ClearAllTxnUploadNum(void);
int sysSetbacklighttime(int keep);
int sysGetbacklighttime(int *keep);
uint8_t SelectPBOCTrans(void);
int   CheckWirelessAttached(uint32_t uiTimeOut);
int   ExitApplication(void);
void  AfterProcQpboc(void);
void _root_beep(unsigned int frequency, unsigned int keepms);
void AppMagOpen(void);
void AppMagClose(void);

void ledSetStatus(int status);
void ClearAllTranLog(void);
void CleaCheckInTranLog(void);
uint8_t PrintSettleBatch(void);

void ssl_initialize(void);
void GetBaseStationInfo(uint8_t *szStationInfo);
uint8_t BalanceCheck(void);
uint8_t NetpayDealFeild63(uint8_t *szField63Buff);
uint8_t LastMovement(void);
uint8_t PaymentwithBalance(void);
uint8_t PaymentwithCard(void);
uint8_t SelectSaleType(void);
uint8_t SelectInteresesSaleType(void);
uint8_t ProcBalanceCheck(char* H1,char* H2);
uint8_t PayWithCard(void);
uint8_t ProcPayWithCardBalanceCheck(char* C0);
uint8_t PayWithCardBalanceCheck(void);
uint8_t PrintBalanceCheck(char *T0,char *T1,char *T2,char *T3,char *T4,char *T5,char *T6);
uint8_t CheckAllBalance(void);
uint8_t ProcCheckAllBanlance(char* T0,char* T1,char* T2,char* T3,char* T4,char* T5,char* T6);
uint8_t SaveLogonMsg(uint8_t *szField63);
void NetpayPrtTranTicket(int flag);
void NetpayPrtBalance(void);

uint8_t ProcTheLastMent(uint8_t *field49,uint8_t *field59,uint8_t *field60,uint8_t *field61,uint8_t *field63);
uint8_t NetpayPaywithCardDealFeild63(uint8_t *szField63Buff);
uint8_t NetpayPhoneTopup(void);
uint8_t InputPhoneNo(int flag,char *szPhoneNo);
uint8_t EchoTest(void);
uint8_t NetpayDealFeild120(uint8_t *szField120Buff,int RI);
uint8_t SelectTipPercent(uint8_t *BcdNormalAmount);
uint8_t SelectTipPercent(uint8_t *BcdNormalAmount);
uint8_t ConfirmTipAmount(void);
uint8_t FindTheLastSale(NEWPOS_LOG_STRC *stLog);
uint8_t CheckFindTheLastSale(NEWPOS_LOG_STRC *stLog);
uint8_t CalTotalTransNo(NEWPOS_LOG_STRC *pstLog);
uint8_t PrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
void  NetpayPrintAmount(int flag);
uint8_t InputWaiterNo(void);
uint8_t ComfirAmount(uint8_t *szTransAmount);
void ProcComField63(uint8_t *field63,uint8_t *szMsgCode);
uint8_t NetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t PrintFaildTicket(int flag,uint8_t *RSP_CODE,uint8_t *szErrorReason);
void ProcFaildTransField63(uint8_t *field63);
uint8_t EnterSeguridadCode(void);
uint8_t AuthConfirmCheckData(int lPosStan, NEWPOS_LOG_STRC *pstOutLog);
void   RemoteDownLoad(void);
int TransInit(uint8_t ucTranType);
uint CheckTransValid(void);
void SetLogonOff(void);
//void RemoteDownloadMenu(void);
uint8_t QueryPostAuthIfFinish(void);
void DispBalanceCheck(char *T0,char *T1,char *T2,char *T3,char *T4,char *T5,char *T6);
uint8_t DispLogon(void);
int  DispVenta(void);
int  DispCheck_Balance(void);
uint8_t SelectTransTypeAmount(void);
int tms_process(int update_now);
int read_uicc_mutecard(void);
int query_down_flag(void);
int check_ppp_connected(void);
int  SetRemoteCommParam(void);
int down_now(void);
void WritePubComm(void);
void MakeFormatPrintDate(uint8_t *szDate,uint8_t *PrintDate);
void FormAmountFormmat(uint8_t *BcdAmount,uint8_t *AscAmount);
uint8_t NetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
void NetpaydispRetCode(uint8_t ucFlag);
uint8_t ReadNetpay( char *temp);
uint8_t DealTmsParam(uint8_t *temp);
uint8_t GetCardHolderName(uint8_t *track1);
uint8_t NetpayPrtOneTranTip(NEWPOS_LOG_STRC *pstLog);
uint8_t PrintCtlvolume(uint8_t bPrintAll);
uint8_t PrintTipDetail(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t DealFeild120(uint8_t *szField120Buff);
uint8_t SelectMoneyType(void);
uint8_t CheckIn(void);
uint8_t CheckOut(void);
uint8_t NetpayCheckInPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t CheckOutCheckData(int lPosStan, NEWPOS_LOG_STRC *pstOutLog);
uint8_t QueryPostCheckInIfFinish(void);
uint8_t GetOldReference(void);
uint8_t Netpay_RefundGetData(void);
uint8_t Netpay_RefundTran(void);
uint8_t AdjustTran(void);
uint8_t NetpayAdjustTranGetData(void);
void SetupAppSwitch(void);
uint8_t GetCardType(void);
int    NetpaySelectSetTwo(uint8_t *buf);
uint8_t InputCardNoSale(void);
uint8_t ManualAutorNo(void);
uint8_t SelectOldTrans(void);
uint8_t PrintSelectMoneyType(void);
uint8_t GasstationPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog,int flag);
uint8_t PrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t RetailCalTotalTransNo(NEWPOS_LOG_STRC *pstLog);
uint8_t RetailNetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t RetailPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t HotelPrintDetail(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t HotelNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t HotelPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t HotelNetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t HotelCalTotalTransNo(NEWPOS_LOG_STRC *pstLog);
uint8_t HotelSettleCalTotalTransNo(NEWPOS_LOG_STRC *pstLog);
uint8_t CheckInreprintTrans(uint8_t prt_bz);
uint8_t CheckOutreprintTrans(uint8_t prt_bz);
uint8_t HotelPrintCheckInDetail(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t HotelCheckInNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t HotelPrintCheckOutDetail(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t HotelCheckOutNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t HotelCheckOutSettlePrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t HotelCheckInSettlePrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t RestaurantNetpayPrtOneTranTxn(NEWPOS_LOG_STRC *pstLog);
uint8_t RestaurantNetpayPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);
uint8_t RestaurantCalTotalTransNo(NEWPOS_LOG_STRC *pstLog);
uint8_t RestaurantPrintTransTotal(uint8_t bPrintAll,uint8_t ucTitleFlag);

//gasplatfrom
uint8_t AppGetTip(void);
uint8_t AppGetBomba(void);
uint8_t GetAmount_FromGasPlat(void);
uint8_t GetRules_FromBankPlat(int flag);
uint8_t GetBalance_FromBankPlat(int flag);
uint8_t ComfirmAmount_ToGasPlat(void);

uint8_t AppGetCtlvalue(uint8_t flag);
void GetRules_Field63(uint8_t *field63,uint8_t *szMsgCode);
void GetAuth_Field63(uint8_t *field63,uint8_t tflag);

void GetBalance_Field63(uint8_t *field63,uint8_t *szMsgCode);
int AppInputNip(void);
uint8_t AppInputDrivernum_nip(int flag);
uint8_t Get_Rule_Balance_Feild63(uint8_t *szField63Buff);
uint8_t CheckCardType(void);
void PrtPreAuthTicket(int flag);




#endif

// end of file
