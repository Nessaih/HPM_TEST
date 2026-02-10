#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"

static INT32 tbox_svr_init(UINT8 seq);
static VOID  tbox_svr_exit(VOID);

/*模块定义*/
TBOX_MODULE_FUN(TBOXSVR, tbox_svr_init, NULL_PTR, NULL_PTR, NULL_PTR, tbox_svr_exit, NULL_PTR);
TBOX_MODULE(TBOXSVR, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, TRUE, FALSE);
TBOX_MODULE_LOADER(TBOXSVR)
{
    /*TODO:加载其他模块*/
}

static INT32 tbox_svr_init(UINT8 seq)
{
    MODULE_LOG_D(TBOXSVR, "serivce init seq:%d", seq);
    return (INT32)TBOX_E_OK;
}

static VOID  tbox_svr_exit(VOID)
{
    MODULE_LOG_D(TBOXSVR, "serivce exit");
}