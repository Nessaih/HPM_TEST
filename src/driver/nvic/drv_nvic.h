#ifndef __DRV_NVIC_H__
#define __DRV_NVIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int32_t drv_nvic_init(void);
int32_t drv_nvic_deinit(void);

#ifdef __cplusplus
}
#endif

#endif //__DRV_NVIC_H__