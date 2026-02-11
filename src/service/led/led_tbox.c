#include "tbox_common.h"
#include "tbox_core.h"
#include "drv_pin.h"
#include "stimer.h"

#include "led_if.h"
#include "led.h"
#include "led_tbox.h"
#include "can_if.h"
#include "4g_if.h"
#include "gnss_if.h"
#include "analog_if.h"

#define LED_START_MODE_PERIOD (100U)
#define LED_RUN_MODE_PERIOD (1000U)
#define LED_GNSS_ANT_FAULT_CNT_MAX (60U)
#define LED_RAN_MODE_COUNT (15U)

#define led_run_toggle() drv_pin_toggle(PIN_LED_TBOX_RUN)
#define led_can_toggle() drv_pin_toggle(PIN_LED_TBOX_CAN)
#define led_lte_toggle() drv_pin_toggle(PIN_LED_TBOX_LTE)
#define led_gnss_toggle() drv_pin_toggle(PIN_LED_TBOX_GNSS)

#define led_run_on() drv_pin_set_level(PIN_LED_TBOX_RUN, 1U)
#define led_can_on() drv_pin_set_level(PIN_LED_TBOX_CAN, 1U)
#define led_lte_on() drv_pin_set_level(PIN_LED_TBOX_LTE, 1U)
#define led_gnss_on() drv_pin_set_level(PIN_LED_TBOX_GNSS, 1U)

#define led_run_off() drv_pin_set_level(PIN_LED_TBOX_RUN, 0U)
#define led_can_off() drv_pin_set_level(PIN_LED_TBOX_CAN, 0U)
#define led_lte_off() drv_pin_set_level(PIN_LED_TBOX_LTE, 0U)
#define led_gnss_off() drv_pin_set_level(PIN_LED_TBOX_GNSS, 0U)

typedef enum
{
    LED_TBOX_MODE_START = 0,
    LED_TBOX_MODE_RUN,
    LED_TBOX_MODE_STOP,
    LED_TBOX_MODE_MAX,
} LED_TBOX_MODE_E;

typedef enum
{
    LED_TBOX_ID_RUN = 0,
    LED_TBOX_ID_CAN = 1,
    LED_TBOX_ID_GNSS = 2,
    LED_TBOX_ID_LTE = 3,
    LED_TBOX_ID_MAX,
} LED_TBOX_ID_E;

typedef enum
{
    LED_TBOX_STATE_NORMAL = 0,
    LED_TBOX_STATE_ERROR = 1,
    LED_TBOX_STATE_ABNORMAL = 2,
} LED_TBOX_STATE_E;

typedef VOID (*LED_TBOX_HANDLE_FUN)(LED_TBOX_STATE_E state, BOOL enable);

typedef struct
{
    LED_TBOX_ID_E id : 4;
    LED_TBOX_STATE_E state : 3;
    UINT8 enable : 1;
} LED_TBOX_HANDLE_T;

typedef struct
{
    LED_TBOX_MODE_E mode : 2;
    UINT8 start_count : 6;
    UINT32 gnss_ant_fault_count;
    LED_TBOX_HANDLE_T handle[LED_TBOX_ID_MAX];
} LED_TBOX_CONTENT_T;

static STIMER_ID led_tbox_timer;
static SemaphoreHandle_t led_tbox_mutex = NULL_PTR;
static LED_TBOX_CONTENT_T led_tbox_ctx;

static VOID led_tbox_run_handle(LED_TBOX_STATE_E state, BOOL enable)
{
    (void)(enable);
    (void)(state);
    led_run_toggle();
}

static VOID led_tbox_can_handle(LED_TBOX_STATE_E state, BOOL enable)
{
    if (!enable)
    {
        led_can_off();
        return;
    }

    switch (state)
    {
    case LED_TBOX_STATE_NORMAL:
        led_can_toggle();
        break;
    case LED_TBOX_STATE_ERROR:
        led_can_on();
        break;
    case LED_TBOX_STATE_ABNORMAL:
        led_can_off();
        break;
    default:
        break;
    }
}

static VOID led_tbox_lte_handle(LED_TBOX_STATE_E state, BOOL enable)
{
    if (!enable)
    {
        led_lte_off();
        return;
    }

    switch (state)
    {
    case LED_TBOX_STATE_NORMAL:
        led_lte_toggle();
        break;
    case LED_TBOX_STATE_ERROR:
        led_lte_on();
        break;
    case LED_TBOX_STATE_ABNORMAL:
        led_lte_off();
        break;
    default:
        break;
    }
}

static VOID led_tbox_gnss_handle(LED_TBOX_STATE_E state, BOOL enable)
{
    if (!enable)
    {
        led_gnss_off();
        return;
    }

    switch (state)
    {
    case LED_TBOX_STATE_NORMAL:
        led_gnss_toggle();
        break;
    case LED_TBOX_STATE_ERROR:
        led_gnss_on();
        break;
    case LED_TBOX_STATE_ABNORMAL:
        led_gnss_off();
        break;
    default:
        break;
    }
}

static LED_TBOX_HANDLE_FUN led_tbox_handle_fun[LED_TBOX_ID_MAX] = {
    led_tbox_run_handle,
    led_tbox_can_handle,
    led_tbox_gnss_handle,
    led_tbox_lte_handle,
};

static VOID led_tbox_set_state(LED_TBOX_ID_E id, LED_TBOX_STATE_E state)
{
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    led_tbox_ctx.handle[id].state = state;
    xSemaphoreGive(led_tbox_mutex);
}

static LED_TBOX_STATE_E led_tbox_get_state(LED_TBOX_ID_E id)
{
    LED_TBOX_STATE_E state = LED_TBOX_STATE_NORMAL;
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    state = led_tbox_ctx.handle[id].state;
    xSemaphoreGive(led_tbox_mutex);
    return state;
}

static BOOL led_tbox_get_enable(LED_TBOX_ID_E id)
{
    BOOL enable = FALSE;
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    enable = led_tbox_ctx.handle[id].enable;
    xSemaphoreGive(led_tbox_mutex);
    return enable;
}

static VOID led_tbox_set_enable(LED_TBOX_ID_E id, BOOL enable)
{
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    led_tbox_ctx.handle[id].enable = enable;
    xSemaphoreGive(led_tbox_mutex);
}

static VOID led_tbox_set_mode(LED_TBOX_MODE_E mode)
{
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    led_tbox_ctx.mode = mode;
    xSemaphoreGive(led_tbox_mutex);
}

static LED_TBOX_MODE_E led_tbox_get_mode(VOID)
{
    LED_TBOX_MODE_E mode = LED_TBOX_MODE_RUN;
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    mode = led_tbox_ctx.mode;
    xSemaphoreGive(led_tbox_mutex);
    return mode;
}

static VOID led_tbox_check_can(VOID)
{
    BOOL can_busy = FALSE;
    for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++)
    {
        if (CAN_INSTANCE_BUSY == can_if_state_get(i))
        {
            can_busy = TRUE;
            break;
        }
    }

    if (TRUE == can_busy)
    {
        led_tbox_set_state(LED_TBOX_ID_CAN, LED_TBOX_STATE_NORMAL);
    }
    else
    {
        led_tbox_set_state(LED_TBOX_ID_CAN, LED_TBOX_STATE_ABNORMAL);
    }
}

static VOID led_tbox_check_lte(VOID)
{
    if (IF_4G_STATE_CONNECTED == if_4g_get_socket_conn_state(IF_4G_HPM_CONN_ID))
    {
        led_tbox_set_state(LED_TBOX_ID_LTE, LED_TBOX_STATE_NORMAL);
    }
    else
    {
        led_tbox_set_state(LED_TBOX_ID_LTE, LED_TBOX_STATE_ABNORMAL);
    }
}

static VOID led_tbox_check_gnss(VOID)
{
    /*GPS模组状态，需要添加完成后添加*/
    BOOL gnss_wdg_err = FALSE;
    if (gnss_wdg_err)
    {
        led_tbox_set_state(LED_TBOX_ID_GNSS, LED_TBOX_STATE_ERROR);
        return;
    }

    /*GNSS天线状态*/
    if (ANT_SHORT == analog_gps_ant_status())
    {
        led_tbox_ctx.gnss_ant_fault_count++;
        if (led_tbox_ctx.gnss_ant_fault_count >= LED_GNSS_ANT_FAULT_CNT_MAX)
        {
            led_tbox_set_state(LED_TBOX_ID_GNSS, LED_TBOX_STATE_ERROR);
            return;
        }
    }
    else
    {
        led_tbox_ctx.gnss_ant_fault_count = 0U;
    }

    if (GNSS_POS_STATE_FIX == gnss_get_fix_state())
    {
        led_tbox_set_state(LED_TBOX_ID_GNSS, LED_TBOX_STATE_NORMAL);
    }
    else
    {
        led_tbox_set_state(LED_TBOX_ID_GNSS, LED_TBOX_STATE_ABNORMAL);
    }
}

static VOID led_tbox_timer_callback(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(LED, module_id);
    tbox_message_send(LED_TBOX_TIMER_EVENT, module_id, module_id, NULL_PTR);
}

static VOID led_tbox_start_mode(VOID)
{
    switch (led_tbox_ctx.start_count++)
    {
    case 0:
    {
        led_run_off();
        led_can_off();
        led_gnss_off();
        led_lte_off();
        break;
    }
    case 5:
    {
        led_run_on();
        break;
    }
    case 6:
    {
        led_can_on();
        break;
    }
    case 7:
    {
        led_gnss_on();
        break;
    }
    case 8:
    {
        led_lte_on();
        break;
    }
    case 9:
    {
        led_run_off();
        led_can_off();
        led_gnss_off();
        led_lte_off();
        break;
    }
    case 10:
    {
        led_run_on();
        break;
    }
    case 11:
    {
        led_can_on();
        break;
    }
    case 12:
    {
        led_gnss_on();
        break;
    }
    case 13:
    {
        led_lte_on();
        break;
    }
    case 14:
    {
        led_run_off();
        led_can_off();
        led_gnss_off();
        led_lte_off();
        led_tbox_ctx.start_count = 0;
        led_tbox_set_mode(LED_TBOX_MODE_RUN);
        stimer_start(led_tbox_timer, LED_RUN_MODE_PERIOD);
        break;
    }
    default:
        break;
    }
}

static VOID led_tbox_run_mode(VOID)
{
    led_tbox_check_can();
    led_tbox_check_lte();
    led_tbox_check_gnss();
    for (LED_TBOX_ID_E i = LED_TBOX_ID_RUN; i < LED_TBOX_ID_MAX; i++)
    {
        LED_TBOX_STATE_E state = led_tbox_get_state(i);
        BOOL enable = led_tbox_get_enable(i);
        led_tbox_handle_fun[i](state, enable);
    }
}

static VOID led_tbox_stop_mode(VOID)
{
    led_run_off();
    led_can_off();
    led_lte_off();
    led_gnss_off();
}

static VOID led_tbox_reset_content(VOID)
{
    led_tbox_ctx.mode = LED_TBOX_MODE_START;
    led_tbox_ctx.gnss_ant_fault_count = 0U;
    led_tbox_ctx.start_count = 0U;

    for (LED_TBOX_ID_E i = LED_TBOX_ID_RUN; i < LED_TBOX_ID_MAX; i++)
    {
        led_tbox_ctx.handle[i].id = i;
        led_tbox_ctx.handle[i].enable = TRUE;
        led_tbox_ctx.handle[i].state = LED_TBOX_STATE_NORMAL;
    }
}

INT32 led_tbox_init(VOID)
{
    INT32 ret = 0;

    led_tbox_timer = stimer_create(STIMER_TYPE_PERIOD, led_tbox_timer_callback);
    if (STIMER_ID_INVALID == led_tbox_timer)
    {
        MODULE_LOG_E(LED, "create led tbox timer failed");
        return -1;
    }

    led_tbox_mutex = xSemaphoreCreateMutex();
    if (NULL_PTR == led_tbox_mutex)
    {
        MODULE_LOG_E(LED, "led tbox mutex create failed");
        return -1;
    }

    led_tbox_reset_content();

    stimer_start(led_tbox_timer, LED_START_MODE_PERIOD);

    return ret;
}

VOID led_tbox_sleep(VOID)
{
    stimer_stop(led_tbox_timer);
    led_tbox_stop_mode();
}

VOID led_tbox_wake(VOID)
{
    xSemaphoreTake(led_tbox_mutex, portMAX_DELAY);
    led_tbox_reset_content();
    xSemaphoreGive(led_tbox_mutex);
    stimer_start(led_tbox_timer, LED_START_MODE_PERIOD);
}

VOID led_tbox_timeout(VOID)
{
    LED_TBOX_MODE_E mode = led_tbox_get_mode();
    switch (mode)
    {
    case LED_TBOX_MODE_START:
        led_tbox_start_mode();
        break;
    case LED_TBOX_MODE_RUN:
        led_tbox_run_mode();
        break;
    case LED_TBOX_MODE_STOP:
        led_tbox_stop_mode();
        break;
    default:
        break;
    }
}

VOID led_tbox_set_can(BOOL active)
{
    led_tbox_set_enable(LED_TBOX_ID_CAN, active);
}

VOID led_tbox_set_lte(BOOL active)
{
    led_tbox_set_enable(LED_TBOX_ID_LTE, active);
}

VOID led_tbox_set_gns(BOOL active)
{
    led_tbox_set_enable(LED_TBOX_ID_GNSS, active);
}
