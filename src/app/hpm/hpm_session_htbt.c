#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_memory.h"
#include "tbox_log.h"
#include "time_if.h"

#include "hpm_session_htbt.h"
#include "hpm_content.h"
#include "hpm_socket.h"
#include "hpm_cfg.h"
#include "hpm_net.h"
#include "hpm_pack.h"

static UINT32 hpm_session_htbt_tick;

INT32 hpm_session_htbt_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			hpm_session_htbt_tick = 0;
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

VOID hpm_session_htbt_wake(VOID)
{
	hpm_session_htbt_tick = 0;
}

VOID hpm_session_htbt_sleep(VOID)
{
	hpm_session_htbt_tick = 0;
}


static BOOL hpm_session_htbt_precondition(VOID)
{
	INT32  ret = 0;
	UINT32 htbt = 0;
	UINT32 cur_tick = 0;
	
	ret = hpm_cfg_get_htbt(&htbt, sizeof(htbt));
	if(0 != ret)
	{
		return FALSE;
	}

	if(0 == hpm_session_htbt_tick)
	{
		hpm_session_htbt_tick = time_if_get_systick_s();
		return FALSE;
	}

	cur_tick = time_if_get_systick_s();
	if((cur_tick - hpm_session_htbt_tick) < htbt)
	{
		return FALSE;
	}

	return TRUE;
}

VOID hpm_session_htbt(VOID)
{
	UINT8 *buf;
    INT32  len;

	if(FALSE == hpm_session_htbt_precondition())
	{
		return;
	}

    buf = mempool_alloc(HPM_SESSION_HTBT_MEM_SIZE);
	if(NULL == buf)
	{
		MODULE_LOG_E(HPM, "htbt mem alloc failed");	
		return;
	}	
    len = hpm_pack_heartbeat(buf);
    if (len <= 0)
    {
        MODULE_LOG_E(HPM, "htbt pack fail");
        mempool_free(buf);
        return;
    }
	
	MODULE_LOG_DUMP(HPM, "hpm htbt:", buf, len);

    if (0 != hpm_net_send(buf, len))
    {
		hpm_socket_reset();
        MODULE_LOG_E(HPM, "send fail, len:%d", len);
    }
    
    mempool_free(buf);	
	hpm_session_htbt_tick = time_if_get_systick_s();
}

