#ifndef TBOX_CONFIG_H
#define TBOX_CONFIG_H

#define TBOX_CPU_IS_LITTLE_ENDIAN

/* Memory pool configuration */
#define TBOX_MEMPOOL_NUM                     (5U)
/* 内存池大小配置，这里可以只配置一个内存池，也可以配置多个内存池，每个内存池大小不同。
 * MCU可以配置一个CORE内存池，MPU可以配置CORE、SERVICE、APP三个内存池。根据实际情况配置
 * 每个内存池大小，单位为字节。
*/
#define TBOX_MEMORY_CORE_SIZE                (0x5000U) //20KB
//#define TBOX_MEMORY_SERVICE_SIZE             (0U) 
//#define TBOX_MEMORY_APP_SIZE                 (0U)

/* Message configuration */
#define TBOX_MESSAGE_DATA_MAX_LEN            (256U)
/*配置消息注册表可以存储消息的个数，最多支持{TBOX_MESSAGE_MAX_NUM}个消息，如果系统比较复杂可以适当增加*/
#define TBOX_MESSAGE_MAX_NUM                 (64U)
/*配置消息列表存储消息的个数，高优先级消息列表最多支持{TBOX_HIGH_MESSAGE_MAX_NUM}个消息，
 *中优先级消息列表最多支持{TBOX_NORMAL_MESSAGE_MAX_NUM}个消息，低优先级消息列表最多支持{TBOX_LOW_MESSAGE_MAX_NUM}个消息，
 *如果系统比较复杂可以适当增加*/
#define TBOX_HIGH_MESSAGE_MAX_NUM            (8U)
#define TBOX_NORMAL_MESSAGE_MAX_NUM          (24U)
#define TBOX_LOW_MESSAGE_MAX_NUM             (4U)
#define TBOX_MESSAGE_MAX_HANDLE_TIME         (5000U) //5s

/*任务优先级配置，优先级分为高、中、低三个等级，值越大优先级越高，任务调度时优先调度优先级高的任务
 *根据系统{configMAX_PRIORITIES}配置优先级，{configMAX_PRIORITIES}默认为20，如果{configMAX_PRIORITIES}
 *变更，下面配置也需要跟着变更。
 */
#define TBOX_TASK_PRIORITY_HIGH              (18U)
#define TBOX_TASK_PRIORITY_MID3              (17U)
#define TBOX_TASK_PRIORITY_MID2              (16U)
#define TBOX_TASK_PRIORITY_MID1              (13U)
#define TBOX_TASK_PRIORITY_MID               (10U)
#define TBOX_TASK_PRIORITY_LOW2              (8U)
#define TBOX_TASK_PRIORITY_LOW1              (6U)
#define TBOX_TASK_PRIORITY_LOW               (5U)
/*任务堆栈配置，这里配置了三个任务堆栈大小，分别为大、中、小适应不同任务的需求，如果系统比较复杂可以适当增加。
 *注意，如果任务里面函数调用层级比较深、调用系统函数或者局部变量过多，需要适当增加任务堆栈大小。
 *如果函数里需要使用大容量局部变量，可以使用tbox_memory_alloc分配内存，这样可以避免堆栈溢出。
*/
#define TBOX_TASK_LARGE_STACK_SIZE           (3072U) // 128*24， 适用复杂任务，比如企标业务、国标业务
#define TBOX_TASK_MEDIUM_STACK_SIZE_1        (2048U) // 128*16
#define TBOX_TASK_MEDIUM_STACK_SIZE          (1536U) // 128*12， 适用中等复杂任务，服务处理
#define TBOX_TASK_SMALL_STACK_SIZE_1         (1024U) // 128*8
#define TBOX_TASK_SMALL_STACK_SIZE           (896U)  // 128*7， 适用驱动回调，服务处理或者业务简单任务
#define TBOX_TASK_LARGER_SIZE_NUM            (1U)
#define TBOX_TASK_MEDIUM_SIZE_NUM            (6U)
#define TBOX_TASK_SMALL_SIZE_NUM             (1U)
#define TBOX_RUNLOOP_TASK_NUM                (16U)
#define TBOX_TASK_HIGHWATER_THRESHOLD        (8U)  //32B
#define TBOX_TASK_MSG_QUEUE_SIZE             (512U)  //每个任务消息队列的长度，单位为字节

/*配置系统最大模块注册数量，如果系统比较复杂可以适当增加*/
#define TBOX_MODULE_MAX_NUM                  (32U)

/*日志一次最大写入大小和最大条目数量，日志模块占有最大内存为：{TBOX_LOG_LINEBUFF_SIZE}*{TBOX_LOG_ITEM_MAX_NUM}*/
#define TBOX_LOG_LINEBUFF_SIZE               (256U)
#define TBOX_LOG_ITEM_MAX_NUM                (12U)

/*STIMER配置*/
#define TBOX_STIMER_NUMBER                   (24U)
#define TBOX_STIMER_WHEEL_SLOTE_NUM          (32U)

#define TBOX_CFG_ITEM_NUMBER                 (48U)
#define TBOX_CFG_VALUE_MAX_LEN               (128U)

#endif /* TBOX_CONFIG_H */