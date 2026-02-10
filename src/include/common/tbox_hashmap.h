#ifndef TBOX_HASHMAP_H
#define TBOX_HASHMAP_H

#include "tbox_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tag_tbox_hashmap TBOX_HASHMAP;
typedef UINT32 (*TBOX_HASHMAP_HASH_FUNC)(const VOID *key);
typedef BOOL   (*TBOX_HASHMAP_KEY_COMPARE_FUNC)(const VOID *key1, const VOID *key2);

TBOX_HASHMAP *tbox_hashmap_create(UINT8 *buffer, 
                                  UINT32 size,
                                  UINT32 capacity, 
                                  UINT32 ele_size, 
                                  TBOX_HASHMAP_HASH_FUNC hash_func, 
                                  TBOX_HASHMAP_KEY_COMPARE_FUNC key_compare_func);
VOID tbox_hashmap_destroy(TBOX_HASHMAP *map);
INT32 tbox_hashmap_get(TBOX_HASHMAP *map, const VOID *key, VOID *value);
INT32 tbox_hashmap_put(TBOX_HASHMAP *map, const VOID *key, VOID *value);
VOID tbox_hashmap_remove(TBOX_HASHMAP *map, const VOID *key);
INT32 tbox_hashmap_remove_one(TBOX_HASHMAP *map, VOID *value);
UINT32 tbox_hashmap_get_count(TBOX_HASHMAP *map);
UINT32 tbox_hashmap_default_hash_func(const VOID *key, UINT32 size);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_HASHMAP_H */