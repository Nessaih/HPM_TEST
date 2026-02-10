#ifndef TBOX_4G_DIAL_H
#define TBOX_4G_DIAL_H

#include "4g_if.h"

#define DIAL_4G_APN_LEN         (32)
#define DIAL_4G_USERNAME_LEN    (16)
#define DIAL_4G_PASSWORD_LEN    (16)

typedef enum
{
    DIAL_4G_MGR_INIT = 0,
    DIAL_4G_MGR_DOCALL,
    DIAL_4G_MGR_CALL_FINISH,
    DIAL_4G_MGR_ABORT_CALL,
    DIAL_4G_MGR_DOSTOP,
    DIAL_4G_MGR_STOP_FINISH
}DIAL_4G_MGR_STATE;

typedef enum
{
    DIAL_4G_PUBLIC_APN = IF_4G_PUBLIC_APN,
    DIAL_4G_PRIVATE_APN = IF_4G_PRIVATE_APN,
    DIAL_4G_OTA_APN = IF_4G_OTA_APN,
    DIAL_4G_APN_MAX_INDEX
}DIAL_4G_APN_INDEX;

typedef enum
{
    DIAL_4G_STATE_DISCONNECTED = IF_4G_STATE_DISCONNECTED,
    DIAL_4G_STATE_DISCONNECTING = IF_4G_STATE_DISCONNECTEING,
    DIAL_4G_STATE_CONNECTING = IF_4G_STATE_CONNECTING,
    DIAL_4G_STATE_CONNECTED = IF_4G_STATE_CONNECTED,
    DIAL_4G_STATE_MAX
}DIAL_4G_CONN_STATE;

typedef struct
{
    uint8 state;
    uint8 apn[DIAL_4G_APN_LEN];
    uint8 usrname[DIAL_4G_USERNAME_LEN];
    uint8 password[DIAL_4G_PASSWORD_LEN];
}DIAL_4G_NET;

void dial_4g_init(void);

void dial_4g_reset(void);

void dial_4g_set_apn(uint8 index,
                     uint8 *apn,
                     uint8 *username,
                     uint8 *password);

uint8 dial_4g_docall(void);

uint8 dial_4g_stopcall(void);

uint8 dial_4g_get_mgr_state(void);

void dial_4g_set_all_disconnect(void);

boolean dial_4g_is_all_connect(void);

boolean dial_4g_is_all_disconnect(void);

sint8 dial_4g_get_callstate(uint8 index);

void dial_4g_periodic(void);

#endif /* TBOX_4G_DIAL_H */

