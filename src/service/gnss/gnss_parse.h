#ifndef __GNSS_PARSE_H__
#define __GNSS_PARSE_H__

INT32 gnss_parse_init(UINT8 seq);

VOID gnss_parse_sleep(VOID);

VOID gnss_parse_wake(VOID);

VOID gnss_parse_periodic(VOID);

VOID gnss_parse_show_info(VOID);

VOID gnss_parse_show_nmea(bool enable);

VOID gnss_parse_info_reset(VOID);

#endif

