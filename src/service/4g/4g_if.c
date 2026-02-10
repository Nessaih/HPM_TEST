#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_sms.h"
#include "4g_dial.h"
#include "4g_socket.h"
#include "4g_sequence_mgr.h"
#include "4g_power.h"
#include "4g_at_startup.h"
#include "4g_ftp.h"
#include "4g_time.h"
#include "4g_mgr.h"

typedef struct
{
    INT8 conn_id;
    IF_4G_SEND_CALLBACK send_callbak;
    IF_4G_RECV_CALLBAKC recv_callback;
}IF_4G_TRANSMIT_INFO;

static IF_4G_TRANSMIT_INFO if_4g_trans_mgr[SOCKET_4G_COUNT];

INT8 if_4g_get_call_state(UINT8 apn_index)
{
    return dial_4g_get_callstate(apn_index);
}

UINT8 if_4g_get_state(VOID)
{
    return tbox_4g_get_state();
}

VOID if_4g_reset(VOID)
{
    /*只是复位模块内部状态，不涉及到硬件复位。4G模块复位完成后，调用这个函数重置状态*/
    tbox_4g_reset();
}

VOID if_4g_set_apn(UINT8 index,
                   UINT8 *apn,
                   UINT8 *username,
                   UINT8 *password)
{
    dial_4g_set_apn(index, apn, username, password);
    dial_4g_stopcall();
}

UINT8 if_4g_reg_transmit_callback(UINT8 conn_id,
                                  IF_4G_SEND_CALLBACK send_callback,
                                  IF_4G_RECV_CALLBAKC recv_callback)
{
    UINT8 index;

    for(index = 0U; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id ==  if_4g_trans_mgr[index].conn_id)
        {
            TBOX_4G_MUTEX_LOCK();
            if_4g_trans_mgr[index].recv_callback = recv_callback;
            if_4g_trans_mgr[index].send_callbak = send_callback;
            TBOX_4G_MUTEX_UNLOCK();
            break;
        }
    }
    if(index < SOCKET_4G_COUNT)
    {
        return 0U;
    }

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(-1 ==  if_4g_trans_mgr[index].conn_id)
        {
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        return 1U;
    }
    
    TBOX_4G_MUTEX_LOCK();
    if_4g_trans_mgr[index].conn_id = conn_id;
    if_4g_trans_mgr[index].recv_callback = recv_callback;
    if_4g_trans_mgr[index].send_callbak = send_callback;
    TBOX_4G_MUTEX_UNLOCK();

    return 0U;
}

UINT8 if_4g_socket_connect(UINT8 context_id,
                           UINT8 conn_id,
                           UINT8 sockt_type,
                           UINT8 *ip, UINT32 port)
{
    return socket_4g_connect(context_id, conn_id, sockt_type, ip, port);
}

UINT8 if_4g_socket_close(UINT8 conn_id)
{
    return socket_4g_close(conn_id);
}

UINT8 if_4g_socket_send(UINT8 conn_id, UINT8 *data, UINT16 len)
{
    return socket_4g_send(IF_4G_SEND_PRI_LOW, conn_id, data, len);
}

UINT8 if_4g_socket_send_ex(IF_4G_SEND_PRI priority, UINT8 conn_id, UINT8 *data, UINT16 len)
{
    return socket_4g_send(priority, conn_id, data, len);
}

INT8 if_4g_get_socket_conn_state(INT8 conn_id)
{
    return socket_4g_get_conn_state(conn_id);
}

VOID if_4g_init(VOID)
{
    UINT8 index;

    for(index = 0U; index < SOCKET_4G_COUNT; index++)
    {
        if_4g_trans_mgr[index].conn_id = -1;
        if_4g_trans_mgr[index].recv_callback = NULL;
        if_4g_trans_mgr[index].send_callbak = NULL;
    }
}

IF_4G_SEND_CALLBACK if_4g_get_send_callback(UINT8 conn_id)
{
    UINT8 index;

    IF_4G_SEND_CALLBACK callback = NULL_PTR;

    for(index = 0U; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id ==  if_4g_trans_mgr[index].conn_id)
        {
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        return 0U;
    }

    TBOX_4G_MUTEX_LOCK();
    callback = if_4g_trans_mgr[index].send_callbak;
    TBOX_4G_MUTEX_UNLOCK();

    return callback;
}

IF_4G_RECV_CALLBAKC if_4g_get_recv_callback(UINT8 conn_id)
{
    UINT8 index;
    
    IF_4G_RECV_CALLBAKC callback = NULL_PTR;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id ==  if_4g_trans_mgr[index].conn_id)
        {
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        return 0;
    }

    TBOX_4G_MUTEX_LOCK();
    callback = if_4g_trans_mgr[index].recv_callback;
    TBOX_4G_MUTEX_UNLOCK();

    return callback;
}

BOOL if_4g_get_imei(UINT8 *imei, UINT8 *len)
{
    TBOX_4G_MUTEX_LOCK();

    uint8 temp_len = sizeof(modem_4g_info.modem_4g_imei);
    if(FALSE == modem_4g_info.imei_is_valid)
    {
		TBOX_4G_MUTEX_UNLOCK();
        return FALSE;
    }
    if(*len < MODEM_4G_IMEI_MAX)
    {
		TBOX_4G_MUTEX_UNLOCK();
        return FALSE;
    }

    memcpy(imei, modem_4g_info.modem_4g_imei, temp_len);
    *len = temp_len;

	TBOX_4G_MUTEX_UNLOCK();

    return TRUE;
}

BOOL if_4g_get_iccid(UINT8 *iccid, UINT8 *len)
{
    TBOX_4G_MUTEX_LOCK();

    UINT8 temp_len = sizeof(sim_4g_info.sim_4g_iccid);
    if(FALSE == sim_4g_info.iccid_is_valid)
    {
		TBOX_4G_MUTEX_UNLOCK();
        return FALSE;
    }
    if(*len < SIM_4G_ICCID_MAX)
    {
		TBOX_4G_MUTEX_UNLOCK();
        return FALSE;
    }

    memcpy(iccid, sim_4g_info.sim_4g_iccid, temp_len);
    *len = temp_len;

	TBOX_4G_MUTEX_UNLOCK();
    return TRUE;
}

BOOL if_4g_get_phone_num(UINT8 *phone_num, UINT8 *len)
{
    TBOX_4G_MUTEX_LOCK();

    UINT8 temp_len = sizeof(sim_4g_info.sim_4g_num);
    if(FALSE == sim_4g_info.num_is_valid)
    {
		TBOX_4G_MUTEX_UNLOCK();
        return FALSE;
    }
    if(*len < SIM_4G_NUM_MAX)
    {
		TBOX_4G_MUTEX_UNLOCK();
        return FALSE;
    }

    memcpy(phone_num, sim_4g_info.sim_4g_num, temp_len);
    *len = temp_len;

	TBOX_4G_MUTEX_UNLOCK();

    return TRUE;
}

BOOL if_4g_set_phone_num(UINT8 *phone_num)
{
    seqmgr_4g_abort_allseq(1);
    return (sim_4g_set_num_ex(AT_4G_CMD_HIGH, (char*)phone_num, 3) == 0);
}

VOID if_4g_query_phone_num(VOID)
{
    sim_4g_query_num_ex(AT_4G_CMD_LOW, 6);
}

UINT8 if_4g_get_signal_signalstrength(void)
{
    UINT8 signalstrength = 99;

    TBOX_4G_MUTEX_LOCK();
    signalstrength = net_4g_info.signalstrength;
	TBOX_4G_MUTEX_UNLOCK();

    return signalstrength;
}

UINT8 if_4g_get_sim_state(VOID)
{
    uint8 sim_state;

    TBOX_4G_MUTEX_LOCK();
    sim_state = sim_4g_info.sim_state;
    TBOX_4G_MUTEX_UNLOCK();

    return (UINT8)sim_state;
}

BOOL if_4g_is_downloading(void)
{
    return ftp_4g_isdownloading();
}

VOID if_ftp_4g_download(UINT8 *url, UINT16 len,
                        UINT8 context_id,
                        IF_FTP_4G_DOWNLOAD_CALLBACK call_bak)
{
    ftp_4g_download(url, len, context_id, call_bak);
}

UINT8 if_4g_get_reg_state(VOID)
{
    uint8 reg_state;
    
    TBOX_4G_MUTEX_LOCK();
    reg_state = net_4g_info.reg_state;
    TBOX_4G_MUTEX_UNLOCK();

    return (UINT8)reg_state;
}

UINT8 if_4g_get_gprs_state(void)
{
    uint8 reg_state;

    TBOX_4G_MUTEX_LOCK();
    reg_state = net_4g_info.gprs_reg_state;
    TBOX_4G_MUTEX_UNLOCK();

    return (UINT8)reg_state;
}

UINT8 if_4g_get_cereg_state(void)
{
    uint8 reg_state;

    TBOX_4G_MUTEX_LOCK();
    reg_state = net_4g_info.ereg_state;
    TBOX_4G_MUTEX_UNLOCK();

    return (UINT8)reg_state;
}

UINT8 if_4g_get_temperature(void)
{
    uint8 tempvalue;

    TBOX_4G_MUTEX_LOCK();
    tempvalue = modem_4g_info.modem_4g_temp;
    TBOX_4G_MUTEX_UNLOCK();
    
    return (UINT8)tempvalue;
}

UINT16 if_4g_get_testalivefailcount(VOID)
{
    return modem_4g_get_alivefaile_count();
}

INT32 if_4g_get_chipid(UINT8 *chipid, UINT8 len)
{
    if(NULL_PTR == chipid || 0U == len)
    {
        return (INT32)TBOX_E_FAILED;
    }
    if(len < MODEM_4G_MTID_LEN)
    {
        return (INT32)TBOX_E_FAILED;
    }

    TBOX_4G_MUTEX_LOCK();
    if(FALSE == modem_4g_info.mtid_is_valid)
    {
        TBOX_4G_MUTEX_UNLOCK();
        return (INT32)TBOX_E_FAILED;
    }
    memcpy(chipid, modem_4g_info.modem_4g_mtid, MODEM_4G_MTID_LEN);
    TBOX_4G_MUTEX_UNLOCK();

    return (INT32)TBOX_E_OK;
}

INT32 if_4g_get_fwversion(UINT8 *fwversion, UINT8 len)
{
    if(NULL_PTR == fwversion || 0U == len)
    {
        return (INT32)TBOX_E_FAILED;
    }
    if(len < MODEM_4G_TAID_LEN)
    {
        return (INT32)TBOX_E_FAILED;
    }

    TBOX_4G_MUTEX_LOCK();
    if(FALSE == modem_4g_info.taid_is_valid)
    {
        TBOX_4G_MUTEX_UNLOCK();
        return (INT32)TBOX_E_FAILED;
    }
    memcpy(fwversion, modem_4g_info.modem_4g_taid, MODEM_4G_TAID_LEN);
    TBOX_4G_MUTEX_UNLOCK();

    return (INT32)TBOX_E_OK;
}

VOID if_4g_reg_dependent_fun(IF_4G_SLEEP_DEPEND_FUN fun)
{
    power_4g_reg_depend_fun(fun);
}

VOID if_4g_cclk(IF_4G_TIME_CALLBAK call_back)
{
    time_4g_cclk(AT_4G_CMD_LOW, call_back);
}

VOID if_4g_ntp(UINT8 context_id, UINT8 *ip, UINT16 port, IF_4G_TIME_CALLBAK call_back)
{
    time_4g_ntp(AT_4G_CMD_LOW, context_id, ip, port, call_back);
}

VOID if_4g_periodic(VOID)
{
    /*TODO*/
}