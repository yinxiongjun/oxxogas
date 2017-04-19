#include "glbvar.h"
#include "keydownload.h"
#include "openssl/sha.h"
int  	GetRandom(void *random,int length)
{
	unsigned char *pBuf;
	size_t st;

	pBuf = (unsigned char *)random;
	srand((unsigned)time(NULL));

	for(st=0; st<length; st++)
	{
		*pBuf++ = (unsigned char)(rand()%256);
	}

	return OK;
}

uint8_t PosGetRKICard(uint8_t ucMode)
{
	uint8_t	ucRet,sDispBuf1[30],sDispBuf2[30];
	UINT32 AtrLen = 0;
	UINT8 AtrData[500];
	int ierrno;
	lcdCls();
	while(1)
	{
		ShowGetCardScreen(ucMode,sDispBuf1,sDispBuf2);
		ucRet = DetectCardEvent(ucMode,sDispBuf1,sDispBuf2,&ierrno);
		if( ucRet==CARD_KEYIN )
		{
			if( kbGetKey()==KEY_CANCEL )
			{
				return E_TRANS_CANCEL;
			}
		}
		else if( ucRet==CARD_INSERTED )
		{
			break;
		}
	}
	iccSetPara(ICC_USERCARD,ICC_VOL_5V,ICC_EMV);
	ucRet = iccPowerUp(ICC_USERCARD, &AtrLen, AtrData);
	if( ucRet )
	{
		lcdClrLine(2,7);
		lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"READ CARD ERR");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return -1;
	}
	return OK;
}

int SelectAid(uint8_t *data,int len)
{
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;

	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0xA4;
	stSendApdu.P1 =  0x04;
	stSendApdu.P2 =  0x0C;
	stSendApdu.Lc =  0x07;
	memcpy(stSendApdu.DataIn, "\xA0\x00\x00\x47\x53\x05\x01",7);//data,len); //
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	return OK;
}
int GetAppletInfo(void)
{
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x12;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Le =  0x00;
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}

	return OK;
}
int VerifyPin(uint8_t *pin,int length,const unsigned char *pTsk)
{
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
    unsigned char szPadData[17] = {0};
    unsigned char ePinValue[9] = {0};
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));

    if (length != 6)
        return KEY_INJECT_VERIFY_PIN_ERR;
    memcpy(szPadData,pin,length);
    memcpy(&szPadData[length],"\x80\x00",2);
    length += 2;
    PubDes(TRI_ENCRYPT, (uint8_t *)szPadData, (uint8_t *)pTsk, (uint8_t *)ePinValue);
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x20;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Lc =  length;
	memcpy(stSendApdu.DataIn, ePinValue,length);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		if(stRespApdu.SWA ==0x65)
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"Remain Tries %02x",stRespApdu.SWB);
		else
			lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return KEY_INJECT_VERIFY_PIN_ERR;
	}
	return OK;
}
int ChangePin(uint8_t *newpin,int length)
{

	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x80;
	stSendApdu.INS = 0x30;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Lc =  length;
	memcpy(stSendApdu.DataIn, newpin,length);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	return OK;
}

int EstablishSecureChannel(uint8_t * Data,int length,uint8_t*sTsk,uint8_t* kcv )
{
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x45;
	stSendApdu.P1 =  0x00;
    /*
    * algorithm1 used in personalization process
    * algorithm2 used in operation
    */
	stSendApdu.P2 =  0x01;
	stSendApdu.Lc =  0x10;
	memcpy(stSendApdu.DataIn, Data,length);
	//memcpy(stSendApdu.DataIn,"\x01\x02\x03\x04\x05\x06\x07\x08\xFF\x2D\xB2\xCB\x4C\xB4\x7D\xE1",16);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAILxxx");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	memcpy(sTsk,stRespApdu.DataOut,16);
	memcpy(kcv,stRespApdu.DataOut+16,2);
	return OK;
}

int GetAppkey(uint8_t *AppKey,uint8_t * Kcv )
{

	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x46;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;	//APP KEY 0x00
	stSendApdu.Le =  0x1A;
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	memcpy(AppKey,stRespApdu.DataOut+8,16);
	memcpy(Kcv,stRespApdu.DataOut+24,2);
	return OK;
}
int InitMutualAuth(uint8_t * RndA,uint8_t*RndB)
{
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x43;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Lc =  0x08;
	memcpy(stSendApdu.DataIn, RndA,8);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	memcpy(RndB,stRespApdu.DataOut,16);
	return OK;
}
int MutualAuthProcess(uint8_t*RndB)
{

	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x44;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Lc =  0x08;
	memcpy(stSendApdu.DataIn, RndB,8);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	return OK;
}
int RequestSession(const uint8_t*AcqID,const uint8_t*VendorID,uint8_t* SamID,uint8_t*EncSKey,uint8_t* Kcv)
{

	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x41;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Lc =  0x28;
	memcpy(stSendApdu.DataIn, AcqID,20);
	memcpy(stSendApdu.DataIn+20, VendorID,20);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return KEY_INJECT_REQ_SESSTION_ERR;
	}
	memcpy(SamID,stRespApdu.DataOut,20);
	memcpy(EncSKey,stRespApdu.DataOut+20,16);
	memcpy(Kcv,stRespApdu.DataOut+36,2);
	return OK;
}

/*
* This function authenticates response from RKI host,and return the key
* information to terminal in encrypted with TSK that previous generated
* in mutual auth
* input data: encryted response message from host
*/
int AuthSession(const uint8_t *EncResp ,int Length,uint8_t *pKeyInfo)
{
	APDU_SEND stSendApdu;
	APDU_RESP stRespApdu;
	int iRet;
	memset(&stSendApdu,0,sizeof(APDU_SEND));
	memset(&stRespApdu,0,sizeof(APDU_RESP));
	stSendApdu.CLA = 0x00;
	stSendApdu.INS = 0x42;
	stSendApdu.P1 =  0x00;
	stSendApdu.P2 =  0x00;
	stSendApdu.Lc =  Length;
	memcpy(stSendApdu.DataIn+20, EncResp,Length);
	iRet = iccExchangeAPDU(ICC_USERCARD,&stSendApdu,&stRespApdu); //   ICC_USERCARD
	if(iRet != ICC_SUCCESS
		|| stRespApdu.SWA != 0x90
		|| stRespApdu.SWB != 0x00)
	{
		lcdClrLine(2,7);
		lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"TRAN FAIL");
		lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY,"%02x %02x READ ERR",stRespApdu.SWA,stRespApdu.SWB);
		lcdDisplay(0,6,DISP_CFONT|DISP_MEDIACY,"RETIRE TARJETA");
		lcdFlip();
		PubBeepErr();
		PubWaitKey(5);
		return NO_DISP;
	}
	memcpy(pKeyInfo,stRespApdu.DataOut,stRespApdu.LenOut);
	return OK;
}
void CALIAK(uint8_t * AcqID,uint8_t * VenderID,uint8_t *RndX,uint8_t *hashG)
{
	unsigned char blockF[21]={0};//blockF=ADBC XOR RNDX
	int iCnt;
	char acqtemp[20]={0};
	char ventemp[20]={0};
	strcpy(acqtemp,(char*)AcqID);
	strcpy(ventemp,(char*)VenderID);
	for(iCnt=0;iCnt<10;iCnt++){
		blockF[iCnt] = acqtemp[iCnt]^ventemp[iCnt+10];
		blockF[iCnt+10] = acqtemp[iCnt+10]^ventemp[iCnt];
	}
	for(iCnt=0;iCnt<8;iCnt++){
		blockF[iCnt]^=RndX[iCnt];
	}
	SHA1(blockF, 20, (unsigned char*)hashG);
}
int RequestAppKey(uint8_t * AcqID,uint8_t * VenderID ,uint8_t* Apk)
{
	int iRet,Length;
	uint8_t  RndX[9],IAK[21],SendData[64],CryptX[9],Tsk[17];
	uint8_t eKey[17];//the Key encrypted , receive from the card
	uint8_t kcv[3],ckcv[9];
	//GetRandom(RndX,8);
    memcpy(RndX,"\x01\x02\x03\x04\x05\x06\x07\x08",8);
	CALIAK(AcqID,VenderID,RndX,IAK);
	PubDes(TRI_ENCRYPT, RndX, IAK, CryptX);
	SelectAid(NULL,0);
	GetAppletInfo();
	memset(eKey,0,sizeof(eKey));
	memset(kcv,0,sizeof(kcv));
	memset(SendData,0,sizeof(SendData));
	memcpy(SendData,RndX,8);
	memcpy(SendData+8,CryptX,8);
	Length=16;
    /*
    * establish secure channel(rndx + cryptogramx)
    * 16 bytes etsk and 2 bytes kcv
    */
	iRet=EstablishSecureChannel(SendData,16,eKey,kcv);
	if(iRet!=OK)
		return NO_DISP ;
    /*
    * decrypt etsk and validate kcv
    * common known tsk
    */
	PubDes(TRI_DECRYPT,eKey,IAK,Tsk);
	PubXor((char*)&eKey[8],(char*)Tsk,8);
	PubDes(TRI_DECRYPT,eKey+8,IAK,Tsk+8);
	memset(eKey,0,sizeof(eKey));
	PubDes(TRI_ENCRYPT,eKey,Tsk,ckcv);
	if(memcmp(ckcv,kcv,2)!=0){
		return NO_DISP;
	}
    /*
    * get application key from card
    * this application key encrypted by tsk
    * the application key also used for mutual auth
    */
	GetAppkey(eKey,kcv);
	PubDes(TRI_DECRYPT,eKey,Tsk,Apk);
	PubXor((char*)&eKey[8],(char*)Apk,8);
	PubDes(TRI_DECRYPT,eKey+8,Tsk,Apk+8);
	memset(eKey,0,sizeof(eKey));
	PubDes(TRI_ENCRYPT,eKey,Apk,ckcv);
	if(memcmp(ckcv,kcv,2)!=0){
		return NO_DISP;
	}
	return OK;
}
int MutualAuth(uint8_t *AcqID,uint8_t *VenderID,uint8_t *Apk)
{
	int iRet;
	uint8_t RndA[9]={0},eRndA[9]={0},RndB[17]={0},eRndB[17]={0};
    /*
    * insert smart card
    */
	iRet=PosGetRKICard(CARD_INSERTED);
	if(iRet!=OK)
		return NO_DISP ;
    /*
    * select AID, get card applet information
    * establish secure channel and get application key
    */
	iRet=RequestAppKey(AcqID,VenderID,Apk);
	if(iRet!=OK)
		return iRet;
    /*
    * init mutual authentication
    */
	GetRandom(RndA,8);
	PubDes(TRI_ENCRYPT,RndA,Apk,eRndA);
	iRet=InitMutualAuth(eRndA,eRndB);
	PubDes(TRI_DECRYPT,eRndB,Apk,RndB);
	PubXor((char*)&eRndB[8],(char*)RndB,8);
	PubDes(TRI_DECRYPT,eRndB+8,Apk,RndB+8);
    /*
    * clear RndA1B2B1A2, ensure it has the same value
    * as random generated in earlier state
    */
	if(memcmp(RndB,RndA,4)!=0||memcmp(RndB+12,RndA+4,4)!=0){
        /*
        * terminal should decline the key injection process
        */
		return NO_DISP;
	}
	memset(RndA,0,sizeof(RndA));
	memset(eRndA,0,sizeof(eRndA));
	memcpy(RndA,RndB+8,4);
	memcpy(RndA+4,RndB+4,4);
	PubDes(TRI_ENCRYPT,RndA,Apk,eRndA);
    /*
    * complete mutual authentication
    */
	iRet=MutualAuthProcess(eRndA);
	if(iRet!=0)
		return NO_DISP;
	return OK;
}
/*
*Smart card setting
*Acq ID:         UOB
*Acq Name:       UOB
*Vendor ID:      SinoPay
*SmartAcct ID:   00413
*SmartAcct Desc: SinoPay
*Duration:       00:01:00 - 23:59:59
*User Name:      01
*First-time PIN: 594371
*/


static KeyInjectExitCode EstablishSecureChannel_Ex(const uint8_t * AcqID,const uint8_t * VenderID ,uint8_t* pTsk)
{
	int iRet,Length;
	uint8_t  RndX[9],IAK[21],SendData[64],CryptX[9],Tsk[17];
	uint8_t eKey[17];//the Key encrypted , receive from the card
	uint8_t kcv[3],ckcv[9];
	//GetRandom(RndX,8);
    memcpy(RndX,"\x01\x02\x03\x04\x05\x06\x07\x08",8);
	CALIAK(AcqID,VenderID,RndX,IAK);
	PubDes(TRI_ENCRYPT, RndX, IAK, CryptX);
	SelectAid(NULL,0);
	GetAppletInfo();
	memset(eKey,0,sizeof(eKey));
	memset(kcv,0,sizeof(kcv));
	memset(SendData,0,sizeof(SendData));
	memcpy(SendData,RndX,8);
	memcpy(SendData+8,CryptX,8);
	Length=16;
    /*
    * establish secure channel(rndx + cryptogramx)
    * 16 bytes etsk and 2 bytes kcv
    */
	iRet=EstablishSecureChannel(SendData,16,eKey,kcv);
	if(iRet!=OK)
		return NO_DISP ;
    /*
    * decrypt etsk and validate kcv
    * common known tsk
    */
	PubDes(TRI_DECRYPT,eKey,IAK,Tsk);
	PubXor((char*)&eKey[8],(char*)Tsk,8);
	PubDes(TRI_DECRYPT,eKey+8,IAK,Tsk+8);
	memset(eKey,0,sizeof(eKey));
	PubDes(TRI_ENCRYPT,eKey,Tsk,ckcv);
	if(memcmp(ckcv,kcv,2)!=0)
       return NO_DISP;
    memcpy(pTsk,Tsk,16);
    return KEY_INJECT_SUCCESS;
}
static KeyInjectExitCode ReqAppKeyFromCard(unsigned char *pApk,const unsigned char *pTsk)
{
    KeyInjectExitCode rc = KEY_INJECT_SUCCESS;
    unsigned char eKey[17] = {0},Apk[17] = {0};
    unsigned char kcv[3] = {0},ckcv[9] = {0};
    /*
    * get application key from card
    * this application key encrypted by tsk
    * the application key also used for mutual auth
    */
	GetAppkey(eKey,kcv);
	PubDes(TRI_DECRYPT,(uint8_t *)eKey,(uint8_t *)pTsk,(uint8_t *)Apk);
	PubXor((char*)&eKey[8],(char*)Apk,8);
	PubDes(TRI_DECRYPT,(uint8_t *)(eKey+8),(uint8_t *)pTsk,(uint8_t *)(Apk+8));
	memset(eKey,0,sizeof(eKey));
	PubDes(TRI_ENCRYPT,(uint8_t *)eKey,(uint8_t *)Apk,(uint8_t *)ckcv);
	if(memcmp(ckcv,kcv,2)!=0)
		rc = KEY_INJECT_KCV_ERR;
    memcpy(pApk,Apk,16);
	return rc;   
}
static KeyInjectExitCode MutualAuthProc(const unsigned char *pApk)
{
    KeyInjectExitCode rc = KEY_INJECT_SUCCESS;
    unsigned char RndA[9]={0},eRndA[9]={0},RndB[17]={0},eRndB[17]={0};
    int iRet ;
    /*
    * init mutual authentication
    */
	GetRandom(RndA,8);
	PubDes(TRI_ENCRYPT,RndA,(uint8_t *)pApk,eRndA);
	iRet=InitMutualAuth(eRndA,eRndB);
    if (iRet != OK){
        rc = KEY_INJECT_INIT_AUTH_ERR;
        return rc;
    }
	PubDes(TRI_DECRYPT,eRndB,(uint8_t *)pApk,RndB);
	PubXor((char*)&eRndB[8],(char*)RndB,8);
	PubDes(TRI_DECRYPT,eRndB+8,(uint8_t *)pApk,RndB+8);
    /*
    * clear RndA1B2B1A2, ensure it has the same value
    * as random generated in earlier state
    */
	if(memcmp(RndB,RndA,4)!=0 || memcmp(RndB+12,RndA+4,4)!=0){
        /*
        * terminal should decline the key injection process
        */
        rc = KEY_INJECT_INIT_AUTH_ERR;
		return rc;
	}
	memset(RndA,0,sizeof(RndA));
	memset(eRndA,0,sizeof(eRndA));
	memcpy(RndA,RndB+8,4);
	memcpy(RndA+4,RndB+4,4);
	PubDes(TRI_ENCRYPT,RndA,(uint8_t *)pApk,eRndA);
    /*
    * complete mutual authentication
    */
	iRet=MutualAuthProcess(eRndA);
	if(iRet!=OK)
		rc = KEY_INJECT_AUTH_ERR;
    return rc;
}
static KeyInjectExitCode UserInputPIN(unsigned char *pucPin)
{
    KeyInjectExitCode rc = KEY_INJECT_SUCCESS;
    unsigned char szPwd[12] = {0};
    int iRet;

	lcdClrLine(2,7);
	memset(szPwd, 0, sizeof(szPwd));
	DispMulLanguageString(0, 2, DISP_CFONT, NULL, "PLS INPUT PIN:");
	lcdFlip();
	lcdGoto(56, 6);
	iRet = kbGetString(KB_EN_CRYPTDISP+KB_EN_BIGFONT+KB_EN_NUM+KB_EN_SHIFTLEFT+KB_EN_REVDISP, 6, 6,
		(stPosParam.ucOprtLimitTime*1000), (char *)szPwd);
	if( iRet==6 )
        memcpy(pucPin,szPwd,6);
	else
		rc = KEY_INJECT_INPUTPIN_ERR;
    return rc;
}
static KeyInjectExitCode SendDownRKIMsg(void)
{
    KeyInjectExitCode rc = KEY_INJECT_SUCCESS;
    int ucRet ;
	InitCommData();
	stTemp.iTransNo = RKI_INJECTION;
	SetCommReqField((uint8_t *)"0800", (uint8_t *)"950001");
	ucRet = SendRecvPacket();
	CommHangUp(FALSE);
	DispResult(ucRet);
    if (ucRet != OK)
        rc = KEY_INJECT_SEND_REQ_ERR;
    return rc;
}

/*
*  int ProcRKIKeyInjection(void)
*  RKI key injection with SMART card
*  download RKI DEK&MEK and TLE DEK&MEK
*  为什么第二个文档提到了2个TSK,第一个是创建安全通道后,第二个是相互认证后的
*  the session key TSK is used in the following APDU
*  1.Encrypt PIN data in Verify_PIN
*  2.Encrypt PIN data in Change_PIN
*  3.Decrypt key information in Auth_Sesstion
*/
int ProcRKIKeyInjection(const unsigned char *pucAcqID,const unsigned char *pucVendID)
{
    KeyInjectExitCode rc = KEY_INJECT_SUCCESS;
    unsigned char szTsk[17] = {0},szApk[17] = {0},szPin[7] = {0};
    unsigned char szSAMID[21] = {0},szEncSKey[17] = {0};
    unsigned char szKcv[3] = {0},szKeyInfo[128] = {0},szEncRsp[128] = {0};
    int len = 0;

    if (NULL == pucAcqID || NULL == pucVendID){
        rc = KEY_INJECT_INPUTPARA_ERR;
        goto ERR_EXIT;
    }
    /*
    * a.user insert Smartcard
    */
    rc = PosGetRKICard(CARD_INSERTED);
    if (rc != KEY_INJECT_SUCCESS){
        rc = KEY_INJECT_GETCARD_ERR;
        goto ERR_EXIT;
    }
    /*
    * b.terminal establish secure channel with Smartcard
    */
    rc = EstablishSecureChannel_Ex(pucAcqID,pucVendID,szTsk);
    if (rc != KEY_INJECT_SUCCESS){
        rc = KEY_INJECT_SECURE_CHANNEL_ERR;
        goto ERR_EXIT;
    }
    /*
    * c.terminal requests application key
    */
    rc = ReqAppKeyFromCard(szApk, szTsk);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * d.terminal performs mutual authentication with Smartcard
    */
    rc = MutualAuthProc(szApk);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * e.user enter Smartcard PIN
    */
    rc = UserInputPIN(szPin);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * f.Smartcard verify PIN,PIN value encrypted with TSK
    */
    rc = VerifyPin(szPin, 6,szTsk);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * g.terminal gets Encrypted Payload from Smartcard
    */
    rc = RequestSession(pucAcqID, pucVendID, szSAMID, szEncSKey, szKcv);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * h.terminal send the request message to NetMATRIX RKI Host
    */
    rc = SendDownRKIMsg();
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * 2.parse response message from NetMATRIX RKI Host
    */
    //rc = ParseDownRKIMsg();
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * 3.key authentication, will get the encrypted key with TSK that
    * generated in previous mutual authentication
    */
    rc = AuthSession(szEncRsp, len, szKeyInfo);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
    /*
    * 4. store the clear Key data into tamper-resistant/reactive storage.
    */
    //rc = StoryKeyData(szKeyInfo);
    if (rc != KEY_INJECT_SUCCESS)
        goto ERR_EXIT;
ERR_EXIT:
    /*
    * terminal prompt error message by rc value
    */
    return rc;
}

int InjectRKIKey(void)
{
    int siRet = 0;
    siRet = ProcRKIKeyInjection((unsigned char *)"UOB", (unsigned char *)"SinoPay");		

	return OK;
}
























































