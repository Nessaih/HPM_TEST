#include "tbox_common.h"
#include "tbox_core.h"
#include "4g_content.h"
#include "4g_data.h"
#include "4g_dev.h"
#include "4g_mgr.h"
#include "AC784xx_Uart_Reg.h"
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "Device_Register.h"
#include "Rcm_Hal.h"


#define DEV_4G_STATE_CLOSED 0U
#define DEV_4G_STATE_OPENED 1U

static DEV_4G_SEND_CALLBACK send_callback  = NULL_PTR;
static UINT8                dev_4g_is_open = (UINT8)DEV_4G_STATE_CLOSED;
static UINT8                dev_4g_rx_buffer[DEV_4G_RXBUF_SIZE];
static UINT8               *dev_4g_tx_buffer = NULL_PTR;
static UINT16               dev_4g_rx_length = 0U;
static UINT16               dev_4g_tx_length = 0U;
/*
static VOID                 dev_4g_rx_callback(UINT8 instance, Uart_EventType event)
{
    UINT32         len;
    Hal_StatusType status;

    NVIC_DisableIRQ(UART1_IRQn);
    switch (event) {
    case UART_EVENT_END_TRANSFER:
        len = DEV_4G_RXBUF_SIZE;
        break;

    case UART_EVENT_IDLE_LINE:
        status = Uart_Hal_GetReceiveStatus(DEV_4G_INSTANCE, &len);
        len    = DEV_4G_RXBUF_SIZE - len;
        if (STATUS_BUSY == status) {
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

    if (len > 0U && len <= DEV_4G_RXBUF_SIZE) {
        // dev_4g_rx_buffer[len] = '\0';

        for (UINT16 i = 0; i < len; i++) {
            data_4g_recv_push(dev_4g_rx_buffer[i]);
        }
    }
    Uart_Hal_ReceiveData(DEV_4G_INSTANCE, dev_4g_rx_buffer, DEV_4G_RXBUF_SIZE);
    NVIC_EnableIRQ(UART1_IRQn);
}

static VOID dev_4g_tx_callback(UINT8 instance, Uart_EventType event)
{
    UNUSED(instance);

    if (instance != DEV_4G_INSTANCE) {
        return;
    }

    if (UART_EVENT_END_TRANSFER != event) {
        return;
    }

    NVIC_DisableIRQ(UART1_IRQn);
    if (NULL_PTR != send_callback) {
        send_callback();
    }
    NVIC_EnableIRQ(UART1_IRQn);
}
*/

void UART1_ErrHandler(void)
{
    uint32_t mask;
    uint32_t error;

    mask  = UART_LSR0_OE_Msk | UART_LSR0_PE_Msk | UART_LSR0_FE_Msk | UART_LSR0_BI_Msk | UART_LSR0_NE_Msk;
    error = READ_BIT32(DEV_4G_UART->LSR0, mask);
    if (error) {
        uint8_t TmpByte = 0U;
        WRITE_REG32(DEV_4G_UART->LSR0, mask);
        Uart_Reg_SetReceiverCmd(DEV_4G_UART, FALSE);
        Uart_Reg_SetErrorInterrupts(DEV_4G_UART, FALSE);
        Uart_Reg_GetChar(DEV_4G_UART, &TmpByte);
        Uart_Reg_SetIntMode(DEV_4G_UART, UART_INT_RX_NOT_EMPTY, FALSE);
    }
}

void UART1_RxHandler(void)
{
    if (Uart_Reg_GetStatusFlag(DEV_4G_UART, UART_RX_DATA_READY)) {
        uint8 RxBuff = 0U;
        Uart_Reg_GetChar(DEV_4G_UART, &RxBuff);
        data_4g_recv_push(RxBuff);
    }

    if (TRUE == Uart_Reg_GetStatusFlag(DEV_4G_UART, UART_IDLE_LINE)) {
        Uart_Reg_ClearStatusFlag(DEV_4G_UART, UART_IDLE_LINE);
    }
}

void UART1_TxHandler(void)
{
    if (Uart_Reg_GetStatusFlag(DEV_4G_UART, UART_TX_DATA_NOT_FULL)) {
        if (dev_4g_tx_length > 0U) {

            if (dev_4g_tx_length == 1U) {
                Uart_Reg_SetIntMode(DEV_4G_UART, UART_INT_TX_COMPLETE, TRUE);
                Uart_Reg_SetIntMode(DEV_4G_UART, UART_INT_TX_NOT_FULL, FALSE);
            }

            Uart_Reg_PutChar(DEV_4G_UART, *dev_4g_tx_buffer);
            ++dev_4g_tx_buffer;
            --dev_4g_tx_length;
        }
    }

    if (Uart_Reg_GetStatusFlag(DEV_4G_UART, UART_TX_COMPLETE)) {
        Uart_Reg_SetIntMode(DEV_4G_UART, UART_INT_TX_COMPLETE, FALSE);
        if (NULL_PTR != send_callback) {
            send_callback();
        }
    }
}

void UART1_IRQHandler(void)
{
    UART1_ErrHandler();
    UART1_RxHandler();
    UART1_TxHandler();
}

void UART1_Tx(uint8 *data, uint16 len)
{
    dev_4g_tx_buffer = data;
    dev_4g_tx_length = len;

    Uart_Reg_SetIntMode(DEV_4G_UART, UART_INT_TX_NOT_FULL, TRUE);

    if (!Uart_Reg_GetIntMode(DEV_4G_UART, UART_INT_RX_NOT_EMPTY)) {
        uint8_t TmpByte = 0U;
        Uart_Reg_GetChar(DEV_4G_UART, &TmpByte);
        Uart_Reg_SetReceiverCmd(DEV_4G_UART, TRUE);
        Uart_Reg_SetErrorInterrupts(DEV_4G_UART, TRUE);
        Uart_Reg_SetIntMode(DEV_4G_UART, UART_INT_RX_NOT_EMPTY, TRUE);
    }
}

INT32 dev_4g_open(VOID)
{
    if ((UINT8)DEV_4G_STATE_OPENED == dev_4g_is_open) {
        MODULE_LOG_W(TBOX4G, "4g uart is already opened");
        return (INT32)TBOX_E_HASSTART;
    }

    Ckgen_Hal_EnablePeriphClk(CKGEN_UART1_BUS_CLK, TRUE);
    Rcm_Hal_SetResetState(RCM_RESET_ID_UART1, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_UART1, RCM_RESET_STATE_DEASSERT);

    Core_Hal_DisableIrq(UART1_IRQn);
    WRITE_REG32(DEV_4G_UART->SMP_CNT, 0x00000000U);
    WRITE_REG32(DEV_4G_UART->DIV_L, 0x00000020U);
    WRITE_REG32(DEV_4G_UART->DIV_H, 0x00000000U);
    WRITE_REG32(DEV_4G_UART->DIV_FRAC, 0x00000011U);
    WRITE_REG32(DEV_4G_UART->LCR0, 0x00000001U);
    WRITE_REG32(DEV_4G_UART->FCR, 0x00000001U);
    WRITE_REG32(DEV_4G_UART->LCR1, 0x00000003U);
    WRITE_REG32(DEV_4G_UART->IDLE, 0x00000090U);
    WRITE_REG32(DEV_4G_UART->IER, 0x00000079U);
    Core_Hal_EnableIrq(UART1_IRQn);

    dev_4g_is_open = (UINT8)DEV_4G_STATE_OPENED;

    return (INT32)TBOX_E_OK;
}

INT32 dev_4g_close(VOID)
{
    if ((UINT8)DEV_4G_STATE_CLOSED == dev_4g_is_open) {
        MODULE_LOG_E(TBOX4G, "4g uart is already closed");
        return (INT32)TBOX_E_HASSTOP;
    }

    Core_Hal_DisableIrq(UART1_IRQn);
    Rcm_Hal_SetResetState(RCM_RESET_ID_UART1, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_UART1, RCM_RESET_STATE_DEASSERT);
    Ckgen_Hal_EnablePeriphClk(CKGEN_UART1_BUS_CLK, FALSE);
    Core_Hal_ClearPendingIrq(UART1_IRQn);

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
    uint16 len      = 0;

    data_ptr = data_4g_get_send_data(&len);
    if (NULL == data_ptr || 0 == len) {
        return;
    }

    MODULE_LOG_I(TBOX4G, "send data len:%d, data:%s", len, data_ptr);

    NVIC_DisableIRQ(UART1_IRQn);
    send_callback = data_4g_send_finish_ind;
    NVIC_EnableIRQ(UART1_IRQn);

    UART1_Tx(data_ptr, len);
}

VOID dev_4g_direct_send(UINT8 *data, UINT16 len, DEV_4G_SEND_CALLBACK callback)
{
    MODULE_LOG_DUMP(TBOX4G, "direct send data", data, len);

    NVIC_DisableIRQ(UART1_IRQn);
    send_callback = callback;
    if (NULL == callback) {
        send_callback = data_4g_send_finish_ind;
    }
    NVIC_EnableIRQ(UART1_IRQn);

    UART1_Tx(data, len);
}

// VOID dev_4g_direct_send_blocking(uint8 *data, uint16 len)
// {
// #define DEV_4G_SEND_TIMEOUT 500

//     MODULE_LOG_DUMP(TBOX4G, "direct send data blocking", data, len);

//     Uart_Hal_ReceiveDataBlocking(DEV_4G_INSTANCE, data, len, DEV_4G_SEND_TIMEOUT);

//     data_4g_send_finish_ind();
// }
