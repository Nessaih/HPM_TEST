#ifndef __SE_MGR_H__
#define __SE_MGR_H__

INT32 se_mgr_init(UINT8 seq);

VOID se_mgr_wake(VOID);

VOID se_mgr_sleep(VOID);

INT32 se_mgr_deinit(VOID);

VOID se_mgr_timeout(VOID);

#endif

