#ifndef __LED_TBOX_H__
#define __LED_TBOX_H__

/**
 * @brief LED设计逻辑
 *
 * 1. 初始化时，需要执行跑马灯操作
 *   1.1 全灭
 *   1.2 逐个点亮（RUN -> CAN -> GNSS -> LTE）
 *   1.3 全灭
 *   1.4 逐个点亮（RUN -> CAN -> GNSS -> LTE）
 *   1.5 全灭
 *
 * 2. 进入休眠状态时，需要执行全灭操作
 *
 * 3. 运行模式灯的切换逻辑
 *  3.1 RUN灯 : 运行时，RUN灯闪烁（1s闪烁一次）
 *  3.2 GNSS灯:
 *   3.2.1 模组故障或天线短路，GNSS灯常亮,当前模组故障未实现
 *         开路当前未添加，开路有误报的情况待确认
 *   3.2.2 未定位时，熄灭
 *   3.2.3 定位时，闪烁（1s闪烁一次）
 *  3.3 LTE灯:
 *   3.3.1 未企标平台连接时，熄灭
 *   3.3.2 已企标平台连接时，闪烁（1s闪烁一次）
 *  3.4 CAN灯:
 *   3.4.1 CANBUS状态为BUSY时，闪烁（1s闪烁一次）
 *   3.4.2 CANBUS状态为IDLE时，熄灭
 */

INT32 led_tbox_init(VOID);

VOID led_tbox_sleep(VOID);

VOID led_tbox_wake(VOID);

VOID led_tbox_timeout(VOID);

#endif //__LED_TBOX_H__
