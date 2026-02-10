#ifndef TBOX_CORE_H
#define TBOX_CORE_H

#include "tbox_config.h"
#include "tbox_common.h"
#include "tbox_module.h"
#include "tbox_log.h"
#include "tbox_memory.h"
#include "tbox_message.h"
#include "tbox_supervise.h"

/*********************************************************
 *CORE说明
 * 1、启动流程：
 *   1.1、系统启动后，可以直接调用tbox_core_main()完成core的初始化，模块加载和启动。
 *        也可以按照tbox_core_init->tbox_core_start调用顺序完成core的初始化和启动，
 *        具体时序可以参照tbox_core_main()。
 *   1.2、core的初始化顺序为：memory->log->task->module->message->supervise。
 *   1.3、core的启动顺序为：log->task->supervise。memory,module,message等模块没有
 *        处理实体，所以只要初始化就可以了。
 * 
 * 2、模块加载流程：
 *   模块通过TBOX_MODULE()注册到core,core会根据注册的优先级进行加载，初始化和启动。
 * 
 * 3、模块注册：
 *   2.1、首先定义模块相应函数，包括初始化，启动，停止，使能，退出和是否允许停止的函数。
 *        模块可以根据自身业务需要实现对应函数，如果不需要实现，可以调用TBOX_MODULE_FUN()
 *        时设置为NULL。
 *   2.2、然后调用TBOX_MODULE()注册模块，注册时需要提供模块名称，优先级，日志级别，堆栈大小，
 *        是否为接口模块，是否为关键模块。注册模块后保存模块的ID和模块任务HANDLE。对于非critical
 *        任务，任务HANDLE为空，对于critical任务，模块关联到对应的Task，任务HANDLE为Task的hanle。
 *   2.3、runloop任务调用TBOX_RUNLOOP_MODULE()注册。有些模块直接从中断接收数据，需要在runloop中
 *        不断处理这些数据，对于中断的特殊性，不能用TBOX_MODULE()注册并且通过发消息的发送实现，那么
 *        可以TBOX_RUNLOOP_MODULE()注册runloop任务。模块注册后保存模块对应任务的handle，模块可以通过
 *        handle调用freeRTOS的API（如xTaskNotify）进行事件通知，runloop任务等待事件通知(如xTaskNotifyWait)
 *        收到消息后处理。
 *   2.4、模块如果需要发送消息，可以调用TBOX_MESSSAGE()定义消息，在模块加载(TBOX_MODULE_LOADER)的时候，注册
 *        消息。模块需要通过TBOX_MODULE_LOADER()实现模块加载逻辑，在通过LOAD_TBOX_MODULE()加载模块。
 *   2.5、模块加载（注册）完成后，模块的ID和HANDLE都被保存，可以供外部模块使用。模块加载(注册)后。
 *   2.6、所有模块加载完成后，core可以调用tbox_module_init_all_regmodule()初始化模块，调用tbox_core_start()
 *        启动模块。
 *   2.7、所有模块的停止（休眠）。所有模块需要停止之前应该调用tbox_allmodule_canbe_stop()判断是否可以停止，
 *        只有tbox_allmodule_canbe_stop()返回TRUE才可以停止。如果所有模块都可以停止，则调用tbox_module_stop()
 *        停止所有注册模块运行。 
 * ********************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TBOX_MODULE_FUN(module, init, stop, start, enable, exit, canbe_stop)\
    MODULE_INIT_FUN tbox_##module##_init = init;\
    MODULE_STOP_FUN tbox_##module##_stop = stop;\
    MODULE_START_FUN tbox_##module##_start = start;\
    MODULE_ENABLE_FUN tbox_##module##_enable = enable;\
    MODULE_EXIT_FUN tbox_##module##_exit = exit;\
    MODULE_CANBE_STOP_FUN tbox_##module##_can_be_stop = canbe_stop;

#define TBOX_MODULE(module, pri, level, size, interface, critical)\
    TBOX_ID tbox_##module##_id = TBOX_ID_INVALID;\
    MODULE_HANDLE tbox_##module##_handle = NULL_PTR;\
    VOID tbox_##module##_load_other(VOID);\
    VOID tbox_##module##_load(VOID);\
    VOID tbox_##module##_load(VOID)\
    {\
        TBOX_MODULE_INFO info = {\
            .name = #module,\
            .priority = pri,\
            .log_level = level,\
            .stack_size = size,\
            .is_interface = interface,\
            .is_critical = critical,\
            .is_runloop = FALSE,\
            .init_fun = tbox_##module##_init,\
            .stop_fun = tbox_##module##_stop,\
            .start_fun = tbox_##module##_start,\
            .enable_fun = tbox_##module##_enable,\
            .exit_fun = tbox_##module##_exit,\
            .canbe_stop_fun = tbox_##module##_can_be_stop\
        };\
        tbox_##module##_load_other();\
        tbox_##module##_id = tbox_module_register(&info);\
        tbox_##module##_handle = tbox_module_get_handle(tbox_##module##_id);\
    }

#define TBOX_RUNLOOP_MODULE(module, pri, level, size, runloop)\
    TBOX_ID tbox_##module##_id = TBOX_ID_INVALID;\
    MODULE_HANDLE tbox_##module##_handle = NULL_PTR;\
    VOID tbox_##module##_load_other(VOID);\
    VOID tbox_##module##_load(VOID);\
    VOID tbox_##module##_load(VOID)\
    {\
        TBOX_MODULE_INFO info = {\
            .name = #module,\
            .priority = pri,\
            .log_level = level,\
            .stack_size = size,\
            .is_interface = FALSE,\
            .is_critical = FALSE,\
            .is_runloop = TRUE,\
            .init_fun = tbox_##module##_init,\
            .stop_fun = tbox_##module##_stop,\
            .start_fun = tbox_##module##_start,\
            .enable_fun = tbox_##module##_enable,\
            .exit_fun = tbox_##module##_exit,\
            .canbe_stop_fun = tbox_##module##_can_be_stop\
        };\
        tbox_##module##_load_other();\
        tbox_##module##_id = tbox_module_register(&info);\
        if(TBOX_E_OK == tbox_module_start_runloop(tbox_##module##_id, runloop))\
        {\
            tbox_##module##_handle = tbox_module_get_handle(tbox_##module##_id);\
        }\
    }

#define TBOX_MODULE_LOADER(module)\
    VOID tbox_##module##_load_other(VOID)

#define GET_TBOX_MODULE_ID(module, id) {extern TBOX_ID tbox_##module##_id; id = tbox_##module##_id;}

#define GET_TBOX_MODULE_HANDLE(module, handle) {extern MODULE_HANDLE tbox_##module##_handle; handle = tbox_##module##_handle;}

#define LOAD_TBOX_MODULE(module)\
    {VOID tbox_##module##_load(VOID);\
     tbox_##module##_load();}

#define TBOX_MESSSAGE(message, msg_pri, msg_type)\
    VOID tbox_##message##_register(VOID);\
    VOID tbox_##message##_register(VOID)\
    {\
        TBOX_MSG_REGINFO reginfo = {\
            .name = #message,\
            .priority = msg_pri,\
            .type = msg_type,\
            .enable = TRUE\
        };\
        tbox_message_register(&reginfo);\
    }\

#define REGISTRY_TBOX_MESSAGE(message)\
    {VOID tbox_##message##_register(VOID);\
     tbox_##message##_register();}

typedef VOID (*TBOX_CORE_LOAD_FUN)(VOID);

#define TBOX_CORE_STATE_NOINIT 0x00
#define TBOX_CORE_STATE_STOP   0x01
#define TBOX_CORE_STATE_START  0x02

INT32 tbox_core_init(VOID);
VOID tbox_core_deinit(VOID);
INT32 tbox_core_start(BOOL start_module);
VOID tbox_core_stop(BOOL stop_module);
UINT8 tbox_core_get_state(VOID);
VOID tbox_core_main(TBOX_CORE_LOAD_FUN load_module_fun);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_CORE_H */
