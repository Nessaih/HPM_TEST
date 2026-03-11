#pragma once

#if UDS_SYS == UDS_SYS_CUSTOM

/* 覆盖开源库的默认配置 */
#include "../uds_config.h"

/* tbox 平台的系统头文件 */
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* IAR 编译器不支持 ssize_t，定义为 int32_t */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef int32_t ssize_t;
#endif

/* 确保使用 isotp-c 传输层 */
#ifndef UDS_TP_ISOTP_C
#define UDS_TP_ISOTP_C
#endif

/* 声明自定义的时间戳函数（在 uds_adapter.c 中实现） */
#if UDS_CUSTOM_MILLIS
uint32_t UDSMillis(void);
#endif

/**
 * 自定义时间戳函数
 * 说明：使用 tbox 平台的时间戳函数
 */
#define UDS_CUSTOM_MILLIS 1

#endif
