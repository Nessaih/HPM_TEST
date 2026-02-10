#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "stimer.h"
#include "tbox_shell_if.h"
#include "tbox_mode_if.h"
#include "tbox_check_power.h"

#define TBOX_MODE_MAGICNO 0x5A5A5A5A
#define TBOX_MODE_TIMER_EVENT  "MODE_TIMER_EVENT"
#define TBOX_MODE_INFO_NAME  "TBOX_MODE_INFO"

typedef BOOL (*TBOX_MODE_CAN_SWITCH_FUN)(VOID);
typedef VOID (*TBOX_MODE_SWITCH_FUN)(TBOX_MODE_TYPE new_mode);
typedef VOID (*TBOX_MODE_HANDLE_FUN)(VOID);

typedef struct
{
    UINT32 magicno;
    UINT16 last_mode;
    UINT16 mode;
}TBOX_MODE_INFO;

typedef struct 
{
    TBOX_MODE_HANDLE_FUN handle;    
    TBOX_MODE_CAN_SWITCH_FUN can_switch;
    TBOX_MODE_SWITCH_FUN switch_mode;
}TBOX_MODE_FUN;

static INT32 tbox_mode_init(UINT8 seq);
static VOID tbox_mode_start(VOID);
static VOID tbox_mode_stop(VOID);
static VOID tbox_mode_exit(VOID);
static VOID tbox_mode_timeout_callback(VOID);
static VOID tbox_mode_handle_timer_event(const CHAR *name, TBOX_MSG_DATA *data);
static inline VOID tbox_mode_switch_notify(TBOX_MODE_TYPE new_mode);
static BaseType_t tbox_mode_shell_show_mode(CHAR *buf, UINT32 bufsz, const CHAR *cmd);

static BOOL tbox_mode_factory_can_switch(VOID);
static VOID tbox_mode_factory_hanle(VOID);
static VOID tbox_mode_factory_switch(TBOX_MODE_TYPE new_mode);
static VOID tbox_mode_normal_hanle(VOID);
static VOID tbox_mode_normal_switch(TBOX_MODE_TYPE new_mode);
static BOOL tbox_mode_normal_can_switch(VOID);
static BOOL tbox_mode_maintenance_can_switch(VOID);
static VOID tbox_mode_maintenance_hanle(VOID);
static VOID tbox_mode_maintenance_switch(TBOX_MODE_TYPE new_mode);
static BOOL tbox_mode_undervoltage_can_switch(VOID);
static VOID tbox_mode_undervoltage_hanle(VOID);
static VOID tbox_mode_undervoltage_switch(TBOX_MODE_TYPE new_mode);
static BOOL tbox_mode_emergency_can_switch(VOID);
static VOID tbox_mode_emergency_hanle(VOID);
static VOID tbox_mode_emergency_switch(TBOX_MODE_TYPE new_mode);
static BOOL tbox_mode_abnormal_can_switch(VOID);
static VOID tbox_mode_abnormal_hanle(VOID);
static VOID tbox_mode_abnormal_switch(TBOX_MODE_TYPE new_mode);

static TBOX_MODE_TYPE tbox_current_mode;
static TBOX_MODE_TYPE tbox_last_mode;

static STIMER_ID tbox_mode_timer;
static TBOX_MODE_FUN tbox_mode_fun_table[TBOX_MODE_MAX] = 
{
    {tbox_mode_factory_hanle,     tbox_mode_factory_can_switch,      tbox_mode_factory_switch},
    {tbox_mode_normal_hanle,      tbox_mode_normal_can_switch,       tbox_mode_normal_switch},
    {tbox_mode_maintenance_hanle, tbox_mode_maintenance_can_switch,  tbox_mode_maintenance_switch},
    {tbox_mode_undervoltage_hanle,tbox_mode_undervoltage_can_switch, tbox_mode_undervoltage_switch},
    {tbox_mode_emergency_hanle,   tbox_mode_emergency_can_switch,    tbox_mode_emergency_switch},
    {tbox_mode_abnormal_hanle,    tbox_mode_abnormal_can_switch,     tbox_mode_abnormal_switch},
};

TBOX_MODULE_FUN(TBOXMODE, tbox_mode_init, tbox_mode_stop, tbox_mode_start, NULL_PTR, tbox_mode_exit, NULL_PTR);
TBOX_MODULE(TBOXMODE, TBOX_TASK_PRIORITY_LOW2, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(MODE_SWITCH_NOTIFY, TBOX_MSG_PRIORITY_HIGH, TBOX_MSG_TYPE_TOPIC);
TBOX_MESSSAGE(MODE_TIMER_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_SHELL_DEFINE(showmode, "display current mode", 0, tbox_mode_shell_show_mode);
TBOX_MODULE_LOADER(TBOXMODE)
{
    REGISTRY_TBOX_MESSAGE(MODE_SWITCH_NOTIFY);
    REGISTRY_TBOX_MESSAGE(MODE_TIMER_EVENT);
}

INT32 tbox_mode_switch(TBOX_MODE_TYPE mode)
{
    if(mode >= TBOX_MODE_MAX || 
      tbox_current_mode >= TBOX_MODE_MAX)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    tbox_mode_fun_table[tbox_current_mode].switch_mode(mode);
    return (INT32)TBOX_E_OK;
}

INT32 tbox_mode_get(TBOX_MODE_TYPE *mode)
{
    if(NULL_PTR == mode)
    {
        return (INT32)TBOX_E_INVALID_PARAM;       
    }

    *mode = tbox_current_mode;
    return (INT32)TBOX_E_OK;
}

static INT32 tbox_mode_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            break;

        case MODULE_INIT_SEQ_STORAGE:
            {
                TBOX_MODE_INFO mode_info;
                if((INT32)TBOX_E_OK != tbox_cfg_getkv(TBOX_MODE_INFO_NAME, &mode_info, sizeof(TBOX_MODE_INFO)))
                {
                    mode_info.magicno = TBOX_MODE_MAGICNO;
                    mode_info.mode = TBOX_MODE_FACTORY;
                    mode_info.last_mode = TBOX_MODE_FACTORY;
                    tbox_cfg_setkv(TBOX_MODE_INFO_NAME, &mode_info, sizeof(TBOX_MODE_INFO));
                }
                else
                {
                    if(mode_info.magicno != TBOX_MODE_MAGICNO)
                    {
                        mode_info.magicno = TBOX_MODE_MAGICNO;
                        mode_info.mode = TBOX_MODE_FACTORY;
                        mode_info.last_mode = TBOX_MODE_FACTORY;
                        tbox_cfg_setkv(TBOX_MODE_INFO_NAME, &mode_info, sizeof(TBOX_MODE_INFO));
                    }
                    else
                    {
                        tbox_last_mode = (TBOX_MODE_TYPE)mode_info.last_mode;
                        tbox_current_mode = (TBOX_MODE_TYPE)mode_info.mode;
                    }
                }
            }
            break;

        case MODULE_INIT_SEQ_MODULE:
            {
                TBOX_ID module_id;
                GET_TBOX_MODULE_ID(TBOXMODE, module_id);

                tbox_mode_timer = stimer_create(STIMER_TYPE_PERIOD, tbox_mode_timeout_callback);
                if(STIMER_ID_INVALID == tbox_mode_timer)
                {
                    MODULE_LOG_E(TBOXMODE, "create mode timer failed");
                    return (INT32)TBOX_E_FAILED_INIT;
                }
                tbox_message_add_handler(TBOX_MODE_TIMER_EVENT, module_id, tbox_mode_handle_timer_event);

                TBOX_SHELL_REGISTER(showmode);

                tbox_check_power_init();
            }
            break;

        default:
            break;
    }

    MODULE_LOG_D(TBOXMODE, "tbox mode init seq:%d", seq);

    return (INT32)TBOX_E_OK;
}

static VOID tbox_mode_start(VOID)
{
    if(STIMER_ID_INVALID != tbox_mode_timer)
    {
        stimer_start(tbox_mode_timer, 500U);
    }
    
    tbox_check_power_start();

    MODULE_LOG_D(TBOXMODE, "tbox mode start");
}

static VOID tbox_mode_stop(VOID)
{
    TBOX_MODE_INFO mode_info;
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXMODE, module_id);

    if(STIMER_ID_INVALID != tbox_mode_timer)
    {
        stimer_stop(tbox_mode_timer);
    }
    
    mode_info.magicno = TBOX_MODE_MAGICNO;
    mode_info.mode = tbox_current_mode;
    mode_info.last_mode = tbox_last_mode;
    tbox_cfg_setkv(TBOX_MODE_INFO_NAME, &mode_info, sizeof(TBOX_MODE_INFO));

    tbox_check_power_stop();

    tbox_module_set_state(module_id, TBOX_MODULE_STATE_STOP);

    MODULE_LOG_D(TBOXMODE, "tbox mode stop");
}

static VOID tbox_mode_exit(VOID)
{
    /*NOTHING TO DO*/
    MODULE_LOG_D(TBOXMODE, "tbox mode exit");
}

static VOID tbox_mode_timeout_callback(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXMODE, module_id);

    tbox_message_send(TBOX_MODE_TIMER_EVENT, module_id, module_id, NULL_PTR);   
}

static VOID tbox_mode_handle_timer_event(const CHAR *name, TBOX_MSG_DATA *data)
{
    UNUSED(data);

    if(0U != strncmp(name, TBOX_MODE_TIMER_EVENT, strlen(TBOX_MODE_TIMER_EVENT)))
    {
         return;
    }
    if(tbox_current_mode < TBOX_MODE_MAX)
    {
        tbox_mode_fun_table[tbox_current_mode].handle();
    }

    tbox_check_period();
}

static inline VOID tbox_mode_switch_notify(TBOX_MODE_TYPE new_mode)
{
    TBOX_MSG_DATA data = {.data = (UINT8 *)&new_mode, .size = sizeof(UINT16)};
    tbox_message_publish(TBOX_MODE_SWITCH_NOTIFY, &data);
}

static BaseType_t tbox_mode_shell_show_mode(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    CHAR *mode_str[TBOX_MODE_MAX] = {"FACTORY", "NORMAL", "MAINTENANCE", "UNDERVOLTAGE", "EMERGENCY", "ABNORMAL"};

    UNUSED(cmd);
    
    if(NULL_PTR == buf || 0U == bufsz)
    {
        return pdFALSE;
    }
    if(tbox_current_mode >= TBOX_MODE_MAX ||
       tbox_last_mode >= TBOX_MODE_MAX)
    {
        snprintf(buf, bufsz, "last mode:UNKONWN, current mode:UNKONWN \r\n");
    }
    else
    {
        snprintf(buf, bufsz, "last mode:%s, current mode:%s \r\n", mode_str[tbox_last_mode], mode_str[tbox_current_mode]);
    }

    return pdFALSE;
}

static BOOL tbox_mode_factory_can_switch(VOID)
{
    /*TODO:添加切换到工厂模式的条件判断:
      1、TBOX 出厂时厂家预配置
      2、设备未激活状态
      3、车辆报废，车企后台下发永久停机指令
      4、没发生欠压，异常，紧急事件，升级等情况*/
    return FALSE;    
}

static VOID tbox_mode_factory_hanle(VOID)
{
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_MAINTENANCE].can_switch())
    {
        tbox_mode_factory_switch(TBOX_MODE_MAINTENANCE);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_UNDERVOLTAGE].can_switch())
    {
        tbox_mode_factory_switch(TBOX_MODE_UNDERVOLTAGE);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_ABNORMAL].can_switch())
    {
        tbox_mode_factory_switch(TBOX_MODE_ABNORMAL);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_NORMAL].can_switch())
    {
        tbox_mode_factory_switch(TBOX_MODE_NORMAL);
        return;
    }
}

static VOID tbox_mode_factory_switch(TBOX_MODE_TYPE new_mode)
{
    switch(new_mode)
    {
        case TBOX_MODE_NORMAL:
        case TBOX_MODE_MAINTENANCE:
        case TBOX_MODE_UNDERVOLTAGE:
        case TBOX_MODE_ABNORMAL:
            tbox_last_mode = tbox_current_mode;
            tbox_current_mode = new_mode;
            tbox_mode_switch_notify(tbox_current_mode);
            MODULE_LOG_I(TBOXMODE, "tbox mode switch from %d to %d", tbox_last_mode, tbox_current_mode);
            break;

        default:
            break;
    }
}

static VOID tbox_mode_normal_hanle(VOID)
{
    for(INT8 i = TBOX_MODE_ABNORMAL; i >= 0; i--)
    {
        if(TBOX_MODE_NORMAL == (TBOX_MODE_TYPE)i)
        {
            continue;
        }

        if(TRUE == tbox_mode_fun_table[i].can_switch())
        {
            tbox_mode_normal_switch((TBOX_MODE_TYPE)i);
            return;
        }
    }
}

static VOID tbox_mode_normal_switch(TBOX_MODE_TYPE new_mode)
{
    if(TBOX_MODE_NORMAL == new_mode)
    {
        return;
    }

    tbox_last_mode = tbox_current_mode;    
    tbox_current_mode = new_mode;
    tbox_mode_switch_notify(tbox_current_mode);
    MODULE_LOG_I(TBOXMODE, "tbox mode switch from %d to %d", tbox_last_mode, tbox_current_mode);
}

static BOOL tbox_mode_normal_can_switch(VOID)
{
    /*TODO:添加切换到正常模式的条件判断:
      1、根据项目特定情况，添加相应的条件判断，目前如果获取VIN成功，则认为设备norma模式，否则处于出厂模式
      2、没发生欠压，异常，紧急事件，升级等情况*/
    if(TRUE == tbox_check_power_isundervoltage())
    {
        return FALSE;
    }
    
    CHAR vin[TBOX_CFG_VIN_LEN] = {'\0'};
    TBOX_CFG_ID cfg_id = CFG_ID_INVALID;
    TBOX_CFG_ID_GET(VIN, cfg_id);
    tbox_cfg_read(cfg_id, vin);
    if(strlen(vin) > 0U && 
       strncmp(vin, "0", TBOX_CFG_VIN_LEN-1U) != 0U &&
       strncmp(vin, "00000000000000000", TBOX_CFG_VIN_LEN-1U) != 0U)
    {
        return TRUE;
    }

    return FALSE; 
}

static BOOL tbox_mode_maintenance_can_switch(VOID)
{
    /*TODO:添加切换到维护模式的条件判断:
      1、TBOX收到车企下发升级指令。
      2、手动触发进去维护模式。
      3、不处于低压模式。*/
      if(TRUE == tbox_check_power_isundervoltage())
      {
         return FALSE;
      }
      return FALSE;
}

static VOID tbox_mode_maintenance_hanle(VOID)
{
    for(INT8 i = TBOX_MODE_ABNORMAL; i >= 0; i--)
    {
        if(TBOX_MODE_MAINTENANCE == (TBOX_MODE_TYPE)i)
        {
            continue;
        }

        if(TRUE == tbox_mode_fun_table[i].can_switch())
        {
            tbox_mode_maintenance_switch((TBOX_MODE_TYPE)i);
            return;
        }
    }    
}

static VOID tbox_mode_maintenance_switch(TBOX_MODE_TYPE new_mode)
{
    if(TBOX_MODE_MAINTENANCE == new_mode)
    {
        return;
    }

    tbox_last_mode = tbox_current_mode;    
    tbox_current_mode = new_mode;
    tbox_mode_switch_notify(tbox_current_mode);
    MODULE_LOG_I(TBOXMODE, "tbox mode switch from %d to %d", tbox_last_mode, tbox_current_mode);
}

static BOOL tbox_mode_undervoltage_can_switch(VOID)
{
    /*TODO:添加切换到维护模式的条件判断:
      1、工作电压低于安全电压阈值。*/
      if(TRUE == tbox_check_power_isundervoltage())
      {
          return TRUE;
      }

      return FALSE;
}

static VOID tbox_mode_undervoltage_hanle(VOID)
{
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_NORMAL].can_switch())
    {
        tbox_mode_undervoltage_switch(TBOX_MODE_NORMAL);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_FACTORY].can_switch())
    {
        tbox_mode_undervoltage_switch(TBOX_MODE_FACTORY);
    }
}

static VOID tbox_mode_undervoltage_switch(TBOX_MODE_TYPE new_mode)
{
    switch(new_mode)
    {
        case TBOX_MODE_NORMAL:
        case TBOX_MODE_FACTORY:
            tbox_last_mode = tbox_current_mode;
            tbox_current_mode = new_mode;
            tbox_mode_switch_notify(tbox_current_mode);
            MODULE_LOG_I(TBOXMODE, "tbox mode switch from %d to %d", tbox_last_mode, tbox_current_mode);
            break;

        default:
            break;
    }
}

static BOOL tbox_mode_emergency_can_switch(VOID)
{
    /*TODO:添加切换到紧急模式的条件判断:
      1、车身 ECU 发送碰撞信号
      2、用户手动触发 SOS按键*/

      return FALSE;
}

static VOID tbox_mode_emergency_hanle(VOID)
{
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_NORMAL].can_switch())
    {
        tbox_mode_emergency_switch(TBOX_MODE_NORMAL);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_UNDERVOLTAGE].can_switch())
    {
        tbox_mode_emergency_switch(TBOX_MODE_UNDERVOLTAGE);
    }
}

static VOID tbox_mode_emergency_switch(TBOX_MODE_TYPE new_mode)
{
    switch(new_mode)
    {
        case TBOX_MODE_NORMAL:
        case TBOX_MODE_UNDERVOLTAGE:
            tbox_last_mode = tbox_current_mode;
            tbox_current_mode = new_mode;
            tbox_mode_switch_notify(tbox_current_mode);
            MODULE_LOG_I(TBOXMODE, "tbox mode switch from %d to %d", tbox_last_mode, tbox_current_mode);
            break;

        default:
            break;
    }
}

static BOOL tbox_mode_abnormal_can_switch(VOID)
{
    /*TODO: 添加切换到异常模式的条件判断
       1、软件异常，会触发功能降级，关闭不重要功能*/
    
    return FALSE;
}

static VOID tbox_mode_abnormal_hanle(VOID)
{
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_EMERGENCY].can_switch())
    {
        tbox_mode_abnormal_switch(TBOX_MODE_EMERGENCY);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_MAINTENANCE].can_switch())
    {
        tbox_mode_abnormal_switch(TBOX_MODE_MAINTENANCE);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_NORMAL].can_switch())
    {
        tbox_mode_abnormal_switch(TBOX_MODE_NORMAL);
        return;
    }
    if(TRUE == tbox_mode_fun_table[TBOX_MODE_FACTORY].can_switch())
    {
        tbox_mode_abnormal_switch(TBOX_MODE_FACTORY);
    }
}

static VOID tbox_mode_abnormal_switch(TBOX_MODE_TYPE new_mode)
{
    switch(new_mode)
    {
        case TBOX_MODE_FACTORY:
        case TBOX_MODE_NORMAL:
        case TBOX_MODE_MAINTENANCE:
        case TBOX_MODE_EMERGENCY:
            tbox_last_mode = tbox_current_mode;
            tbox_current_mode = new_mode;
            tbox_mode_switch_notify(tbox_current_mode);
            MODULE_LOG_I(TBOXMODE, "tbox mode switch from %d to %d", tbox_last_mode, tbox_current_mode);
            break;

        default:
            break;
    }
}
