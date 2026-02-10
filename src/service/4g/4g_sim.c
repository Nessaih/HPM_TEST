#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_sim.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"

static uint8 sim_4g_query_initstate_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_query_pinstate_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_query_iccid_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_query_imsi_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_enable_setnum_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_query_num_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_set_num_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_query_state_resp(void *context, uint8 *data, uint16 *len);

static uint8 sim_4g_initstate;
static uint8 sim_4g_pinstate;

SIM_4G_INFO sim_4g_info;

void sim_4g_init(void)
{
    sim_4g_info.sim_state = IF_4G_SIMSTATE_UNKNOWN;
    sim_4g_initstate = SIM_4G_INITSTATE_DEFAULT;
    sim_4g_pinstate = SIM_4G_PINSTATE_UNKNOWN;
    sim_4g_info.iccid_is_valid = FALSE;
    memset(sim_4g_info.sim_4g_iccid, 0, sizeof(sim_4g_info.sim_4g_iccid));
    sim_4g_info.imsi_is_valid  = FALSE;
    memset(sim_4g_info.sim_4g_imsi, 0, sizeof(sim_4g_info.sim_4g_imsi));
    sim_4g_info.setnum_flag = SIM_4G_SETNUM_UNKNOWN;
    sim_4g_info.num_is_valid = FALSE;
    memset(sim_4g_info.sim_4g_num, 0, sizeof(sim_4g_info.sim_4g_num));
}

void sim_4g_reinit(void)
{
    sim_4g_initstate = SIM_4G_INITSTATE_DEFAULT;
    sim_4g_pinstate = SIM_4G_PINSTATE_UNKNOWN;
    sim_4g_info.setnum_flag = SIM_4G_SETNUM_UNKNOWN;
    sim_4g_info.num_is_valid = FALSE;
    memset(sim_4g_info.sim_4g_num, 0, sizeof(sim_4g_info.sim_4g_num));
}


uint8 sim_4g_query_initstate(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QINISTAT" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_INITSTATE;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_initstate_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_query_pinstate(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CPIN?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_PINSTATE;
    cmd.retry_count = 0;
    cmd.tick = 5000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_pinstate_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_query_iccid(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QCCID" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_ICCID;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_iccid_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

	TBOX_4G_MUTEX_LOCK();
    sim_4g_info.iccid_is_valid = FALSE;
    memset(sim_4g_info.sim_4g_iccid, 0, sizeof(sim_4g_info.sim_4g_iccid));
	TBOX_4G_MUTEX_UNLOCK();

    return 0;
}

uint8 sim_4g_query_imsi(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CIMI" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_IMSI;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_imsi_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

	TBOX_4G_MUTEX_LOCK();
    sim_4g_info.imsi_is_valid  = FALSE;
    memset(sim_4g_info.sim_4g_imsi, 0, sizeof(sim_4g_info.sim_4g_imsi));
	TBOX_4G_MUTEX_UNLOCK();

    return 0;
}

uint8 sim_4g_enable_setnum(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CPBS=\"SM\"" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_ENABLE_SETNUM;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_enable_setnum_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_query_num(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CNUM" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_NUM;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_num_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_query_num_ex(AT_4G_CMD_PRIORITY pri, uint8 retry)
{
    UNUSED(pri);
    UNUSED(retry);
#if 0 /*TODO: 后续实现*/
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CPBR=1\r\n";
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_NUM;
    cmd.retry_count = retry;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_num_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

#endif
    return 0;
}

uint8 sim_4g_set_num(AT_4G_CMD_PRIORITY pri, char* num)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CPBW=1,\"%s\"\r\n";
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((char*)ele->mem_ptr, 31, at_str, num);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_SET_NUM;
    cmd.retry_count = 0;
    cmd.tick = 300;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_set_num_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_set_num_ex(AT_4G_CMD_PRIORITY pri, char* num, uint8 retry)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CPBW=1,\"%s\"\r\n";
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((char*)ele->mem_ptr, 31, at_str, num);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_SET_NUM;
    cmd.retry_count = retry;
    cmd.tick = 300;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_set_num_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_query_state(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QSIMSTAT?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_SIMSTAE;
    cmd.retry_count = 0U;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = sim_4g_query_state_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 sim_4g_get_initstate(void)
{
    return sim_4g_initstate;
}

void sim_4g_set_initstate(uint8 state)
{
    sim_4g_initstate |= state;
}

uint8 sim_4g_get_pinstate(void)
{
    return sim_4g_pinstate;
}

void sim_4g_set_pinstate(uint8 state)
{
    sim_4g_pinstate = state;
    if(SIM_4G_PINSTATE_READY == state)
    {
        sim_4g_initstate = (sim_4g_initstate | SIM_4G_INITSTATE_PINREADY);
    }
}

static uint8 sim_4g_query_initstate_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_INITSTATE != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint32 initstate = 0;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QINISTAT: ");
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
        if(FALSE == tbox_string_extract_num(temp, temp_len, (uint32*)&initstate))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        sim_4g_initstate = (uint8)initstate;

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_INITSTATE, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_INITSTATE, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sim_4g_query_pinstate_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_PINSTATE != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CPIN: ");
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
        if(NULL != tbox_string_get_substring(temp, temp_len, "READY"))
        {
            sim_4g_pinstate = SIM_4G_PINSTATE_READY;
        }
        else if(NULL != tbox_string_get_substring(temp, temp_len, "NOT INSERTED"))
        {
            sim_4g_pinstate = SIM_4G_PINSTATE_NOINSERT;
        }
        else if(NULL != tbox_string_get_substring(temp, temp_len, "PIN"))
        {
            sim_4g_pinstate = SIM_4G_PINSTATE_NEEDPIN;
        }
        else if(NULL != tbox_string_get_substring(temp, temp_len, "PUK"))
        {
            sim_4g_pinstate = SIM_4G_PINSTATE_NEEDPUK;
        }
        else
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_PINSTATE, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_CMEERR_SIMNOINSERT == resp->resp_code ||
            AT_4G_RESP_CMSERR_SIMNOINSERT == resp->resp_code)
    {
        sim_4g_pinstate = SIM_4G_PINSTATE_NOINSERT;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_PINSTATE, SEQMGR_4G_CMD_EXE_OK);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            AT_4G_RESP_ERROR == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        sim_4g_pinstate = SIM_4G_PINSTATE_UNKNOWN;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_PINSTATE, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sim_4g_query_iccid_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_ICCID != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QCCID: ");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(temp, temp_len - (uint16)(temp-data), AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        temp = temp + strlen("+QCCID: ");
        temp_len = temp_len - (uint16)(temp-data) - strlen(AT_4G_RECV_END_OK_STRING);

		TBOX_4G_MUTEX_LOCK();
        sim_4g_info.iccid_is_valid = TRUE;
        if(temp_len >= (SIM_4G_ICCID_MAX-1))
        {
            strncpy((char*)sim_4g_info.sim_4g_iccid, (char*)temp, SIM_4G_ICCID_MAX-1);
        }
        else
        {
            strncpy((char*)sim_4g_info.sim_4g_iccid, (char*)temp, temp_len);
        }
		TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_ICCID, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            AT_4G_RESP_ERROR == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
		TBOX_4G_MUTEX_LOCK();
        sim_4g_info.iccid_is_valid = FALSE;
		TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_ICCID, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sim_4g_query_imsi_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
     uint8 *token;
     uint8 index, count;
     uint16 temp_len = 0;

     if(AT_4G_QUERY_IMSI != resp->cmd.cmd_id)
     {
        return AT_4G_DECODE_NOMATCH;
     }

     if(AT_4G_RESP_UNKNOWN == resp->resp_code)
     {
         temp_len = *len;
         token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
         if(NULL == token)
         {
             return AT_4G_DECODE_NOMATCH;
         }
         count = 0;
         temp_len = (uint16)(token-data);
         for(index = 0; index < temp_len; index++)
         {
             if(!isdigit(data[index]))
             {
                 break;
             }
             if(count >= SIM_4G_IMSI_MAX)
             {
                 break;
             }
             sim_4g_info.sim_4g_imsi[count++] = data[index];
         }
         if(count > 0)
         {
             sim_4g_info.imsi_is_valid = TRUE;
         }

         seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_IMSI, SEQMGR_4G_CMD_EXE_OK);

         token = token + strlen(AT_4G_RECV_END_OK_STRING);
         *len = (uint16)(token-data);
         return AT_4G_DECODE_ISMATCH;
     }
     else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
             AT_4G_RESP_ABORT == resp->resp_code ||
             (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
              resp->resp_code <= AT_4G_RESP_MAX_ERROR))
     {
         sim_4g_info.imsi_is_valid = FALSE;
         seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_IMSI, SEQMGR_4G_CMD_EXE_FAILED);
         return AT_4G_DECODE_ISMATCH;
     }
     else
     {
         /**/
     }

     return AT_4G_DECODE_NOMATCH;
}

static uint8 sim_4g_enable_setnum_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    if(AT_4G_RESP_OK == resp->resp_code)
    {
        sim_4g_info.setnum_flag = SIM_4G_ENABLE_SETNUM;
    }
    return at_4g_common_resp(AT_4G_ENABLE_SETNUM, resp, data, len);
}

static uint8 sim_4g_query_num_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;
    uint16 begin_write = 0U;
    uint16 write_index = 0U;

    if(AT_4G_QUERY_NUM != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_OK == resp->resp_code)
    {
        sim_4g_info.num_is_valid = FALSE;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_NUM, SEQMGR_4G_CMD_EXE_OK);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CNUM:");
        if(NULL == token)
        {
            return AT_4G_DECODE_NOMATCH;
        }
        temp = token;
        token = tbox_string_get_substring(data, temp_len, "OK\r\n");
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }
        temp = temp + strlen("+CNUM:");
        temp_len = temp_len - (uint16)(temp-data);
        TBOX_4G_MUTEX_LOCK();
        for(uint16 i = 0; i < temp_len; i++)
        {
            if(temp[i] == '\"')
            {
                if(0U == begin_write)
                {
                    begin_write = 1U;
                    continue;
                }
                else
                {
                    break;
                }
            }
            if(1U == begin_write)
            {
                if(write_index < SIM_4G_NUM_MAX)
                {
                    sim_4g_info.sim_4g_num[write_index++] = temp[i];
                }
            }
        }
        if(write_index > 0U)
        {
            sim_4g_info.num_is_valid = TRUE;
        }
        TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_NUM, SEQMGR_4G_CMD_EXE_OK);

        token += strlen("OK\r\n");
        *len = (uint16)(token-data);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
		TBOX_4G_MUTEX_LOCK();
        sim_4g_info.num_is_valid = FALSE;
		TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_NUM, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sim_4g_set_num_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;

    UNUSED(len);
    UNUSED(data);

    if(AT_4G_SET_NUM != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        return AT_4G_DECODE_NOMATCH;
    }
    else if(AT_4G_RESP_OK == resp->resp_code)
    {
#if 0
        sim_4g_info.num_is_valid = FALSE;
        seqmgr_4g_handle_cmd_exe_result(AT_4G_SET_NUM, SEQMGR_4G_CMD_EXE_OK);
#endif
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_SET_NUM, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 sim_4g_query_state_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token, *temp;
    uint16 temp_len = 0;
    uint32 retvalue = IF_4G_SIMSTATE_UNKNOWN;

    if(AT_4G_QUERY_SIMSTAE != resp->cmd.cmd_id)
    {
        return AT_4G_DECODE_NOMATCH;
    }

     if(AT_4G_RESP_UNKNOWN == resp->resp_code)
     {
         temp_len = *len;
         token = tbox_string_get_substring(data, temp_len, "+QSIMSTAT: ");
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
         if(sscanf((char *)temp, "+QSIMSTAT: %*d,%u", &retvalue) > 0)
         {
             TBOX_4G_MUTEX_LOCK();
             sim_4g_info.sim_state = (uint8)retvalue;
             if(sim_4g_info.sim_state == IF_4G_SIMSTATE_PULLOUT)
             {
                sim_4g_pinstate = SIM_4G_PINSTATE_NOINSERT;
             }
             else if(sim_4g_info.sim_state == IF_4G_SIMSTATE_INSERT)
             {
                sim_4g_pinstate = SIM_4G_PINSTATE_READY;
             }
             else
             {
                /*TODO*/
             }
             TBOX_4G_MUTEX_UNLOCK();
         }
         
         seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SIMSTAE, SEQMGR_4G_CMD_EXE_OK);

         token = token + strlen(AT_4G_RECV_END_OK_STRING);
         *len = (uint16)(token-data);
         return AT_4G_DECODE_ISMATCH;
     }
     else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
             AT_4G_RESP_ABORT == resp->resp_code ||
             (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
              resp->resp_code <= AT_4G_RESP_MAX_ERROR))
     {
         seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_SIMSTAE, SEQMGR_4G_CMD_EXE_FAILED);
         return AT_4G_DECODE_ISMATCH;
     }
     else
     {
         /**/
     }

     return AT_4G_DECODE_NOMATCH;
}