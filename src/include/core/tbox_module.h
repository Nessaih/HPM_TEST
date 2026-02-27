#ifndef TBOX_MODULE_H
#define TBOX_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef INT32 (*MODULE_INIT_FUN)(UINT8 seq);
typedef INT32 (*MODULE_ENABLE_FUN)(BOOL enable);
typedef VOID  (*MODULE_EXIT_FUN)(VOID);
typedef VOID  (*MODULE_STOP_FUN)(VOID);
typedef VOID  (*MODULE_START_FUN)(VOID);
typedef VOID  (*MODULE_RUNLOOP_FUN)(VOID *param);
typedef BOOL  (*MODULE_CANBE_STOP_FUN)(VOID);
typedef VOID*  MODULE_HANDLE;

#define MODULE_INIT_SEQ_OS          (0U)
#define MODULE_INIT_SEQ_STORAGE     (1U)
#define MODULE_INIT_SEQ_MODULE      (2U)

typedef struct tag_tbox_module_info
{
    CHAR   *name;
    UINT8  priority;
    UINT8  log_level;
    UINT16 stack_size;
    BOOL   is_interface;
    BOOL   is_critical;
    BOOL   is_runloop;
    MODULE_INIT_FUN init_fun;
    MODULE_STOP_FUN stop_fun;
    MODULE_START_FUN start_fun;
    MODULE_ENABLE_FUN enable_fun;
    MODULE_EXIT_FUN exit_fun;
    MODULE_CANBE_STOP_FUN canbe_stop_fun;
}TBOX_MODULE_INFO;

typedef enum tag_tbox_module_state
{
    TBOX_MODULE_STATE_INVALID = -1,
    TBOX_MODULE_STATE_NOINIT = 0,
    TBOX_MODULE_STATE_INIT,
    TBOX_MODULE_STATE_START,
    TBOX_MODULE_STATE_STOP,
}TBOX_MODULE_STATE;

VOID tbox_module_start(VOID);
VOID tbox_module_start_specific(TBOX_ID module_id);
VOID tbox_module_stop(VOID);
VOID tbox_module_stop_specific(TBOX_ID module_id);
TBOX_ID tbox_module_register(TBOX_MODULE_INFO *module_info);
VOID tbox_module_show_allname(VOID);
INT32 tbox_module_start_runloop(TBOX_ID module_id, MODULE_RUNLOOP_FUN runloop_fun);
VOID tbox_module_get_config(TBOX_ID module_id, TBOX_MODULE_INFO *config);
INT32 tbox_module_enable(TBOX_ID module_id, BOOL enable);
BOOL  tbox_module_is_enabled(TBOX_ID module_id);
CHAR *tbox_module_get_name(TBOX_ID module_id);
UINT8 tbox_module_get_log_level(TBOX_ID module_id);
VOID  tbox_module_set_log_level(TBOX_ID module_id, UINT8 log_level);
INT32  tbox_module_set_log_level_byname(CHAR *name, UINT8 log_level);
BOOL tbox_module_is_interface(TBOX_ID module_id);
BOOL tbox_module_is_critical(TBOX_ID module_id);
MODULE_HANDLE tbox_module_get_handle(TBOX_ID module_id);
VOID tbox_module_set_state(TBOX_ID module_id, TBOX_MODULE_STATE state);
TBOX_MODULE_STATE tbox_module_get_state(TBOX_ID module_id);
BOOL tbox_allmodule_canbe_stop(VOID);
BOOL tbox_allmodule_isstoped(VOID);
BOOL tbox_allmodule_istarted(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_MODULE_H */