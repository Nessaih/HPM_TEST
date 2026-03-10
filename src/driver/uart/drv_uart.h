#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "drv_uart_def.h"



extern int32_t drv_uart_init(void);
extern int32_t drv_uart_deinit(void);
extern int32_t drv_uart_sleep(void);
extern int32_t drv_uart_wake(void);
extern int32_t drv_uart_tx(const uint8_t *data, uint32_t len);
extern int32_t drv_uart_rx(uint8_t *data, uint32_t len);
extern int32_t drv_uart_get_data(uint8_t *data, uint32_t len);
extern void drv_uart_discard_data(uint32_t len);
extern int32_t drv_uart_register(uart_cb_t cb);
extern void drv_uart_flush(void);

#ifdef __cplusplus
}
#endif

#endif //__DRV_UART_H__