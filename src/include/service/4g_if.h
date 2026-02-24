#ifndef TBOX_4G_IF_H
#define TBOX_4G_IF_H

#include "tbox_common.h"

#define IF_4G_TIME_MAX_LEN              6U
#define IF_4G_LD_FLASH_INFO_LEN         32U
#define IF_4G_LD_APPEND_INFO_LEN        64U
#define IF_4G_MAX_ICCID_LEN             21U
#define IF_4G_MAX_IMEI_LEN              15U
#define IF_4G_MAX_PHONE_NUM_LEN         20U
#define IF_4G_SOCKET_MAX_DATA_LEN       512U

typedef struct
{
    UINT8 is_valid;
    UINT8 time[IF_4G_TIME_MAX_LEN];  //year,mon,day,hour,min,sec
}IF_4G_TIMINFO;

typedef enum
{
    IF_4G_LD_CMD_START     = 0U,
    IF_4G_LD_CMD_TRANSLATE,
    IF_4G_LD_CMD_END
}IF_4G_LD_CMD;

typedef struct
{
    UINT8   cmd;
    UINT8   seq; //from 1
    UINT16  total_len;
    UINT16  offset;
    UINT16  send_len;
}IF_4G_LD_CTL;

typedef struct
{
    UINT8 flash_info[IF_4G_LD_FLASH_INFO_LEN];
    UINT8 append_len;
    UINT8 append[IF_4G_LD_APPEND_INFO_LEN];
    IF_4G_LD_CTL ctl;
}IF_4G_LD;

typedef void (*IF_4G_SEND_CALLBACK)(UINT8 conn_id, UINT8 result);
typedef void (*IF_4G_RECV_CALLBAKC)(UINT8 conn_id, UINT8 *data, UINT16 len);
typedef void (*IF_4G_TIME_CALLBAK)(IF_4G_TIMINFO *info);
/**
 * when notify_code is FTP_4G_NOTIFY_ERROR, the len is error code
 */
typedef UINT8 (*IF_FTP_4G_DOWNLOAD_CALLBACK)(UINT8 notify_code, UINT8 *data, UINT16 len);
typedef BOOL (*IF_4G_SLEEP_DEPEND_FUN)(void);

typedef enum
{
    IF_FTP_4G_CALLBACK_RET_OK = 0U,
    IF_FTP_4G_CALLBACK_RET_ABORT,
    IF_FTP_4G_CALLBACK_RET_CONTINUE,
}IF_FTP_4G_CALLBACK_RET;

typedef enum
{
    IF_FTP_4G_NOTIFY_PROCESS = 0U,
    IF_FTP_4G_NOTIFY_ERROR,
    IF_FTP_4G_NOTIFY_FINISH
}IF_FTP_4G_NOTIFY_CODE;

typedef enum
{
    IF_FTP_4G_ERR_NO_DIALED = 0U,
    IF_FTP_4G_ERR_INVALID_URL,
    IF_FTP_4G_ERR_BUFF_IS_SMALL,
    IF_FTP_4G_ERR_INVALID_STATE,
    IF_FTP_4G_ERR_DWN_TIMEOUT,
    IF_FTP_4G_ERR_DWN_ERROR
}IF_FTP_4G_ERROR_CODE;

typedef enum
{
    IF_4G_SOCKT_TCP = 0U,
    IF_4G_SOCKT_UDP
}IF_4G_SOCKT_TYPE;

typedef enum
{
    IF_4G_SEND_OK = 0U,
    IF_4G_SEND_NG,
}IF_4G_SEND_RESULT;

typedef enum
{
    IF_4G_SET_APN_OK = 0U,
    IF_4G_SET_APN_NG,
}IF_4G_SETAPN_RESULT;

typedef enum
{
    IF_4G_SIMSTATE_PULLOUT  = 0U,
    IF_4G_SIMSTATE_INSERT,
    IF_4G_SIMSTATE_UNKNOWN
}IF_4G_SIMSTATE;

typedef enum
{
    IF_4G_STATE_DISCONNECTED = 0U,
    IF_4G_STATE_DISCONNECTEING,
    IF_4G_STATE_CONNECTING,
    IF_4G_STATE_CONNECTED
}IF_4G_CONN_STATE;

typedef enum
{
    IF_4G_PUBLIC_APN = 0U,
    IF_4G_PRIVATE_APN,
    IF_4G_OTA_APN
}IF_4G_APN_INDEX;

typedef enum
{
    IF_4G_GBF_CONN_ID = 1U,
    IF_4G_HPM_CONN_ID = 2U,
    IF_4G_CONN_MAX
}IF_4G_CONN_ID;

typedef enum
{
    IF_4G_SLP_TYPE_SLEEP = 0U,
    IF_4G_SLP_TYPE_SHUTDOWN,
    IF_4G_SLP_TYPE_SHELL_SLEEP,
    IF_4G_SLP_TYPE_SHELL_SHUTDOWN
}IF_4G_SLEEP_TYPE;

typedef enum
{
    IF_4G_SEND_PRI_HIGH = 0U,
    IF_4G_SEND_PRI_MID,
    IF_4G_SEND_PRI_LOW
}IF_4G_SEND_PRI;

typedef enum
{
    IF_4G_UNREGISTERED = 0U,
    IF_4G_REGISTERED,
    IF_4G_BUSY,
    IF_4G_DENIED,
    IF_4G_UNKNOWN,
    IF_4G_ROAMING,
} IF_4G_REG_STATE;

typedef enum
{
    TBOX_4G_STATE_SHUTDOWN = 0U,
    TBOX_4G_STATE_STARTING,
    TBOX_4G_STATE_RUNNING,
    TBOX_4G_STATE_STOPPING,
    TBOX_4G_STATE_UNKNOWN
}TBOX_4G_STATE;

INT8 if_4g_get_call_state(UINT8 apn_index);

UINT8 if_4g_get_state(VOID);

VOID if_4g_reset(VOID);

VOID if_4g_set_apn(UINT8 index,
                   UINT8 *apn,
                   UINT8 *username,
                   UINT8 *password);

UINT8 if_4g_reg_transmit_callback(UINT8 conn_id,
                                 IF_4G_SEND_CALLBACK send_callback,
                                 IF_4G_RECV_CALLBAKC recv_callback);

UINT8 if_4g_socket_connect(UINT8 context_id,
                           UINT8 conn_id,
                           UINT8 sockt_type,
                           UINT8 *ip, UINT32 port);

UINT8 if_4g_socket_close(UINT8 conn_id);

UINT8 if_4g_socket_send(UINT8 conn_id, UINT8 *data, UINT16 len);

UINT8 if_4g_socket_send_ex(IF_4G_SEND_PRI priority, UINT8 conn_id, UINT8 *data, UINT16 len);

INT8 if_4g_get_socket_conn_state(INT8 conn_id);

BOOL if_4g_get_imei(UINT8 *imei, UINT8 *len);

BOOL if_4g_get_iccid(UINT8 *iccid, UINT8 *len);

BOOL if_4g_get_phone_num(UINT8 *phone_num, UINT8 *len);

BOOL if_4g_set_phone_num(UINT8 *phone_num);

VOID if_4g_query_phone_num(VOID);

UINT8 if_4g_get_signal_signalstrength(VOID);

UINT8 if_4g_get_sim_state(VOID);

BOOL if_4g_is_downloading(VOID);

VOID if_ftp_4g_download(UINT8 *url, 
                        UINT16 len,
                        UINT8 context_id,
                        IF_FTP_4G_DOWNLOAD_CALLBACK call_bak);

UINT8 if_4g_get_reg_state(VOID);

UINT8 if_4g_get_gprs_state(VOID);

UINT8 if_4g_get_cereg_state(VOID);

UINT8 if_4g_get_temperature(VOID);

UINT16 if_4g_get_testalivefailcount(VOID);

INT32 if_4g_get_chipid(UINT8 *chipid, UINT8 len);

INT32 if_4g_get_fwversion(UINT8 *fwversion, UINT8 len);

VOID if_4g_reg_dependent_fun(IF_4G_SLEEP_DEPEND_FUN fun);

VOID if_4g_cclk(IF_4G_TIME_CALLBAK call_back);

VOID if_4g_ntp(UINT8 context_id, UINT8 *ip, UINT16 port, IF_4G_TIME_CALLBAK call_back);

#endif /* TBOX_4G_IF_H */

