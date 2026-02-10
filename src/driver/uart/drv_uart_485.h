#ifndef __DRV_UART_485_H__
#define __DRV_UART_485_H__
    
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "drv_uart_def.h"


extern int32_t drv_uart_485_init(void);
extern int32_t drv_uart_485_deinit(void);
extern int32_t drv_uart_485_sleep(void);
extern int32_t drv_uart_485_wake(void);
extern int32_t drv_uart_485_tx(const uint8_t *data, uint32_t len);
extern int32_t drv_uart_485_rx(uint8_t *data, uint32_t len);
extern int32_t drv_uart_485_register(uart_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif //__DRV_UART_485_H__