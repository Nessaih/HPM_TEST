

#include <stdbool.h>
#include <stddef.h>
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

static uint8_t           eio_tx_buf[EIO_TX_SIZE];
static uint8_t           eio_rx_buf[EIO_RX_SIZE];
static uint8_t           eio_tq_buf[EIO_TQ_SIZE];
static uint8_t           eio_rq_buf[EIO_RQ_SIZE];
static cqueue_t          eio_queue_tx;
static cqueue_t          eio_queue_rx;
static volatile uint16_t eio_rx_len  = 0;
static volatile uint16_t eio_tx_len  = 0;
static volatile bool     eio_is_txbz = FALSE;
static uint32_t          eio_speed   = 0U;
static bool              eio_is_init = FALSE;

static void (*eio_rxcb)(void);

static void reverse_buffer(unsigned char *buffer, uint16_t length)
{
    uint16_t l, r;
    uint8_t  t;

    if (buffer == NULL || length == 0)
        return;

    for (l = 0, r = length - 1; l < r; l++, r--)
    {
        t         = buffer[l];
        buffer[l] = buffer[r];
        buffer[r] = t;
    }
}

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

    tmpvalue = (uint32_t)(2000000.0 / speed - 0.5);
    tmpvalue = (tmpvalue & 0xFFFFFF00U) ? 0xFFU : tmpvalue;
    tmpvalue = 0x0F00U | (tmpvalue & 0x00FF);

    Ckgen_Hal_EnablePeriphClk(CKGEN_EIO_BUS_CLK, TRUE);
    Rcm_Hal_SetResetState(RCM_RESET_ID_EIO, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_EIO, RCM_RESET_STATE_DEASSERT);

    MODIFY_REG32(EIO->CTRL, EIO_CTRL_SWRST_Msk, EIO_CTRL_SWRST_Pos, 1UL);
    WRITE_REG32(EIO->CTRL, 0);
    WRITE_REG32(EIO->CTRL, 1);

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
    WRITE_REG32(EIO->SHIFTSTAT, 0x03U);
    WRITE_REG32(EIO->TIMSTAT, 0x03U);
    WRITE_REG32(EIO->SHIFTSIEN, 0x02U);
    WRITE_REG32(EIO->SHIFTEIEN, 0x02U);
    // WRITE_REG32(EIO->CTRL, 0x01U);

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
    port.pull  = PIN_PULL_UP;
    port.mux   = PIN_MUX_OPTION6;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 4;
    gpio.mode  = GPIO_MODE_HIGH_Z;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 3;
    port.pull  = PIN_PULL_UP;
    port.mux   = PIN_MUX_OPTION6;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 3;
    gpio.mode  = GPIO_MODE_HIGH_Z;
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

#pragma diag_suppress = Pa082
static uint8_t drv_eio_get_state(void)
{
    uint32_t flag[3];
    uint8_t  i, j, state = 0;

    flag[0] = READ_BIT32(EIO->SHIFTSIEN, EIO->SHIFTSTAT);
    flag[1] = READ_BIT32(EIO->SHIFTEIEN, EIO->SHIFTERR);
    flag[2] = READ_BIT32(EIO->TIMIEN, EIO->TIMSTAT);

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (flag[j] & (1U << i))
            {
                state = state | (1U << (j + i * 4));
            }
        }
    }
    return state;
}
#pragma diag_default = Pa082

static void drv_eio_tx_process(uint8_t state)
{
    static uint8_t flush = 0;
    uint32_t       reg;

    if (0U == READ_BIT32(state, 0x0FU))
        return;

    if (READ_BIT32(state, 0x03))
    {
        if (eio_tx_len > 0)
        {
            reg = eio_tx_buf[--eio_tx_len];
            WRITE_REG32(EIO->SHIFTBUF[0], reg);
        }
        else
        {
            uint32_t l = cqueue_get(&eio_queue_tx, eio_tx_buf, EIO_TX_SIZE);
            if (l > 0)
            {
                reverse_buffer(eio_tx_buf, l);
                reg = eio_tx_buf[--l];
                WRITE_REG32(EIO->SHIFTBUF[0], reg);
                eio_tx_len = l;
            }
            else
            {
                CLEAR_BIT32(EIO->SHIFTSIEN, 1U);
                SET_BIT32(EIO->TIMSTAT, 1U);
                SET_BIT32(EIO->TIMIEN, 1U);
                flush = 2;
            }
        }

        if (READ_BIT(state, 1))
        {
            SET_BIT32(EIO->SHIFTERR, 1U);
        }
    }

    if (READ_BIT(state, 2U))
    {
        SET_BIT32(EIO->TIMSTAT, 1U);
        if (--flush > 0)
        {
            if (READ_BIT(EIO->SHIFTSTAT, 0))
            {
                reg = READ_REG32(EIO->SHIFTCFG[0]);
                MODIFY_REG32(reg, EIO_SHIFTCFG0_SSTART_Msk, EIO_SHIFTCFG0_SSTART_Pos, 3U);
                WRITE_REG32(EIO->SHIFTCFG[0], reg);
                reg = 0xFFFFFFFFU;
                WRITE_REG32(EIO->SHIFTBUF[0], reg);
            }
        }
        else
        {
            CLEAR_BIT32(EIO->TIMCTL[0], 0x03U);
            CLEAR_BIT32(EIO->SHIFTCTL[0], 0x07U);
            CLEAR_BIT32(EIO->SHIFTEIEN, 0x01UL);
            CLEAR_BIT32(EIO->TIMIEN, 0x01UL);

            MODIFY_REG32(EIO->SHIFTCFG[0], EIO_SHIFTCFG0_SSTART_Msk, EIO_SHIFTCFG0_SSTART_Pos, 2U);
            MODIFY_REG32(EIO->SHIFTCTL[0], EIO_SHIFTCTL0_SMOD_Msk, EIO_SHIFTCTL0_SMOD_Pos, 2U);
        }
    }
}

static void drv_eio_rx_process(uint8_t state)
{

    uint32_t reg;

    if (0U == READ_BIT32(state, 0xF0U))
        return;
    if (READ_BIT32(state, 0x30))
    {
        reg = READ_REG32(EIO->SHIFTBUFBYS[1]);
        if (READ_BIT(state, 5))
        {
            SET_BIT32(EIO->SHIFTERR, 2U);
        }

        if (eio_rx_len < EIO_RX_SIZE)
        {
            eio_rx_buf[eio_rx_len++] = reg;
            if (eio_rx_len == EIO_RX_SIZE)
            {
                cqueue_put(&eio_queue_rx, eio_rx_buf, eio_rx_len);
                eio_rx_len = 0;
                if (eio_rxcb)
                {
                    eio_rxcb();
                }
            }
        }
    }

    if (READ_BIT(state, 6))
    {
        SET_BIT32(EIO->TIMSTAT, 2);
    }
}

void EIO_IRQHandler(void)
{
    uint32_t state;

    state = drv_eio_get_state();
    drv_eio_tx_process(state);
    drv_eio_rx_process(state);
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

    if (cqueue_datalen(&eio_queue_rx))
    {
        len = cqueue_get(&eio_queue_rx, data, len);
    }
    else
    {
        len = 0;
    }

    return len;
}

int32_t drv_eio_write(uint8_t *data, uint32_t len)
{
    int32_t l;

    if (!eio_is_init)
        return -1;

    len = cqueue_put(&eio_queue_tx, data, len);
    if (eio_is_txbz)
        return len;

    l = cqueue_get(&eio_queue_tx, eio_tx_buf, EIO_TX_SIZE);
    if (l > 0)
    {
        eio_tx_len = l;
        reverse_buffer(eio_tx_buf, l);
        SET_BIT32(EIO->SHIFTERR, 0x01U);
        SET_BIT32(EIO->TIMCTL[0], 0x01U);
        SET_BIT32(EIO->SHIFTSIEN, 0x01U);
        SET_BIT32(EIO->SHIFTEIEN, 0x01U);
    }
    eio_is_txbz = TRUE;

    return len;
}
