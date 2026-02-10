#ifndef __LED_KEY_H__
#define __LED_KEY_H__

#include <stdint.h>

// #define KEY_LED_ENABLE 1

#if defined(KEY_LED_ENABLE) && (1 == KEY_LED_ENABLE)

#define DRV_KLED_MODE_COUNT 16

extern VOID    led_key_set_mode(UINT8 mode);
extern VOID    led_key_timeout(VOID);
extern VOID    led_key_sleep(VOID);
extern VOID    led_key_wake(VOID);
extern INT32   led_key_init(VOID);

#else

#define led_key_set_mode(x) ((VOID)0)
#define led_key_timeout()   ((VOID)0)
#define led_key_sleep()     ((VOID)0)
#define led_key_wake()      ((VOID)0)
#define led_key_init()      ((VOID)0)

#endif

#endif //__LED_KEY_H__

