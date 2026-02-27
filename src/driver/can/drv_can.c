
#include <string.h>
#include "api_rtos.h"
#include "can_config.h"
#include "can_hal.h"
#include "drv_can.h"
#include "drv_log.h"
#include "macros.h"
#include "AC784xx_Can_Reg.h"

// clang-format off
#define DRV_CAN_EXTEND_ID_MASK                              0x80000000U
#define DRV_CAN_RUN_CALLBACK(ins, event, para)              do {                                                                                                                                                                                               \
                                                                if (can_cbs[(ins)]) {                                                                                                                                                                          \
                                                                    can_cbs[(ins)]((ins), (event), (void *)para);                                                                                                                                              \
                                                                }                                                                                                                                                                                              \
                                                            } while (0)
// clang-format on

typedef struct
{
    volatile uint8_t  is_init   : 1;
    volatile uint8_t  is_busoff : 1;//暂不使用，此状态统一由服务层处理
    volatile uint8_t  tx_buzy   : 1;
    volatile uint8_t  rx_buzy   : 1;
    volatile uint32_t rx_tick;
} drv_can_state_t;

static drv_can_state_t  can_state[DRV_CAN_INS_COUNT];
static drv_can_reg_cb_t can_cbs[DRV_CAN_INS_COUNT];

int32_t drv_can_send(const can_msg_t *msg)
{
    Hal_StatusType      status;
    Can_MessageInfoType info = {0};

    if ((uint8_t)msg->ins >= (uint8_t)DRV_CAN_INS_COUNT) {
        return -1;
    }

    if (!can_state[msg->ins].is_init) {
        return -2;
    }

    // if (can_state[msg->ins].is_busoff) {
    //     return -3;
    // }

    if (CAN_STATE_RUNNING != Can_Hal_GetControllerState(msg->ins)) {
        return -4;
    }

    bool is_extend_id = (0U != (DRV_CAN_EXTEND_ID_MASK & msg->id));

    info.MsgFdf     = FALSE;
    info.MsgRtr     = FALSE;
    info.MsgBrs     = FALSE;
    info.MsgId      = msg->id & (~DRV_CAN_EXTEND_ID_MASK);
    info.MsgIdType  = is_extend_id ? CAN_EXTENDED_MSG : CAN_STANDARD_MSG;
    info.MsgDataLen = msg->len;
    info.MsgDataPtr = (uint8_t *)msg->data;
#if defined(AC7843X)
    info.EfcAnmf = FALSE;
    info.MmFidx  = 0;
#endif

    status = Can_Hal_WriteTxBuffer(msg->ins, (uint8_t)CAN_TX_BUF_SECONDARY, &info);
    if (STATUS_SUCCESS != status)
        return -3;

    can_state[msg->ins].tx_buzy = TRUE;

    return 0;
}

static void drv_can_handle_receive(uint8_t ins)
{

    Hal_StatusType      status;
    can_msg_t           msg  = {0};
    Can_MessageInfoType info = {0};

    info.MsgDataPtr = msg.data;
    status          = Can_Hal_ReadRxBuffer(ins, 0, &info);

    if (STATUS_SUCCESS != status) {
        DRV_LOG_E(DRVCAN, "CAN%d receive failed:%d\n", ins, status);
        return;
    }

    can_state[ins].is_busoff = FALSE;

    if (info.MsgIdType == CAN_EXTENDED_MSG) {
        msg.id = info.MsgId | DRV_CAN_EXTEND_ID_MASK;
    } else {
        msg.id = info.MsgId;
    }

    can_state[ins].rx_tick = xTaskGetTickCount();

    if (info.MsgRtr) {
        msg.ins = ins;
        msg.len = 0U;
        DRV_CAN_RUN_CALLBACK(ins, DRV_CAN_EVENT_RX_DONE, NULL);
    } else {
        msg.ins = ins;
        msg.len = info.MsgDataLen;
        DRV_CAN_RUN_CALLBACK(ins, DRV_CAN_EVENT_RX_DONE, &msg);
    }
}

static void drv_can_handle_send(uint8_t ins)
{
    can_state[ins].tx_buzy   = FALSE;
    can_state[ins].is_busoff = FALSE;
    DRV_CAN_RUN_CALLBACK(ins, DRV_CAN_EVENT_TX_DONE, NULL);
}

static void drv_can_handle_buserr(uint8_t ins)
{
    DRV_CAN_RUN_CALLBACK(ins, DRV_CAN_EVENT_BUS_ERROR, NULL);
}

static void drv_can_handle_busoff(uint8_t ins)
{

    Can_DevErrorStateType error_state = Can_Hal_GetErrorState(ins);

    switch (error_state) {
    case CAN_ERR_STATE_ACTIVE:
        can_state[ins].is_busoff = FALSE;
        DRV_CAN_RUN_CALLBACK(ins, DRV_CAN_EVENT_BUS_OFF, 0U);
        break;
    case CAN_ERR_STATE_BUSOFF:
        can_state[ins].is_busoff = TRUE;
        can_state[ins].tx_buzy   = FALSE;
        DRV_CAN_RUN_CALLBACK(ins, DRV_CAN_EVENT_BUS_OFF, 1U);
        break;

    default:
        break;
    }
}





static void drv_can_callback(void *args)
{
    Can_IrqCBParams *param = (Can_IrqCBParams *)args;
    uint32_t         flags = param->CommonIrqFlagMasks;
    uint8_t          ins   = param->Instance;

    
    if (flags & CAN_CTRL1_RIF_Msk) {
        drv_can_handle_receive(ins);
    }

    
    if (flags & CAN_CTRL1_TPIF_Msk) {
        
    }

    
    if (flags & CAN_CTRL1_TSIF_Msk) {
        drv_can_handle_send(ins);
    }


    if (flags & CAN_CTRL1_EIF_Msk) {
        drv_can_handle_busoff(ins);
    }

    
    if (flags & CAN_CTRL1_BEIF_Msk) {
        drv_can_handle_buserr(ins);
    }

    
    if (flags & CAN_CTRL1_EPIF_Msk) {
        
    }

    
    if (flags & CAN_CTRL1_ALIF_Msk) {
        
    }
}

bool drv_can_busy(uint8_t ins)
{
    uint32_t tick[2];

    if ((uint8_t)ins >= (uint8_t)DRV_CAN_INS_COUNT) {
        return false;
    }

    if (!can_state[ins].is_init) {
        return false;
    }

    tick[0] = can_state[ins].rx_tick;
    tick[1] = xTaskGetTickCount();

    if ((0 == tick[0]) || IS_TIMEOUT(tick[1], tick[0], 10000)) {
        return false;
    }

    return true;
}

bool    drv_can_has_init(uint8_t ins)
{
    if ((uint8_t)ins >= (uint8_t)DRV_CAN_INS_COUNT) 
    {
        return false;
    }

    return (TRUE == can_state[ins].is_init) ? true : false;
}

bool    drv_can_is_bus_off(uint8_t ins)
{
    if ((uint8_t)ins >= (uint8_t)DRV_CAN_INS_COUNT) 
    {
        return false;
    }

    return (TRUE == can_state[ins].is_busoff) ? true : false;    
}

bool    drv_can_is_sending(uint8_t ins)
{
    if ((uint8_t)ins >= (uint8_t)DRV_CAN_INS_COUNT) 
    {
        return false;
    }

    return (TRUE == can_state[ins].tx_buzy) ? true : false; 
}

int32_t drv_can_register(uint8_t ins, drv_can_reg_cb_t cb)
{
    if ((uint8_t)ins >= (uint8_t)DRV_CAN_INS_COUNT) {
        return -1;
    }

    can_cbs[ins] = cb;
    return 0;
}

int32_t drv_can_init(uint8_t ins, uint32_t rate, uint8_t mode)
{
    Can_HalConfigType  cfg;
    uint8_t            can_rate_index;
    Can_ExtendModeType extend_mode;

    if (ins >= DRV_CAN_INS_COUNT) {
        return -1;
    }

    if(TRUE == can_state[ins].is_init)
    {
        drv_can_deinit(ins);
    }

    if (250U == rate) {
        can_rate_index = DRV_CAN_RATE_250K;
    } else if (500U == rate) {
        can_rate_index = DRV_CAN_RATE_500K;
    } else if (1000U == rate) {
        can_rate_index = DRV_CAN_RATE_1000K;
    } else {
        return -2;
    }

    if (mode > DRV_CAN_MODE_LOOPBACK) {
        return -3;
    }

    extend_mode = can_mode_list[mode];

    memcpy(&cfg, &can_init_config, sizeof(cfg));
    cfg.BaudrateConfigPtr = (Can_BaudrateConfigType *)&can_rate_list[can_rate_index];
    cfg.IrqCallback       = drv_can_callback;

    Can_Hal_Init(ins, &cfg);
    Can_Hal_ConfigExtendMode(ins, extend_mode);
    Can_Hal_SetControllerState(ins, CAN_STATE_RUNNING);

    can_state[ins].is_init   = TRUE;
    can_state[ins].is_busoff = FALSE;
    can_state[ins].tx_buzy   = FALSE;
    can_state[ins].rx_buzy   = FALSE;
    can_state[ins].rx_tick   = xTaskGetTickCount();

    return 0;
}

int32_t drv_can_deinit(uint8_t ins)
{
    if (ins >= DRV_CAN_INS_COUNT) {
        return -1;
    }

    Can_Hal_Deinit(ins);
    can_state[ins].is_init = FALSE;
    return 0;
}

int32_t drv_can_enable_error_report(uint8_t ins, bool enable)
{
    CAN_Type *base;
    uint32_t irq_en_mask;

    if (ins >= DRV_CAN_INS_COUNT) {
        DRV_LOG_E(DRVCAN, "CAN%d enable error report failed: invalid instance", ins);
        return -1;
    }

    if (!can_state[ins].is_init) {
        DRV_LOG_E(DRVCAN, "CAN%d enable error report failed: not initialized", ins);
        return -2;
    }

    base = Can_Hal_GetBase(ins);
    if (NULL == base) {
        DRV_LOG_E(DRVCAN, "CAN%d enable error report failed: get base failed", ins);
        return -3;
    }

    irq_en_mask = base->CTRL1 & CAN_COMMON_IRQ_EN_MASKS;
    
    if (enable) {
        irq_en_mask |= CAN_CTRL1_BEIE_Msk;
        DRV_LOG_I(DRVCAN, "CAN%d bus error interrupt enabled", ins);
    } else {
        irq_en_mask &= ~CAN_CTRL1_BEIE_Msk;
        DRV_LOG_I(DRVCAN, "CAN%d bus error interrupt disabled (busoff still enabled)", ins);
    }

    Can_Reg_SetIntEnable(base, irq_en_mask);

    return 0;
}
