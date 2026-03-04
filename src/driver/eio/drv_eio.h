#ifndef __DRV_EIO_H__
#define __DRV_EIO_H__

#include <stdint.h>

int32_t drv_eio_init(uint32_t speed, void (*rxcb)(void));
int32_t drv_eio_deinit(void);
int32_t drv_eio_sleep(void);
int32_t drv_eio_wake(void);
int32_t drv_eio_read(uint8_t *data, uint32_t len);
int32_t drv_eio_write(uint8_t *data, uint32_t len);

#endif //__DRV_EIO_H__