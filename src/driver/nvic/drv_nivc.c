

#include <stdint.h>
#include "Core_Hal.h"
#include "Device_Register.h"
#include "macros.h"

#pragma pack(1)
typedef struct
{
    int8_t  irq;
    uint8_t prio;
} drv_nvic_prio_t;
#pragma pack()

static const drv_nvic_prio_t priorities[] = {
    {UART0_IRQn,          14},
    {UART1_IRQn,          7 },
    {UART2_IRQn,          7 },
    {UART3_IRQn,          7 },
    {SPI0_IRQn,           2 },
    {I2C0_IRQn,           4 },
    {DMA0_CHANNEL0_IRQn,  2 },
    {DMA0_CHANNEL1_IRQn,  2 },
    {PCT_IRQn,            5 },
    {PDT0_IRQn,           5 },
    {PDT1_IRQn,           5 },
    {TIMER_CHANNEL0_IRQn, 5 },
    {TIMER_CHANNEL1_IRQn, 5 },
    {TIMER_CHANNEL2_IRQn, 5 },
    {TIMER_CHANNEL3_IRQn, 5 },
    {CAN0_IRQn,           4 },
    {CAN1_IRQn,           4 },
    {CAN2_IRQn,           4 },
    {CAN0_WAKEUP_IRQn,    7 },
    {CAN1_WAKEUP_IRQn,    7 },
    {CAN2_WAKEUP_IRQn,    7 },
    {ADC0_IRQn,           14},
    {ADC1_IRQn,           14},
    {PORTA_IRQn,          13},
    {PORTB_IRQn,          13},
    {PORTC_IRQn,          13},
    {PORTD_IRQn,          13},
    {PORTE_IRQn,          13},
    {EIO_IRQn,            7 },
};

int32_t drv_nvic_init(void)
{
    drv_nvic_prio_t *s, *e;

    NVIC_SetPriorityGrouping(3);

    s = (drv_nvic_prio_t *)priorities;
    e = s + ARRAY_SIZE(priorities);
    while (s < e)
    {
        NVIC_SetPriority((IRQn_Type)(s->irq), s->prio);
        ++s;
    }
    return 0;
}

int32_t drv_nvic_deinit(void)
{
    drv_nvic_prio_t *s, *e;

    NVIC_SetPriorityGrouping(0);

    s = (drv_nvic_prio_t *)priorities;
    e = s + ARRAY_SIZE(priorities);

    while (s < e)
    {
        NVIC_DisableIRQ((IRQn_Type)(s->irq));
        ++s;
    }
    return 0;
}
