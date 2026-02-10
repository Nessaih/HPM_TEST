#ifndef TBOX_4G_TIME_H
#define TBOX_4G_TIME_H

void time_4g_init(void);

uint8 time_4g_cclk(AT_4G_CMD_PRIORITY pri, IF_4G_TIME_CALLBAK call_back);

uint8 time_4g_cclk_settime(AT_4G_CMD_PRIORITY pri, uint8 *time);

uint8 time_4g_ntp(AT_4G_CMD_PRIORITY pri, uint8 context_id, uint8 *ip, uint16 port, IF_4G_TIME_CALLBAK call_back);

#endif /* TBOX_4G_TIME_H */
