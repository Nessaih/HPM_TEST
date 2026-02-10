#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_pm_inner.h"
#include "tbox_pm_io.h"
#include "tbox_shell_if.h"

typedef struct
{
    UINT8      pin;
    UINT8      edge;
    drv_pin_cb_t cb;
    UINT32     event;
} pm_io_int_cfg_t;

static VOID tbox_pm_io_wake_callback(UINT32 event);
static BaseType_t tbox_pm_showwakesrc(CHAR *buf, UINT32 bufsz, const CHAR *cmd);

TBOX_SHELL_DEFINE(wakesrc, "show wake source", 0, tbox_pm_showwakesrc);
static UINT32 wake_source_bits = 0U;
static const pm_io_int_cfg_t wake_source_ints[] = 
{
    {PIN_WAKE_ACC,   PIN_INT_FALLING, tbox_pm_io_wake_callback, PM_WAKE_SOURCE_ACC    },
    {PIN_WAKE_RTC,   PIN_INT_FALLING, tbox_pm_io_wake_callback, PM_WAKE_SOURCE_RTC    },
    {PIN_WAKE_PWD,   PIN_INT_RISING,  tbox_pm_io_wake_callback, PM_WAKE_SOURCE_PWD    },
    {PIN_WAKE_ANT,   PIN_INT_FALLING, tbox_pm_io_wake_callback, PM_WAKE_SOURCE_ANT    },
    {PIN_WAKE_IMU,   PIN_INT_FALLING, tbox_pm_io_wake_callback, PM_WAKE_SOURCE_MOV    },
    {PIN_WAKE_RING,  PIN_INT_FALLING, tbox_pm_io_wake_callback, PM_WAKE_SOURCE_RING   },
    {PIN_WAKE_LIGHT, PIN_INT_FALLING, tbox_pm_io_wake_callback, PM_WAKE_SOURCE_LIGHT  },
};

INT32 tbox_pm_io_init(VOID)
{
    TBOX_SHELL_REGISTER(wakesrc);
    return (INT32)TBOX_E_OK;
}

VOID  tbox_pm_io_sleep(UINT32 set_wake_src)
{
    const pm_io_int_cfg_t *cfg;
    UINT8 index, count;

    drv_pin_sleep();

    taskENTER_CRITICAL();
    wake_source_bits = 0U;
    taskEXIT_CRITICAL();

    /*设置唤醒源中断*/
    count = (UINT8)(sizeof(wake_source_ints) / sizeof(pm_io_int_cfg_t));
    for (index = 0U; index < count; index++)
    {
        if(set_wake_src & (1U << wake_source_ints[index].event))
        {
            cfg = &wake_source_ints[index];
            drv_pin_set_interrupt(cfg->pin, cfg->edge, cfg->cb, cfg->event);
        }
    }
}

VOID  tbox_pm_io_wake(VOID)
{
	drv_pin_wake();

    /*清除唤醒源中断*/
    drv_pin_clear_interrupt();
}

UINT32 tbox_pm_io_get_wakesrc(VOID)
{
    UINT32 wakesrc = 0U;
    
    taskENTER_CRITICAL();
    wakesrc = wake_source_bits;
    taskEXIT_CRITICAL();

    return wakesrc;
}

BOOL   tbox_pm_io_mainpower_is_active(VOID)
{
    return (drv_pin_get_level(PIN_WAKE_PWD) > 0U) ? FALSE : TRUE;
}

BOOL   tbox_pm_io_acc_is_active(VOID)
{
    return (drv_pin_get_level(PIN_WAKE_ACC) > 0U) ? FALSE : TRUE;
}

BOOL   tbox_pm_io_fcw_is_active(VOID)
{
    /*TODO: 后续添加FCW检测*/
    return FALSE;
}

BOOL   tbox_pm_io_doh_is_active(VOID)
{
    return (drv_pin_get_level(PIN_ENABLE_DOH) > 0U) ? FALSE : TRUE;
}

BOOL   tbox_pm_io_dol_is_active(VOID)
{
    return (drv_pin_get_level(PIN_ENABLE_DOL) > 0U) ? FALSE : TRUE;
}

BOOL   tbox_pm_io_is_removed(VOID)
{
    return (drv_pin_get_level(PIN_MCU_DIL) > 0U) ? FALSE : TRUE;
}

VOID   tbox_pm_io_mpu_power_on(BOOL enable)
{
    INT32 level = (enable == TRUE)  ? 1 : 0;
    drv_pin_set_level(PIN_POWER_MPU, level);
}

VOID   tbox_pm_io_mpu_sleep(VOID)
{
    drv_pin_set_level(PIN_MPU_SLEEP, 0U);
}

VOID   tbox_pm_io_mpu_wake(VOID)
{
   drv_pin_set_level(PIN_MPU_SLEEP, 1U);
}

static VOID tbox_pm_io_wake_callback(UINT32 event)
{
    if(event >= PM_WAKE_SOURCE_MAX)
    {
        return;
    }

    UBaseType_t interruptstatus = taskENTER_CRITICAL_FROM_ISR();
    wake_source_bits |= (1U << event);
    taskEXIT_CRITICAL_FROM_ISR(interruptstatus);
}

static BaseType_t tbox_pm_showwakesrc(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *wake_src_name[] = {"ON", "ACC", "RTC", "PWD", "CAN", "ANT", "IMU",   "RING", "LIGHT", "REMOVED"};

    INT32  len  = 0;
    UINT32 wake_src = tbox_pm_io_get_wakesrc();

    len += snprintf(&buf[len], bufsz - len, "\r\n----------------------------------------\r\n");
    len += snprintf(&buf[len], bufsz - len, "Wake Sources\r\nBits: %#X\r\nName:", wake_src);
    for (UINT8 i = 0U; i < PM_WAKE_SOURCE_MAX; i++) 
    {
        if (wake_src & 1UL)
        {
            len += snprintf(&buf[len], bufsz - len, " %s", wake_src_name[i]);
        }
        wake_src >>= 1;
    }
    len += snprintf(&buf[len], bufsz - len, "\r\n----------------------------------------\r\n");

    return pdFALSE;    
}