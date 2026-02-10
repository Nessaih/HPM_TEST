#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "Device_Register.h"
#include "Uart_Hal.h"
#include "cqueue.h"
#include "delay.h"
#include "drv_pin.h"
#include "drv_uart.h"
#include "drv_wdg.h"

#define UART_BT_INSTANCE        2U
#define UART_BT_BASE            UART2
#define UART_BT_IRQN            UART2_IRQn
#define UART_BT_RATE            115200U
#define UART_BT_TX_SIZE         128U
#define UART_BT_RX_SIZE         128U
#define UART_BT_TQ_SIZE         512U
#define UART_BT_RQ_SIZE         512U

#define UART_BT_RX_STATE_IDLE   0
#define UART_BT_RX_STATE_DATAIN 1

#define UART_BT_TX_STATE_IDLE   0
#define UART_BT_TX_STATE_BUZY   1

#define UART_BT_DIR_TX          1
#define UART_BT_DIR_RX          0

typedef struct
{
    volatile int32_t state;
    uart_cb_t        callback;
    cqueue_t         queue;
} uart_channel_t;

typedef struct
{
    bool           init;
    uart_channel_t rx;
    uart_channel_t tx;
} uart_device_t;

static uint8_t       uart_tx_buf[UART_BT_TX_SIZE];
static uint8_t       uart_rx_buf[UART_BT_RX_SIZE];
static uint8_t       uart_tq_buf[UART_BT_TQ_SIZE];
static uint8_t       uart_rq_buf[UART_BT_RQ_SIZE];

static uart_device_t uart_device;

static void rx_callback(uint8 instance, Uart_EventType event)
{
    uart_device_t *dev = &uart_device;
    uint32_t       l   = 0U;
    Hal_StatusType status;

    NVIC_DisableIRQ(UART_BT_IRQN);
    switch (event)
    {
    case UART_EVENT_END_TRANSFER:
        l = UART_BT_RX_SIZE;
        break;

    case UART_EVENT_IDLE_LINE:
        status = Uart_Hal_GetReceiveStatus(UART_BT_INSTANCE, &l);
        l      = UART_BT_RX_SIZE - l;
        if (STATUS_BUSY == status)
        {
            Uart_Hal_AbortReceivingData(UART_BT_INSTANCE);
        }
        break;

    case UART_EVENT_ERROR:
        break;

    default:
        break;
    }
    Uart_Hal_ReceiveData(UART_BT_INSTANCE, uart_rx_buf, UART_BT_RX_SIZE);

    NVIC_EnableIRQ(UART_BT_IRQN);

    if (l > 0U)
    {
        l = cqueue_put(&dev->rx.queue, uart_rx_buf, l);
        if (dev->rx.callback)
        {
            dev->rx.callback();
        }
    }
}

static void tx_callback(uint8 instance, Uart_EventType event)
{
    uart_device_t *dev = &uart_device;

    NVIC_DisableIRQ(UART_BT_IRQN);
    switch (event)
    {
    case UART_EVENT_END_TRANSFER:
    {
        uint32_t gl = cqueue_get(&dev->tx.queue, uart_tx_buf, UART_BT_TX_SIZE);
        if (gl > 0U)
        {

            Hal_StatusType ret = Uart_Hal_SendData(UART_BT_INSTANCE, uart_tx_buf, gl);
            if (STATUS_SUCCESS != ret)
            {
                cqueue_put(&dev->tx.queue, uart_tx_buf, gl);
                dev->tx.state = UART_BT_TX_STATE_IDLE;
            }
            else
            {
                dev->tx.state = UART_BT_TX_STATE_BUZY;
            }
        }
        else
        {
            dev->tx.state = UART_BT_TX_STATE_IDLE;
        }
        break;
    }

    case UART_EVENT_TX_EMPTY:
        break;

    default:
        break;
    }
    NVIC_EnableIRQ(UART_BT_IRQN);
}

static const Uart_ChannelConfigType uart_config = {
    .BaudRate        = UART_BT_RATE,
    .ParityMode      = UART_PARITY_DISABLED,
    .StopBitCount    = UART_ONE_STOP_BIT,
    .BitCountPerChar = UART_8_BITS_PER_CHAR,
    .TransferType    = UART_USING_INTERRUPTS,
    .SampleCnt       = UART_SMP_CNT16,
    .RxDmaChannel    = 0,
    .TxDmaChannel    = 0,
    .RxCallback      = rx_callback,
    .TxCallback      = tx_callback,
};

int32_t drv_uart_bt_init(void)
{
    Hal_StatusType status;

    uart_device.init = false;

    uart_device.rx.state = UART_BT_RX_STATE_IDLE;
    cqueue_init(&uart_device.rx.queue, uart_rq_buf, UART_BT_RQ_SIZE);

    uart_device.tx.state = UART_BT_TX_STATE_IDLE;
    cqueue_init(&uart_device.tx.queue, uart_tq_buf, UART_BT_TQ_SIZE);

    Uart_Hal_Init(UART_BT_INSTANCE, &uart_config);
    Uart_Hal_SetIdleInterrupt(UART_BT_INSTANCE, true);
    status = Uart_Hal_ReceiveData(UART_BT_INSTANCE, uart_rx_buf, UART_BT_RX_SIZE);

    if (STATUS_SUCCESS != status)
    {
        return -1;
    }
    uart_device.init = true;

    return 0;
}

int32_t drv_uart_bt_deinit(void)
{
    Uart_Hal_DeInit(UART_BT_INSTANCE);
    uart_device.init = false;
    return 0;
}

void drv_uart_bt_gpio_sleep(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 31;
    port.pull  = PIN_PULL_DOWN;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 31;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_LOW;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 32;
    port.pull  = PIN_PULL_DOWN;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 32;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_LOW;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
}

void drv_uart_bt_gpio_wake(void)
{

    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 31;
    port.pull  = PIN_PULL_DOWN;
    port.mux   = PIN_MUX_OPTION2;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 31;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 32;
    port.pull  = PIN_PULL_DOWN;
    port.mux   = PIN_MUX_OPTION2;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 32;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
}

int32_t drv_uart_bt_sleep(void)
{
    if (uart_device.init)
    {
        drv_uart_bt_deinit();
        drv_uart_bt_gpio_sleep();
    }
    return 0;
}

int32_t drv_uart_bt_wake(void)
{
    if (!uart_device.init)
    {
        drv_uart_bt_gpio_wake();
        drv_uart_bt_init();
    }
    return 0;
}

int32_t drv_uart_bt_tx(const uint8_t *data, uint32_t len)
{
    int32_t  ret = -1;
    uint32_t l;

    if ((0U == uart_device.tx.queue.size) || (0U == len))
    {
        return -1;
    }

    if (cqueue_surplus(&uart_device.tx.queue) < len)
    {
        return -1;
    }

    NVIC_DisableIRQ(UART_BT_IRQN);
    cqueue_put(&uart_device.tx.queue, data, len);
    if (uart_device.init)
    {
        bool isIdle = (UART_BT_TX_STATE_IDLE == uart_device.tx.state);
        if (isIdle)
        {

            l                     = cqueue_get(&uart_device.tx.queue, uart_tx_buf, UART_BT_TX_SIZE);
            Hal_StatusType halRet = Uart_Hal_SendData(UART_BT_INSTANCE, uart_tx_buf, l);
            if (STATUS_SUCCESS == halRet)
            {
                uart_device.tx.state = UART_BT_TX_STATE_BUZY;
                ret                  = (int32_t)l;
            }
            else
            {
                uart_device.tx.state = UART_BT_TX_STATE_IDLE;
            }
        }
    }
    NVIC_EnableIRQ(UART_BT_IRQN);

    return ret;
}

int32_t drv_uart_bt_rx(uint8_t *data, uint32_t len)
{
    uint32_t l;

    if (0U == uart_device.rx.queue.size)
    {
        return -1;
    }

    NVIC_DisableIRQ(UART_BT_IRQN);
    l = cqueue_get(&uart_device.rx.queue, data, len);
    NVIC_EnableIRQ(UART_BT_IRQN);
    return (int32_t)l;
}

int32_t drv_uart_bt_register(uart_cb_t cb)
{
    uart_device.rx.callback = cb;
    return 0;
}
