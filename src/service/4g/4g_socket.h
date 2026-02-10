#ifndef TBOX_4G_SOCKET_H
#define TBOX_4G_SOCKET_H

typedef enum
{
    SOCKET_4G_SOCKT_TCP = IF_4G_SOCKT_TCP,
    SOCKET_4G_SOCKT_UDP = IF_4G_SOCKT_UDP
}SOCKET_4G_SOCKT_TYPE;

typedef enum
{
    SOCKET_4G_MGR_IDLE = 0,
    SOCKET_4G_MGR_BUSY
}SOCKET_4G_MGR_STATE;

typedef enum
{
    SOCKET_4G_STATE_DISCONNECTED = IF_4G_STATE_DISCONNECTED,
    SOCKET_4G_STATE_DISCONNECTEING = IF_4G_STATE_DISCONNECTEING,
    SOCKET_4G_STATE_CONNECTING = IF_4G_STATE_CONNECTING,
    SOCKET_4G_STATE_CONNECTED = IF_4G_STATE_CONNECTED
}SOCKET_4G_CONN_STATE;

void socket_4g_init(void);

uint8 socket_4g_connect(uint8 context_id,
                        uint8 conn_id,
                        uint8 sockt_type,
                        uint8 *ip,
                        uint32 port);

uint8 socket_4g_close(uint8 conn_id);

void socket_4g_close_all(void);

boolean socket_4g_isall_close(void);

INT8 socekt_4g_get_connid(uint8 index);

uint8 socket_4g_get_conn_state(uint8 conn_id);

uint8 socket_4g_mgr_state(void);

void socket_4g_set_conn_state(uint8 conn_id, uint8 state);

void socket_4g_set_all_disconnect(void);

uint8 socket_4g_send(uint8 pri, uint8 conn_id, uint8 *data, uint16 len);

void socket_4g_periodic(void);

//void socket_4g_close_cursock(void);

#endif /* TBOX_4G_SOCKET_H */
