#ifndef __HPM_OTA_TBOX_H__
#define __HPM_OTA_TBOX_H__

#include "hpm_control.h"

INT32 hpm_ota_tbox_init(UINT8 seq);

VOID hpm_ota_tbox_wake(VOID);

VOID hpm_ota_tbox_sleep(VOID);

INT32 hpm_ota_tbox_handle(HPM_CTRL_FOTA_INFO_T fota_info);

VOID hpm_ota_tbox_handle_msg(TBOX_MSG_DATA *data);

BOOL hpm_ota_tbox_upgrading(VOID);

VOID hpm_ota_tbox_process(VOID);

VOID hpm_ota_tbox_info_dump(VOID);

#endif
