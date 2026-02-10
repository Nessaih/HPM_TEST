#include <string.h>
#include "tbox_config.h"
#include "tbox_common.h"
#include "tbox_mp.h"
#include "tbox_supervish_inner.h"
#include "tbox_memory_inner.h"
#include "tbox_memory.h"
#include "tbox_log_inner.h"

static MP_HANDLE mps[TBOX_MEMORY_TYPE_MAX];
#if defined (TBOX_MEMORY_CORE_SIZE)
static UINT8 tbox_core_memory[TBOX_MEMORY_CORE_SIZE];
#endif

#if defined(TBOX_MEMORY_SERVICE_SIZE)
static UINT8 tbox_service_memory[TBOX_MEMORY_SERVICE_SIZE];
#endif

#if defined(TBOX_MEMORY_APP_SIZE)
static UINT8 tbox_app_memory[TBOX_MEMORY_APP_SIZE];
#endif

INT32 tbox_memory_init(VOID)
{
    MP_HANDLE mp = MP_INVALID_HANDLE;
    
    for(UINT8 i = 0; i < TBOX_MEMORY_TYPE_MAX; i++)
    {
        mps[i] = MP_INVALID_HANDLE;
    }
    
    #if defined(TBOX_MEMORY_CORE_SIZE)
    mp = mp_create(tbox_core_memory, TBOX_MEMORY_CORE_SIZE);
    if(MP_INVALID_HANDLE == mp)
    {
        return (INT32)TBOX_E_FAILED_CREATE;
    }
    mps[TBOX_MEMORY_TYPE_CORE] = mp;
    #endif

    #if defined(TBOX_MEMORY_SERVICE_SIZE)
    mp = mp_create(tbox_service_memory, TBOX_MEMORY_SERVICE_SIZE);
    if(MP_INVALID_HANDLE == mp)
    {
        return (INT32)TBOX_E_FAILED_CREATE;
    }
    mps[TBOX_MEMORY_TYPE_SERVICE] = mp;
    #endif

    #if defined(TBOX_MEMORY_APP_SIZE)
    mp = mp_create(tbox_app_memory, TBOX_MEMORY_APP_SIZE);
    if(MP_INVALID_HANDLE == mp)
    {
        return (INT32)TBOX_E_FAILED_CREATE;
    }
    mps[TBOX_MEMORY_TYPE_APP] = mp;
    #endif
    
    return (INT32)TBOX_E_OK;
}

VOID tbox_memory_deinit(VOID)
{
    #if defined(TBOX_MEMORY_CORE_SIZE)
    if(MP_INVALID_HANDLE != mps[TBOX_MEMORY_TYPE_CORE])
    {
        mp_destroy(mps[TBOX_MEMORY_TYPE_CORE]);
        mps[TBOX_MEMORY_TYPE_CORE] = MP_INVALID_HANDLE;
    }
    #endif

    #if defined(TBOX_MEMORY_SERVICE_SIZE)
    if(MP_INVALID_HANDLE != mps[TBOX_MEMORY_TYPE_SERVICE])
    {
        mp_destroy(mps[TBOX_MEMORY_TYPE_SERVICE]);
        mps[TBOX_MEMORY_TYPE_SERVICE] = MP_INVALID_HANDLE;
    }
    #endif

    #if defined(TBOX_MEMORY_APP_SIZE)
    if(MP_INVALID_HANDLE != mps[TBOX_MEMORY_TYPE_APP])
    {
        mp_destroy(mps[TBOX_MEMORY_TYPE_APP]);
        mps[TBOX_MEMORY_TYPE_APP] = MP_INVALID_HANDLE;
    }
    #endif

    MODULE_LOG_D(ICORE, "deinit memory pool finish");    
}

VOID *tbox_memory_alloc(TBOX_MEMORY_TYPE type, UINT32 size)
{
    if(type >= TBOX_MEMORY_TYPE_MAX)
    {
        MODULE_LOG_E(ICORE, "type is invalid, type:%d", type);
        return NULL;
    }
    if(MP_INVALID_HANDLE == mps[type])
    {
        MODULE_LOG_E(ICORE, "handle is invalid, handle:%d", mps[type]);
        return NULL;
    }
    return mp_alloc(mps[type], size);
}

VOID tbox_memory_free(TBOX_MEMORY_TYPE type, VOID *ptr)
{
    if(type >= TBOX_MEMORY_TYPE_MAX || NULL_PTR == ptr)
    {
        MODULE_LOG_E(ICORE, "type is invalid, type:%d", type);
        return;
    }
    if(MP_INVALID_HANDLE == mps[type])
    {
        MODULE_LOG_E(ICORE, "type is invalid, type:%d", type);
        return;
    }
    mp_free(mps[type], ptr);
}

INT32 tbox_memory_check(TBOX_MEMORY_TYPE type, VOID *ptr)
{
    if(type >= TBOX_MEMORY_TYPE_MAX || NULL_PTR == ptr)
    {
        MODULE_LOG_E(ICORE, "type is invalid, type:%d", type);
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(MP_INVALID_HANDLE == mps[type])
    {
        MODULE_LOG_E(ICORE, "type is invalid, type:%d", type);
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    
    TBOX_CORE_MEMORY_ABNORMAL_INFO info = {.addr = ptr};
    TBOX_ERROR_CODE ret = mp_check(mps[type], ptr);
    if(TBOX_E_OVERFLOW == ret)
    {
        MODULE_LOG_W(ICORE, "memory overflow, type:%d, addr:%p", type, ptr);
        tbox_supervish_report_memory_abnormal(MODULE_ABNORMAL_MEMORY_OVERFLOW,
                                              &info);
        return (INT32)TBOX_E_OVERFLOW;
    }
    else if(TBOX_E_UNDERFLOW == ret)
    {
        MODULE_LOG_W(ICORE, "memory underflow, type:%d, addr:%p", type, ptr);
        tbox_supervish_report_memory_abnormal(MODULE_ABNORMAL_MEMORY_UNDERFLOW,
                                              &info);
        return (INT32)TBOX_E_UNDERFLOW;                                       
    }
    else if(TBOX_E_OK != ret)
    {
        MODULE_LOG_W(ICORE, "memory failed, type:%d, addr:%p", type, ptr);
        return (INT32)TBOX_E_FAILED; 
    }
    else
    {
        /*TODO do noting*/
    }

    return (INT32)TBOX_E_OK; 
}
