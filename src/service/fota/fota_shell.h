#ifndef __FOTA_SHELL_H__
#define __FOTA_SHELL_H__

INT32 fota_shell_init(UINT8 seq);

VOID fota_shell_value_changed_handle(TBOX_MSG_DATA *data);

#endif
