#ifndef __DRV_EIO_GNSS_H__
#define __DRV_EIO_GNSS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "drv_eio_def.h"

extern int32_t drv_eio_gnss_init(void);
extern int32_t drv_eio_gnss_deinit(void);
extern int32_t drv_eio_gnss_sleep(void);
extern int32_t drv_eio_gnss_wake(void);
extern int32_t drv_eio_gnss_tx(const uint8_t *data, uint32_t len);
extern int32_t drv_eio_gnss_rx(uint8_t *data, uint32_t len);
extern int32_t drv_eio_gnss_register(uart_cb_t cb);

extern void    drv_eio_gnss_set_rate(uint32 rate);
extern uint32  drv_eio_gnss_get_rate(void);


#ifdef __cplusplus
}
#endif

#endif
