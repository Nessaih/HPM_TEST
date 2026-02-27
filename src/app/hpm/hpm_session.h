#ifndef __HPM_SESSION_H__
#define __HPM_SESSION_H__

typedef enum
{
    HPM_SESSION_STEP_INIT = 0x00,
    HPM_SESSION_STEP_LOGIN = 0x01,
    HPM_SESSION_STEP_SESSION = 0x02,
    HPM_SESSION_STEP_LOGOUT = 0x03,
    HPM_SESSION_STEP_MAX
} HPM_SESSION_STEP_E;

UINT32 hpm_session_init(UINT8 seq);

VOID hpm_session_wake(VOID);

VOID hpm_session_sleep(VOID);

VOID hpm_session_start(VOID);

VOID hpm_session_stop(VOID);

VOID hpm_session_force_stop(VOID);

INT32 hpm_sesion_get_login_seq(UINT8 *buf);

INT32 hpm_session_get_logout_seq(UINT8 *buf);

INT32 hpm_sesion_get_data_seq(UINT8 *buf);

VOID hpm_session_process(VOID);

HPM_SESSION_STEP_E hpm_session_get_step(VOID);

#endif
