#ifndef TBOX_LOG_INNER_H
#define TBOX_LOG_INNER_H

#include "tbox_log.h"

#ifdef __cplusplus
extern "C" {
#endif

INT32 tbox_log_init(VOID);
VOID tbox_log_deinit(VOID);
INT32 tbox_log_start(VOID);
VOID tbox_log_stop(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_LOG_INNER_H */