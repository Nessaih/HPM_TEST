#ifndef TBOX_SUPERVISE_H
#define TBOX_SUPERVISE_H

#ifdef __cplusplus
extern "C" {
#endif

#define TBOX_CORE_MEMORY_ABNORMAL_NOTIFY      "core_memoryabnormal_notify"
#define TBOX_CORE_TASK_ABNORMAL_NOTIFY        "core_taskabnormal_notify"
#define TBOX_CORE_TASK_UPDATE_NOTIFY          "core_taskupdate_notify"

typedef enum tag_tbox_module_abnormal_type
{
    MODULE_ABNORMAL_MEMORY_OVERFLOW = 0U,
    MODULE_ABNORMAL_MEMORY_UNDERFLOW,
    MODULE_ABNORMAL_TASK_BLOCKED,
    MODULE_ABNORMAL_TASK_DELETED,
    MODULE_ABNORMAL_TASKSTACK_SMALL, 
    MODULE_ABNORMAL_STACK_OVERFLOW,
    MODULE_ABNORMAL_TYPE_MAX
}TBOX_MODULE_ABNORMAL_TYPE;

typedef enum tag_tbox_module_abnormal_process_type
{
    MODULE_ABNORMAL_PROCESS_WARNING = 0U,
    MODULE_ABNORMAL_PROCESS_RESET,
    MODULE_ABNORMAL_PROCESS_DEGRADE,
    MODULE_ABNORMAL_PROCESS_FORBID
}TBOX_MODULE_ABNORMAL_PROCESS_TYPE;

typedef struct tag_tbox_core_memory_abnormal_info
{
    VOID *addr;
}TBOX_CORE_MEMORY_ABNORMAL_INFO;

typedef struct tag_tbox_core_task_abnormal_info
{
    TBOX_ID task_id;
    VOID *task_handle;
    TBOX_ID module_id;
    CHAR *msg_name;
}TBOX_CORE_TASK_ABNORMAL_INFO;

typedef struct tag_tbox_core_task_update_info
{
    VOID *task_handle;
    TBOX_ID module_id;
}TBOX_CORE_TASK_UPDATE_INFO;

VOID tbox_supervice_set_abnormal_process(TBOX_MODULE_ABNORMAL_TYPE abnormal_type, 
                                         TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_SUPERVISE_H */