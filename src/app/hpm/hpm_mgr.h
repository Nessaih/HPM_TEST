#ifndef __HPM_MGR_H__
#define __HPM_MGR_H__

INT32 hpm_mgr_init(UINT8 seq);

VOID hpm_mgr_process(VOID);

VOID hpm_mgr_wakeup(VOID);

BOOL hpm_mgr_allow_sleep(VOID);

#endif
