#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "tbox_common.h"
#include "tbox_log.h"
#include "can_if.h"
#include "can_mgr.h"
#include "can_buserr.h"
#include "can_busoff.h"
#include "can_types.h"
#include "tbox_cfg_if.h"
#include "drv_can.h"
#include "time_if.h"

extern INT32 can_if_init(UINT8 ins, UINT32 rate, UINT8 mode);
extern INT32 can_if_deinit(UINT8 ins);

/* CAN 统计信息 */
#define CAN_STAT_LAST_MSG_COUNT  (10)  /* 每路 CAN 保存最近 10 条消息 */
#define CAN_RATE_UPDATE_PERIOD_MS (100)  /* 帧率更新周期：100ms */

typedef struct {
    uint32_t total_recv[DRV_CAN_INS_COUNT];    /* 各端口总接收条数 */
    uint32_t drop_recv[DRV_CAN_INS_COUNT];     /* 各端口丢弃接收条数 */    
    uint32_t total_send[DRV_CAN_INS_COUNT];    /* 各端口总发送条数 */
    uint32_t drop_send[DRV_CAN_INS_COUNT];     /* 各端口丢弃发送条数 */
    can_msg_t last_msgs[DRV_CAN_INS_COUNT][CAN_STAT_LAST_MSG_COUNT]; /* 每路 CAN 最近接收的消息 */
    uint8_t   last_msg_idx[DRV_CAN_INS_COUNT];  /* 每路 CAN 的消息索引 */
    uint8_t   last_msg_count[DRV_CAN_INS_COUNT]; /* 每路 CAN 的消息数量 */
    
    /* 帧率统计 */
    uint32_t recv_count_current_sec[DRV_CAN_INS_COUNT];  /* 当前周期的接收计数 */
    uint32_t send_count_current_sec[DRV_CAN_INS_COUNT];  /* 当前周期的发送计数 */
    double   recv_rate[DRV_CAN_INS_COUNT];               /* 接收帧率（fps） */
    double   send_rate[DRV_CAN_INS_COUNT];               /* 发送帧率（fps） */
    uint32_t last_update_time_ms[DRV_CAN_INS_COUNT];     /* 上次更新时间戳(ms) */
} can_stat_t;

static uint32_t tbox_can_baudrate[DRV_CAN_INS_COUNT] = {0};
static can_stat_t can_stat;
static TimerHandle_t xRateUpdateTimer = NULL;

/* 帧率更新定时器回调 */
static void can_rate_timer_callback(TimerHandle_t xTimer)
{
    uint8_t ins;
    uint32_t current_time_ms;
    uint32_t elapsed_ms;
    
    (void)xTimer;
    
    current_time_ms = time_if_get_systick_ms();
    
    /* 更新所有CAN通道的帧率 */
    for (ins = 0; ins < DRV_CAN_INS_COUNT; ins++) {
        elapsed_ms = current_time_ms - can_stat.last_update_time_ms[ins];
        
        if (elapsed_ms > 0) {
            /* 计算帧率（fps）
             * 帧率 = (本周期计数 / 经过时间) × 1000ms
             */
            can_stat.recv_rate[ins] = 
                (double)can_stat.recv_count_current_sec[ins] * 1000.0 / (double)elapsed_ms;
            can_stat.send_rate[ins] = 
                (double)can_stat.send_count_current_sec[ins] * 1000.0 / (double)elapsed_ms;
        }
        
        /* 重置本周期计数器 */
        can_stat.recv_count_current_sec[ins] = 0;
        can_stat.send_count_current_sec[ins] = 0;
        
        /* 更新时间戳 */
        can_stat.last_update_time_ms[ins] = current_time_ms;
    }
}

/* 更新统计信息 - 保存最近消息 */
void can_mgr_stat_add_recv_msgs(can_msg_t *msgs, uint32_t count)
{
    uint32_t i;
    uint8_t ins;
    
    if (msgs == NULL || count == 0) return;
    
    for (i = 0; i < count; i++) {
        ins = msgs[i].ins;
        if (ins >= DRV_CAN_INS_COUNT) continue;
        
        can_stat.total_recv[ins]++;
        
        can_stat.recv_count_current_sec[ins]++;
        
        /* 保存到对应 CAN 通道的环形缓冲区 */
        memcpy(&can_stat.last_msgs[ins][can_stat.last_msg_idx[ins]], &msgs[i], sizeof(can_msg_t));
        can_stat.last_msg_idx[ins] = (can_stat.last_msg_idx[ins] + 1) % CAN_STAT_LAST_MSG_COUNT;
        if (can_stat.last_msg_count[ins] < CAN_STAT_LAST_MSG_COUNT) {
            can_stat.last_msg_count[ins]++;
        }
    }
}

void can_mgr_stat_add_droprx_msgs(uint8_t ins, uint32_t count)
{
    if (ins < DRV_CAN_INS_COUNT) {
        can_stat.drop_recv[ins] += count;
    }  
}

void can_mgr_stat_add_send_msg(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        can_stat.total_send[ins]++;
        can_stat.send_count_current_sec[ins]++;
    }
}

void can_mgr_stat_add_droptx_msg(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        can_stat.drop_send[ins]++;
    } 
}

uint32_t can_mgr_stat_get_recv_count(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        return can_stat.total_recv[ins];
    }
    return 0;
}

uint32_t can_mgr_stat_get_droprx_count(uint8_t ins)
{
     if (ins < DRV_CAN_INS_COUNT) {
        return can_stat.drop_recv[ins];
    }
    return 0U;   
}

uint32_t can_mgr_stat_get_send_count(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        return can_stat.total_send[ins];
    }
    return 0;
}

uint32_t can_mgr_stat_get_droptx_count(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        return can_stat.drop_send[ins];
    }
    return 0U;
}

uint8_t can_mgr_stat_get_last_msg_count(uint8_t ins)
{
    if (ins >= DRV_CAN_INS_COUNT) {
        return 0;
    }
    return can_stat.last_msg_count[ins];
}

can_msg_t *can_mgr_stat_get_last_msg(uint8_t ins, uint8_t idx)
{
    uint8_t actual_idx;
    
    if (ins >= DRV_CAN_INS_COUNT) {
        return NULL;
    }
    
    if (idx >= can_stat.last_msg_count[ins] || idx >= CAN_STAT_LAST_MSG_COUNT) {
        return NULL;
    }
    
    actual_idx = (can_stat.last_msg_idx[ins] + CAN_STAT_LAST_MSG_COUNT - 1 - idx) % CAN_STAT_LAST_MSG_COUNT;
    return &can_stat.last_msgs[ins][actual_idx];
}

uint32_t can_mgr_get_baudrate(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        return tbox_can_baudrate[ins];
    }
    return 0;
}

void can_mgr_update_baudrate(uint8_t ins, uint32_t baudrate)
{
    if (ins < DRV_CAN_INS_COUNT) {
        tbox_can_baudrate[ins] = baudrate;
    }
}

void can_mgr_init(void)
{
    unsigned int i;
    int          ret;
    unsigned int baudrate;

    memset(&can_stat, 0, sizeof(can_stat));

    /* 创建帧率更新定时器（100ms周期，自动重载） */
    xRateUpdateTimer = xTimerCreate(
        "CanRateTmr",                           /* 定时器名称 */
        pdMS_TO_TICKS(CAN_RATE_UPDATE_PERIOD_MS), /* 100ms周期 */
        pdTRUE,                                 /* 自动重载 */
        (void *)0,                              /* 定时器ID */
        can_rate_timer_callback);               /* 回调函数 */
    
    if (xRateUpdateTimer != NULL) {
        xTimerStart(xRateUpdateTimer, 0);
    } else {
        MODULE_LOG_E(CAN, "create rate update timer failed");
    }

    for (i = 0; i < DRV_CAN_INS_COUNT; i++) {
        ret = can_if_init(i, 0, DRV_CAN_MODE_NORMAL);
        if (ret == 0) {
            baudrate = can_mgr_get_baudrate(i);
            if (baudrate > 0) {
                MODULE_LOG_D(CAN, "CAN%u init success, baudrate:%u", i + 1, baudrate);
            }
        }
    }
}

void can_mgr_deinit(void)
{
    int          ret;
    unsigned int i;

    if (xRateUpdateTimer != NULL) {
        xTimerStop(xRateUpdateTimer, 0);
        xTimerDelete(xRateUpdateTimer, 0);
        xRateUpdateTimer = NULL;
    }

    for (i = 0; i < DRV_CAN_INS_COUNT; i++) {
        ret = can_if_deinit(i);
        if (ret != 0) {
            MODULE_LOG_E(CAN, "deinit CAN%u failed, ret:%u", i + 1, ret);
            continue;
        }
        MODULE_LOG_I(CAN, "CAN%u deinit success", i + 1);
        tbox_can_baudrate[i] = 0;
    }
}

uint8_t can_if_state_get(uint8_t instance)
{
    uint8_t state = CAN_INSTANCE_NOUSED;

    if (instance >= DRV_CAN_INS_COUNT) {
        return state;
    }

    /* 查询恢复模块状态 */
    if (can_is_busoff_recovering(instance)) {
        state = CAN_INSTANCE_OFF;
    } else if (can_is_buserr_recovering(instance)) {
        state = CAN_INSTANCE_ERROR;
    } else {
        if (!drv_can_has_init(instance)) {
            return CAN_INSTANCE_NOUSED;
        }
        if (drv_can_busy(instance)) {
            state = CAN_INSTANCE_BUSY;
        } else {
            state = CAN_INSTANCE_IDLE;
        }
    }

    return state;
}

double can_mgr_stat_get_recv_rate(uint8_t ins)
{
    if (ins >= DRV_CAN_INS_COUNT) {
        return 0.0;
    }
    return can_stat.recv_rate[ins];
}

double can_mgr_stat_get_send_rate(uint8_t ins)
{
    if (ins >= DRV_CAN_INS_COUNT) {
        return 0.0;
    }
    return can_stat.send_rate[ins];
}

uint32_t can_if_get_recv_count(uint8_t instance)
{
    return can_mgr_stat_get_recv_count(instance);
}