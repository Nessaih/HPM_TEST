#ifndef TBOX_MEMORY_H
#define TBOX_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_tbox_memory_type
{
    TBOX_MEMORY_TYPE_CORE    = 0U,
    TBOX_MEMORY_TYPE_SERVICE,
    TBOX_MEMORY_TYPE_APP,
    TBOX_MEMORY_TYPE_MAX
}TBOX_MEMORY_TYPE;

#define MP_DEFAULT_TYPE TBOX_MEMORY_TYPE_CORE
#define mempool_alloc(size) tbox_memory_alloc(MP_DEFAULT_TYPE, size)
#define mempool_free(ptr) tbox_memory_free(MP_DEFAULT_TYPE, ptr)
#define mempool_check(ptr) tbox_memory_check(MP_DEFAULT_TYPE, ptr)

VOID *tbox_memory_alloc(TBOX_MEMORY_TYPE type, UINT32 size);
VOID tbox_memory_free(TBOX_MEMORY_TYPE type, VOID *ptr);
INT32 tbox_memory_check(TBOX_MEMORY_TYPE type, VOID *ptr);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_MEMORY_H */