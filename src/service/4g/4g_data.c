#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_data.h"
#include "4g_at.h"
#include "4g_sharememery.h"

#define DATA_4G_RECV_BUFF_LEN (2686)
#define DATA_4G_TEMP_BUFF_LEN (2560)

AT_4G_CMD_RESP data_4g_cur_send;
static uint8 data_4g_recv_buf[DATA_4G_RECV_BUFF_LEN];
static uint16 data_4g_recv_len;
static uint8 data_4g_temp_buff[DATA_4G_TEMP_BUFF_LEN];
static uint16 data_4g_temp_len;

void data_4g_init(void)
{
    data_4g_recv_len = 0;
    data_4g_cur_send.cmd.sharm_index = -1;
    data_4g_cur_send.cmd.state = 0;
    data_4g_cur_send.cmd.retry_count = 0;
    data_4g_cur_send.cmd.resp = NULL;
    data_4g_cur_send.cmd.cmd_id = AT_4G_UNKNOWN_CMD;
    memset(data_4g_recv_buf, 0, sizeof(data_4g_recv_buf));

    data_4g_temp_len = 0;
    memset(data_4g_temp_buff, 0, sizeof(data_4g_temp_buff));
}

VOID data_4g_recv_push(UINT8 data)
{
   //NVIC_DisableIRQ(UART1_IRQn); /*中断已经关闭中断*/

    if(data_4g_temp_len >= DATA_4G_TEMP_BUFF_LEN)
    {
        data_4g_temp_len = 0;
        memset(data_4g_temp_buff, 0, sizeof(data_4g_temp_buff));
        
        //NVIC_EnableIRQ(UART1_IRQn);

        //log_e(MODULE_ID_SVR, "the temp buffer is overflow");
        return;
    }

    data_4g_temp_buff[data_4g_temp_len] = data;
    data_4g_temp_len++;

	//NVIC_EnableIRQ(UART1_IRQn);
}

uint8* data_4g_recv_pull(uint16 *len)
{
    uint8 *temp_ptr = NULL;

    NVIC_DisableIRQ(UART1_IRQn);

    if((data_4g_temp_len+data_4g_recv_len) >= DATA_4G_RECV_BUFF_LEN)
    {
        data_4g_recv_len = 0;
        MODULE_LOG_E(TBOX4G, "the buffer is overflow");
    }

    if(0 != data_4g_temp_len)
    {
        temp_ptr = data_4g_recv_buf+data_4g_recv_len;
        memcpy(temp_ptr, data_4g_temp_buff, data_4g_temp_len);
        data_4g_recv_len = data_4g_recv_len+data_4g_temp_len;

        data_4g_temp_len = 0;
    }

    *len = data_4g_recv_len;

	NVIC_EnableIRQ(UART1_IRQn);

    return data_4g_recv_buf;
}

void data_4g_recv_remove(uint16 remove_len)
{
    NVIC_DisableIRQ(UART1_IRQn);
    if(remove_len >= data_4g_recv_len)
    {
        data_4g_recv_len = 0;
        memset(data_4g_recv_buf, 0, sizeof(data_4g_recv_buf));
    }
    else
    {
        data_4g_recv_len = data_4g_recv_len - remove_len;
        memmove(data_4g_recv_buf, data_4g_recv_buf+remove_len, data_4g_recv_len);
    }
    NVIC_EnableIRQ(UART1_IRQn);
}

uint8* data_4g_get_send_data(uint16 *len)
{
    sharemem_4g_ele *ele = NULL;

    if((AT_CMD_STATE_REQ != (data_4g_cur_send.cmd.state & AT_CMD_STATE_REQ)) ||
        data_4g_cur_send.cmd.sharm_index < 0)
    {
        *len = 0;
        return NULL;
    }

    ele = sharemem_4g_getmem(data_4g_cur_send.cmd.sharm_index);
    if(NULL == ele)
    {
        *len = 0;
        return NULL;
    }

    data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_REQ);
    *len = ele->len;
    return ele->mem_ptr;
}

void data_4g_send_finish_ind(void)
{
    if((AT_CMD_STATE_WAITSEND != (data_4g_cur_send.cmd.state & AT_CMD_STATE_WAITSEND)))
    {
        return;
    }

    data_4g_cur_send.cmd.state &= (~AT_CMD_STATE_WAITSEND);
    if(AT_CMD_STATE_IDLE == data_4g_cur_send.cmd.state)
    {
        if(NULL != data_4g_cur_send.cmd.resp)
        {
            data_4g_cur_send.resp_code = AT_4G_RESP_SENDCMP;
            data_4g_cur_send.cmd.resp(&data_4g_cur_send, NULL, 0);
            data_4g_cur_send.cmd.resp = NULL;
            data_4g_cur_send.resp_code = AT_4G_RESP_UNKNOWN;
        }
        sharemem_4g_free(data_4g_cur_send.cmd.sharm_index);
        data_4g_cur_send.cmd.sharm_index = -1;
    }
}

