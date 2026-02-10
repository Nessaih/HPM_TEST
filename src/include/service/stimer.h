#ifndef  STIMER_H
#define  STIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tbox_type.h"

#define STIMER_ID_INVALID -1
typedef INT32 STIMER_ID;

typedef VOID (*STIMER_CALLBACK)(VOID);

typedef enum 
{
    STIMER_TYPE_ONCE    = 0U,
    STIMER_TYPE_PERIOD,
} STIMER_TYPE;

/*休眠唤醒时，定时器需要优先启动和最后停止*/
VOID tbox_stimer_start(VOID);
VOID tbox_stimer_stop(VOID);
STIMER_ID stimer_create(STIMER_TYPE type, STIMER_CALLBACK func);
INT32 stimer_start(STIMER_ID id, UINT32 timeout_ms);
INT32 stimer_stop(STIMER_ID id);
INT32 stimer_trigger(STIMER_ID id);
INT32 stimer_clear(STIMER_ID id);
BOOL stimer_is_started(STIMER_ID id);

#ifdef __cplusplus
}
#endif

#endif //STIMER_H