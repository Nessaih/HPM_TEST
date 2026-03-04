

#include <stdbool.h>
#include <stdint.h>
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "Device_Register.h"
#include "Rcm_Hal.h"
#include "cqueue.h"
#include "drv_pin.h"

#define EIO_TX_SIZE 128U
#define EIO_RX_SIZE 128U
#define EIO_TQ_SIZE 512U
#define EIO_RQ_SIZE 2048U

static uint8_t       eio_tx_buf[EIO_TX_SIZE];
static uint8_t       eio_rx_buf[EIO_RX_SIZE];
static uint8_t       eio_tq_buf[EIO_TQ_SIZE];
static uint8_t       eio_rq_buf[EIO_RQ_SIZE];
static cqueue_t      eio_queue_tx;
static cqueue_t      eio_queue_rx;
static uint16_t      eio_rx_len  = 0;
static uint16_t      eio_tx_len  = 0;
static uint32_t      eio_speed   = 0U;
static bool          eio_is_init = FALSE;
static volatile bool eio_is_txbz = FALSE;

static void (*eio_rxcb)(void);

static void drv_eio_init_data(void (*rxcb)(void))
{
    cqueue_init(&eio_queue_tx, eio_tq_buf, EIO_TQ_SIZE);
    cqueue_init(&eio_queue_rx, eio_rq_buf, EIO_RQ_SIZE);
    eio_tx_len = 0;
    eio_rx_len = 0;
    if (rxcb)
        eio_rxcb = rxcb;
}

static void drv_eio_init_device(uint32_t speed)
{
    uint32_t tmpvalue;

    if (speed == 0)
        return;

    eio_speed = speed;

    tmpvalue = 2000000U / speed;
    tmpvalue = (tmpvalue & 0xFFFFFF00U) ? 0xFFU : tmpvalue;
    tmpvalue = 0x0F00U | (tmpvalue & 0x00FF);

    Ckgen_Hal_EnablePeriphClk(CKGEN_EIO_BUS_CLK, TRUE);
    Rcm_Hal_SetResetState(RCM_RESET_ID_EIO, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_EIO, RCM_RESET_STATE_DEASSERT);

    MODIFY_REG32(EIO->CTRL, EIO_CTRL_SWRST_Msk, EIO_CTRL_SWRST_Pos, 1UL);
    WRITE_REG32(EIO->CTRL, 0);

    Core_Hal_DisableIrq(EIO_IRQn);

    WRITE_REG32(EIO->SHIFTCFG[0], 0x00000032U);
    WRITE_REG32(EIO->SHIFTCTL[0], 0x00030002U);
    WRITE_REG32(EIO->TIMCMP[0], tmpvalue);
    WRITE_REG32(EIO->TIMCFG[0], 0x00002222U);
    WRITE_REG32(EIO->TIMCTL[0], 0x01c00000U);

    WRITE_REG32(EIO->SHIFTCFG[1], 0x00000032U);
    WRITE_REG32(EIO->SHIFTCTL[1], 0x01800101U);
    WRITE_REG32(EIO->TIMCMP[1], tmpvalue);
    WRITE_REG32(EIO->TIMCFG[1], 0x02042422U);
    WRITE_REG32(EIO->TIMCTL[1], 0x00000181U);

    tmpvalue = READ_REG32(EIO->SHIFTBUF[1]);
    WRITE_REG32(EIO->SHIFTERR, 0x03U);
    WRITE_REG32(EIO->SHIFTERR, 0x03U);
    WRITE_REG32(EIO->SHIFTSTAT, 0x03U);
    WRITE_REG32(EIO->SHIFTSTAT, 0x03U);
    WRITE_REG32(EIO->SHIFTSIEN, 0x02U);
    WRITE_REG32(EIO->SHIFTEIEN, 0x02U);
    WRITE_REG32(EIO->CTRL, 0x05U);

    Core_Hal_EnableIrq(EIO_IRQn);
}

static void drv_eio_deinit_device(void)
{
    Core_Hal_DisableIrq(EIO_IRQn);
    WRITE_REG32(EIO->CTRL, 0);
    Rcm_Hal_SetResetState(RCM_RESET_ID_EIO, RCM_RESET_STATE_ASSERT);
    Ckgen_Hal_EnablePeriphClk(CKGEN_EIO_BUS_CLK, FALSE);
}

static void drv_eio_init_pin(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 4;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_OPTION6;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 4;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 3;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_OPTION6;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 3;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
}

static void drv_eio_deinit_pin(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 4;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 4;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_LOW;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 3;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 3;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_LOW;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
}

void EIO_IRQHandler(void)
{
    uint32_t reg;
    uint8_t  data;
    uint8_t  mask = 0;

    if (READ_BIT(EIO->SHIFTERR, 1))
    {
        reg = READ_REG32(EIO->SHIFTBUF[1]);
        mask |= 3U;
    }
    else if (READ_BIT(EIO->SHIFTSTAT, 1))
    {
        reg = READ_REG32(EIO->SHIFTBUF[1]);
        mask |= 2U;
    }
    else
    {
        mask = 0U;
    }

    if (mask & 1U)
    {
        WRITE_REG32(EIO->SHIFTERR, 0x02U);
    }

    if (mask & 2U)
    {
        data = reg >> 24U;
        if (eio_rx_len >= EIO_RX_SIZE)
        {
            cqueue_put(&eio_queue_rx, eio_rx_buf, eio_rx_len);
            eio_rx_len = 0;
        }
        eio_rx_buf[eio_rx_len++] = data;

        if (eio_rxcb)
        {
            eio_rxcb();
        }
    }
}

int32_t drv_eio_init(uint32_t speed, void (*rxcb)(void))
{
    drv_eio_init_data(rxcb);
    drv_eio_init_device(speed);
    eio_is_init = TRUE;
    return 0;
}

int32_t drv_eio_deinit(void)
{
    drv_eio_deinit_device();
    eio_is_init = FALSE;
    return 0;
}

int32_t drv_eio_sleep(void)
{
    if (eio_is_init)
    {
        drv_eio_deinit();
        drv_eio_deinit_pin();
    }
    return 0;
}

int32_t drv_eio_wake(void)
{
    if (!eio_is_init)
    {
        drv_eio_init_pin();
        drv_eio_init(eio_speed, eio_rxcb);
    }
    return 0;
}

int32_t drv_eio_read(uint8_t *data, uint32_t len)
{
    int32_t l = 0;

    if (cqueue_datalen(&eio_queue_rx))
    {
        l = cqueue_get(&eio_queue_rx, data, len);
    }

    return l;
}

int32_t drv_eio_write(uint8_t *data, uint32_t len)
{
    int32_t l = 0;

    if (!eio_is_init)
        return -1;

    if (eio_is_txbz)
    {
        l = cqueue_put(&eio_queue_tx, data, len);
    }
    else
    {
        l = cqueue_put(&eio_queue_tx, data, len);
        l = cqueue_get(&eio_queue_tx, eio_tx_buf, EIO_TX_SIZE);
        if (l > 0)
        {
            eio_tx_len = l-1;
#if 0
            WRITE_REG32(EIO->SHIFTBUF[0], eio_tx_buf[0]);
            MODIFY_REG32(EIO->SHIFTSIEN, 1U, 0U, 1U);
            MODIFY_REG32(EIO->SHIFTEIEN, 1U, 0U, 1U);
            // todo isr process
#endif
        }

        eio_is_txbz = TRUE;
    }

    return 0;
}
