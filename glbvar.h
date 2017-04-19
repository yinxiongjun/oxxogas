
#ifndef _GLBVAR_H
#define _GLBVAR_H

#include "posapi.h"
#include "terstruc.h"
#include "posmacro.h"
#include "emvapi.h"
#include "MathLib.h"
#include "PosLib.h"
#include "Display.h"
#include "Commlib.h"
#include "wireless.h"
#include "tmslib.h"
#include "ChkOption.h"
#include "Rs232lib.h"
#include "New7110.h"
#include "New8110.h"
#include "New8210.h"
#include "qpbocProc.h"
#include "showDebug/showDebug.h"
#include "SappTms.h"


extern struct NEWPOS_COM_STRUCT PosCom;
extern struct NEWPOS_COM_STRUCT BackPosCom;
extern struct _total_strc stTotal; 
extern struct _limit_total_strc stLimitTotal; 
extern struct _oper_structure operStrc[MAX_OPER_NUM];
extern struct _TRANS_CTRL_PARAM stTransCtrl; 
extern struct _NEWPOS_PARAM_STRC stPosParam;    
extern struct _st_UPLTRACENO_SYNC stUplTraceSync;

extern int gUplTrace;

extern int    icc_fd;
extern int    iPsam_fd; 
extern int    imif_fd;


extern uint16_t usSendPacketLen;  
extern uint16_t usRecvPacketLen;
extern uint8_t  sSendPacketBuff[MAX_PACKET_LEN];	
extern uint8_t  sRecvPacketBuff[MAX_PACKET_LEN];	

extern struct temp_global_var_strc stTemp;

extern struct emv_core_init tEMVCoreInit;
extern struct terminal_mck_configure tConfig;
extern struct emv_callback_func tEMVCallBackFunc;

extern DIAL_STAT dial_stat;	//Dial statistics
extern struct errinfostru sRetErrInfo[];
extern NEWPOS_8583PACKET_STRC TrTable[];

extern int start_from_manager;	//zyl+ 用于标志程序是否通过主控启动的
extern int g_ums_img;			//zyl+ 是否ums镜像
extern int wifi_status;

extern TermAidList glTermAidList[MAX_APP_NUM];
extern int       glTermAidNum;

extern ST_POS_VERSION        gstPosVersion;
extern ST_POS_CONFIG_INFO    gstPosCapability;      
extern ST_COMM_CONFIG		 gstCurCommCfg;		 
extern  char                 gszParamFileName[50]; 

extern  unsigned char Bmp_Show_in_PIN[];
extern  unsigned char Bmp_Show_Balance[];
extern  unsigned char Bmp_Show_CurrName[];
extern  unsigned char Bmp_Show_AmtName[];
extern  unsigned char Bmp_Show_LytBalance[];
extern  unsigned char Bmp_Show_JiFen[];
extern  unsigned char Bmp_Display[];
extern  unsigned char Bmp_passive[];
extern  unsigned char Bmp_Prn[];
extern  unsigned char Bmp_Display_NoColor[];

extern  struct _NETYAY   Netpay;
extern struct _POS_COM_CONTROL PosComconTrol;

//netpay
extern NETPAYPHONETOPUP TELCEL_SKUTable[];
extern NETPAYPHONETOPUP MOVISTAR_SKUTable[];
extern NETPAYPHONETOPUP IUSACEL_SKUTable[];
extern NETPAYPHONETOPUP UNEFON_SKUTable[];
extern NETPAYPHONETOPUP NEXTEL_SKUTable[];
extern TOTAL_TRANS		AllTransTable[20]; 
extern TOTAL_TRANS           SIGNALTransTable[5];
extern char TitelName[50];//界面标题

//TMS
extern char      ParamPath[256];//补登路径
extern char      s_gszAppName[50];   
extern uint8_t   gucManageCommType;
extern int       giIfInitWirelessSuccess;
extern ST_MANAGE_TCP_INFO    gstManageTcpCfg;

//tms
extern struct ST_PUB_APP_INFO stPubAppInfo;

extern int   DirectIntoMenu;
extern int   NoDispTmsPrecessingFlag;




#define MAX_ITEM_MENU       9
#define MAX_CHARS_PERLINE   39
#define NO_TRANS            -1
#define MAX_HIDE_MENU       5
#define MAX_HIDE_ITEM       5


#define MENU_MODE_1         0x01
/*
1.xxx     2.xxx
3.xxxxx   4.xxx
...
*/

#define MENU_MODE_2         0x02
/*
1.xxx
2.xxxxx
...
*/

typedef int (*SubMenuFunc)(void);

typedef struct{
    int iHideMenuKey;
    SubMenuFunc pHideMenu;
}T_HIDE_MENU;

typedef struct{
    int iHideItemkey;
    int iTransNo;
}T_HIDE_ITEM;

typedef struct _tagDynamicMenu{
    uint8_t ucMenuMode;
    uint8_t ucTotalNum;
    uint8_t ucExsitNum;
    uint8_t ucExistLines;
    int szTrans[MAX_ITEM_MENU+1];
    uint8_t bTitle;
    char szTitle[MAX_CHARS_PERLINE];
    char szMenuItem[MAX_ITEM_MENU+1][MAX_CHARS_PERLINE+1];
    char szDynamicMenu[MAX_ITEM_MENU+1][MAX_CHARS_PERLINE+1];
    SubMenuFunc pSubMenuFunc[MAX_ITEM_MENU+1];
    uint8_t bLastMenu;
    SubMenuFunc pLastMenu;

    uint8_t ucExistHideMenu;
    T_HIDE_MENU stHideMenu[MAX_HIDE_MENU];

    uint8_t ucExistHideItem;
    T_HIDE_ITEM stHideItem[MAX_HIDE_ITEM];
}T_Dynamic_Menu;

 T_Dynamic_Menu gstDynamicMenu;

void InitMenu(uint8_t ucMode, const char * pszTitle);//初始化菜单
void AddMenuTitle(char *pszTitle);//更改或者添加菜单标题
void AddMenuItem(uint8_t bVisible, int iTransNo, char *pszName, SubMenuFunc subMenu);//添加交易显示菜单

int DispDynamicMenu(int flag);//显示动态菜单
void AddHideMenu(int iKey, SubMenuFunc HideMenu);//添加隐藏按键菜单或者功能
void AddHideItem(int iKey, int iTransNo);//添加隐藏按键功能
void AddLastMenu(SubMenuFunc LastMenu);//添加上一级菜单(按取消键返回指定的菜单)
int FindHideMenu(int iKey);
int FindHideItem(int iKey);

int PosManageMenu(void);
int MesesMenu(void);
void MainMenuAddMenuItem(uint8_t bVisible, int iTransNo, char *pszName, SubMenuFunc subMenu);
int MainMenuDispFlushMenu(int iStartKey, int iEndKey, int iTimeOut,int flag);
void MesesAddMenuItem(uint8_t bVisible, int iTransNo, char *pszName, SubMenuFunc subMenu);





#endif
