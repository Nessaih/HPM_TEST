#ifndef __FOTA_COM_H__
#define __FOTA_COM_H__

INT32 fota_com_init(UINT8 seq);

VOID  fota_com_timeout_proc(VOID);

VOID fota_com_info_dump(VOID);

VOID fota_com_reset(VOID);

INT32 fota_com_get_state(VOID);

#endif
