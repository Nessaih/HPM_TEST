#ifndef __DRV_WDG_H__
#define __DRV_WDG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



int32_t drv_wdg_init(void);
int32_t drv_wdg_deinit(void);
int32_t drv_wdg_wake(void);
int32_t drv_wdg_sleep(void);
int32_t drv_wdg_feed(void);
void    drv_wdg_reset(void);

#ifdef __cplusplus
}
#endif

#endif //__DRV_WDG_H__