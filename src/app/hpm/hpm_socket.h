#ifndef __HPM_SOCKET_H__
#define __HPM_SOCKET_H__

typedef enum
{
    HPM_SOCKET_STEP_INIT = 0x00,
    HPM_SOCKET_STEP_PRECONDITION,
    HPM_SOCKET_STEP_WAIT_DIAL,
    HPM_SOCKET_STEP_OPEN,
    HPM_SOCKET_STEP_CONNECTING,
    HPM_SOCKET_STEP_CONNECTED,
    HPM_SOCKET_STEP_CLOSE,
    HPM_SOCKET_STEP_MAX
} HPM_SOCKET_STEP_E;

INT32 hpm_socket_init(UINT8 seq);
VOID hpm_socket_process(VOID);
VOID hpm_socket_wakeup(VOID);

VOID hpm_socket_handle_msg(TBOX_MSG_DATA *data);

VOID hpm_socket_start(VOID);
VOID hpm_socket_stop(VOID);
VOID hpm_socket_force_stop(VOID);
BOOL hpm_socket_is_connected(VOID);
BOOL hpm_socket_in_idle(VOID);

#endif
