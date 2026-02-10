#include "tbox_common.h"
#include "tbox_core.h"
#include "drv_pin.h"
#include "stimer.h"
#include "4g_if.h"
#include "can_if.h"
#include "gnss_if.h"

#include "led_if.h"
#include "led.h"
#include "led_tbox.h"
#if 0
#include "can_if.h"
#include "4g_if.h"
#include "gnss_if.h"
#endif

#define LED_START_MODE_PERIOD 100
#define LED_RUN_MODE_PERIOD   1000

typedef enum
{
	LED_TBOX_MOD_START	= 0x00,
	LED_TBOX_MOD_RUN	= 0x01,
	LED_TBOX_MOD_STOP	= 0x02,
	
	LED_TBOX_MOD_MAX	= LED_TBOX_MOD_STOP + 1,
}LED_TBOX_MODE_E;

typedef union {
    struct
    {
        UINT8 run  : 1;
        UINT8 mode : 3;
        UINT8 lte  : 1;
        UINT8 gns  : 1;
        UINT8 can  : 1;
    };
    UINT8 state;
} LED_TBOX_STATE_T;

static STIMER_ID    	led_tbox_timer;
static LED_TBOX_STATE_T led_tbox_state;

static INT32 led_tbox_start_mode(VOID)
{
    static BOOL  start = false;
    static UINT8 tick  = 0U;
    static UINT8 cycle = 0U;

    if (start) {
        return 0;
    }
    switch (++tick) {
    case 1U: {
        drv_pin_set_level(PIN_LED_TBOX_RUN, 0U);
        drv_pin_set_level(PIN_LED_TBOX_CAN, 0U);
        drv_pin_set_level(PIN_LED_TBOX_LTE, 0U);
        drv_pin_set_level(PIN_LED_TBOX_GNSS, 0U);
        if (2U == cycle) {
            start = true;
            return -1;
        }
        break;
    }
    case 2U: {
        drv_pin_set_level(PIN_LED_TBOX_RUN, 1U);
        break;
    }
    case 3U: {
        drv_pin_set_level(PIN_LED_TBOX_CAN, 1U);
        break;
    }
    case 4U: {
        if (1U == ++cycle) {
            drv_pin_set_level(PIN_LED_TBOX_GNSS, 1U);
        } else if (2U == cycle) {
            drv_pin_set_level(PIN_LED_TBOX_LTE, 1U);
        } else {
        }
        tick = 0U;
        break;
    }
    default:
        break;
    }
    return 0;
}

static VOID led_tbox_run_mode(VOID)
{
	if((CAN_INSTANCE_BUSY == can_if_state_get(0)) || (CAN_INSTANCE_BUSY == can_if_state_get(1)))
	{
		led_tbox_set_can(TRUE);
	}
	else
	{
		led_tbox_set_can(FALSE);
	}

	if(IF_4G_STATE_CONNECTED == if_4g_get_call_state(IF_4G_PUBLIC_APN))
	{
		led_tbox_set_lte(TRUE);
	}
	else
	{
		led_tbox_set_lte(FALSE);
	}

	if(GNSS_POS_STATE_FIX == gnss_get_fix_state())
	{
		led_tbox_set_gns(TRUE);
	}
	else
	{
		led_tbox_set_gns(FALSE);
	}

    drv_pin_toggle(PIN_LED_TBOX_RUN);
    drv_pin_set_level(PIN_LED_TBOX_CAN, led_tbox_state.can);
	drv_pin_set_level(PIN_LED_TBOX_LTE, led_tbox_state.lte);
	drv_pin_set_level(PIN_LED_TBOX_GNSS, led_tbox_state.gns);

    return;
}

static VOID led_tbox_stop_mode(VOID)
{
    drv_pin_set_level(PIN_LED_TBOX_RUN, 0U);
    drv_pin_set_level(PIN_LED_TBOX_CAN, 0U);
    drv_pin_set_level(PIN_LED_TBOX_LTE, 0U);
    drv_pin_set_level(PIN_LED_TBOX_GNSS,0U);
}

VOID led_tbox_sleep(VOID)
{
    led_tbox_state.run  = 0;
    led_tbox_state.mode = LED_TBOX_MOD_STOP;

    stimer_stop(led_tbox_timer);
	led_tbox_stop_mode();
}

VOID led_tbox_wake(VOID)
{
    led_tbox_state.run  = 1;
    led_tbox_state.mode = LED_TBOX_MOD_RUN;

    led_tbox_run_mode();
    stimer_start(led_tbox_timer, LED_RUN_MODE_PERIOD);
}

VOID led_tbox_timeout(VOID)
{
    INT32 ret;

    if (0U == led_tbox_state.run) 
	{
        return;
    }

    switch (led_tbox_state.mode) 
	{
	    case LED_TBOX_MOD_START:
	        ret = led_tbox_start_mode();
	        if (0 != ret) 
			{
	            led_tbox_state.mode = LED_TBOX_MOD_RUN;
	            stimer_start(led_tbox_timer, LED_RUN_MODE_PERIOD);
	        }
	        break;

	    case LED_TBOX_MOD_RUN:
	        led_tbox_run_mode();
	        break;

	    case LED_TBOX_MOD_STOP:
	        led_tbox_stop_mode();
	        break;

	    default:
	        break;
    }
}

static VOID led_tbox_timer_callback(VOID)
{
	TBOX_ID module_id;
    GET_TBOX_MODULE_ID(LED, module_id);

    tbox_message_send(LED_TBOX_TIMER_EVENT, module_id, module_id, NULL_PTR); 
}

INT32 led_tbox_init(VOID)
{
    INT32 ret = 0;

    led_tbox_state.state = 0;
    led_tbox_state.run   = 1;

    led_tbox_timer = stimer_create(STIMER_TYPE_PERIOD, led_tbox_timer_callback);
	if(STIMER_ID_INVALID == led_tbox_timer)
	{
		MODULE_LOG_E(LED, "create led tbox timer failed");
		return -1;
	}
    stimer_start(led_tbox_timer, LED_START_MODE_PERIOD);

    return ret;
}

VOID led_tbox_set_can(BOOL active)
{
    led_tbox_state.can = active ? 1U : 0U;
}

VOID led_tbox_set_lte(BOOL active)
{
    led_tbox_state.lte = active ? 1U : 0U;
}

VOID led_tbox_set_gns(BOOL active)
{
    led_tbox_state.gns = active ? 1U : 0U;
}

