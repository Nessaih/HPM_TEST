#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_memory.h"
#include "mem_hpm.h"
#include "time_if.h"

#include "hpm_content.h"
#include "hpm_session.h"
#include "hpm_session_htbt.h"
#include "hpm_pack.h"
#include "hpm_net.h"
#include "hpm_mgr.h"
#include "hpm_data.h"
#include "hpm_data_recv.h"
#include "hpm_socket.h"

#define HPM_SESSION_RUN_INFO_MIGC (0x11223344)

#define HPM_SESSION_MAX_RECV_LEN  (256)
#define HPM_SESSION_LOG_TIMEOUT   (10)

typedef struct
{
	UINT32 migc;
	UINT32 login_seq_info;
	UINT32 data_seq_info;
}HPM_SESSION_RUN_INFO_T;

typedef enum
{
    HPM_SESSION_STEP_INIT		= 0x00,
    HPM_SESSION_STEP_LOGIN		= 0x01,
    HPM_SESSION_STEP_LOGIN_WAIT	= 0x02, 
    HPM_SESSION_STEP_SESSION	= 0x03,
    HPM_SESSION_STEP_LOGOUT		= 0x04,
    HPM_SESSION_STEP_LOGOUT_WAIT= 0x05,
    HPM_SESSION_STEP_MAX		= HPM_SESSION_STEP_LOGOUT_WAIT+1
} HPM_SESSION_STEP_E;

typedef enum
{
	HPM_SESSION_STA_IDLE		= 0x00,
	HPM_SESSION_STA_SENDING 	= 0x01,
	HPM_SESSION_STA_SECCESS  	= 0x02,
}HPM_SESSION_SEND_RSE_STA_E;

typedef struct
{
	UINT32 tick;
	HPM_SESSION_SEND_RSE_STA_E sta; 
}HPM_SESSION_INFO_T;

typedef struct
{	
	UINT16 cmd;
	UINT16 seq_id;
	UINT16 sub_cmd;
	UINT16 resp_flag;
	UINT16 data_len;
	UINT8  data[HPM_SESSION_MAX_RECV_LEN];
}HPM_SESSION_RECV_INFO;


typedef VOID (*hpm_session_process_func)(VOID);


static HPM_SESSION_RUN_INFO_T hpm_session_run_info;
static HPM_SESSION_STEP_E 	  hpm_session_step;
static HPM_SESSION_INFO_T	  hpm_session_info;
static HPM_SESSION_RECV_INFO  hpm_session_recv_info;

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
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm session write run info failed, ret: %d", ret);
	}
}

static VOID hpm_session_run_info_read(VOID)
{
	INT32 ret = 0;
    DEV_TIME time;
	
	ret = drv_flash_nor_read(FLASH_NOR_ADDR_HPM_SES_INFO, (uint8_t *)&hpm_session_run_info, sizeof(hpm_session_run_info));
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm session run info read failed, ret: %d", ret);
		time_if_get(&time);
		hpm_login_date = time.month*100 + time.day;
		hpm_login_seq  = 1;
		hpm_session_run_info.login_seq_info = (hpm_login_date << 16) | hpm_login_seq;

		hpm_data_date = time.month*100 + time.day;
		hpm_data_seq  = 1;
		hpm_session_run_info.data_seq_info = (hpm_data_date << 16) | hpm_data_seq;
		hpm_session_run_info_write();
		return;
	}

	if(HPM_SESSION_RUN_INFO_MIGC != hpm_session_run_info.migc)
	{
		MODULE_LOG_E(HPM, "hpm session run info migc err, migc: 0x%x", hpm_session_run_info.migc);
		
		time_if_get(&time);
		hpm_login_date = time.month*100 + time.day;
		hpm_login_seq  = 1;
		hpm_session_run_info.login_seq_info = (hpm_login_date << 16) | hpm_login_seq;

		hpm_data_date = time.month*100 + time.day;
		hpm_data_seq  = 1;
		hpm_session_run_info.data_seq_info = (hpm_data_date << 16) | hpm_data_seq;
		hpm_session_run_info_write();
		return;
	}

	hpm_login_date = hpm_session_run_info.login_seq_info >> 16;
	hpm_login_seq  = (UINT16)hpm_session_run_info.login_seq_info&0x00FF;
	hpm_data_date  = hpm_session_run_info.login_seq_info >> 16;
	hpm_data_seq   = (UINT16)hpm_session_run_info.login_seq_info&0x00FF;
	return;
}

INT32 hpm_sesion_get_login_seq(UINT8 *buf)
{
    DEV_TIME time;
	UINT16 max_seq = 0xFFFF;

	time_if_get(&time);
	
	if (++hpm_login_seq > max_seq)
	{
	    hpm_login_seq = 1;
	}
    else if (hpm_login_date != time.month * 100 + time.day)
    {
        hpm_login_date  = time.month * 100 + time.day;
        hpm_login_seq   = 1;
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
UINT32 hpm_session_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            memset(&hpm_session_run_info, 0, sizeof(hpm_session_run_info));
			memset(&hpm_session_info, 0, sizeof(hpm_session_info));
			memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
			hpm_session_step = HPM_SESSION_STEP_INIT;
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:           
			hpm_session_run_info_read();
            break;
            
        default:
            break;
    }

	return 0;
}

VOID hpm_session_wake(VOID)
{
	memset(&hpm_session_info, 0, sizeof(hpm_session_info));
	memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
	hpm_session_step = HPM_SESSION_STEP_INIT;
}

VOID hpm_session_sleep(VOID)
{
	hpm_session_run_info_write();
}

static VOID hpm_session_proc_init(VOID)
{
	if(FALSE == hpm_mgr_acc_is_active())
	{
		hpm_socket_reset();
		return;
	}

	hpm_session_step = HPM_SESSION_STEP_LOGIN;	
	memset(&hpm_session_info, 0, sizeof(hpm_session_info));
}

static VOID hpm_session_proc_login(VOID)
{
	INT32  len = 0;
	UINT8 *buf = NULL;
	
	buf = mempool_alloc(HPM_SESSION_LOG_MEM_SIZE);
	if(NULL == buf)
	{
		MODULE_LOG_E(HPM, "memalloc login buf failed");
		return;
	}

	len = hpm_pack_login(buf);
	if(len <= 0)
	{		
		MODULE_LOG_E(HPM, "pack failed");
		mempool_free(buf);
		return;
	}

	MODULE_LOG_DUMP(HPM, "hpm login:", buf, len);

	if(0 != hpm_net_send(buf, len))
	{
		hpm_socket_reset();
		MODULE_LOG_E(HPM, "send failed, len: %d", len);
		return;
	}
	else
	{
		hpm_session_step = HPM_SESSION_STEP_LOGIN_WAIT;
		hpm_session_info.tick = time_if_get_systick_s();
	}
	
	mempool_free(buf);
}

static VOID hpm_session_proc_login_wait(VOID)
{
	UINT32 cur_tick = 0;
	if(HPM_CMD_TSP_COMMON_ACK == hpm_session_recv_info.cmd && HPM_CMD_LOGIN == hpm_session_recv_info.sub_cmd)
	{
		if(HPM_RECV_RESP_SUCCESS == hpm_session_recv_info.resp_flag)
		{
			MODULE_LOG_I(HPM, "login accept");
			hpm_session_step = HPM_SESSION_STEP_SESSION;
			hpm_session_info.tick = 0;
		}
		else
		{
			MODULE_LOG_E(HPM, "login reject");
		}
		memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
	}
	else
	{
		cur_tick = time_if_get_systick_s();
		if(cur_tick-hpm_session_info.tick >= HPM_SESSION_LOG_TIMEOUT)
		{
			hpm_socket_reset();
			MODULE_LOG_E(HPM, "login timeout, reset socket");
		}
	}
}

static VOID hpm_session_proc_session(VOID)
{
	//登出条件: acc关闭
	if(FALSE == hpm_mgr_acc_is_active())
	{
		hpm_session_step = HPM_SESSION_STEP_LOGOUT;
		return;
	}

	//TODO: 心跳, 实时数据,补发数据, 升级tbox, 参数查询设置
	hpm_session_htbt();

	
}

static VOID hpm_session_proc_loginout(VOID)
{
	INT32  len = 0;
	UINT8 *buf = NULL;
	
	buf = mempool_alloc(HPM_SESSION_LOG_MEM_SIZE);
	if(NULL == buf)
	{
		MODULE_LOG_E(HPM, "memalloc logout buf failed");
		return;
	}

	len = hpm_pack_logout(buf);
	if(len <= 0)
	{		
		MODULE_LOG_E(HPM, "pack failed");
		mempool_free(buf);
		return;
	}

	MODULE_LOG_DUMP(HPM, "hpm login:", buf, len);
	if(0 != hpm_net_send(buf, len))
	{
		hpm_socket_reset();
		MODULE_LOG_E(HPM, "send failed, len: %d", len);
		mempool_free(buf);
		return;
	}
	else
	{
		hpm_session_step = HPM_SESSION_STEP_LOGOUT_WAIT;
		hpm_session_info.tick = time_if_get_systick_s();
	}
	
	mempool_free(buf);
}

static VOID hpm_session_proc_loginout_wait(VOID)
{
	UINT32 cur_tick = 0;
	if(HPM_CMD_TSP_COMMON_ACK == hpm_session_recv_info.cmd && HPM_CMD_LOGOUT == hpm_session_recv_info.sub_cmd)
	{
		if(HPM_RECV_RESP_SUCCESS == hpm_session_recv_info.resp_flag)
		{
			MODULE_LOG_I(HPM, "logout accept");
			hpm_session_step = HPM_SESSION_STEP_INIT;
			hpm_session_info.tick = 0;
			hpm_socket_reset();
		}
		else
		{
			MODULE_LOG_E(HPM, "logout reject");
		}
		memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
	}
	else
	{
		cur_tick = time_if_get_systick_s();
		if(cur_tick-hpm_session_info.tick >= HPM_SESSION_LOG_TIMEOUT)
		{
			hpm_socket_reset();
			MODULE_LOG_E(HPM, "logout timeout, reset socket");
		}
	}
}


static const hpm_session_process_func hpm_session_process[] = 
{
	hpm_session_proc_init,
	hpm_session_proc_login,
	hpm_session_proc_login_wait,
	hpm_session_proc_session,
	hpm_session_proc_loginout,
	hpm_session_proc_loginout_wait,
};

static VOID hpm_session_receive(VOID)
{
	UINT8     *recv;
    HPM_PACK_FRAME_T *parse;
    UINT16     parse_len;
    UINT16     read_len;
    UINT16     data_len;

    recv     = mempool_alloc(HPM_SESSION_RECV_MEM_SIZE);
	if(NULL == recv)
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
	if(NULL == parse)
	{
		MODULE_LOG_E(HPM, "memalloc recv parse buf failed");	
	    mempool_free(recv);
		return;
	}	

    read_len = 0;
	memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
    parse_len = HPM_SESSION_RECV_MEM_SIZE;
    read_len  = hpm_pack_unpack(recv, data_len, parse, &parse_len);
	if(read_len <= 0)
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
			hpm_session_recv_info.seq_id = parse->data[0]*256 + parse->data[1];				
			hpm_session_recv_info.sub_cmd = parse->data[2];
			hpm_session_recv_info.resp_flag = parse->data[3];
			memcpy(hpm_session_recv_info.data,parse,parse_len);
			MODULE_LOG_I(HPM, "receive common ack");
			data_len -= read_len;
			if(data_len > 0)
			{					
				hpm_data_recv_put(recv+read_len, data_len);
			}
			break;

		}
		case HPM_CMD_CONTROL:
	    {
			hpm_session_recv_info.cmd = parse->cmd;
			hpm_session_recv_info.data_len = parse_len;	
			memcpy(hpm_session_recv_info.data,parse->data,parse_len);
			data_len -= read_len;
			if(data_len > 0)
			{					
				hpm_data_recv_put(recv+read_len, data_len);
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

VOID hpm_session_proc(VOID)
{
	if(HPM_SESSION_STEP_LOGIN_WAIT == hpm_session_step || 
		HPM_SESSION_STEP_LOGOUT_WAIT == hpm_session_step||
		HPM_SESSION_STEP_SESSION == hpm_session_step)
	{
		hpm_session_receive();
	}

	MODULE_LOG_I(HPM, "hpm session step: %d", hpm_session_step);
	hpm_session_process[hpm_session_step]();
}

VOID hpm_session_reset(VOID)
{
	memset(&hpm_session_info, 0, sizeof(hpm_session_info));
	memset(&hpm_session_recv_info, 0, sizeof(hpm_session_recv_info));
	hpm_session_step = HPM_SESSION_STEP_INIT;
}

