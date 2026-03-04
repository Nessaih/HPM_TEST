#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "time_if.h"
#include "4g_if.h"
#include "gnss_if.h"

#include "hpm_socket.h"
#include "hpm_session.h"
#include "hpm_net.h"
#include "hpm_cfg.h"
#include "hpm_data_recv.h"

#define HPM_SOCKET_WAIT_ALLOWED_TIMEOUT (30UL) // 30s
#define HPM_SOCKET_CLOSE_TIMEOUT (10UL)        // 10s
#define HPM_SOCKET_STOP_TIMEOUT (30UL)         // 30s
#define HPM_SOCKET_RECONNECT_TIMEOUT (30UL)    // 30s

typedef enum
{
    HPM_SOCKET_STATUS_IDLE = 0,
    HPM_SOCKET_STATUS_WAIT_DIAL,
    HPM_SOCKET_STATUS_CONNECTING,
    HPM_SOCKET_STATUS_CONNECTED,
    HPM_SOCKET_STATUS_STOPPING,
    HPM_SOCKET_STATUS_CLOSING,
    HPM_SOCKET_STATUS_CLOSED,
    HPM_SOCKET_STATUS_MAX
} hpm_socket_status_e;

typedef enum
{
    HPM_SOCKET_STOPPED = 0,
    HPM_SOCKET_RUNNING = 1,
} hpm_socket_run_e;

typedef enum
{
    HPM_SOCKET_MAIN_URL = 0,
    HPM_SOCKET_MAIN_IP,
    HPM_SOCKET_SLAVER_URL,
    HPM_SOCKET_SLAVER_IP,
    HPM_SOCKET_ADDR_MAX
} HPM_SOCKET_ADDR_TYPE_E;

typedef struct
{
    UINT32 tick;
    hpm_socket_status_e status : 4;
    hpm_socket_run_e run : 4;
    HPM_SOCKET_ADDR_TYPE_E type : 8;
} hpm_socket_info_t;

static hpm_socket_info_t hpm_socket_info;

INT32 hpm_socket_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        hpm_socket_info.status = HPM_SOCKET_STATUS_IDLE;
        hpm_socket_info.run = HPM_SOCKET_STOPPED;
        hpm_socket_info.type = HPM_SOCKET_MAIN_URL;
        break;

    case MODULE_INIT_SEQ_STORAGE:
        break;

    case MODULE_INIT_SEQ_MODULE:
        break;
    default:
        break;
    }

    return 0;
}

VOID hpm_socket_start(VOID)
{
    if (HPM_SOCKET_RUNNING == hpm_socket_info.run)
    {
        MODULE_LOG_I(HPM, "it is has started status %d", hpm_socket_info.status);
        return;
    }
    hpm_socket_info.run = HPM_SOCKET_RUNNING;
}

VOID hpm_socket_stop(VOID)
{
    if (HPM_SOCKET_STOPPED == hpm_socket_info.run)
    {
        MODULE_LOG_I(HPM, "it is has stopped status %d", hpm_socket_info.status);
        return;
    }
    hpm_socket_info.run = HPM_SOCKET_STOPPED;
}

VOID hpm_socket_force_stop(VOID)
{
    MODULE_LOG_I(HPM, "force stop start = %d, count = %d, status = %d",
                 hpm_socket_info.run, hpm_socket_info.tick, hpm_socket_info.status);

    if ((HPM_SOCKET_STATUS_CONNECTED == hpm_socket_info.status) ||
        (HPM_SOCKET_STATUS_CONNECTING == hpm_socket_info.status))
    {
        if_4g_socket_close(IF_4G_HPM_CONN_ID);
    }
    hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSED;
    hpm_socket_info.run = HPM_SOCKET_STOPPED;
    hpm_socket_info.tick = 0;
    hpm_session_force_stop();
}

VOID hpm_socket_handle_msg(TBOX_MSG_DATA *data)
{
    TBOX_CFG_CHANGE_INFO *info = (TBOX_CFG_CHANGE_INFO *)data->data;
    if (NULL_PTR == info)
    {
        MODULE_LOG_E(HPM, "invalid param");
        return;
    }

    const char *main_url[] = {"HPMMURL", "HPMMIP", "HPMMPORT"};
    for (UINT8 i = 0; i < sizeof(main_url) / sizeof(main_url[0]); i++)
    {
        if (0 == strncmp(info->name, main_url[i], strlen(main_url[i])))
        {
            if (hpm_socket_info.type == HPM_SOCKET_MAIN_URL || hpm_socket_info.type == HPM_SOCKET_MAIN_IP)
            {
                hpm_socket_force_stop();
                return;
            }
        }
    }

    const char *slave_url[] = {"HPMSURL", "HPMSIP", "HPMSPORT"};
    for (UINT8 i = 0; i < sizeof(slave_url) / sizeof(slave_url[0]); i++)
    {
        if (0 == strncmp(info->name, slave_url[i], strlen(slave_url[i])))
        {
            if (hpm_socket_info.type == HPM_SOCKET_SLAVER_URL || hpm_socket_info.type == HPM_SOCKET_SLAVER_IP)
            {
                hpm_socket_force_stop();
                return;
            }
        }
    }
}

BOOL hpm_socket_is_connected(VOID)
{
    return (HPM_SOCKET_STATUS_CONNECTED == hpm_socket_info.status) ? TRUE : FALSE;
}

BOOL hpm_socket_in_idle(VOID)
{
    BOOL idle = FALSE;
    if ((HPM_SOCKET_STATUS_IDLE == hpm_socket_info.status) ||
        (HPM_SOCKET_STATUS_CLOSED == hpm_socket_info.status))
    {
        idle = TRUE;
    }
    return idle;
}

static INT32 hpm_socket_get_addr(UINT8 *url, UINT32 *port, HPM_SOCKET_ADDR_TYPE_E type)
{
    INT32 ret = 0;
    if (NULL == url || NULL == port)
    {
        MODULE_LOG_E(HPM, "invalid param");
        return -1;
    }

    switch (type)
    {
    case HPM_SOCKET_MAIN_URL:
        ret = hpm_cfg_get_murl(url, TBOX_CFG_URL_LEN);
        if (0 != ret)
        {
            break;
        }
        ret = hpm_cfg_get_mport(port, sizeof(UINT32));
        if (0 != ret)
        {
            break;
        }
        break;
    case HPM_SOCKET_MAIN_IP:
        ret = hpm_cfg_get_mip(url, TBOX_CFG_URL_LEN);
        if (0 != ret)
        {
            break;
        }
        ret = hpm_cfg_get_mport(port, sizeof(UINT32));
        if (0 != ret)
        {
            break;
        }
        break;
    case HPM_SOCKET_SLAVER_URL:
        ret = hpm_cfg_get_surl(url, TBOX_CFG_URL_LEN);
        if (0 != ret)
        {
            break;
        }
        ret = hpm_cfg_get_sport(port, sizeof(UINT32));
        if (0 != ret)
        {
            break;
        }
        break;
    case HPM_SOCKET_SLAVER_IP:
        ret = hpm_cfg_get_sip(url, TBOX_CFG_URL_LEN);
        if (0 != ret)
        {
            break;
        }
        ret = hpm_cfg_get_sport(port, sizeof(UINT32));
        if (0 != ret)
        {
            break;
        }
        break;
    default:
        ret = -1;
        break;
    }

    if ((NULL == url) || ('\0' == url[0]) || (*port == 65535) || (0 == *port))
    {
        ret = -1;
    }

    return ret;
}

static BOOL hpm_socket_connect_server(VOID)
{
    UINT32 port = 0;
    UINT8 url[TBOX_CFG_URL_LEN] = {0};

    const char *type_str[] = {
        "HPM_SOCKET_MAIN_URL",
        "HPM_SOCKET_MAIN_IP",
        "HPM_SOCKET_SLAVER_URL",
        "HPM_SOCKET_SLAVER_IP",
    };

    if (0 != hpm_socket_get_addr(url, &port, hpm_socket_info.type))
    {
        MODULE_LOG_I(HPM, "get addr failed, type = %s", type_str[hpm_socket_info.type]);
        hpm_socket_info.type = (HPM_SOCKET_ADDR_TYPE_E)((hpm_socket_info.type + 1) % HPM_SOCKET_ADDR_MAX);
        return FALSE;
    }

    MODULE_LOG_I(HPM, "connect server, url = %s, port = %d", url, port);

    if (0 != hpm_net_connect(url, (UINT16)port))
    {
        MODULE_LOG_E(HPM, "connect server failed, url = %s, port = %d", url, port);
        hpm_socket_info.type = (HPM_SOCKET_ADDR_TYPE_E)((hpm_socket_info.type + 1) % HPM_SOCKET_ADDR_MAX);
        return FALSE;
    }
    return TRUE;
}

static VOID hpm_socket_handle_idle(VOID)
{
    if (hpm_socket_info.tick > 0)
    {
        hpm_socket_info.tick--;
    }

    if (HPM_SOCKET_STOPPED == hpm_socket_info.run)
    {
        return;
    }
    hpm_socket_info.status = HPM_SOCKET_STATUS_WAIT_DIAL;
}

static VOID hpm_socket_handle_wait_dial(VOID)
{
    if (HPM_SOCKET_STOPPED == hpm_socket_info.run)
    {
        hpm_socket_info.status = HPM_SOCKET_STATUS_IDLE;
        return;
    }

    if (HPM_SESSION_STEP_INIT != hpm_session_get_step())
    {
        hpm_session_force_stop();
        return;
    }

    if ((IF_4G_STATE_CONNECTED == if_4g_get_call_state(IF_4G_PUBLIC_APN)) &&
        (GNSS_POS_STATE_FIX == gnss_get_fix_state()))
    {
        MODULE_LOG_I(HPM, "hpm connect allow");
        if (TRUE == hpm_socket_connect_server())
        {
            hpm_socket_info.tick = HPM_SOCKET_RECONNECT_TIMEOUT;
            hpm_socket_info.status = HPM_SOCKET_STATUS_CONNECTING;
        }
    }
    else
    {
        hpm_socket_info.tick++;
        if (hpm_socket_info.tick >= HPM_SOCKET_WAIT_ALLOWED_TIMEOUT)
        {
            MODULE_LOG_I(HPM, "wait allow timeout, and direct connect");
            if (TRUE == hpm_socket_connect_server())
            {
                hpm_socket_info.tick = HPM_SOCKET_RECONNECT_TIMEOUT;
                hpm_socket_info.status = HPM_SOCKET_STATUS_CONNECTING;
            }
        }
    }
}

static VOID hpm_socket_handle_connecting(VOID)
{
    if (hpm_socket_info.tick > 0)
    {
        hpm_socket_info.tick--;
    }

    if (HPM_SOCKET_STOPPED == hpm_socket_info.run)
    {
        MODULE_LOG_I(HPM, "connecting stopped");
        if_4g_socket_close(IF_4G_HPM_CONN_ID);
        hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSING;
        return;
    }

    if (IF_4G_STATE_CONNECTED == if_4g_get_socket_conn_state(IF_4G_HPM_CONN_ID))
    {
        hpm_session_start();
        hpm_socket_info.tick = 0;
        hpm_socket_info.status = HPM_SOCKET_STATUS_CONNECTED;
        MODULE_LOG_I(HPM, "connecting success");
    }
    else
    {
        if (0 == hpm_socket_info.tick)
        {
            MODULE_LOG_I(HPM, "connecting timeout");
            if_4g_socket_close(IF_4G_HPM_CONN_ID);
            hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSING;
            hpm_socket_info.type = (HPM_SOCKET_ADDR_TYPE_E)((hpm_socket_info.type + 1) % HPM_SOCKET_ADDR_MAX);
        }
    }
}

static VOID hpm_socket_handle_connected(VOID)
{
    if (hpm_socket_info.tick > 0)
    {
        hpm_socket_info.tick--;
    }

    if (HPM_SOCKET_STOPPED == hpm_socket_info.run)
    {
        MODULE_LOG_I(HPM, "stop transfer data");
        hpm_session_stop();
        hpm_socket_info.status = HPM_SOCKET_STATUS_STOPPING;
        hpm_socket_info.tick = HPM_SOCKET_STOP_TIMEOUT;
        return;
    }

    if (IF_4G_STATE_CONNECTED != if_4g_get_socket_conn_state(IF_4G_HPM_CONN_ID) ||
        (IF_4G_STATE_CONNECTED != if_4g_get_call_state(IF_4G_PUBLIC_APN)))
    {
        MODULE_LOG_I(HPM, "connecting lost");
        hpm_data_recv_clear();
        hpm_session_force_stop();
        if_4g_socket_close(IF_4G_HPM_CONN_ID);
        hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSING;
        return;
    }

    if (HPM_SESSION_STEP_INIT == hpm_session_get_step())
    {
        hpm_session_start();
    }
}

static VOID hpm_socket_handle_stopping(VOID)
{
    if (hpm_socket_info.tick > 0)
    {
        hpm_socket_info.tick--;
    }

    if (HPM_SESSION_STEP_INIT == hpm_session_get_step())
    {
        MODULE_LOG_I(HPM, "the data transfer is stopped, and direct close socket");
        hpm_data_recv_clear();
        if_4g_socket_close(IF_4G_HPM_CONN_ID);
        if (hpm_socket_info.tick < HPM_SOCKET_CLOSE_TIMEOUT)
        {
            hpm_socket_info.tick = HPM_SOCKET_CLOSE_TIMEOUT;
        }
        hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSING;
    }
    else
    {
        if (0 == hpm_socket_info.tick)
        {
            MODULE_LOG_I(HPM, "the data transfer stop timeout, force stop transfer data and close socket");
            hpm_data_recv_clear();
            hpm_session_force_stop();
            if_4g_socket_close(IF_4G_HPM_CONN_ID);
            hpm_socket_info.tick = HPM_SOCKET_CLOSE_TIMEOUT;
            hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSING;
        }
    }
}

static VOID hpm_socket_handle_closing(VOID)
{
    if (hpm_socket_info.tick > 0)
    {
        hpm_socket_info.tick--;
    }

    if (IF_4G_STATE_DISCONNECTED == if_4g_get_socket_conn_state(IF_4G_HPM_CONN_ID))
    {
        MODULE_LOG_I(HPM, "hpm connect closed");
        hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSED;
    }
    else
    {
        if (0 == hpm_socket_info.tick)
        {
            MODULE_LOG_I(HPM, "hpm connect close timeout");
            hpm_socket_info.status = HPM_SOCKET_STATUS_CLOSED;
        }
    }
}

static VOID hpm_socket_handle_closed(VOID)
{
    if (hpm_socket_info.tick > 0)
    {
        hpm_socket_info.tick--;
        return;
    }

    if (HPM_SOCKET_STOPPED == hpm_socket_info.run)
    {
        return;
    }

    if (HPM_SESSION_STEP_INIT != hpm_session_get_step())
    {
        hpm_session_force_stop();
    }

    hpm_socket_info.status = HPM_SOCKET_STATUS_WAIT_DIAL;
    hpm_socket_info.tick = HPM_SOCKET_WAIT_ALLOWED_TIMEOUT;
}

VOID hpm_socket_process(VOID)
{
    MODULE_LOG_I(HPM, "hpm socket process, status = %d start = %d", hpm_socket_info.status, hpm_socket_info.run);
    switch (hpm_socket_info.status)
    {
    case HPM_SOCKET_STATUS_IDLE:
        hpm_socket_handle_idle();
        break;
    case HPM_SOCKET_STATUS_WAIT_DIAL:
        hpm_socket_handle_wait_dial();
        break;
    case HPM_SOCKET_STATUS_CONNECTING:
        hpm_socket_handle_connecting();
        break;
    case HPM_SOCKET_STATUS_CONNECTED:
        hpm_socket_handle_connected();
        break;
    case HPM_SOCKET_STATUS_STOPPING:
        hpm_socket_handle_stopping();
        break;
    case HPM_SOCKET_STATUS_CLOSING:
        hpm_socket_handle_closing();
        break;
    case HPM_SOCKET_STATUS_CLOSED:
        hpm_socket_handle_closed();
        break;
    default:
        break;
    }
}

VOID hpm_socket_wakeup(VOID)
{
    hpm_socket_info.tick = 0;
    hpm_socket_info.status = HPM_SOCKET_STATUS_IDLE;
}
