#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_modem.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"
#include "4g_if.h"

static uint8 modem_4g_test_alive_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_readbaud_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_setbaud_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_echomode_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_enable_errnum_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_set_airmode_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_set_normalmode_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_query_funmode_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_query_imei_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_config_urcport_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_urc_other_off_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_ring_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_smsincoming_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_query_temp_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_query_mtid_resp(void *context, uint8 *data, uint16 *len);

static uint8 modem_4g_query_taid_resp(void *context, uint8 *data, uint16 *len);

static uint32 modem_4g_baud;
static uint8  modem_4g_funmode;
MODEM_4G_INFO modem_4g_info;

void  modem_4g_init(void)
{
    modem_4g_baud = 0U;
    modem_4g_funmode = MODEM_4G_FUN_UNKNOWN;
    modem_4g_info.imei_is_valid = FALSE;
    memset(modem_4g_info.modem_4g_imei, 0, sizeof(modem_4g_info.modem_4g_imei));
    modem_4g_info.modem_4g_temp = 0U;
    modem_4g_info.mtid_is_valid = FALSE;
    memset(modem_4g_info.modem_4g_mtid, 0, sizeof(modem_4g_info.modem_4g_mtid));
    modem_4g_info.taid_is_valid = FALSE;
    memset(modem_4g_info.modem_4g_taid, 0, sizeof(modem_4g_info.modem_4g_taid));

    modem_4g_info.testlive_fail_count = 0U;
    modem_4g_info.act = 10U; //automatic
    modem_4g_info.funmode = 1U;
}

void  modem_4g_reinit(void)
{
    modem_4g_baud = 0;
    modem_4g_funmode = MODEM_4G_FUN_UNKNOWN;
}

void  modem_4g_clear_alivefaile_count(void)
{
    TBOX_4G_MUTEX_LOCK();
    modem_4g_info.testlive_fail_count = 0U;
    TBOX_4G_MUTEX_UNLOCK();
}

uint16 modem_4g_get_alivefaile_count(void)
{
    uint16 count;

    TBOX_4G_MUTEX_LOCK();
    count = modem_4g_info.testlive_fail_count;
    TBOX_4G_MUTEX_UNLOCK();
    
    return count;
}

uint8 modem_4g_test_alive(AT_4G_CMD_PRIORITY pri, uint8 retry_count)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_TESTALIVE_CMD;
    cmd.retry_count = retry_count;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_test_alive_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_read_baud(AT_4G_CMD_PRIORITY pri, uint8 retry_count)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+IPR?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_READ_BAUD;
    cmd.retry_count = retry_count;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_readbaud_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_set_baud(AT_4G_CMD_PRIORITY pri, uint32 baud)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+IPR=%d" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((char*)ele->mem_ptr, 31, at_str, baud);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_WRITE_BAUD;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_setbaud_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_echomode(AT_4G_CMD_PRIORITY pri, uint8 mode)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "ATE%d" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    snprintf((char*)ele->mem_ptr,15, at_str, mode);
    ele->len = strlen((char*)ele->mem_ptr);

    cmd.cmd_id = AT_4G_WRITE_ECHOMODE;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_echomode_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_enable_errnum(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+CMEE=1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_ENABLE_ERR_NUM;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_enable_errnum_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}


uint8 modem_4g_set_airmode(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+CFUN=0" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_SET_AIRMODE;
    cmd.retry_count = 0;
    cmd.tick = 5500;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_set_airmode_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_set_normalmode(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+CFUN=1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_SET_NORMALMODE;
    cmd.retry_count = 0;
    cmd.tick = 5500;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_set_normalmode_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

void modem_4g_query_funmode(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+CFUN?" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_FUNMODE;
    cmd.retry_count = 0;
    cmd.tick = 5500;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_query_funmode_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return;
    }
}

uint8 modem_4g_query_imei(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+GSN=1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_IMEI;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_query_imei_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

	TBOX_4G_MUTEX_LOCK();
    modem_4g_info.imei_is_valid = FALSE;
    memset(modem_4g_info.modem_4g_imei, 0, sizeof(modem_4g_info.modem_4g_imei));
	TBOX_4G_MUTEX_UNLOCK();

    return 0;
}

uint8 modem_4g_config_urc_port(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+QURCCFG=\"urcport\",\"uart1\"" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_32BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CONFIG_URCPORT;
    cmd.retry_count = 0;
    cmd.tick = 1000;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_config_urcport_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_urc_other_off(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+QCFG=\"urc/ri/other\",\"off\"" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CONFIG_URC_OTHER_OFF;
    cmd.retry_count = 0;
    cmd.tick = 1000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_urc_other_off_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_urc_ring(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+QCFG=\"urc/ri/ring\",\"pulse\",120,1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CONFIG_RING;
    cmd.retry_count = 0;
    cmd.tick = 1000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_ring_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}

uint8 modem_4g_urc_smsincoming(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    char *at_str = "AT+QCFG=\"urc/ri/smsincoming\",\"pulse\",120,1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_64BYTE);
    if(NULL == ele)
    {
        return 1;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_CONFIG_SMSINCOMING;
    cmd.retry_count = 0;
    cmd.tick = 1000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_smsincoming_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1;
    }

    return 0;
}


uint32 modem_4g_get_baud_value(void)
{
    return modem_4g_baud;
}

void modem_4g_set_baud_value(uint32 baud)
{
    modem_4g_baud = baud;
}

uint8 modem_4g_get_funmode_value(void)
{
    return modem_4g_funmode;
}

void modem_4g_set_funmode_value(uint8 mode)
{
    modem_4g_funmode = mode;
}


uint8 modem_4g_query_temp(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QTEMP" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1U;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_TEMP;
    cmd.retry_count = 0;
    cmd.tick = 1000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_query_temp_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1U;
    }

    return 0U;
}

uint8 modem_4g_query_mtid(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CGMM" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1U;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_MTID;
    cmd.retry_count = 0;
    cmd.tick = 1000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_query_mtid_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1U;
    }

    return 0U;
}

uint8 modem_4g_query_taid(AT_4G_CMD_PRIORITY pri)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+CGMR" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        return 1U;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_QUERY_TAID;
    cmd.retry_count = 0;
    cmd.tick = 1000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = modem_4g_query_taid_resp;
    if(0 != at_4g_transmit_putcmd(pri, &cmd))
    {
        sharemem_4g_free(index);
        return 1U;
    }

    return 0U;
}

static uint8 modem_4g_test_alive_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    if(AT_4G_RESP_TIMEOUT == resp->resp_code)
    {
        modem_4g_info.testlive_fail_count++;
    }
    return at_4g_common_resp(AT_4G_TESTALIVE_CMD, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 modem_4g_readbaud_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_READ_BAUD != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        uint32 baud = 0;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+IPR: ");
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

        temp_len = temp_len - (uint16)(temp-data);
        if(FALSE == tbox_string_extract_num(temp, temp_len, &baud))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        modem_4g_baud = baud;

        seqmgr_4g_handle_cmd_exe_result(AT_4G_READ_BAUD, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);

        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_READ_BAUD, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 modem_4g_setbaud_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_WRITE_BAUD, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 modem_4g_echomode_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_WRITE_ECHOMODE, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 modem_4g_enable_errnum_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_ENABLE_ERR_NUM, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 modem_4g_set_airmode_resp(void *context, uint8 *data, uint16 *len)
{
    uint8 ret = at_4g_common_resp(AT_4G_SET_AIRMODE, (AT_4G_CMD_RESP *)context, data, len);
    if(AT_4G_DECODE_ISMATCH == ret)
    {
        modem_4g_funmode = MODEM_4G_FUN_AIRMODE;
    }
    return ret;
}

static uint8 modem_4g_set_normalmode_resp(void *context, uint8 *data, uint16 *len)
{
    uint8 ret = at_4g_common_resp(AT_4G_SET_NORMALMODE, (AT_4G_CMD_RESP *)context, data, len);
    if(AT_4G_DECODE_ISMATCH == ret)
    {
        modem_4g_funmode = MODEM_4G_FUN_NORMAL;
    }
    return ret;
}

static uint8 modem_4g_query_funmode_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0;

    if(AT_4G_QUERY_FUNMODE != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        INT32 fun;

        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+CFUN: ");
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

        if(sscanf((char*)temp, "+CFUN: %d", &fun))
        {
            MODULE_LOG_I(TBOX4G, "fun:%d", fun);
            modem_4g_info.funmode = (uint8)fun;
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_FUNMODE, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data)+strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_FUNMODE, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 modem_4g_query_imei_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint8 index;
    uint16 temp_len = 0;
    uint16 has_wr_len = 0;

    if(AT_4G_QUERY_IMEI != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+GSN: ");
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

        temp = temp + strlen("+GSN: ");
        temp_len = temp_len - (uint16)(temp-data) - strlen(AT_4G_RECV_END_OK_STRING);

		TBOX_4G_MUTEX_LOCK();
        memset(modem_4g_info.modem_4g_imei, 0, sizeof(modem_4g_info.modem_4g_imei));
        for(index = 0; index < temp_len; index++)
        {
            if(!isdigit(*temp))
            {
                if(0 == has_wr_len)
                {
                    temp++;
                    continue;
                }
                else
                {
                    break;
                }
            }
            if(has_wr_len >= MODEM_4G_IMEI_MAX)
            {
                break;
            }
            modem_4g_info.modem_4g_imei[has_wr_len++] = *temp - '0';
            temp++;
        }
        if(has_wr_len > 0)
        {
            modem_4g_info.imei_is_valid = 1;
        }
		TBOX_4G_MUTEX_UNLOCK();

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_IMEI, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code   ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_IMEI, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 modem_4g_config_urcport_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_URCPORT, (AT_4G_CMD_RESP *)context, data, len);
}
static uint8 modem_4g_urc_other_off_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_URC_OTHER_OFF, (AT_4G_CMD_RESP *)context, data, len);
}
static uint8 modem_4g_ring_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_RING, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 modem_4g_smsincoming_resp(void *context, uint8 *data, uint16 *len)
{
    return at_4g_common_resp(AT_4G_CONFIG_SMSINCOMING, (AT_4G_CMD_RESP *)context, data, len);
}

static uint8 modem_4g_query_temp_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *temp;
    uint8 *token;
    uint16 temp_len = 0U;
    uint32 temp_val = 0U;

    if(AT_4G_QUERY_TEMP != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, "+QTEMP: ");
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

        temp = temp + strlen("+QTEMP: ");
        if(TRUE == tbox_string_get_num(temp, (uint8)',', &temp_val))
        {
            TBOX_4G_MUTEX_LOCK();
            modem_4g_info.modem_4g_temp = temp_val;
            TBOX_4G_MUTEX_UNLOCK();
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_TEMP, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code   ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_TEMP, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;    
}

static uint8 modem_4g_query_mtid_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint8 *pos;
    uint16 temp_len = 0U;
    uint16 find_len;

    if(AT_4G_QUERY_MTID != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }
        temp_len = (uint16)(token - data);
        pos = data;
        find_len = temp_len;
        while (find_len > 0u)
        {
             data = tbox_string_get_substring(pos, find_len, "\r\n");
             if(NULL_PTR == data)
             {
                data = pos;
                temp_len = find_len;
                break;
             }
             temp_len = (uint16)(data - pos)+strlen("\r\n");
             if(temp_len >= find_len)
             {
                data = data + strlen("\r\n");
                temp_len = 0U;
                break;
             }
             pos = data + strlen("\r\n");
             find_len = find_len - temp_len;
        }    
        if(temp_len > (MODEM_4G_MTID_LEN-1U))
        {
            temp_len = MODEM_4G_MTID_LEN-1U;
        }
        if(temp_len > 0U)
        {
            TBOX_4G_MUTEX_LOCK();
            strncpy((CHAR *)modem_4g_info.modem_4g_mtid, (CHAR *)data, temp_len);
            modem_4g_info.mtid_is_valid = 1U;
            TBOX_4G_MUTEX_UNLOCK();
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_MTID, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code   ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_MTID, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;    
}

static uint8 modem_4g_query_taid_resp(void *context, uint8 *data, uint16 *len)
{
    AT_4G_CMD_RESP *resp = (AT_4G_CMD_RESP *)context;
    uint8 *token;
    uint8 *pos;
    uint16 temp_len = 0U;
    uint16 find_len;

    if(AT_4G_QUERY_TAID != resp->cmd.cmd_id)
    {
       return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        temp_len = *len;
        token = tbox_string_get_substring(data, temp_len, AT_4G_RECV_END_OK_STRING);
        if(NULL == token)
        {
            return AT_4G_DECODE_CONTINUE;
        }
        temp_len = (uint16)(token - data);
        pos = data;
        find_len = temp_len;
        while (find_len > 0u)
        {
             data = tbox_string_get_substring(pos, find_len, "\r\n");
             if(NULL_PTR == data)
             {
                data = pos;
                temp_len = find_len;
                break;
             }
             temp_len = (uint16)(data - pos)+strlen("\r\n");
             if(temp_len >= find_len)
             {
                data = data + strlen("\r\n");
                temp_len = 0U;
                break;
             }
             pos = data + strlen("\r\n");
             find_len = find_len - temp_len;
        }
        if(temp_len > (MODEM_4G_TAID_LEN-1U))
        {
            temp_len = MODEM_4G_TAID_LEN-1U;
        }
        if(temp_len > 0U)
        {
            TBOX_4G_MUTEX_LOCK();
            strncpy((CHAR *)modem_4g_info.modem_4g_taid, (CHAR *)data, temp_len);
            modem_4g_info.taid_is_valid = 1U;
            TBOX_4G_MUTEX_UNLOCK();
        }

        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_TAID, SEQMGR_4G_CMD_EXE_OK);

        *len = (uint16)(token-data) + strlen(AT_4G_RECV_END_OK_STRING);
        return AT_4G_DECODE_ISMATCH;
    }
    else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
            AT_4G_RESP_ABORT == resp->resp_code   ||
            (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
             resp->resp_code <= AT_4G_RESP_MAX_ERROR))
    {
        seqmgr_4g_handle_cmd_exe_result(AT_4G_QUERY_TAID, SEQMGR_4G_CMD_EXE_FAILED);
        return AT_4G_DECODE_ISMATCH;
    }
    else
    {
        /**/
    }

    return AT_4G_DECODE_NOMATCH;     
}