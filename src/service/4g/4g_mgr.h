#ifndef TBOX_4G_MGR_H
#define TBOX_4G_MGR_H

#include "tbox_common.h"

typedef enum
{
    MGR_4G_SEQ_IDLE = 0,
    MGR_4G_SEQ_DO_STARTUP,
    MGR_4G_SEQ_DO_QUERYINFO,
    MGR_4G_SEQ_DO_DAIL,
    MGR_4G_SEQ_DOSTOP,
    MGR_4G_SEQ_DOSTOP_STARTUP,
    MGR_4G_SEQ_DOSTOP_QUERYINFO,
    MGR_4G_SEQ_DOSTOP_SOCKET,
    MGR_4G_SEQ_DOSTOP_DIAL,
    MGR_4G_SEQ_STOPFINISH
}MGR_4G_SEQ_STATE;

void mgr_4g_init(void);

uint8 mgr_4g_get_state(void);

boolean mgr_4g_is_stopped(void);

void mgr_4g_do_stop(void);

void mgr_4g_handle_when_sleep(void);

void mgr_4g_handle_when_wakeup(void);

void mgr_4g_all_period(void);

/*wakeup and reset*/
void mgr_4g_reset(void);

void mgr_4g_reset_sequence(void);

void mgr_4g_dump_4ginfo(void);

#endif /*TBOX_4G_MGR_H*/

