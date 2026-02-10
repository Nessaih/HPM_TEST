#ifndef TBOX_TASK_H
#define TBOX_TASK_H

#include "tbox_module.h"
#include "tbox_message_inner.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_tbox_task_state
{
    TBOX_TASK_STATE_INVALID = -1,
    TBOX_TASK_NO_INIT = 0U,
    TBOX_TASK_STOP,
    TBOX_TASK_RUN_IDLE,
    TBOX_TASK_RUN_BUSY,
    TBOX_TASK_ABNORMAL,
    TBOX_TASK_RECOVERING,
    TBOX_TASK_ISFORBIDDEN, 
}TBOX_TASK_STATE;

INT32 tbox_task_init(VOID);
VOID tbox_task_deinit(VOID);
INT32 tbox_task_start(VOID);
INT32 tbox_task_stop(VOID);
TBOX_ID tbox_task_create_runloop(TBOX_ID module_id, 
                               UINT8 pripority, 
                               UINT32 stack_size, 
                               MODULE_RUNLOOP_FUN runloop);
MODULE_HANDLE tbox_task_get_handle(TBOX_ID task_id);
TBOX_ID tbox_task_find_by_handle(VOID *handle);
TBOX_ID tbox_task_attach(TBOX_ID module_id, UINT8 pripority, UINT32 stack_size);
INT32 tbox_task_detach(TBOX_ID task_id);
INT32 tbox_task_get_priority(TBOX_ID task_id);
INT32 tbox_task_add_msg(TBOX_ID task_id, 
                        CHAR *msg_name, 
                        TBOX_MSG_HADNLER msg_handle, 
                        UINT16 msg_len, 
                        UINT8 *msg_data);
INT32 tbox_taskpool_add_msg(UINT32 stack_size, 
                            CHAR *msg_name, 
                            TBOX_MSG_HADNLER msg_handle, 
                            UINT16 msg_len, 
                            UINT8 *msg_data);
TBOX_TASK_STATE tbox_task_get_state(TBOX_ID task_id);
VOID  tbox_reset_task(TBOX_ID task_id);
VOID  tbox_forbid_task(TBOX_ID task_id);
INT32 tbox_task_expend_stack(TBOX_ID task_id);
VOID  tbox_task_check(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_TASK_H */