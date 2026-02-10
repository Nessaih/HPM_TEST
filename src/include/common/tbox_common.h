#ifndef TBOX_COMMON_H
#define TBOX_COMMON_H

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Device_Register.h"
#include "api_drv.h"
#include "api_rtos.h"
#include "tbox_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TBOX_BUILD_TYPE_RELEASE       0U
#define TBOX_BUILD_TYPE_DEBUG         1U
#define TBOX_BUILD_TYPE_UNIT_TEST     2U
#if defined(TEST_DEDBU)
#define TBOX_BUILD_TYPE TBOX_BUILD_TYPE_DEBUG
#elif defined(UNIT_TEST)
#define TBOX_BUILD_TYPE TBOX_BUILD_TYPE_UNIT_TEST
#else
#define TBOX_BUILD_TYPE TBOX_BUILD_TYPE_RELEASE
#endif

#define GET_BYTE(data)      (*(data))
#define GET_HWBE(data)      (GET_BYTE(data) * 0x100 + GET_BYTE((data) + 1))
#define GET_WDBE(data)      (GET_HWBE(data) * 0x10000 + GET_HWBE((data) + 2))
#define GET_WORD(data)      GET_HWBE(data)
#define GET_DWORD(data)     GET_WDBE(data)

#define STR_TO_HEX(in, in_len, hex, ret)({									\
		int idx = 0;															\
		char h,l;																\
		char *p = in;															\
		if(0 != (in_len%2)) 													\
		{																		\
			ret = -1;															\
		}																		\
		for(idx = 0; idx < (in_len/2); idx++)									\
		{																		\
			h = ((*p > '9') && ((*p <= 'F') || (*p <= 'f')))?*p-48-7:*p-48; 	\
			p++;																\
			l = (*(p) > '9' && ((*p <= 'F') || (*p <= 'f')))?*p-48-7:*p-48; 	\
			p++;																\
			hex[idx] = (h&0x0F) << 4 | (l&0x0F);								\
		}																		\
		ret = 0;																\
		})
		
#define TBOX_OFFSET(type, member)           ((UINT32)&((type *)0)->member)
#define TBOX_CONTAINER(ptr, type, member)   ((type *)((UINT32)(ptr) - TBOX_OFFSET(type, member)))
#define TBOX_MAX(a, b)                      ((a) > (b)? (a) : (b))
#define TBOX_MIN(a, b)                      ((a) < (b)? (a) : (b))
#define TBOX_ABS(a)                         ((a) >= 0 ? (a) : (-(a)))
#define TBOX_ARRAY_SIZE(arr)                (sizeof(arr) / sizeof((arr)[0]))
#define TBOX_COND_CALL(COND_VAR, COND, FUNC) (((COND) == (COND_VAR)) ? (FUNC) : (COND_VAR))

#define DISABLE_INTERRUPT __asm volatile("cpsid i" : : : "memory")
#define ENABLE_INTERRUPT  __asm volatile("cpsie i" : : : "memory")
#define WDG_RESET         drv_wdg_reset()
#define SOFTWARE_RESET    NVIC_SystemReset()
#define SYSTEM_RESET      SOFTWARE_RESET

#define offset_of(data,member)      (unsigned int)(&(((data*)0)->member))

#define container_of(ptr, type, member) ((type *)( (unsigned char *)ptr - offset_of(type,member) ))

#ifdef __cplusplus
}
#endif

#endif /* TBOX_COMMON_H */