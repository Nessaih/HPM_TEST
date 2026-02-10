#include "4g_depend_header.h"
#include "4g_if.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_time.h"
#include "4g_dial.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"

static IF_4G_TIME_CALLBAK time_4g_cclk_callback;

static IF_4G_TIME_CALLBAK time_4g_ntp_callback;

static uint8 time_4g_cclk_resp(void *context, uint8 *data, uint16 *len);

static uint8 time_4g_ntp_resp(void *context, uint8 *data, uint16 *len);

void time_4g_init(void)
{
    time_4g_cclk_callback = NULL;
    time_4g_ntp_callback = NULL;
}

uint8 time_4g_cclk(AT_4G_CMD_PRIORITY pri, IF_4G_TIME_CALLBAK call_back)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;
    char *at_str = "AT+CCLK?" AT_4G_REQ_SUFFIX;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_TIM_CCLK;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = time_4g_cclk_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }
    time_4g_cclk_callback = call_back;

    return 0;
}

uint8 time_4g_cclk_settime(AT_4G_CMD_PRIORITY pri, uint8 *time)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((char*)ele->mem_ptr, 63, "AT+CCLK=\"%s\"\r\n", (char*)time);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_TIM_CCLK_SETTIME;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_NORESP;
    cmd.resp = NULL;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 time_4g_ntp(AT_4G_CMD_PRIORITY pri, uint8 context_id, uint8 *ip, uint16 port, IF_4G_TIME_CALLBAK call_back)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    sharemem_4g_ele *ele = NULL;

    if(IF_4G_STATE_CONNECTED != dial_4g_get_callstate(context_id))
    {
        MODULE_LOG_E(TBOX4G, "the context is not connect");
        return 1;
    }

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_128BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = 128;
    memset(ele->mem_ptr, 0, ele->len);
    snprintf((CHAR*)ele->mem_ptr, 127, "AT+QNTP=%d,\"%s\",%d,1" AT_4G_REQ_SUFFIX, context_id+1, (CHAR*)ip, port);
    ele->len = strlen((CHAR*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_TIM_NTP;
    cmd.retry_count = 0;
    cmd.tick = 10000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = time_4g_ntp_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }
    time_4g_ntp_callback = call_back;

    return 0;
}

static uint8 time_4g_cclk_resp(void *context, uint8 *data, uint16 *len)
{
     AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
     uint8 *temp;
     uint8 *token;
     IF_4G_TIMINFO time_info;
     uint16 temp_len = 0;
     int temp_year, temp_mon, temp_day, temp_hour, temp_min, temp_sec;

     time_info.is_valid = 0;
     memset(time_info.time, 0, sizeof(time_info.time));

     if(AT_4G_TIM_CCLK != resp->cmd.cmd_id)
     {
        return AT_4G_DECODE_NOMATCH;
     }

     if(AT_4G_RESP_UNKNOWN == resp->resp_code)
     {
         temp_len = *len;
         token = tbox_string_get_substring(data, temp_len, "+CCLK: ");
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

         if(6 != sscanf((CHAR*)temp, "+CCLK: \"%02d/%02d/%02d,%02d:%02d:%02d",
                         &temp_year,
                         &temp_mon,
                         &temp_day,
                         &temp_hour,
                         &temp_min,
                         &temp_sec))
         {
             return AT_4G_DECODE_INVALID_PACKET;
         }
         time_info.time[0] = temp_year;
         time_info.time[1] = temp_mon;
         time_info.time[2] = temp_day;
         time_info.time[3] = temp_hour;
         time_info.time[4] = temp_min;
         time_info.time[5] = temp_sec;

         /*TODO 通过配置项配置正确的时区*/
         //utc_to_local(time_info.time, 8);

         MODULE_LOG_I(TBOX4G, "cclk time:%d:%d:%d %d:%d:%d",
            temp_year, temp_mon, temp_day, temp_hour, temp_min, temp_sec);

         time_info.is_valid = 1;
         if(NULL != time_4g_cclk_callback)
         {
             time_4g_cclk_callback(&time_info);
         }

         seqmgr_4g_handle_cmd_exe_result(AT_4G_TIM_CCLK, SEQMGR_4G_CMD_EXE_OK);

         *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
         return AT_4G_DECODE_ISMATCH;
     }
     else if(AT_4G_RESP_OK == resp->resp_code)
     {
         if(NULL != time_4g_cclk_callback)
         {
             time_4g_cclk_callback(&time_info);
         }

         seqmgr_4g_handle_cmd_exe_result(AT_4G_TIM_CCLK, SEQMGR_4G_CMD_EXE_OK);
         return AT_4G_DECODE_ISMATCH;
     }
     else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
             AT_4G_RESP_ABORT == resp->resp_code ||
             (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
              resp->resp_code <= AT_4G_RESP_MAX_ERROR))
     {
         if(NULL != time_4g_cclk_callback)
         {
             time_4g_cclk_callback(&time_info);
         }

         seqmgr_4g_handle_cmd_exe_result(AT_4G_TIM_CCLK, SEQMGR_4G_CMD_EXE_FAILED);
         return AT_4G_DECODE_ISMATCH;
     }
     else
     {
         /**/
     }

     return AT_4G_DECODE_NOMATCH;
}

static uint8 time_4g_ntp_resp(void *context, uint8 *data, uint16 *len)
{
    uint8 ret;
    IF_4G_TIMINFO time_info;

    time_info.is_valid = 0;
    memset(time_info.time, 0, sizeof(time_info.time));

    ret = at_4g_common_resp(AT_4G_TIM_NTP, (AT_4G_CMD_RESP *)context, data, len);
    if(AT_4G_DECODE_ISMATCH == ret)
    {
        if(AT_4G_RESP_OK == ((AT_4G_CMD_RESP *)context)->resp_code)
        {
            time_info.is_valid = 1;
        }
        if(NULL != time_4g_ntp_callback)
        {
            time_4g_ntp_callback(&time_info);
        }
    }

    MODULE_LOG_I(TBOX4G, "code:%d ret:%d", ((AT_4G_CMD_RESP *)context)->resp_code, ret);

    return ret;
}
