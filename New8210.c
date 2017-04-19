#include "glbvar.h"
#include "tranfunclist.h"
#include "posparams.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef _POS_TYPE_8210
#include "New8210.h"
#include <pthread.h>
#include "wccom.h"
#include <netif.h>


#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>



int    icc_fd;
struct _EPP_CONFIGURE_ *glbEpp;
int  g_epp_Port_Num;

pthread_t PreDialTid=-1,InitCommTid=-1;
static int          sg_Wiresocket = -1; 
static int          sg_Tcpsocket =  -1;
static uint8_t      bCheckPPP     = 1;
static int          CurStatus;


unsigned char Bmp_Display[] = "logo.png";
unsigned char Bmp_Prn[] = "logo_p.png";
unsigned char Bmp_passive[] = "passive.png";
unsigned char Bmp_Display_NoColor[] = "logo_nocolor.png";


static  timer_t timerid; 
static  int TcpConnect(uint8_t ucWireless);


void CheckPinPad(void)
{
	int       uiLen=0, iRet;
	char      szBuffer[20];
	

	PedSetPinInputTimeout(60);
	if(stPosParam.ucSupPinpad==PARAM_OPEN)
	{
		while(1)
		{
			glbEpp = epp_open("/dev/ttyS3", O_RDWR);
			iRet = epp_get_rand(glbEpp,szBuffer,uiLen);
			if( iRet != 0 )
			{
				lcdClrLine(2,7);
				lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "CHECK PINPAD");
				lcdFlip();
				kbFlush();
				if (KEY_CANCEL == kbGetKey())
				{
					return;
				}
			}
			else
				return;
		}
		epp_light(glbEpp, 0);
		epp_set_pin_input_timeout(glbEpp,60000);

	}
	return ;
}

static int check_process(void)//check the same application
{
	DIR	*dir;
	struct dirent *dirent;
	int ret;	
	int pid=0,execpt_pid=0;
	char link_file[1024],link_target[1024], self[1024];	

	execpt_pid = getpid();
	snprintf(link_file, sizeof(link_file)-1, "/proc/%d/exe", execpt_pid);
	ret = readlink(link_file, self, sizeof(self));
	if (ret >= 0)
		self[ret] = '\0';

	if ((dir = opendir("/proc/"))) {
		while ((dirent = readdir(dir))) {
			pid = strtol(dirent->d_name, NULL, 10);//PID
			if (pid < 2 || pid == execpt_pid) {
				pid = 0;
				continue;
			}
				
			snprintf(link_file, sizeof(link_file)-1, "/proc/%s/exe", dirent->d_name);
			ret = readlink(link_file, link_target, sizeof(link_target));
			if (ret >= 0) {	
				link_target[ret] = '\0';
				if( !strcmp(link_target, self) ) {
					break;
				}
			} 
			pid = 0;
		}
	}

	closedir(dir);
	return pid;
}


int BootInit(int argc, char *argv[])
{
	IDirectFBFont *font1;
	DFBColor bgColor={0xF0,0xFF,0xFF,0xFF};
	DFBColor fgColor={0xF0,0x00,0x00,0x00};
	struct kbdset stdbCfg;
	int	   big_font_size = 30,font_size32= 32, small_font_size = 28,font_size36 = 36,NP_font = 20;
	int   ibacklight_set_time_bak,iRet=0;
   	int    iScreenwidth, iScreenheight;

	iRet = check_process();
	if (iRet!=0)
	{
		return 1;
	}
	application_init(&argc, &argv);	

	font1 =dfb_get_font( font_size32, 0);
	if(font1){
		dfb_destroy_font(font1);
	}																//netpay
	dfb_create_font("/usr/share/fonts/arialnb.ttf",  font_size32, 0);
	lcdGetSize(&iScreenwidth, &iScreenheight);
	if (!(iScreenheight==64 && iScreenwidth == 128))
	{
		// for display 18 card number
		font1 =dfb_get_font( small_font_size, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/wqy-microhei.ttf",  small_font_size, 0);

		font1 = dfb_get_font( small_font_size/2, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/wqy-microhei.ttf", small_font_size/2, 0);
	
		//normal font
		font1 =dfb_get_font( big_font_size, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/wqy-microhei.ttf", big_font_size, 0);
		font1 = dfb_get_font( big_font_size/2, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/wqy-microhei.ttf",    big_font_size/2, 0);
		iRet = lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);

		//normal font
		font1 =dfb_get_font( NP_font, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/wqy-microhei.ttf", NP_font, 0);
		font1 = dfb_get_font( NP_font/2, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/wqy-microhei.ttf",    NP_font/2, 0);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 20, 0);
		
		//PosSong-16
		font1 =dfb_get_font( font_size36, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/PosSong-16.ttf", font_size36, 0);
		font1 = dfb_get_font( small_font_size/2, 0);
		if(font1){
			dfb_destroy_font(font1);
		}
		dfb_create_font("/usr/share/fonts/PosSong-16.ttf",    font_size36/2, 0);
		lcdSetFont("/usr/share/fonts/PosSong-16.ttf", "GBK", 0, font_size36, 0);

		lcdSetFgColor(fgColor);
		lcdSetBgColor(bgColor);
		lcdSetFont("/usr/share/fonts/wqy-microhei.ttf", "GBK", 0, 30, 0);
		lcdDrBgLogo(0, 0, 320, 240, "bg_image.png");
	}

	sys_kbd_get(&stdbCfg);
	stdbCfg.beep_enable = 1;
	stdbCfg.beep_freq = 2800;
	stdbCfg.beep_time = 20;
	stdbCfg.backlight = 60;
	stdbCfg.feed_enable = 1;
	sys_kbd_set(&stdbCfg);

	sysGetbacklighttime(&ibacklight_set_time_bak);
	if (ibacklight_set_time_bak == -1)
	{
		sysSetbacklighttime(60);
	}

	ped_set_pin_input_region(0, 105, 320, 43);

	signal(SIGPIPE,SIG_IGN);

	Init_timer();
	return 0;
}

INT32 PedWriteMasterKey(UINT32 Mode, UINT16  srcKeyID, UINT16 destKeyID, UINT16 KeyLen, UINT8 *KeyBuf)
{
	int iRet;
	iRet = ped_write_master_key(Mode,srcKeyID,destKeyID, KeyLen, KeyBuf);
	return (iRet ? errno : 0);
}

INT32 PedWritePinKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 PinKeyID, UINT16 PinKeyLen, UINT8 *PinKey)
{
	int iRet;
	iRet = ped_write_pin_key(MasterKeyID, PinKeyID, PinKeyLen, PinKey);
	return (iRet ? errno : 0);
}

INT32 PedWriteMacKey(UINT32 Mode, UINT16  MasterKeyID, UINT16 MacKeyID, UINT16 MacKeyLen, UINT8 *MacKey)
{
	int iRet;
	iRet = ped_write_mac_key(MasterKeyID, MacKeyID, MacKeyLen, MacKey);
	return (iRet ? errno : 0);
}

INT32  PedGetPin(UINT16 PinKeyID, UINT8 *ExpectPinLen, UINT8 *CardNo, INT32 Mode, UINT8 *PinBlock)
{
	int iRet;
	iRet = ped_get_pin(PinKeyID,Mode,(const char*)ExpectPinLen,(const char*)CardNo,PinBlock);
	return (iRet ? errno : 0);
}

INT32 PedGetMac(UINT16 MacKeyID, UINT8 *DataIn, UINT16 InLen, INT32 Mode, UINT8 *MacOut)
{
	int iRet;
	iRet = ped_get_mac(MacKeyID,Mode,(UINT32)InLen,(const char*)DataIn,MacOut);
	return (iRet ? errno : 0);
}

int32_t PedGetSensitiveTimer(int32_t SensitiveType)
{
	return ped_get_sensitive_timer(SensitiveType);
}

INT32  PedSetPinInputTimeout(int iTimeOut)
{
	return ped_set_pin_input_timeout(iTimeOut);
}

int iccCardExchangeAPDU(int fd, uint32_t sendlen, const void *senddata, uint32_t *recvlen, void *recvdata)
{
	int ret;
	ret = iccard_exchange_apdu(fd, sendlen, senddata, recvlen, recvdata);
#ifdef __DEBUG
	printf("\n==================================================\n");
	printf("ret = %d\n",ret);
	printf("cmd,len=%-3d,data: ",sendlen);
	for(fd=0; fd<sendlen;fd++)
	  printf("%02x",*(uint8_t*)(senddata+fd));
	
	printf("\nrsp,len=%-3d,data: ",*recvlen);
	for(fd=0; fd<*recvlen;fd++)
	  printf("%02x",*(uint8_t*)(recvdata+fd));
	printf("\n==================================================\n\n");
#endif
	return ret;
}

void PrnBigFontChnData(void)
{
	int  iRet;
    
    if (gstPosCapability.uiPrnType == 'S')
      iRet = prnSetFont("/usr/share/fonts/PosSong-16.ttf", "GB18030", 0, 16, SET_PRIMARY_FONT);
    else
      iRet = prnSetFont("/usr/share/fonts/PosSong-24.ttf", "GB18030", 0, 24, SET_PRIMARY_FONT);
  if ( iRet!=OK )
  {
    lcdClrLine(2, 7);
    DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST ");
    lcdFlip();
    kbGetKey();
    return ;
  }
  return ;	
}

void PrnBlackEngData(void)
{
	int  iRet;
	
	if (gstPosCapability.uiPrnType == 'S')
		iRet = prnSetFont("/usr/share/fonts/PosSong-16.ttf", "GB18030", 0, 16, SET_PRIMARY_FONT);
	else
		iRet = prnSetFont("/usr/share/fonts/arialnb.ttf", FONT_CHARSET_ASCII, 0, 32, SET_PRIMARY_FONT);
  if ( iRet!=OK )
  {
    lcdClrLine(2, 7);
    DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST ");
    lcdFlip();
    kbGetKey();
    return ;
  }
  return ;	
}

void PrnSmallFontData(void)
{
  int  iRet;
  
  if (gstPosCapability.uiPrnType == 'S')
    iRet = prnSetFont("/usr/share/fonts/PosSong-8.ttf", "GB18030", 0, 8, SET_PRIMARY_FONT);
  else
    iRet = prnSetFont("/usr/share/fonts/PosSong-16.ttf", "GB18030", 0, 16, SET_PRIMARY_FONT);
  if ( iRet!=OK )
  {
    lcdClrLine(2, 7);
    DispMulLanguageString(0, 2, DISP_CFONT, NULL, "FONT NOT EXIST");
    lcdDisplay(0, 4, DISP_CFONT, "PosSong-16.ttf ");
    lcdFlip();
    kbGetKey();
    return ;
  }
  return ;
}

void PrnSmallEngData(void)
{
	PrnSmallFontData();
}

void lcdDrLogo(INT32 X, INT32 Y, UINT8 *pucBitMap)
{
	if(gstPosCapability.uiScreenType)
	{
		lcdDrawPicture(X,Y,170,90, (char*)pucBitMap);
	}
	else
	{
		lcdDrawPicture(X,Y,48,32, (char *)pucBitMap);
	}
}

void lcdDispScreen(INT32 X, INT32 Y, UINT8 *pucBitMap,UINT8 ucFirst,UINT8 *pDispBuf1,UINT8 *pDispBuf2,UINT8 ucMode)
{
	int iNewTime;
	static int iOldTime=0;
	static UINT8 ucFlip1,ucFlip2;

	iNewTime = sysGetTimerCount();	
	if (ucFirst == 0)
	{
		iOldTime = iNewTime;
		ucFlip1 = ucFlip2 = 0;
	}

	if(gstPosCapability.uiScreenType)
	{
		if (ucFirst == 0)
		{
			lcdClrLine(2,7);		
			if (strlen((char *)pDispBuf1)>5)
			{
				lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf1);
			}
			
			if (strlen((char *)pDispBuf2))
			{
				lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf2);
			}
/*			if ((ucMode & 0x20) == CARD_PASSIVE)
				lcdDrawPicture(X,Y,144,90, (char*)pucBitMap);
*/			lcdFlip();
		}
	}
	else
	{		
		if ((ucMode&0x20)==CARD_PASSIVE && fileExist((char *)pucBitMap)>=0)
		{
			if (iNewTime-iOldTime<=3000)
			{
				ucFlip2 = 0;				
				if (ucFlip1 == 0)
				{
					lcdClrLine(2,7);
					lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf1);
					if (strlen((char *)pDispBuf2))
					{
						lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf2);
					}
					lcdFlip();
					ucFlip1 = 1;
				}
			}
			else if(iNewTime-iOldTime>3000 && iNewTime-iOldTime<6000) 
			{
				ucFlip1 = 0;			
				if (ucFlip2 == 0)
				{
					lcdClrLine(2,7);
//					lcdDrawPicture(40,25,54,36, (char *)pucBitMap);
					lcdFlip();
					ucFlip2 = 1;
				}
			}
			else
			{
				iOldTime = iNewTime;
			}
		}
		else
		{
			if (ucFirst == 0)
			{
				lcdClrLine(2,7);
				lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf1);
				if (strlen((char *)pDispBuf2))
				{
					lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, (char *)pDispBuf2);
				}
				lcdFlip();
			}
		}		
	}
}

int sysSetbacklighttime(int keep)
{
	return sys_backlight_set_time(keep);
}

int sysGetbacklighttime(int *keep)
{
	return sys_backlight_get_time(keep);
}

int EppLight(DWORD dwMode, DWORD dwTimeMs)
{
	if (dwMode==2)
	{
		return epp_light(glbEpp,-1);
	}
	else if (dwMode==3)
	{
		return epp_light(glbEpp,0);
	}
	else
	{
		return epp_light(glbEpp,dwTimeMs);
	}
}

int EppDisplayLogo(int X, int Y, const uint8_t *pbyBmpLogoIn)
{
	return epp_display_logo(glbEpp,X,Y,pbyBmpLogoIn);
}

int EppDisplayString(int X, int Y, int iMode, const void *str, int iStrLen)
{
	return epp_display_string(glbEpp,X,Y,iMode,str,iStrLen);
}

int EppClearScreen(void)
{
	return epp_clear_screen(glbEpp);
}

int EppSetPinInputTimeout(WORD wTimeout100Ms)
{
	return epp_set_pin_input_timeout(glbEpp,wTimeout100Ms);
}

int EppGetPin(uint32_t byKeyType, uint32_t byKeyIndex, uint32_t byDisplayMode,
            uint32_t byMode, const uint8_t *pbyCardInfo, const uint8_t *pbyLenInfo, uint8_t *pbyPinBlockOut)
{
	return epp_get_pin(glbEpp,byKeyType,byKeyIndex,byDisplayMode,byMode,pbyCardInfo,pbyLenInfo,pbyPinBlockOut);
}

int EppRestoreDefaultIdleLogo(void)
{
	return epp_resume_default_idle_logo(glbEpp);
}

int EppGetMac(uint32_t byKeyType, uint32_t byKeyIndex, const uint8_t *pbyMacDataIn,
                        WORD wMacLen, uint32_t byMode, uint8_t *pbyMacOut)
{
	return epp_get_mac(glbEpp,byKeyType,byKeyIndex,byMode,pbyMacDataIn,wMacLen,pbyMacOut);
}

int EppDownloadAppKey(uint32_t byKeyType, uint32_t byMode, uint32_t byKeyIndex,
    const uint8_t *pbyKeyDataIn, uint32_t byKeyLen, uint32_t byMasterKeyIndex)
{
	EppAppKey_t  AppTempKey;
	
	AppTempKey.KeyType = byKeyType;
	AppTempKey.MasterKeyIndex = byMasterKeyIndex;
	AppTempKey.KeyIndex = byKeyIndex;
	AppTempKey.Mode = byMode;
	AppTempKey.KeyLen = byKeyLen;
	memcpy(AppTempKey.KeyData, pbyKeyDataIn, byKeyLen);
	return epp_download_appkey(glbEpp,&AppTempKey);
}

int EppClose(void)
{
	return epp_close(glbEpp);
}

int EppBeep(DWORD dwFrequency, DWORD dwTimeMs)
{
	return epp_beep(glbEpp,dwFrequency,dwTimeMs);
}

int EppClearAKey(uint32_t byKeyType, uint32_t byKeyIndex)
{
	return epp_clear_key(glbEpp,byKeyType,byKeyIndex);
}



int   InitWiFiModule(uint8_t bInitModule)
{
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}
	return 0;
}

int   GetWiFiStatus(void)
{
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}

	return 0;
}

int    SetWIFIParam(int iMode)
{
	int		iRet;
	
	if (!(gstPosCapability.uiCommType & CT_WIFI))
	{
		return ERR_COMM_INV_TYPE;
	}
	
	lcdCls();
	DispMulLanguageString(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE,NULL, "SETUP WIFI");
	iRet = SetTcpIpParam(iMode);
	if (iRet!=0)
	{
		return iRet;
	}
	
	return 0;
}

int CheckConnectPinPad(uint8_t ucFlag)
{
	int 	  iRet,uiLen=0;
	char      szBuffer[20];

	iRet = epp_get_rand(glbEpp,szBuffer,uiLen);
	if( iRet != 0 )
	{
		lcdClrLine(2, 7);    
		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "CHECK PINPAD");
		lcdFlip();
		ErrorBeep();
		kbGetKeyMs(3000);
		return NO_DISP;
	}
	return 0;
}

void IfSupportPinpad(void)
{
	if (PARAM_OPEN != stPosParam.ucSupPinpad && PARAM_CLOSE != stPosParam.ucSupPinpad)
	{
		stPosParam.ucSupPinpad = PARAM_CLOSE;
	}
}
void   AdjustMenuContext(const char *pszInStr, char *pszOutStr)
{
	int  index,ilen,iCnt,iNum=0;

	if (pszInStr==NULL || pszOutStr==NULL)
	{
		return;
	}

	ilen = strlen(pszInStr);
	for (index=1;index<ilen; index++)
	{
		if (pszInStr[index]>='0' && pszInStr[index] <= '9')
		{
			break;
		}
	}

	memcpy(pszOutStr,"  ",2);

	if (index>=ilen)
	{
		strcpy(pszOutStr+2,pszInStr);
	}
	else
	{
		memcpy(pszOutStr+2,pszInStr,index);
		for (iCnt=0;iCnt<index;iCnt++)
		{
			if (pszInStr[iCnt]==' ')
			{
				iNum+=1;
			}
		}
		memset(pszOutStr+index+2,' ',iNum+2);
		strcpy(pszOutStr+index+2+iNum+2,pszInStr+index);
	}
}


#define WIRELESS_ERR(a)		(ERR_COMM_WIRELESS_BASE|(a))
int     CheckWirelessAttached(uint32_t uiTimeOut)
{
	int iRet = WIRELESS_ERR(ERR_COMM_TIMEOUT);
	
	uiTimeOut += sysGetTimerCount();
	while(1)
	{
		if (sysGetTimerCount()>=uiTimeOut){
			wnet_set_attached(0);
			return iRet;
		}
		
		iRet = wnet_set_attached(1);
		if (iRet != 0)
		{
			sysDelayMs(500);
			continue;
		}
		break;
	}
	
	if (iRet != 0)
	{
		wnet_set_attached(0);
	}
	return iRet;
}

void   DrawRect(uint32_t uiBegRow, uint32_t uiBegCol, uint32_t uiEndRow, uint32_t uiEndCol)
{
	return;
}

void timer_thread(union sigval v)
{
	sys_delay_to_suspend();
}


int    Init_timer(void)
{ 
    struct sigevent evp;  
    memset(&evp, 0, sizeof(struct sigevent)); 
	
    evp.sigev_value.sival_int = 111; 
    evp.sigev_notify = SIGEV_THREAD;  
    evp.sigev_notify_function = timer_thread; 
	
    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)  
    {  
        perror("fail to timer_create");  
        exit(-1);  
    } 

	return 0;
}

int start_timer(time_t sec, suseconds_t usec)
{	
	struct itimerspec value;

	if (timerid < 0){
		return -1;
	}
	
    value.it_interval.tv_sec = sec;  
    value.it_interval.tv_nsec = usec;  
    value.it_value.tv_sec = 0;
    value.it_value.tv_nsec = 10;

	if (timer_settime(timerid, 0, &value, NULL) == -1)  
    {  
        perror("fail to timer_settime");  
        exit(-1);  
    } 

    return 0;
}

int stop_timer(void)
{	
	struct itimerspec value;
	
	if (timerid < 0){
		return -1;
	}
	 
    value.it_interval.tv_sec = 0;  
    value.it_interval.tv_nsec = 0;  
    value.it_value.tv_sec = 0;  
    value.it_value.tv_nsec = 0;
	
	if (timer_settime(timerid, 0, &value, NULL) == -1)  
    {  
        perror("fail to timer_settime");  
        exit(-1);  
    } 
	
    return 0;
}

void newprnLogo(int x, int y, int w, int h, uint8_t *filename)
{
	pntDrawPicture(x, y, w, h, (char *)filename);
}

int ExitApplication(void)
{
	int iRet;
	
	if( CheckParamPass() != 1)
	{
		return NO_DISP;
	}
	lcdClrLine(2,7);
	lcdDisplay(0,2,DISP_CFONT|DISP_MEDIACY,"EXIT APP?");
	lcdDisplay(0,4,DISP_CFONT|DISP_MEDIACY, "1.YES   0.NO");
	lcdFlip();
	iRet = kbGetKeyMs(30000);
	if( iRet==KEY1 )
	{
		exit(0);
	}
	return 0;
}

void SetSupportPinpad(void)
{
	UINT8 ucKey;
	
	ucKey = OptionsSelect(" SUPPORT PINPAD ", stPosParam.ucSupPinpad==PARAM_OPEN,
		"CURRENT:YES", "CURRENT:NO",	"[1].YES", "[2].NO", (stPosParam.ucOprtLimitTime));
	
	if( ucKey==KEY2 ) 
	{
		if(stPosParam.ucSupPinpad != PARAM_CLOSE)
		{
			stPosParam.ucSupPinpad = PARAM_CLOSE;
			stTransCtrl.ucLogonFlag=LOGGEDOFF;
		}
	}
	else if( ucKey==KEY1 ) 
	{
		
		if(stPosParam.ucSupPinpad != PARAM_OPEN)
		{
			stPosParam.ucSupPinpad = PARAM_OPEN;
			stTransCtrl.ucLogonFlag=LOGGEDOFF;
		}
		glbEpp = epp_open("/dev/ttyS3", O_RDWR);
	}
	else if( ucKey==KEY_CANCEL )
		return;		
}

void AppMagOpen(void)
{
}

void AppMagClose(void)
{
}

int InitWirelessModule(ST_WIRELESS_INFO *pstParam)
{
	WnetPPPInfo_t wnetinfo;
	int state=0,idispinfo=0,ierror=0,sg_iStartWnet;
	
	if( wc_get_state(&state, &idispinfo, &ierror)==FAIL)
	{
		sg_iStartWnet = wc_start_wnet();
		if( sg_iStartWnet!=0)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT, "start wnet fail");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(3);
			return sg_iStartWnet;
		}
	}

	strcpy(wnetinfo.acSimPin,(char *)pstParam->szSimPin);
	strcpy(wnetinfo.acApn,(char *)pstParam->szAPN);
	strcpy(wnetinfo.acUserName,(char *)pstParam->szUID);
	strcpy(wnetinfo.acPassWord,(char *)pstParam->szPwd);
	strcpy(wnetinfo.acDialNum,(char *)stPosParam.szDialNum);
	wnetinfo.uiAuth = stPosParam.iPPPAuth; 
	if(stPosParam.ucWnetMode==PARAM_OPEN)
		wnetinfo.eWnetMode = E_WNET_RENEW_PPPOPEN;
	else
		wnetinfo.eWnetMode = E_WNET_AUTO_RESTRAT;
	wc_param_set(&wnetinfo);
	
	wc_set_state(STATE_POWER_ON);
	usleep(10*1000);//Because the wireless process delay 10ms
	return 0;	
}

static uint32_t  DispCurrentstate(E_DISPLAY_INFO edisplay, int ierror)
{	
	int iNum = edisplay;
	int DispPosition,line_no;

	if(gstPosCapability.uiScreenType)
	{
		DispPosition = 268;
		line_no = 10;		
	}
	else
	{
		DispPosition = 104;
		line_no = 7;	
	}

	switch(iNum)
	{
	case DISP_POWERONING:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_POWERONING);
		break;
	case DISP_POWERON_OVERTIMES:
		CurStatus = DISP_POWERON_OVERTIMES;
		break;
	case DISP_WNETINTING:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_WNETINTING);
		break;
	case DISP_WNETINITFAILED:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_WNETINITFAILED);
		break;	
	case DISP_CHECKSIM_TIMOUT:
		CurStatus = DISP_CHECKSIM_TIMOUT;
		break;
	case DISP_SIMCARD_FAILED:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_SIMCARD_FAILED);
		break;	
	case DISP_CHECKSIM_NOINSTER:
		CurStatus = DISP_CHECKSIM_NOINSTER;
		break;
	case DISP_INPUTPIN_ERROR:
		CurStatus = DISP_INPUTPIN_ERROR;
		break;
	case DISP_PUK_ERROR:
		CurStatus = DISP_PUK_ERROR;
		break;	
	case DISP_NO_SIMPIN:
		CurStatus = DISP_NO_SIMPIN;
		break;	
	case DISP_SIMCARD_OK:
		CurStatus = 0;
		break;	
	case DISP_ATTACHING:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_ATTACHING);
		break;	
	case DISP_ATTACH_TIMEOUT:
		CurStatus = DISP_ATTACH_TIMEOUT;
		break;	
	case DISP_PPP_BUILDING:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_PPP_BUILDING);
		break;
	case DISP_PPPCHECK_FAILED:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_PPPCHECK_FAILED);
		break;	
	case DISP_PPPCLOSEING:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_PPPCLOSEING);
		break;
	case DISP_PPPOPENING:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_PPPOPENING);
		break;	
	case DISP_PPPOPEN_OVERTIMES:
		CurStatus = DISP_PPPOPEN_OVERTIMES;
		break;
	case DISP_PPPOPEN_FAILE:
		lcdDisplay(DispPosition, line_no, DISP_ASCII, "%d", DISP_PPPOPEN_FAILE);
		break;	
	default:
		return 0;
	}
	lcdFlip();
	return 0;
}

static int DispErrStatus(int iStatus)
{
	int iRet = 0;
	
	lcdClrLine(2,7);
	DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL, "COMM FAIL");
	switch(iStatus)
	{
	case DISP_POWERON_OVERTIMES:		
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Power on Over");
		break;
	case DISP_CHECKSIM_TIMOUT:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY,NULL, "Check SIM timeout");
		break;	
	case DISP_CHECKSIM_NOINSTER:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "pls insert sim");
		iRet = 1;
		break;
	case DISP_INPUTPIN_ERROR:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "pin error");
		iRet = 1;
		break;
	case DISP_PUK_ERROR:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "puk error");
		iRet = 1;
		break;		
	case DISP_ATTACH_TIMEOUT:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Attach timeout");
		break;		
	case DISP_PPPOPEN_OVERTIMES:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "PPP OPEN Over");
		break;
	case DISP_NO_SIMPIN:
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL, "Enter SIM Pin");
		iRet = 1;
		break;	
	default:
		
		break;	
	}
	lcdFlip();
	PubBeepErr();
	return iRet;
}

static int  DispCurrentConnect(E_CONNECT_DISPLAY econnect, int ierror)
{		
	char szEng[30];
	int  iNum = econnect;

	lcdClrLine(2,7);
	switch(iNum)
	{
	case CONNECT_FIRST_IP:		
//		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "TCPIP CONNECT(1)");
		break;
	case CONNECT_SECOND_IP:
//		DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "TCPIP CONNECT(2)");
		break;	
	case CONNECT_FIRSTIP_TIMEOUT:
		snprintf(szEng,sizeof(szEng),"timeout(1)");
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;
	case CONNECT_FIRSTIP_FAILED:
		snprintf(szEng,sizeof(szEng),"Fallo Conexion(1):%d",ierror);
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;	
	case CONNECT_SECONDIP_TIMEOUT:
		snprintf(szEng,sizeof(szEng),"timeout(2)");
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;
	case CONNECT_SECONDIP_FAILED:
		snprintf(szEng,sizeof(szEng),"Fallo Conexion(2):%d",ierror);
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;	
	default:
		snprintf(szEng,sizeof(szEng),"other error:%d",iNum);
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;
	}
	lcdFlip();

	return 0;
}


uint32_t  DispFailComm(int iReturn, int ierror)
{	
	char szEng[30];

	lcdClrLine(2,7);
	switch(iReturn)
	{
	case FAIL:		
		snprintf(szEng,sizeof(szEng),"comm fail:%d",ierror);
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;
	case TIMEOUT:
		snprintf(szEng,sizeof(szEng),"timeout");
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;	
	case CANCEL:
		snprintf(szEng,sizeof(szEng),"Boton de cancelar");
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;		
	default:
		snprintf(szEng,sizeof(szEng),"other error:%d", iReturn);
		lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, szEng);
		break;
	}
	lcdFlip();
	PubBeepErr();
	PubWaitKey(5);
	return 0;
}

static uint32_t CheckKey(void)
{
	int iHit;
	
	iHit = kbhit();
	if( iHit==YES )
	{
		iHit = kbGetKey();
		if( iHit==KEY_CANCEL )
		{
			return 0;
		}
	}
	return 1;
}

int WirelessDial(uint8_t ucDialFlag)
{	
	WnetCallback_t   WneCallBack;
	WnetPPPInfo_t    wnetinfo;
	int iRet,iCnt=0,state=0,idispinfo=0,ierror=0,sg_iStartWnet,ifCheckSim=0;
	static int iReply=0;
	
	if(sg_Wiresocket!=-1)
	{
		return 0;
	}  

	if( wc_get_state(&state, &idispinfo, &ierror)==FAIL)
	{
		sg_iStartWnet = wc_start_wnet();
		if( sg_iStartWnet!=0 && ucDialFlag==ACTDIAL_MODE) //启动无线进程不成功
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT, "start wnet fail");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(3);
			return sg_iStartWnet;
		}

		if(PosCom.stTrans.iTransNo == DOWNLOAD)
		{
			strcpy(wnetinfo.acSimPin,(char *)stPosParam.stDownParamCommCfg.stWirlessPara.szSimPin);
			strcpy(wnetinfo.acApn,(char *)stPosParam.stDownParamCommCfg.stWirlessPara.szAPN);
			strcpy(wnetinfo.acUserName,(char *)stPosParam.stDownParamCommCfg.stWirlessPara.szUID);
			strcpy(wnetinfo.acPassWord,(char *)stPosParam.stDownParamCommCfg.stWirlessPara.szPwd);
		}
		else
		{
			strcpy(wnetinfo.acSimPin,(char *)stPosParam.stTxnCommCfg.stWirlessPara.szSimPin);
			strcpy(wnetinfo.acApn,(char *)stPosParam.stTxnCommCfg.stWirlessPara.szAPN);
			strcpy(wnetinfo.acUserName,(char *)stPosParam.stTxnCommCfg.stWirlessPara.szUID);
			strcpy(wnetinfo.acPassWord,(char *)stPosParam.stTxnCommCfg.stWirlessPara.szPwd);
		}

		strcpy(wnetinfo.acDialNum,(char *)stPosParam.szDialNum);
		wnetinfo.uiAuth = stPosParam.iPPPAuth; 
		if(stPosParam.ucWnetMode==PARAM_OPEN)
			wnetinfo.eWnetMode = E_WNET_RENEW_PPPOPEN;
		else
			wnetinfo.eWnetMode = E_WNET_AUTO_RESTRAT;
		wc_param_set(&wnetinfo);
		wc_set_state(STATE_POWER_ON);
		usleep(10*1000);//Because the wireless process delay 10ms
	}

	if(ucDialFlag==PREDIAL_MODE)
	{
		stTemp.bCheckPrdial = 1;
		wc_get_state(&state,  &idispinfo, &ierror);
		if(state==STATE_END)
		{
			wc_set_state(STATE_POWER_ON);
			usleep(10*1000);//Because the wireless process delay 10ms
		}
		else if(state==STATE_PPP_READY || state==STATE_PPP_SHORT_CONNET)
		{
			wc_set_state(STATE_CHECK_SIM_2);
			usleep(10*1000);//Because the wireless process delay 10ms	
		}
		return 0;
	}

	if(bCheckPPP == 1)
	{
		wc_get_state(&state,  &idispinfo, &ierror);
		if(state!=STATE_PPP_READY || stTemp.bCheckPrdial==0)
			ifCheckSim = 1;
	}	

	if(ifCheckSim == 1)
	{
PPP_READY:	
		CurStatus = 0;
		if(ucDialFlag==ACTDIAL_MODE)
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "PROCESANDO...");
//			DispMulLanguageString(0, 5, DISP_CFONT|DISP_MEDIACY, "pls waiting","pls waiting...");
			lcdFlip();
			WneCallBack.disp_currentstate = DispCurrentstate;
			WneCallBack.wait_key = CheckKey;
		}
		else
		{
			WneCallBack.disp_currentstate = NULL;
			WneCallBack.wait_key = NULL;
		}

		// netpay
		if( ucDialFlag!=PPP_ONLY_MODE )
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "PROCESANDO");
			lcdFlip();
			PubWaitKey(1);
		}	
		
		iRet = wc_wait_ppp_ready(&WneCallBack);
		if(iRet != 0) 
		{
			if(iRet == CANCEL)
			{
				iReply = 1;
				if(ucDialFlag==ACTDIAL_MODE)
				{
					lcdClrLine(2, 7);
					DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, "按键取消", "key cancel");
					lcdFlip();
					PubBeepErr();
					PubWaitKey(5);
				}
			}
			else
			{
				if(iReply==1)//If the previous transaction press the Cancel button,execute 2 times
				{
					iReply = 0;
					goto PPP_READY;
				}
				if(ucDialFlag==ACTDIAL_MODE)
				{
					iRet = DispErrStatus(CurStatus);
					if(iRet == 0)
					{
						while(1)//需要等下电完成，要不然进程同步去上电会有问题
						{
							wc_get_state(&state,  &idispinfo, &ierror);
							if(state!=STATE_END)
							{
								usleep(10*1000);
								continue;
							}
							break;
						}
						wc_set_state(STATE_POWER_ON);//在后台初始化
						usleep(10*1000);//因为无线进程里面有延时10ms，这里也必须延时
					}
					PubWaitKey(5);	
				}
			}	
			PubWaitKey(5);		
			return NO_DISP;
		}
		wc_get_ppp_auth(&stPosParam.iPPPAuth);
		SaveAppParam();
	}

	if(ucDialFlag==PPP_ONLY_MODE)  //仅做PPP连接
	    return 0;
	
	iRet = TcpConnect(1);	
	if(iRet < 0)
	{
		if(iRet!=CANCEL && iRet!=PARAM_ERR)
		{
			wc_ready_pppclose();
			if(iCnt==0)
			{
				iCnt++;
				goto PPP_READY;
			}
			PubBeepErr();
			PubWaitKey(5);	
		}
		return NO_DISP;
	}
	sg_Wiresocket = iRet;
	return 0;
}


int WirelessClose(uint8_t bRelease,uint8_t bPowerDown,uint8_t ucFlag)
{	
	int iRet=0,state=0,idispinfo=0,ierror=0;

	if( wc_get_state(&state, &idispinfo, &ierror)==FAIL)
	{
		return NO_DISP;
	}
	
	if(bPowerDown)
	{
		lcdCls();
		DispMulLanguageString(0, 2, DISP_CFONT|DISP_MEDIACY, NULL,"power down...");
		DispMulLanguageString(0, 4, DISP_CFONT|DISP_MEDIACY, NULL,"pls waiting...");
		lcdFlip();
		iRet = wc_ppp_close(sg_Wiresocket,HANGUP_TCP);//hang up TCP		
		wc_stop_wnet(); //hang up ppp and power down
		while(1)
		{
			wc_get_state(&state,  &idispinfo, &ierror);
			if(state!=STATE_END)
			{
				usleep(10*1000);
				continue;
			}
			break;
		}
	}
	else
	{
		if(bRelease)
		{
			wc_get_state(&state,  &idispinfo, &ierror);
			if(state==STATE_PPP_READY)//if the status is power down,fordibben ppp close
			{
				if(sg_Wiresocket==-1)//wc_ppp_close detect illegal value
					iRet = wc_ppp_close(1024,HANGUP_TCP_PPP);
				else
					iRet = wc_ppp_close(sg_Wiresocket,HANGUP_TCP_PPP);
			}
		}
		else
			iRet = wc_ppp_close(sg_Wiresocket,HANGUP_TCP);
	}
	sg_Wiresocket = -1;
	if(ucFlag == 0)
		bCheckPPP = 0;
	else
		bCheckPPP = 1;
	return iRet;
}


int TcpSendData(int iHandle, uint8_t *psTxdData, uint32_t uiDataLen)
{
	int iRet,ierror;

	iRet = wc_send_data(iHandle, psTxdData, uiDataLen);
	if(iRet != 0)
	{
		ierror = errno;
		DispFailComm(iRet,ierror);
		return NO_DISP;
	}
	return 0;
}

uint32_t DispRecvTime(uint32_t uiTimeOut)
{
	if(gstPosCapability.uiScreenType)
		;
		//netpay
//		lcdDisplay(200,4,DISP_CFONT|DISP_FLIP,"(%02d)",uiTimeOut);
	else
		;
//		lcdDisplay(87,4,DISP_CFONT|DISP_FLIP,"(%02d)",uiTimeOut);
	return 0;
}


static int TcpRecvData(int iHandle,uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int iRet=0,ierror,iLen=0,iLenbak=0,iOldTime,iNewTime,iNewTimeOut = 0;

	iOldTime = sysGetTimerCount();
	while(1)
	{
		iRet = wc_recv_data(iHandle,psRxdData+iLenbak,uiExpLen-iLenbak, uiTimeOut-iNewTimeOut,DispRecvTime);
		iNewTime = sysGetTimerCount(); 
		if(iRet<=0)
		{
			ierror = errno;
			if(iNewTime-iOldTime > uiTimeOut*1000)
				DispFailComm(TIMEOUT,ierror);
			else
				DispFailComm(iRet,ierror);
			return NO_DISP;
		}

		iLenbak += iRet;
		if(iLenbak >= 2)
			iLen = PubChar2Long(psRxdData, 2, NULL);	

		if(iLenbak>=iLen+2)
		{
			*puiOutLen = iLen+2;
			break;
		}
		iNewTimeOut = (iNewTime-iOldTime)/1000;
		if(iNewTimeOut > uiTimeOut)
		{
			ierror = errno;
			DispFailComm(TIMEOUT,ierror);
			return NO_DISP;
		}
	}
	return 0;
}

int WirelessTxdNormal(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	return TcpSendData(sg_Wiresocket, psTxdData, uiDataLen);
}

int WirelessRxdNormal(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	int iRet;
	
	 iRet = TcpRecvData(sg_Wiresocket, psRxdData, uiExpLen, uiTimeOut, puiOutLen);
	 if(iRet != 0)
	 {
	 	wc_ready_pppclose();
		WirelessClose(FALSE,0,1);
	 }
	 else
	 {
	 	WirelessClose(FALSE,0,0);
	 }
	 return iRet;
}


int  GetCurWirelessSocket(void)
{
	return sg_Wiresocket;
}

#if 0
//????
//??IP?????
//??:0=??,-1=??
static int getlocalip(char* outip)
{
	int i=0;
	int sockfd;
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	char* ip;
	
	//???ifconf
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		return -1;
	}
	ioctl(sockfd, SIOCGIFCONF, &ifconf);    //????????
	close(sockfd);
	
	//??????????IP??
	ifreq = (struct ifreq*)buf;
	for(i=(ifconf.ifc_len/sizeof(struct ifreq));i>0; i--)
	{
	    ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
	
		if(strcmp(ip,"127.0.0.1")==0)  //??127.0.0.1,?????
		{
			ifreq++;
			continue;
		}
		
		if(outip)
			strcpy(outip, ip);
		return 0;
	}

	return -1;
}
#else
int getlocalip(char* outip)//????????IP
{
	struct ifreq ifr_link, ifr_ip;
	unsigned long int *ip;
	int retval;

	/* check the link */
	if ( (retval = netif_get_flags("wlan0", &ifr_link)) )
	{
		return retval;
	}

	/* check IP */
	memset(&ifr_ip, 0, sizeof(ifr_ip));
	if ( (retval = netif_get_ipaddr("wlan0", &ifr_ip))  && errno != EADDRNOTAVAIL)
	{
		return retval;
	}

	ip = (typeof(ip))&((struct sockaddr_in*)&ifr_ip.ifr_addr)->sin_addr;
	if ( (ifr_link.ifr_flags & IFF_RUNNING) && *ip != INADDR_NONE && *ip != INADDR_ANY)
		return 0;
	else
		return -1;	
}
#endif

static int TcpConnect(uint8_t ucWireless)
{
	Wnet_TcpipInfo_t pstTcpipInfo;
	int  iRet;
	uint8_t			 ucCurCommType;

	if (PosCom.stTrans.iTransNo == DOWNLOAD)
	{
		strcpy(pstTcpipInfo.HostIp[0],(char*)stPosParam.stDownParamCommCfg.stWirlessPara.szRemoteIP);
		pstTcpipInfo.HostPort[0] = atoi((char*)stPosParam.stDownParamCommCfg.stWirlessPara.szRemotePort);
		pstTcpipInfo.iTimeout = stPosParam.stDownParamCommCfg.ulSendTimeOut;
		pstTcpipInfo.iDnsMethod = 0;
		ucCurCommType = stPosParam.stDownParamCommCfg.ucCommType;
		pstTcpipInfo.iDnsIP = 0;//??DNS,???DNS?,????DNS????IP?
	}
	else
	{
		if(stPosParam.ucTCPIndex == 0 || stPosParam.ucTCPIndex == 1)
		{
			PrintDebug("%s%d","TcpConnect ->PosCom.stTrans.isCtlplat:",PosCom.stTrans.isCtlplat);
			if(PosCom.stTrans.isCtlplat ==1)
			{				
				strcpy(pstTcpipInfo.HostIp[0],(char*)stPosParam.szPOSRemoteIP);
				strcpy(pstTcpipInfo.HostIp[1],(char*)stPosParam.szPOSRemoteIP2);
				pstTcpipInfo.HostPort[0] = atoi((char*)stPosParam.szPOSRemotePort);
				pstTcpipInfo.HostPort[1] = atoi((char*)stPosParam.szPOSRemotePort2);

			}
			else
			{
				strcpy(pstTcpipInfo.HostIp[0],(char*)stPosParam.szPOSRemoteIP2);
				strcpy(pstTcpipInfo.HostIp[1],(char*)stPosParam.szPOSRemoteIP);
				pstTcpipInfo.HostPort[0] = atoi((char*)stPosParam.szPOSRemotePort2);
				pstTcpipInfo.HostPort[1] = atoi((char*)stPosParam.szPOSRemotePort);
			}
		}
		else
		{
			strcpy(pstTcpipInfo.HostIp[0],(char*)stPosParam.szPOSRemoteIP2);
			strcpy(pstTcpipInfo.HostIp[1],(char*)stPosParam.szPOSRemoteIP);
			pstTcpipInfo.HostPort[0] = atoi((char*)stPosParam.szPOSRemotePort2);
			pstTcpipInfo.HostPort[1] = atoi((char*)stPosParam.szPOSRemotePort);		
		}
		
		
		pstTcpipInfo.iTimeout = stPosParam.stTxnCommCfg.ulSendTimeOut;
		pstTcpipInfo.iDnsMethod = stPosParam.ucCommHostFlag - '0';
		if(pstTcpipInfo.iDnsMethod != 0)
		{
			strcpy(pstTcpipInfo.HostDns,(char*)stPosParam.stTxnCommCfg.stTcpIpPara.szDNS);
			pstTcpipInfo.HostPort[0] = atoi((char*)stPosParam.szPOSRemotePort);
			pstTcpipInfo.HostPort[1] = atoi((char*)stPosParam.szPOSRemotePort);	
	        	pstTcpipInfo.iDnsIP = 1;//????IP??,????DNS,????IP???DNS???? ?
	        	if(stPosParam.ucCommDNS != PARAM_OPEN)    //?????ip
	            		pstTcpipInfo.iDnsMethod = 0;
	    	}
	    	else
	    	{
	        	pstTcpipInfo.iDnsIP = 0;
		}
		ucCurCommType = stPosParam.stTxnCommCfg.ucCommType;
	}
	
	if(ucWireless)
		pstTcpipInfo.iCurrentComm = E_COMM_WIRELESS;
	else
		pstTcpipInfo.iCurrentComm = E_COMM_TCPIP;//Distinguish wireless and tcp
	pstTcpipInfo.wnet_conncet_display = DispCurrentConnect;	
	pstTcpipInfo.wait_key = CheckKey;
	if(ucCurCommType == CT_TCPIP)
		pstTcpipInfo.eCurrentComm = E_COMM_CONNECT_TCPIP;
	else if(ucCurCommType == CT_WIFI)
		pstTcpipInfo.eCurrentComm = E_COMM_CONNECT_WIFI;
	else
		pstTcpipInfo.eCurrentComm = E_COMM_CONNECT_PPP;//lock communication mode

	lcdClrLine(2, 7);
	lcdDisplay(0, 3, DISP_CFONT|DISP_MEDIACY, "PROCESANDO");
	lcdFlip();
	PubWaitKey(1);

	iRet = wc_tcpip_connect(&pstTcpipInfo);
	if(iRet < 0)
	{
		if(iRet == CANCEL)
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "Boton de cancelar");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(5);	
		}
		else if(iRet == PARAM_ERR)
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 3, DISP_CFONT|DISP_MEDIACY, NULL, "param error");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(5);	
		}
		else
		{
			if(!ucWireless)
			{
				PubBeepErr();
				PubWaitKey(5);	
			}
		}			
	}
	else
	{
		if(pstTcpipInfo.iDnsMethod!=0 && stPosParam.ucCommDNS==PARAM_OPEN)
		{
			stPosParam.ucCommDNS = PARAM_CLOSE;                              //?????DNS
			strcpy((char*)stPosParam.szPOSRemoteIP, pstTcpipInfo.HostIp[0]); //??dns?????ip
			strcpy((char*)stPosParam.szPOSRemoteIP2, pstTcpipInfo.HostIp[0]);//??dns?????ip
		}
	
		if( pstTcpipInfo.iCurrentIp == 1  )
		{
			if( stPosParam.ucTCPIndex != 2 )
			{
				stPosParam.ucSecondIpFlag = 1;
			}
			else
			{
				stPosParam.ucSecondIpFlag = 2;
			}
			
		}
		else if( pstTcpipInfo.iCurrentIp == 2  )
		{
			if( stPosParam.ucTCPIndex == 2 )
			{
				stPosParam.ucSecondIpFlag = 1;
			}
			else
			{
				stPosParam.ucSecondIpFlag = 2;
			}
		}
		//返回1和2值
		if(pstTcpipInfo.iCurrentIp == 2)
		{
			if(stPosParam.ucTCPIndex == 2)
				stPosParam.ucTCPIndex = 1;
			else
				stPosParam.ucTCPIndex = 2;
		}
		SaveAppParam();
	}

	
	
	return iRet;
}

int TcpDial(uint8_t ucDialMode)
{
	int iRet;

	if(ucDialMode==PREDIAL_MODE || sg_Tcpsocket!=-1)  
		return 0;
	
	iRet = TcpConnect(0);
	if(iRet < 0)
	{
		return NO_DISP;
	}
	sg_Tcpsocket = iRet;
	return 0;
}


int   WiFiDial(uint8_t ucDialMode)
{
	int iRet, t0;
	uint8_t			 ucGetIP=0;

	if(ucDialMode==PREDIAL_MODE || sg_Tcpsocket!=-1)  
		return 0;

	lcdClrLine(2, 7);	
	t0 = sysGetTimerCount();
	while(sysGetTimerCount()-t0 < 60000)	
	{
		if( getlocalip(NULL) == 0)
		{
			ucGetIP = 1;
			break;
		}

			if( PubChkKeyInput() )
			{
				if (kbGetKey() == KEY_CANCEL)
				{
					return ERR_USERCANCEL;
				}
			}

			lcdDisplay(0,3,DISP_CFONT|DISP_MEDIACY,"Recibiendo...");
			lcdFlip();	
			sysDelayMs(100);	
			continue;
		if(ucGetIP == 0)
		{
			lcdClrLine(2, 7);
			DispMulLanguageString(0, 4, DISP_CFONT | DISP_MEDIACY, NULL, "Fallo Conexion");
//			DispMulLanguageString(0, 4, DISP_CFONT | DISP_MEDIACY, NULL, "get ip fail");
			lcdFlip();
			PubBeepErr();
			PubWaitKey(5);
			return NO_DISP;
		}		
	}
	
	iRet = TcpConnect(0);
	if(iRet < 0)
	{
		if(wifi_reassoc && iRet == FAIL)
			wifi_reassoc();//????
		return NO_DISP;
	}
	sg_Tcpsocket = iRet;
	return 0;
}


int TcpTxdNormal(uint8_t *psTxdData, uint32_t uiDataLen, uint32_t uiTimeOut)
{
	return TcpSendData(sg_Tcpsocket, psTxdData, uiDataLen);
}

int TcpRxdNormal(uint8_t *psRxdData, uint32_t uiExpLen, uint32_t uiTimeOut, uint32_t *puiOutLen)
{
	return TcpRecvData(sg_Tcpsocket, psRxdData, uiExpLen, uiTimeOut, puiOutLen);
}

int TcpOnHook(uint8_t bReleaseAll)
{
	int  iRet=0;

	if(sg_Tcpsocket >= 0)
	{
		iRet = close(sg_Tcpsocket);
		sg_Tcpsocket = -1;
	}

	if (bReleaseAll)
	{
		iRet = PPPLogout(OPT_DEVETH);
	}

	return iRet;
}

int  GetCurTcpSocket(void)
{
	return sg_Tcpsocket;
}

void CheckWireless(void)
{
	int     iWentMdoel;
	
	if(gstPosCapability.uiCommType & (CT_GPRS|CT_CDMA))
	{
		if(stPosParam.ucCommModule>0)
			iWentMdoel = stPosParam.ucCommModule;
		else
		{
			lcdClrLine(2, 7);
			lcdDisplay(0, 2, DISP_CFONT|DISP_MEDIACY, "module info...");
			lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "pls waiting");
			lcdFlip();
			iWentMdoel = wc_get_wirelesstype();//?????
			stPosParam.ucCommModule = iWentMdoel;
			SaveAppParam();
		}
			
		if(iWentMdoel==E_WNET_CDMA || iWentMdoel==E_WNET_CDMA2000)
		{
			gstPosCapability.uiCommType &= ~CT_GPRS;
			gstPosCapability.uiCommType |= CT_CDMA;
		}
		else if(iWentMdoel==E_WNET_GPRS || iWentMdoel==E_WNET_WCDMA || iWentMdoel==E_WNET_TDSCDMA)
		{
			gstPosCapability.uiCommType &= ~CT_CDMA;
			gstPosCapability.uiCommType |= CT_GPRS;
		}
	}
}

void GetBaseStationInfo(uint8_t *szStationInfo)
{
	int iRet, i = 0, iTimeout = 0, piOutNum = 0, num = 0;	
	tBaseStation oBaseStation;
	tBaseStation oBaseStationExt[6];
	char myszStationInfo[200] = {0};	
	uint8_t szTemp[32 + 1], szCur[32 + 1];
	uint16_t newlac  = 0;
	static uint8_t bStoredSimCardId=FALSE;	
	uint8_t	sim_card_id[25];

	// 1   2 CDMA
	if (!(stPosParam.stTxnCommCfg.ucCommType & CT_GPRS) 
	&& !(stPosParam.stTxnCommCfg.ucCommType  & CT_CDMA))
	{
		return;
	}

	
	if( !bStoredSimCardId )
	{
		memset(sim_card_id, 0, sizeof(sim_card_id));
		iRet = wc_get_simid((char*)sim_card_id, 24);
		if(iRet == OK)
		{
			bStoredSimCardId = TRUE;
		}
	}
	memset(szStationInfo, ' ', 25);
	memcpy(szStationInfo, sim_card_id, strlen((char*)sim_card_id));	
	
	memset(&oBaseStation, 0, sizeof(oBaseStation));	
	memset(oBaseStationExt, 0, sizeof(oBaseStationExt));

	CheckWireless();
	if(stPosParam.ucCommModule==E_WNET_GPRS)
		oBaseStation.iFlag = 1;
	else if(stPosParam.ucCommModule==E_WNET_WCDMA)
		oBaseStation.iFlag = 3;
	else
		oBaseStation.iFlag = 2; //CDMA?CDMA2000??2
	iRet = wc_get_basestationinfo(&oBaseStation,10); 		
    if (NET_OK != iRet)
		return;
	
	if (1==oBaseStation.iFlag || 3==oBaseStation.iFlag)
		sprintf((char*)szStationInfo+25, "%05u%05u", oBaseStation.Wnet.oGprs.lac, oBaseStation.Wnet.oGprs.cell_id);
	else
	{
		sprintf((char*)szStationInfo+25, "%05u%05u", 0, oBaseStation.Wnet.oCdma.BaseStationID);
		return; //CDMA?CDMA2000????????
	}

	i = 0;
	iTimeout = 300;	
	piOutNum = 0;
	while(1)
	{
		iRet = wc_get_neighbourbasestationinfo(oBaseStationExt, 6, &piOutNum);
		if ((NET_OK != iRet) || (oBaseStationExt[0].Wnet.oGprs.mcc == -1) ||
			(oBaseStationExt[0].Wnet.oGprs.mnc == -1) || (oBaseStationExt[0].Wnet.oGprs.mnc == 255))
		{
			i++;
			if(i > 3)
			{
				return;
			}			
			sysDelayMs(iTimeout);//????????????500ms
			continue;
		}
		break;
	}

	if(1 == oBaseStationExt[0].iFlag)
	{
		num = 0;
		memset(szTemp, 0, sizeof(szTemp));
		for(i = 0; (i < piOutNum) && (num < 5); i++)
		{				
			if ((oBaseStationExt[i].Wnet.oGprs.lac == -1) || (oBaseStationExt[i].Wnet.oGprs.cell_id == -1))
			{
				continue;
			}

			if ((oBaseStationExt[i].Wnet.oGprs.lac == 65535) || (oBaseStationExt[i].Wnet.oGprs.cell_id == 65535))
			{
				continue;
			}

			if ((oBaseStationExt[i].Wnet.oGprs.lac == 0) || (oBaseStationExt[i].Wnet.oGprs.cell_id == 0))
			{
				continue;
			}

			memset(szCur, 0, sizeof(szCur));			
			newlac = (uint16_t)oBaseStationExt[i].Wnet.oGprs.lac;
			sprintf((char *)szCur, "%05u%05u", newlac, oBaseStationExt[i].Wnet.oGprs.cell_id);
			if(memcmp(szTemp, szCur, 10) == 0)
			{
				continue;
			}

			strcpy((char *)szTemp, (char *)szCur);
			
			num++;
			newlac = (uint16_t)oBaseStationExt[i].Wnet.oGprs.lac;	
			sprintf(myszStationInfo + strlen(myszStationInfo), "|%05u%05u", newlac, oBaseStationExt[i].Wnet.oGprs.cell_id);
		}
	}
	sprintf((char*)szStationInfo+35, "%s", myszStationInfo);

	return;
}
#endif

//end of line







