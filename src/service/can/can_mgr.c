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

extern INT32 can_if_init(UINT8 ins, UINT32 rate, UINT8 mode);
extern INT32 can_if_deinit(UINT8 ins);

/* CAN 统计信息 */
#define CAN_STAT_LAST_MSG_COUNT  (10)  /* 保存最近 10 条消息 */

typedef struct {
    uint32_t total_recv[DRV_CAN_INS_COUNT];     /* 各端口总接收条数 */
    uint32_t total_send[DRV_CAN_INS_COUNT];     /* 各端口总发送条数 */
    can_msg_t last_msgs[CAN_STAT_LAST_MSG_COUNT]; /* 最近接收的消息 */
    uint8_t   last_msg_idx;                     /* 最近消息索引 */
    uint8_t   last_msg_count;                   /* 最近消息数量 */
} can_stat_t;

static uint32_t tbox_can_baudrate[DRV_CAN_INS_COUNT] = {0};
static can_stat_t can_stat;

/* 更新统计信息 - 保存最近消息 */
void can_mgr_stat_add_recv_msg(can_msg_t *msg)
{
    if (msg == NULL) return;
    
    if (msg->ins < DRV_CAN_INS_COUNT) {
        can_stat.total_recv[msg->ins]++;
    }
    
    memcpy(&can_stat.last_msgs[can_stat.last_msg_idx], msg, sizeof(can_msg_t));
    can_stat.last_msg_idx = (can_stat.last_msg_idx + 1) % CAN_STAT_LAST_MSG_COUNT;
    if (can_stat.last_msg_count < CAN_STAT_LAST_MSG_COUNT) {
        can_stat.last_msg_count++;
    }
}

void can_mgr_stat_add_send_msg(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        can_stat.total_send[ins]++;
    }
}

uint32_t can_mgr_stat_get_recv_count(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        return can_stat.total_recv[ins];
    }
    return 0;
}

uint32_t can_mgr_stat_get_send_count(uint8_t ins)
{
    if (ins < DRV_CAN_INS_COUNT) {
        return can_stat.total_send[ins];
    }
    return 0;
}

uint8_t can_mgr_stat_get_last_msg_count(void)
{
    return can_stat.last_msg_count;
}

can_msg_t *can_mgr_stat_get_last_msg(uint8_t idx)
{
    if (idx >= can_stat.last_msg_count || idx >= CAN_STAT_LAST_MSG_COUNT) {
        return NULL;
    }
    
    uint8_t actual_idx = (can_stat.last_msg_idx + CAN_STAT_LAST_MSG_COUNT - 1 - idx) % CAN_STAT_LAST_MSG_COUNT;
    return &can_stat.last_msgs[actual_idx];
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

    if (!drv_can_has_init(instance)) {
        return CAN_INSTANCE_NOUSED;
    }

    /* 查询恢复模块状态 */
    if (can_is_busoff_recovering(instance)) {
        state = CAN_INSTANCE_OFF;
    } else if (can_is_buserr_recovering(instance)) {
        state = CAN_INSTANCE_ERROR;
    } else {
        if (drv_can_busy(instance)) {
            state = CAN_INSTANCE_BUSY;
        } else {
            state = CAN_INSTANCE_IDLE;
        }
    }

    return state;
}

uint32_t can_if_get_recv_count(uint8_t instance)
{
    return can_mgr_stat_get_recv_count(instance);
}