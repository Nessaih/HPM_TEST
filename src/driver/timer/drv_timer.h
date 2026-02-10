#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#include <stdbool.h>
#include <stdint.h>

typedef void (*drv_timer_callback_t)(void);

typedef struct __drv_tcb
{
    drv_timer_callback_t func;
    struct __drv_tcb    *next;
} drv_tcb_t;

typedef struct drv_timer
{
    uint32_t   ins;
    uint32_t   period;
    uint32_t   mode;
    drv_tcb_t *list;
} drv_timer_t;

#define DRV_TIMER_MODE_INTERRUPT 0x1U
#define DRV_TIMER_MODE_PERIODIC  0x2U
#define DRV_TIMER_MODE_STARTUP   0x4U

#define DRV_TIMER_INS_PCT        0x0U
#define DRV_TIMER_INS_PDT0       0x1U
#define DRV_TIMER_INS_PDT1       0x2U
#define DRV_TIMER_INS_TIMER0     0x3U
#define DRV_TIMER_INS_TIMER1     0x4U
#define DRV_TIMER_INS_TIMER2     0x5U
#define DRV_TIMER_INS_TIMER3     0x6U
#define DRV_TIMER_INS_COUNT      0x7U

#define DRV_TIMER_CMD_SET_CFG    0x0U
#define DRV_TIMER_CMD_SET_INS    0x1U
#define DRV_TIMER_CMD_SET_MODE   0x2U
#define DRV_TIMER_CMD_CLS_MODE   0x3U
#define DRV_TIMER_CMD_SET_PERIOD 0x4U
#define DRV_TIMER_CMD_START      0x5U
#define DRV_TIMER_CMD_STOP       0x6U
#define DRV_TIMER_CMD_ATTACH     0x7U
#define DRV_TIMER_CMD_DETACH     0x8U
#define DRV_TIMER_CMD_SHIELD     0x9U

int32_t drv_timer_init(drv_timer_t *timer);
int32_t drv_timer_deinit(drv_timer_t *timer);
int32_t drv_timer_control(drv_timer_t *timer, ...);

#endif //__DRV_TIMER_H__