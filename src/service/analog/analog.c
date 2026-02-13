
#include <math.h>
#include "tbox_core.h"
#include "tbox_shell_if.h"
#include "drv_adc.h"
#include "analog_if.h"
#include "tbox_log.h"

typedef struct
{
    int   temperature;
    float standardval;
    float coefficient;
    float offset;
} bat_curve_t;

static const bat_curve_t bat_curve[7] = {
    {-40, 3916.0f, -16.05f, 3274.0f},
    {-20, 3595.0f, -29.25f, 3010.0f},
    {0,   3010.0f, -37.8f,  3010.0f},
    {40,  1498.0f, -20.25f, 2303.0f},
    {60,  1093.0f, -25.3f,  2611.0f},
    {80,  587.0f,  -11.55f, 1511.0f},
    {100, 356.0f,  -6.9f,   1046.0f},
};

int32_t analog_pwr_vtg(void)
{
    const float k = 8.5414789598145f;
    const float b = 977.574513f;
    float       vtg;

    vtg = (float)drv_adc_get(DRV_ADC_PWR_VTG);
    vtg = k * vtg + b;
    return (int32_t)vtg;
}

int32_t analog_bat_vtg(void)
{
    const float k = 0.8056640625f;
    float       vtg;
    float       result;

    vtg = (float)drv_adc_get(DRV_ADC_BAT_VTG);
    result = k * vtg;
    result = result * 2.0f;
    vtg = result;
    return (int32_t)vtg;
}

int32_t analog_bat_tmp(void)
{
    float   tmp;
    float   vtg;
    uint8_t i;

    vtg = (float)drv_adc_get(DRV_ADC_BAT_TMP);
    for (i = 0U; i < 7U; i++) {
        if (vtg <= bat_curve[6U - i].standardval) {
            break;
        }
    }
    if (i < 7U) {
        tmp = (vtg - bat_curve[6U - i].offset) / bat_curve[6U - i].coefficient;
    } else {
        if (vtg < 4020.0f) {
            tmp = -40.0f;
        } else {
            tmp = 255.0f;
        }
    }
    return (int32_t)tmp;
}

int32_t analog_gnss_ant(void)
{
    return drv_adc_get(DRV_ADC_GNSS_2);
}

int32_t analog_lte_ant(void)
{
    return drv_adc_get(DRV_ADC_MAIN_ANT);
}

int32_t analog_light(void)
{
    return drv_adc_get(DRV_ADC_LIGHT);
}

ant_status_t analog_gps_ant_status(void)
{
    int32_t open_gnss;
    int32_t adc_value;
    uint32_t adc_gnss;

    /* 读取GNSS天线开路检测GPIO引脚，1表示开路，0表示正常 */
    open_gnss = drv_pin_get_level(PIN_STATE_GNSS_ANT);

    if (open_gnss > 0) {
        return ANT_OPEN;
    } else {
        adc_value = analog_gnss_ant();
        if (adc_value < 0) {
            return ANT_UNKNOW;
        }
        
        adc_gnss = (uint32_t)adc_value * 3300U / 4096U;
        
        if (adc_gnss < 2500U) {
            return ANT_SHORT;
        } else {
            return ANT_NORMAL;
        }
    }
}

ant_status_t analog_lte_ant_status(void)
{
    const uint32_t ANT_LTE_SHORT_CIRCUIT_MAX = (0.3 * 1000);
    const uint32_t ANT_LTE_OPEN_CIRCUIT_MIN  = (1.4 * 1000);

    int32_t adc_value;
    uint32_t adc_lte;

    adc_value = analog_lte_ant();
    if (adc_value < 0) {
        return ANT_UNKNOW;
    }

    adc_lte = (uint32_t)adc_value * 3300U / 4096U;

    if (adc_lte < ANT_LTE_SHORT_CIRCUIT_MAX) {
        return ANT_SHORT;
    } else if (adc_lte > ANT_LTE_OPEN_CIRCUIT_MIN) {
        return ANT_OPEN;
    } else {
        return ANT_NORMAL;
    }
}

static BaseType_t analog_show_adc_cmd(char *buf, size_t bufsz, const char *cmd)
{
    int len = 0;
    int raw;
    float voltage_mv;
    const char *ant_status_str[] = {"Normal", "Open", "Short", "Unknow"};
    (void)cmd;
    
    len += snprintf(buf + len, bufsz - len, "\r\n=== ADC Channel Values ===\r\n");
    
    /* PWR_VTG */
    raw = drv_adc_get(DRV_ADC_PWR_VTG);
    voltage_mv = (float)raw * 3300.0f / 4095.0f;
    len += snprintf(buf + len, bufsz - len, "PWR_VTG   : Raw=%4d, Voltage=%4.0f mV, Power=%d mV\r\n", 
                    raw, (double)voltage_mv, analog_pwr_vtg());

    /* LTE_ANT */
    raw = drv_adc_get(DRV_ADC_MAIN_ANT);
    voltage_mv = (float)raw * 3300.0f / 4095.0f;
    len += snprintf(buf + len, bufsz - len, "LTE_ANT   : Raw=%4d, Voltage=%4.0f mV, Status=%s\r\n", 
                    raw, (double)voltage_mv, ant_status_str[analog_lte_ant_status()]);

    /* GNSS_ANT */
    raw = drv_adc_get(DRV_ADC_GNSS_2);
    voltage_mv = (float)raw * 3300.0f / 4095.0f;
    len += snprintf(buf + len, bufsz - len, "GNSS_ANT  : Raw=%4d, Voltage=%4.0f mV, Status=%s\r\n", 
                    raw, (double)voltage_mv, ant_status_str[analog_gps_ant_status()]);
    
    /* BAT_VTG */
    raw = drv_adc_get(DRV_ADC_BAT_VTG);
    voltage_mv = (float)raw * 3300.0f / 4095.0f;
    len += snprintf(buf + len, bufsz - len, "BAT_VTG   : Raw=%4d, Voltage=%4.0f mV, Battery=%d mV\r\n", 
                    raw, (double)voltage_mv, analog_bat_vtg());

    /* BAT_TMP */
    raw = drv_adc_get(DRV_ADC_BAT_TMP);
    voltage_mv = (float)raw * 3300.0f / 4095.0f;
    len += snprintf(buf + len, bufsz - len, "BAT_TMP   : Raw=%4d, Voltage=%4.0f mV, Temp=%d C\r\n", 
                    raw, (double)voltage_mv, analog_bat_tmp());
    
    /* LIGHT */
    raw = drv_adc_get(DRV_ADC_LIGHT);
    voltage_mv = (float)raw * 3300.0f / 4095.0f;
    len += snprintf(buf + len, bufsz - len, "LIGHT     : Raw=%4d, Voltage=%4.0f mV\r\n", 
                    raw, (double)voltage_mv);

    len += snprintf(buf + len, bufsz - len, "==========================\r\n");
    
    return pdFALSE;
}

static BaseType_t analog_show_ant_status_cmd(char *buf, size_t bufsz, const char *cmd)
{
    const char *ant_status_str[] = {"Normal", "Open", "Short", "Unknow"};
    int len = 0;
    (void)cmd;
    
    len += snprintf(buf + len, bufsz - len, "\r\n=== Antenna Status ===\r\n");

    len += snprintf(buf + len, bufsz - len, "LTE_ANT   : Status=%s\r\n", ant_status_str[analog_lte_ant_status()]);
    len += snprintf(buf + len, bufsz - len, "GNSS_ANT  : Status=%s\r\n", ant_status_str[analog_gps_ant_status()]);
    
    len += snprintf(buf + len, bufsz - len, "==========================\r\n");
    
    return pdFALSE;
}

TBOX_SHELL_DEFINE(showadc, "show all ADC channel values", 0U, analog_show_adc_cmd);
TBOX_SHELL_DEFINE(showant, "show all antenna status", 0U, analog_show_ant_status_cmd);

static INT32 analog_init(UINT8 seq)
{
    if (seq == MODULE_INIT_SEQ_MODULE) {
        MODULE_LOG_D(ANALOG, "Analog module initialized");
        TBOX_SHELL_REGISTER(showadc);
        TBOX_SHELL_REGISTER(showant);
    }
    return TBOX_E_OK;
}

TBOX_MODULE_FUN(ANALOG, analog_init, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR);
TBOX_MODULE(ANALOG, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_INFO, 0, TRUE, FALSE);
TBOX_MODULE_LOADER(ANALOG) {}
