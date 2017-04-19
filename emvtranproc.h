
#ifndef _NEWPOS_EMVPROC_H
#define _NEWPOS_EMVPROC_H

#define TAGMASK_CLASS			0xC0	
#define TAGMASK_CONSTRUCTED		0x20	
#define TAGMASK_FIRSTBYTE		0x1F	
#define TAGMASK_NEXTBYTE		0x80	

#define LENMASK_NEXTBYTE		0x80	
#define LENMASK_LENBYTES		0x7F	

#define NULL_TAG_1				0x00	
#define NULL_TAG_2				0xFF	

#define EMV_RET_SUCCESS  0
#define ONLINE_APPROVE   0

typedef void (*SaveTLVData)(uint16_t uiTag, uint8_t *psData, int iDataLen);


#define DE55_MUST_SET	1	
#define DE55_OPT_SET	2	
#define DE55_COND_SET	3	
// DE55 tag list
typedef struct _tagDefield55EmvTag
{
	uint16_t	uiTagNum;
	uint8_t	ucTagOpt;
}Def55EmvTag;

typedef struct _tagEmvScriptInfo
{
	uint16_t	uiTag;
	int	iIDLen;
	uint8_t	sScriptID[4];
	int	iCmdLen[20];
	uint8_t	sScriptCmd[20][300];
}EmvScriptInfo;


typedef struct _tagST_EMVPRINT_INFO
{
	uint8_t  sEcIssureAuthCode[6];   // 9F73£¨EC Issuer Authorization Code£©
	uint8_t  sTerminalRand[4];       // 9F37 random
	uint8_t  sAip[2];                // 82 
	uint8_t  ucIssureAppDatalen;     // 
	uint8_t  sIssureAppData[32];     // 9F10 
	uint8_t  sTerminalCapab[3];       // 9F33 
	uint8_t  sAppAtc[2];             // 9F36
	uint8_t  sTVR[5];                //95 
	uint8_t  ucTVRoK;
	uint8_t  sTIDSn[8];              // 9F1E½
	uint8_t  ucTIDSnoK;
	uint8_t  sScritptRet[21];        // DF31
	uint8_t  ucScritptRetlen;        // DF31·
	uint8_t  sARQC[8];               // 9F26 
	uint8_t  sCVMR[3];				//9F34 CVM
}ST_EMVPRINT_INFO;


uint8_t ProcEmvTran(void);
uint8_t SetOnlineTranDE55(Def55EmvTag *pstList, uint8_t *psTLVData, int *piDataLen);
int GetTlvValue(uint8_t **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uint8_t bExpandAll);
int IsConstructedTag(uint16_t uiTag);
void  Char2Long(uint8_t *psSource, uint16_t uiSCnt, uint32_t *pulTarget);
void  SaveRspICCData(uint16_t uiTag, uint8_t *psData, int iDataLen);
uint8_t SetIssuerName(void);
uint8_t UpdateEmvRevFile(void);
uint8_t UpdateEcRevFile(void);
void  MakeTlvString(uint16_t uiTagNum, uint8_t *psData, int iLength, uint8_t **ppsOutData);
void  SaveEmvTranData(void);
uint8_t SaveScriptResult(void);
uint8_t SendScriptResult(void);
void  AdjustIssuerScript(void);
void  SaveScriptData(uint16_t uiTag, uint8_t *psData, int iDataLen);
void  PackTLVHead(uint16_t uiTag, uint16_t uiTLVDataLen, uint8_t *psOutData, int *piOutLen);
void  PackTLVTag(uint16_t uiTag, uint8_t *psOutData, int *piOutLen);
void  PackTLVLength(uint16_t uiTLVDataLen, uint8_t *psOutData, int *piOutLen);
int CalcTLVTotalLen(uint16_t uiTag, uint16_t uiDataLen);
void  PackScriptData(void);

int emvCheckCRL(uint8_t index,const void *rid,const void *cert_sn);

void  SavePrintICCData(uint16_t uiTag, uint8_t *psData, int iDataLen);

void CheckEcCard(void);
uint8_t EcAppQue(void);
uint8_t UpdateARPCEmvRevFile(void);
int     JudgeEcCard(void);

void  GetARPCTag91(char *pszData, int *iDatalen);

extern ST_EMVPRINT_INFO gstIccPrintData;

#endif


// end of file
