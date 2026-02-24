#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_memory.h"
#include "flash_common.h"
#include "time_if.h"

#include "hpm_content.h"
#include "hpm_session.h"
#include "hpm_pack.h"
#include "hpm_net.h"
#include "hpm_mgr.h"
#include "hpm_data.h"
#include "hpm_control.h"
#include "hpm_data_recv.h"
#include "hpm_socket.h"

#define HPM_SESSION_RUN_INFO_MIGC (0x11223344)

#define HPM_SESSION_MAX_RECV_LEN (256)
#define HPM_SESSION_MAX_SEND_LEN (1024)
#define HPM_SESSION_LOG_TIMEOUT (10)
#define HPM_SESSION_RETRY_CNT (3)

typedef INT32 (*hpm_send_handle_func)(UINT8 resp);

typedef enum
{
    HPM_SESSION_SEND_INIT = 0,
    HPM_SESSION_SEND_WAIT = 1,
    HPM_SESSION_SEND_RETRY = 2,
    HPM_SESSION_SEND_SUCCESS = 3,
    HPM_SESSION_SEND_MAX
} hpm_session_send_status_e;

typedef enum
{
    HPM_SESSION_LOGIN_ACK = 1 << 0,
    HPM_SESSION_LOGIN_NACK = 1 << 1,
    HPM_SESSION_REPORT_ACK = 1 << 2,
    HPM_SESSION_REPORT_NACK = 1 << 3,
    HPM_SESSION_HEARTBEAT_ACK = 1 << 4,
    HPM_SESSION_HEARTBEAT_NACK = 1 << 5,
    HPM_SESSION_LOGOUT_ACK = 1 << 6,
    HPM_SESSION_LOGOUT_NACK = 1 << 7,
} hpm_session_recv_resp_e;

typedef enum
{
    HPM_SESSION_EVENT_LOGIN = 0,
    HPM_SESSION_EVENT_LOGOUT = 1,
    HPM_SESSION_EVENT_HEARTBEAT = 2,
    HPM_SESSION_EVENT_REPORT = 3,
    HPM_SESSION_EVENT_MAX
} hpm_session_event_e;

typedef struct
{
    UINT8 retry_count;
    UINT8 request;
    UINT8 state;
    UINT8 result;
    UINT32 wait_time;
    hpm_send_handle_func handle;
} hpm_session_send_info_t;

typedef struct
{
    UINT32 migc;
    UINT32 login_seq_info;
    UINT32 data_seq_info;
} HPM_SESSION_RUN_INFO_T;

typedef enum
{
    HPM_SESSION_STEP_INIT = 0x00,
    HPM_SESSION_STEP_LOGIN = 0x01,
    HPM_SESSION_STEP_SESSION = 0x02,
    HPM_SESSION_STEP_LOGOUT = 0x03,
    HPM_SESSION_STEP_MAX
} HPM_SESSION_STEP_E;

typedef struct
{
    UINT16 cmd;
    UINT16 seq_id;
    UINT16 sub_cmd;
    UINT16 resp_flag;
    UINT16 data_len;
    UINT8 data[HPM_SESSION_MAX_RECV_LEN];
} HPM_SESSION_RECV_INFO;

static INT32 hpm_session_send_login(UINT8 resp);
static INT32 hpm_session_send_logout(UINT8 resp);
static INT32 hpm_session_send_heartbeat(UINT8 resp);
static INT32 hpm_session_send_report(UINT8 resp);

static HPM_SESSION_RUN_INFO_T hpm_session_run_info;
static HPM_SESSION_STEP_E hpm_session_step;
static HPM_SESSION_RECV_INFO hpm_session_recv_info;

static hpm_session_send_info_t hpm_session_send_info[HPM_SESSION_EVENT_MAX] = {
    {0, 0, 0, 0, 0, hpm_session_send_login},
    {0, 0, 0, 0, 0, hpm_session_send_logout},
    {0, 0, 0, 0, 0, hpm_session_send_heartbeat},
    {0, 0, 0, 0, 0, hpm_session_send_report},
};

static UINT8 hpm_session_send_buffer[HPM_SESSION_MAX_SEND_LEN];

static UINT16 hpm_login_date;
static UINT16 hpm_login_seq;
static UINT16 hpm_data_date;
static UINT16 hpm_data_seq;

static VOID hpm_session_run_info_write(VOID)
{
    INT32 ret = 0;
    hpm_session_run_info.migc = HPM_SESSION_RUN_INFO_MIGC;
    drv_flash_nor_erase(FLASH_NOR_ADDR_HPM_SES_INFO, 1);
    ret = drv_flash_nor_write(FLASH_NOR_ADDR_HPM_SES_INFO, (uint8_t *)&hpm_session_run_info, sizeof(hpm_session_run_info));
    if (0 != ret)
    {
        MODULE_LOG_E(HPM, "hpm session write run info failed, ret: %d", ret);
    }
}

static VOID hpm_session_run_info_read(VOID)
{
    INT32 ret = 0;
    DEV_TIME time;

    ret = drv_flash_nor_read(FLASH_NOR_ADDR_HPM_SES_INFO, (uint8_t *)&hpm_session_run_info, sizeof(hpm_session_run_info));
    if (0 != ret)
    {
        MODULE_LOG_E(HPM, "hpm session run info read failed, ret: %d", ret);
        time_if_get(&time);
        hpm_login_date = time.month * 100 + time.day;
        hpm_login_seq = 1;
        hpm_session_run_info.login_seq_info = (hpm_login_date << 16) | hpm_login_seq;

        hpm_data_date = time.month * 100 + time.day;
        hpm_data_seq = 1;
        hpm_session_run_info.data_seq_info = (hpm_data_date << 16) | hpm_data_seq;
        hpm_session_run_info_write();
        return;
    }

    if (HPM_SESSION_RUN_INFO_MIGC != hpm_session_run_info.migc)
    {
        MODULE_LOG_E(HPM, "hpm session run info migc err, migc: 0x%x", hpm_session_run_info.migc);

        time_if_get(&time);
        hpm_login_date = time.month * 100 + time.day;
        hpm_login_seq = 1;
        hpm_session_run_info.login_seq_info = (hpm_login_date << 16) | hpm_login_seq;

        hpm_data_date = time.month * 100 + time.day;
        hpm_data_seq = 1;
        hpm_session_run_info.data_seq_info = (hpm_data_date << 16) | hpm_data_seq;
        hpm_session_run_info_write();
        return;
    }

    hpm_login_date = hpm_session_run_info.login_seq_info >> 16;
    hpm_login_seq = (UINT16)hpm_session_run_info.login_seq_info & 0x00FF;
    hpm_data_date = hpm_session_run_info.login_seq_info >> 16;
    hpm_data_seq = (UINT16)hpm_session_run_info.login_seq_info & 0x00FF;
    return;
}

INT32 hpm_sesion_get_login_seq(UINT8 *buf)
{
    DEV_TIME time;
    time_if_get(&time);
    if (0 == (hpm_login_seq++))
    {
        hpm_login_seq = 1;
    }
    else if (hpm_login_date != time.month * 100 + time.day)
    {
        hpm_login_date = time.month * 100 + time.day;
        hpm_login_seq = 1;
    }

    buf[0] = hpm_login_seq >> 8;
    buf[1] = hpm_login_seq;

    return 2;
}

INT32 hpm_session_get_logout_seq(UINT8 *buf)
{
    buf[0] = hpm_login_seq >> 8;
    buf[1] = hpm_login_seq;

    return 2;
}

INT32 hpm_sesion_get_data_seq(UINT8 *buf)
{
    DEV_TIME time;
    time_if_get(&time);

    if (0 == (hpm_data_seq++))
    {
        hpm_data_seq = 1;
    }
    else if (hpm_data_date != time.month * 100 + time.day)
    {
        hpm_data_date = time.month * 100 + time.day;
        hpm_data_seq = 1;
    }

    buf[0] = hpm_data_seq >> 8;
    buf[1] = hpm_data_seq;

    return 2;
}

UINT32 hpm_session_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        memset(&hpm_session_run_info, 0, sizeof(hpm_session_run_info));
        memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
        hpm_session_step = HPM_SESSION_STEP_INIT;
        break;

    case MODULE_INIT_SEQ_STORAGE:
        break;

    case MODULE_INIT_SEQ_MODULE:
        hpm_session_run_info_read();
        hpm_data_init();
        break;

    default:
        break;
    }

    return 0;
}

VOID hpm_session_wake(VOID)
{
    memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
    hpm_session_step = HPM_SESSION_STEP_INIT;
    hpm_data_flush_trans_list();
    hpm_data_flush_realtm_data();
}

VOID hpm_session_sleep(VOID)
{
    hpm_session_run_info_write();
    hpm_data_flush_alldata();
}

static INT32 hpm_session_com_login(VOID)
{
    INT32 len = 0;
    UINT8 *buf = NULL;

    buf = mempool_alloc(HPM_SESSION_LOG_MEM_SIZE);
    if (NULL == buf)
    {
        MODULE_LOG_E(HPM, "memalloc login buf failed");
        return -1;
    }

    len = hpm_pack_login(buf);
    if (len <= 0)
    {
        MODULE_LOG_E(HPM, "pack failed");
        mempool_free(buf);
        return -1;
    }

    MODULE_LOG_DUMP(HPM, "hpm login:", buf, len);

    if (0 != hpm_net_send(buf, len))
    {
        hpm_socket_reset();
        MODULE_LOG_E(HPM, "send failed, len: %d", len);
    }

    mempool_free(buf);
    return 0;
}

static INT32 hpm_session_com_logout(VOID)
{
    INT32 len = 0;
    UINT8 *buf = NULL;

    buf = mempool_alloc(HPM_SESSION_LOG_MEM_SIZE);
    if (NULL == buf)
    {
        MODULE_LOG_E(HPM, "memalloc logout buf failed");
        return -1;
    }

    len = hpm_pack_logout(buf);
    if (len <= 0)
    {
        MODULE_LOG_E(HPM, "pack failed");
        mempool_free(buf);
        return -1;
    }

    MODULE_LOG_DUMP(HPM, "hpm login:", buf, len);
    if (0 != hpm_net_send(buf, len))
    {
        MODULE_LOG_E(HPM, "send failed, len: %d", len);
    }

    mempool_free(buf);
    return 0;
}

static INT32 hpm_session_com_heartbeat(VOID)
{
    INT32 len = 0;
    UINT8 *buf = NULL;

    buf = mempool_alloc(HPM_SESSION_LOG_MEM_SIZE);
    if (NULL == buf)
    {
        MODULE_LOG_E(HPM, "memalloc heartbeat buf failed");
        return -1;
    }

    len = hpm_pack_heartbeat(buf);
    if (len <= 0)
    {
        MODULE_LOG_E(HPM, "pack failed");
        mempool_free(buf);
        return -1;
    }

    MODULE_LOG_DUMP(HPM, "hpm heartbeat:", buf, len);
    if (0 != hpm_net_send(buf, len))
    {
        hpm_socket_reset();
        MODULE_LOG_E(HPM, "send failed, len: %d", len);
    }

    mempool_free(buf);
    return 0;
}

static VOID hpm_session_receive(VOID)
{
    UINT8 *recv;
    HPM_PACK_FRAME_T *parse;
    UINT16 parse_len;
    UINT16 read_len;
    UINT16 data_len;

    recv = mempool_alloc(HPM_SESSION_RECV_MEM_SIZE);
    if (NULL == recv)
    {
        MODULE_LOG_E(HPM, "memalloc recv buf failed");
        return;
    }
    data_len = HPM_SESSION_RECV_MEM_SIZE;

    if (0 != hpm_net_recv(recv, &data_len))
    {
        mempool_free(recv);
        return;
    }

    parse = mempool_alloc(HPM_SESSION_RECV_MEM_SIZE);
    if (NULL == parse)
    {
        MODULE_LOG_E(HPM, "memalloc recv parse buf failed");
        mempool_free(recv);
        return;
    }

    read_len = 0;
    memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
    parse_len = HPM_SESSION_RECV_MEM_SIZE;
    read_len = hpm_pack_unpack(recv, data_len, parse, &parse_len);
    if (read_len <= 0)
    {
        mempool_free(recv);
        mempool_free(parse);
        return;
    }
    MODULE_LOG_DUMP(HPM, "receive:", recv, read_len);

    switch (parse->cmd)
    {
    case HPM_CMD_TSP_COMMON_ACK:
    {
        hpm_session_recv_info.data_len = parse_len;
        hpm_session_recv_info.cmd = parse->cmd;
        hpm_session_recv_info.seq_id = parse->data[0] * 256 + parse->data[1];
        hpm_session_recv_info.sub_cmd = parse->data[2];
        hpm_session_recv_info.resp_flag = parse->data[3];
        memcpy(hpm_session_recv_info.data, parse, parse_len);
        MODULE_LOG_I(HPM, "receive common ack");
        data_len -= read_len;
        if (data_len > 0)
        {
            hpm_data_recv_put(recv + read_len, data_len);
        }
        break;
    }
    case HPM_CMD_CONTROL:
    {
        hpm_session_recv_info.cmd = parse->cmd;
        hpm_session_recv_info.data_len = parse_len;
        memcpy(hpm_session_recv_info.data, parse->data, parse_len);
        MODULE_LOG_I(HPM, "receive control cmd: 0x%02x", hpm_session_recv_info.cmd);
        hpm_control_cmd_handle(hpm_session_recv_info.cmd, hpm_session_recv_info.data, hpm_session_recv_info.data_len);
        data_len -= read_len;
        if (data_len > 0)
        {
            hpm_data_recv_put(recv + read_len, data_len);
        }
        break;
    }

    default:
        MODULE_LOG_E(HPM, "unknow cmd: 0x%02X", parse->cmd);
        break;
    }

    mempool_free(recv);
    mempool_free(parse);
}

VOID hpm_session_reset(VOID)
{
    hpm_session_step = HPM_SESSION_STEP_INIT;
    for (INT32 i = 0; i < HPM_SESSION_EVENT_MAX; i++)
    {
        hpm_session_send_info[i].state = HPM_SESSION_SEND_INIT;
        hpm_session_send_info[i].retry_count = 0;
        hpm_session_send_info[i].wait_time = 0;
        hpm_session_send_info[i].result = 0;
    }
    memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
}

static INT32 hpm_session_send_login(UINT8 resp)
{
    INT32 ret = 0;
    INT32 send_intv = 10;
    INT32 server_timeout = 10;

    hpm_session_send_info_t *sender = &hpm_session_send_info[HPM_SESSION_EVENT_LOGIN];
    UINT32 current_tick = time_if_get_systick_s();

    switch (sender->state)
    {
    case HPM_SESSION_SEND_INIT:
    case HPM_SESSION_SEND_SUCCESS:
    {
        if ((0 == sender->wait_time) || (current_tick - sender->wait_time >= send_intv))
        {
            ret = hpm_session_com_login();
            if (0 == ret)
            {
                sender->state = HPM_SESSION_SEND_WAIT;
                sender->wait_time = current_tick;
            }
        }
        break;
    }

    case HPM_SESSION_SEND_WAIT:
    case HPM_SESSION_SEND_RETRY:
    {
        if ((HPM_SESSION_LOGIN_NACK & resp) == HPM_SESSION_LOGIN_NACK)
        {
            sender->retry_count = 0;
            sender->state = HPM_SESSION_SEND_SUCCESS;
        }
        else if ((HPM_SESSION_LOGIN_ACK & resp) == HPM_SESSION_LOGIN_ACK)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
        }
        else
        {
            if (current_tick - sender->wait_time >= server_timeout)
            {
                MODULE_LOG_E(HPM, "login timeout");
                sender->state = HPM_SESSION_SEND_SUCCESS;
                hpm_socket_reset();
                break;
            }

            if (current_tick - sender->wait_time >= send_intv)
            {
                ret = hpm_session_com_login();
                if (0 == ret)
                {
                    sender->state = HPM_SESSION_SEND_WAIT;
                    sender->wait_time = current_tick;
                    sender->retry_count = 0;
                }
            }
        }
        break;
    }
    default:
        break;
    }
    return ret;
}

static INT32 hpm_session_send_logout(UINT8 resp)
{
    INT32 ret = 0;
    INT32 server_timeout = 10;

    hpm_session_send_info_t *sender = &hpm_session_send_info[HPM_SESSION_EVENT_LOGOUT];
    UINT32 current_tick = time_if_get_systick_s();

    switch (sender->state)
    {
    case HPM_SESSION_SEND_INIT:
    case HPM_SESSION_SEND_SUCCESS:
    {
        ret = hpm_session_com_logout();
        if (0 == ret)
        {
            sender->state = HPM_SESSION_SEND_WAIT;
            sender->retry_count = 0;
            sender->wait_time = current_tick;
        }
    }

    case HPM_SESSION_SEND_WAIT:
    {
        if ((HPM_SESSION_LOGOUT_ACK & resp) == HPM_SESSION_LOGOUT_ACK)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_socket_reset();
            break;
        }

        if (current_tick - sender->wait_time >= server_timeout)
        {
            ret = hpm_session_com_logout();
            if (0 == ret)
            {
                sender->state = HPM_SESSION_SEND_WAIT;
                sender->retry_count = 1;
                sender->wait_time = current_tick;
            }
        }
        break;
    }

    case HPM_SESSION_SEND_RETRY:
    {
        if ((HPM_SESSION_LOGOUT_ACK & resp) == HPM_SESSION_LOGOUT_ACK)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_socket_reset();
            break;
        }

        if (current_tick - sender->wait_time >= server_timeout)
        {
            if (sender->retry_count > HPM_SESSION_RETRY_CNT)
            {
                sender->retry_count = 0;
                sender->state = HPM_SESSION_SEND_SUCCESS;
                hpm_socket_reset();
            }
            else
            {
                ret = hpm_session_com_logout();
                if (0 == ret)
                {
                    sender->retry_count++;
                    sender->wait_time = current_tick;
                }
            }
        }
        break;
    }

    default:
        break;
    }

    return ret;
}

static INT32 hpm_session_send_heartbeat(UINT8 resp)
{
    INT32 ret = 0;
    INT32 send_intv = 60;
    hpm_session_send_info_t *sender = &hpm_session_send_info[HPM_SESSION_EVENT_HEARTBEAT];
    UINT32 current_tick = time_if_get_systick_s();

    switch (sender->state)
    {
    case HPM_SESSION_SEND_INIT:
    case HPM_SESSION_SEND_SUCCESS:
    {
        if ((0 == sender->wait_time) || (current_tick - sender->wait_time >= send_intv))
        {
            ret = hpm_session_com_heartbeat();
            if (0 == ret)
            {
                sender->state = HPM_SESSION_SEND_SUCCESS;
                sender->retry_count = 0;
                sender->wait_time = current_tick;
            }
        }
        break;
    }
    }
    return ret;
}

static INT32 hpm_session_send_report(UINT8 resp)
{
    INT32 ret = 0;
    INT32 server_timeout = 10;
    UINT32 current_tick = time_if_get_systick_s();
    hpm_session_send_info_t *sender = &hpm_session_send_info[HPM_SESSION_EVENT_REPORT];

    switch (sender->state)
    {
    case HPM_SESSION_SEND_INIT:
    case HPM_SESSION_SEND_SUCCESS:
    {
        HPM_PACKET *pack = hpm_data_get_report_pack();
        if (NULL_PTR == pack)
        {
            return 0;
        }

        memset(hpm_session_send_buffer, 0, HPM_SESSION_MAX_SEND_LEN);
        INT32 len = hpm_pack_report_data(pack, hpm_session_send_buffer);
        if (len <= 0)
        {
            hpm_data_put_back_report_pack(pack);
            return -1;
        }

        sender->request = pack->type;

        if (0 != hpm_net_send(hpm_session_send_buffer, len))
        {
            hpm_data_put_back_report_pack(pack);
            return -1;
        }
        else
        {
            hpm_data_put_trans_list(pack);
            sender->wait_time = current_tick;
            sender->state = HPM_SESSION_SEND_WAIT;
        }

        MODULE_LOG_DUMP(HPM, "report data", hpm_session_send_buffer, len);

        hpm_session_send_info[HPM_SESSION_EVENT_HEARTBEAT].wait_time = current_tick;
        break;
    }
    case HPM_SESSION_SEND_WAIT:
    {
        if ((HPM_SESSION_REPORT_ACK & resp) == HPM_SESSION_REPORT_ACK)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_data_ack_pack();
            MODULE_LOG_I(HPM, "tsp ack");
            break;
        }
        else if ((HPM_SESSION_REPORT_NACK & resp) == HPM_SESSION_REPORT_NACK)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_data_flush_trans_list();
            MODULE_LOG_E(HPM, "tsp nack");
            break;
        }

        if (current_tick - sender->wait_time >= server_timeout)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_data_flush_trans_list();
            MODULE_LOG_E(HPM, "tsp timeout");
        }

        break;
    }
    default:
        break;
    }
    return ret;
}

static VOID hpm_session_proc_init(VOID)
{
    if (FALSE == hpm_mgr_acc_is_active())
    {
        return;
    }

    if (0 == hpm_session_send_info[HPM_SESSION_EVENT_LOGIN].handle(0))
    {
        hpm_session_step = HPM_SESSION_STEP_LOGIN;
    }
}

static VOID hpm_session_proc_login(VOID)
{
    if (FALSE == hpm_mgr_acc_is_active())
    {
        hpm_session_step = HPM_SESSION_STEP_INIT;
        return;
    }

    UINT8 resp = 0;
    if (HPM_CMD_TSP_COMMON_ACK == hpm_session_recv_info.cmd &&
        HPM_CMD_LOGIN == hpm_session_recv_info.sub_cmd)
    {
        if (HPM_RECV_RESP_SUCCESS == hpm_session_recv_info.resp_flag)
        {
            resp = HPM_SESSION_LOGIN_ACK;
            hpm_session_step = HPM_SESSION_STEP_SESSION;

            MODULE_LOG_I(HPM, "login success");
            hpm_data_flush_trans_list();
            hpm_data_flush_realtm_data();
        }
        else
        {
            resp = HPM_SESSION_LOGIN_NACK;
            MODULE_LOG_E(HPM, "login reject");
        }
    }
    hpm_session_send_info[HPM_SESSION_EVENT_LOGIN].handle(resp);
    memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
}

static VOID hpm_session_proc_session(VOID)
{
    UINT8 resp = 0;
    if (HPM_CMD_TSP_COMMON_ACK == hpm_session_recv_info.cmd &&
        (HPM_CMD_LIVE_DATA == hpm_session_recv_info.sub_cmd ||
         HPM_CMD_REISSUE_DATA == hpm_session_recv_info.sub_cmd))
    {
        if (HPM_RECV_RESP_SUCCESS == hpm_session_recv_info.resp_flag)
        {
            resp = HPM_SESSION_REPORT_ACK;
        }
        else
        {
            resp = HPM_SESSION_REPORT_NACK;
        }
    }

    hpm_session_send_info[HPM_SESSION_EVENT_REPORT].handle(resp);
    hpm_session_send_info[HPM_SESSION_EVENT_HEARTBEAT].handle(resp);

    if (FALSE == hpm_mgr_acc_is_active())
    {
        hpm_session_step = HPM_SESSION_STEP_LOGOUT;
    }
    memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
}

static VOID hpm_session_proc_logout(VOID)
{
    UINT8 resp = 0;
    if (HPM_CMD_TSP_COMMON_ACK == hpm_session_recv_info.cmd &&
        HPM_CMD_LOGOUT == hpm_session_recv_info.sub_cmd)
    {
        if (HPM_RECV_RESP_SUCCESS == hpm_session_recv_info.resp_flag)
        {
            resp = HPM_SESSION_LOGOUT_ACK;
            hpm_session_step = HPM_SESSION_STEP_INIT;
        }
        else
        {
            resp = HPM_SESSION_LOGOUT_NACK;
        }
    }
    hpm_session_send_info[HPM_SESSION_EVENT_LOGOUT].handle(resp);
    memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
}

VOID hpm_session_proc(VOID)
{
    hpm_session_receive();
    switch (hpm_session_step)
    {
    case HPM_SESSION_STEP_INIT:
        hpm_session_proc_init();
        break;
    case HPM_SESSION_STEP_LOGIN:
        hpm_session_proc_login();
        break;
    case HPM_SESSION_STEP_SESSION:
        hpm_session_proc_session();
        break;
    case HPM_SESSION_STEP_LOGOUT:
        hpm_session_proc_logout();
        break;
    default:
        break;
    }
    MODULE_LOG_I(HPM, "hpm session step: %d", hpm_session_step);
}
