#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "I2c_Hal.h"
#include "macros.h"

#define I2C_INSTANCE          0U
#define I2C_READ_BUFFER_SIZE  1U
#define I2C_WRITE_BUFFER_SIZE 16U
#define I2C_TIMEOUT           1200000

static bool    i2c_transmit_done  = false;
static bool    i2c_is_initialized = false;
static uint8_t i2c_read_buffer[I2C_READ_BUFFER_SIZE];
static uint8_t i2c_write_buffer[I2C_WRITE_BUFFER_SIZE];

static void update_transmit_struct(
    DataTransmitType *transmit,
    uint8_t           slave_addr,
    uint8_t          *data,
    uint16_t          data_len,
    I2c_Hal_DirType   dir,
    bool              send_stop)
{
    transmit->SlaveAddress  = slave_addr; 
    transmit->DirType       = dir;        
    transmit->SendStop      = send_stop;  
    transmit->Is10bitAddr   = FALSE;      
    transmit->DataLength    = data_len;   
    transmit->DataBufferPtr = data;       
}

static Hal_StatusType iic_wait_finish(uint32_t timeout)
{
    while (timeout > 0U) {
        if (i2c_transmit_done) {
            i2c_transmit_done = false;
            return STATUS_SUCCESS;
        }
        __NOP();
        --timeout;
    }
    return STATUS_TIMEOUT;
}

static void i2c_callback(uint8_t ins, uint32_t event)
{
    if (event == (uint32_t)I2C_MASTER_EVENT_END_TRANSFER) {
        i2c_transmit_done = true;
    }
}

int32_t drv_i2c_init(void)
{
    I2c_Hal_MasterConfigType master_cfg = {
        .BaudRate     = 100000,
        .TransferType = I2C_USING_INTERRUPTS,
    };
    I2c_Hal_ChannelConfigType channel_cfg = {
        .I2cMode         = I2C_MASTER,
        .MasterConfigPtr = &master_cfg,
        .Callback        = (I2c_Hal_CallbackType)i2c_callback,
    };

    I2c_Hal_Init(I2C_INSTANCE, &channel_cfg);
    (void)master_cfg;
    i2c_is_initialized = true;

    return 0;
}

int32_t drv_i2c_deinit(void)
{
    I2c_Hal_DeInit(I2C_INSTANCE);
    i2c_is_initialized = false;
    return 0;
}

int32_t drv_i2c_wake(void)
{
    if (!i2c_is_initialized) {
        drv_i2c_init();
    }
    return 0;
}

int32_t drv_i2c_sleep(void)
{
    if (i2c_is_initialized) {
        drv_i2c_deinit();
    }
    return 0;
}



int32_t drv_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    DataTransmitType transmit;
    Hal_StatusType   status;

    i2c_read_buffer[0] = reg_addr;
    update_transmit_struct(&transmit, dev_addr, i2c_read_buffer, 1, I2C_WRITE, FALSE);
    status = I2c_Hal_AsyncTransceive(I2C_INSTANCE, &transmit);
    if (status != STATUS_SUCCESS) {
        return -1;
    }
    status = iic_wait_finish(I2C_TIMEOUT);
    if (status != STATUS_SUCCESS) {
        return -2;
    }

    update_transmit_struct(&transmit, dev_addr, data, len, I2C_READ, TRUE);
    status = I2c_Hal_AsyncTransceive(I2C_INSTANCE, &transmit);
    if (status != STATUS_SUCCESS) {
        return -1;
    }
    status = iic_wait_finish(I2C_TIMEOUT);
    if (status != STATUS_SUCCESS) {
        return -2;
    }

    return 0;
}

int32_t drv_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    DataTransmitType transmit;
    Hal_StatusType   status;

    i2c_write_buffer[0] = reg_addr;

    len = MIN_VALUE(len, (uint16_t)(I2C_WRITE_BUFFER_SIZE - 1U));
    (void)memcpy(&i2c_write_buffer[1], data, len);
    update_transmit_struct(&transmit, dev_addr, i2c_write_buffer, len + 1U, I2C_WRITE, TRUE);
    status = I2c_Hal_AsyncTransceive(I2C_INSTANCE, &transmit);
    if (status != STATUS_SUCCESS) {
        return -1;
    }
    status = iic_wait_finish(I2C_TIMEOUT);
    if (status != STATUS_SUCCESS) {
        return -2;
    }

    return 0;
}
