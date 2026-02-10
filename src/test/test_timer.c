

#include "drv_pin.h"
#include "drv_timer.h"
#include "stimer.h"

static drv_timer_t hw_timer;
static drv_tcb_t   hw_tcb;
static stimer_t    sw_timer;

static void hw_timer_callback(void)
{
    drv_pin_toggle(PIN_TEST_OUT);
}

static void hw_timer_test(void)
{
    drv_timer_control(&hw_timer, DRV_TIMER_CMD_SET_CFG, DRV_TIMER_INS_TIMER1, 100, DRV_TIMER_MODE_INTERRUPT | DRV_TIMER_MODE_STARTUP);
    drv_timer_control(&hw_timer, DRV_TIMER_CMD_ATTACH, hw_timer_callback, &hw_tcb);
    drv_timer_init(&hw_timer);
}

static void sw_timer_callback(void)
{
    drv_pin_toggle(PIN_TEST_OUT);
}

static void sw_timer_test(void)
{
    stimer_create(&sw_timer, STIMER_TYPE_PERIOD, sw_timer_callback);
    stimer_start(&sw_timer, 100);
}

void test_timer_init(void *param)
{
    hw_timer_test();
    sw_timer_test();
}
