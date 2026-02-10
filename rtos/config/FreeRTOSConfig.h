/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*******************************************************************************
 * 本文件提供了一个FreeRTOSConfig.h头文件示例，包含每个配置项的简要说明。
 * 在线文档和参考文档提供了更多信息。
 * https://www.freertos.org/a00110.html
 *
 * 方括号('['和']')中的常量值必须在使用本文件前完成设置。
 *
 * 如果有可用的RTOS端口提供的FreeRTOSConfig.h文件，请使用该文件而非此通用文件。
 ******************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#if (defined(__ARMCC_VERSION) || defined(__GNUC__) || defined(__ICCARM__))
#include <stdint.h>
#include "Device_Register.h"
#endif

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __NVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS                                     __NVIC_PRIO_BITS
#define configHARDWARE_INTERRUPT_PRIORITY(priority)         ((priority) << (8 - configPRIO_BITS))

#else
/* 7 priority levels */
#define configPRIO_BITS                                     4
#define configHARDWARE_INTERRUPT_PRIORITY(priority)         ((priority) << (8 - configPRIO_BITS))

#endif




/******************************************************************************/
/* 硬件描述相关定义 ***********************************************************/
/******************************************************************************/

/* 在大多数情况下，configCPU_CLOCK_HZ必须设置为驱动内核周期性tick中断的外设时钟频率。
 * 默认值设置为20MHz，与QEMU演示设置匹配。您的应用程序肯定需要不同的值，请正确设置。
 * 这通常（但不总是）等于主系统时钟频率。 */
#define configCPU_CLOCK_HZ                                   (120000000UL)

/* configSYSTICK_CLOCK_HZ是仅适用于ARM Cortex-M端口的可选参数。
 *
 * 默认情况下，ARM Cortex-M端口从Cortex-M SysTick定时器生成RTOS tick中断。
 * 大多数Cortex-M MCU以与MCU本身相同的频率运行SysTick定时器 - 在这种情况下不需要configSYSTICK_CLOCK_HZ，
 * 应保持未定义。如果SysTick定时器的时钟频率与MCU核心不同，则将configCPU_CLOCK_HZ设置为MCU时钟频率，
 * 如常，并将configSYSTICK_CLOCK_HZ设置为SysTick时钟频率。如果未定义则不使用。
 * 默认值未定义（注释掉）。如果需要此值，请取消注释并设置为适当的值。 */

/*
 #define configSYSTICK_CLOCK_HZ                          [平台特定]
 */

/******************************************************************************/
/* 调度行为相关定义 ***********************************************************/
/******************************************************************************/

/* configTICK_RATE_HZ以Hz为单位设置tick中断的频率，通常根据configCPU_CLOCK_HZ值计算。 */
#define configTICK_RATE_HZ                                   1000

/* 将configUSE_PREEMPTION设置为1使用抢占式调度。设置为0使用协作式调度。
 * 参见https://www.freertos.org/single-core-amp-smp-rtos-scheduling.html。 */
#define configUSE_PREEMPTION                                 1

/* 将configUSE_TIME_SLICING设置为1，使调度程序在每个tick中断时在就绪状态的同等优先级任务之间切换。
 * 设置为0可防止调度程序仅因为有tick中断就在就绪状态任务之间切换。
 * 参见https://freertos.org/single-core-amp-smp-rtos-scheduling.html。 */
#define configUSE_TIME_SLICING                               1

/* 将configUSE_PORT_OPTIMISED_TASK_SELECTION设置为1，使用针对目标硬件指令集优化的算法选择下一个要运行的任务 -
 * 通常使用计数前导零汇编指令。设置为0使用适用于所有FreeRTOS端口的通用C算法选择下一个任务。
 * 并非所有FreeRTOS端口都有此选项。如果未定义则默认为0。 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION              0

/* 将configUSE_TICKLESS_IDLE设置为1使用低功耗无tick模式。设置为0保持tick中断始终运行。
 * 并非所有FreeRTOS端口都支持无tick模式。参见https://www.freertos.org/low-power-tickless-rtos.html
 * 如果未定义则默认为0。 */
#define configUSE_TICKLESS_IDLE                              0

/* configMAX_PRIORITIES设置可用的任务优先级数量。任务可分配优先级从0到(configMAX_PRIORITIES - 1)。
 * 0是最低优先级。 */
#define configMAX_PRIORITIES                                 20

/* configMINIMAL_STACK_SIZE定义空闲任务使用的堆栈大小（以字为单位，不是字节！）。
 * 内核不将此常量用于任何其他目的。演示应用程序使用此常量使演示在不同硬件架构间具有一定可移植性。 */
#define configMINIMAL_STACK_SIZE                             128

/* configMAX_TASK_NAME_LEN设置任务人类可读名称的最大长度（以字符为单位）。包括NULL终止符。 */
#define configMAX_TASK_NAME_LEN                              16

/* 时间以'tick'为单位测量 - 这是自RTOS内核启动以来tick中断执行的次数。
 * tick计数保存在TickType_t类型的变量中。
 *
 * configTICK_TYPE_WIDTH_IN_BITS控制TickType_t的类型（因此也是位宽）：
 *
 * 将configTICK_TYPE_WIDTH_IN_BITS定义为TICK_TYPE_WIDTH_16_BITS会导致
 * TickType_t被定义（typedef）为无符号16位类型。
 *
 * 将configTICK_TYPE_WIDTH_IN_BITS定义为TICK_TYPE_WIDTH_32_BITS会导致
 * TickType_t被定义（typedef）为无符号32位类型。
 *
 * 将configTICK_TYPE_WIDTH_IN_BITS定义为TICK_TYPE_WIDTH_64_BITS会导致
 * TickType_t被定义（typedef）为无符号64位类型。 */
#define configTICK_TYPE_WIDTH_IN_BITS                        TICK_TYPE_WIDTH_32_BITS

/* 将configIDLE_SHOULD_YIELD设置为1，如果有可以运行的优先级为0（空闲优先级）的应用程序任务，则空闲任务会让出给该应用程序任务。
 * 设置为0使空闲任务使用其所有时间片。如果未定义则默认为1。 */
#define configIDLE_SHOULD_YIELD                              0

/* 每个任务都有一个任务通知数组。
 * configTASK_NOTIFICATION_ARRAY_ENTRIES设置数组中的索引数量。
 * 参见https://www.freertos.org/RTOS-task-notifications.html 如果未定义则默认为1。 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES                1

/* configQUEUE_REGISTRY_SIZE设置可以从队列注册表引用的队列和信号量的最大数量。
 * 仅在使用内核感知调试器时需要。如果未定义则默认为0。 */
#define configQUEUE_REGISTRY_SIZE                            0

/* 将configENABLE_BACKWARD_COMPATIBILITY设置为1，将旧版FreeRTOS的函数名和数据类型映射到其最新等效项。
 * 如果未定义则默认为1。 */
#define configENABLE_BACKWARD_COMPATIBILITY                  0

/* 每个任务都有自己的指针数组，可用作线程本地存储。
 * configNUM_THREAD_LOCAL_STORAGE_POINTERS设置数组中的索引数量。
 * 参见https://www.freertos.org/thread-local-storage-pointers.html
 * 如果未定义则默认为0。 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS              0

/* 当configUSE_MINI_LIST_ITEM设置为0时，MiniListItem_t和ListItem_t相同。
 * 当configUSE_MINI_LIST_ITEM设置为1时，MiniListItem_t比ListItem_t少3个字段，
 * 这节省了一些RAM，但违反了一些编译器用于优化的严格别名规则。如果未定义则默认为1。 */
#define configUSE_MINI_LIST_ITEM                             1

/* 设置xTaskCreate()参数中指定任务堆栈大小的类型。相同的类型用于各种其他API调用中返回有关堆栈使用的信息。
 * 如果未定义则默认为size_t。 */
#define configSTACK_DEPTH_TYPE                               size_t

/* configMESSAGE_BUFFER_LENGTH_TYPE设置用于存储写入FreeRTOS消息缓冲区的每条消息长度的类型
 * （长度也会写入消息缓冲区）。如果未定义则默认为size_t - 但如果消息长度从不超过uint8_t可容纳的长度，则可能会浪费空间。 */
#define configMESSAGE_BUFFER_LENGTH_TYPE                     size_t

/* 如果configHEAP_CLEAR_MEMORY_ON_FREE设置为1，则使用pvPortMalloc()分配的内存块在通过vPortFree()释放时将被清除（即设置为零）。
 * 如果未定义则默认为0。 */
#define configHEAP_CLEAR_MEMORY_ON_FREE                      0

/* vTaskList和vTaskGetRunTimeStats API将缓冲区作为参数，并假设缓冲区的长度为configSTATS_BUFFER_MAX_LENGTH。
 * 如果未定义则默认为0xFFFF。
 * 建议新应用程序使用vTaskListTasks和vTaskGetRunTimeStatistics API，并显式提供缓冲区长度以避免内存损坏。 */
#define configSTATS_BUFFER_MAX_LENGTH                        0xFFFF

/* 将configUSE_NEWLIB_REENTRANT设置为1为每个任务分配一个newlib reent结构。
 * 设置为0不支持newlib reent结构。如果未定义则默认为0。
 * 注意：Newlib支持因大众需求而被包含，但FreeRTOS维护者本身并不使用或测试。
 * FreeRTOS不对由此产生的newlib操作负责。用户必须熟悉newlib并必须提供
 * 必要的存根的系统级实现。注意（在撰写本文时）当前的newlib设计实现了
 * 必须提供锁的系统级malloc()。 */
#define configUSE_NEWLIB_REENTRANT                           0

/******************************************************************************/
/* 软件定时器相关定义 ********************************************************/
/******************************************************************************/

/* 将configUSE_TIMERS设置为1在构建中包含软件定时器功能。
 * 设置为0从构建中排除软件定时器功能。如果configUSE_TIMERS设置为1，
 * 则必须将FreeRTOS/source/timers.c源文件包含在构建中。
 * 如果未定义则默认为0。参见https://www.freertos.org/RTOS-software-timer.html。 */
#define configUSE_TIMERS                                     1

/* configTIMER_TASK_PRIORITY设置定时器任务使用的优先级。
 * 仅在configUSE_TIMERS设置为1时使用。定时器任务是标准的FreeRTOS任务，
 * 因此其优先级设置与其他任务相同。参见
 * https://www.freertos.org/RTOS-software-timer-service-daemon-task.html
 * 仅在configUSE_TIMERS设置为1时使用。 */
#define configTIMER_TASK_PRIORITY                            (configMAX_PRIORITIES - 1)

/* configTIMER_TASK_STACK_DEPTH设置分配给定时器任务的堆栈大小（以字为单位，不是字节！）。
 * 定时器任务是标准的FreeRTOS任务。参见
 * https://www.freertos.org/RTOS-software-timer-service-daemon-task.html
 * 仅在configUSE_TIMERS设置为1时使用。 */
#define configTIMER_TASK_STACK_DEPTH                         (configMINIMAL_STACK_SIZE * 4)

/* configTIMER_QUEUE_LENGTH设置用于向定时器任务发送命令的队列长度
 * （队列可以容纳的离散项目数）。参见
 * https://www.freertos.org/RTOS-software-timer-service-daemon-task.html
 * 仅在configUSE_TIMERS设置为1时使用。 */
#define configTIMER_QUEUE_LENGTH                             10

/******************************************************************************/
/* 事件组相关定义 ************************************************************/
/******************************************************************************/

/* 将configUSE_EVENT_GROUPS设置为1在构建中包含事件组功能。
 * 设置为0从构建中排除事件组功能。如果configUSE_EVENT_GROUPS设置为1，
 * 则必须将FreeRTOS/source/event_groups.c源文件包含在构建中。
 * 如果未定义则默认为1。 */

#define configUSE_EVENT_GROUPS                               1

/******************************************************************************/
/* 流缓冲区相关定义 *********************************************************/
/******************************************************************************/

/* 将configUSE_STREAM_BUFFERS设置为1在构建中包含流缓冲区功能。
 * 设置为0从构建中排除事件组功能。如果configUSE_STREAM_BUFFERS设置为1，
 * 则必须将FreeRTOS/source/stream_buffer.c源文件包含在构建中。
 * 如果未定义则默认为1。 */

#define configUSE_STREAM_BUFFERS                             1

/******************************************************************************/
/* 内存分配相关定义 **********************************************************/
/******************************************************************************/

/* 将configSUPPORT_STATIC_ALLOCATION设置为1在构建中包含使用静态分配内存创建
 * FreeRTOS对象（任务、队列等）的FreeRTOS API函数。设置为0从构建中排除创建
 * 静态分配对象的能力。如果未定义则默认为0。参见
 * https://www.freertos.org/Static_Vs_Dynamic_Memory_Allocation.html。 */
#define configSUPPORT_STATIC_ALLOCATION                      0

/* 将configSUPPORT_DYNAMIC_ALLOCATION设置为1在构建中包含使用动态分配内存创建
 * FreeRTOS对象（任务、队列等）的FreeRTOS API函数。设置为0从构建中排除创建
 * 动态分配对象的能力。如果未定义则默认为1。参见
 * https://www.freertos.org/Static_Vs_Dynamic_Memory_Allocation.html。 */
#define configSUPPORT_DYNAMIC_ALLOCATION                     1

/* 当heap_1.c、heap_2.c或heap_4.c包含在构建中时，设置FreeRTOS堆的总大小（以字节为单位）。
 * 此值默认为4096字节，但必须针对每个应用程序进行调整。注意堆将出现在.bss部分。
 * 参见https://www.freertos.org/a00111.html。 */
#define configTOTAL_HEAP_SIZE                                (64 * 1024) /*根据业务需求大小调整大小*/

/* 将configAPPLICATION_ALLOCATED_HEAP设置为1让应用程序分配用作FreeRTOS堆的数组。
 * 设置为0让链接器分配用作FreeRTOS堆的数组。如果未定义则默认为0。 */
#define configAPPLICATION_ALLOCATED_HEAP                     0

/* 将configSTACK_ALLOCATION_FROM_SEPARATE_HEAP设置为1让任务堆栈从FreeRTOS堆以外的地方分配。
 * 如果想确保堆栈保存在快速内存中，这很有用。设置为0让任务堆栈来自标准FreeRTOS堆。
 * 如果设置为1，应用程序编写者必须提供pvPortMallocStack()和vPortFreeStack()的实现。
 * 如果未定义则默认为0。 */
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP            0

/* 将configENABLE_HEAP_PROTECTOR设置为1启用heap_4.c和heap_5.c中对内部堆块指针的边界检查和混淆，
 * 以帮助捕获指针损坏。如果未定义则默认为0。 */
#define configENABLE_HEAP_PROTECTOR                          0

/******************************************************************************/
/* 中断嵌套行为配置 *********************************************************/
/******************************************************************************/

/* configKERNEL_INTERRUPT_PRIORITY设置tick和执行上下文切换的中断优先级。
 * 并非所有FreeRTOS端口都支持。有关ARM Cortex-M设备的特定信息，参见
 * https://www.freertos.org/RTOS-Cortex-M3-M4.html。 */



/* 配置内核中断（SysTick、PendSV）的硬件优先级，确保它们低于高优先级外设中断
（如硬件定时器、通信接口），但高于用户任务。*/
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY              0x0F

/* 配置FreeRTOS API调用的安全中断优先级阈值， 高于此优先级
（ ＜ configLIBRARY_API_CALL_INTERRUPT_PRIORITY）
的不能调用 FreeRTOS API, 低于或等于此优先级的可以安全调用 FreeRTOS API*/
#define configLIBRARY_API_CALL_INTERRUPT_PRIORITY            2

#define configKERNEL_INTERRUPT_PRIORITY                      configHARDWARE_INTERRUPT_PRIORITY(configLIBRARY_LOWEST_INTERRUPT_PRIORITY)

/* configMAX_SYSCALL_INTERRUPT_PRIORITY设置中断优先级，高于此优先级的
 * FreeRTOS API调用不得进行。高于此优先级的中断永远不会被禁用，因此永远不会
 * 被RTOS活动延迟。默认值设置为最高中断优先级（0）。
 * 并非所有FreeRTOS端口都支持。有关ARM Cortex-M设备的特定信息，参见
 * https://www.freertos.org/RTOS-Cortex-M3-M4.html。 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY                 configHARDWARE_INTERRUPT_PRIORITY(configLIBRARY_API_CALL_INTERRUPT_PRIORITY)

/* configMAX_SYSCALL_INTERRUPT_PRIORITY的另一个名称 - 使用的名称取决于FreeRTOS端口。 */
#define configMAX_API_CALL_INTERRUPT_PRIORITY                configHARDWARE_INTERRUPT_PRIORITY(configLIBRARY_API_CALL_INTERRUPT_PRIORITY)

/******************************************************************************/
/* 钩子和回调函数相关定义 ****************************************************/
/******************************************************************************/

/* 将以下configUSE_*常量设置为1在构建中包含命名的钩子功能。
 * 设置为0从构建中排除钩子功能。对于任何设置为1的钩子功能，
 * 应用程序编写者负责提供钩子函数。参见https://www.freertos.org/a00016.html。 */
#define configUSE_IDLE_HOOK                                  0
#define configUSE_TICK_HOOK                                  0
#define configUSE_MALLOC_FAILED_HOOK                         1
#define configUSE_DAEMON_TASK_STARTUP_HOOK                   0

/* 将configUSE_SB_COMPLETED_CALLBACK设置为1为每个流缓冲区或消息缓冲区实例
 * 提供发送和接收完成的回调。当选项设置为1时，可以使用API
 * xStreamBufferCreateWithCallback()和xStreamBufferCreateStaticWithCallback()
 * （以及消息缓冲区的类似API）创建带有应用程序提供的回调的流缓冲区或消息缓冲区实例。
 * 如果未定义则默认为0。 */
#define configUSE_SB_COMPLETED_CALLBACK                      0

/* 将configCHECK_FOR_STACK_OVERFLOW设置为1或2让FreeRTOS在上下文切换时检查堆栈溢出。
 * 设置为0不检查堆栈溢出。如果configCHECK_FOR_STACK_OVERFLOW为1，
 * 则检查仅在任务上下文保存到其堆栈时检查堆栈指针是否越界 - 这很快但有些无效。
 * 如果configCHECK_FOR_STACK_OVERFLOW为2，则检查任务堆栈末尾写入的模式是否被覆盖。
 * 这较慢，但会捕获大多数（但不是全部）堆栈溢出。当configCHECK_FOR_STACK_OVERFLOW
 * 设置不为0时，应用程序编写者必须提供堆栈溢出回调。
 * 参见https://www.freertos.org/Stacks-and-stack-overflow-checking.html
 * 如果未定义则默认为0。 */
#define configCHECK_FOR_STACK_OVERFLOW                       2

/******************************************************************************/
/* 运行时和任务统计收集相关定义 *********************************************/
/******************************************************************************/

/* 将configGENERATE_RUN_TIME_STATS设置为1让FreeRTOS收集每个任务使用的处理时间数据。
 * 设置为0不收集数据。如果设置为1，应用程序编写者需要提供时钟源。
 * 如果未定义则默认为0。参见https://www.freertos.org/rtos-run-time-stats.html。 */
#define configGENERATE_RUN_TIME_STATS                        0

#if configGENERATE_RUN_TIME_STATS == 1

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()              \
    do {                                                      \
        MODIFY_MEM32(0xE000EDFCU, 0x01000000, 24U, 1U);       \
        WRITE_MEM32(0xE0001004U, 0U);                         \
        MODIFY_MEM32(0xE0001000U, 0x00000001, 0U, 1U);        \
    } while (0)

#define portGET_RUN_TIME_COUNTER_VALUE()                      READ_MEM32(0xE0001004U)

#endif

/* 将configUSE_TRACE_FACILITY设置为1在任务结构成员中包含跟踪和可视化功能及工具
 * 使用的附加信息。设置为0从结构中排除附加信息。如果未定义则默认为0。 */
#define configUSE_TRACE_FACILITY                             1

/* 设置为1在构建中包含vTaskList()和vTaskGetRunTimeStats()函数。
 * 设置为0从构建中排除这些函数。这两个函数引入了对字符串格式化函数的依赖，
 * 否则这些依赖将不存在 - 因此它们被单独保留。如果未定义则默认为0。 */
#define configUSE_STATS_FORMATTING_FUNCTIONS                 1

/******************************************************************************/
/* 协程相关定义 *************************************************************/
/******************************************************************************/

/* 将configUSE_CO_ROUTINES设置为1在构建中包含协程功能，
 * 或0从构建中省略协程功能。要包含协程，必须将croutine.c包含在项目中。
 * 如果未定义则默认为0。 */
#define configUSE_CO_ROUTINES                                1

/* configMAX_CO_ROUTINE_PRIORITIES定义可用于应用程序协程的优先级数量。
 * 任意数量的协程可以共享相同的优先级。如果未定义则默认为0。 */
#define configMAX_CO_ROUTINE_PRIORITIES                      1

/******************************************************************************/
/* 调试辅助 ***************************************************************/
/******************************************************************************/

/* configASSERT()与标准C assert()具有相同的语义。它可以定义为在断言失败时采取行动，
 * 或者完全不被定义（即注释掉或删除定义）以完全移除断言。configASSERT()可以定义为任何你想要的，
 * 例如可以在断言失败时调用一个函数，传递失败断言的文件名和行号
 * （例如，"vAssertCalled( __FILE__, __LINE__ )"），或者简单地禁用中断并在循环中停止所有执行，
 * 以便在调试器中查看失败的行。 */
#define configASSERT(x)                                                            \
    if ((x) == 0) {                                                                \
        taskDISABLE_INTERRUPTS();                                                  \
        for (;;)                                                                   \
            ;                                                                      \
    }



/******************************************************************************/
/* FreeRTOS MPU特定定义 *******************************************************/
/******************************************************************************/

/* 如果configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS设置为1，
 * 则应用程序编写者可以提供在特权模式下执行的函数。
 * 参见：https://www.freertos.org/a00110.html#configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS
 * 如果未定义则默认为0。仅由FreeRTOS Cortex-M MPU端口使用，而非标准ARMv7-M Cortex-M端口。 */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS         0

/* 将configTOTAL_MPU_REGIONS设置为目标硬件上实现的MPU区域数量。通常为8或16。
 * 仅由FreeRTOS Cortex-M MPU端口使用，而非标准ARMv7-M Cortex-M端口。
 * 如果未定义则默认为8。 */
#define configTOTAL_MPU_REGIONS                                        8

/* configTEX_S_C_B_FLASH允许应用程序编写者覆盖Flash MPU区域的TEX、可共享(S)、可缓存(C)和可缓冲(B)位的默认值。
 * 如果未定义则默认为0x07UL（表示TEX=000，S=1，C=1，B=1）。
 * 仅由FreeRTOS Cortex-M MPU端口使用，而非标准ARMv7-M Cortex-M端口。 */
#define configTEX_S_C_B_FLASH                                          0x07UL

/* configTEX_S_C_B_SRAM允许应用程序编写者覆盖RAM MPU区域的TEX、可共享(S)、可缓存(C)和可缓冲(B)位的默认值。
 * 如果未定义则默认为0x07UL（表示TEX=000，S=1，C=1，B=1）。
 * 仅由FreeRTOS Cortex-M MPU端口使用，而非标准ARMv7-M Cortex-M端口。 */
#define configTEX_S_C_B_SRAM                                           0x07UL

/* 将configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY设置为0可防止任何源自内核代码本身之外的权限提升。
 * 设置为1允许应用程序任务提升权限。如果未定义则默认为1。
 * 仅由FreeRTOS Cortex-M MPU端口使用，而非标准ARMv7-M Cortex-M端口。 */
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY                    0

/* 将configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS设置为1允许非特权任务进入临界区（有效地屏蔽中断）。
 * 设置为0防止非特权任务进入临界区。如果未定义则默认为1。
 * 仅由FreeRTOS Cortex-M MPU端口使用，而非标准ARMv7-M Cortex-M端口。 */
#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS                     1

/* FreeRTOS内核版本10.6.0引入了新的v2 MPU包装器，即mpu_wrappers_v2.c。
 * 将configUSE_MPU_WRAPPERS_V1设置为0使用新的v2 MPU包装器。
 * 设置为1使用旧的v1 MPU包装器（mpu_wrappers.c）。如果未定义则默认为0。 */
#define configUSE_MPU_WRAPPERS_V1                                      0

/* 当使用v2 MPU包装器时，将configPROTECTED_KERNEL_OBJECT_POOL_SIZE设置为
 * 内核对象的总数，包括任务、队列、信号量、互斥量、事件组、定时器、流缓冲区和消息缓冲区。
 * 应用程序在任何时间点都不能拥有超过configPROTECTED_KERNEL_OBJECT_POOL_SIZE个内核对象。 */
#define configPROTECTED_KERNEL_OBJECT_POOL_SIZE                        10

/* 当使用v2 MPU包装器时，将configSYSTEM_CALL_STACK_SIZE设置为系统调用堆栈的大小（以字为单位）。
 * 每个任务都有一个静态分配的此大小的内存缓冲区，用作执行系统调用的堆栈。
 * 例如，如果configSYSTEM_CALL_STACK_SIZE定义为128，并且应用程序中有10个任务，
 * 则用于系统调用堆栈的总内存量为128 * 10 = 1280字。 */
#define configSYSTEM_CALL_STACK_SIZE                                   128

/* 当使用v2 MPU包装器时，将configENABLE_ACCESS_CONTROL_LIST设置为1启用访问控制列表(ACL)功能。
 * 当ACL启用时，非特权任务默认无权访问除自身之外的任何内核对象。
 * 应用程序编写者需要使用提供的API显式授予非特权任务对其所需内核对象的访问权限。
 * 如果未定义则默认为0。 */
#define configENABLE_ACCESS_CONTROL_LIST                               1

/******************************************************************************/
/* SMP(对称多处理)特定配置定义 ***********************************************/
/******************************************************************************/

/* 将configNUMBER_OF_CORES设置为可用的处理器核心数量。如果未定义则默认为1。 */

/*
 #define configNUMBER_OF_CORES                             [可用核心数]
 */

/* 当使用SMP（即configNUMBER_OF_CORES大于1）时，将configRUN_MULTIPLE_PRIORITIES设置为0，
 * 仅允许优先级不相等的多个任务同时运行，从而保持较低优先级任务在较高优先级任务能够运行时永不运行的范式。
 * 如果configRUN_MULTIPLE_PRIORITIES设置为1，则不同优先级的多个任务可能同时运行 -
 * 因此较高和较低优先级的任务可能在不同核心上同时运行。 */
#define configRUN_MULTIPLE_PRIORITIES                                  0

/* 当使用SMP（即configNUMBER_OF_CORES大于1）时，将configUSE_CORE_AFFINITY设置为1启用核心亲和性功能。
 * 当核心亲和性功能启用时，可以使用vTaskCoreAffinitySet和vTaskCoreAffinityGet API
 * 设置和检索任务可以运行的核心。如果configUSE_CORE_AFFINITY设置为0，
 * 则FreeRTOS调度程序可以自由在任何可用核心上运行任何任务。 */
#define configUSE_CORE_AFFINITY                                        0

/* 当使用启用核心亲和性功能的SMP时，设置configTASK_DEFAULT_CORE_AFFINITY以更改
 * 未指定亲和性掩码创建的任务的默认核心亲和性掩码。将此定义设置为1将使此类任务在核心0上运行，
 * 设置为(1 << portGET_CORE_ID())将使此类任务在当前核心上运行。
 * 如果交换任务核心不受支持（例如Tricore）或需要控制遗留代码，此配置值很有用。
 * 如果未定义则默认为tskNO_AFFINITY。 */
#define configTASK_DEFAULT_CORE_AFFINITY                               tskNO_AFFINITY

/* 当使用SMP（即configNUMBER_OF_CORES大于1）时，如果configUSE_TASK_PREEMPTION_DISABLE设置为1，
 * 可以使用vTaskPreemptionDisable和vTaskPreemptionEnable API将单个任务设置为抢占或协作模式。 */
#define configUSE_TASK_PREEMPTION_DISABLE                              0

/* 当使用SMP（即configNUMBER_OF_CORES大于1）时，将configUSE_PASSIVE_IDLE_HOOK设置为1，
 * 允许应用程序编写者使用被动空闲任务钩子添加后台功能，而无需单独任务的开销。
 * 如果未定义则默认为0。 */
#define configUSE_PASSIVE_IDLE_HOOK                                    0

/* 当使用SMP（即configNUMBER_OF_CORES大于1）时，configTIMER_SERVICE_TASK_CORE_AFFINITY
 * 允许应用程序编写者设置RTOS守护进程/定时器服务任务的核心亲和性。
 * 如果未定义则默认为tskNO_AFFINITY。 */
#define configTIMER_SERVICE_TASK_CORE_AFFINITY                         tskNO_AFFINITY

/******************************************************************************/
/* ARMv8-M安全端口相关定义 ***************************************************/
/******************************************************************************/

/* secureconfigMAX_SECURE_CONTEXTS定义可以调用ARMv8-M芯片安全端的任务的最大数量。
 * 不被任何其他端口使用。 */
#define secureconfigMAX_SECURE_CONTEXTS                                5

/* 定义内核提供的vApplicationGetIdleTaskMemory()和vApplicationGetTimerTaskMemory()实现，
 * 以分别提供空闲任务和定时器任务使用的内存。应用程序可以通过将configKERNEL_PROVIDED_STATIC_MEMORY
 * 设置为0或保持未定义来提供自己的vApplicationGetIdleTaskMemory()和vApplicationGetTimerTaskMemory()实现。 */
#define configKERNEL_PROVIDED_STATIC_MEMORY                            1

/******************************************************************************/
/* ARMv8-M端口特定配置定义 ***************************************************/
/******************************************************************************/

/* 当在非安全端运行FreeRTOS时，将configENABLE_TRUSTZONE设置为1以启用FreeRTOS ARMv8-M端口中的TrustZone支持，
 * 允许非安全FreeRTOS任务调用从安全端导出的（非安全可调用）函数。 */
#define configENABLE_TRUSTZONE                                         1

/* 如果应用程序编写者不想使用TrustZone，但硬件不支持禁用TrustZone，
 * 则整个应用程序（包括FreeRTOS调度程序）可以在安全端运行，而无需分支到非安全端。
 * 为此，除了将configENABLE_TRUSTZONE设置为0外，还需将configRUN_FREERTOS_SECURE_ONLY设置为1。 */
#define configRUN_FREERTOS_SECURE_ONLY                                 1

/* 将configENABLE_MPU设置为1以启用内存保护单元(MPU)，或0以保持内存保护单元禁用。 */
#define configENABLE_MPU                                               1

/* 将configENABLE_FPU设置为1以启用浮点单元(FPU)，或0以保持浮点单元禁用。 */
#define configENABLE_FPU                                               1

/* 将configENABLE_MVE设置为1以启用M-Profile向量扩展(MVE)支持，
 * 或0以保持MVE支持禁用。此选项仅适用于Cortex-M55和Cortex-M85端口，
 * 因为M-Profile向量扩展(MVE)仅在这些架构上可用。
 * 对于Cortex-M23、Cortex-M33和Cortex-M35P端口，configENABLE_MVE必须保持未定义或定义为0。 */
#define configENABLE_MVE                                               1

/******************************************************************************/
/* ARMv7-M和ARMv8-M端口特定配置定义 *****************************************/
/******************************************************************************/

/* 将configCHECK_HANDLER_INSTALLATION设置为1以启用额外的断言来验证
 *
 * 应用程序可以通过以下方式之一安装FreeRTOS中断处理程序：
 *   1. 直接路由 - 分别为SVC调用和PendSV中断安装vPortSVCHandler和xPortPendSVHandler函数
 *   2. 间接路由 - 为SVC调用和PendSV中断安装单独的处理程序，并从这些处理程序
 *      将程序控制路由到vPortSVCHandler和xPortPendSVHandler函数
 * 使用间接路由的应用程序必须将configCHECK_HANDLER_INSTALLATION设置为0。
 *
 * 如果未定义则默认为1。 */
#define configCHECK_HANDLER_INSTALLATION                               1





/******************************************************************************/
/* FreeRTOS-Plus 扩展组件定义 **************************************************/
/******************************************************************************/

/* FreeRTOS_Plus_Cli 组件定义：输出字符串的最大长度 */

#define configCOMMAND_INT_MAX_OUTPUT_SIZE                              512



/* FreeRTOS 系统软件定时器任务名定义, 未定义则为 "Tmr Svc" */

#define configTIMER_SERVICE_TASK_NAME                                 "FTmr Svc"


/******************************************************************************/
/* 功能包含或排除定义 ********************************************************/
/******************************************************************************/

/* 将以下configUSE_*常量设置为1在构建中包含命名功能，
 * 或0从构建中排除命名功能。 */
#define configUSE_TASK_NOTIFICATIONS                                   1
#define configUSE_MUTEXES                                              1
#define configUSE_RECURSIVE_MUTEXES                                    1
#define configUSE_COUNTING_SEMAPHORES                                  1
#define configUSE_QUEUE_SETS                                           1

/* 使能此配置将添加额外的结构体成员和函数,以此来协助可视化和跟踪,在使用IAR中的FreeRTOS插件时要使能这个配置,否则无法显示任务栈的使用情况 */
#define configUSE_APPLICATION_TASK_TAG                                 1

/* 将以下INCLUDE_*常量设置为1包含命名API函数，
 * 或0排除命名API函数。大多数链接器即使在常量为1时也会移除未使用的函数。 */
#define INCLUDE_vTaskPrioritySet                                       1
#define INCLUDE_uxTaskPriorityGet                                      1
#define INCLUDE_vTaskDelete                                            1
#define INCLUDE_vTaskSuspend                                           1
#define INCLUDE_xResumeFromISR                                         1
#define INCLUDE_vTaskDelayUntil                                        1
#define INCLUDE_vTaskDelay                                             1
#define INCLUDE_xTaskGetSchedulerState                                 1
#define INCLUDE_xTaskGetCurrentTaskHandle                              1
#define INCLUDE_uxTaskGetStackHighWaterMark                            1
#define INCLUDE_xTaskGetIdleTaskHandle                                 1
#define INCLUDE_eTaskGetState                                          1
#define INCLUDE_xEventGroupSetBitFromISR                               1
#define INCLUDE_xTimerPendFunctionCall                                 1
#define INCLUDE_xTaskAbortDelay                                        1
#define INCLUDE_xTaskGetHandle                                         1
#define INCLUDE_xTaskResumeFromISR                                     1

#define xPortPendSVHandler                                             PendSV_Handler
#define vPortSVCHandler                                                SVC_Handler
#define xPortSysTickHandler                                            SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
