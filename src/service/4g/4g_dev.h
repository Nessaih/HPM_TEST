#ifndef TBOX_4G_DEV_H
#define TBOX_4G_DEV_H

typedef VOID (*DEV_4G_SEND_CALLBACK)(VOID);

INT32 dev_4g_open(VOID);

INT32 dev_4g_close(VOID);

BOOL dev_4g_is_opened(VOID);

VOID dev_4g_check_send(VOID);

VOID dev_4g_direct_send(UINT8 *data, UINT16 len, DEV_4G_SEND_CALLBACK callback);

// VOID dev_4g_direct_send_blocking(UINT8 *data, UINT16 len);

#endif /*TBOX_4G_DEV_H*/
