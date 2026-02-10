#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_sms.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"

SMS_4G_INFO sms_4g_info;

static uint8 sms_4g_query_centernum_resp(void *context, uint8 *data, uint16 *len);

static uint8 sms_4g_set_centernum_resp(void *context, uint8 *data, uint16 *len);

static uint8 sms_4g_set_pdumode_resp(void *context, uint8 *data, uint16 *len);

static uint8 sms_4g_set_txtmode_resp(void *context, uint8 *data, uint16 *len);

static uint8 sms_4g_send_sms_len_resp(void *context, uint8 *data, uint16 *len);

static uint8 sms_4g_send_sms_data_resp(void *context, uint8 *data, uint16 *len);

static uint8 sms_4g_del_all_resp(void *context, uint8 *data, uint16 *len);

void sms_4g_init(void)
{
    sms_4g_info.sms_mode = SMS_4G_MODE_UNKNOWN;
    sms_4g_info.centernum_valid = FALSE;
    memset(sms_4g_info.centernum, 0, sizeof(sms_4g_info.centernum));
}

uint8 sms_4g_query_centernum(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;
    char *at_str = "AT+CSCA?" AT_4G_REQ_SUFFIX;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_CENTERNUM;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_query_centernum_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sms_4g_set_centernum(AT_4G_CMD_PRIORITY pri, uint8 *num)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((char*)ele->mem_ptr, 31,"AT+CSCA=\"%s\"\r\n", (char*)num);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_SET_CENTERNUM;
    cmd.retry_count = 0;
    cmd.tick = 300;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_set_centernum_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sms_4g_set_pdumode(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;
    char *at_str = "AT+CMGF=0" AT_4G_REQ_SUFFIX;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_SMS_SETPDUMODE;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_set_pdumode_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sms_4g_set_txtmode(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;
    char *at_str = "AT+CMGF=1\r\n";

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_SMS_SETTXTMODE;
    cmd.retry_count = 0;
    cmd.tick = 300;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_set_txtmode_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sms_4g_send_sms_len(AT_4G_CMD_PRIORITY pri, uint8 len)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }
    
    snprintf((char*)ele->mem_ptr, 31, "AT+CMGS=%d\r\n", len);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_SMS_SENDLEN;
    cmd.retry_count = 0;
    cmd.tick = 500;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_send_sms_len_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sms_4g_send_sms_data(uint8 *data, uint8 len)
{
    AT_4G_CMD cmd;

    cmd.cmd_id = AT_4G_SMS_SENDDATA;
    cmd.retry_count = 0;
    cmd.tick = 500;
    cmd.sharm_index = -1;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_send_sms_data_resp;
    if(AT_4G_TRANS_SEND_OK != at_4g_transmit_direct_setcmd(&cmd))
    {
        return 1;
    }
    at_4g_transmit_direct_send(data, len, NULL);

    return 0;
}

uint8 sms_4g_del_all(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;
    char *at_str = "AT+CMGD=1,4\r\n";

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_SMS_DELALL;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sms_4g_del_all_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

static uint8 sms_4g_query_centernum_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint8 count = 0;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_CENTERNUM != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CSCA: \"");
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

        memset(sms_4g_info.centernum, 0, sizeof(sms_4g_info.centernum));
        temp = temp + strlen("+CSCA: \"");
        while(*temp != '\"')
        {
            sms_4g_info.centernum[count++] = *temp;
            temp++;
            if(count >= SMS_4G_CENTERNUM_MAX)
            {
                return AT_4G_DECODE_INVALID_PACKET;
            }
        }
        if(count > 0)
        {
            sms_4g_info.centernum_valid = TRUE;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_CENTERNUM, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        sms_4g_info.centernum_valid = FALSE;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_CENTERNUM, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sms_4g_set_centernum_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_SET_CENTERNUM, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 sms_4g_set_pdumode_resp(void *context, uint8 *data, uint16 *len)
{
    if(AT_4G_RESP_OK == ((AT_4G_CMD_RESP *)context)->resp_code)
    {
        sms_4g_info.sms_mode = SMS_4G_MODE_PDU;
    }
    return at_4g_common_resp(AT_4G_SMS_SETPDUMODE, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 sms_4g_set_txtmode_resp(void *context, uint8 *data, uint16 *len)
{
    if(AT_4G_RESP_OK == ((AT_4G_CMD_RESP *)context)->resp_code)
    {
        sms_4g_info.sms_mode = SMS_4G_MODE_TEXT;
    }
    return at_4g_common_resp(AT_4G_SMS_SETTXTMODE, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 sms_4g_send_sms_len_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_SMS_SENDLEN != resp->cmd.cmd_id)
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

        seqmgr_4g_handle_cmd_exe_result(AT_4G_SMS_SENDLEN, SEQMGR_4G_CMD_EXE_OK);

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
        seqmgr_4g_handle_cmd_exe_result(AT_4G_SMS_SENDLEN, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sms_4g_send_sms_data_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_SMS_SENDDATA != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CMGS: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_SMS_SENDDATA, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_SMS_SENDDATA, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sms_4g_del_all_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_SMS_DELALL, (AT_4G_CMD_RESP *)context, data, len);
}
