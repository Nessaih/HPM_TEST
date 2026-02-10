#ifndef TBOX_SUPERVISH_INNER_H_
#define TBOX_SUPERVISH_INNER_H_

#include "tbox_supervise.h"

#ifdef __cplusplus
extern "C" {
#endif

INT32 tbox_supervise_init(VOID);
VOID tbox_supervise_deinit(VOID);
INT32 tbox_supervise_start(VOID);
VOID tbox_supervise_stop(VOID);
VOID tbox_supervish_report_memory_abnormal(TBOX_MODULE_ABNORMAL_TYPE type, 
                                            const TBOX_CORE_MEMORY_ABNORMAL_INFO *info);
VOID tbox_supervish_report_task_abnormal(TBOX_MODULE_ABNORMAL_TYPE type, 
                                        BOOL is_priavte,
                                        const TBOX_CORE_TASK_ABNORMAL_INFO *info);
#ifdef __cpluscplus
}
#endif

#endif /* TBOX_SUPERVISH_INNER_H_ */