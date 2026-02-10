#include "drv_pin.h"
#include "stimer.h"

#include "led_key.h"

#if defined(KEY_LED_ENABLE) && (1 == KEY_LED_ENABLE)

#define LED_TICK_MS   100

#define LED_RED_ID    0
#define LED_GRE_ID    1
#define LED_ID_CNT    2

#define STATE_LED_OFF 0
#define STATE_LED_ON  1
#define STATE_LED_INV 2

typedef struct
{
    UINT8 hp  : 6;
    UINT8 org : 1;
    UINT8 act : 1;
} LED_KEY_MODE_T;

typedef struct
{
    UINT8 tick;
    UINT8 pin;
    UINT8 id    : 2;
    UINT8 state : 2;
    UINT8 mode  : 4;
} LED_KEY_STATE_T;

const LED_KEY_MODE_T led_modes[DRV_KLED_MODE_COUNT][LED_ID_CNT] = {
    {{0, 0, 0}, {0, 0, 0}}, // 00 --  红绿常灭
    {{0, 1, 0}, {0, 0, 0}}, // 01 --  红色常亮
    {{0, 0, 0}, {0, 1, 0}}, // 02 --  绿色常亮
    {{0, 1, 0}, {0, 1, 0}}, // 03 --  红绿常亮
    {{5, 0, 1}, {0, 0, 0}}, // 04 --  红色闪烁 1 Hz
    {{0, 0, 0}, {5, 0, 1}}, // 05 --  绿色闪烁 1 Hz
    {{5, 1, 1}, {5, 0, 1}}, // 06 --  红绿交替 1 Hz
    {{5, 0, 1}, {5, 0, 1}}, // 07 --  红绿同步 1 Hz
    {{2, 0, 1}, {0, 0, 0}}, // 08 --  红色闪烁 5 Hz
    {{0, 0, 0}, {2, 0, 1}}, // 09 --  绿色闪烁 5 Hz
    {{2, 1, 1}, {2, 0, 1}}, // 10 --  红绿交替 5 Hz
    {{2, 0, 1}, {2, 0, 1}}, // 11 --  红绿同步 5 Hz
    {{1, 0, 1}, {0, 0, 0}}, // 12 --  红色闪烁 10 Hz
    {{0, 0, 0}, {1, 0, 1}}, // 13 --  绿色闪烁 10 Hz
    {{1, 1, 1}, {1, 0, 1}}, // 14 --  红绿交替 10 Hz
    {{1, 0, 1}, {1, 0, 1}}, // 15 --  红绿同步 10 Hz
};

static LED_KEY_STATE_T led_state[LED_ID_CNT];
static UINT8           led_mode = 0;
static STIMER_ID       led_timer;

static VOID led_key_on(VOID)
{
    drv_pin_set_level(PIN_LED_KEY_SOS, 1U);
    drv_pin_set_level(PIN_LED_KEY_SVC, 1U);
}

static VOID led_key_off(VOID)
{
    drv_pin_set_level(PIN_LED_KEY_SOS, 0U);
    drv_pin_set_level(PIN_LED_KEY_SVC, 0U);
    drv_pin_set_level(PIN_LED_KEY_RED, 0U);
    drv_pin_set_level(PIN_LED_KEY_GRE, 0U);
}

static VOID led_key_handle(LED_KEY_STATE_T *led)
{
    UINT8 hp, act;

    hp  = led_modes[led->mode][led->id].hp;
    act = led_modes[led->mode][led->id].act;

    if (act)
        led->tick++;
    else
        led->tick = 0;

    switch (led->state) {
    case STATE_LED_OFF:
        drv_pin_set_level(led->pin, 0);
        if (act)
            led->state = STATE_LED_INV;
        break;

    case STATE_LED_ON:
        drv_pin_set_level(led->pin, 1);
        if (act)
            led->state = STATE_LED_INV;
        break;

    case STATE_LED_INV:
        if (led->tick > hp) {
            led->tick = 0;
            drv_pin_toggle(led->pin);
        }
        break;

    default:
        break;
    }
}

VOID led_key_set_mode(UINT8 mode)
{
    if (mode >= DRV_KLED_MODE_COUNT)
        return;

    led_mode                   = mode;
    led_state[LED_RED_ID].pin  = PIN_LED_KEY_RED;
    led_state[LED_RED_ID].mode = mode;
    led_state[LED_RED_ID].id   = LED_RED_ID;
    if (led_modes[mode][LED_RED_ID].org)
        led_state[LED_RED_ID].state = STATE_LED_ON;
    else
        led_state[LED_RED_ID].state = STATE_LED_OFF;

    led_state[LED_GRE_ID].pin  = PIN_LED_KEY_GRE;
    led_state[LED_GRE_ID].mode = mode;
    led_state[LED_GRE_ID].id   = LED_GRE_ID;
    if (led_modes[mode][LED_GRE_ID].org)
        led_state[LED_GRE_ID].state = STATE_LED_ON;
    else
        led_state[LED_GRE_ID].state = STATE_LED_OFF;
}

/**
 *@brief  需要按照100ms的周期调用该函数
 *@param  None
 *@retval None
 *@date   2025-02-14
 */
VOID led_key_polling(VOID)
{
    for (INT32 i = 0; i < LED_ID_CNT; i++) {
        led_key_handle(&led_state[i]);
    }
}

INT32 led_key_init(VOID)
{
    INT32 ret = 0;

    led_key_set_mode(3);
    led_key_on();

    led_timer = stimer_create(STIMER_TYPE_PERIOD, led_key_timeout);
    ret |= stimer_start(led_timer, 100);

    return 0;
}

VOID led_key_sleep(VOID)
{
    UINT8 mode;

    stimer_stop(&led_timer);

    mode = led_mode;     // get the current mode
    led_key_set_mode(0); // set off mode
    led_mode = mode;     // record the current mode

    led_key_off();
}

VOID led_key_wake(VOID)
{
    led_key_set_mode(led_mode); // recover the led mode
    led_key_on();
    stimer_start(&led_timer, 100);
}


#endif // KEY_LED_ENABLE

