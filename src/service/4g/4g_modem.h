#ifndef TBOX_4G_MODEM_H
#define TBOX_4G_MODEM_H

typedef enum
{
    MODEM_4G_FUN_UNKNOWN = 0,
    MODEM_4G_FUN_NORMAL,
    MODEM_4G_FUN_AIRMODE
}MODEM_4G_FUN_MODE;

typedef enum
{
    MODEM_4G_SLEEP_SW = 0x00,
    MODEM_4G_SLEEP_DTR = 0x02
}MODEM_4G_SLEEP_MODE;

typedef struct
{
    boolean imei_is_valid;
    uint8 modem_4g_imei[MODEM_4G_IMEI_MAX];

    boolean mtid_is_valid;
    uint8 modem_4g_mtid[MODEM_4G_MTID_LEN];

    boolean taid_is_valid;
    uint8 modem_4g_taid[MODEM_4G_TAID_LEN];

    uint16 testlive_fail_count;
    uint8 modem_4g_temp; /*4G温度*/
    uint8 act;
    uint8 funmode;
}MODEM_4G_INFO;

extern MODEM_4G_INFO modem_4g_info;

void  modem_4g_init(void);

void  modem_4g_reinit(void);

void  modem_4g_clear_alivefaile_count(void);

uint16 modem_4g_get_alivefaile_count(void); 

uint8 modem_4g_test_alive(AT_4G_CMD_PRIORITY pri, uint8 retry_count);

uint8 modem_4g_read_baud(AT_4G_CMD_PRIORITY pri, uint8 retry_count);

uint8 modem_4g_set_baud(AT_4G_CMD_PRIORITY pri, uint32 baud);

uint8 modem_4g_echomode(AT_4G_CMD_PRIORITY pri, uint8 mode);

uint8 modem_4g_enable_errnum(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_set_airmode(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_set_normalmode(AT_4G_CMD_PRIORITY pri);

void modem_4g_query_funmode(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_query_imei(AT_4G_CMD_PRIORITY pri);

uint32 modem_4g_get_baud_value(void);

void modem_4g_set_baud_value(uint32 baud);

uint8 modem_4g_get_funmode_value(void);

void modem_4g_set_funmode_value(uint8 mode);

uint8 modem_4g_config_urc_port(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_urc_other_off(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_urc_ring(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_urc_smsincoming(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_query_temp(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_query_mtid(AT_4G_CMD_PRIORITY pri);

uint8 modem_4g_query_taid(AT_4G_CMD_PRIORITY pri);

#endif /* TBOX_4G_MODEM_H */
