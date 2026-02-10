#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_dial.h"
#include "4g_socket.h"
#include "4g_sequence_mgr.h"
#include "4g_if_inner.h"
#include "4g_ftp.h"
#include "4g_at_transmit.h"
#include "4g_sharememery.h"
#include "4g_mgr.h"

#define FTP_4G_USENAME_LEN 16
#define FTP_4G_PASSWD_LEN  32
#define FTP_4G_DNAME_LEN   32
#define FTP_4G_DIR_LEN     64
#define FTP_4G_FILE_NAME_LEN 64
#define FTP_4G_DOWNLOAD_LEN 2*1024  //2048 byte by once
#define FTP_4G_DOWNLOAD_TIMEOUT  (uint32)(170*1000)/(uint32)PERIODIC_UNIT_4G  //170S
#define FTP_4G_PERIOD_TIMEOUT    (uint32)(500)/(uint32)PERIODIC_UNIT_4G       //500ms

typedef enum
{
    FTP_4G_IDLE = 0,
    FTP_4G_BEGIN_DO_DOWNLOAD,
    FTP_4G_DO_SET_CONTEXTID,
    FTP_4G_WAIRFOR_SET_CONTEXTID_RESP,
    FTP_4G_DO_SET_ACCOUNT,
    FTP_4G_WAIRFOR_SET_ACCOUNT_RESP,
    FTP_4G_DO_SET_FILETYPE,
    FTP_4G_WAIRFOR_SET_FILETYPE_RESP,
    FTP_4G_DO_SET_TRANSMODE,
    FTP_4G_WAIRFOR_SET_TRANSMODE_RESP,
    FTP_4G_DO_SET_RSPTIME,
    FTP_4G_WAIRFOR_SET_RSPTIME_RESP,
    FTP_4G_DO_LOGIN,
    FTP_4G_WAIRFOR_LOGIN_RESP,
    FTP_4G_DO_CWD,
    FTP_4G_WAIRFOR_CWD_RESP,
    FTP_4G_DO_FATCH_FILESIZE,
    FTP_4G_WAIRFOR_FATCH_FILESIZE_RESP,
    FTP_4G_DO_DOWNLOAD,
    FTP_4G_WAIRFOR_DOWNLOAD_RESP,
    FTP_4G_DO_LOGOUT,
    FTP_4G_WAIRFOR_LOGOUT_RESP
}FTP_4G_DOWNLOAD_STATE;

typedef struct
{
    uint8 context_id;
    uint32 port;
    uint8 dname[FTP_4G_DNAME_LEN];
    uint8 user_name[FTP_4G_USENAME_LEN];
    uint8 password[FTP_4G_PASSWD_LEN];
    uint8 dir[FTP_4G_DIR_LEN];
    uint8 file_name[FTP_4G_FILE_NAME_LEN];
    uint32 file_size;
    uint32 has_download_size;
    uint16 dwn_size_once;

    uint8 state;
    IF_FTP_4G_DOWNLOAD_CALLBACK callback;
}FTP_4G_DOWNLOAD_CONTEXT;

static uint8 ftp_4g_set_account(void);

static uint8 ftp_4g_set_context_id(void);

static uint8 ftp_4g_set_binfile(void);

static uint8 ftp_4g_set_passivemode(void);

static uint8 ftp_4g_set_resp_timeout(void);

static uint8 ftp_4g_trans_set_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_login(void);

static uint8 ftp_4g_login_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_cwd(void);

static uint8 ftp_4g_cwd_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_get_filesize(void);

static uint8 ftp_4g_get_filesize_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_get_download(uint32 start_pos, uint16 len);

static uint8 ftp_4g_get_download_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_logout(void);

static uint8 ftp_4g_logout_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_stat_resp(void *context, uint8 *data, uint16 *len);

static uint8 ftp_4g_seq_resp(uint8 cmd, uint8 result);

static uint8 ftp_4g_dwn_data_callback(void *context, uint8 *data, uint16 *len);

static INT8 ftp_4g_parse_url(uint8 *url, uint16 len);

static FTP_4G_DOWNLOAD_CONTEXT ftp_4g_dwn_ctx;

static uint8 ftp_4g_period_value;

static uint8 ftp_4g_logout_retry_login;

static SEQ_4G ftp_4g_seq = {SEQ_4G_IDLE,
                            0,
                            NULL,
                            NULL,
                            ftp_4g_seq_resp
                           };

void ftp_4g_init(void)
{
    ftp_4g_period_value = 0;

    ftp_4g_logout_retry_login = 0;
    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
    ftp_4g_seq.state = SEQ_4G_IDLE;
}

/*
 * URL:ftp://vehicle:Vehicle#*@114.215.188.121:12021/H54N/TEST/H54N_VPU.bin
 */
void ftp_4g_download(uint8 *url, uint16 len,
                     uint8 context_id, IF_FTP_4G_DOWNLOAD_CALLBACK call_bak)
{

    uint8 ret;

    if(DIAL_4G_STATE_CONNECTED != dial_4g_get_callstate(context_id))
    {
        MODULE_LOG_E(TBOX4G, "the apn is no connect");
        if(NULL != call_bak)
        {
            call_bak(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_NO_DIALED);
        }
        return;
    }
    if(FTP_4G_IDLE != ftp_4g_dwn_ctx.state)
    {
        MODULE_LOG_E(TBOX4G, "the state is not idle");
        if(NULL != call_bak)
        {
            call_bak(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_INVALID_STATE);
        }
        return;
    }

    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
    ret = ftp_4g_parse_url(url, len);
    if(ret != 0)
    {
        if(1 == ret)
        {
            MODULE_LOG_E(TBOX4G, "the buffer is small");
            if(NULL != call_bak)
            {
                call_bak(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_BUFF_IS_SMALL);
            }
        }
        else
        {
            MODULE_LOG_E(TBOX4G, "the url is invalid");
            if(NULL != call_bak)
            {
                call_bak(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_INVALID_URL);
            }
        }
        return;
    }

    at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
    ftp_4g_period_value = 0;
    ftp_4g_logout_retry_login = 0;
    ftp_4g_dwn_ctx.context_id = context_id;
    ftp_4g_dwn_ctx.callback = call_bak;
    ftp_4g_dwn_ctx.state = FTP_4G_BEGIN_DO_DOWNLOAD;
    ftp_4g_dwn_ctx.file_size = ftp_4g_dwn_ctx.has_download_size = 0;
    ftp_4g_dwn_ctx.dwn_size_once = 0;
}

void ftp_4g_period(void)
{
    if(ftp_4g_period_value > 0)
    {
        ftp_4g_period_value--;
    }
    if(ftp_4g_period_value > 0)
    {
        return;
    }

    switch(ftp_4g_dwn_ctx.state)
    {
        case FTP_4G_BEGIN_DO_DOWNLOAD:
            ftp_4g_seq.state = SEQ_4G_IDLE;
            ftp_4g_seq.timeout = FTP_4G_DOWNLOAD_TIMEOUT;
            if(0 != seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &ftp_4g_seq, SEQ_4G_ABORT))
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_DO_SET_CONTEXTID;
            }
            break;

        case FTP_4G_DO_SET_CONTEXTID:
            if(0 != ftp_4g_set_context_id())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_SET_CONTEXTID_RESP;
            }
            break;

        case FTP_4G_DO_SET_ACCOUNT:
            if(0 != ftp_4g_set_account())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_SET_ACCOUNT_RESP;
            }
            break;

        case FTP_4G_DO_SET_FILETYPE:
            if(0 != ftp_4g_set_binfile())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_SET_FILETYPE_RESP;
            }
            break;

        case FTP_4G_DO_SET_TRANSMODE:
            if(0 != ftp_4g_set_passivemode())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_SET_TRANSMODE_RESP;
            }
            break;

        case FTP_4G_DO_SET_RSPTIME:
            if(0 != ftp_4g_set_resp_timeout())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_SET_RSPTIME_RESP;
            }
            break;

        case FTP_4G_DO_LOGIN:
            if(0 != ftp_4g_login())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_LOGIN_RESP;
            }
            break;

        case FTP_4G_DO_CWD:
            if(0 != ftp_4g_cwd())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_CWD_RESP;
            }
            break;

        case FTP_4G_DO_FATCH_FILESIZE:
            if(0 != ftp_4g_get_filesize())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_FATCH_FILESIZE_RESP;
            }
            break;

        case FTP_4G_DO_DOWNLOAD:
            {
                if(ftp_4g_dwn_ctx.file_size <=  ftp_4g_dwn_ctx.has_download_size)
                {
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
                    break;
                }

                if((ftp_4g_dwn_ctx.file_size - ftp_4g_dwn_ctx.has_download_size) > FTP_4G_DOWNLOAD_LEN)
                {
                    ftp_4g_dwn_ctx.dwn_size_once = FTP_4G_DOWNLOAD_LEN;
                }
                else
                {
                    ftp_4g_dwn_ctx.dwn_size_once = (ftp_4g_dwn_ctx.file_size - ftp_4g_dwn_ctx.has_download_size);
                }

                MODULE_LOG_I(TBOX4G, "download data, has_download_size:%d dwn_size_once:%d",
                        ftp_4g_dwn_ctx.has_download_size, ftp_4g_dwn_ctx.dwn_size_once);

                if(0 != ftp_4g_get_download(ftp_4g_dwn_ctx.has_download_size, ftp_4g_dwn_ctx.dwn_size_once))
                {
                    ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
                }
                else
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_DOWNLOAD_RESP;
                }
            }
            break;

        case FTP_4G_DO_LOGOUT:
            if(0 != ftp_4g_logout())
            {
                ftp_4g_period_value = FTP_4G_PERIOD_TIMEOUT;
            }
            else
            {
                ftp_4g_dwn_ctx.state = FTP_4G_WAIRFOR_LOGOUT_RESP;
            }
            break;

        default:
            break;
    }
}

boolean ftp_4g_isdownloading(void)
{
    return (ftp_4g_dwn_ctx.state != FTP_4G_IDLE) ? TRUE : FALSE;
}

static uint8 ftp_4g_set_account(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_128BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr,127 ,"AT+QFTPCFG=\"account\",\"%s\",\"%s\"" AT_4G_REQ_SUFFIX,
                       ftp_4g_dwn_ctx.user_name, ftp_4g_dwn_ctx.password);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_TRANS_SET;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_trans_set_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_set_context_id(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31, "AT+QFTPCFG=\"contextid\",%d" AT_4G_REQ_SUFFIX,
                       ftp_4g_dwn_ctx.context_id+1);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_TRANS_SET;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_trans_set_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_set_binfile(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr,31, "AT+QFTPCFG=\"filetype\",0" AT_4G_REQ_SUFFIX);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_TRANS_SET;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_trans_set_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_set_passivemode(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31,"AT+QFTPCFG=\"transmode\",1" AT_4G_REQ_SUFFIX);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_TRANS_SET;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_trans_set_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_set_resp_timeout(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 31, "AT+QFTPCFG=\"rsptimeout\",20" AT_4G_REQ_SUFFIX);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_TRANS_SET;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_trans_set_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_trans_set_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_FTP_TRANS_SET != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QFTPCFG: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_TRANS_SET, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_OK == resp->resp_code)
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_TRANS_SET, SEQMGR_4G_CMD_EXE_OK);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_TRANS_SET, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 ftp_4g_login(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 63, "AT+QFTPOPEN=\"%s\",%d" AT_4G_REQ_SUFFIX,
                        ftp_4g_dwn_ctx.dname,
                        ftp_4g_dwn_ctx.port);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_LOGIN;
    cmd.retry_count = 3;
    cmd.tick = 20000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_login_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_login_resp(void *context, uint8 *data, uint16 *len)
{
     AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
     uint8 *token;
     uint8 *temp_ptr;
     uint16 temp_len = 0;
     uint32 temp_var;

     if(AT_4G_FTP_LOGIN != resp->cmd.cmd_id)
     {
        return AT_4G_DECODE_NOMATCH;
     }

     if(AT_4G_RESP_UNKNOWN == resp->resp_code ||
        AT_4G_RESP_OK == resp->resp_code)
     {
         temp_len = *len;
         token = tbox_string_get_substring(data, temp_len, "+QFTPOPEN: ");
         if(NULL == token)
         {
             return AT_4G_DECODE_NOMATCH;
         }
         temp_ptr = token;
         temp_len = temp_len - (temp_ptr-data);
         token = tbox_string_get_substring(temp_ptr, temp_len, "\r\n");
         if(NULL == token)
         {
             return AT_4G_DECODE_CONTINUE;
         }

         if(FALSE == tbox_string_get_num(temp_ptr+strlen("+QFTPOPEN: "), ',', &temp_var))
         {
             return AT_4G_DECODE_INVALID_PACKET;
         }

         if(0 == temp_var)
         {
             seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_LOGIN, SEQMGR_4G_CMD_EXE_OK);
         }
         else
         {
             seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_LOGIN, SEQMGR_4G_CMD_EXE_FAILED);
         }

         *len = (uint16)(token-data)+strlen("\r\n");
         return AT_4G_DECODE_ISMATCH;
     }
     else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
             AT_4G_RESP_ABORT == resp->resp_code ||
             (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
              resp->resp_code <= AT_4G_RESP_MAX_ERROR))
     {
         seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_LOGIN, SEQMGR_4G_CMD_EXE_FAILED);
         return AT_4G_DECODE_ISMATCH;
     }
     else
     {
         /**/
     }

     return AT_4G_DECODE_NOMATCH;
}

static uint8 ftp_4g_cwd(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_128BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 127, "AT+QFTPCWD=\"%s\"" AT_4G_REQ_SUFFIX, ftp_4g_dwn_ctx.dir);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_CWD;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_cwd_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_cwd_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
     uint8 *token;
     uint8 *temp_ptr;
     uint16 temp_len = 0;
     uint32 temp_var;

     if(AT_4G_FTP_CWD != resp->cmd.cmd_id)
     {
        return AT_4G_DECODE_NOMATCH;
     }

     if(AT_4G_RESP_UNKNOWN == resp->resp_code ||
        AT_4G_RESP_OK == resp->resp_code)
     {
         temp_len = *len;
         token = tbox_string_get_substring(data, temp_len, "+QFTPCWD: ");
         if(NULL == token)
         {
             return AT_4G_DECODE_NOMATCH;
         }
         temp_ptr = token;
         temp_len = temp_len - (temp_ptr-data);
         token = tbox_string_get_substring(temp_ptr, temp_len, "\r\n");
         if(NULL == token)
         {
             return AT_4G_DECODE_CONTINUE;
         }

         if(FALSE == tbox_string_get_num(temp_ptr+strlen("+QFTPCWD: "), ',', &temp_var))
         {
             return AT_4G_DECODE_INVALID_PACKET;
         }

         if(0 == temp_var)
         {
             seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_CWD, SEQMGR_4G_CMD_EXE_OK);
         }
         else
         {
             seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_CWD, SEQMGR_4G_CMD_EXE_FAILED);
         }

         *len = (uint16)(token-data)+strlen("\r\n");
         return AT_4G_DECODE_ISMATCH;
     }
     else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
             AT_4G_RESP_ABORT == resp->resp_code ||
             (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
              resp->resp_code <= AT_4G_RESP_MAX_ERROR))
     {
         seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_CWD, SEQMGR_4G_CMD_EXE_FAILED);
         return AT_4G_DECODE_ISMATCH;
     }
     else
     {
         /**/
     }

     return AT_4G_DECODE_NOMATCH;
}

static uint8 ftp_4g_get_filesize(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_128BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 127, "AT+QFTPSIZE=\"%s\"" AT_4G_REQ_SUFFIX, ftp_4g_dwn_ctx.file_name);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_GET_SIZE;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_get_filesize_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_get_filesize_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;
    uint32 temp_value;

    if(AT_4G_FTP_GET_SIZE != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code ||
       AT_4G_RESP_OK == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QFTPSIZE: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        temp_len = temp_len-(temp-data);
        token = tbox_string_get_substring(temp, temp_len, "\r\n");
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        *len = (uint16)(token-data)+strlen("\r\n");

        temp_value = 0;
        if(FALSE == tbox_string_get_num(temp+strlen("+QFTPSIZE: "), ',', &temp_value))
        {
            MODULE_LOG_E(TBOX4G, "failed to get code");
            return AT_4G_DECODE_INVALID_PACKET;
        }

        if(0 != temp_value)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_GET_SIZE, SEQMGR_4G_CMD_EXE_FAILED);
            return AT_4G_DECODE_ISMATCH;
        }

        token = tbox_string_get_substring(temp, temp_len, ",");
        if(NULL == token)
        {
            MODULE_LOG_E(TBOX4G, "failed to get size");
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_value = 0;
        if(FALSE == tbox_string_get_num(token+1, '\r', &temp_value))
        {
            MODULE_LOG_E(TBOX4G, "the size is invalid");
            return AT_4G_DECODE_INVALID_PACKET;
        }

        MODULE_LOG_I(TBOX4G, "the file size:%d", temp_value);

        ftp_4g_dwn_ctx.file_size = temp_value;

        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_GET_SIZE, SEQMGR_4G_CMD_EXE_OK);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_GET_SIZE, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 ftp_4g_get_download(uint32 start_pos, uint16 len)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_128BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 127, "AT+QFTPGET=\"%s\",\"COM:\",%d,%d" AT_4G_REQ_SUFFIX, ftp_4g_dwn_ctx.file_name,
                        start_pos, len);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_DOWNLOAD;
    cmd.retry_count = 3;
    cmd.tick = 25*1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_get_download_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_get_download_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_FTP_DOWNLOAD != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "CONNECT\r\n");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }

        *len = (uint16)(token-data)+strlen("CONNECT\r\n");

        MODULE_LOG_I(TBOX4G, "begin download data, size:%d", ftp_4g_dwn_ctx.dwn_size_once);
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_DOWNLOAD, SEQMGR_4G_CMD_EXE_OK);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_CONNECT == resp->resp_code)
    {
        MODULE_LOG_I(TBOX4G, "begin download data, size:%d", ftp_4g_dwn_ctx.dwn_size_once);
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_DOWNLOAD, SEQMGR_4G_CMD_EXE_OK);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_DOWNLOAD, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 ftp_4g_logout(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 15, "AT+QFTPCLOSE" AT_4G_REQ_SUFFIX);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_QUIT;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_logout_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_logout_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_FTP_QUIT, (AT_4G_CMD_RESP *)context, data, len);
}

__attribute__((unused)) uint8 ftp_4g_stat(void)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((CHAR*)ele->mem_ptr, 15, "AT+QFTPSTAT\r\n");
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_FTP_STAT;
    cmd.retry_count = 3;
    cmd.tick = 2000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = ftp_4g_stat_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 ftp_4g_stat_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;
    uint32 temp_value;

    if(AT_4G_FTP_STAT != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code ||
       AT_4G_RESP_OK == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QFTPSTAT: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        temp_len = temp_len-(temp-data);
        token = tbox_string_get_substring(temp, temp_len, "\r\n");
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        *len = (uint16)(token-data)+strlen("\r\n");

        temp_value = 0;
        if(FALSE == tbox_string_get_num(temp+strlen("+QFTPSTAT: "), ',', &temp_value))
        {
           MODULE_LOG_E(TBOX4G, "failed to get code");
            return AT_4G_DECODE_INVALID_PACKET;
        }

        if(0 != temp_value)
        {
            seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_STAT, SEQMGR_4G_CMD_EXE_FAILED);
            return AT_4G_DECODE_ISMATCH;
        }

        token = tbox_string_get_substring(temp, temp_len, ",");
        if(NULL == token)
        {
            MODULE_LOG_E(TBOX4G, "failed to get size");
            return AT_4G_DECODE_INVALID_PACKET;
        }
        temp_value = 0;
        if(FALSE == tbox_string_get_num(token+1, '\r', &temp_value))
        {
            MODULE_LOG_E(TBOX4G, "the stat is invalid");
            return AT_4G_DECODE_INVALID_PACKET;
        }

        MODULE_LOG_I(TBOX4G, "the stat size:%d", temp_value);

        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_STAT, SEQMGR_4G_CMD_EXE_OK);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_FTP_STAT, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 ftp_4g_seq_resp(uint8 cmd, uint8 result)
{
    uint8 ret = SEQ_4G_CMD_NOMATCH;

    MODULE_LOG_E(TBOX4G, "recv info, cmd:%d result:%d state:%d",
                   cmd, result, ftp_4g_dwn_ctx.state);

    if(SEQMGR_4G_CMD_EXE_TIMEOUT == result ||
       SEQMGR_4G_CMD_EXE_ABORT == result ||
       SEQMGR_4G_CMD_EXE_FAILED == result)
    {
        if(SEQMGR_4G_CMD_EXE_FAILED == result)
        {
           MODULE_LOG_E(TBOX4G, "download failed,res:%d",(int)result);
            if(NULL != ftp_4g_dwn_ctx.callback)
            {
                ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
            }
        }
        else
        {
            MODULE_LOG_E(TBOX4G, "download timeout,res:%d",(int)result);
            if(NULL != ftp_4g_dwn_ctx.callback)
            {
                ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_TIMEOUT);
            }
        }

        ftp_4g_logout();
        memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
        at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
        ftp_4g_init();
        return SEQ_4G_CMD_FINISH;
    }

    switch(ftp_4g_dwn_ctx.state)
    {
        case FTP_4G_WAIRFOR_SET_CONTEXTID_RESP:
            if(AT_4G_FTP_TRANS_SET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_SET_ACCOUNT;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to set context id");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case FTP_4G_WAIRFOR_SET_ACCOUNT_RESP:
            if(AT_4G_FTP_TRANS_SET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_SET_FILETYPE;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to set account");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case FTP_4G_WAIRFOR_SET_FILETYPE_RESP:
            if(AT_4G_FTP_TRANS_SET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_SET_TRANSMODE;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to set file type");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case FTP_4G_WAIRFOR_SET_TRANSMODE_RESP:
            if(AT_4G_FTP_TRANS_SET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_SET_RSPTIME;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to set tansmode");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case FTP_4G_WAIRFOR_SET_RSPTIME_RESP:
            if(AT_4G_FTP_TRANS_SET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGIN;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to set rsp timeout");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case FTP_4G_WAIRFOR_LOGIN_RESP:
            if(AT_4G_FTP_LOGIN == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_CWD;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else
                {
                    if(0 == ftp_4g_logout_retry_login)
                    {
                        ftp_4g_logout_retry_login = 1;
                        ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
                        ret = SEQ_4G_CMD_ISMATCH;
                    }
                    else
                    {
                        MODULE_LOG_E(TBOX4G, "failed to login");
                        ftp_4g_logout_retry_login = 0;
                        if(NULL != ftp_4g_dwn_ctx.callback)
                        {
                            ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                        }
                        memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                        ret = SEQ_4G_CMD_FINISH;
                    }
                }
            }
            break;

        case FTP_4G_WAIRFOR_CWD_RESP:
            if(AT_4G_FTP_CWD == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_FATCH_FILESIZE;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to cwd");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
                }

                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case FTP_4G_WAIRFOR_FATCH_FILESIZE_RESP:
            if(AT_4G_FTP_GET_SIZE == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    if(0 == ftp_4g_dwn_ctx.file_size)
                    {
                        MODULE_LOG_E(TBOX4G, "file size is 0");
                        if(NULL != ftp_4g_dwn_ctx.callback)
                        {
                            ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                        }
                        ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
                    }
                    else
                    {
                        ftp_4g_dwn_ctx.has_download_size = 0;
                        ftp_4g_dwn_ctx.dwn_size_once = 0;
                        ftp_4g_dwn_ctx.state = FTP_4G_DO_DOWNLOAD;
                    }
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to fatch file size");
                    if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }

                    ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case FTP_4G_WAIRFOR_DOWNLOAD_RESP:
            if(AT_4G_FTP_DOWNLOAD == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, ftp_4g_dwn_data_callback);
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed to download, retry");
                    ftp_4g_dwn_ctx.dwn_size_once = 0;
                    at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_DOWNLOAD;

                   /* if(NULL != ftp_4g_dwn_ctx.callback)
                    {
                        ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                    }

                    ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;*/
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case FTP_4G_WAIRFOR_LOGOUT_RESP:
            if(AT_4G_FTP_QUIT == cmd)
            {
                if(0 == ftp_4g_logout_retry_login)
                {
                    memset(&ftp_4g_dwn_ctx, 0, sizeof(ftp_4g_dwn_ctx));
                    at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
                    ret = SEQ_4G_CMD_FINISH;
                }
                else
                {
                    ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGIN;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

static char* find_substring(const char* str, int len, const char* substr) 
{
    int substr_len = strlen(substr);
    for (int i = 0; i <= len - substr_len; i++) 
    {
        if (strncmp(str + i, substr, substr_len) == 0)
        {
            return (char*)(str + i);
        }
    }
    return NULL;
}

static uint8 ftp_4g_dwn_data_callback(void *context, uint8 *data, uint16 *len)
{
    uint8 *token = data;
    uint16 temp_len = *len;
    uint8 temp_buff[64] = "\0";

    UNUSED(context);

    if(FTP_4G_WAIRFOR_DOWNLOAD_RESP != ftp_4g_dwn_ctx.state)
    {
        return AT_4G_DECODE_ISMATCH;
    }
	
    MODULE_LOG_I(TBOX4G, "recv_len:%d has_download:%d file_size:%d \r\n", temp_len, ftp_4g_dwn_ctx.has_download_size,
           ftp_4g_dwn_ctx.file_size);

    snprintf((char*)temp_buff, sizeof(temp_buff)-1, "\r\nOK\r\n\r\n+QFTPGET: 0,%d\r\n", ftp_4g_dwn_ctx.dwn_size_once);
    if(temp_len >= (ftp_4g_dwn_ctx.dwn_size_once+strlen((char*)temp_buff)))
    {
    	 if(temp_len > (ftp_4g_dwn_ctx.dwn_size_once+strlen((char*)temp_buff)))
        {
            uint8 temp_buff_abnormal[32] = "\0";

			token[temp_len] = 0;
            snprintf((char*)temp_buff_abnormal, sizeof(temp_buff_abnormal)-1, "\r\n+QFTPGET: 0,%d\r\n", ftp_4g_dwn_ctx.dwn_size_once);
            MODULE_LOG_E(TBOX4G, "the download data len is overflow, try to remove unexpected string!(%s)", (char*)temp_buff_abnormal);

            if(NULL != find_substring((char *)token, temp_len,((char*)temp_buff_abnormal)))
            {
                temp_len = ftp_4g_dwn_ctx.dwn_size_once;
				MODULE_LOG_E(TBOX4G, "fix download data len");
            }
        }
        else 
        {
	        if(0 != tbox_string_cmp_string_from_end(token, temp_len, (char*)temp_buff))
	        {
	            MODULE_LOG_E(TBOX4G, "the download data is not match, retry");
	            ftp_4g_dwn_ctx.dwn_size_once = 0;
	            at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
	            ftp_4g_dwn_ctx.state = FTP_4G_DO_DOWNLOAD;
	            return AT_4G_DECODE_ISMATCH;
	        }
	        temp_len = temp_len - strlen((char*)temp_buff);
        }

        if(temp_len != ftp_4g_dwn_ctx.dwn_size_once)
        {
            MODULE_LOG_E(TBOX4G, "the download data is invalid, retry");

            ftp_4g_dwn_ctx.dwn_size_once = 0;
            at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
            ftp_4g_dwn_ctx.state = FTP_4G_DO_DOWNLOAD;
            return AT_4G_DECODE_ISMATCH;
        }

        MODULE_LOG_I(TBOX4G, "len is oklen:%d", temp_len);

        ftp_4g_dwn_ctx.dwn_size_once = 0;
        at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
        ftp_4g_dwn_ctx.state = FTP_4G_DO_DOWNLOAD;
    }
    else
    {
        if(temp_len >= strlen((char*)temp_buff))
        {
            if(0 == tbox_string_cmp_string_from_end(token, temp_len, (char*)temp_buff))
            {
                MODULE_LOG_E(TBOX4G, "the download data length:%d is invalid, retry", temp_len);

                ftp_4g_dwn_ctx.dwn_size_once = 0;
                at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
                ftp_4g_dwn_ctx.state = FTP_4G_DO_DOWNLOAD;
                return AT_4G_DECODE_ISMATCH;
            }
            else
            {
                return AT_4G_DECODE_CONTINUE;
            }
        }
        else
        {
            return AT_4G_DECODE_CONTINUE;
        }
    }

    if((ftp_4g_dwn_ctx.has_download_size + temp_len) < ftp_4g_dwn_ctx.file_size)
    {
        ftp_4g_dwn_ctx.has_download_size += temp_len;
        if(NULL != ftp_4g_dwn_ctx.callback)
        {
            if(IF_FTP_4G_CALLBACK_RET_ABORT == ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_PROCESS, token, temp_len))
            {
                MODULE_LOG_E(TBOX4G, "callback abort download");

                if(NULL != ftp_4g_dwn_ctx.callback)
                {
                    ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_ERROR, NULL, IF_FTP_4G_ERR_DWN_ERROR);
                }
                ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
                at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
                return AT_4G_DECODE_ISMATCH;
            }
        }
    }
    else
    {
        temp_len = ftp_4g_dwn_ctx.file_size - ftp_4g_dwn_ctx.has_download_size;
        ftp_4g_dwn_ctx.has_download_size = ftp_4g_dwn_ctx.file_size;
        if(temp_len > 0)
        {
            if(NULL != ftp_4g_dwn_ctx.callback)
            {
                ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_PROCESS, token, temp_len);
            }
        }
        if(NULL != ftp_4g_dwn_ctx.callback)
        {
            ftp_4g_dwn_ctx.callback(IF_FTP_4G_NOTIFY_FINISH, NULL, 0);
        }
        ftp_4g_dwn_ctx.state = FTP_4G_DO_LOGOUT;
        at_4g_transmit_reg_callback(AT_4G_DECODE_FILTER, NULL);
    }

    return AT_4G_DECODE_ISMATCH;
}

/*
 * URL:ftp://vehicle:Vehicle#*@114.215.188.121:12021/H54N/TEST/H54N_VPU.bin
 */
static INT8  ftp_4g_parse_url(uint8 *url, uint16 len)
{
    uint8 *begin_pos;
    uint8 *end_pos;
    uint8 *token;
    uint8 index;
    uint16 temp_len;
    uint32 temp_var;

    if(strncmp((char*)url, "ftp", 3) != 0 &&
       strncmp((char*)url, "FTP", 3) != 0)
    {
        return -1;
    }

    temp_len = len;
    token = tbox_string_get_substring(url, temp_len, "//");
    if(NULL == token)
    {
        return -1;
    }
    begin_pos = token + strlen("//");
    if(begin_pos-url >= temp_len)
    {
        return -1;
    }
    temp_len = temp_len - (begin_pos-url);
    token = tbox_string_get_substring(begin_pos, temp_len, ":");
    if(NULL == token)
    {
        return -1;
    }
    end_pos = token;
    if(begin_pos >= end_pos)
    {
        return -1;
    }
    if((uint16)(end_pos - begin_pos) >= sizeof(ftp_4g_dwn_ctx.user_name))
    {
        return 1;
    }
    memcpy(ftp_4g_dwn_ctx.user_name, begin_pos, (end_pos-begin_pos));

    begin_pos = end_pos + 1;
    if(begin_pos-url > len)
    {
        return -1;
    }
    temp_len = len-(begin_pos-url);
    token = tbox_string_get_substring(begin_pos, temp_len, "@");
    if(NULL == token)
    {
        return -1;
    }
    end_pos = token;
    if((uint16)(end_pos - begin_pos) >= sizeof(ftp_4g_dwn_ctx.password))
    {
        return 1;
    }
    memcpy(ftp_4g_dwn_ctx.password, begin_pos, (end_pos-begin_pos));

    token = end_pos + 1;
    memset(ftp_4g_dwn_ctx.dname, 0, sizeof(ftp_4g_dwn_ctx.dname));
    for(index = 0; index < FTP_4G_DNAME_LEN; index++)
    {
        if(*token == ':')
        {
            break;
        }
        ftp_4g_dwn_ctx.dname[index] = *token;
        token++;
    }
    if(index >= FTP_4G_DNAME_LEN)
    {
        return -1;
    }

    begin_pos = token + 1;
    if(begin_pos-url > len)
    {
        return -1;
    }
    temp_len = len-(begin_pos-url);
    token = tbox_string_get_substring(begin_pos, temp_len, "/");
    if(NULL == token)
    {
        return -1;
    }
    if(FALSE == tbox_string_get_num(begin_pos, '/', &temp_var))
    {
        return -1;
    }
    end_pos = token;
    ftp_4g_dwn_ctx.port = temp_var;

    begin_pos = end_pos;
    if(begin_pos-url > len)
    {
        return -1;
    }
    temp_len = len;
    while(temp_len-- > 0)
    {
        if(url[temp_len] == '/')
        {
            break;
        }
    }
    if(temp_len == 0)
    {
        return -1;
    }
    end_pos = &url[temp_len];

    if(end_pos < begin_pos)
    {
        return -1;
    }
    else if(begin_pos == end_pos)
    {
        ftp_4g_dwn_ctx.dir[0] = '/';
    }
    else
    {
        temp_len = end_pos-begin_pos+1;
        if(temp_len >= sizeof(ftp_4g_dwn_ctx.dir))
        {
            return 1;
        }
        memcpy(ftp_4g_dwn_ctx.dir, begin_pos, temp_len);
    }

    begin_pos = end_pos + 1;
    if(begin_pos-url > len)
    {
        return -1;
    }
    temp_len = len-(begin_pos-url);
    if(temp_len >= sizeof(ftp_4g_dwn_ctx.file_name))
    {
        return 1;
    }
    memcpy(ftp_4g_dwn_ctx.file_name, begin_pos, temp_len);

    return 0;
}
