/******************************************************************


*******************************************************************/
#include "posapi.h"
#include "posmacro.h"
#include "terstruc.h"
#include "glbvar.h"
#include "posappapi.h"
#include "glbvar.h"
#include "libsbTms.h"
#include "sapptms.h"



#define  TMS_INFO_FILE		    "tms_info.dat"

TMS_INFO_ST tms_info;

static int save_tms_file(TMS_INFO_ST *ptms_info)
{
	int fd, ret;

	if (access(TMS_INFO_FILE, F_OK) < 0)
	{
		fd = open(TMS_INFO_FILE, O_RDWR|O_CREAT, 0664);
	}
	else
	{
		fd = open(TMS_INFO_FILE, O_RDWR);
	}

	ret = write(fd, ptms_info, sizeof(TMS_INFO_ST));
	close(fd);
	return ret;
}

int get_tms_file(TMS_INFO_ST *ptms_info)
{
	int fd, ret;

    	memset(ptms_info, 0, sizeof(TMS_INFO_ST));
	fd = open(TMS_INFO_FILE, O_RDONLY);
	if(fd < 0)
		return fd;
	
	ret = read(fd, ptms_info, sizeof(TMS_INFO_ST));
	close(fd);
	return ret;
}

void time_add(int hour, int min, int sec, uint8_t *dest)    //�Ե�ǰʱ��Ϊ��׼�Ӽ�ʱ��
{
    time_t NowTime; // ��ǰʱ����
    struct tm *now; // ��ǰʱ��ṹ��
    char buf[20];
    
    NowTime = time(NULL);
    NowTime += hour*3600 + min*60 +sec;
    now = localtime(&NowTime);
    sprintf(buf, "%04d%02d%02d%02d%02d%02d", 
        now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);
    PubAsc2Bcd(buf+2, 12, (char*)dest);
}

static int check_logon_auto(void)  //����Ƿ�Ҫ�Զ�ǩ��
{
	uint8_t nowtime[8],tmp[8],asctime[15],asctmp[15];
	
	int ret;

	sysGetTime(nowtime);    //YYMMDDhhmmss
/*
	if(memcmp(nowtime, tms_info.last_logon, 2) >0)  //YYMMDD��ͬ
	{
		memcpy(tms_info.last_logon,nowtime,6);
	}

	time_add(-24, 0, 0, tmp);   //24h
	BcdToAsc0(asctime,  tms_info.last_logon, 12);
	BcdToAsc0(asctmp,  nowtime+3, 2);

	PrintDebug("%s %s", "tms_info.last_logon",asctime);
	PrintDebug("%s %s", "nowtime",asctmp);
*/	
	//24ʱ�Ժ����
	if(nowtime[3] ==0x24)
		ret = 1;
	else
		ret = 0;
/*
	time_add(0, -1, 0, tmp);   //10 min xxxxxxxxxxxxxx test hlh

	if(memcmp(tmp, tms_info.last_logon, 6) < 0)
		ret = 0;
	else
		ret = 1;
*/
#ifdef ALIPAY_BUG
	printf("check_logon_auto()=%d\n", ret);
#endif
	return ret;
}


static void update_logon_time(void)
{
    tms_info.retry_cnt = 0;
    sysGetTime(tms_info.last_logon);
    save_tms_file(&tms_info);
}
static void disp_err_msg(char *msg)
{
    lcdClrLine(2,11);
    lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, msg);
    lcdFlip();
    ErrorBeep();
    kbGetKeyMs(10000);
}

int query_down_flag(void)
{
    int ret;
    char ver_info[256];
    TRANS_RET_CODE RetCode=0;
    APP_UPDATE_TYPE pAppUpdateType=0;
    static int req_cnt = 0;

    if(tms_info.down_falg == DOWN_FALG_QUERY)   //����Ѳ�������û����
    {
        if(req_cnt > 30)
        {
            tms_info.down_falg = DOWN_FALG_NONE;
            save_tms_file(&tms_info);
            req_cnt = 0;
            return -1;
        }
        
        memset(ver_info, 0, sizeof(ver_info));
        ret = Check_Auto_Inspect_Result(&RetCode, &pAppUpdateType,ver_info);
        if(ret == 0)
        {
            if(RetCode == Trans_Done)           //Ѳ�������
            {
                tms_info.task_flag = pAppUpdateType;
                tms_info.down_falg = DOWN_FALG_READY;
                strcpy(tms_info.ver_info, ver_info);
		   update_logon_time();
                req_cnt = 0;
                return 1;
            }
            else if(RetCode == Trans_Error)        //Ѳ��ʧ��
            {
                tms_info.down_falg = DOWN_FALG_NONE;
                save_tms_file(&tms_info);
            }
            req_cnt++;
            return 0;
        }
        req_cnt++;
        return -2;
    }
    req_cnt = 0;
    return 0;
}

int check_ppp_connected(void)
{
    int ret;

    if(stPosParam.stTxnCommCfg.ucCommType!=CT_GPRS && stPosParam.stTxnCommCfg.ucCommType!=CT_CDMA)
        return 1;
 
	ret = WirelessDial(PPP_ONLY_MODE);
	if(ret == 0)
	{
	    return 1;
	}
    return 0;
}

int tms_process(int update_now)
{
    int ret,ppp_connected=0;
    TRANS_RET_CODE RetCode=0;
    APP_UPDATE_TYPE pAppUpdateType=0;
//	int iRet;


	if(check_logon_auto())  //24Сʱûǩ����
	{
		ret = Check_Download_App_Result(&RetCode, &pAppUpdateType);
		if(ret != 0)  //del || RetCode==Trans_Error
		    return -3;
		if(RetCode == Trans_Done)       //���������
		{
		    tms_info.down_falg = DOWN_FALG_DONE;
		    save_tms_file(&tms_info);
		}
		else
		{	
			NoDispTmsPrecessingFlag = FALSE;
		    ppp_connected = check_ppp_connected();
			NoDispTmsPrecessingFlag = TRUE;
		    if(ppp_connected != 1)
		    {
		        return -1;
		    }
		    tms_info.down_falg = DOWN_FALG_QUERY;
		    save_tms_file(&tms_info);
		    update_logon_time();
		    ret = Start_Auto_Inspect_Now(&RetCode);     //Ѳ��
		    if(ret!=0 || RetCode!=Accept_Trans)
		        return -1;
		}
	}
            
	
    if(tms_info.down_falg == DOWN_FALG_DONE)        //���������
    {
        goto done;
    }
    ret = query_down_flag();
    if(ret < 0)
        return -2;

    if((update_now==0) && (tms_info.task_flag==TASK_FLAG_NONE || tms_info.task_flag>TASK_FLAG_AUTO))
        return 0;
	printf("========���ߵ����˵������������======\n");
//���ߵ����˵������������
    if(tms_info.retry_cnt > 10)     //��ѯ��10�ζ���5��8
        return 0;
	//NETPAY ����ǰ������н���(��Ҫ��Ԥ��Ȩ��ɱ�������)
/*	iRet = PosSettleTran();
	if(iRet != OK )
	{
		return iRet;
	}
 */   ret = Check_Download_App_Result(&RetCode, &pAppUpdateType);
    //printf("Check_Download_App_Result()=%d,RetCode=%d,pAppUpdateType=%d\n",ret,RetCode,pAppUpdateType);
    if(ret != 0)  //del || RetCode==Trans_Error
        return -3;
    if(RetCode == Trans_Done)       //���������
    {
    	memset(PosComconTrol.AutoUpdateFlag,0,sizeof(PosComconTrol.AutoUpdateFlag));
        tms_info.down_falg = DOWN_FALG_DONE;
        save_tms_file(&tms_info);
    }

	
    if(tms_info.down_falg <= DOWN_FALG_DOING)
    {
        if(RetCode != Trans_Doing)
        {
            tms_info.retry_cnt++;
        }
        if(tms_info.retry_cnt > 10)
        {
            save_tms_file(&tms_info);
            return 1;
        }
        
        if(ppp_connected != 1)
            ppp_connected = check_ppp_connected();
        if(ppp_connected != 1)
        {
            return -1;
        }
        
        tms_info.down_falg = DOWN_FALG_DOING;
        save_tms_file(&tms_info);
        ret = Start_Download_App_Now(&RetCode);    //����
        //printf("Start_Download_App_Now()=%d,RetCode=%d\n",ret,RetCode);
        if(ret!=0 || RetCode!=Accept_Trans)
            return -4;
    }

done:
    if(tms_info.down_falg == DOWN_FALG_DONE)        //���������
    {
        if(SelectDownloadExit(1)==E_APP_RESET)
        {
            ret = Update_App_Doanload_File();           //���³���
 //           printf("Update_App_Doanload_File()=%d\n",ret);
            if(ret != 0)
            {
                Clr_Tmp_App_Doanload_File();    //��������ص��ļ�������
                tms_info.task_flag = TASK_FLAG_NONE;
                tms_info.down_falg = DOWN_FALG_NONE;
                save_tms_file(&tms_info);
                return -5;
            }
            tms_info.task_flag = TASK_FLAG_NONE; //TASK_FLAG_RST
            tms_info.down_falg = DOWN_FALG_NONE;
            save_tms_file(&tms_info);
            return E_APP_RESET;
        }
    }

    return 0;    
}


int down_now(void)
{
    int ret,cnt,ppp_connected=0;
    TRANS_RET_CODE RetCode=0;
    APP_UPDATE_TYPE pAppUpdateType=0;
    
    lcdCls();
    lcdDisplay(0, 0, DISP_CFONT|DISP_MEDIACY|DISP_INVLINE, "VERSION UPDATE");
    lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "QUERY VERSION MSG");
    lcdFlip();
    
    if((tms_info.task_flag==TASK_FLAG_NONE) || (tms_info.task_flag>TASK_FLAG_AUTO))
    {
        ret = Check_Download_App_Result(&RetCode, &pAppUpdateType);
        if(ret != 0)  //del || RetCode==Trans_Error
        {
            disp_err_msg("QUERY RESULT FAIL-1");
            return -1;
        }
        if(RetCode == Trans_Done)       //���������
        {
            tms_info.down_falg = DOWN_FALG_DONE;
            save_tms_file(&tms_info);
        }
        else
        {
            ppp_connected = check_ppp_connected();
            if(ppp_connected != 1)
            {
                return -1;
            }
            
            tms_info.down_falg = DOWN_FALG_QUERY;
            save_tms_file(&tms_info);
            ret = Start_Auto_Inspect_Now(&RetCode);     //Ѳ��
            if(ret!=0 || RetCode!=Accept_Trans)
            {
                disp_err_msg("AUTO INSPECT FAIL-2");
                return -2;
            }
            sleep(1);
            
            cnt = 0;
            do{
                ret = query_down_flag();
                if(ret == 1)
                    break;
                else if(ret < 0)
                {
                    disp_err_msg("AUTO INSPECT FAIL-3");
                    return -3;
                }
                sleep(1);
            }while(cnt++ < 30);
			update_logon_time();
        }
    }
    
    if(tms_info.down_falg == DOWN_FALG_DONE)        //���������
    {
        goto done;
    }
    
    if((tms_info.task_flag==TASK_FLAG_NONE) || (tms_info.task_flag>TASK_FLAG_AUTO))
    {
        lcdClrLine(2,11);
        lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY, "NEWEST VERSION");
        lcdDisplay(0, 6, DISP_CFONT|DISP_MEDIACY, "DONT UPDATE");
        lcdFlip();
        ErrorBeep();
        kbGetKeyMs(3000);
        return 0;
    }
    
//���ߵ����˵������������
    ret = Check_Download_App_Result(&RetCode, &pAppUpdateType);
//    DEBUG("Check_Download_App_Result()=%d,RetCode=%d,pAppUpdateType=%d\n",ret,RetCode,pAppUpdateType);
    if(ret != 0)  //del || RetCode==Trans_Error
    {
        disp_err_msg("��ѯ���ؽ��ʧ��-4");
        return -4;
    }
    if(RetCode == Trans_Done)       //���������
    {
        tms_info.down_falg = DOWN_FALG_DONE;
        save_tms_file(&tms_info);
    }
    
    if(tms_info.down_falg <= DOWN_FALG_DOING)
    {
        if(ppp_connected != 1)
            ppp_connected = check_ppp_connected();
        if(ppp_connected != 1)
        {
            return -1;
        }
        
        tms_info.down_falg = DOWN_FALG_DOING;
        save_tms_file(&tms_info);
        ret = Start_Download_App_Now(&RetCode);    //����
//        DEBUG("Start_Download_App_Now()=%d,RetCode=%d\n",ret,RetCode);
        if(ret!=0 || RetCode!=Accept_Trans)
        {
            disp_err_msg("DOWNLOAD FAIL-5");
            return -5;
        }
        
        lcdDisplay(0, 4, DISP_CFONT|DISP_MEDIACY|DISP_FLIP, "DOWNLOAD WAIT...");
        cnt = 0;
        do{
            ret = Check_Download_App_Result(&RetCode, &pAppUpdateType);
//            DEBUG("Check_Download_App_Result()=%d,RetCode=%d,pAppUpdateType=%d\n",ret,RetCode,pAppUpdateType);
            if(ret!=0 || RetCode==Trans_Error)
            {
                disp_err_msg("QUERY DOWNLOAD FAIL-6");
                return -6;
            }
            else if(RetCode == Trans_Done)       //���������
            {
                tms_info.down_falg = DOWN_FALG_DONE;
                save_tms_file(&tms_info);
                break;
            }
            else if(RetCode != Trans_Doing)
            {
                cnt++;
                sleep(1);
            }
        } while(cnt < 60);
    }

done:
    if(tms_info.down_falg == DOWN_FALG_DONE)        //���������
    {
        if(SelectDownloadExit(1)==E_APP_RESET)
        {
            ret = Update_App_Doanload_File();           //���³���
 //           DEBUG("Update_App_Doanload_File()=%d\n",ret);
            if(ret != 0)
            {
                Clr_Tmp_App_Doanload_File();    //��������ص��ļ�������
                tms_info.task_flag = TASK_FLAG_NONE;
                tms_info.down_falg = DOWN_FALG_NONE;
                save_tms_file(&tms_info);
                disp_err_msg("PROGRAM UPDATE FAIL-5");
                return -5;
            }
            tms_info.task_flag = TASK_FLAG_NONE; //TASK_FLAG_RST
            tms_info.down_falg = DOWN_FALG_NONE;
            save_tms_file(&tms_info);
            return E_APP_RESET;
        }
    }

    return 0;    
}
