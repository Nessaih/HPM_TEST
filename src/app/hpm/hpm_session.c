#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_memory.h"
#include "flash_common.h"
#include "time_if.h"
#include "tbox_cfg_if.h"
#include "4g_if.h"
#include "tbox_pm_io.h"

#include "hpm_content.h"
#include "hpm_session.h"
#include "hpm_pack.h"
#include "hpm_net.h"
#include "hpm_data.h"
#include "hpm_control.h"
#include "hpm_data_recv.h"
#include "hpm_socket.h"
#include "hpm_ota_tbox.h"
#include "hpm_cfg.h"
#include "hpm_data_recv.h"

#define HPM_SESSION_RUN_INFO_MIGC (0x20260225)
#define HPM_SESSION_SEQ_NAME "HPM_SESSION_SEQ"

#define HPM_SESSION_MAX_RECV_LEN (256)
#define HPM_SESSION_MAX_SEND_LEN (1024UL)
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
    UINT16 login_seq;
    UINT16 login_date;
    UINT16 data_seq;
    UINT16 data_date;
} HPM_SESSION_RUN_INFO_T;

typedef enum
{
    HPM_SESSION_STOP = 0,
    HPM_SESSION_START = 1,
} hpm_session_run_e;

static INT32 hpm_session_send_login(UINT8 resp);
static INT32 hpm_session_send_logout(UINT8 resp);
static INT32 hpm_session_send_heartbeat(UINT8 resp);
static INT32 hpm_session_send_report(UINT8 resp);

static HPM_SESSION_RUN_INFO_T hpm_session_run_info;
static HPM_SESSION_STEP_E hpm_session_step;
static hpm_session_run_e hpm_session_run_status;

static hpm_session_send_info_t hpm_session_send_info[HPM_SESSION_EVENT_MAX] = {
    {0, 0, 0, 0, 0, hpm_session_send_login},
    {0, 0, 0, 0, 0, hpm_session_send_logout},
    {0, 0, 0, 0, 0, hpm_session_send_heartbeat},
    {0, 0, 0, 0, 0, hpm_session_send_report},
};

static VOID hpm_session_run_info_write(VOID)
{
    hpm_session_run_info.migc = HPM_SESSION_RUN_INFO_MIGC;
    if (0 != tbox_cfg_setkv(HPM_SESSION_SEQ_NAME, (uint8_t *)&hpm_session_run_info, sizeof(hpm_session_run_info)))
    {
        MODULE_LOG_E(HPM, "hpm session write run info failed");
    }
    else
    {
        MODULE_LOG_I(HPM, "hpm session write run info success");
    }
}

static VOID hpm_session_run_info_read(VOID)
{
    memset(&hpm_session_run_info, 0, sizeof(hpm_session_run_info));
    tbox_cfg_getkv(HPM_SESSION_SEQ_NAME, (uint8_t *)&hpm_session_run_info, sizeof(hpm_session_run_info));

    if (HPM_SESSION_RUN_INFO_MIGC != hpm_session_run_info.migc)
    {
        MODULE_LOG_E(HPM, "hpm session run info read failed");
        DEV_TIME time;
        time_if_get(&time);
        UINT16 date = time.month * 100 + time.day;
        hpm_session_run_info.login_date = date;
        hpm_session_run_info.login_seq = 1;
        hpm_session_run_info.data_date = date;
        hpm_session_run_info.data_seq = 1;
        hpm_session_run_info_write();
    }
}

INT32 hpm_sesion_get_login_seq(UINT8 *buf)
{
    DEV_TIME time;
    time_if_get(&time);
    UINT16 date = time.month * 100 + time.day;
    if (date != hpm_session_run_info.login_date)
    {
        hpm_session_run_info.login_date = date;
        hpm_session_run_info.login_seq = 1;
        hpm_session_run_info_write();
    }
    else
    {
        hpm_session_run_info.login_seq++;
        if (0 == hpm_session_run_info.login_seq)
        {
            hpm_session_run_info.login_seq = 1;
        }
    }

    buf[0] = hpm_session_run_info.login_seq >> 8;
    buf[1] = hpm_session_run_info.login_seq;

    return 2;
}

INT32 hpm_session_get_logout_seq(UINT8 *buf)
{
    buf[0] = hpm_session_run_info.login_seq >> 8;
    buf[1] = hpm_session_run_info.login_seq;

    return 2;
}

INT32 hpm_sesion_get_data_seq(UINT8 *buf)
{
    DEV_TIME time;
    time_if_get(&time);
    UINT16 date = time.month * 100 + time.day;
    if (date != hpm_session_run_info.data_date)
    {
        hpm_session_run_info.data_date = date;
        hpm_session_run_info.data_seq = 1;
        hpm_session_run_info_write();
    }
    else
    {
        hpm_session_run_info.data_seq++;
        if (0 == hpm_session_run_info.data_seq)
        {
            hpm_session_run_info.data_seq = 1;
        }
    }

    buf[0] = hpm_session_run_info.data_seq >> 8;
    buf[1] = hpm_session_run_info.data_seq;

    return 2;
}

static VOID hpm_session_step_set(HPM_SESSION_STEP_E status)
{
    hpm_session_step = status;
    MODULE_LOG_W(HPM, "hpm session status: %d", status);
}

UINT32 hpm_session_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        memset(&hpm_session_run_info, 0, sizeof(hpm_session_run_info));
        hpm_session_step = HPM_SESSION_STEP_INIT;
        hpm_session_run_status = HPM_SESSION_STOP;
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
    hpm_session_step_set(HPM_SESSION_STEP_INIT);
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

    MODULE_LOG_DUMP(HPM, "hpm login", buf, len);

    if (0 != hpm_net_send(buf, len))
    {
        hpm_session_force_stop();
        MODULE_LOG_E(HPM, "send failed, len: %d", len);
    }

    mempool_free(buf);
    return 0;
}

static INT32 hpm_session_com_report(VOID)
{
    HPM_PACKET *pack = hpm_data_get_report_pack();
    if (NULL_PTR == pack)
    {
        return -1;
    }

    if (0 == pack->type)
    {
        MODULE_LOG_E(HPM, "invalid cmd %X", pack->type);
        hpm_data_put_to_list(HPM_FREE_LIST, pack);
        return -1;
    }

    if ((pack->len + HPM_PARSE_POS_DATA) >= HPM_SESSION_MAX_SEND_LEN)
    {
        hpm_data_put_to_list(HPM_FREE_LIST, pack);
        MODULE_LOG_E(HPM, "report data len overflow, len: %d", pack->len);
        return -1;
    }

    UINT8* buf = mempool_alloc(HPM_SESSION_MAX_SEND_LEN);
    if (NULL_PTR == buf)
    {
        hpm_data_put_back_report_pack(pack);
        MODULE_LOG_E(HPM, "alloc buf error");
        return -1;
    }
    memset(buf, 0, HPM_SESSION_MAX_SEND_LEN);

    INT32 len = hpm_pack_report_data(pack, buf);
    if (len <= 0)
    {
        mempool_free(buf);
        hpm_data_put_back_report_pack(pack);
        return -1;
    }

    if (0 != hpm_net_send(buf, len))
    {
        hpm_data_put_back_report_pack(pack);
    }
    else
    {
        hpm_data_put_trans_list(pack);
        MODULE_LOG_DUMP(HPM, "report data", buf, len);
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

    MODULE_LOG_DUMP(HPM, "hpm logout", buf, len);
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

    MODULE_LOG_DUMP(HPM, "hpm heartbeat", buf, len);
    if (0 != hpm_net_send(buf, len))
    {
        hpm_session_force_stop();
        MODULE_LOG_E(HPM, "send failed, len: %d", len);
    }

    mempool_free(buf);
    return 0;
}

static INT32 hpm_session_send_login(UINT8 resp)
{
    INT32 ret = 0;
    INT32 send_intv = 10;

    UINT32 server_timeout = 10UL;
    hpm_cfg_get_server_timeout(&server_timeout, sizeof(server_timeout));

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
                hpm_session_send_info[HPM_SESSION_EVENT_HEARTBEAT].wait_time = current_tick;
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
                MODULE_LOG_W(HPM, "login timeout");
                sender->state = HPM_SESSION_SEND_SUCCESS;
                hpm_session_step_set(HPM_SESSION_STEP_INIT);
                break;
            }

            if (current_tick - sender->wait_time >= send_intv)
            {
                ret = hpm_session_com_login();
                if (0 == ret)
                {
                    sender->state = HPM_SESSION_SEND_WAIT;
                    sender->wait_time = current_tick;
                    hpm_session_send_info[HPM_SESSION_EVENT_HEARTBEAT].wait_time = current_tick;
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
    UINT32 server_timeout = 10UL;
    hpm_cfg_get_server_timeout(&server_timeout, sizeof(server_timeout));

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
        break;
    }

    case HPM_SESSION_SEND_WAIT:
    {
        if ((HPM_SESSION_LOGOUT_ACK & resp) == HPM_SESSION_LOGOUT_ACK)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_session_step_set(HPM_SESSION_STEP_INIT);
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
            hpm_session_step_set(HPM_SESSION_STEP_INIT);
            break;
        }

        if (current_tick - sender->wait_time >= server_timeout)
        {
            if (sender->retry_count > HPM_SESSION_RETRY_CNT)
            {
                sender->retry_count = 0;
                sender->state = HPM_SESSION_SEND_SUCCESS;
                hpm_session_step_set(HPM_SESSION_STEP_INIT);
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
    UINT32 send_intv = 60UL;
    hpm_cfg_get_htbt(&send_intv, sizeof(send_intv));
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
    UINT32 server_timeout = 10;
    hpm_cfg_get_server_timeout(&server_timeout, sizeof(server_timeout));
    UINT32 current_tick = time_if_get_systick_s();
    hpm_session_send_info_t *sender = &hpm_session_send_info[HPM_SESSION_EVENT_REPORT];

    switch (sender->state)
    {
    case HPM_SESSION_SEND_INIT:
    case HPM_SESSION_SEND_SUCCESS:
    {
        if (0 == hpm_session_com_report())
        {
            sender->wait_time = current_tick;
            sender->state = HPM_SESSION_SEND_WAIT;
            hpm_session_send_info[HPM_SESSION_EVENT_HEARTBEAT].wait_time = current_tick;
        }
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
            MODULE_LOG_W(HPM, "tsp nack");
            hpm_socket_force_stop();
            break;
        }

        if (current_tick - sender->wait_time >= server_timeout)
        {
            sender->state = HPM_SESSION_SEND_SUCCESS;
            sender->retry_count = 0;
            sender->wait_time = 0;
            hpm_data_flush_trans_list();
            MODULE_LOG_W(HPM, "tsp timeout");
            hpm_socket_force_stop();
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
    if (HPM_SESSION_STOP == hpm_session_run_status)
    {
        return;
    }

    if (0 == hpm_session_send_info[HPM_SESSION_EVENT_LOGIN].handle(0))
    {
        hpm_session_step_set(HPM_SESSION_STEP_LOGIN);
    }
}

static VOID hpm_session_proc_login(VOID)
{
    if (HPM_SESSION_STOP == hpm_session_run_status)
    {
        hpm_session_step_set(HPM_SESSION_STEP_INIT);
        return;
    }

    hpm_pack_recv_t *recv_info = hpm_data_recv_info_get();

    UINT8 resp = 0;
    if (HPM_CMD_TSP_COMMON_ACK == recv_info->cmd &&
        HPM_CMD_LOGIN == recv_info->common_resp.cmd)
    {
        if (HPM_RECV_RESP_SUCCESS == recv_info->common_resp.result)
        {
            resp = HPM_SESSION_LOGIN_ACK;
            hpm_session_step_set(HPM_SESSION_STEP_SESSION);

            MODULE_LOG_I(HPM, "login success");
            hpm_data_flush_trans_list();
            hpm_data_flush_realtm_data();
        }
        else
        {
            resp = HPM_SESSION_LOGIN_NACK;
            MODULE_LOG_W(HPM, "login reject");
        }
    }
    hpm_session_send_info[HPM_SESSION_EVENT_LOGIN].handle(resp);
}

static VOID hpm_session_proc_session(VOID)
{
    if (HPM_SESSION_STOP == hpm_session_run_status)
    {
        hpm_session_step_set(HPM_SESSION_STEP_LOGOUT);
        return;
    }

    hpm_pack_recv_t *recv_info = hpm_data_recv_info_get();

    UINT8 resp = 0;
    if (HPM_CMD_TSP_COMMON_ACK == recv_info->cmd)
    {
        if (HPM_RECV_RESP_SUCCESS == recv_info->common_resp.result)
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
}

static VOID hpm_session_proc_logout(VOID)
{
    hpm_pack_recv_t *recv_info = hpm_data_recv_info_get();

    UINT8 resp = 0;
    if (HPM_CMD_TSP_COMMON_ACK == recv_info->cmd &&
        HPM_CMD_LOGOUT == recv_info->common_resp.cmd)
    {
        if (HPM_RECV_RESP_SUCCESS == recv_info->common_resp.result)
        {
            resp = HPM_SESSION_LOGOUT_ACK;
            hpm_session_step_set(HPM_SESSION_STEP_INIT);
        }
        else
        {
            resp = HPM_SESSION_LOGOUT_NACK;
        }
    }
    hpm_session_send_info[HPM_SESSION_EVENT_LOGOUT].handle(resp);
}

VOID hpm_session_process(VOID)
{
    if (TRUE == if_4g_is_downloading())
    {
        return;
    }

    hpm_data_recv_process();
    switch (hpm_session_get_step())
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
}

HPM_SESSION_STEP_E hpm_session_get_step(VOID)
{
    return hpm_session_step;
}

VOID hpm_session_start(VOID)
{
    if (HPM_SESSION_START == hpm_session_run_status)
    {
        MODULE_LOG_W(HPM, "session already start");
        return;
    }
    hpm_session_run_status = HPM_SESSION_START;
}

VOID hpm_session_stop(VOID)
{
    if (HPM_SESSION_STOP == hpm_session_run_status)
    {
        MODULE_LOG_W(HPM, "session already stop");
        return;
    }
    hpm_session_run_status = HPM_SESSION_STOP;
}

VOID hpm_session_force_stop(VOID)
{
    hpm_session_step_set(HPM_SESSION_STEP_INIT);
    for (INT32 i = 0; i < HPM_SESSION_EVENT_MAX; i++)
    {
        hpm_session_send_info[i].state = HPM_SESSION_SEND_INIT;
        hpm_session_send_info[i].retry_count = 0;
        hpm_session_send_info[i].wait_time = 0;
        hpm_session_send_info[i].result = 0;
    }
    hpm_session_run_status = HPM_SESSION_STOP;
    hpm_data_recv_clear();
}
