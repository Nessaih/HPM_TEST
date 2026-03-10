#ifndef __HPM_PARAM_FECTH_H__
#define __HPM_PARAM_FECTH_H__

#include "hpm_param_parse.h"

INT32 hpm_param_fetch_init(UINT8 seq);
VOID hpm_param_fetch_deinit(VOID);
VOID hpm_param_fetch_process(VOID);
VOID hpm_param_fetch_wakeup(VOID);
VOID hpm_param_fetch_sleep(VOID);

INT32 hpm_param_fetch_ftp_start(UINT8 *data, UINT16 len, UINT8 *resp, UINT16 *resp_len);

INT32 hpm_param_fetch_report(UINT8 *data, INT32 remain_size);

INT32 hpm_param_register(const char *param, INT32 len);

INT32 hpm_param_unregister(const char *param, INT32 len);

VOID hpm_param_show_cfg(VOID);

#endif //__HPM_PARAM_FECTH_H__
