#include <string.h>
#include "tbox_common.h"
#include "tbox_log.h"
#include "can_buserr.h"
#include "can_if.h"
#include "drv_can.h"

typedef struct
{
    uint16_t cnt;           /* 错误计数 */
    bool     is_recovering; /* 是否正在恢复 */
} CAN_BUSERR_ITEM;

static CAN_BUSERR_ITEM can_buserr_ctl[DRV_CAN_INS_COUNT];
static TimerHandle_t   buserr_timer[DRV_CAN_INS_COUNT] = {NULL};

/* BusError 恢复定时器回调 */
static void can_buserr_timer_callback(TimerHandle_t xTimer)
{
    uint8_t port = (uint8_t)(uintptr_t)pvTimerGetTimerID(xTimer);
    
    if (port >= DRV_CAN_INS_COUNT) {
        return;
    }
    
    MODULE_LOG_D(CAN, "CAN%u error recovery timeout, re-enable error interrupt", port + 1);
    if (drv_can_has_init(port)) {
        drv_can_enable_error_report(port, true);
    }
    can_buserr_ctl[port].is_recovering = false;
    can_buserr_ctl[port].cnt = 0;
}

void can_buserr_callback(uint8_t port, bool is_error)
{
    if (port >= DRV_CAN_INS_COUNT) {
        return;
    }
    
    if (is_error) {
        if (can_buserr_ctl[port].cnt > CAN_BUSERR_THRD) {
            if (!can_buserr_ctl[port].is_recovering) {
                MODULE_LOG_D(CAN, "CAN%u error count:%u > threshold:%u, disable error interrupt for %ums", 
                      port + 1, can_buserr_ctl[port].cnt, CAN_BUSERR_THRD, CAN_BUSERR_RECOVER_TIME);
                
                /* 关闭错误中断 */
                drv_can_enable_error_report(port, false);
                can_buserr_ctl[port].is_recovering = true;
                
                /* 启动恢复定时器 */
                if (buserr_timer[port] != NULL) {
                    xTimerStart(buserr_timer[port], 0);
                }
            }
        }
        
        can_buserr_ctl[port].cnt++;
    } else {
        /* 总线恢复正常 */
        if (can_buserr_ctl[port].is_recovering || can_buserr_ctl[port].cnt > 0) {
            MODULE_LOG_D(CAN, "CAN%u bus recovered to normal (error count was %u)", 
                  port + 1, can_buserr_ctl[port].cnt);
        }
        
        if (buserr_timer[port] != NULL && pdTRUE == xTimerIsTimerActive(buserr_timer[port])) {
            if(NULL != buserr_timer[port]) {
                xTimerStop(buserr_timer[port], 0);
            }
        }
        can_buserr_ctl[port].cnt = 0;
        can_buserr_ctl[port].is_recovering = false;
    }
}

bool can_is_buserr_recovering(uint8_t port)
{
    if (port >= DRV_CAN_INS_COUNT) {
        return false;
    }
    return can_buserr_ctl[port].is_recovering;
}

void can_buserr_sleep(void)
{
    for (uint8_t i = 0; i < DRV_CAN_INS_COUNT; i++) {
        if (buserr_timer[i] != NULL && pdTRUE == xTimerIsTimerActive(buserr_timer[i])) {
            if(NULL != buserr_timer[i]) {
                xTimerStop(buserr_timer[i], 0);
            }
        }
        
        if (can_buserr_ctl[i].is_recovering) {
            drv_can_enable_error_report(i, true);
        }
        
        can_buserr_ctl[i].cnt = 0;
        can_buserr_ctl[i].is_recovering = false;
    }
}

void can_buserr_init(void)
{
    memset(can_buserr_ctl, 0, sizeof(can_buserr_ctl));
    
    for (uint8_t i = 0; i < DRV_CAN_INS_COUNT; i++) {
        /* 创建 BusError 恢复定时器（一次性定时器） */
        buserr_timer[i] = xTimerCreate(
            "Can BusErr",                    /* 定时器名称 */
            pdMS_TO_TICKS(CAN_BUSERR_RECOVER_TIME), /* 100ms */
            pdFALSE,                         /* 一次性定时器 */
            (void *)(uintptr_t)i,            /* 定时器ID（存储port） */
            can_buserr_timer_callback);      /* 回调函数 */
        configASSERT(NULL != buserr_timer[i]);
    }
    
    MODULE_LOG_D(CAN, "CAN BUSERR init done");
}
