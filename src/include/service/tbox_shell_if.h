#ifndef TBOX_SHELL_IF_H
#define TBOX_SHELL_IF_H

#include "tbox_type.h"
#include "api_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/*采用freeRTOS的CLI命令行接口，定义一个命令行命令，定义shell执行函数为:
 *BaseType_t shell_exe_func(CHAR *buf, UINT32 bufsz, const CHAR *cmd) 
 */
#define TBOX_SHELL_DEFINE(name, tip, param_num, exec)\
    static CLI_Command_Definition_t tbox_shell_##name = {#name, tip, exec, param_num}

#define TBOX_SHELL_REGISTER(name)\
    {\
        FreeRTOS_CLIRegisterCommand(&tbox_shell_##name);\
    }
    
#ifdef __cplusplus
}
#endif

#endif //TBOX_SHELL_IF_H