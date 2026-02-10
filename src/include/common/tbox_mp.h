#ifndef TBOX_MP_H
#define TBOX_MP_H

#include "tbox_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef INT32 MP_HANDLE;
#define MP_INVALID_HANDLE ((MP_HANDLE)(-1))

MP_HANDLE mp_create(UINT8 *buffer, UINT32 size);
VOID mp_destroy(MP_HANDLE handle);
VOID* mp_alloc(MP_HANDLE handle, UINT32 size);
VOID mp_free(MP_HANDLE handle, VOID *ptr);
TBOX_ERROR_CODE mp_check(MP_HANDLE handle, VOID *ptr);
TBOX_ERROR_CODE mp_check_pool(MP_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_MP_H */