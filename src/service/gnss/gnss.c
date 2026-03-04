#include "tbox_common.h"
#include "tbox_core.h"

#include "gnss.h"
#include "gnss_com.h"
#include "gnss_parse.h"
#include "gnss_shell.h"

static INT32 gnss_init(UINT8 seq);
static VOID  gnss_stop(VOID);
static VOID  gnss_start(VOID);
static VOID  gnss_exit(VOID);

//模块定义
TBOX_MODULE_FUN(GNSS, gnss_init, gnss_stop, gnss_start, NULL_PTR, gnss_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(GNSS, TBOX_TASK_PRIORITY_MID1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, gnss_com_task);
TBOX_MODULE_LOADER(GNSS)
{
    /*TODO 加载其他信息*/
}


static TBOX_ID gnss_module_id;
SemaphoreHandle_t  gnss_mutex;

static INT32 gnss_init(UINT8 seq)
{
	INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			gnss_mutex = NULL;
            GET_TBOX_MODULE_ID(GNSS, gnss_module_id);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            tbox_module_set_state(gnss_module_id, TBOX_MODULE_STATE_START);
			gnss_mutex = xSemaphoreCreateMutex();
			if(NULL == gnss_mutex)
			{
				MODULE_LOG_E(GNSS, "gnss creat mutex faied");
				vSemaphoreDelete(gnss_mutex);
                gnss_mutex = NULL;
			}
            break;
            
        default:
            break;
    }

	ret |= gnss_parse_init(seq);
	ret |= gnss_com_init(seq);
	ret |= gnss_shell_init(seq);

    MODULE_LOG_D(GNSS, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID gnss_stop(VOID)
{
	tbox_module_set_state(gnss_module_id, TBOX_MODULE_STATE_STOP);
    drv_eio_sleep();
	gnss_parse_sleep();

}

static VOID  gnss_start(VOID)
{
	tbox_module_set_state(gnss_module_id, TBOX_MODULE_STATE_START);
    drv_eio_sleep();
	gnss_parse_wake();
}

static VOID  gnss_exit(VOID)
{
	//TODO: deinit
}



