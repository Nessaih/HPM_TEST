#ifndef TBOX_4G_NETWORK_H
#define TBOX_4G_NETWORK_H

#define NET_4G_SOCKET_CONN_OK    1
#define NET_4G_SOCKET_DISCONNECT 2
#define NET_4G_APN_MAX_COUNT     3

#include "4g_if.h"

typedef enum
{
    NET_4G_UNREGISTERED = IF_4G_UNREGISTERED,
    NET_4G_REGISTERED = IF_4G_REGISTERED,
    NET_4G_BUSY = IF_4G_BUSY,
    NET_4G_DENIED = IF_4G_DENIED,
    NET_4G_UNKNOWN = IF_4G_UNKNOWN,
    NET_4G_ROAMING = IF_4G_ROAMING,
} NET_4G_REG_STATE;

typedef enum
{
    NET_4G_PS_ATTACH_UNKNOWN = 0,
    NET_4G_PS_ATTACH,
    NET_4G_PS_DETACH,
}NET_4G_PS_ATTACH_STATE;

typedef enum
{
    NET_4G_PROTOCOL_TCP = 0x0,
    NET_4G_PROTOCOL_UDP
}NET_4G_PROTOCOL_TYPE;

typedef enum
{
    NET_4G_DATA_DECODE = 0x00,
    NET_4G_DATA_NODECODE = 0x02
}NET_4G_DATA_FORMAT;

typedef struct
{
    uint8 act;
    uint8 name[NET_OPERATOR_NAME_MAX];
}NET_4G_OPERATOR_INFO;

typedef struct
{
    INT8  cid;
    uint8 exists_in4g;
    uint8 name[NET_APN_NAME_MAX];
}NET_4G_APN_INFO;

typedef struct
{
   uint8 reg_state;          /*CS 域网络注册状态*/
   uint8 gprs_reg_state;     /*PS 域网络注册状态*/
   uint8 ereg_state;         /*EPS网络注册状态*/
   uint8 signalstrength;
   uint8 ps_attach_state;
   boolean  laccellid_valid;
   uint8 lac_cellid[SIM_4G_LACCELLID_MAX];
   boolean operator_valid;
   NET_4G_OPERATOR_INFO operator;
   uint8 cops_mode;
   NET_4G_APN_INFO apns[NET_4G_APN_MAX_COUNT];
}NET_4G_INFO;

extern NET_4G_INFO net_4g_info;

void net_4g_init(void);

uint8 net_4g_creg(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_grps_reg(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_ereg(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_query_signalstrength(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_query_ps_attachstate(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_config_auto_scan(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_query_cops(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_lockat_2g(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_get_lac_cellid(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_get_operator_info(AT_4G_CMD_PRIORITY pri);

uint8 net_4g_config_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id, uint8 *apn,
                        uint8 *name, uint8 *psw);

void net_4g_set_apn(uint8 context_id, uint8 *apn);

uint8 net_4g_apn_query_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id);

boolean net_4g_apn_isalready_config(uint8 context_id, uint8 *apn);

uint8 net_4g_active_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id);

uint8 net_4g_deactive_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id);

uint8 net_4g_open_socket(AT_4G_CMD_PRIORITY pri, uint8 context_id, uint8 conn_id,
                         uint8 *service_type, uint8 *ip, uint32 port);

uint8 net_4g_close_socket(AT_4G_CMD_PRIORITY pri, uint8 conn_id);

uint8 net_4g_query_socket_state(AT_4G_CMD_PRIORITY pri, uint8 conn_id);

uint8 net_4g_send_data_len(AT_4G_CMD_PRIORITY pri, uint8 conn_id, uint16 len);

uint8 net_4g_send_data(uint8 *data, uint16 len);

uint8 net_4g_enable_updata_timezone(AT_4G_CMD_PRIORITY pri);

#endif /* TBOX_4G_NETWORK_H */
