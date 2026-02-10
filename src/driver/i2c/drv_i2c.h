#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern int32_t drv_i2c_init(void);
extern int32_t drv_i2c_deinit(void);
extern int32_t drv_i2c_wake(void);
extern int32_t drv_i2c_sleep(void);
extern int32_t drv_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
extern int32_t drv_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif //__DRV_I2C_H__