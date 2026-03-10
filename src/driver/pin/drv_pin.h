#ifndef __DRV_PIN_H__
#define __DRV_PIN_H__

#include <stdint.h>

// ====================================== GPIO OUTPUT PIN DEFINITIONS START ============================================= //
#define PIN_POWER_5V0         (17U) // CAN 5V
#define PIN_POWER_3V3         (41U)
#define PIN_POWER_1V8         (26U)
#define PIN_POWER_BT          (69U)
#define PIN_POWER_MPU         (42U)
#define PIN_POWER_485         (7U)

#define PIN_RESET_VSE         (52U)
#define PIN_RESET_MPU         (50U)
#define PIN_RESET_GNSS        (22U)
#define PIN_RESET_BT          (80U)

#define PIN_PWRKEY_MPU        (49U)
#define PIN_MPU_SLEEP         (58U)

#define PIN_BAT_CHARGE        (34U)
#define PIN_BAT_SUPPLY        (36U) // EN_VEB

#define PIN_LED_TBOX_RUN      (99U)
#define PIN_LED_TBOX_CAN      (92U)
#define PIN_LED_TBOX_LTE      (5U)
#define PIN_LED_TBOX_GNSS     (6U)

#define PIN_FLS_NAND_HOLD     (25U)
#define PIN_FLS_NAND_WP       (27U)
#define PIN_FLS_NAND_CS       (82U)

#define PIN_FLS_NOR_HOLD      (67U)
#define PIN_FLS_NOR_WP        (68U)
#define PIN_FLS_NOR_CS        (63U)

#define PIN_485_DIR           (95U)

#define PIN_ENABLE_CAN        (18U)
#define PIN_ENABLE_WIFI       (75U)
#define PIN_ENABLE_BT         (62U)
#define PIN_ENABLE_DOL        (35U) // 0:Hz, 1:L
#define PIN_ENABLE_DOH        (94U) // 0:Hz, 1:H

#define PIN_OD_PU_CT          (33U)
// ====================================== GPIO OUTPUT PIN DEFINITIONS END ============================================= //

// ====================================== GPIO INPUT PIN DEFINITIONS START ============================================= //
#define PIN_WAKE_ACC          (19U)
#define PIN_WAKE_RTC          (1U)
#define PIN_WAKE_ANT          (20U)
#define PIN_WAKE_RING         (83U)
#define PIN_WAKE_IMU          (59U)
#define PIN_WAKE_PWD          (93U)
#define PIN_WAKE_LIGHT        (77U)
#define PIN_WAKE_WIFI         (74U)

#define PIN_MCU_DIL           (28U)
#define PIN_STATE_NETWORK     (21U)

#define PIN_STATE_GNSS_ANT    (46U)
#define PIN_STATE_MPU_START   (51U) // 0: mpu is started, 1: mpu is not started

// ====================================== GPIO INPUT PIN DEFINITIONS END ============================================= //

// ====================================== PORT CONFIGURATION DEFINITIONS START  ====================================== //

#define PIN_PULL_DISABLE      0U
#define PIN_PULL_UP           1U
#define PIN_PULL_DOWN         2U

#define PIN_DRIVER_LOW        0U
#define PIN_DRIVER_HIGH       1U

#define PIN_MUX_DISABLE       0U
#define PIN_MUX_GPIO          1U
#define PIN_MUX_OPTION0       0U
#define PIN_MUX_OPTION1       1U
#define PIN_MUX_OPTION2       2U
#define PIN_MUX_OPTION3       3U
#define PIN_MUX_OPTION4       4U
#define PIN_MUX_OPTION5       5U
#define PIN_MUX_OPTION6       6U
#define PIN_MUX_OPTION7       7U

#define PIN_LOCK_DISABLE      0U
#define PIN_LOCK_ENABLE       1U

#define PIN_INT_DISABLE       0U
#define PIN_INT_DMA_RISING    1U
#define PIN_INT_DMA_FALLING   2U
#define PIN_INT_DMA_BOTH_EDGE 3U
#define PIN_INT_RISING        9U
#define PIN_INT_FALLING       10U
#define PIN_INT_BOTH_EDGE     11U

#define PIN_ISFCLR_DISABLE    0U
#define PIN_ISFCLR_ENABLE     1U

#define PIN_FILTER_DISABLE    0U
#define PIN_FILTER_ENABLE     1U

#define PIN_SLEEP_DISABLE     0U
#define PIN_SLEEP_ENABLE      1U

// ====================================== PORT CONFIGURATION DEFINITIONS END  ====================================== //

// ====================================== GPIO CONFIGURATION DEFINITIONS START  ====================================== //
#define GPIO_MODE_INPUT       0U
#define GPIO_MODE_OUTPUT      1U
#define GPIO_MODE_HIGH_Z      2U

#define GPIO_LEVEL_LOW        0U
#define GPIO_LEVEL_HIGH       1U
#define GPIO_LEVEL_NONE       2U
// ====================================== GPIO CONFIGURATION DEFINITIONS END  ====================================== //


typedef void (*drv_pin_cb_t)(uint32_t context);

typedef struct
{
    uint8_t pin;
    uint8_t pull   : 2;
    uint8_t mux    : 3;
    uint8_t edge   : 4;
    uint8_t sleep  : 1;
    uint8_t clear  : 1;
    uint8_t lock   : 1;
    uint8_t drive  : 1;
    uint8_t filter : 1;
} pin_port_cfg_t;

typedef struct
{
    uint8_t pin;
    uint8_t mode  : 2;
    uint8_t level : 2;
} pin_gpio_cfg_t;

/********************   Pin interface functions definitions  ********************/
extern int32_t drv_pin_port_config(const pin_port_cfg_t *cfg);
extern int32_t drv_pin_gpio_config(const pin_gpio_cfg_t *cfg);
extern int32_t drv_pin_set_level(uint8_t pin, uint8_t level);
extern int32_t drv_pin_get_level(uint8_t pin);
extern int32_t drv_pin_toggle(uint8_t pin);
extern int32_t drv_pin_set_interrupt(uint8_t pin, uint8_t mode, drv_pin_cb_t callback, uint32_t context);
extern int32_t drv_pin_rst_interrupt(uint8_t pin);
extern int32_t drv_pin_clear_interrupt(void);
extern int32_t drv_pin_init(void);
extern int32_t drv_pin_wake(void);
extern int32_t drv_pin_sleep(void);

#endif //__DRV_PIN_H__