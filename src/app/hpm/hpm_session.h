#ifndef __HPM_SESSION_H__
#define __HPM_SESSION_H__

UINT32 hpm_session_init(UINT8 seq);

VOID hpm_session_wake(VOID);

VOID hpm_session_sleep(VOID);

INT32 hpm_sesion_get_login_seq(UINT8 *buf);

INT32 hpm_session_get_logout_seq(UINT8 *buf);

INT32 hpm_sesion_get_data_seq(UINT8 *buf);

VOID hpm_session_proc(VOID);

VOID hpm_session_reset(VOID);

#endif
