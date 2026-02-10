#ifndef __BATTERY_IF_H__
#define __BATTERY_IF_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CHARGE_MODE_OFF = 0,   /**< 未充电 */
    CHARGE_MODE_ON,        /**< 正在充电 */
    CHARGE_MODE_MAX
} charge_mode_t;

extern bool battery_is_charging(void);

#ifdef __cplusplus
}
#endif

#endif /* __BATTERY_IF_H__ */
