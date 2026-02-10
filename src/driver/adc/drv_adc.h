#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#include <stdint.h>

#define DRV_ADC_LIGHT    0U
#define DRV_ADC_BAT_TMP  1U
#define DRV_ADC_GNSS_2   2U
#define DRV_ADC_PWR_VTG  3U
#define DRV_ADC_BAT_VTG  4U
#define DRV_ADC_MAIN_ANT 5U
#define DRV_ADC_COUNT    6U

extern int32_t drv_adc_init(void);
extern int32_t drv_adc_deinit(void);
extern int32_t drv_adc_sleep(void);
extern int32_t drv_adc_wake(void);
extern int32_t drv_adc_get(uint8_t adc_id);

#endif