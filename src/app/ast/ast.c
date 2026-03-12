#include "tbox_common.h"
#include "tbox_core.h"
#include "stimer.h"
#include "tbox_adsp_if.h"
#include "ast.h"

#define AST_APP_TIMEOUT_MS (100)

static INT32 ast_init(UINT8 seq);
static VOID ast_stop(VOID);
static VOID ast_start(VOID);
static VOID ast_exit(VOID);

TBOX_MODULE_FUN(AST, ast_init, ast_stop, ast_start, NULL_PTR, ast_exit, NULL_PTR);
TBOX_MODULE(AST, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(AST_APP_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(AST)
{
    REGISTRY_TBOX_MESSAGE(AST_APP_TIMEOUT_EVENT);
}

static TBOX_ID ast_module_id = TBOX_ID_INVALID;
static STIMER_ID ast_timer_handle;

static VOID ast_handle_msg_event(const CHAR *name, TBOX_MSG_DATA *data)
{
    if (NULL_PTR == name)
    {
        MODULE_LOG_E(AST, "invalid param");
        return;
    }

    if (0 == strncmp(name, AST_APP_TIMER_EVENT, strlen(AST_APP_TIMER_EVENT)))
    {
        MODULE_LOG_I(AST, "timeout");
    }
}

static VOID ast_timer_callback(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(AST, module_id);
    tbox_message_send(AST_APP_TIMER_EVENT, module_id, module_id, NULL_PTR);
}

static VOID ast_create_timer(VOID)
{
    ast_timer_handle = stimer_create(STIMER_TYPE_PERIOD, ast_timer_callback);
    if (STIMER_ID_INVALID == ast_timer_handle)
    {
        MODULE_LOG_E(AST, "create timer failed");
    }
}

static VOID ast_start_timer(VOID)
{
    if (STIMER_ID_INVALID == ast_timer_handle)
    {
        MODULE_LOG_E(AST, "timer handle is invalid");
        return;
    }

    stimer_start(ast_timer_handle, AST_APP_TIMEOUT_MS);
}

static VOID ast_stop_timer(VOID)
{
    if (STIMER_ID_INVALID == ast_timer_handle)
    {
        MODULE_LOG_E(AST, "timer handle is invalid");
        return;
    }

    stimer_stop(ast_timer_handle);
}

static INT32 ast_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        GET_TBOX_MODULE_ID(AST, ast_module_id);
        ret = tbox_message_add_handler(AST_APP_TIMER_EVENT, ast_module_id, ast_handle_msg_event);
        break;
    case MODULE_INIT_SEQ_STORAGE:
        break;
    case MODULE_INIT_SEQ_MODULE:
        ast_create_timer();
        tbox_module_set_state(ast_module_id, TBOX_MODULE_STATE_START);
        break;
    default:
        break;
    }
    return ret;
}

static VOID ast_stop(VOID)
{
    ast_stop_timer();
    tbox_module_set_state(ast_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID ast_start(VOID)
{
    tbox_module_set_state(ast_module_id, TBOX_MODULE_STATE_START);
}

static VOID ast_exit(VOID)
{
    // do nothing
}
