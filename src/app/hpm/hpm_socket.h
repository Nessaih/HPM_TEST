#ifndef __HPM_SOCKET_H__
#define __HPM_SOCKET_H__

typedef enum
{
    HPM_SOCKET_STEP_INIT			= 0x00,
    HPM_SOCKET_STEP_PRECONDITION	= 0x01,
    HPM_SOCKET_STEP_WAIT_DIAL		= 0x02,
    HPM_SOCKET_STEP_OPEN			= 0x03,
    HPM_SOCKET_STEP_CONNECTING		= 0x04,
    HPM_SOCKET_STEP_CONNECTED		= 0x05,
    HPM_SOCKET_STEP_CLOSE			= 0x06,
    HPM_SOCKET_STEP_MAX				= HPM_SOCKET_STEP_CLOSE+1
}HPM_SOCKET_STEP_E;

INT32 hpm_socket_init(UINT8 seq);

VOID hpm_socket_timeout_proc(VOID);

VOID hpm_socket_reset(VOID);

HPM_SOCKET_STEP_E hpm_socket_get_status(VOID);

#endif
