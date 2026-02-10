#ifndef __RTOS_INCLUDE_H__
#define __RTOS_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif

// #ifndef __CSTAT__

/* code here is not visible to the analysis tool */

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

// #endif //__CSTAT__

#ifdef __cplusplus
}
#endif

#endif //__RTOS_INCLUDE_H__