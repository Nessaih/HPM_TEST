#include <stdarg.h>
#include <stddef.h>
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "Rcm_Hal.h"
#include "Timer_Hal.h"
#include "drv_timer.h"

#define PCT_INS_MAX  0U

#define PDT_INS_MAX  1U
#define PDT_TICK_MS  1875U
#define PDT_TICK_MAX 34U

#define TMR_INS_MAX  3U

#define TIMER_INIT   0U
#define TIMER_DEINIT 1U
#define TIMER_START  2U
#define TIMER_STOP   3U
#define TIMER_OPS    4U

#define pct_handler  PCT_IRQHandler

typedef int32_t (*timer_ops_t)(uint8_t ins);

static drv_timer_t    *timer_table[DRV_TIMER_INS_COUNT];
static bool            tmr_inited = false;
static const uint8_t   tmr_ins[4] = {0, 1, 2, 3};
static const IRQn_Type pct_irqs[] = PCT_IRQS;
// static const IRQn_Type pdt_irqs[] = PDT_IRQS;
static const IRQn_Type tmr_irqs[] = TIMER_IRQS;

static inline void timer_iterate(uint8_t ins)
{
    drv_tcb_t *curr;

    if (ins >= (uint8_t)DRV_TIMER_INS_COUNT)
        return;

    curr = timer_table[ins]->list;
    while (curr != NULL) {
        if (curr->func != NULL) {
            curr->func();
        }
        curr = curr->next;
    }
}

static int32_t timer_attach(uint8_t ins, void (*callback)(void), drv_tcb_t *list)
{

    drv_tcb_t **curr;

    if (ins >= (uint8_t)DRV_TIMER_INS_COUNT) {
        return -1;
    }

    if (callback == NULL || list == NULL) {
        return -2;
    }

    curr = &(timer_table[ins]->list);
    while (*curr != NULL) {
        if (*curr == list) {
            return -3;
        }
        curr = &((*curr)->next);
    }

    list->func = callback;
    list->next = NULL;
    *curr      = list;

    return 0;
}

static int32_t timer_detach(uint8_t ins, void (*callback)(void))
{
    drv_tcb_t **curr;

    if (ins >= DRV_TIMER_INS_COUNT) {
        return -1;
    }

    if (callback == NULL) {
        return -2;
    }

    curr = &(timer_table[ins]->list);

    while (*curr) {
        if ((*curr)->func == callback) {
            *curr = (*curr)->next;
            return 0;
        }
        curr = &(*curr)->next;
    }
    return -3;
}

void pct_handler(void)
{
    if (PCT->CSR & PCT_CSR_CF_Msk) {
        PCT->CSR |= PCT_CSR_CF_Msk; 
        timer_iterate(DRV_TIMER_INS_PCT);
    }
}






static void tmr_handler(void *args)
{
    uint8_t ins = *(uint8_t *)args;
    timer_iterate(DRV_TIMER_INS_TIMER0 + ins);
}

static int32_t pct_init(uint8_t ins)
{
    Hal_StatusType status;
    IRQn_Type      irqn;
    uint32_t       mode;
    uint32_t       freq, div, ms, n;
    uint32_t       prescale_value;
    uint32_t       prescale_disable;
    uint32_t       interrupt_enable;
    uint32_t       timer_enable;

    ms   = timer_table[ins]->period;
    mode = timer_table[ins]->mode;

    ins -= DRV_TIMER_INS_PCT;
    if (ins > (uint8_t)PCT_INS_MAX) {
        return -1;
    }

    status = Ckgen_Hal_EnablePeriphClk(CKGEN_PCT_BUS_CLK, TRUE);
    if (STATUS_SUCCESS != status) {
        return (int32_t)status;
    }

    status = Ckgen_Hal_SetPeriphClkMux(CKGEN_PCT_CLK, CKGEN_SPLL_DIV2_CLK);
    if (STATUS_SUCCESS != status) {
        return (int32_t)status;
    }

    
    Rcm_Hal_SetResetState(RCM_RESET_ID_PCT, RCM_RESET_STATE_DEASSERT);

    status = Ckgen_Hal_GetFreq(CKGEN_PCT_CLK, (uint32 *)&freq);
    if (STATUS_SUCCESS != status) {
        return (int32_t)status;
    }

    
    status = STATUS_ERROR;
    n      = 0xFFFF;
    for (prescale_value = 0; prescale_value < 16U; ++prescale_value) {
        div = 1UL << (prescale_value + 1U);
        
        n = freq / 1000U / div;
        n = n * ms;
        if (1UL <= n && n <= 65535U) {
            status = STATUS_SUCCESS;
            break;
        }
    }

    if (prescale_value != 0U) {
        prescale_disable = 0U;
    } else {
        prescale_disable = 1U;
    }

    if (mode & DRV_TIMER_MODE_INTERRUPT)
        interrupt_enable = 1U;
    else
        interrupt_enable = 0U;

    if (mode & DRV_TIMER_MODE_STARTUP)
        timer_enable = 1U;
    else
        timer_enable = 0U;

    if (STATUS_SUCCESS != status) {
        return (int32_t)status;
    }

    irqn = pct_irqs[ins];
    NVIC_DisableIRQ(irqn);

    
    MODIFY_REG32(PCT->CSR, PCT_CSR_PCT_EN_Msk, PCT_CSR_PCT_EN_Pos, 0);              
    WRITE_REG32(PCT->PSR, 0);                                                       
    WRITE_REG32(PCT->CMR, 0);                                                       
    WRITE_REG32(PCT->CSR, 0);                                                       

    MODIFY_REG32(PCT->PSR, PCT_PSR_BYP_Msk, PCT_PSR_BYP_Pos, prescale_disable);     
    MODIFY_REG32(PCT->PSR, PCT_PSR_PS_VAL_Msk, PCT_PSR_PS_VAL_Pos, prescale_value); 
    MODIFY_REG32(PCT->PSR, PCT_PSR_CVAL_SEL_Msk, PCT_PSR_CVAL_SEL_Pos, 0);          
    MODIFY_REG32(PCT->PSR, PCT_PSR_PCS_Msk, PCT_PSR_PCS_Pos, 3);                    

    WRITE_REG32(PCT->CMR, n);                                                       

    MODIFY_REG32(PCT->CSR, PCT_CSR_CF_Msk, PCT_CSR_CF_Pos, 1);                      
    MODIFY_REG32(PCT->CSR, PCT_CSR_CIE_Msk, PCT_CSR_CIE_Pos, interrupt_enable);     
    MODIFY_REG32(PCT->CSR, PCT_CSR_FRE_Msk, PCT_CSR_FRE_Pos, 0);                    
    MODIFY_REG32(PCT->CSR, PCT_CSR_MS_Msk, PCT_CSR_MS_Pos, 0);                      
    MODIFY_REG32(PCT->CSR, PCT_CSR_PCT_EN_Msk, PCT_CSR_PCT_EN_Pos, timer_enable);   
    

    
    

    if (interrupt_enable) {
        NVIC_ClearPendingIRQ(irqn);
        NVIC_EnableIRQ(irqn);
    }

    return 0;
}

static int32_t pct_deinit(uint8_t ins)
{

    ins -= DRV_TIMER_INS_PCT;
    if (ins > PCT_INS_MAX)
        return -1;

    MODIFY_REG32(PCT->CSR, PCT_CSR_PCT_EN_Msk, PCT_CSR_PCT_EN_Pos, 0); 
    WRITE_REG32(PCT->PSR, 0);                                          
    WRITE_REG32(PCT->CMR, 0);                                          
    WRITE_REG32(PCT->CSR, 0);                                          

    Rcm_Hal_SetResetState(RCM_RESET_ID_PCT, RCM_RESET_STATE_ASSERT);
    Ckgen_Hal_EnablePeriphClk(CKGEN_PCT_BUS_CLK, FALSE);
    NVIC_DisableIRQ(PCT_IRQn);
    NVIC_ClearPendingIRQ(PCT_IRQn);

    return 0;
}

static int32_t pct_start(uint8_t ins)
{
    ins -= DRV_TIMER_INS_PCT;
    if (ins > PCT_INS_MAX)
        return -1;

    MODIFY_REG32(PCT->CSR, PCT_CSR_PCT_EN_Msk, PCT_CSR_PCT_EN_Pos, 1); 

    return 0;
}

static int32_t pct_stop(uint8_t ins)
{
    ins -= DRV_TIMER_INS_PCT;
    if (ins > PCT_INS_MAX)
        return -1;

    MODIFY_REG32(PCT->CSR, PCT_CSR_PCT_EN_Msk, PCT_CSR_PCT_EN_Pos, 0); 

    return 0;
}

static int32_t pdt_init(uint8_t ins)
{
#if 0
    bool               start;
    uint32_t           period;
    uint32_t           mode;
    pdt_timer_config_t cfg = {
        .loadValueMode        = PDT_LOAD_VAL_IMMEDIATELY,
        .clkPreDiv            = PDT_CLK_PREDIV_BY_64,
        .clkPreMultFactor     = PDT_CLK_PREMULT_FACT_AS_1,
        .triggerInput         = PDT_SOFTWARE_TRIGGER,
        .continuousModeEnable = true,
        .callback             = pdt_handler,
    };

    period = timer_table[ins]->period;
    mode   = timer_table[ins]->mode;

    ins -= DRV_TIMER_INS_PDT0;
    if (ins > PDT_INS_MAX)
        return -1;

    if (period > PDT_TICK_MAX)
        return -2;

    if (mode & DRV_TIMER_MODE_INTERRUPT)
        cfg.intEnable = true;
    else
        cfg.intEnable = false;

    if (mode & DRV_TIMER_MODE_STARTUP)
        start = true;
    else
        start = false;

    period = (uint32_t)(PDT_TICK_MS * period) - 1;

    PDT_DRV_Init(ins, &cfg);
    PDT_DRV_SetValueForTimerInterrupt(ins, 0);
    PDT_DRV_SetTimerModulusValue(ins, period);
    PDT_DRV_LoadValuesCmd(ins);

    if (start) {
        PDT_DRV_Enable(ins);
        PDT_DRV_SoftTriggerCmd(ins);
    }
#endif
    return 0;
}

static int32_t pdt_deinit(uint8_t ins)
{
#if 0
    ins -= DRV_TIMER_INS_PDT0;
    if (ins > PDT_INS_MAX)
        return -1;
    PDT_DRV_Deinit(ins);
#endif
    return 0;
}

static int32_t pdt_start(uint8_t ins)
{
#if 0
    ins -= DRV_TIMER_INS_PDT0;
    if (ins > PDT_INS_MAX)
        return -1;
    NVIC_EnableIRQ(pdt_irqs[ins]);
    PDT_DRV_Enable(ins);
#endif
    return 0;
}

static int32_t pdt_stop(uint8_t ins)
{
#if 0
    ins -= DRV_TIMER_INS_PDT0;
    if (ins > PDT_INS_MAX)
        return -1;
    NVIC_DisableIRQ(pdt_irqs[ins]);
    PDT_DRV_Disable(ins);
#endif
    return 0;
}

static int32_t tmr_init(uint8_t ins)
{

    bool                     start;
    uint32_t                 period;
    uint32_t                 mode;
    Hal_StatusType           status;
    IRQn_Type                irqn;
    Timer_Channel_ConfigType cfg = {
        .Mode       = TIMER_MODE_0,
        .Config     = TIMER_IRQ_EN,
        .TriggerSrc = TRIG_SOURCE_TIMER_CH0,
    };

    period = timer_table[ins]->period;
    mode   = timer_table[ins]->mode;

    ins -= DRV_TIMER_INS_TIMER0;
    if (ins > TMR_INS_MAX)
        return (int32_t)STATUS_ERROR;

    if (mode & DRV_TIMER_MODE_INTERRUPT) {
        cfg.Config |= TIMER_IRQ_EN;
    } else {
        cfg.Config &= ~TIMER_IRQ_EN;
    }

    if (mode & DRV_TIMER_MODE_STARTUP) {
        start = true;
    } else {
        start = false;
    }

    if (!tmr_inited) {
        Timer_Hal_Init(TIMER_CLOCK_SPLL);
        tmr_inited = true;
    }

    status = Timer_Hal_SetConfig(ins, &cfg);
    if (STATUS_SUCCESS != status) {
        return (int32_t)status;
    }
    Timer_Hal_InstallCallback(ins, tmr_handler, (void *)&tmr_ins[ins]);
    if (start) {
        period = Timer_Hal_MicrosToTicks(period * 1000U);
        status = Timer_Hal_Start(ins, period);
        irqn   = tmr_irqs[ins];
        NVIC_ClearPendingIRQ(irqn);
        NVIC_EnableIRQ(irqn);
    }
    return (int32_t)status;
}

static int32_t tmr_deinit(uint8_t ins)
{
    Hal_StatusType status;

    ins -= DRV_TIMER_INS_TIMER0;
    if (ins > TMR_INS_MAX)
        return (int32_t)STATUS_ERROR;
    status = Timer_Hal_Stop(ins);
    Core_Hal_DisableIrq(tmr_irqs[ins]);

    return (int32_t)status;
}

static int32_t tmr_start(uint8_t ins)
{
    Hal_StatusType status;
    uint32_t       period;

    if (ins >= DRV_TIMER_INS_COUNT) {
        return (int32_t)STATUS_ERROR;
    }
    period = timer_table[ins]->period;
    ins -= DRV_TIMER_INS_TIMER0;
    if (ins > (uint8_t)TMR_INS_MAX) {
        return (int32_t)STATUS_ERROR;
    }

    period = Timer_Hal_MicrosToTicks(period * 1000U);
    status = Timer_Hal_Start(ins, period);
    Core_Hal_EnableIrq(tmr_irqs[ins]);

    return (int32_t)status;
}

static int32_t tmr_stop(uint8_t ins)
{
    Hal_StatusType status;

    ins -= DRV_TIMER_INS_TIMER0;
    if (ins > (uint8_t)TMR_INS_MAX) {
        return (int32_t)STATUS_ERROR;
    }

    status = Timer_Hal_Stop(ins);
    Core_Hal_DisableIrq(tmr_irqs[ins]);

    return (int32_t)status;
}

static timer_ops_t timer_ops[DRV_TIMER_INS_COUNT][TIMER_OPS] = {
    {pct_init, pct_deinit, pct_start, pct_stop},
    {pdt_init, pdt_deinit, pdt_start, pdt_stop},
    {pdt_init, pdt_deinit, pdt_start, pdt_stop},
    {tmr_init, tmr_deinit, tmr_start, tmr_stop},
    {tmr_init, tmr_deinit, tmr_start, tmr_stop},
    {tmr_init, tmr_deinit, tmr_start, tmr_stop},
    {tmr_init, tmr_deinit, tmr_start, tmr_stop},
};

static int32_t timer_start(drv_timer_t *timer)
{
    int32_t ret;

    
    ret = timer_ops[timer->ins][TIMER_START](timer->ins);
    return ret;
}

static int32_t timer_stop(drv_timer_t *timer)
{
    int32_t ret;

    
    ret = timer_ops[timer->ins][TIMER_STOP](timer->ins);
    return ret;
}

static int32_t timer_shield(drv_timer_t *timer, bool shield)
{
    IRQn_Type irq;

    

    switch (timer->ins) {
    case DRV_TIMER_INS_PCT:
        irq = PCT_IRQn;
        break;
    case DRV_TIMER_INS_PDT0:
        irq = PDT0_IRQn;
        break;
    case DRV_TIMER_INS_PDT1:
        irq = PDT1_IRQn;
        break;
    case DRV_TIMER_INS_TIMER0:
        irq = TIMER_CHANNEL0_IRQn;
        break;
    case DRV_TIMER_INS_TIMER1:
        irq = TIMER_CHANNEL1_IRQn;
        break;
    case DRV_TIMER_INS_TIMER2:
        irq = TIMER_CHANNEL2_IRQn;
        break;
    case DRV_TIMER_INS_TIMER3:
        irq = TIMER_CHANNEL3_IRQn;
        break;
    default:
        irq = NonMaskableInt_IRQn;
        break;
    }

    if (irq == NonMaskableInt_IRQn) {
        return -1;
    }

    if (shield) {
        NVIC_DisableIRQ(irq);
    } else {
        NVIC_EnableIRQ(irq);
    }

    return 0;
}

int32_t drv_timer_init(drv_timer_t *timer)
{
    int32_t ret;

    if (timer == NULL)
        return -1;
    ret = timer_ops[timer->ins][TIMER_INIT](timer->ins);
    return ret;
}

int32_t drv_timer_deinit(drv_timer_t *timer)
{
    int32_t ret;

    if (timer == NULL)
        return -1;
    ret = timer_ops[timer->ins][TIMER_INIT](timer->ins);

    timer->ins    = 0xFFU;
    timer->period = 0xFFFFFFFFU;
    return ret;
}

int32_t drv_timer_control(drv_timer_t *timer, ...)
{
    int32_t ret = 0;
    int32_t cmd;
    va_list args;

    if (timer == NULL)
        return -1;

    va_start(args, timer);
    cmd = va_arg(args, int);
    switch (cmd) {

    case DRV_TIMER_CMD_SET_CFG: {
        timer->ins    = va_arg(args, uint32_t);
        timer->period = va_arg(args, uint32_t);
        timer->mode   = va_arg(args, uint32_t);
        timer->list   = NULL;

        timer_table[timer->ins] = timer;

        break;
    }

    case DRV_TIMER_CMD_SET_INS: {
        timer->ins = va_arg(args, uint32_t);

        timer_table[timer->ins] = timer;
        break;
    }

    case DRV_TIMER_CMD_SET_MODE: {
        uint8_t mask = va_arg(args, uint32_t);
        timer->mode |= mask;
        break;
    }

    case DRV_TIMER_CMD_CLS_MODE: {
        uint8_t mask = va_arg(args, uint32_t);
        timer->mode &= ~mask;
        break;
    }

    case DRV_TIMER_CMD_SET_PERIOD: {
        timer->period = va_arg(args, uint32_t);
        break;
    }

    case DRV_TIMER_CMD_START: {
        ret = timer_start(timer);
        break;
    }

    case DRV_TIMER_CMD_STOP: {
        ret = timer_stop(timer);
        break;
    }

    case DRV_TIMER_CMD_ATTACH: {
        drv_timer_callback_t func;
        drv_tcb_t           *list;

        func = (drv_timer_callback_t)va_arg(args, void *);
        list = (drv_tcb_t *)va_arg(args, void *);
        ret  = timer_attach((uint8_t)timer->ins, func, list);
        break;
    }

    case DRV_TIMER_CMD_DETACH: {
        drv_timer_callback_t func;

        func = (drv_timer_callback_t)va_arg(args, void *);
        ret  = timer_detach((uint8_t)timer->ins, func);
        break;
    }

    case DRV_TIMER_CMD_SHIELD: {
        int32_t enable = va_arg(args, int);
        if (enable) {
            ret = timer_shield(timer, true);
        } else {
            ret = timer_shield(timer, false);
        }
        break;
    }

    default:
        ret = -1;
        break;
    }
    va_end(args);
    return ret;
}