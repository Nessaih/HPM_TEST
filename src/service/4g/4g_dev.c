#include "tbox_common.h"
#include "tbox_core.h"
#include "Device_Register.h"
#include "Uart_Hal.h"
#include "4g_content.h"
#include "4g_dev.h"
#include "4g_data.h"
#include "4g_mgr.h"

#define DEV_4G_STATE_CLOSED 0U
#define DEV_4G_STATE_OPENED 1U

static DEV_4G_SEND_CALLBACK send_callback = NULL_PTR;
static UINT8 dev_4g_is_open = (UINT8)DEV_4G_STATE_CLOSED;
static UINT8 dev_4g_rx_buffer[DEV_4G_RXBUF_SIZE];

static VOID dev_4g_rx_callback(UINT8 instance, Uart_EventType event)
{
    UINT32         len;
    Hal_StatusType status;
	
    NVIC_DisableIRQ(UART1_IRQn);
    switch (event)
    {
    case UART_EVENT_END_TRANSFER:
        len = DEV_4G_RXBUF_SIZE;
        break;

    case UART_EVENT_IDLE_LINE:
        status = Uart_Hal_GetReceiveStatus(DEV_4G_INSTANCE, &len);
        len    = DEV_4G_RXBUF_SIZE - len;
        if (STATUS_BUSY == status)
        {
            Uart_Hal_AbortReceivingData(DEV_4G_INSTANCE);
        }
        break;

    case UART_EVENT_ERROR:
        len = 0;
        break;

    default:
        len = 0;
        break;
    }

    if (len > 0U && len <= DEV_4G_RXBUF_SIZE)
    {
        //dev_4g_rx_buffer[len] = '\0';
        
        for (UINT16 i = 0; i < len; i++)
        {
            data_4g_recv_push(dev_4g_rx_buffer[i]);
        }
    }
    Uart_Hal_ReceiveData(DEV_4G_INSTANCE, dev_4g_rx_buffer, DEV_4G_RXBUF_SIZE);
    NVIC_EnableIRQ(UART1_IRQn);
}

static VOID dev_4g_tx_callback(UINT8 instance, Uart_EventType event)
{
    UNUSED(instance);

    if(instance != DEV_4G_INSTANCE)
    {
        return;
    }

	if(UART_EVENT_END_TRANSFER != event)
	{
		return;
	}
    
    NVIC_DisableIRQ(UART1_IRQn);
    if(NULL_PTR != send_callback)
    {
        send_callback();
    }
    NVIC_EnableIRQ(UART1_IRQn);
}

INT32 dev_4g_open(VOID)
{
    if((UINT8)DEV_4G_STATE_OPENED == dev_4g_is_open)
    {
        MODULE_LOG_W(TBOX4G, "4g uart is already opened");
        Uart_Hal_ReceiveData(DEV_4G_INSTANCE, dev_4g_rx_buffer, DEV_4G_RXBUF_SIZE);
        return (INT32)TBOX_E_HASSTART;
    }

    static const Uart_ChannelConfigType lte_uart_config = {
        .BaudRate        = 115200U,
        .ParityMode      = UART_PARITY_DISABLED,
        .StopBitCount    = UART_ONE_STOP_BIT,
        .BitCountPerChar = UART_8_BITS_PER_CHAR,
        .TransferType    = UART_USING_INTERRUPTS,
        .SampleCnt       = UART_SMP_CNT16,
        .RxDmaChannel    = 0,
        .TxDmaChannel    = 0,
        .RxCallback      = dev_4g_rx_callback,
        .TxCallback      = dev_4g_tx_callback,
    };

    Uart_Hal_Init(DEV_4G_INSTANCE, &lte_uart_config);
    Uart_Hal_SetIdleInterrupt(DEV_4G_INSTANCE, true);
    Uart_Hal_ReceiveData(DEV_4G_INSTANCE, dev_4g_rx_buffer, DEV_4G_RXBUF_SIZE);
    
    dev_4g_is_open = (UINT8)DEV_4G_STATE_OPENED;

    return (INT32)TBOX_E_OK;
}

INT32 dev_4g_close(VOID)
{
    if((UINT8)DEV_4G_STATE_CLOSED == dev_4g_is_open)
    {
        MODULE_LOG_E(TBOX4G, "4g uart is already closed");
        return (INT32)TBOX_E_HASSTOP;
    }

    Uart_Hal_DeInit(DEV_4G_INSTANCE);

    dev_4g_is_open = (UINT8)DEV_4G_STATE_CLOSED;

    return (INT32)TBOX_E_OK;
}

BOOL dev_4g_is_opened(VOID)
{
    return (dev_4g_is_open == (UINT8)DEV_4G_STATE_OPENED) ? TRUE : FALSE;
}

VOID dev_4g_check_send(VOID)
{
    uint8 *data_ptr = NULL;
    uint16 len = 0;
	
    data_ptr = data_4g_get_send_data(&len);
    if(NULL == data_ptr ||
       0 == len)
    {
        return;
    }

    MODULE_LOG_I(TBOX4G, "send data len:%d, data:%s", len, data_ptr);

    NVIC_DisableIRQ(UART1_IRQn);
    send_callback = data_4g_send_finish_ind;
    NVIC_EnableIRQ(UART1_IRQn);
    Uart_Hal_SendData(DEV_4G_INSTANCE, data_ptr, len);
}

VOID dev_4g_direct_send(UINT8 *data, UINT16 len, DEV_4G_SEND_CALLBACK callback)
{
    MODULE_LOG_DUMP(TBOX4G, "direct send data", data, len);

    NVIC_DisableIRQ(UART1_IRQn);
    send_callback = callback;
    if(NULL == callback)
    {
        send_callback = data_4g_send_finish_ind;
    }
    NVIC_EnableIRQ(UART1_IRQn);
    Uart_Hal_SendData(DEV_4G_INSTANCE, data, len);
}

VOID dev_4g_direct_send_blocking(uint8 *data, uint16 len)
{
#define DEV_4G_SEND_TIMEOUT 500

    MODULE_LOG_DUMP(TBOX4G, "direct send data blocking", data, len);

    Uart_Hal_ReceiveDataBlocking(DEV_4G_INSTANCE, data, len, DEV_4G_SEND_TIMEOUT);

    data_4g_send_finish_ind();
}

