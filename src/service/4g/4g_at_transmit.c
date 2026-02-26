#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_dev.h"
#include "4g_data.h"
#include "4g_mgr.h"
#include "4g_modem.h"
#include "4g_at_transmit.h"
#include "4g_sharememery.h"

#define AT_4G_LOWCMD_QUEUE_COUNT  10
#define AT_4G_MIDCMD_QUEUE_COUNT  2
#define AT_4G_HIGHCMD_QUEUE_COUNT 1

typedef struct
{
    AT_4G_CMD cmd;
    uint8 is_used;
}AT_4G_CMD_QUEUE_ELE;

static AT_4G_DECODE_CALLBACK at_decode_callbak[AT_4G_DECODE_CALLBACK_TYPE_MAX];
static AT_4G_TRANS_FILTER_FUN at_trans_filter_fun[AT_4G_TRANS_FILTER_MAX];
static AT_4G_CMD_QUEUE_ELE at_4g_trans_lowqueue[AT_4G_LOWCMD_QUEUE_COUNT];
static AT_4G_CMD_QUEUE_ELE at_4g_trans_midqueue[AT_4G_MIDCMD_QUEUE_COUNT];
static AT_4G_CMD_QUEUE_ELE at_4g_trans_highqueue[AT_4G_HIGHCMD_QUEUE_COUNT];
static uint16 at_4g_trans_current_tick;
static uint8 at_4g_trans_recv_continue;

static uint8 at_4g_trans_parse_iurc(uint8 *data, uint16 *len);

static uint8 at_4g_trans_parse_state(uint8 *data, uint16 *len);

static boolean at_4g_trans_has_higherpri_data(uint8 pri);

static uint8  at_4g_get_cme_errorcode(uint8 *data, uint16 len);

static uint8  at_4g_get_cms_errorcode(uint8 *data, uint16 len);

static uint8  at_4g_get_initstate_code(uint8 *data, uint16 len);

static uint8  at_4g_get_fun_code(uint8 *data, uint16 len);

static void at_4g_remove_check_iuc(uint8 *data, uint16 len);

void at_4g_transmit_init(void)
{
    uint8 index;

    at_4g_trans_current_tick = 0;

    at_4g_trans_recv_continue = 0;

    for(index = 0; index < AT_4G_DECODE_CALLBACK_TYPE_MAX; index++)
    {
        at_decode_callbak[index] = NULL;
    }
    for(index = 0; index < AT_4G_TRANS_FILTER_MAX; index++)
    {
        at_trans_filter_fun[index] = NULL;
    }

    for(index = 0; index < AT_4G_LOWCMD_QUEUE_COUNT; index++)
    {
        at_4g_trans_lowqueue[index].cmd.resp = NULL;
        at_4g_trans_lowqueue[index].cmd.state = AT_CMD_STATE_IDLE;
        at_4g_trans_lowqueue[index].cmd.sharm_index = -1;
        at_4g_trans_lowqueue[index].is_used = 0;
    }
    for(index = 0; index < AT_4G_MIDCMD_QUEUE_COUNT; index++)
    {
        at_4g_trans_midqueue[index].cmd.resp = NULL;
        at_4g_trans_midqueue[index].cmd.state = AT_CMD_STATE_IDLE;
        at_4g_trans_midqueue[index].cmd.sharm_index = -1;
        at_4g_trans_midqueue[index].is_used = 0;
    }
    for(index = 0; index < AT_4G_HIGHCMD_QUEUE_COUNT; index++)
    {
        at_4g_trans_highqueue[index].cmd.resp = NULL;
        at_4g_trans_highqueue[index].cmd.state = AT_CMD_STATE_IDLE;
        at_4g_trans_highqueue[index].cmd.sharm_index = -1;
        at_4g_trans_highqueue[index].is_used = 0;
    }
}

void at_4g_transmit_reset(void)
{
    uint8 index;

    at_4g_trans_current_tick = 0;

    at_4g_trans_recv_continue = 0;

    for(index = 0; index < AT_4G_LOWCMD_QUEUE_COUNT; index++)
    {
        at_4g_trans_lowqueue[index].cmd.resp = NULL;
        at_4g_trans_lowqueue[index].cmd.state = AT_CMD_STATE_IDLE;
        at_4g_trans_lowqueue[index].cmd.sharm_index = -1;
        at_4g_trans_lowqueue[index].is_used = 0;
    }
    for(index = 0; index < AT_4G_MIDCMD_QUEUE_COUNT; index++)
    {
        at_4g_trans_midqueue[index].cmd.resp = NULL;
        at_4g_trans_midqueue[index].cmd.state = AT_CMD_STATE_IDLE;
        at_4g_trans_midqueue[index].cmd.sharm_index = -1;
        at_4g_trans_midqueue[index].is_used = 0;
    }
    for(index = 0; index < AT_4G_HIGHCMD_QUEUE_COUNT; index++)
    {
        at_4g_trans_highqueue[index].cmd.resp = NULL;
        at_4g_trans_highqueue[index].cmd.state = AT_CMD_STATE_IDLE;
        at_4g_trans_highqueue[index].cmd.sharm_index = -1;
        at_4g_trans_highqueue[index].is_used = 0;
    }
}

void at_4g_transmit_check_recv(void)
{
    uint8 *data_ptr = (uint8 *)NULL;
    uint8 *token = (uint8 *)NULL;
    uint8 ret;
    uint16 len, temp_len;

    data_ptr = data_4g_recv_pull(&len);

    /*in data mode*/
    if(NULL != at_decode_callbak[AT_4G_DECODE_FILTER])
    {
        if(0 == len)
        {
            return;
        }

        temp_len = len;
        ret = at_decode_callbak[AT_4G_DECODE_FILTER](&data_4g_cur_send, data_ptr, &temp_len);
        if(AT_4G_DECODE_ISMATCH == ret)
        {
            data_4g_recv_remove(temp_len);
        }
        else if(AT_4G_DECODE_INVALID_PACKET == ret)
        {
            data_4g_recv_remove(len);
        }
        else
        {
            /**/
        }

        return;
    }

    /*in at mode*/
    if(len <= 2)
    {
        return;
    }

    MODULE_LOG_DUMP(TBOX4G, "at recv data", data_ptr, len);

    if(0 == at_4g_trans_recv_continue)
    {
        token = tbox_string_get_substring(data_ptr, len,  "\r\n");
        if(NULL == token)
        {
            if('\r' == data_ptr[len-1])
            {
                data_4g_recv_remove(len-1);
            }
            else
            {
                data_4g_recv_remove(len);
            }
            return;
        }
        if((uint16)(token-data_ptr) >= len)
        {
            data_4g_recv_remove(len);
            return;
        }
        data_4g_recv_remove((uint16)(token-data_ptr)+2); /*remove leading characters:\r\n*/

        data_ptr = data_4g_recv_pull(&len);
        if(len <= 0)
        {
            return;
        }
    }

    temp_len = len;
    if(AT_CMD_STATE_WAITRESP == (data_4g_cur_send.cmd.state & AT_CMD_STATE_WAITRESP))
    {
        if(NULL != data_4g_cur_send.cmd.resp)
        {
            ret = data_4g_cur_send.cmd.resp(&data_4g_cur_send, data_ptr, &temp_len);
            at_4g_trans_recv_continue = (ret == AT_4G_DECODE_CONTINUE) ? 1 : 0;

            if(AT_4G_DECODE_ISMATCH == ret)
            {
                modem_4g_clear_alivefaile_count();
                data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_WAITRESP);
                if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
                {
                    data_4g_cur_send.cmd.resp = NULL;
                    data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
                    sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
                    data_4g_cur_send.cmd.sharm_index = -1;
                    at_4g_trans_current_tick = 0;
                }

                at_4g_remove_check_iuc(data_ptr, temp_len);
                data_4g_recv_remove(temp_len);
                return;
            }
            else if(AT_4G_DECODE_CONTINUE == ret)
            {
                return;
            }
            else if(AT_4G_DECODE_INVALID_PACKET == ret)
            {
                at_4g_remove_check_iuc(data_ptr, len);
                data_4g_recv_remove(len);
                return;
            }
            else
            {
                /**/
            }
        }
    }

    ret = at_4g_trans_parse_iurc(data_ptr, &temp_len);
    at_4g_trans_recv_continue = (ret == AT_4G_DECODE_CONTINUE) ? 1 : 0;
    if(AT_4G_DECODE_ISMATCH == ret)
    {
        data_4g_recv_remove(temp_len);
        return;
    }
    else if(AT_4G_DECODE_CONTINUE == ret)
    {
        return;
    }
    else if(AT_4G_DECODE_INVALID_PACKET == ret)
    {
        data_4g_recv_remove(len);
        return;
    }
    else
    {
        /**/
    }

    ret = at_4g_trans_parse_state(data_ptr, &temp_len);
    at_4g_trans_recv_continue = (ret == AT_4G_DECODE_CONTINUE) ? 1 : 0;
    if(AT_4G_DECODE_ISMATCH == ret)
    {
        data_4g_recv_remove(temp_len);
    }
    else if(AT_4G_DECODE_INVALID_PACKET == ret)
    {
        data_4g_recv_remove(len);
    }
}

void at_4g_transmit_check_send(void)
{
    uint8 index;

    if(NULL != at_trans_filter_fun[AT_4G_TRANS_FILTER] &&
        AT_4G_TRANS_NONEEDSEND_DATA == at_trans_filter_fun[AT_4G_TRANS_FILTER]())
    {
        return;
    }

    if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
    {
       return;
    }

    for(index = 0; index < AT_4G_HIGHCMD_QUEUE_COUNT; index++)
    {
        if(1 == at_4g_trans_highqueue[index].is_used)
        {
            memcpy(&data_4g_cur_send.cmd, &at_4g_trans_highqueue[index].cmd, sizeof(data_4g_cur_send.cmd));
            data_4g_cur_send.pri = AT_4G_CMD_HIGH;
            at_4g_trans_highqueue[index].is_used = 0;
            goto send_at_data;
        }
    }
    for(index = 0; index < AT_4G_MIDCMD_QUEUE_COUNT; index++)
    {
        if(1 == at_4g_trans_midqueue[index].is_used)
        {
            memcpy(&data_4g_cur_send.cmd, &at_4g_trans_midqueue[index].cmd, sizeof(data_4g_cur_send.cmd));
            data_4g_cur_send.pri = AT_4G_CMD_MID;
            at_4g_trans_midqueue[index].is_used = 0;
            goto send_at_data;
        }
    }
    for(index = 0; index < AT_4G_LOWCMD_QUEUE_COUNT; index++)
    {
        if(1 == at_4g_trans_lowqueue[index].is_used)
        {
            memcpy(&data_4g_cur_send.cmd, &at_4g_trans_lowqueue[index].cmd, sizeof(data_4g_cur_send.cmd));
            data_4g_cur_send.pri = AT_4G_CMD_LOW;
            at_4g_trans_lowqueue[index].is_used = 0;
            break;
        }
    }
    if(index >= AT_4G_LOWCMD_QUEUE_COUNT)
    {
        return;
    }

send_at_data:
    if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
    {
        at_4g_trans_current_tick = data_4g_cur_send.cmd.tick/PERIODIC_UNIT_4G+1;
        data_4g_cur_send.bak_state = data_4g_cur_send.cmd.state;
        data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
        dev_4g_check_send();
    }
}

void at_4g_transmit_check_timeout(void)
{
    if(at_4g_trans_current_tick <= 0)
    {
        return;
    }

    if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
    {
        at_4g_trans_current_tick = 0;
        return;
    }

    at_4g_trans_current_tick--;
    if(at_4g_trans_current_tick > 0)
    {
        return;
    }

    MODULE_LOG_I(TBOX4G, "transmit timeout, cmd:%d retry_count:%d", data_4g_cur_send.cmd.cmd_id,
                   data_4g_cur_send.cmd.retry_count);

    if(data_4g_cur_send.cmd.retry_count <= 0)
    {
        data_4g_cur_send.resp_code = AT_4G_RESP_TIMEOUT;
        if(NULL != data_4g_cur_send.cmd.resp)
        {
            data_4g_cur_send.cmd.resp(&data_4g_cur_send, NULL, NULL);
        }
        data_4g_cur_send.cmd.state = AT_CMD_STATE_IDLE;
        data_4g_cur_send.cmd.resp = NULL;
        data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
        sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
        data_4g_cur_send.cmd.sharm_index = -1;
    }
    else
    {
        if(TRUE == at_4g_trans_has_higherpri_data(data_4g_cur_send.pri))
        {
            MODULE_LOG_I(TBOX4G, "the higher priority data is need to be send");

            data_4g_cur_send.resp_code = AT_4G_RESP_ABORT;
            if(NULL != data_4g_cur_send.cmd.resp)
            {
                data_4g_cur_send.cmd.resp(&data_4g_cur_send, NULL, NULL);
            }
            data_4g_cur_send.cmd.state = AT_CMD_STATE_IDLE;
            data_4g_cur_send.cmd.resp = NULL;
            data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
            sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
            data_4g_cur_send.cmd.sharm_index = -1;
            return;
        }

        data_4g_cur_send.cmd.state = data_4g_cur_send.bak_state;
        data_4g_cur_send.cmd.retry_count--;
        at_4g_trans_current_tick = data_4g_cur_send.cmd.tick/PERIODIC_UNIT_4G+1;
        dev_4g_check_send();
    }
}

void at_4g_transmit_monitor_queue(void)
{
    uint8 index;
    INT32 temp_value;
    AT_4G_CMD_RESP temp_resp;

    for(index = 0; index < AT_4G_HIGHCMD_QUEUE_COUNT; index++)
    {
        if(1 == at_4g_trans_highqueue[index].is_used)
        {
            temp_value = at_4g_trans_highqueue[index].cmd.tick  - PERIODIC_UNIT_4G;
            if(temp_value <= 0)
            {
                if(at_4g_trans_highqueue[index].cmd.retry_count == 0)
                {
                    MODULE_LOG_E(TBOX4G, "transmit timeout cmd:%d", at_4g_trans_highqueue[index].cmd.cmd_id);

                    memcpy(&temp_resp.cmd, &at_4g_trans_highqueue[index].cmd, sizeof(temp_resp.cmd));
                    temp_resp.resp_code = AT_4G_RESP_TIMEOUT;
                    if(NULL != temp_resp.cmd.resp)
                    {
                        temp_resp.cmd.resp(&temp_resp, NULL, NULL);
                    }
                    sharemem_4g_free(temp_resp.cmd.sharm_index);

                    at_4g_trans_highqueue[index].is_used = 0;
                    at_4g_trans_highqueue[index].cmd.state = AT_CMD_STATE_IDLE;
                    at_4g_trans_highqueue[index].cmd.resp = NULL;
                    at_4g_trans_highqueue[index].cmd.sharm_index = -1;
                    at_4g_trans_highqueue[index].cmd.tick = 0;
                }
                else
                {
                    at_4g_trans_highqueue[index].cmd.retry_count--;
                    at_4g_trans_highqueue[index].cmd.tick =  at_4g_trans_highqueue[index].cmd.org_tick;
                }
            }
            else
            {
                at_4g_trans_highqueue[index].cmd.tick = (uint16)temp_value;
            }
        }
    }

    for(index = 0; index < AT_4G_MIDCMD_QUEUE_COUNT; index++)
    {
        if(1 == at_4g_trans_midqueue[index].is_used)
        {
            temp_value = at_4g_trans_midqueue[index].cmd.tick  - PERIODIC_UNIT_4G;
            if(temp_value <= 0)
            {
                if(at_4g_trans_midqueue[index].cmd.retry_count == 0)
                {
                    MODULE_LOG_E(TBOX4G, "transmit timeout cmd:%d", at_4g_trans_midqueue[index].cmd.cmd_id);

                    memcpy(&temp_resp.cmd, &at_4g_trans_midqueue[index].cmd, sizeof(temp_resp.cmd));
                    temp_resp.resp_code = AT_4G_RESP_TIMEOUT;
                    if(NULL != temp_resp.cmd.resp)
                    {
                        temp_resp.cmd.resp(&temp_resp, NULL, NULL);
                    }
                    sharemem_4g_free(temp_resp.cmd.sharm_index);

                    at_4g_trans_midqueue[index].is_used = 0;
                    at_4g_trans_midqueue[index].cmd.state = AT_CMD_STATE_IDLE;
                    at_4g_trans_midqueue[index].cmd.resp = NULL;
                    at_4g_trans_midqueue[index].cmd.sharm_index = -1;
                    at_4g_trans_midqueue[index].cmd.tick = 0;
                }
                else
                {
                    at_4g_trans_midqueue[index].cmd.retry_count--;
                    at_4g_trans_midqueue[index].cmd.tick = at_4g_trans_midqueue[index].cmd.org_tick;
                }
            }
            else
            {
                at_4g_trans_midqueue[index].cmd.tick = (uint16)temp_value;
            }
        }
    }

    for(index = 0; index < AT_4G_LOWCMD_QUEUE_COUNT; index++)
    {
        if(1 == at_4g_trans_lowqueue[index].is_used)
        {
            temp_value = at_4g_trans_lowqueue[index].cmd.tick  - PERIODIC_UNIT_4G;
            if(temp_value <= 0)
            {
                if(at_4g_trans_lowqueue[index].cmd.retry_count == 0)
                {
                    MODULE_LOG_E(TBOX4G, "transmit timeout cmd:%d", at_4g_trans_lowqueue[index].cmd.cmd_id);

                    memcpy(&temp_resp.cmd, &at_4g_trans_lowqueue[index].cmd, sizeof(temp_resp.cmd));
                    temp_resp.resp_code = AT_4G_RESP_TIMEOUT;
                    if(NULL != temp_resp.cmd.resp)
                    {
                        temp_resp.cmd.resp(&temp_resp, NULL, NULL);
                    }
                    sharemem_4g_free(temp_resp.cmd.sharm_index);

                    at_4g_trans_lowqueue[index].is_used = 0;
                    at_4g_trans_lowqueue[index].cmd.state = AT_CMD_STATE_IDLE;
                    at_4g_trans_lowqueue[index].cmd.resp = NULL;
                    at_4g_trans_lowqueue[index].cmd.sharm_index = -1;
                    at_4g_trans_lowqueue[index].cmd.tick = 0;
                }
                else
                {
                    at_4g_trans_lowqueue[index].cmd.retry_count--;
                    at_4g_trans_lowqueue[index].cmd.tick = at_4g_trans_lowqueue[index].cmd.org_tick;
                }
            }
            else
            {
                at_4g_trans_lowqueue[index].cmd.tick = (uint16)temp_value;
            }
        }
    }
}

uint8 at_4g_transmit_direct_send(uint8 *data, uint16 len, DEV_4G_SEND_CALLBACK callback)
{
    if((INT32)TBOX_E_OK == dev_4g_direct_send(data, len, callback))
    {
        return AT_4G_TRANS_SEND_OK;
    }

    return AT_4G_TRANS_SEND_IS_BUSY;
}

uint8 at_4g_transmit_direct_setcmd(AT_4G_CMD *cmd)
{
    MODULE_LOG_I(TBOX4G, "direct send data, cmd:%d data_index:%d", cmd->cmd_id, cmd->sharm_index);

    if(NULL != at_trans_filter_fun[AT_4G_DIRECT_TRANS_FILTER] &&
        AT_4G_TRANS_NONEEDSEND_DATA   == at_trans_filter_fun[AT_4G_DIRECT_TRANS_FILTER]())
    {
        return AT_4G_TRANS_SEND_IS_BUSY;
    }

    if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
    {
        return AT_4G_TRANS_SEND_IS_BUSY;
    }

    memcpy(&data_4g_cur_send.cmd, cmd, sizeof(data_4g_cur_send.cmd));
    data_4g_cur_send.pri = AT_4G_CMD_DIRECT;
    data_4g_cur_send.bak_state = data_4g_cur_send.cmd.state;
    data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
    at_4g_trans_current_tick = data_4g_cur_send.cmd.tick/PERIODIC_UNIT_4G+1;

    return AT_4G_TRANS_SEND_OK;
}

uint8 at_4g_transmit_putcmd(AT_4G_CMD_PRIORITY pri, AT_4G_CMD *cmd)
{
    uint8 index, count;
    AT_4G_CMD_QUEUE_ELE *queue = NULL;

    MODULE_LOG_I(TBOX4G, "send data, pri:%d cmd:%d data_index:%d", pri, cmd->cmd_id, cmd->sharm_index);

    cmd->org_tick = cmd->tick;

    switch(pri)
    {
        case AT_4G_CMD_LOW:
            queue = at_4g_trans_lowqueue;
            count = AT_4G_LOWCMD_QUEUE_COUNT;
            break;

        case AT_4G_CMD_MID:
            queue = at_4g_trans_midqueue;
            count = AT_4G_MIDCMD_QUEUE_COUNT;
            break;

        case AT_4G_CMD_HIGH:
            queue = at_4g_trans_highqueue;
            count = AT_4G_HIGHCMD_QUEUE_COUNT;
            break;

        default:
            break;
    }

    if(NULL == queue)
    {
        return 1;
    }
    for(index = 0; index < count; index++)
    {
        if(0 == queue[index].is_used)
        {
            memcpy(&queue[index].cmd, cmd, sizeof(queue[index].cmd));
            queue[index].is_used = 1;
            return 0;
        }
    }

    return 1;
}

uint8 at_4g_transmit_reg_callback(AT_4G_DECODE_CALLBACK_TYPE type, AT_4G_DECODE_CALLBACK callback)
{
    if(type >= AT_4G_DECODE_CALLBACK_TYPE_MAX)
    {
        return 1;
    }

    at_decode_callbak[type] = callback;
    return 0;
}

uint8 at_4g_transmit_reg_filter(AT_4G_TRANSMIT_FILTER filter, AT_4G_TRANS_FILTER_FUN filter_fun)
{
    if(filter >= AT_4G_TRANS_FILTER_MAX)
    {
        return 1;
    }

    at_trans_filter_fun[filter] = filter_fun;
    return 0;
}

static uint8 at_4g_trans_parse_iurc(uint8 *data, uint16 *len)
{
    uint8 *token = NULL;
    uint8 *temp_ptr = NULL;
    uint16 temp_len = 0;
    uint8 ret;

    temp_len = *len;
    token = tbox_string_get_substring(data, temp_len, "+QIURC");
    if(NULL == token)
    {
        return AT_4G_DECODE_NOMATCH;
    }
    if(temp_len <= (uint16)(token-data))
    {
        return AT_4G_DECODE_CONTINUE;
    }

    temp_len = temp_len - (uint16)(token-data);

	/*check receive data*/
	 temp_ptr = tbox_string_get_substring(token, temp_len, ": \"recv\",");
	 if(NULL != temp_ptr) /*+QIURC: "recv",<connectID>,<currentrecvlength><CR><LF><data>*/
	 {
		 uint32  conn_id = 0;
		 uint32 data_len = 0;
		 uint16 size = strlen("+QIURC: \"recv\",");
	
		 if(temp_len <= size)
		 {
			 return AT_4G_DECODE_CONTINUE;
		 }
	
		 temp_len = temp_len - size;
		 temp_ptr = token + size;
	
		 token = tbox_string_get_substring(temp_ptr, temp_len, ",");
		 if(NULL == token)
		 {
			 return AT_4G_DECODE_CONTINUE;
		 }
		 if(FALSE == tbox_string_get_num(temp_ptr, ',', (uint32*)&conn_id))
		 {
			 return AT_4G_DECODE_INVALID_PACKET;
		 }
		 if(conn_id >= 10)
		 {
			 temp_len = temp_len - 3;
			 temp_ptr = temp_ptr + 3;
		 }
		 else
		 {
			 temp_len = temp_len - 2;
			 temp_ptr = temp_ptr + 2;
		 }
	
		 token = tbox_string_get_substring(temp_ptr, temp_len, AT_4G_RESP_SUFFIX);
		 if(NULL == token)
		 {
			 return AT_4G_DECODE_CONTINUE;
		 }
		 if(FALSE == tbox_string_get_num(temp_ptr, '\r', (uint32*)&data_len))
		 {
			 return AT_4G_DECODE_INVALID_PACKET;
		 }
		 size = (uint16)(token-temp_ptr);
		 temp_len = temp_len - size - 2;
		 temp_ptr = temp_ptr + size + 2;
		 if(temp_len <= data_len)
		 {
			 return AT_4G_DECODE_CONTINUE;
		 }
	
		 if(NULL != at_decode_callbak[AT_4G_DECODE_DATAIN_IND])
		 {
			 at_decode_callbak[AT_4G_DECODE_DATAIN_IND](&conn_id, temp_ptr, (uint16 *)&data_len);
		 }
	
		 if(data_len > temp_len)
		 {
			 return AT_4G_DECODE_CONTINUE;
		 }
	
		 token = temp_ptr + data_len;
		 temp_len = temp_len - data_len;
		 if('\r' == *token && temp_len > 0)
		 {
			 token++;
			 temp_len--;
		 }
		 if('\n' == *token && temp_len > 0)
		 {
			 token++;
			 temp_len--;
		 }

		 MODULE_LOG_I(TBOX4G, "receive datain notify");

		 *len = *len - temp_len;
		 return AT_4G_DECODE_ISMATCH;
	 }

    temp_ptr = tbox_string_get_substring(token, temp_len, ": \"closed\",");
    if(NULL != temp_ptr) /*+QIURC: "closed",<connectID>*/
    {
        uint32 conn_id = 0;

        temp_ptr = tbox_string_get_substring(token, temp_len, AT_4G_RESP_SUFFIX);
        if(NULL == temp_ptr)
        {
            return AT_4G_DECODE_CONTINUE;
        }
        if(temp_len <= strlen("+QIURC: \"closed\","))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        temp_len = temp_len - strlen("+QIURC: \"closed\",");
        token = token + strlen("+QIURC: \"closed\",");
        if(FALSE == tbox_string_get_num(token, '\r', (uint32*)&conn_id))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        *len = (uint16)(temp_ptr-data) + 2;

        if(conn_id > 11)
        {
            MODULE_LOG_E(TBOX4G, "the connid[%d] is invalid", conn_id);
            return AT_4G_DECODE_ISMATCH;
        }

        temp_len = sizeof(conn_id);
        if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
        {
            data_4g_cur_send.resp_code = AT_4G_RESP_IURC_CLOSE;
            ret = data_4g_cur_send.cmd.resp(&data_4g_cur_send, (uint8*)&conn_id, &temp_len);
            if(AT_4G_DECODE_ISMATCH == ret)
            {
                data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_WAITRESP);
                if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
                {
                    data_4g_cur_send.cmd.resp = NULL;
                    data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
                    sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
                    data_4g_cur_send.cmd.sharm_index = -1;
                    at_4g_trans_current_tick = 0;
                }
            }
        }
        if(NULL != at_decode_callbak[AT_4G_DECODE_STATE_IND])
        {
            uint8 resp = AT_4G_RESP_IURC_CLOSE;
            at_decode_callbak[AT_4G_DECODE_STATE_IND](&resp,  (uint8*)&conn_id, &temp_len);
        }

        MODULE_LOG_I(TBOX4G, "receive connect[%d] close notify", conn_id);

        return AT_4G_DECODE_ISMATCH;
    }

    temp_ptr = tbox_string_get_substring(token, temp_len, ": \"incoming full\"");
    if(NULL != temp_ptr) /*+QIURC: "incoming full"*/
    {
        temp_ptr = tbox_string_get_substring(token, temp_len, AT_4G_RESP_SUFFIX);
        if(NULL == temp_ptr)
        {
            return AT_4G_DECODE_CONTINUE;
        }

        *len = (uint16)(temp_ptr-data) + 2;

        if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
        {
            data_4g_cur_send.resp_code = AT_4G_RESP_IURC_FULL;
            ret = data_4g_cur_send.cmd.resp(&data_4g_cur_send, NULL, NULL);
            if(AT_4G_DECODE_ISMATCH == ret)
            {
                data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_WAITRESP);
                if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
                {
                    data_4g_cur_send.cmd.resp = NULL;
                    data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
                    sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
                    data_4g_cur_send.cmd.sharm_index = -1;
                    at_4g_trans_current_tick = 0;
                }
            }
        }
        if(NULL != at_decode_callbak[AT_4G_DECODE_STATE_IND])
        {
            uint8 resp = AT_4G_RESP_IURC_FULL;
            at_decode_callbak[AT_4G_DECODE_STATE_IND](&resp, NULL, NULL);
        }

        MODULE_LOG_I(TBOX4G, "receive incoming full notify");

        return AT_4G_DECODE_ISMATCH;
    }

    temp_ptr = tbox_string_get_substring(token, temp_len, ": \"pdpdeact\",");
    if(NULL != temp_ptr) /*+QIURC: "pdpdeact",<contextID>*/
    {
        uint32 context_id = 0;

        temp_ptr = tbox_string_get_substring(token, temp_len, AT_4G_RESP_SUFFIX);
        if(NULL == temp_ptr)
        {
            return AT_4G_DECODE_CONTINUE;
        }
        if(temp_len <= strlen("+QIURC: \"pdpdeact\","))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        temp_len = temp_len - strlen("+QIURC: \"pdpdeact\",");
        token = token + strlen("+QIURC: \"pdpdeact\",");
        if(FALSE == tbox_string_get_num(token, '\r', (uint32*)&context_id))
        {
            return AT_4G_DECODE_INVALID_PACKET;
        }

        *len = (uint16)(temp_ptr-data) + 2;

        if(context_id < 1 || context_id > 16)
        {
            MODULE_LOG_E(TBOX4G, "the context id[%d] is invalid", context_id);
            return AT_4G_DECODE_ISMATCH;
        }

        temp_len = sizeof(context_id);
        if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
        {
            data_4g_cur_send.resp_code = AT_4G_RESP_IURC_DEACTIVE;
            ret = data_4g_cur_send.cmd.resp(&data_4g_cur_send, (uint8*)&context_id, &temp_len);
            if(AT_4G_DECODE_ISMATCH == ret)
            {
                data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_WAITRESP);
                if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
                {
                    data_4g_cur_send.cmd.resp = NULL;
                    data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
                    sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
                    data_4g_cur_send.cmd.sharm_index = -1;
                    at_4g_trans_current_tick = 0;
                }
            }
        }
        if(NULL != at_decode_callbak[AT_4G_DECODE_STATE_IND])
        {
            uint8 resp = AT_4G_RESP_IURC_DEACTIVE;
            at_decode_callbak[AT_4G_DECODE_STATE_IND](&resp,  (uint8*)&context_id, &temp_len);
        }
        
        MODULE_LOG_I(TBOX4G, "receive context[%d] deactive notify", context_id);

        return AT_4G_DECODE_ISMATCH;
    }

    return AT_4G_DECODE_NOMATCH;
}

static uint8 at_4g_trans_parse_state(uint8 *data, uint16 *len)
{
    uint8 *token = NULL;
    uint8 *temp_ptr = NULL;
    uint16 temp_len = 0;
    uint8 resp_code = AT_4G_RESP_UNKNOWN;

    temp_len = *len;
    temp_ptr = data;
    token = tbox_string_get_substring(temp_ptr, temp_len, AT_4G_RESP_SUFFIX); /*end \r\n*/
    if(NULL == token)
    {
        return AT_4G_DECODE_CONTINUE;
    }

    if((uint16)(token - data + strlen(AT_4G_RESP_SUFFIX)) > *len)
    {
        return AT_4G_DECODE_INVALID_PACKET;
    }
    *len = (uint16)(token - data + strlen(AT_4G_RESP_SUFFIX));

    token = tbox_string_get_substring(temp_ptr, temp_len, "+CME ERROR: ");
    if(NULL != token)
    {
        resp_code = at_4g_get_cme_errorcode(token, temp_len);
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "+CMS ERROR: ");
    if(NULL != token)
    {
        resp_code = at_4g_get_cms_errorcode(token, temp_len);
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "OK");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_OK;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "> ");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_SENDOK;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "SEND OK");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_SENDOK;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "ERROR");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_ERROR;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "BUSY");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_BUSY;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "CONNECT");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_CONNECT;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "+CPIN:");
    if(NULL != token)
    {
        token = tbox_string_get_substring(temp_ptr, temp_len, "READY");
        if(NULL != token)
        {
            resp_code = AT_4G_RESP_CPIN_READY;
            goto end_decode;
        }
        token = tbox_string_get_substring(temp_ptr, temp_len, "NOT INSERTED");
        if(NULL != token)
        {
            resp_code = AT_4G_RESP_CPIN_NOINSERT;
            goto end_decode;
        }
        token = tbox_string_get_substring(temp_ptr, temp_len, "PIN");
        if(NULL != token)
        {
            resp_code = AT_4G_RESP_CPIN_PIN;
            goto end_decode;
        }
        token = tbox_string_get_substring(temp_ptr, temp_len, "PUK");
        if(NULL != token)
        {
            resp_code = AT_4G_RESP_CPIN_PUK;
            goto end_decode;
        }
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "+QINISTAT:");
    if(NULL != token)
    {
        resp_code = at_4g_get_initstate_code(temp_ptr, temp_len);
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "+CFUN:");
    if(NULL != token)
    {
        resp_code = at_4g_get_fun_code(temp_ptr, temp_len);
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "+QIND: SMS DONE");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_SMS_DONE;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "+QIND: PB DONE");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_PB_DONE;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "RDY");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_ME_READY;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "RING");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_RING;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "NO ANSWER");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_NO_ANSWER;
        goto end_decode;
    }

    token = tbox_string_get_substring(temp_ptr, temp_len, "NO CARRIER");
    if(NULL != token)
    {
        resp_code = AT_4G_RESP_NO_CARRIER;
        goto end_decode;
    }

    return AT_4G_DECODE_NOMATCH;

end_decode:
    if(AT_4G_RESP_UNKNOWN != resp_code)
    {
        MODULE_LOG_I(TBOX4G, "receive state notify, code:%d cmd:%d", resp_code, data_4g_cur_send.cmd.cmd_id);

        if(AT_CMD_STATE_IDLE != data_4g_cur_send.cmd.state)
        {
            data_4g_cur_send.resp_code = resp_code;
            if(AT_4G_DECODE_ISMATCH == data_4g_cur_send.cmd.resp(&data_4g_cur_send, NULL, NULL))
            {
                data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_WAITRESP);
                if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
                {
                    data_4g_cur_send.cmd.resp = NULL;
                    data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
                    sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
                    data_4g_cur_send.cmd.sharm_index = -1;
                    at_4g_trans_current_tick = 0;
                }
            }
        }
        if(NULL != at_decode_callbak[AT_4G_DECODE_STATE_IND])
        {
            at_decode_callbak[AT_4G_DECODE_STATE_IND](&resp_code, NULL, NULL);
        }
    }
    return AT_4G_DECODE_ISMATCH;
}

static boolean at_4g_trans_has_higherpri_data(uint8 pri)
{
    uint8 index;

    switch(pri)
    {
        case AT_4G_CMD_LOW:
            for(index = 0; index < AT_4G_MIDCMD_QUEUE_COUNT; index++)
            {
                if(1 == at_4g_trans_midqueue[index].is_used)
                {
                    return TRUE;
                }
            }
            for(index = 0; index < AT_4G_HIGHCMD_QUEUE_COUNT; index++)
            {
                if(1 == at_4g_trans_highqueue[index].is_used)
                {
                    return TRUE;
                }
            }
            break;

        case AT_4G_CMD_MID:
            for(index = 0; index < AT_4G_HIGHCMD_QUEUE_COUNT; index++)
            {
                if(1 == at_4g_trans_highqueue[index].is_used)
                {
                    return TRUE;
                }
            }
            break;

        case AT_4G_CMD_HIGH:
            break;

        default:
            break;
    }

    return FALSE;
}

static uint8  at_4g_get_cme_errorcode(uint8 *data, uint16 len)
{
    uint8 resp = AT_4G_RESP_UNKNOWN;
    uint8 index;
    uint32 code = 0;

    for(index = 0; index < len; index++)
    {
        if(isdigit(data[index]))
        {
           break;
        }
    }
    if(index >= len)
    {
        return resp;
    }
    
    if(FALSE == tbox_string_get_num(data+index, '\r', &code))
    {
        return resp;
    }

    switch(code)
    {
        case 3:
            resp = AT_4G_RESP_CMEERR_NOALLOWED;
            break;
        case 4:
            resp = AT_4G_RESP_CMEERR_NOSUPPORT;
            break;
        case 10:
            resp = AT_4G_RESP_CMEERR_SIMNOINSERT;
            break;
        case 11:
            resp = AT_4G_RESP_CMEERR_PINREQ;
            break;
        case 12:
            resp = AT_4G_RESP_CMEERR_PUKREQ;
            break;
        case 13:
            resp = AT_4G_RESP_CMEERR_SIMFAILED;
            break;
        case 14:
            resp = AT_4G_RESP_CMEERR_SIMBUSY;
            break;
        case 15:
            resp = AT_4G_RESP_CMEERR_SIMWRONG;
            break;
        case 20:
            resp = AT_4G_RESP_CMEERR_MEMFULL;
            break;
        case 21:
            resp = AT_4G_RESP_CMEERR_INVALID;
            break;
        case 22:
            resp = AT_4G_RESP_CMEERR_NOFIND;
            break;
        case 24:
            resp = AT_4G_RESP_CMEERR_TOOLONG;
            break;
        case 30:
            resp = AT_4G_RESP_CMEERR_NONETSERVICE;
            break;
        case 31:
            resp = AT_4G_RESP_CMEERR_NETTIMEOUT;
            break;
        default:
            resp = AT_4G_RESP_CMEERR_OTHER;
            break;
    }

    MODULE_LOG_I(TBOX4G, "receive cme error resp:%d", code);

    return resp;
}

static uint8  at_4g_get_cms_errorcode(uint8 *data, uint16 len)
{
    uint8 resp = AT_4G_RESP_UNKNOWN;
    uint16 code = 0;
    uint8 index;

    for(index = 0; index < len; index++)
    {
        if(isdigit(data[index]))
        {
           break;
        }
    }
    if(index >= len)
    {
        return resp;
    }
    if(FALSE == tbox_string_get_num(data+index, '\r', (uint32*)&code))
    {
        return resp;
    }

    switch(code)
    {
        case 302:
            resp = AT_4G_RESP_CMSERR_NOALLOWED;
            break;
        case 303:
            resp = AT_4G_RESP_CMSERR_NOSUPPORT;
            break;
        case 304:
            resp = AT_4G_RESP_CMSERR_INVALID_PDU;
            break;
        case 305:
            resp = AT_4G_RESP_CMSERR_INVALID_TXT;
            break;
        case 310:
            resp = AT_4G_RESP_CMSERR_SIMNOINSERT;
            break;
        case 313:
            resp = AT_4G_RESP_CMSERR_SIMFAILED;
            break;
        case 314:
            resp = AT_4G_RESP_CMSERR_SIMBUSY;
            break;
        case 322:
            resp = AT_4G_RESP_CMSERR_MEMFULL;
            break;
        case 331:
            resp = AT_4G_RESP_CMSERR_NONET;
            break;
        case 332:
            resp = AT_4G_RESP_CMSERR_NETTIMEOUT;
            break;
        case 512:
            resp = AT_4G_RESP_CMSERR_SIMNOTREADY;
            break;
        case 514:
            resp = AT_4G_RESP_CMSERR_INVALIDPARAM;
            break;
        case 517:
            resp = AT_4G_RESP_CMSERR_INVALIDSVCMOD;
            break;
        default:
            break;
    }

    MODULE_LOG_I(TBOX4G, "receive cms error resp:%d", resp);

    return resp;
}

static uint8  at_4g_get_initstate_code(uint8 *data, uint16 len)
{
    uint8 resp = AT_4G_RESP_UNKNOWN;
    uint8 code = 0;
    uint8 index;

    for(index = 0; index < len; index++)
    {
        if(isdigit(data[index]))
        {
           break;
        }
    }
    if(index >= len)
    {
        return resp;
    }
    if(FALSE == tbox_string_get_num(data+index, '\r', (uint32*)&code))
    {
        return resp;
    }

    switch(code)
    {
        case 0:
            resp = AT_4G_RESP_SIM_INIT;
            break;
        case 1:
            resp = AT_4G_RESP_SIM_READY;
            break;
        case 2:
            resp = AT_4G_RESP_SIM_SMSDONE;
            break;
        case 3:
            resp = AT_4G_RESP_SIM_READY_SMSDONE;
            break;
        case 4:
            resp = AT_4G_RESP_SIM_PBDONE;
            break;
        case 7:
            resp = AT_4G_RESP_SIM_ALL;
            break;
        default:
            break;
    }

    MODULE_LOG_I(TBOX4G, "receive initstate code:%d", resp);

    return resp;
}

static uint8  at_4g_get_fun_code(uint8 *data, uint16 len)
{
    uint8 resp = AT_4G_RESP_UNKNOWN;
    uint8 code = 0;
    uint8 index;

    for(index = 0; index < len; index++)
    {
        if(isdigit(data[index]))
        {
           break;
        }
    }
    if(index >= len)
    {
        return resp;
    }
    if(FALSE == tbox_string_get_num(data+index, '\r', (uint32*)&code))
    {
        return resp;
    }

    switch(code)
    {
        case 0:
            resp = AT_4G_RESP_FUN_MINI;
            break;
        case 1:
            resp = AT_4G_RESP_FUN_FULL;
            break;
        case 4:
            resp = AT_4G_RESP_FUN_DISABLE_TRANS;
            break;
        default:
            break;
    }

    MODULE_LOG_I(TBOX4G, "receive function code:%d", resp);

    return resp;
}

static void at_4g_remove_check_iuc(uint8 *data, uint16 len)
{
    uint8 ret;
    uint8 *temp_ptr = data;
    uint16 data_len = len;
    uint16 temp_len = 0;
    uint16 index;

    if(0 == len)
    {
        return;
    }

    for(index = 0; index < len; index++)
    {
        temp_len = data_len;
        ret = at_4g_trans_parse_iurc(temp_ptr, &temp_len);
        if(AT_4G_DECODE_ISMATCH == ret ||
           AT_4G_DECODE_INVALID_PACKET == ret)
        {
           if(0 == temp_len ||
              temp_len >= data_len)
           {
               break;
           }
           data_len -= temp_len;
           temp_ptr += temp_len;
        }
        else
        {
            break;
        }
    }
}
