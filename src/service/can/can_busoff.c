#include <string.h>
#include "tbox_common.h"
#include "tbox_log.h"
#include "can_busoff.h"
#include "can_mgr.h"
#include "can_if.h"
#include "drv_can.h"
#include "drv_pin.h"
#include "tbox_pm_io.h"

extern INT32 can_if_init(UINT8 ins, UINT32 rate, UINT8 mode);
extern INT32 can_if_deinit(UINT8 ins);

typedef struct
{
    uint16_t cnt;           /* 恢复尝试计数 */
    bool     is_recovering; /* 是否正在恢复 */
} CAN_BUSOFF_ITEM;

static CAN_BUSOFF_ITEM can_busoff_ctl[DRV_CAN_INS_COUNT];
static TimerHandle_t   busoff_timer[DRV_CAN_INS_COUNT] = {NULL};

/* BusOff 恢复定时器回调 - 执行恢复 */
static void can_busoff_timer_callback(TimerHandle_t xTimer)
{
    uint8_t port = (uint8_t)(uintptr_t)pvTimerGetTimerID(xTimer);
    
    if (port >= DRV_CAN_INS_COUNT) {
        return;
    }
    
    /* 检查是否仍在恢复中 */
    if (!can_busoff_ctl[port].is_recovering) {
        MODULE_LOG_W(CAN, "CAN%u recovery skipped, not recovering", port);
        return;
    }
    
    /* 检查 主电 或 ACC 状态 */
    if (!tbox_pm_io_mainpower_is_active() || !tbox_pm_io_acc_is_active()) {
        /* 继续等待，重新启动定时器 */
        if (busoff_timer[port] != NULL) {
            if (can_busoff_ctl[port].cnt < CAN_BUSOFF_THRD) {
                xTimerChangePeriod(busoff_timer[port], pdMS_TO_TICKS(CAN_BUSOFF_RECOVER_FAST), 0);
            } else {
                xTimerChangePeriod(busoff_timer[port], pdMS_TO_TICKS(CAN_BUSOFF_RECOVER_SLOW), 0);
            }
            xTimerStart(busoff_timer[port], 0);
        }
        MODULE_LOG_I(CAN, "CAN%u recovery skipped, mainpower or acc not active, wait for recovery", port);
        return;
    }

    can_if_init(port, 0, DRV_CAN_MODE_NORMAL);
    
    /* 执行恢复：重新初始化 CAN */
    MODULE_LOG_I(CAN, "CAN%u recovery timeout, attempt:%u, re-init CAN", 
          port, can_busoff_ctl[port].cnt);
}

void can_busoff_callback(uint8_t port, bool is_busoff)
{
    if (port >= DRV_CAN_INS_COUNT) {
        return;
    }
    
    if (is_busoff) {

        if(tbox_pm_io_mainpower_is_active()) {
            can_if_deinit(port);
        }

        can_busoff_ctl[port].is_recovering = true;
        
        if (can_busoff_ctl[port].cnt < CAN_BUSOFF_THRD) {
            /* 快恢复：100ms */
            MODULE_LOG_I(CAN, "CAN%u bus off detected, start fast recovery (attempt:%u/%u)", 
                  port, can_busoff_ctl[port].cnt + 1, CAN_BUSOFF_THRD);
            
            if (busoff_timer[port] != NULL) {
                xTimerChangePeriod(busoff_timer[port], pdMS_TO_TICKS(CAN_BUSOFF_RECOVER_FAST), 0);
                xTimerStart(busoff_timer[port], 0);
            }
        } else {
            /* 慢恢复：1000ms */
            MODULE_LOG_I(CAN, "CAN%u bus off detected, start slow recovery (attempt:%u)", 
                  port, can_busoff_ctl[port].cnt + 1);
            
            if (busoff_timer[port] != NULL) {
                xTimerChangePeriod(busoff_timer[port], pdMS_TO_TICKS(CAN_BUSOFF_RECOVER_SLOW), 0);
                xTimerStart(busoff_timer[port], 0);
            }
        }
        
        can_busoff_ctl[port].cnt++;
    } else {
        if (can_busoff_ctl[port].is_recovering) {
            /* 在恢复过程中，通过RX_DONE或SEND_DONE清除恢复状态
             * 只有在真正恢复时（收到数据或发送成功），才会执行到这里清除状态
             */
            MODULE_LOG_I(CAN, "CAN%u bus recovered to normal after %u attempts", 
                  port, can_busoff_ctl[port].cnt);
            
            if (busoff_timer[port] != NULL && pdTRUE == xTimerIsTimerActive(busoff_timer[port])) {
                if(NULL != busoff_timer[port]) {
                    xTimerStop(busoff_timer[port], 0);
                }
            }
            can_busoff_ctl[port].cnt = 0;
            can_busoff_ctl[port].is_recovering = false;
        }
    }
}

bool can_is_busoff_recovering(uint8_t port)
{
    if (port >= DRV_CAN_INS_COUNT) {
        return false;
    }
    return can_busoff_ctl[port].is_recovering;
}

void can_busoff_sleep(void)
{
    for (uint8_t i = 0; i < DRV_CAN_INS_COUNT; i++) {
        if (busoff_timer[i] != NULL && pdTRUE == xTimerIsTimerActive(busoff_timer[i])) {
            if(NULL != busoff_timer[i]) {
                xTimerStop(busoff_timer[i], 0);
            }
        }
        can_busoff_ctl[i].cnt = 0;
        can_busoff_ctl[i].is_recovering = false;
    }
}

void can_busoff_init(void)
{
    memset(can_busoff_ctl, 0, sizeof(can_busoff_ctl));
    
    for (uint8_t i = 0; i < DRV_CAN_INS_COUNT; i++) {
        /* 创建 BusOff 恢复定时器（一次性定时器） */
        busoff_timer[i] = xTimerCreate(
            "Can BusOff",                    /* 定时器名称 */
            pdMS_TO_TICKS(CAN_BUSOFF_RECOVER_FAST), /* 初始 100ms */
            pdFALSE,                         /* 一次性定时器 */
            (void *)(uintptr_t)i,            /* 定时器ID（存储port） */
            can_busoff_timer_callback);      /* 回调函数 */
        configASSERT(NULL != busoff_timer[i]);
    }
    
    MODULE_LOG_D(CAN, "CAN BUSOFF init done");
}
