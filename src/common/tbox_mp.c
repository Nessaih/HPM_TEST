#include <string.h>

#include "tbox_config.h"
#include "tbox_common.h"
#include "api_rtos.h"
#include "tbox_dlist.h"
#include "tbox_mp.h"

#define MEM_MAGIC               0x51AB51ABU
#define MEM_CHUNK_MAGIC         0x38FCU
#define MEM_ZONE_COUNT          72U
#define MEM_ALIGN_SIZE          4U
#define MEM_ALIGN(size)         (((UINT32)(size) + (MEM_ALIGN_SIZE)-1) & ~((MEM_ALIGN_SIZE)-1))
#define MEM_ALIGN_DOWN(size)    ((UINT32)(size)&~((MEM_ALIGN_SIZE)-1))
#define MEM_CHUNK_SIZE(size)    (MEM_ALIGN((UINT32)sizeof(MEM_CHUNK) + (UINT32)(size) + 2*MEM_GUARD_SIZE))
#define MEM_GUARD_PATTERN       (0xDEADBEEFUL)
#define MEM_GUARD_SIZE          ((UINT32)sizeof(UINT32))
#define MEM_GET_POOL(MP)        (&mem_pool[(UINT32)MP])

typedef struct __attribute__((aligned(1))) tag_mem_chunk
{
    UINT32 magic:15;
    UINT32 used:1;    
    UINT32 size:16;
    DLIST_NODE link;
}MEM_CHUNK;

typedef struct __attribute__((aligned(1))) tag_mem_zone
{
    UINT32 magic;
    UINT16 chunk_size;
    DLIST_NODE free_list;
}MEM_ZONE;

typedef struct tag_mem_pool
{
    BOOL is_used;
    UINT8 *buffer;
    UINT32 size;
    UINT32 pos;
    MEM_ZONE *zones;
    SemaphoreHandle_t mutex;
}MEM_POOL, *MP_POOL_PTR;

static MEM_POOL mem_pool[TBOX_MEMPOOL_NUM] = 
{
    {FALSE, NULL_PTR, 0U, 0U, NULL_PTR, NULL},
    {FALSE, NULL_PTR, 0U, 0U, NULL_PTR, NULL},
    {FALSE, NULL_PTR, 0U, 0U, NULL_PTR, NULL},
    {FALSE, NULL_PTR, 0U, 0U, NULL_PTR, NULL},
    {FALSE, NULL_PTR, 0U, 0U, NULL_PTR, NULL}
};

static UINT16 mem_chunk_size[MEM_ZONE_COUNT] = 
{
    8U, 16U, 24U, 32U, 40U, 48U, 56U, 64U, 72U, 80U, 88U, 96U, 104U, 112U, 120U, 128U, 
    144U, 160U, 176U, 192U, 208U, 224U, 240U, 256U, 288U, 320U, 352U, 384U, 416U, 448U, 480U, 512U, 
    576U, 640U, 704U, 768U, 832U, 896U, 960U, 1024U, 1152U, 1280U, 1408U, 1536U, 1664U, 1792U, 1920U, 2048U, 
    2304U, 2560U, 2816U, 3072U, 3328U, 3584U, 3840U, 4096U, 4608U, 5120U, 5632U, 6144U, 6656U, 7168U, 7680U, 8192U, 
    9216U, 10240U, 11264U, 12288U, 13312U, 14336U, 15360U, 16384U
};

static inline VOID *mem_alloc_internal(MP_HANDLE handle, UINT32 size)
{
    MP_POOL_PTR pool = MEM_GET_POOL(handle);
    if(pool->pos+size > pool->size)
    {
        return NULL;
    }
    VOID *ptr = pool->buffer + pool->pos;    
    pool->pos += size;
    return ptr;
}

static inline UINT32 mem_zoneindex(UINT32 *size)
{
    UINT32 n = (UINT32)(*size);

    if (n < 128U)
    {
        *size = n = (n + 7U) & ~7U;

        return (n / 8U - 1U);
    }
    if (n < 256U)
    {
        *size = n = (n + 15U) & ~15U;

        return (n / 16U + 7U);
    }
    if (n < 8192U)
    {
        if (n < 512U)
        {
            *size = n = (n + 31U) & ~31U;

            return (n / 32U + 15U);
        }
        if (n < 1024U)
        {
            *size = n = (n + 63U) & ~63U;

            return (n / 64U + 23U);
        }
        if (n < 2048U)
        {
            *size = n = (n + 127U) & ~127U;

            return (n / 128U + 31U);
        }
        if (n < 4096U)
        {
            *size = n = (n + 255U) & ~255U;

            return (n / 256U + 39U);
        }
        *size = n = (n + 511U) & ~511U;

        return (n / 512U + 47U);
    }
    if (n < 16384U)
    {
        *size = n = (n + 1023U) & ~1023U;

        return (n / 1024U + 55U);
    }

    return MEM_ZONE_COUNT;
}

MP_HANDLE mp_create(UINT8 *buffer, UINT32 size)
{
    if (NULL == buffer)
    {
        return (MP_HANDLE)TBOX_E_INVALID_PARAM;
    }
    if(size <= sizeof(MEM_ZONE)*MEM_ZONE_COUNT)
    {
        return (MP_HANDLE)TBOX_E_TOOSMALL;
    }

    MP_HANDLE handle = (MP_HANDLE)-1;
    MP_POOL_PTR pool = NULL;
    for (UINT32 i = 0; i < TBOX_MEMPOOL_NUM; i++)
    {
        if (FALSE == mem_pool[i].is_used)
        {
            handle = (MP_HANDLE)i;
            pool = &mem_pool[i];
            break;
        }
    }
    if(handle < 0)
    {
        return (MP_HANDLE)TBOX_E_NORESOURCES;
    }

    memset(buffer, 0, size);
    pool->buffer = (UINT8 *)MEM_ALIGN(buffer);
    pool->size = MEM_ALIGN_DOWN((UINT32)buffer + size - (UINT32)pool->buffer);
    pool->zones = (MEM_ZONE *)(pool->buffer);
    pool->pos = sizeof(MEM_ZONE)*MEM_ZONE_COUNT;    
    for (UINT32 i = 0; i < MEM_ZONE_COUNT; i++)
    {
        pool->zones[i].magic = MEM_MAGIC;
        pool->zones[i].chunk_size = mem_chunk_size[i];
        dlist_init(&pool->zones[i].free_list);
    }
    pool->mutex = xSemaphoreCreateMutex();
    if(NULL == pool->mutex)
    {
        return (MP_HANDLE)TBOX_E_FAILED_CREATE;
    }
    pool->is_used = TRUE;

    return handle;    
}

VOID mp_destroy(MP_HANDLE handle)
{
    if(handle >= TBOX_MEMPOOL_NUM || handle < 0)
    {
        return;
    }

    MP_POOL_PTR pool = MEM_GET_POOL(handle);
    xSemaphoreTake(pool->mutex, portMAX_DELAY);
    {
        if(FALSE == pool->is_used)
        {
            xSemaphoreGive(pool->mutex);
            return;
        }
        pool->buffer = NULL;
        pool->size = 0U;
        pool->pos = 0U;
        pool->zones = NULL;    
        pool->is_used = FALSE;   
    }
    xSemaphoreGive(pool->mutex);

    vSemaphoreDelete(pool->mutex);
    pool->mutex = NULL;
}

VOID* mp_alloc(MP_HANDLE handle, UINT32 size)
{
    if(handle >= TBOX_MEMPOOL_NUM || handle < 0)
    {
        return NULL;
    }
    if(0U == size)
    {
        return NULL;
    }

    UINT32 data_size = MEM_CHUNK_SIZE(size);
    UINT32 zone_index = mem_zoneindex(&data_size);
    if(zone_index >= MEM_ZONE_COUNT)
    {
        return NULL;
    }

    MP_POOL_PTR pool = MEM_GET_POOL(handle);
    if(FALSE == pool->is_used || 
       NULL == pool->mutex || 
       NULL_PTR == pool->buffer || 
       0U == pool->size)
    {
        return NULL;
    }

    UINT8 *ptr = NULL;
    xSemaphoreTake(pool->mutex, portMAX_DELAY);
    {
        MEM_ZONE *zone = &pool->zones[zone_index];
        if(MEM_MAGIC != zone->magic || data_size > zone->chunk_size)
        {
            xSemaphoreGive(pool->mutex);
            return NULL;   
        }
        if(TRUE == dlist_empty(&zone->free_list))
        {
            ptr = (UINT8 *)mem_alloc_internal(handle, data_size);
            if(NULL == ptr)
            {
                xSemaphoreGive(pool->mutex);
                return NULL;
            }
            MEM_CHUNK *chunk = (MEM_CHUNK *)ptr;
            ptr += sizeof(MEM_CHUNK);
            chunk->magic = MEM_CHUNK_MAGIC;
            chunk->used = 1U;
            chunk->size = size;
            *(UINT32 *)ptr = MEM_GUARD_PATTERN;
            ptr += MEM_GUARD_SIZE;
            *(UINT32 *)((UINT32)ptr+size) = MEM_GUARD_PATTERN;
        }
        else
        {
            DLIST_NODE *node = dlist_pop_first_node(&zone->free_list);
            if(NULL == node)
            {
                xSemaphoreGive(pool->mutex);
                return NULL;
            }
            MEM_CHUNK *chunk = TBOX_CONTAINER(node, MEM_CHUNK, link);
            chunk->magic = MEM_CHUNK_MAGIC;
            chunk->used = 1U;
            chunk->size = size;
            ptr = (UINT8 *)chunk + sizeof(MEM_CHUNK);
            *(UINT32 *)ptr = MEM_GUARD_PATTERN;
            ptr += MEM_GUARD_SIZE;
            *(UINT32 *)((UINT32)ptr+size) = MEM_GUARD_PATTERN;
        }       
    }
    xSemaphoreGive(pool->mutex);

    memset(ptr, 0, size);
    return (VOID *)ptr;  
}

VOID mp_free(MP_HANDLE handle, VOID *ptr)
{
    if(handle >= TBOX_MEMPOOL_NUM || handle < 0)
    {
        return;
    }
    if(NULL == ptr)
    {
        return;
    }

    MP_POOL_PTR pool = MEM_GET_POOL(handle);
    if(FALSE == pool->is_used || 
       NULL == pool->mutex || 
       NULL_PTR == pool->buffer || 
       0U == pool->size)
    {
        return;
    }

    xSemaphoreTake(pool->mutex, portMAX_DELAY);
    {
        if((UINT32)ptr < (UINT32)pool->buffer || (UINT32)ptr >= ((UINT32)pool->buffer + pool->size))
        {
            xSemaphoreGive(pool->mutex); 
            return;
        }

        MEM_CHUNK *chunk = (MEM_CHUNK *)((UINT32)ptr - sizeof(MEM_CHUNK) - MEM_GUARD_SIZE);
        if(0 == chunk->used || MEM_CHUNK_MAGIC != chunk->magic)
        {
            xSemaphoreGive(pool->mutex);
            return;
        }
        UINT32 chunk_size = MEM_CHUNK_SIZE(chunk->size);
        UINT32 zone_index = mem_zoneindex(&chunk_size);
        if(zone_index >= MEM_ZONE_COUNT)
        {
            xSemaphoreGive(pool->mutex);
            return;
        }
        MEM_ZONE *zone = &pool->zones[zone_index];
        if(MEM_MAGIC != zone->magic || 
        chunk->size > zone->chunk_size || 
        NULL == zone->free_list.next ||
        NULL == zone->free_list.prev)
        {
            xSemaphoreGive(pool->mutex);
            return;
        }

        chunk->used = 0U;
        chunk->size = 0U;
        dlist_add_tail(&chunk->link, &zone->free_list);
    }
    xSemaphoreGive(pool->mutex);
}

TBOX_ERROR_CODE mp_check(MP_HANDLE handle, VOID *ptr)
{
    if(handle >= TBOX_MEMPOOL_NUM || handle < 0)
    {
        return TBOX_E_INVALID_PARAM;
    }
    if(NULL == ptr)
    {
        return TBOX_E_INVALID_PARAM;
    }

    MP_POOL_PTR pool = MEM_GET_POOL(handle);
    if(FALSE == pool->is_used || 
       NULL == pool->mutex || 
       NULL_PTR == pool->buffer || 
       0U == pool->size)
    {
        return TBOX_E_INVALID_DATA;
    }

    xSemaphoreTake(pool->mutex, portMAX_DELAY);
    {
        if((UINT32)ptr < (UINT32)pool->buffer || (UINT32)ptr >= ((UINT32)pool->buffer + pool->size))
        {
            xSemaphoreGive(pool->mutex);
            return TBOX_E_INVALID_POINTER;
        }

        MEM_CHUNK *chunk = (MEM_CHUNK *)((UINT32)ptr - sizeof(MEM_CHUNK) - MEM_GUARD_SIZE);
        if(MEM_CHUNK_MAGIC != chunk->magic || 0 == chunk->used)
        {
            xSemaphoreGive(pool->mutex);
            return TBOX_E_INVALID_DATA;
        }
        UINT32 chunk_size = MEM_CHUNK_SIZE(chunk->size);
        UINT32 zone_index = mem_zoneindex(&chunk_size);
        if(zone_index >= MEM_ZONE_COUNT)
        {
            xSemaphoreGive(pool->mutex);
            return TBOX_E_INVALID_DATA;
        }
        MEM_ZONE *zone = &pool->zones[zone_index];
        if(MEM_MAGIC != zone->magic || 
        chunk->size > zone->chunk_size)
        {
            xSemaphoreGive(pool->mutex);
            return TBOX_E_INVALID_DATA;
        }
        if(MEM_GUARD_PATTERN != *(UINT32 *)((UINT32)ptr - MEM_GUARD_SIZE))
        {
            xSemaphoreGive(pool->mutex);
            return TBOX_E_UNDERFLOW;        
        }
        if(MEM_GUARD_PATTERN != *(UINT32 *)((UINT32)ptr + chunk->size))
        {
            xSemaphoreGive(pool->mutex);
            return TBOX_E_OVERFLOW;
        }
    }
    xSemaphoreGive(pool->mutex);

    return TBOX_E_OK;   
}

TBOX_ERROR_CODE mp_check_pool(MP_HANDLE handle)
{
    if(handle >= TBOX_MEMPOOL_NUM || handle < 0)
    {
        return TBOX_E_INVALID_PARAM;
    }

    MP_POOL_PTR pool = MEM_GET_POOL(handle);
    if(FALSE == pool->is_used || 
       NULL == pool->mutex || 
       NULL_PTR == pool->buffer || 
       0U == pool->size)
    {
        return TBOX_E_INVALID_DATA;
    }

    return TBOX_E_OK;    
}
