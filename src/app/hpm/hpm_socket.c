#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "time_if.h"
#include "4g_if.h"

#include "hpm_socket.h"
#include "hpm_session.h"
#include "hpm_net.h"
#include "hpm_cfg.h"
#include "hpm_mgr.h"

#define HPM_SOCKET_DIAL_TIMEOUT		(300)
#define HPM_SOCKET_CNNT_TIMEOUT		(30)
#define HPM_SOCKET_CLOSE_TIMEOUT	(30)

#define HPM_SOCKET_CNNT_MAX_ERR		(3)

typedef enum
{
    HPM_SOCKET_MAIN_URL 			= 0x00,
    HPM_SOCKET_MAIN_IP				= 0x01,
    HPM_SOCKET_SLAVER_URL			= 0x02,
    HPM_SOCKET_SLAVER_IP			= 0x03,
    HPM_SOCKET_ADDR_MAX				= HPM_SOCKET_SLAVER_IP+1
}HPM_SOCKET_ADDR_TYPE_E;

typedef struct
{
	HPM_SOCKET_ADDR_TYPE_E type;
	CHAR 	url[TBOX_CFG_URL_LEN];
	UINT16 	port;
	UINT32  tick;
	UINT32  error;
}HPM_SOCKET_INFO_T;

typedef VOID (*hpm_socket_process_func)(VOID);

static HPM_SOCKET_STEP_E hpm_socket_step;
static HPM_SOCKET_INFO_T hpm_socket_info;

INT32 hpm_socket_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            hpm_socket_step = HPM_SOCKET_STEP_INIT;
			memset(&hpm_socket_info, 0, sizeof(hpm_socket_info));
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

static VOID hpm_socket_pro_init(VOID)
{
	memset(hpm_socket_info.url, 0, sizeof(hpm_socket_info.url));
	hpm_socket_info.port = 0;
	hpm_socket_info.tick = 0;
	hpm_socket_step = HPM_SOCKET_STEP_PRECONDITION;
}

static VOID hpm_socket_pro_precondition(VOID)
{	
	INT32  ret = 0;
    CHAR   url[TBOX_CFG_URL_LEN] = {0};
    UINT32 port = 0;
	
	if(FALSE == hpm_mgr_acc_is_active())
	{
		return;
	}
	
	switch (hpm_socket_info.type)
	{
		case HPM_SOCKET_MAIN_URL:
			ret = hpm_cfg_get_murl((UINT8 *)url, TBOX_CFG_URL_LEN);
			if(0 != ret)
			{
				break;
			}
			ret = hpm_cfg_get_mport(&port, sizeof(port));
			if(0 != ret)
			{
				break;
			}
			break;
		case HPM_SOCKET_MAIN_IP:
			ret = hpm_cfg_get_mip((UINT8 *)url, TBOX_CFG_URL_LEN);
			if(0 != ret)
			{
				break;
			}
			ret = hpm_cfg_get_mport(&port, sizeof(port));
			if(0 != ret)
			{
				break;
			}
			break;
		case HPM_SOCKET_SLAVER_URL:
			ret = hpm_cfg_get_surl((UINT8 *)url, TBOX_CFG_URL_LEN);
			if(0 != ret)
			{
				break;
			}
			ret = hpm_cfg_get_sport(&port, sizeof(port));
			if(0 != ret)
			{
				break;
			}
			break;
		case HPM_SOCKET_SLAVER_IP:
			ret = hpm_cfg_get_sip((UINT8 *)url, TBOX_CFG_URL_LEN);
			if(0 != ret)
			{
				break;
			}
			ret = hpm_cfg_get_sport(&port, sizeof(port));
			if(0 != ret)
			{
				break;
			}
			break;
		case HPM_SOCKET_ADDR_MAX:
			ret = -1;
			break;
		default:
			break;
	}

	if(0 == ret)
	{
		memset(hpm_socket_info.url, 0, sizeof(hpm_socket_info.url));
		strncpy(hpm_socket_info.url, url, strlen(url));
		hpm_socket_info.port = port;
		hpm_socket_info.tick = time_if_get_systick_s();
		hpm_socket_step = HPM_SOCKET_STEP_WAIT_DIAL;
	}
	else
	{
		hpm_socket_info.type++;
		if(hpm_socket_info.type >= HPM_SOCKET_ADDR_MAX)
		{
			hpm_socket_info.type = HPM_SOCKET_MAIN_URL;
		}
	}
}

static VOID hpm_socket_pro_wait_dail(VOID)
{
	UINT32 cur_tick = 0;
	if(IF_4G_STATE_CONNECTED == if_4g_get_call_state(IF_4G_PUBLIC_APN))
	{		
		hpm_socket_step = HPM_SOCKET_STEP_OPEN;
	}
	else
	{
		cur_tick = time_if_get_systick_s();
		if((cur_tick-hpm_socket_info.tick) > HPM_SOCKET_DIAL_TIMEOUT)
		{
			hpm_socket_step = HPM_SOCKET_STEP_INIT;
		}
	}
}

static VOID hpm_socket_pro_open(VOID)
{
	if(0 != hpm_net_connect((UINT8 *)hpm_socket_info.url, hpm_socket_info.port))
	{
		MODULE_LOG_E(HPM, "socket open failed, url: %s, port: %d, type: %d", 
			hpm_socket_info.url, hpm_socket_info.port, hpm_socket_info.type);
		hpm_socket_step = HPM_SOCKET_STEP_INIT;
		hpm_socket_info.type++;
		if(hpm_socket_info.type >= HPM_SOCKET_ADDR_MAX)
		{
			hpm_socket_info.type = HPM_SOCKET_MAIN_URL;
		}
	}
	else
	{
		MODULE_LOG_I(HPM, "socket open success");
		hpm_socket_step = HPM_SOCKET_STEP_CONNECTING;
		hpm_socket_info.tick = time_if_get_systick_s();
	}
}

static VOID hpm_socket_pro_connecting(VOID)
{
	UINT32 cur_tick = 0;
	if(hpm_net_is_ready())
	{
		hpm_socket_step = HPM_SOCKET_STEP_CONNECTED;
		hpm_socket_info.tick = 0;
		hpm_socket_info.error = 0;
	}
	else
	{
		cur_tick = time_if_get_systick_s();
		if((cur_tick-hpm_socket_info.tick) > HPM_SOCKET_CNNT_TIMEOUT)
		{
			hpm_socket_step = HPM_SOCKET_STEP_CLOSE;
			hpm_socket_info.tick = time_if_get_systick_s();
			hpm_socket_info.error++;
			if(hpm_socket_info.error > HPM_SOCKET_CNNT_MAX_ERR)
			{
				hpm_socket_info.error = 0;
				hpm_socket_info.type++;
				if(hpm_socket_info.type >= HPM_SOCKET_ADDR_MAX)
				{
					hpm_socket_info.type = HPM_SOCKET_MAIN_URL;
				}
			}
			MODULE_LOG_E(HPM, "socket connecting timeout, error: %d", hpm_socket_info.error);
		}
	}
}

static VOID hpm_socket_pro_connected(VOID)
{
	if(FALSE == hpm_net_is_ready())
	{
		MODULE_LOG_E(HPM, "socket connect lost, stop transfer data");		
		hpm_socket_step = HPM_SOCKET_STEP_CLOSE;
		//TODO: 重置会话, 重置接收数据
		hpm_session_reset();
	}
	else
	{
		hpm_session_proc();
	}
}

static VOID hpm_socket_pro_close(VOID)
{
	UINT32 cur_tick = 0;
	if(0 == hpm_net_disconnect())
	{
		MODULE_LOG_I(HPM, "socket close success");
		hpm_socket_step = HPM_SOCKET_STEP_INIT;		
		hpm_session_reset();
	}
	else
	{
		if((cur_tick-hpm_socket_info.tick) > HPM_SOCKET_CLOSE_TIMEOUT)
		{
			hpm_socket_step = HPM_SOCKET_STEP_INIT;
			hpm_socket_info.tick = 0;
			MODULE_LOG_E(HPM, "socket close timeout");
			hpm_session_reset();
		}
	}
}

static const hpm_socket_process_func hpm_socket_process[] = 
{
	hpm_socket_pro_init,
	hpm_socket_pro_precondition,
	hpm_socket_pro_wait_dail,
	hpm_socket_pro_open,
	hpm_socket_pro_connecting,
	hpm_socket_pro_connected,
	hpm_socket_pro_close,
};

VOID hpm_socket_timeout_proc(VOID)
{
	hpm_socket_process[hpm_socket_step]();
}

VOID hpm_socket_reset(VOID)
{
	hpm_socket_step = HPM_SOCKET_STEP_CLOSE;
}

HPM_SOCKET_STEP_E hpm_socket_get_status(VOID)
{
	return hpm_socket_step;
}


