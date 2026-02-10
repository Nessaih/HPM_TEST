#ifndef __DRV_UART_BT_H__
#define __DRV_UART_BT_H__
    
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "drv_uart_def.h"


extern int32_t drv_uart_bt_init(void);
extern int32_t drv_uart_bt_deinit(void);
extern int32_t drv_uart_bt_sleep(void);
extern int32_t drv_uart_bt_wake(void);
extern int32_t drv_uart_bt_tx(const uint8_t *data, uint32_t len);
extern int32_t drv_uart_bt_rx(uint8_t *data, uint32_t len);
extern int32_t drv_uart_bt_register(uart_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif //__DRV_UART_BT_H__