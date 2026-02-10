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

#define UART_485_INSTANCE        3U
#define UART_485_BASE            UART3
#define UART_485_IRQN            UART3_IRQn
#define UART_485_RATE            115200U
#define UART_485_TX_SIZE         128U
#define UART_485_RX_SIZE         128U
#define UART_485_TQ_SIZE         512U
#define UART_485_RQ_SIZE         512U

#define UART_485_RX_STATE_IDLE   0
#define UART_485_RX_STATE_DATAIN 1

#define UART_485_TX_STATE_IDLE   0
#define UART_485_TX_STATE_BUZY   1

#define UART_485_DIR_TX          1
#define UART_485_DIR_RX          0

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

static uint8_t       uart_tx_buf[UART_485_TX_SIZE];
static uint8_t       uart_rx_buf[UART_485_RX_SIZE];
static uint8_t       uart_tq_buf[UART_485_TQ_SIZE];
static uint8_t       uart_rq_buf[UART_485_RQ_SIZE];

static uart_device_t uart_device;

static void          rx_callback(uint8 instance, Uart_EventType event)
{
    uart_device_t *dev = &uart_device;
    uint32_t       l   = 0U;
    Hal_StatusType status;

    NVIC_DisableIRQ(UART_485_IRQN);
    switch (event)
    {
    case UART_EVENT_END_TRANSFER:
        l = UART_485_RX_SIZE;
        break;

    case UART_EVENT_IDLE_LINE:
        status = Uart_Hal_GetReceiveStatus(UART_485_INSTANCE, &l);
        l      = UART_485_RX_SIZE - l;
        if (STATUS_BUSY == status)
        {
            Uart_Hal_AbortReceivingData(UART_485_INSTANCE);
        }
        break;

    case UART_EVENT_ERROR:
        break;

    default:
        break;
    }
    Uart_Hal_ReceiveData(UART_485_INSTANCE, uart_rx_buf, UART_485_RX_SIZE);

    NVIC_EnableIRQ(UART_485_IRQN);

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

    NVIC_DisableIRQ(UART_485_IRQN);
    switch (event)
    {
    case UART_EVENT_END_TRANSFER:
    {
        uint32_t gl = cqueue_get(&dev->tx.queue, uart_tx_buf, UART_485_TX_SIZE);
        if (gl > 0U)
        {
            drv_pin_set_level(PIN_485_DIR, UART_485_DIR_TX);

            Hal_StatusType ret = Uart_Hal_SendData(UART_485_INSTANCE, uart_tx_buf, gl);
            if (STATUS_SUCCESS != ret)
            {
                cqueue_put(&dev->tx.queue, uart_tx_buf, gl);
                dev->tx.state = UART_485_TX_STATE_IDLE;
                drv_pin_set_level(PIN_485_DIR, UART_485_DIR_RX);
            }
            else
            {
                dev->tx.state = UART_485_TX_STATE_BUZY;
            }
        }
        else
        {
            dev->tx.state = UART_485_TX_STATE_IDLE;
            drv_pin_set_level(PIN_485_DIR, UART_485_DIR_RX);
        }
        break;
    }

    case UART_EVENT_TX_EMPTY:
        break;

    default:
        break;
    }
    NVIC_EnableIRQ(UART_485_IRQN);
}

static const Uart_ChannelConfigType uart_config = {
    .BaudRate        = UART_485_RATE,
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

int32_t drv_uart_485_init(void)
{
    Hal_StatusType status;

    uart_device.init = false;

    uart_device.rx.state = UART_485_RX_STATE_IDLE;
    cqueue_init(&uart_device.rx.queue, uart_rq_buf, UART_485_RQ_SIZE);

    uart_device.tx.state = UART_485_TX_STATE_IDLE;
    cqueue_init(&uart_device.tx.queue, uart_tq_buf, UART_485_TQ_SIZE);

    Uart_Hal_Init(UART_485_INSTANCE, &uart_config);
    Uart_Hal_SetIdleInterrupt(UART_485_INSTANCE, true);
    status = Uart_Hal_ReceiveData(UART_485_INSTANCE, uart_rx_buf, UART_485_RX_SIZE);

    if (STATUS_SUCCESS != status)
    {
        return -1;
    }
    uart_device.init = true;

    return 0;
}

int32_t drv_uart_485_deinit(void)
{
    Uart_Hal_DeInit(UART_485_INSTANCE);
    uart_device.init = false;
    return 0;
}

int32_t drv_uart_485_sleep(void)
{
    if (uart_device.init)
    {
        drv_uart_485_deinit();
    }
    return 0;
}

int32_t drv_uart_485_wake(void)
{
    if (!uart_device.init)
    {
        drv_uart_485_init();
    }
    return 0;
}

int32_t drv_uart_485_tx(const uint8_t *data, uint32_t len)
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

    NVIC_DisableIRQ(UART_485_IRQN);
    cqueue_put(&uart_device.tx.queue, data, len);
    if (uart_device.init)
    {
        bool isIdle = (UART_485_TX_STATE_IDLE == uart_device.tx.state);
        if (isIdle)
        {
            drv_pin_set_level(PIN_485_DIR, UART_485_DIR_TX);
            l                     = cqueue_get(&uart_device.tx.queue, uart_tx_buf, UART_485_TX_SIZE);
            Hal_StatusType halRet = Uart_Hal_SendData(UART_485_INSTANCE, uart_tx_buf, l);
            if (STATUS_SUCCESS == halRet)
            {
                uart_device.tx.state = UART_485_TX_STATE_BUZY;
                ret                  = (int32_t)l;
            }
            else
            {
                uart_device.tx.state = UART_485_TX_STATE_IDLE;
            }
        }
    }
    NVIC_EnableIRQ(UART_485_IRQN);

    return ret;
}

int32_t drv_uart_485_rx(uint8_t *data, uint32_t len)
{
    uint32_t l;

    if (0U == uart_device.rx.queue.size)
    {
        return -1;
    }

    NVIC_DisableIRQ(UART_485_IRQN);
    l = cqueue_get(&uart_device.rx.queue, data, len);
    NVIC_EnableIRQ(UART_485_IRQN);
    return (int32_t)l;
}

int32_t drv_uart_485_register(uart_cb_t cb)
{
    uart_device.rx.callback = cb;
    return 0;
}
