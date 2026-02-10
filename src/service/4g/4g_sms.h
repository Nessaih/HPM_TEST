#ifndef TBOX_4G_SMS_H
#define TBOX_4G_SMS_H

typedef enum
{
    SMS_4G_MODE_UNKNOWN,
    SMS_4G_MODE_TEXT,
    SMS_4G_MODE_PDU
}SMS_4G_MODE;

typedef struct
{
    uint8 sms_mode;
    boolean  centernum_valid;
    uint8 centernum[SMS_4G_CENTERNUM_MAX];
}SMS_4G_INFO;

extern SMS_4G_INFO sms_4g_info;

/*TODO +CMTI: + READSMS*/

void sms_4g_init(void);

uint8 sms_4g_query_centernum(AT_4G_CMD_PRIORITY pri);

uint8 sms_4g_set_centernum(AT_4G_CMD_PRIORITY pri, uint8 *num);

uint8 sms_4g_set_pdumode(AT_4G_CMD_PRIORITY pri);

uint8 sms_4g_set_txtmode(AT_4G_CMD_PRIORITY pri);

uint8 sms_4g_send_sms_len(AT_4G_CMD_PRIORITY pri, uint8 len);

uint8 sms_4g_send_sms_data(uint8 *data, uint8 len);

uint8 sms_4g_del_all(AT_4G_CMD_PRIORITY pri);

#endif /* TBOX_4G_SMS_H */
