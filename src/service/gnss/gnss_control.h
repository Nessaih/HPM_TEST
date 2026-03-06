#ifndef __GNSS_CONTROL_H__
#define __GNSS_CONTROL_H__

INT32 gnss_control_init(UINT8 seq);

VOID gnss_control_wake(VOID);

VOID gnss_control_sleep(VOID);

VOID gnss_control_handle_cfg_change(TBOX_MSG_DATA *data);

#endif
