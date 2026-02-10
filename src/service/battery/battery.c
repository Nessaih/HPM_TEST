#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tbox_core.h"
#include "battery_if.h"
#include "analog_if.h"
#include "tbox_cfg_if.h"
#include "drv_pin.h"
#include "tbox_pm_io.h"
#include "tbox_log.h"
#include "tbox_shell_if.h"
/** 
 * 电池充电条件：硬件唤醒有效、有主电(主电大于6V)、电池温度-20C~+60C、电池电压<4.35V
 */

static INT32 battery_init(UINT8 seq);
static VOID battery_stop(VOID);
static VOID battery_task(void *param);

TBOX_MODULE_FUN(BATTERY, battery_init, battery_stop, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR);
TBOX_RUNLOOP_MODULE(BATTERY, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_INFO, TBOX_TASK_SMALL_STACK_SIZE, battery_task);
TBOX_MODULE_LOADER(BATTERY) {}



#define BATTERY_MGR_PERIOD_MS              (1000U)
#define BATTERY_MAIN_PWR_MIN_MV            (6000U)
#define BATTERY_CAP_VOLTAGE_LIMIT_MV       (4800U)
#define BATTERY_BAT_TEMP_MIN_C             (-20.0)
#define BATTERY_BAT_TEMP_MAX_C             (60.0)
#define BATTERY_BAT_VOLTAGE_LIMIT_MV       (4350U)

typedef enum {
    BATTERY_CHARGE_TYPE_BAT = 0,  /* 电池充电模式 */
    BATTERY_CHARGE_TYPE_CAP,      /* 电容充电模式 */
    BATTERY_CHARGE_TYPE_COUNT
} battery_charge_type_t;

static TBOX_ID battery_module_id;
static battery_charge_type_t  charge_type = BATTERY_CHARGE_TYPE_BAT;

static int32_t battery_temperature_get_c(void)
{
    return analog_bat_tmp();
}

static int32_t battery_voltage_get_mv(void)
{
    return analog_bat_vtg();
}

static int32_t battery_main_power_get_mv(void)
{
    return analog_pwr_vtg();
}

static bool battery_acc_is_active(void)
{
    return tbox_pm_io_acc_is_active();
}

static bool battery_main_power_is_ok(void)
{
    return (battery_main_power_get_mv() >= (int32_t)BATTERY_MAIN_PWR_MIN_MV);
}

static void battery_charge_hw_start(void)
{
    drv_pin_set_level(PIN_BAT_CHARGE, 1U);
}

static void battery_charge_hw_stop(void)
{
    drv_pin_set_level(PIN_BAT_CHARGE, 0U);
}

static bool battery_charge_hw_is_on(void)
{
    return (drv_pin_get_level(PIN_BAT_CHARGE) != 0);
}

bool battery_is_charging(void)
{
    return battery_charge_hw_is_on();
}

static BaseType_t show_bat_info(char *buf, size_t bufsz, const char *cmd)
{
    int32_t len = 0;
    (void)cmd;

    len += snprintf(buf + len, bufsz - len, "\r\n=== Battery Status ===\r\n");

    len += snprintf(buf + len, bufsz - len, "Type       : %s\r\n", charge_type == BATTERY_CHARGE_TYPE_BAT ? "Battery" : "Capacitor");
    len += snprintf(buf + len, bufsz - len, "Voltage    : %d mV\r\n", analog_bat_vtg());
    len += snprintf(buf + len, bufsz - len, "Temperature: %d ℃\r\n", analog_bat_tmp());
    len += snprintf(buf + len, bufsz - len, "Charging   : %s\r\n", battery_is_charging() ? "On" : "Off");

    len += snprintf(buf + len, bufsz - len, "==========================\r\n");

    return pdFALSE;
}

TBOX_SHELL_DEFINE(showbat, "show battery info", 0, show_bat_info);

static bool battery_charge_bat_can_charge(void)
{
    const double temperature = (double)battery_temperature_get_c();

    return (battery_acc_is_active() &&
            battery_main_power_is_ok() &&
            (temperature > BATTERY_BAT_TEMP_MIN_C) &&
            (temperature < BATTERY_BAT_TEMP_MAX_C) &&
            (battery_voltage_get_mv() < (int32_t)BATTERY_BAT_VOLTAGE_LIMIT_MV));    
}

static bool battery_charge_cap_can_charge(void)
{
    return (battery_acc_is_active() &&
            battery_main_power_is_ok() &&
            (battery_voltage_get_mv() < (int32_t)BATTERY_CAP_VOLTAGE_LIMIT_MV));
}

static void battery_charge_type_init_from_cfg(void)
{
    uint8_t    type   = 0;
    TBOX_CFG_ID cfg_id = CFG_ID_INVALID;

    TBOX_CFG_ID_GET(BATTYPE, cfg_id);

    if (cfg_id != (TBOX_CFG_ID)CFG_ID_INVALID && cfg_id < TBOX_CFG_ITEM_NUMBER) {
        if (tbox_cfg_read(cfg_id, &type) >= 0) {
            charge_type = (type == 1U) ? BATTERY_CHARGE_TYPE_CAP : BATTERY_CHARGE_TYPE_BAT;
            return;
        }
    }

    charge_type = BATTERY_CHARGE_TYPE_BAT;
}

static void battery_charge_mgr_1s(void)
{
    bool can_charge = false;

    if (charge_type == BATTERY_CHARGE_TYPE_BAT) {
        can_charge = battery_charge_bat_can_charge();
    } else {
        can_charge = battery_charge_cap_can_charge();
    }

    if (can_charge) {
        if (!battery_charge_hw_is_on()) {
            battery_charge_hw_start();
            MODULE_LOG_I(BATTERY, "%s charge started!", 
                        charge_type == BATTERY_CHARGE_TYPE_BAT ? "Battery" : "Capacitor");
        }
    } else {
        if (battery_charge_hw_is_on()) {
            battery_charge_hw_stop();
            MODULE_LOG_I(BATTERY, "%s charge stopped! [vol=%dmV]", 
                        charge_type == BATTERY_CHARGE_TYPE_BAT ? "Battery" : "Capacitor",
                        battery_voltage_get_mv());
        }
    }
}

static void battery_task(void *param)
{
    TickType_t last_wake = xTaskGetTickCount();
    (void)param;

    for (;;) {
        if(tbox_module_get_state(battery_module_id) != TBOX_MODULE_STATE_START)
        {
            MODULE_LOG_D(BATTERY, "task stopped, return");
            return;
        }

        battery_charge_mgr_1s();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(BATTERY_MGR_PERIOD_MS));
    }
}

static INT32 battery_init(UINT8 seq)
{
    INT32 ret = TBOX_E_OK;

    switch (seq) {
        case MODULE_INIT_SEQ_OS:
            /* 特殊处理，不可随意删除 */
            drv_pin_set_level(PIN_BAT_SUPPLY, 1U);
            break;
            
        case MODULE_INIT_SEQ_STORAGE:
            /* 从配置读取电池类型 */
            battery_charge_type_init_from_cfg();
            MODULE_LOG_D(BATTERY, "Battery type: %s", 
                        charge_type == BATTERY_CHARGE_TYPE_BAT ? "Battery" : "Capacitor");
            break;
            
        case MODULE_INIT_SEQ_MODULE:
            TBOX_SHELL_REGISTER(showbat);
            MODULE_LOG_D(BATTERY, "Battery module initialized");
            break;
            
        default:
            ret = TBOX_E_INVALID_PARAM;
            break;
    }

    return ret;
}

static VOID battery_stop(VOID)
{
    /* 停止充电 */
    battery_charge_hw_stop();
    MODULE_LOG_I(BATTERY, "Battery module stopped, charging disabled");
    tbox_module_set_state(battery_module_id, TBOX_MODULE_STATE_STOP);
}
