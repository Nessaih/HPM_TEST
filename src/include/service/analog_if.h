#ifndef _ANALOG_IF_H_
#define _ANALOG_IF_H_

/* ADC原始值读取接口 */
extern int32_t analog_pwr_vtg(void);
extern int32_t analog_bat_vtg(void);
extern int32_t analog_bat_tmp(void);
extern int32_t analog_gnss_ant(void);
extern int32_t analog_lte_ant(void);
extern int32_t analog_light(void);

/* 天线状态类型定义 */
typedef enum {
    ANT_NORMAL = 0,
    ANT_OPEN,
    ANT_SHORT,
    ANT_UNKNOW
} ant_status_t;

/* 天线状态查询接口 */
extern ant_status_t analog_lte_ant_status(void);
extern ant_status_t analog_gps_ant_status(void);

#endif
