#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"
#include "4g_if.h"

#define NET_4G_TMEP_BUFF_LEN 256

static uint8 net_4g_creg_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_gprs_reg_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_ereg_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_query_signalstrength_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_query_ps_attachstate_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_config_autoscan_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_query_cops_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_lockat_2g_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_get_laccellid_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_get_operator_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_config_apn_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_query_apn_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_active_apn_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_deactive_apn_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_open_socket_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_close_socket_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_query_socket_state_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_send_data_len_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_send_data_resp(void *context, uint8 *data, uint16 *len);

static uint8 net_4g_enable_updata_timezone_resp(void *context, uint8 *data, uint16 *len);

NET_4G_INFO net_4g_info;

static INT8 net_4g_current_apn_index;

void net_4g_init(void)
{
    net_4g_info.reg_state = NET_4G_UNREGISTERED;
    net_4g_info.gprs_reg_state = NET_4G_UNREGISTERED;
    net_4g_info.ereg_state = NET_4G_UNREGISTERED;
    net_4g_info.signalstrength = 99;
    net_4g_info.ps_attach_state = NET_4G_PS_ATTACH_UNKNOWN;
    net_4g_info.laccellid_valid = FALSE;
    memset(net_4g_info.lac_cellid, 0, sizeof(net_4g_info.lac_cellid));
    net_4g_info.operator_valid = FALSE;
    memset(&net_4g_info.operator, 0, sizeof(net_4g_info.operator));
    net_4g_info.cops_mode = 0;

    net_4g_current_apn_index = -1;
    for(uint8 index = 0; index < NET_4G_APN_MAX_COUNT; index++)
    {
        net_4g_info.apns[index].cid = -1;
        net_4g_info.apns[index].name[0] = '\0';
        net_4g_info.apns[index].exists_in4g = 0;
    }
}

uint8 net_4g_creg(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CREG?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CREG;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_creg_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    TBOX_4G_MUTEX_LOCK();
    net_4g_info.reg_state = NET_4G_UNREGISTERED;
    TBOX_4G_MUTEX_UNLOCK();

    return 0;
}

uint8 net_4g_grps_reg(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CGREG?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_GPRS_REG;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_gprs_reg_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    TBOX_4G_MUTEX_LOCK();
    net_4g_info.gprs_reg_state = NET_4G_UNREGISTERED;
    TBOX_4G_MUTEX_UNLOCK();

    return 0;
}

uint8 net_4g_ereg(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CEREG?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CEREG;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_ereg_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    TBOX_4G_MUTEX_LOCK();
    net_4g_info.ereg_state = NET_4G_UNREGISTERED;
    TBOX_4G_MUTEX_UNLOCK();

    return 0;
}

uint8 net_4g_query_signalstrength(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CSQ" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_SIGNALSTRENGTH;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_query_signalstrength_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_query_ps_attachstate(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CGATT?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_PS_ATTACH_STATE;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_query_ps_attachstate_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_config_auto_scan(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+COPS=0" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CONFIG_AUTO_SCAN;
    cmd.retry_count = 0;
    cmd.tick = 10000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_config_autoscan_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_query_cops(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+COPS?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_COPS;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_query_cops_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_lockat_2g(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QCFG=\"NWSCANMODE\",1\r\n";
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CONFIG_LOCKAT_2G;
    cmd.retry_count = 0;
    cmd.tick = 300;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_lockat_2g_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_get_lac_cellid(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CGREG=2" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_GET_LACCELLID;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_get_laccellid_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_get_operator_info(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+COPS?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_GET_OPERATOR;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_get_operator_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_config_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id, uint8 *apn,
                        uint8 *name, uint8 *psw)						
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    uint8 i;
    sharemem_4g_ele *ele = NULL;

    if(NULL == apn ||
       0 == strlen((char*)apn))
    {
        return 1;
    }

    for(i = 0; i < NET_4G_APN_MAX_COUNT; i++)
    {
        if(context_id == net_4g_info.apns[i].cid)
        {
            break;
        }
        if(net_4g_info.apns[i].cid < 0)
        {
            break;
        }
    }
    if(i >= NET_4G_APN_MAX_COUNT)
    {
        MODULE_LOG_E(TBOX4G, "failed to save apn information");
        return 1;
    }
    net_4g_info.apns[i].cid = context_id;
    strncpy((CHAR*)net_4g_info.apns[i].name, (CHAR*)apn, NET_APN_NAME_MAX - 1);
    net_4g_current_apn_index = i;

    MODULE_LOG_I(TBOX4G, "config apn[%s] name[%s] password[%s] cur_index:%d", (CHAR*)net_4g_info.apns[i].name, (CHAR*)name, (CHAR*)psw, net_4g_current_apn_index);

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        MODULE_LOG_E(TBOX4G, "failed to alloc memory");
        return 1;
    }


    if(NULL == name ||
       0 == strlen((char*)name) ||
       NULL == psw ||
       0 == strlen((char*)psw))
    {
        snprintf((char*)ele->mem_ptr, 63, "AT+QICSGP=%d,1,\"%s\",\"\",\"\",1" AT_4G_REQ_SUFFIX,
                             context_id, (CHAR*)apn);
        ele->len = strlen((char*)ele->mem_ptr);
    }
    else if(NULL != name &&
        0 < strlen((char*)name) &&
        NULL != psw &&
        0 < strlen((char*)psw))
    {
        snprintf((char*)ele->mem_ptr, 63, "AT+QICSGP=%d,1,\"%s\",\"%s\",\"%s\",1" AT_4G_REQ_SUFFIX,
                             context_id, (CHAR*)apn, (CHAR*)name, (CHAR*)psw);
        ele->len = strlen((char*)ele->mem_ptr);
    }
    else
    {
        sharemem_4g_free(index);
        return 1;
    }

    cmd.cmd_id = AT_4G_CONFIG_APN;
    cmd.retry_count = 0;
    cmd.tick = 3000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_config_apn_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

void net_4g_set_apn(uint8 context_id, uint8 *apn)
{
    uint8 i;
    for(i = 0; i < NET_4G_APN_MAX_COUNT; i++)
    {
        if(context_id == net_4g_info.apns[i].cid)
        {
            break;
        }
        if(net_4g_info.apns[i].cid < 0)
        {
            break;
        }
    }
    if(i >= NET_4G_APN_MAX_COUNT)
    {
        MODULE_LOG_E(TBOX4G, "failed to save apn information");
        return;
    }

    net_4g_info.apns[i].cid = context_id;
    strncpy((CHAR*)net_4g_info.apns[i].name, (CHAR*)apn, NET_APN_NAME_MAX);
    net_4g_info.apns[i].exists_in4g = 0;
}

uint8 net_4g_apn_query_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31, "AT+QICSGP=%d " AT_4G_REQ_SUFFIX,context_id);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_QUERY_APN;
    cmd.retry_count = 0;
    cmd.tick = 3000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_query_apn_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

boolean net_4g_apn_isalready_config(uint8 context_id, uint8 *apn)
{
    uint8 i;
    for(i = 0; i < NET_4G_APN_MAX_COUNT; i++)
    {
        if(context_id == net_4g_info.apns[i].cid &&
           0 == strcmp((CHAR*)apn, (CHAR*)net_4g_info.apns[i].name))
        {
            break;
        }
    }
    if(i >= NET_4G_APN_MAX_COUNT)
    {
        return FALSE;
    }
    if(1 != net_4g_info.apns[i].exists_in4g)
    {
       return FALSE;
    }

    return TRUE;
}

uint8 net_4g_active_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 15 , "AT+QIACT=%d" AT_4G_REQ_SUFFIX, context_id);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_ACTIVE_APN;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_active_apn_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_deactive_apn(AT_4G_CMD_PRIORITY pri, uint8 context_id)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31, "AT+QIDEACT=%d" AT_4G_REQ_SUFFIX, context_id);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_DEACTIVE_APN;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_deactive_apn_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_open_socket(AT_4G_CMD_PRIORITY pri, uint8 context_id, uint8 conn_id,
                         uint8 *service_type, uint8 *ip, uint32 port)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 63, "AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,0,1" AT_4G_REQ_SUFFIX, context_id,
                        conn_id, (CHAR*)service_type, (CHAR*)ip, port);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_OPEN_SOCKET;
    cmd.retry_count = 0;
    cmd.tick = 10000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_open_socket_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_close_socket(AT_4G_CMD_PRIORITY pri, uint8 conn_id)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31, "AT+QICLOSE=%d" AT_4G_REQ_SUFFIX, conn_id); //default close socket timeout is 10s
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_CLOSE_SOCKET;
    cmd.retry_count = 0;
    cmd.tick = 12000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_close_socket_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_query_socket_state(AT_4G_CMD_PRIORITY pri, uint8 conn_id)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31, "AT+QISTATE=1,%d" AT_4G_REQ_SUFFIX, conn_id);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_QUERY_SOCKET_STATE;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_query_socket_state_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_send_data_len(AT_4G_CMD_PRIORITY pri, uint8 conn_id, uint16 len)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31,"AT+QISEND=%d,%d" AT_4G_REQ_SUFFIX, conn_id, len);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_SOCKET_DATA_LEN;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_send_data_len_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 net_4g_send_data(uint8 *data, uint16 len)
{
    AT_4G_CMD cmd;

    cmd.cmd_id = AT_4G_SOCKET_DATA;
    cmd.retry_count = 0;
    cmd.tick = 15*1000;
    cmd.sharm_index = -1;
    cmd.state = (AT_CMD_STATE_WAITSEND|AT_CMD_STATE_WAITRESP);
    cmd.resp = net_4g_send_data_resp;
    if(AT_4G_TRANS_SEND_OK != at_4g_transmit_direct_setcmd(&cmd))
    {
        return 1;
    }

    at_4g_transmit_direct_send(data, len, NULL);

    return 0;
}

uint8 net_4g_enable_updata_timezone(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+CTZU=1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_UPDATA_TZ;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = net_4g_enable_updata_timezone_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 net_4g_creg_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_CREG != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint8 reg_state;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CREG: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        if((uint16)(temp-data) >= temp_len)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_len = temp_len - (uint16)(temp-data);
        temp = tbox_string_get_substring(temp, temp_len, ",");
        if(NULL == temp)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp++;
        if(isdigit(*temp))
        {
            reg_state = *temp - '0';
            if(reg_state > NET_4G_ROAMING)
            {
                return AT_4G_DECODE_INVALID_PACKET;
            }
        }
        else
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        TBOX_4G_MUTEX_LOCK();
        net_4g_info.reg_state = reg_state;
        TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_CREG, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_CREG, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_gprs_reg_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_GPRS_REG != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint8 reg_state;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CGREG: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        if((uint16)(temp-data) >= temp_len)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        if(sscanf((char*)temp, "+CGREG: %*d,%hhu", &reg_state))
        {
            TBOX_4G_MUTEX_LOCK();
            net_4g_info.gprs_reg_state = reg_state;
            TBOX_4G_MUTEX_UNLOCK();
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_GPRS_REG, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_GPRS_REG, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_ereg_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_CEREG != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint8 reg_state;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CEREG: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        if((uint16)(temp-data) >= temp_len)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        if(sscanf((char*)temp, "+CEREG: %*d,%hhu", &reg_state))
        {
            TBOX_4G_MUTEX_LOCK();
            net_4g_info.ereg_state = reg_state;
            TBOX_4G_MUTEX_UNLOCK();
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_CEREG, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_CEREG, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_query_signalstrength_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_SIGNALSTRENGTH != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint32 signalstrength = 99;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CSQ: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        if ((uint16)(token - data) >= temp_len)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_len = temp_len - (uint16)(token - data);
        token = tbox_string_get_substring(temp, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        temp = temp + strlen("+CSQ: ");
        if(FALSE == tbox_string_get_num(temp,  ',', (uint32*)&signalstrength))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

		TBOX_4G_MUTEX_LOCK();
        net_4g_info.signalstrength = (uint8)signalstrength;
		TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SIGNALSTRENGTH, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        TBOX_4G_MUTEX_LOCK();
        net_4g_info.signalstrength = 99;
        TBOX_4G_MUTEX_UNLOCK();
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SIGNALSTRENGTH, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_query_ps_attachstate_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_PS_ATTACH_STATE != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CGATT: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        temp = temp + strlen("+CGATT: ");
        if(!isdigit(*temp))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        net_4g_info.ps_attach_state = *temp - '0';

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_PS_ATTACH_STATE, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        net_4g_info.ps_attach_state = NET_4G_PS_ATTACH_UNKNOWN;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_PS_ATTACH_STATE, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_config_autoscan_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_AUTO_SCAN, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 net_4g_query_cops_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_COPS != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        INT32 cops_mode;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+COPS: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        if(sscanf((char*)temp, "+COPS: %d", &cops_mode))
        {
            MODULE_LOG_I(TBOX4G, "cops_mode:%d", cops_mode);
            net_4g_info.cops_mode = (uint8)cops_mode;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_COPS, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_COPS, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_lockat_2g_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_LOCKAT_2G, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 net_4g_get_laccellid_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;

    if(AT_4G_GET_LACCELLID == resp->cmd.cmd_id)
    {
        uint8 ret = at_4g_common_resp(AT_4G_GET_LACCELLID, (AT_4G_CMD_RESP *)context, data, len);
        if(AT_4G_DECODE_ISMATCH == ret)
        {
            AT_4G_CMD cmd;
            INT8 index = -1;
            char *at_str = "AT+CGREG?" AT_4G_REQ_SUFFIX;
            sharemem_4g_ele *ele = NULL;

            resp->resp_code = AT_4G_RESP_UNKNOWN;

            ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
            if(NULL != ele)
            {
                ele->len = strlen(at_str);
                memcpy(ele->mem_ptr, at_str, ele->len);

                cmd.cmd_id = AT_4G_GPRS_REG;
                cmd.retry_count = 0;
                cmd.tick = 1000;
                cmd.sharm_index = index;
                cmd.state = AT_CMD_SEND_REQ_WAITRESP;
                cmd.resp = net_4g_get_laccellid_resp;
                at_4g_transmit_putcmd(resp->pri, &cmd);
            }
        }
        return ret;
    }
    else if(AT_4G_GPRS_REG == resp->cmd.cmd_id)
    {
        uint8 *temp;
        uint8 *token;
        uint8  count = 0;
        uint16 temp_len = 0;

        if(AT_4G_RESP_UNKNOWN == resp->resp_code)
        {
            temp_len = *len;
            token = tbox_string_get_substring(data, temp_len, "+CGREG: ");
            if(NULL == token)
            {
                return AT_4G_DECODE_NOMATCH;
            }
            temp = token;
            token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
            if(NULL == token)
            {
                return AT_4G_DECODE_CONTINUE;
            }

            memset(net_4g_info.lac_cellid, 0, sizeof(net_4g_info.lac_cellid));

            temp_len = temp_len - (uint16)(temp-data);
            temp = tbox_string_get_substring(temp, temp_len, "\"");
            if(NULL == temp)
            {
                net_4g_info.laccellid_valid = TRUE;
                seqmgr_4g_handle_cmd_exe_result(AT_4G_GPRS_REG, SEQMGR_4G_CMD_EXE_OK);
                *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
                return AT_4G_DECODE_ISMATCH;
            }

            temp++;
            net_4g_info.lac_cellid[count++] = '\"';
            while(*temp != '\"')
            {
                net_4g_info.lac_cellid[count++] = *temp;
                temp++;
                if(count >= SIM_4G_LACCELLID_MAX)
                {
                    return AT_4G_DECODE_INVALID_PACKET;
                }
            }

            temp = temp + 1;
            temp_len = *len - (uint16)(temp-data);
            temp = tbox_string_get_substring(temp, temp_len, "\"");
            if(NULL == temp)
            {
                return AT_4G_DECODE_INVALID_PACKET;
            }
            else
            {
                temp++;
                net_4g_info.lac_cellid[count++] = ',';
            }
            while(*temp != '\"')
            {
                net_4g_info.lac_cellid[count++] = *temp;
                temp++;
                if(count >= SIM_4G_LACCELLID_MAX)
                {
                    return AT_4G_DECODE_INVALID_PACKET;
                }
            }
            if(count+1 >= SIM_4G_LACCELLID_MAX)
            {
                return AT_4G_DECODE_INVALID_PACKET;
            }
            net_4g_info.lac_cellid[count++] = '\"';
            if(count > 3)
            {
                net_4g_info.laccellid_valid = TRUE;
            }

            seqmgr_4g_handle_cmd_exe_result(AT_4G_GPRS_REG, SEQMGR_4G_CMD_EXE_OK);
            *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
            return AT_4G_DECODE_ISMATCH;
         }
         else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
                 AT_4G_RESP_ABORT == resp->resp_code ||
                 (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
                  resp->resp_code <= AT_4G_RESP_MAX_ERROR))
         {
             net_4g_info.laccellid_valid = FALSE;
             seqmgr_4g_handle_cmd_exe_result(AT_4G_GPRS_REG, SEQMGR_4G_CMD_EXE_FAILED);
             return AT_4G_DECODE_ISMATCH;
         }
         else
         {
             /**/
         }
         return AT_4G_DECODE_NOMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_get_operator_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_GET_OPERATOR != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint8 act;
        uint8 name[NET_OPERATOR_NAME_MAX] = {'\0'};

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+COPS: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        if(sscanf((char*)temp, "+COPS: %*d,%*d,\"%15[^\"]\",%hhu", name, &act))
        {
            net_4g_info.operator.act = act;
            strncpy((char*)net_4g_info.operator.name, (char*)name, NET_OPERATOR_NAME_MAX - 1);
            net_4g_info.operator_valid = TRUE;
        }
        else
        {
            net_4g_info.operator.act = 0;
            memset(net_4g_info.operator.name, 0, NET_OPERATOR_NAME_MAX);
            net_4g_info.operator_valid = TRUE;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_GET_OPERATOR, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        net_4g_info.operator_valid = FALSE;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_GET_OPERATOR, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_config_apn_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_APN, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 net_4g_query_apn_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;
    char *quote_start, *quote_end;
    char *apn_str = NULL_PTR;

    if(AT_4G_QUERY_APN != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint8 index;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QICSGP:");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }

        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }
        temp = (uint8 *)strchr((char *)data, ',');
        if(NULL_PTR == temp)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_OK);
            *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
            return AT_4G_DECODE_ISMATCH;            
        }
        quote_start = strchr((char *)(temp+1U), '\"');
        if(NULL_PTR == quote_start)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_OK);
            *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
            return AT_4G_DECODE_ISMATCH; 
        }
        quote_end = strchr(quote_start+1U, '\"');
        if(NULL_PTR == quote_end)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_OK);
            *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
            return AT_4G_DECODE_ISMATCH; 
        }
        apn_str = (char *)mempool_alloc(NET_APN_NAME_MAX+1U);
        if(NULL_PTR == apn_str)
        {
            MODULE_LOG_E(TBOX4G, "failed to alloc memory for apn string");
            seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_OK);
            *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
            return AT_4G_DECODE_ISMATCH;            
        }
        temp_len = (uint16)(quote_end - quote_start - 1U);
        if(temp_len > NET_APN_NAME_MAX)
        {
            MODULE_LOG_E(TBOX4G, "the apn lenght too long:%d", temp_len);
            seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_OK);
            *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
            return AT_4G_DECODE_ISMATCH;               
        }
        strncpy(apn_str, quote_start + 1U, temp_len);

        MODULE_LOG_I(TBOX4G, "receive apn information:%s", apn_str);

        for(index = 0; index < NET_4G_APN_MAX_COUNT; index++)
        {
            if(net_4g_info.apns[index].cid >= 0)
            {
                if(strlen((char*)net_4g_info.apns[index].name) != temp_len)
                {
                    continue;
                }
                if(0 == strncmp((char*)net_4g_info.apns[index].name, apn_str, temp_len))
                {
                    MODULE_LOG_I(TBOX4G, "the apn[%s] exists in the 4G module", (char*)net_4g_info.apns[index].name);
                    net_4g_info.apns[index].exists_in4g = 1;
                }
            }
        }

        mempool_free(apn_str);
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_OK);
        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_APN, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_active_apn_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_ACTIVE_APN, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 net_4g_deactive_apn_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_DEACTIVE_APN, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 net_4g_open_socket_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;
    uint32 ret_code = 0;

    if(AT_4G_OPEN_SOCKET != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code ||
       AT_4G_RESP_OK == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QIOPEN: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        if((uint16)(temp-data) >= temp_len)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_len = temp_len - (uint16)(temp-data);
        temp = tbox_string_get_substring(temp, temp_len, ",");
        if(NULL == temp)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp++;
        if(FALSE == tbox_string_get_num(temp, '\r', (uint32*)&ret_code))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        if(0 == ret_code)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_OPEN_SOCKET, SEQMGR_4G_CMD_EXE_OK);
        }
        else
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_OPEN_SOCKET, SEQMGR_4G_CMD_EXE_FAILED);
        }

        token = tbox_string_get_substring(temp, *len-(uint16)(temp-data), "\r\n");
        if(NULL == token)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        *len = (uint16)(token-data+strlen("\r\n"));
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_OPEN_SOCKET, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_close_socket_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CLOSE_SOCKET, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 net_4g_query_socket_state_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint8 temp_value, count = 0;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_SOCKET_STATE != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QISTATE: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        if((uint16)(temp-data) >= temp_len)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_len = temp_len - (uint16)(temp-data);
        for(temp_value = 0; temp_value < temp_len; temp_value++)
        {
            if(temp[temp_value] == ',')
            {
                count++;
                if(count == 5)
                {
                    break;
                }
            }
        }
        if(count != 5)
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp = temp + temp_value + 1;
        if(!isdigit(*temp))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_value = *temp - '0';
        if(temp_value == 2)
        {
            temp_value = (NET_4G_SOCKET_CONN_OK<<4 | SEQMGR_4G_CMD_EXE_OK);
        }
        else
        {
            temp_value = (NET_4G_SOCKET_DISCONNECT<<4 | SEQMGR_4G_CMD_EXE_OK);
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SOCKET_STATE, temp_value);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_OK == resp->resp_code)
    {
        temp_value = (NET_4G_SOCKET_DISCONNECT<<4 | SEQMGR_4G_CMD_EXE_OK);
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SOCKET_STATE, temp_value);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SOCKET_STATE, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_send_data_len_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_SOCKET_DATA_LEN != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "> ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_SOCKET_DATA_LEN, SEQMGR_4G_CMD_EXE_OK);

        if((uint16)((token-data)+2) >= temp_len)
        {
            *len = temp_len;
            return AT_4G_DECODE_ISMATCH;
        }
        temp_len = *len - (uint16)(token-data) - 2;
        token += 2;
        if(temp_len > 0 && *token == '\r')
        {
            temp_len--;
            token++;
        }
        if(temp_len > 0 && *token == '\n')
        {
            temp_len--;
            token++;
        }
        *len = (uint16)(token-data);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_SOCKET_DATA_LEN, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_send_data_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_SOCKET_DATA != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "SEND OK");
        if(NULL != token)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_SOCKET_DATA, SEQMGR_4G_CMD_EXE_OK);
            token += strlen("SEND OK");
        }
        else
        {
            token = tbox_string_get_substring(data, temp_len, "SEND FAIL");
            if(NULL != token)
            {
                seqmgr_4g_handle_cmd_exe_result(AT_4G_SOCKET_DATA, SEQMGR_4G_CMD_EXE_FAILED);
            }
            else
            {
                return AT_4G_DECODE_NOMATCH;
            }
            token += strlen("SEND FAIL");
        }
        if(*token == '\r')
        {
            token++;
        }
        if(*token == '\n')
        {
            token++;
        }
        *len = (uint16)(token-data);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            AT_4G_RESP_ERROR == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_SOCKET_DATA, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 net_4g_enable_updata_timezone_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_UPDATA_TZ, (AT_4G_CMD_RESP *)context, data, len);
}
