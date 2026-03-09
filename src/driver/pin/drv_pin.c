/*
 *******************************************************************************
 * @file        drv_pin.c
 * @path        src\driver\pin\drv_pin.c
 * @brief       1.默认功能是正确的不再重复配置；（如RESET、晶振输入、调式接口等）
 * @            2.没使用的悬空脚不配置；
 * @            3.没使用的默认High-Z的脚不配置（非悬空脚，可能连接外部）
 * @            4.没使用的非悬空脚中，且默认非High-Z，需要初始化为GPIO，并配置为GPIO HIGH-Z模式。
 * @
 * @peripheral（partial）
 *               ┌──────────────────────┐ ┌───────────────────────────────────────────┐
 *               │    SLAVE SPI 0       │ │                ADC                        │
 *               ├──────────────────────┤ ├───────────────────────────────────────────┤
 *               │  SPI0_CS2      PIN84 │ │  AD_VEB       PIN39        ADC0_IN9       │
 *               │  SPI0_SCK      PIN94 │ │  AD_NTC       PIN40        ADC0_IN8       │
 *               │  SPI0_MOSI     PIN85 │ │  ADC_GPS2     PIN45        ADC0_IN12      │
 *               │  SPI0_MISO     PIN93 │ │  OPEN_GPS     PIN46        GPIO           │
 *               │                      │ │  AD_V12       PIN71        ADC1_IN2       │
 *               └──────────────────────┘ └───────────────────────────────────────────┘
 * @record
 * @Change Logs:
 * Date             Author          Notes
 * 2025-02-06       vic             First version
 * 2025-12-08       vic             Hardware Model: "HPM7820022 V1.00 2025.11.14"
 *******************************************************************************
 */

#include <stddef.h>
#include <stdint.h>
#include "Core_Hal.h"
#include "Device_Register.h"
#include "Gpio_Hal.h"
#include "drv_pin.h"
#include "macros.h"

#define PIN_INDEX_MIN 1U
#define PIN_INDEX_MAX 100U

#define PORT_COUNT    5U
#define PINS_COUNT    18U

#define PIN_DEFAULT   PIN_ISFCLR_ENABLE, PIN_LOCK_DISABLE, PIN_DRIVER_LOW, PIN_FILTER_DISABLE

typedef struct
{
    IRQn_Type    irq;
    uint32_t     pins;
    uint32_t     cts[PINS_COUNT];
    drv_pin_cb_t cbs[PINS_COUNT];
} drv_pin_rec_t;

typedef struct
{
    uint8_t port;
    uint8_t pin;
} pin_port_t;

static const uint8_t pin_to_port[] = {
    /*      1     2     3     4     5     6     7     8     9     10 */
    /* 0 */ 0x90, 0x8F, 0x61, 0x60, 0x8B, 0x8A, 0x8D, 0x85, 0x84, 0xFF,
    /* 1 */ 0xFF, 0xFF, 0xFF, 0xFF, 0x27, 0x26, 0x8E, 0x83, 0x8C, 0x71,
    /* 2 */ 0x70, 0x6F, 0x89, 0x6E, 0x6D, 0x88, 0x25, 0x24, 0x43, 0x42,
    /* 3 */ 0x67, 0x66, 0x65, 0x6C, 0x6B, 0x6A, 0xFF, 0xFF, 0x41, 0x40,
    /* 4 */ 0x69, 0x68, 0x51, 0x50, 0x4F, 0x4E, 0x23, 0x22, 0x4D, 0x4C,
    /* 5 */ 0x4B, 0x4A, 0x21, 0x20, 0x49, 0x48, 0x07, 0x06, 0x87, 0xFF,
    /* 6 */ 0xFF, 0x11, 0x31, 0x30, 0x2F, 0x2E, 0x2D, 0x2C, 0x64, 0x63,
    /* 7 */ 0x62, 0x03, 0x02, 0x2B, 0x2A, 0x29, 0x28, 0x01, 0x00, 0x47,
    /* 8 */ 0x46, 0x10, 0x0F, 0x86, 0x82, 0xFF, 0xFF, 0x0E, 0x0D, 0x0C,
    /* 9 */ 0x0B, 0x0A, 0x81, 0x80, 0x45, 0x44, 0x05, 0x04, 0x09, 0x08,
};

static drv_pin_rec_t    int_rec[PORT_COUNT];
static PORT_Type *const pin_port_ptrs[] = GPIO_PORT_BASE_PTRS;
static GPIO_Type *const pin_gpio_ptrs[] = GPIO_BASE_PTRS;
static IRQn_Type const  pin_port_irqs[] = PORT_IRQS;

static const pin_port_cfg_t pin_port_cfg[] = {
    {1U,  PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {2U,  PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {3U,  PIN_PULL_UP,      PIN_MUX_OPTION6, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {4U,  PIN_PULL_UP,      PIN_MUX_OPTION6, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {5U,  PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {6U,  PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {7U,  PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {8U,  PIN_PULL_DISABLE, PIN_MUX_OPTION5, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {9U,  PIN_PULL_DISABLE, PIN_MUX_OPTION5, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {17U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {18U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {19U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {20U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {21U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {22U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {25U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {26U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {27U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {28U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {29U, PIN_PULL_DISABLE, PIN_MUX_OPTION4, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {30U, PIN_PULL_DISABLE, PIN_MUX_OPTION4, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {31U, PIN_PULL_DISABLE, PIN_MUX_OPTION2, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {32U, PIN_PULL_DISABLE, PIN_MUX_OPTION2, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {34U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {35U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {36U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {39U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {40U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {41U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {42U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {43U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {44U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {45U, PIN_PULL_DISABLE, PIN_MUX_DISABLE, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {46U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {47U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {48U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {49U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {50U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {51U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {52U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {53U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {54U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {55U, PIN_PULL_DISABLE, PIN_MUX_OPTION2, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {56U, PIN_PULL_DISABLE, PIN_MUX_OPTION2, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {57U, PIN_PULL_DISABLE, PIN_MUX_OPTION6, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {58U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {59U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {62U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {63U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {64U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {65U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {66U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {67U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {68U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {69U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {70U, PIN_PULL_DISABLE, PIN_MUX_DISABLE, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {71U, PIN_PULL_DISABLE, PIN_MUX_DISABLE, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {72U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {73U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {74U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {75U, PIN_PULL_DISABLE, PIN_MUX_OPTION5, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {76U, PIN_PULL_DISABLE, PIN_MUX_OPTION5, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {77U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {78U, PIN_PULL_DISABLE, PIN_MUX_DISABLE, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {79U, PIN_PULL_DISABLE, PIN_MUX_DISABLE, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {80U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {81U, PIN_PULL_DISABLE, PIN_MUX_DISABLE, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {82U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {83U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {84U, PIN_PULL_DISABLE, PIN_MUX_OPTION6, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {85U, PIN_PULL_DISABLE, PIN_MUX_OPTION6, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {89U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {90U, PIN_PULL_DISABLE, PIN_MUX_OPTION3, PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {92U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {93U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_DISABLE, PIN_DEFAULT},
    {94U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {95U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
    {99U, PIN_PULL_DISABLE, PIN_MUX_GPIO,    PIN_INT_DISABLE, PIN_SLEEP_ENABLE,  PIN_DEFAULT},
};

static const pin_gpio_cfg_t pin_gpio_cfg[] = {
    {PIN_POWER_5V0,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_POWER_3V3,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_POWER_1V8,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_POWER_BT,        GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_POWER_MPU,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_POWER_485,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_RESET_VSE,       GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_RESET_MPU,       GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_RESET_GNSS,      GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_RESET_BT,        GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_PWRKEY_MPU,      GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_MPU_SLEEP,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_BAT_CHARGE,      GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_BAT_SUPPLY,      GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_LED_TBOX_RUN,    GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_LED_TBOX_CAN,    GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_LED_TBOX_LTE,    GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_LED_TBOX_GNSS,   GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_FLS_NAND_HOLD,   GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_FLS_NAND_WP,     GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_FLS_NOR_HOLD,    GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_FLS_NOR_WP,      GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_485_DIR,         GPIO_MODE_OUTPUT, GPIO_LEVEL_LOW },
    {PIN_ENABLE_CAN,      GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_ENABLE_WIFI,     GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_ENABLE_BT,       GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_ENABLE_DOL,      GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_ENABLE_DOH,      GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},
    {PIN_OD_PU_CT,        GPIO_MODE_OUTPUT, GPIO_LEVEL_HIGH},

    {PIN_WAKE_ACC,        GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_RTC,        GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_ANT,        GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_RING,       GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_IMU,        GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_PWD,        GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_LIGHT,      GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_WAKE_WIFI,       GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_MCU_DIL,         GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_STATE_NETWORK,   GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_STATE_GNSS_ANT,  GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
    {PIN_STATE_MPU_START, GPIO_MODE_INPUT,  GPIO_LEVEL_NONE},
};

static int32_t drv_pin_get_port(uint8_t pin, pin_port_t *base)
{
    uint8_t v;
    if (pin < PIN_INDEX_MIN || pin > PIN_INDEX_MAX)
    {
        return -1;
    }
    if (NULL == base)
    {
        return -2;
    }
    v = pin_to_port[pin - 1U];
    if (0xFFU == v)
    {
        return -3;
    }
    base->port = (v >> 5) & 0x7U;
    base->pin  = (v & 0x1FU);
    return 0;
}

int32_t drv_pin_port_config(const pin_port_cfg_t *cfg)
{
    pin_port_t base;
    int32_t    ret;
    uint32_t   reg;

    ret = drv_pin_get_port(cfg->pin, &base);
    if (0 != ret)
    {
        return ret;
    }

    reg = pin_port_ptrs[base.port]->PCR[base.pin];
    MODIFY_REG32(reg, PORT_PCR_PU_Msk | PORT_PCR_PD_Msk, PORT_PCR_PU_Pos, cfg->pull); // set pull
    MODIFY_REG32(reg, PORT_PCR_DSE_Msk, PORT_PCR_DSE_Pos, cfg->drive);                // set drive
    MODIFY_REG32(reg, PORT_PCR_MUX_Msk, PORT_PCR_MUX_Pos, cfg->mux);                  // set mux
    MODIFY_REG32(reg, PORT_PCR_LK_Msk, PORT_PCR_LK_Pos, cfg->lock);                   // set lock
    MODIFY_REG32(reg, PORT_PCR_IRQC_Msk, PORT_PCR_IRQC_Pos, cfg->edge);               // set edge
    MODIFY_REG32(reg, PORT_PCR_ISF_Msk, PORT_PCR_ISF_Pos, cfg->clear);
    pin_port_ptrs[base.port]->PCR[base.pin] = reg;

    return 0;
}

int32_t drv_pin_gpio_config(const pin_gpio_cfg_t *cfg)
{
    pin_port_t base;
    int32_t    ret;
    uint32_t   reg;

    ret = drv_pin_get_port(cfg->pin, &base);
    if (0 != ret)
    {
        return ret;
    }

    reg = 1UL << base.pin;
    switch (cfg->mode)
    {
    case GPIO_MODE_INPUT:
        pin_gpio_ptrs[base.port]->POER &= ~reg;
        pin_gpio_ptrs[base.port]->PIER |= reg;
        break;

    case GPIO_MODE_OUTPUT:
        pin_gpio_ptrs[base.port]->POER |= reg;
        pin_gpio_ptrs[base.port]->PIER &= ~reg;
        break;

    case GPIO_MODE_HIGH_Z:
        pin_gpio_ptrs[base.port]->POER &= ~reg;
        pin_gpio_ptrs[base.port]->PIER &= ~reg;
        break;

    default:
        break;
    }

    if (GPIO_LEVEL_LOW == cfg->level)
    {
        pin_gpio_ptrs[base.port]->PROR = reg;
    }
    else
    {
        pin_gpio_ptrs[base.port]->PSOR = reg;
    }

    return 0;
}

static int32_t drv_pin_set_mode(uint8_t pin, uint8_t gpio_mode)
{

    pin_port_t base;
    int32_t    ret;
    uint32_t   reg;

    ret = drv_pin_get_port(pin, &base);
    if (0 != ret)
    {
        return ret;
    }

    reg = 1UL << base.pin;
    switch (gpio_mode)
    {
    case GPIO_MODE_INPUT:
        pin_gpio_ptrs[base.port]->POER &= ~reg;
        pin_gpio_ptrs[base.port]->PIER |= reg;
        break;

    case GPIO_MODE_OUTPUT:
        pin_gpio_ptrs[base.port]->POER |= reg;
        pin_gpio_ptrs[base.port]->PIER &= ~reg;
        break;

    case GPIO_MODE_HIGH_Z:
        pin_gpio_ptrs[base.port]->POER &= ~reg;
        pin_gpio_ptrs[base.port]->PIER &= ~reg;
        break;

    default:
        break;
    }

    return 0;
}

int32_t drv_pin_set_level(uint8_t pin, uint8_t level)
{
    pin_port_t base;
    int32_t    ret;
    uint32_t   reg;

    ret = drv_pin_get_port(pin, &base);
    if (0 != ret)
    {
        return ret;
    }

    reg = 1UL << base.pin;

    if (level != 0U)
    {
        pin_gpio_ptrs[base.port]->PSOR = reg;
    }
    else
    {
        pin_gpio_ptrs[base.port]->PROR = reg;
    }

    return 0;
}

int32_t drv_pin_get_level(uint8_t pin)
{
    pin_port_t base;
    int32_t    ret;
    uint32_t   reg;

    ret = drv_pin_get_port(pin, &base);
    if (0 != ret)
    {
        return ret;
    }
    reg = 1UL << base.pin;

    if ((pin_gpio_ptrs[base.port]->POER & reg) != 0U)
    {
        reg &= pin_gpio_ptrs[base.port]->PODR;
    }
    else
    {
        reg &= pin_gpio_ptrs[base.port]->PIDR;
    }

    return (reg != 0U) ? 1 : 0;
}

int32_t drv_pin_toggle(uint8_t pin)
{
    pin_port_t base;
    int32_t    ret;
    uint32_t   reg;

    ret = drv_pin_get_port(pin, &base);
    if (0 != ret)
    {
        return ret;
    }
    reg = 1UL << base.pin;

    pin_gpio_ptrs[base.port]->PIOR = reg;

    return 0;
}

int32_t drv_pin_set_interrupt(uint8_t pin, uint8_t mode, drv_pin_cb_t callback, uint32_t context)
{
    pin_port_t     base = {0};
    pin_port_cfg_t cfg  = {0};
    uint32_t       ret;

    ret = (uint32_t)drv_pin_get_port(pin, &base);
    if (0UL != ret)
    {
        return (int32_t)ret;
    }

    cfg.pin   = (uint8_t)pin;
    cfg.mux   = PIN_MUX_GPIO;
    cfg.edge  = (uint16_t)mode;
    cfg.clear = PIN_ISFCLR_ENABLE;
    NVIC_DisableIRQ(int_rec[base.port].irq);
    ret |= (uint32_t)drv_pin_port_config(&cfg);
    ret |= (uint32_t)drv_pin_set_mode(pin, (int32_t)GPIO_MODE_INPUT);
    int_rec[base.port].pins |= 1UL << base.pin;
    int_rec[base.port].cbs[base.pin] = callback;
    int_rec[base.port].cts[base.pin] = context;
    NVIC_EnableIRQ(int_rec[base.port].irq);

    return (int32_t)ret;
}

int32_t drv_pin_rst_interrupt(uint8_t pin)
{
    pin_port_t     base = {0};
    pin_port_cfg_t cfg  = {0};
    uint32_t       ret;

    ret = (uint32_t)drv_pin_get_port(pin, &base);
    if (0UL != ret)
    {
        return (int32_t)ret;
    }

    NVIC_DisableIRQ(int_rec[base.port].irq);
    cfg.pin   = (uint8_t)pin;
    cfg.mux   = PIN_MUX_DISABLE;
    cfg.edge  = PIN_INT_DISABLE;
    cfg.clear = PIN_ISFCLR_ENABLE;
    ret |= (uint32_t)drv_pin_port_config(&cfg);
    ret |= (uint32_t)drv_pin_set_mode(pin, (int32_t)GPIO_MODE_HIGH_Z);
    int_rec[base.port].pins &= ~(1UL << base.pin);
    int_rec[base.port].cbs[base.pin] = NULL;
    int_rec[base.port].cts[base.pin] = 0;
    NVIC_EnableIRQ(int_rec[base.port].irq);

    return (int32_t)ret;
}

int32_t drv_pin_clear_interrupt(void)
{
    uint8_t  i, j;
    uint32_t mask;

    for (i = 0; i < PORT_COUNT; i++)
    {
        for (j = 0; j < PINS_COUNT; j++)
        {
            mask = 1UL << j;
            if (int_rec[i].pins & mask)
            {
                NVIC_DisableIRQ(int_rec[i].irq);
                Gpio_Hal_SetPinIntSel(i, j, (Port_Hal_InterruptConfigType)PIN_INT_DISABLE);
                int_rec[i].pins &= ~mask;
                int_rec[i].cbs[j] = NULL;
                int_rec[i].cts[j] = 0;
                NVIC_EnableIRQ(int_rec[i].irq);
            }
        }
    }
    return 0;
}

static void drv_pin_int_handler(uint8_t instance, uint32_t status)
{
    drv_pin_rec_t *rec;
    uint32_t       i;

    if (0U == status)
    {
        return;
    }

    rec = &int_rec[instance];
    status &= rec->pins;
    for (i = 0; i < PINS_COUNT; i++)
    {
        if (((status >> i) & 1U) && rec->cbs[i])
            rec->cbs[i](rec->cts[i]);
    }
}

int32_t drv_pin_init(void)
{
    uint32_t ret = 0;
    uint8_t  i;

    for (i = 0; i < ARRAY_SIZE(pin_port_cfg); i++)
    {
        ret |= (uint32_t)drv_pin_port_config(&pin_port_cfg[i]);
    }

    for (i = 0; i < ARRAY_SIZE(pin_gpio_cfg); i++)
    {
        ret |= (uint32_t)drv_pin_gpio_config(&pin_gpio_cfg[i]);
    }

    for (i = 0; i < PORT_COUNT; i++)
    {
        int_rec[i].irq = pin_port_irqs[i];
        Gpio_Hal_InstallCallback(i, drv_pin_int_handler);
    }
    return (int32_t)ret;
}

int32_t drv_pin_wake(void)
{
    uint16_t i;
    uint32_t ret = 0;

    for (i = 0; i < ARRAY_SIZE(pin_port_cfg); i++)
    {
        if (pin_port_cfg[i].sleep)
        {
            ret |= (uint32_t)drv_pin_port_config(&pin_port_cfg[i]);
        }
    }

    for (i = 0; i < ARRAY_SIZE(pin_gpio_cfg); i++)
    {
        drv_pin_set_mode(pin_gpio_cfg[i].pin, pin_gpio_cfg[i].mode);
    }
    return (int32_t)ret;
}

int32_t drv_pin_sleep(void)
{
    pin_port_t base;
    uint8_t    i;
    int32_t    ret;
    int32_t    result = 0;

    for (i = 0; i < ARRAY_SIZE(pin_port_cfg); i++)
    {
        if (pin_port_cfg[i].sleep)
        {
            ret = drv_pin_get_port(pin_port_cfg[i].pin, &base);
            if (0 != ret)
            {
                result = -1;
                continue;
            }
            Gpio_Hal_SetMuxMode(base.port, base.pin, (Port_Hal_MuxType)PIN_MUX_DISABLE);
            Gpio_Hal_SetHighZ(base.port, base.pin, TRUE);
        }
    }
    return result;
}