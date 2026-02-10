#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_pm_if.h"
#include "tbox_phm_if.h"
#include "tbox_phm_4g.h"
#include "tbox_pm_io.h"
#include "4g_if.h"

#define TBOX_PHM_MN_RETRY_COUNT              (3U)
#define TBOX_PHM_MN_4G_TESTALIVE_TIME        (180U)   /*180S*/
#define TBOX_PHM_MN_4G_NETREG_TIME           (180U)   /*180S*/
#define TBOX_PHM_MN_4G_NETSTATE_TIME         (1800U)  /*1800S*/
#define TBOX_PM_MN_APN_COUNT                 (3U)
#define TBOX_PM_MN_CONNECT_COUNT             (5U)
#define TBOX_PHM_MN_4G_START_RECOVER_TIME    (7U)    /*7S*/
#define TBOX_PHM_MN_4G_NETREG_RECOVER_TIME   (2U)    /*2S*/
#define TBOX_PHM_MN_4G_NETSTATE_RECOVER_TIME (210U)  /*210S*/

typedef struct
{
    UINT8 start_flag;
    UINT8 retry_count;
    UINT16 mn_time;
}TBOX_PHM_MN;

typedef struct
{
    UINT8 reg_value;
    UINT8 used;
}TBOX_PHM_MN_REGINFO;

static VOID tbox_phm_mn_4gtestalive_begin(VOID);
static VOID tbox_phm_mn_4gtestalive_end(VOID);
static BOOL tbox_phm_mn_4gtestalive_check(VOID);
static VOID tbox_phm_mn_4gnetreg_begin(VOID);
static VOID tbox_phm_mn_4gnetreg_end(VOID);
static BOOL tbox_phm_mn_4gnetreg_check(VOID);
static VOID tbox_phm_mn_4gnetstate_begin(VOID);
static VOID tbox_phm_mn_4gnetstate_end(VOID);
static BOOL tbox_phm_mn_4gnetstate_check(VOID);

static SemaphoreHandle_t tbox_phm_4g_mutex;
static UINT8 tbox_phm_mn_4g_recover_flag;
static UINT16 tbox_phm_mn_4g_recover_time;
static TBOX_PHM_MN tbox_phm_mn_4g_testalive;
static TBOX_PHM_MN tbox_phm_mn_4g_netreg;
static TBOX_PHM_MN tbox_phm_mn_4g_netstate;
static TBOX_PHM_MN_REGINFO tbox_phm_mn_apn_reginfo[TBOX_PM_MN_APN_COUNT];
static TBOX_PHM_MN_REGINFO tbox_phm_mn_connect_reginfo[TBOX_PM_MN_CONNECT_COUNT];

INT32 tbox_phm_4g_init(VOID)
{
    UINT8 index;

    tbox_phm_mn_4g_testalive.start_flag = 0U;
    tbox_phm_mn_4g_testalive.retry_count = 0U;
    tbox_phm_mn_4g_testalive.mn_time = 0U;
    tbox_phm_mn_4g_netreg.mn_time = 0U;
    tbox_phm_mn_4g_netreg.retry_count = 0U;
    tbox_phm_mn_4g_netreg.start_flag = 0U;
    tbox_phm_mn_4g_netstate.mn_time = 0U;
    tbox_phm_mn_4g_netstate.retry_count = 0U;
    tbox_phm_mn_4g_netstate.start_flag = 0U;
    tbox_phm_mn_4g_recover_flag = 0U;
    tbox_phm_mn_4g_recover_time = 0U;

    for(index = 0; index < TBOX_PM_MN_APN_COUNT; index++)
    {
        tbox_phm_mn_apn_reginfo[index].reg_value = 0U;
        tbox_phm_mn_apn_reginfo[index].used = 0U;
    }
    for(index = 0; index < TBOX_PM_MN_CONNECT_COUNT; index++)
    {
        tbox_phm_mn_connect_reginfo[index].reg_value = 0U;
        tbox_phm_mn_connect_reginfo[index].used = 0U;
    }

    tbox_phm_4g_mutex = xSemaphoreCreateMutex();
    if(tbox_phm_4g_mutex == NULL_PTR)
    {
        MODULE_LOG_E(TBOXPHM, "create mutex fail");
        return (INT32)TBOX_E_FAILED_INIT;
    }

    return (INT32)TBOX_E_OK;
}

VOID tbox_phm_4g_start(VOID)
{
    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        tbox_phm_mn_4gnetreg_begin();
        tbox_phm_mn_4gnetstate_begin();
        tbox_phm_mn_4gtestalive_begin();
        tbox_phm_mn_4g_recover_flag = 0U;
        tbox_phm_mn_4g_recover_time = 0U;        
    }
    xSemaphoreGive(tbox_phm_4g_mutex);
}

VOID tbox_phm_4g_stop(VOID)
{
    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        tbox_phm_mn_4gtestalive_end();
        tbox_phm_mn_4gnetreg_end();           
        tbox_phm_mn_4gnetstate_end();
    }
    xSemaphoreGive(tbox_phm_4g_mutex);
}

VOID tbox_phm_4g_period(VOID)
{
    BOOL do_recovery = FALSE;

    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        /*正在恢复中，不需要监控*/
        if(1U == tbox_phm_mn_4g_recover_flag)
        {
            MODULE_LOG_I(TBOXPHM, "wait[%u S] for recovery end", tbox_phm_mn_4g_recover_time);
            if(tbox_phm_mn_4g_recover_time > 0)
            {
                tbox_phm_mn_4g_recover_time--;
            }
            if(tbox_phm_mn_4g_recover_time > 0)
            {
                xSemaphoreGive(tbox_phm_4g_mutex);
                return;
            }
            tbox_phm_mn_4g_recover_flag = 0U;
        }

        /*1、监控4G心跳，如果连续180秒未收到心跳，开机4G模块*/
        if(FALSE == tbox_phm_mn_4gtestalive_check())
        {
            if(tbox_phm_mn_4g_testalive.mn_time > TBOX_PHM_MN_4G_TESTALIVE_TIME)
            {
                tbox_phm_mn_4g_testalive.retry_count++;
                tbox_phm_mn_4g_testalive.mn_time = 0U;
                tbox_phm_mn_4g_netreg.mn_time = 0U;
                tbox_phm_mn_4g_netstate.mn_time = 0U;
                do_recovery = TRUE;
            }
            xSemaphoreGive(tbox_phm_4g_mutex);
            if(TRUE == do_recovery)
            {
                MODULE_LOG_E(TBOXPHM, "4g test alive abonomal, startup 4g");
                tbox_pm_4g_startup();
                if_4g_reset();
                tbox_phm_mn_4g_recover_flag = 1U;
                tbox_phm_mn_4g_recover_time = TBOX_PHM_MN_4G_START_RECOVER_TIME;
            }
            return;
        }
        /*2、监控4G网络注册，如果连续180秒未注册，重启4G*/
        if(FALSE == tbox_phm_mn_4gnetreg_check())
        {
            if(tbox_phm_mn_4g_netreg.mn_time > TBOX_PHM_MN_4G_NETREG_TIME)
            {
                tbox_phm_mn_4g_netreg.retry_count++;
                tbox_phm_mn_4g_netreg.mn_time = 0U;
                tbox_phm_mn_4g_netstate.mn_time = 0U;
                do_recovery = TRUE;                
            }
            xSemaphoreGive(tbox_phm_4g_mutex);
            if(TRUE == do_recovery)
            {
                MODULE_LOG_E(TBOXPHM, "4g network register abonomal, restart 4g");
                if(TBOX_E_OK != tbox_pm_reboot(TBOX_PM_REBOOT_4G))
                {
                    MODULE_LOG_E(TBOXPHM, "restart 4g fail");
                }
                else
                {
                    tbox_phm_mn_4g_recover_flag = 1U;
                    tbox_phm_mn_4g_recover_time = TBOX_PHM_MN_4G_NETREG_RECOVER_TIME;
                }
            }
            return;
        }
        /*3、监控4G网络状态，如果连续1800秒未正常，重启4G和MCU*/
        if(FALSE == tbox_phm_mn_4gnetstate_check())
        {
            if(tbox_phm_mn_4g_netstate.mn_time > TBOX_PHM_MN_4G_NETSTATE_TIME)
            {
                tbox_phm_mn_4g_netstate.retry_count++;
                tbox_phm_mn_4g_netstate.mn_time = 0U;
                do_recovery = TRUE;
            }
            xSemaphoreGive(tbox_phm_4g_mutex);
            if(TRUE == do_recovery)
            {
                MODULE_LOG_E(TBOXPHM, "4g network state abonomal, restart 4g and mcu");
                if(TBOX_E_OK != tbox_pm_reboot(TBOX_PM_REBOOT_4G_MCU))
                {
                    MODULE_LOG_E(TBOXPHM, "restart 4g and mcu fail");
                }
                else
                {
                    tbox_phm_mn_4g_recover_flag = 1U;
                    tbox_phm_mn_4g_recover_time = TBOX_PHM_MN_4G_NETSTATE_RECOVER_TIME;
                }
            }
            return;
        }
    }
    xSemaphoreGive(tbox_phm_4g_mutex);
}

INT32 tbox_phm_add_apn_monitor(UINT8 apn_index)
{
    UINT8 index;

    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        for(index = 0; index < TBOX_PM_MN_APN_COUNT; index++)
        {
            if(1U == tbox_phm_mn_apn_reginfo[index].used &&
               tbox_phm_mn_apn_reginfo[index].reg_value == apn_index)
            {
                xSemaphoreGive(tbox_phm_4g_mutex);
                return (INT32)TBOX_E_OK;
            }
        }
        for(index = 0; index < TBOX_PM_MN_APN_COUNT; index++)
        {
            if(0U == tbox_phm_mn_apn_reginfo[index].used)
            {
                tbox_phm_mn_apn_reginfo[index].reg_value = apn_index;
                tbox_phm_mn_apn_reginfo[index].used = 1U;
                xSemaphoreGive(tbox_phm_4g_mutex);
                return (INT32)TBOX_E_OK;
            }
        }
    }
    xSemaphoreGive(tbox_phm_4g_mutex);

    return (INT32)TBOX_E_FAILED;
}

VOID tbox_phm_remove_apn_monitor(UINT8 apn_index)
{
    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        for(UINT8 index = 0; index < TBOX_PM_MN_APN_COUNT; index++)
        {
            if(1U == tbox_phm_mn_apn_reginfo[index].used &&
               tbox_phm_mn_apn_reginfo[index].reg_value == apn_index)
            {
                tbox_phm_mn_apn_reginfo[index].used = 0U;
                break;
            }
        }
    }
    xSemaphoreGive(tbox_phm_4g_mutex);
}

INT32 tbox_phm_add_connect_monitor(UINT8 conn_no)
{
    UINT8 index;
    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        for(index = 0; index < TBOX_PM_MN_CONNECT_COUNT; index++)
        {
            if(1U == tbox_phm_mn_connect_reginfo[index].used &&
               tbox_phm_mn_connect_reginfo[index].reg_value == conn_no)
            {
                xSemaphoreGive(tbox_phm_4g_mutex);
                return (INT32)TBOX_E_OK;
            }
        }
        for(index = 0; index < TBOX_PM_MN_CONNECT_COUNT; index++)
        {
            if(0U == tbox_phm_mn_connect_reginfo[index].used)
            {
                tbox_phm_mn_connect_reginfo[index].reg_value = conn_no;
                tbox_phm_mn_connect_reginfo[index].used = 1U;
                xSemaphoreGive(tbox_phm_4g_mutex);
                return (INT32)TBOX_E_OK;
            }
        }
    }
    xSemaphoreGive(tbox_phm_4g_mutex);

    return (INT32)TBOX_E_FAILED;
}

VOID tbox_phm_remove_connect_monitor(UINT8 conn_no)
{
    xSemaphoreTake(tbox_phm_4g_mutex, portMAX_DELAY);
    {
        for(UINT8 index = 0; index < TBOX_PM_MN_CONNECT_COUNT; index++)
        {
            if(1U == tbox_phm_mn_connect_reginfo[index].used &&
               tbox_phm_mn_connect_reginfo[index].reg_value == conn_no)
            {
                tbox_phm_mn_connect_reginfo[index].used = 0U;
                break;
            }
        }
    }
    xSemaphoreGive(tbox_phm_4g_mutex);
}

static VOID tbox_phm_mn_4gtestalive_begin(VOID)
{
    tbox_phm_mn_4g_testalive.start_flag = 1U;
    tbox_phm_mn_4g_testalive.retry_count = 0U;
    tbox_phm_mn_4g_testalive.mn_time = 0U;
}

static VOID tbox_phm_mn_4gtestalive_end(VOID)
{
    tbox_phm_mn_4g_testalive.start_flag = 0U;
    tbox_phm_mn_4g_testalive.mn_time = 0U;
}

static BOOL tbox_phm_mn_4gtestalive_check(VOID)
{
    if(0U == tbox_phm_mn_4g_testalive.start_flag || 
       tbox_phm_mn_4g_testalive.retry_count >= TBOX_PHM_MN_RETRY_COUNT)
    {
        return TRUE;
    }

    if(0U == if_4g_get_testalivefailcount())
    {
        tbox_phm_mn_4g_testalive.mn_time = 0U;
        return TRUE;
    }

    tbox_phm_mn_4g_testalive.mn_time++;
    MODULE_LOG_W(TBOXPHM, "4g network test alive abnormal, time[%u S] retry:%d", tbox_phm_mn_4g_testalive.mn_time, 
                 tbox_phm_mn_4g_testalive.retry_count);    
    return FALSE;    
}

static VOID tbox_phm_mn_4gnetreg_begin(VOID)
{
    tbox_phm_mn_4g_netreg.start_flag = 1U;
    tbox_phm_mn_4g_netreg.retry_count = 0U;
    tbox_phm_mn_4g_netreg.mn_time = 0U;
}

static VOID tbox_phm_mn_4gnetreg_end(VOID)
{
    tbox_phm_mn_4g_netreg.start_flag = 0U;
    tbox_phm_mn_4g_netreg.mn_time = 0U;
}

static BOOL tbox_phm_mn_4gnetreg_check(VOID)
{
#define TBOX_PHM_MN_4G_IS_REG(reg_value) (IF_4G_REGISTERED == reg_value || IF_4G_ROAMING == reg_value)

    if(0U == tbox_phm_mn_4g_netreg.start_flag ||
       tbox_phm_mn_4g_netreg.retry_count >= TBOX_PHM_MN_RETRY_COUNT)
    {
        return TRUE;
    }

    UINT8 cregvalue = if_4g_get_reg_state();
    UINT8 gprsvalue = if_4g_get_gprs_state();
    UINT8 ceregvalue = if_4g_get_cereg_state();
    if(TBOX_PHM_MN_4G_IS_REG(cregvalue) ||
       TBOX_PHM_MN_4G_IS_REG(gprsvalue) ||
       TBOX_PHM_MN_4G_IS_REG(ceregvalue))
    {
        tbox_phm_mn_4g_netreg.mn_time = 0U;
        return TRUE;
    }

    tbox_phm_mn_4g_netreg.mn_time++;
    MODULE_LOG_W(TBOXPHM, "4g network register abnormal, time[%u S] retry_count:%d", tbox_phm_mn_4g_netreg.mn_time, 
                 tbox_phm_mn_4g_netreg.retry_count);    
    return FALSE;
}

static VOID tbox_phm_mn_4gnetstate_begin(VOID)
{
    tbox_phm_mn_4g_netstate.start_flag = 1U;
    tbox_phm_mn_4g_netstate.retry_count = 0U;
    tbox_phm_mn_4g_netstate.mn_time = 0U;
}

static VOID tbox_phm_mn_4gnetstate_end(VOID)
{
    tbox_phm_mn_4g_netstate.start_flag = 0U;
    tbox_phm_mn_4g_netstate.mn_time = 0U;
}

static BOOL tbox_phm_mn_4gnetstate_check(VOID)
{
    UINT8 index;
    BOOL apn_empty = TRUE;
    BOOL connect_empty = TRUE;

    if(0U == tbox_phm_mn_4g_netstate.start_flag ||
       tbox_phm_mn_4g_netstate.retry_count >= TBOX_PHM_MN_RETRY_COUNT)
    {
        return TRUE;
    }

    for(index = 0; index < TBOX_PM_MN_APN_COUNT; index++)
    {
        if(1U == tbox_phm_mn_apn_reginfo[index].used)
        {
            apn_empty = FALSE;
            if(IF_4G_STATE_CONNECTED == if_4g_get_call_state(tbox_phm_mn_apn_reginfo[index].reg_value))
            {
                tbox_phm_mn_4g_netstate.mn_time = 0U;
                return TRUE;
            }
        }
    }
    for(index = 0; index < TBOX_PM_MN_CONNECT_COUNT; index++)
    {
        if(1U == tbox_phm_mn_connect_reginfo[index].used)
        {
            connect_empty = FALSE;
            if(IF_4G_STATE_CONNECTED == if_4g_get_socket_conn_state(tbox_phm_mn_connect_reginfo[index].reg_value))
            {
                tbox_phm_mn_4g_netstate.mn_time = 0U;
                return TRUE;
            }
        }
    }
    if(TRUE == apn_empty && TRUE == connect_empty)
    {
        tbox_phm_mn_4g_netstate.mn_time = 0U;
        return TRUE;
    }
    tbox_phm_mn_4g_netstate.mn_time++;
    MODULE_LOG_W(TBOXPHM, "4g network state abnormal, time[%u S] retry_count:%d", tbox_phm_mn_4g_netstate.mn_time,
                 tbox_phm_mn_4g_netstate.retry_count);
    return FALSE;
}