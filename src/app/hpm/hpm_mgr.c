#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_pm_io.h"

#include "hpm_mgr.h"
#include "hpm_socket.h"


INT32 hpm_mgr_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:            
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

BOOL hpm_mgr_acc_is_active(VOID)
{
	return tbox_pm_io_acc_is_active();
}

BOOL hpm_mgr_allow_sleep(VOID)
{
	if(HPM_SOCKET_STEP_PRECONDITION == hpm_socket_get_status())
	{
		return TRUE;
	}

	return FALSE;
}


