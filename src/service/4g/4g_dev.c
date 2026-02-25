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

static DEV_4G_SEND_CALLBACK send_callback    = NULL_PTR;
static UINT8                dev_4g_is_open   = (UINT8)DEV_4G_STATE_CLOSED;
static volatile BOOL        dev_4g_is_txbusy = FALSE;
static volatile UINT8      *dev_4g_tx_buffer = NULL_PTR;
static volatile UINT16      dev_4g_tx_length = 0U;

void                        UART1_IRQHandler(void)
{
    uint32_t mask;
    uint32_t error;
    uint32_t reg;
    uint8_t  data;

    mask  = 0x9EU;
    error = READ_BIT32(DEV_4G_UART->LSR0, mask);

    if (error) {
        WRITE_REG32(DEV_4G_UART->LSR0, mask);
        MODIFY_REG32(DEV_4G_UART->LCR1, UART_LCR1_RXEN_Msk, UART_LCR1_RXEN_Pos, 0UL);

        mask = 0x79U;
        reg  = READ_REG32(DEV_4G_UART->IER);
        reg &= ~mask;
        WRITE_REG32(DEV_4G_UART->IER, reg);

        Uart_Reg_GetChar(DEV_4G_UART, &data);
    }

    reg = READ_BIT32(DEV_4G_UART->LSR0, UART_LSR0_DR_Msk);
    if (reg) {
        Uart_Reg_GetChar(DEV_4G_UART, &data);
        data_4g_recv_push(data);
    }

    reg = READ_BIT32(DEV_4G_UART->LSR1, UART_LSR1_IDLE_Msk);
    if (reg) {
        WRITE_REG32(DEV_4G_UART->LSR1, UART_LSR1_IDLE_Msk);
    }

    reg = READ_BIT32(DEV_4G_UART->LSR0, UART_LSR0_TXNF_Msk);
    if (reg) {
        if (dev_4g_tx_length > 0U) {
            if (dev_4g_tx_length == 1U) {
                reg = READ_REG32(DEV_4G_UART->IER);
                reg |= UART_IER_ETC_Msk;
                reg &= ~UART_IER_ETXNF_Msk;
                WRITE_REG32(DEV_4G_UART->IER, reg);
            }
            data = *dev_4g_tx_buffer;
            Uart_Reg_PutChar(DEV_4G_UART, data);
            ++dev_4g_tx_buffer;
            --dev_4g_tx_length;
        }
    }

    reg = READ_BIT32(DEV_4G_UART->LSR0, UART_LSR0_TC_Msk);
    if (reg) {
        dev_4g_is_txbusy = FALSE;
        MODIFY_REG32(DEV_4G_UART->IER, UART_IER_ETC_Msk, UART_IER_ETC_Pos, 0UL);
        if (send_callback) {
            send_callback();
        }
    }
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

    // clang-format off
    Core_Hal_DisableIrq(UART1_IRQn);
    WRITE_REG32(DEV_4G_UART->SMP_CNT,  0x00000000U);
    WRITE_REG32(DEV_4G_UART->DIV_L,    0x00000020U);
    WRITE_REG32(DEV_4G_UART->DIV_H,    0x00000000U);
    WRITE_REG32(DEV_4G_UART->DIV_FRAC, 0x00000011U);
    WRITE_REG32(DEV_4G_UART->LCR0,     0x00000001U);
    WRITE_REG32(DEV_4G_UART->FCR,      0x00000001U);
    WRITE_REG32(DEV_4G_UART->LCR1,     0x00000003U);
    WRITE_REG32(DEV_4G_UART->IDLE,     0x00000090U);
    WRITE_REG32(DEV_4G_UART->IER,      0x00000079U);
    Core_Hal_EnableIrq(UART1_IRQn);
    // clang-format on

    dev_4g_is_open   = (UINT8)DEV_4G_STATE_OPENED;
    dev_4g_is_txbusy = FALSE;

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

    dev_4g_is_open   = (UINT8)DEV_4G_STATE_CLOSED;
    dev_4g_is_txbusy = FALSE;

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

    if (dev_4g_is_txbusy) {
        return;
    }

    data_ptr = data_4g_get_send_data(&len);
    if (NULL == data_ptr || 0 == len) {
        return;
    }

    MODULE_LOG_I(TBOX4G, "send data len:%d, data:%s", len, data_ptr);

    NVIC_DisableIRQ(UART1_IRQn);
    send_callback = data_4g_send_finish_ind;
    NVIC_EnableIRQ(UART1_IRQn);

    dev_4g_is_txbusy = TRUE;

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

    dev_4g_is_txbusy = TRUE;

    UART1_Tx(data, len);
}

