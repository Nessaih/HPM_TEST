#ifndef TBOX_4G_AT_TRANSMIT_H
#define TBOX_4G_AT_TRANSMIT_H

#include "4g_dev.h"

#define AT_4G_TRANS_CANSEND_DATA      0
#define AT_4G_TRANS_NONEEDSEND_DATA   1

#define AT_4G_TRANS_SEND_OK      0
#define AT_4G_TRANS_SEND_IS_BUSY 1

#define AT_4G_DECODE_NOMATCH     0
#define AT_4G_DECODE_ISMATCH     1
#define AT_4G_DECODE_CONTINUE    2 /*continue receiving data*/
#define AT_4G_DECODE_INVALID_PACKET 3

typedef enum
{
    AT_4G_DECODE_FILTER = 0,
    AT_4G_DECODE_DATAIN_IND,
    AT_4G_DECODE_STATE_IND,
    AT_4G_DECODE_CALLBACK_TYPE_MAX
}AT_4G_DECODE_CALLBACK_TYPE;

typedef enum
{
    AT_4G_TRANS_FILTER = 0,
    AT_4G_DIRECT_TRANS_FILTER,
    AT_4G_TRANS_FILTER_MAX
}AT_4G_TRANSMIT_FILTER;

typedef uint8 (*AT_4G_DECODE_CALLBACK)(void *context, uint8 *data, uint16 *len);
typedef uint8 (*AT_4G_TRANS_FILTER_FUN)(void);

void at_4g_transmit_init(void);

void at_4g_transmit_reset(void);

void at_4g_transmit_check_recv(void);

void at_4g_transmit_check_send(void);

void at_4g_transmit_check_timeout(void);

void at_4g_transmit_monitor_queue(void);

uint8 at_4g_transmit_direct_send(uint8 *data, uint16 len, DEV_4G_SEND_CALLBACK callback);

uint8 at_4g_transmit_direct_setcmd(AT_4G_CMD *cmd);

uint8 at_4g_transmit_putcmd(AT_4G_CMD_PRIORITY pri, AT_4G_CMD *cmd);

uint8 at_4g_transmit_reg_callback(AT_4G_DECODE_CALLBACK_TYPE type, AT_4G_DECODE_CALLBACK callback);

uint8 at_4g_transmit_reg_filter(AT_4G_TRANSMIT_FILTER filter, AT_4G_TRANS_FILTER_FUN filter_fun);

#endif /* TBOX_4G_AT_TRANSMIT_H */

