#include <string.h>
#include "tbox_config.h"
#include "tbox_common.h"
#include "tbox_dlist.h"
#include "tbox_hashmap.h"

#define TBOX_HASHMAP_BUCKET_RATIO              10U   // 10%             
#define TBOX_HASHMAP_MGR_SIZE(BUCKET_NUM)      (sizeof(TBOX_HASHMAP) + (BUCKET_NUM) * sizeof(TBOX_HASHMAP_NODE_PTR))
#define TBOX_HASHMAP_INT_HASH_VALUE            2166136261ULL

typedef struct tag_tbox_hashmap_node 
{
    VOID *key;
    struct tag_tbox_hashmap_node *next;
} TBOX_HASHMAP_NODE, *TBOX_HASHMAP_NODE_PTR;

typedef struct tag_tbox_hashmap_queue
{
    TBOX_HASHMAP_NODE_PTR head;
    TBOX_HASHMAP_NODE_PTR tail;
} TBOX_HASHMAP_QUEUE;

struct tag_tbox_hashmap
{
    UINT32 capacity;
    UINT32 ele_size;
    UINT32 count;
    TBOX_HASHMAP_HASH_FUNC hash_func;
    TBOX_HASHMAP_KEY_COMPARE_FUNC key_compare_func;    
    TBOX_HASHMAP_NODE_PTR *buckets;
    TBOX_HASHMAP_QUEUE free_queue;
};

static inline VOID tbox_hashmap_init_queue(TBOX_HASHMAP_QUEUE *queue)
{
    queue->head = NULL_PTR;
    queue->tail = NULL_PTR;
}

static inline BOOL tbox_hashmap_queue_isempty(TBOX_HASHMAP_QUEUE *queue)
{
    return (queue->head == NULL_PTR && queue->tail == NULL_PTR) ? TRUE : FALSE;
}

static inline VOID tbox_hashmap_queue_push(TBOX_HASHMAP_QUEUE *queue, TBOX_HASHMAP_NODE_PTR node)
{
    if(NULL_PTR == node)
    {
        return;
    }

    node->next = NULL_PTR;
    if(NULL_PTR == queue->head)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        queue->tail->next = node;
        queue->tail = node;
    }
}

static inline TBOX_HASHMAP_NODE_PTR tbox_hashmap_queue_pop(TBOX_HASHMAP_QUEUE *queue)
{
    if(NULL_PTR == queue->head)
    {
        return NULL_PTR;
    }

    TBOX_HASHMAP_NODE_PTR node = queue->head;
    if(NULL_PTR == queue->head->next)
    {
        queue->head = NULL_PTR;
        queue->tail = NULL_PTR;
    }
    else
    {
        queue->head = node->next;
    }
    node->next = NULL_PTR;
    return node;
}

static inline UINT32 tbox_hashmap_capacity_to_bucket_num(UINT32 capacity)
{
    if(capacity <= 10U)
    {
        return capacity;
    }
    else if(capacity <= 100U)
    {
        return 10U;
    }
    else
    {
        return (capacity+TBOX_HASHMAP_BUCKET_RATIO-1) / TBOX_HASHMAP_BUCKET_RATIO;
    }
}

static inline TBOX_HASHMAP_NODE_PTR tbox_hashmap_find_node(TBOX_HASHMAP *map, const VOID *key)
{
    UINT32 hash_value = map->hash_func(key);
    UINT32 bucket_num = tbox_hashmap_capacity_to_bucket_num(map->capacity);
    UINT32 index = hash_value % bucket_num;
    TBOX_HASHMAP_NODE_PTR node = map->buckets[index];
    for(UINT32 i = 0U; i < map->capacity; i++)
    {
        if(NULL_PTR == node)
        {
            break;
        }
        if(TRUE == map->key_compare_func(key, node->key))
        {
            return node;
        }
        node = node->next;
    }
    return NULL_PTR;
}

TBOX_HASHMAP *tbox_hashmap_create(UINT8 *buffer, 
                                  UINT32 size,
                                  UINT32 capacity, 
                                  UINT32 ele_size, 
                                  TBOX_HASHMAP_HASH_FUNC hash_func, 
                                  TBOX_HASHMAP_KEY_COMPARE_FUNC key_compare_func)
{
    if(NULL_PTR == buffer || 0U == size ||
       0U == capacity || 0U == ele_size || 
       NULL_PTR == hash_func || NULL_PTR == key_compare_func)
    {
        return NULL_PTR;
    }

    memset(buffer, 0, size);

    TBOX_HASHMAP *map = (TBOX_HASHMAP *)buffer;
    UINT32 bucket_num = tbox_hashmap_capacity_to_bucket_num(capacity);
    if(size < (TBOX_HASHMAP_MGR_SIZE(bucket_num)+(ele_size+sizeof(TBOX_HASHMAP_NODE))*capacity))
    {
        return NULL_PTR;
    }
    UINT8 *ptr = (UINT8 *)((UINT32)buffer + TBOX_HASHMAP_MGR_SIZE(bucket_num));
    TBOX_HASHMAP_NODE_PTR node = NULL_PTR;
    tbox_hashmap_init_queue(&map->free_queue);
    for(UINT32 i = 0U; i < capacity; i++)
    {
        node = (TBOX_HASHMAP_NODE_PTR)ptr;
        node->key = NULL_PTR;
        node->next = NULL_PTR;
        tbox_hashmap_queue_push(&map->free_queue, node);
        ptr = (UINT8 *)((UINT32)ptr + ele_size + sizeof(TBOX_HASHMAP_NODE));
    }
    ptr = (UINT8 *)((UINT32)buffer + sizeof(TBOX_HASHMAP));
    map->buckets = (TBOX_HASHMAP_NODE_PTR *)ptr;
    for(UINT32 i = 0U; i < bucket_num; i++)
    {
        map->buckets[i] = NULL_PTR;
    }
    map->capacity = capacity;
    map->ele_size = ele_size;
    map->count = 0U;
    map->hash_func = hash_func;
    map->key_compare_func = key_compare_func;

    return map;
}

VOID tbox_hashmap_destroy(TBOX_HASHMAP *map)
{
    if(NULL_PTR == map)
    {
        return;
    }
    memset((UINT8 *)map, 0, sizeof(TBOX_HASHMAP));
}

INT32 tbox_hashmap_get(TBOX_HASHMAP *map, const VOID *key, VOID *value)
{
    if(NULL_PTR == map || NULL_PTR == key || NULL_PTR == value)
    {
        return TBOX_E_INVALID_PARAM;
    }

    TBOX_HASHMAP_NODE_PTR node = tbox_hashmap_find_node(map, key);
    if(NULL_PTR == node)
    {
        return TBOX_E_NOFOUND;
    }
    UINT8 *ptr = (UINT8 *)node + sizeof(TBOX_HASHMAP_NODE);
    memcpy(value, ptr, map->ele_size);
    return TBOX_E_OK;
}

INT32 tbox_hashmap_put(TBOX_HASHMAP *map, const VOID *key, VOID *value)
{
    if(NULL_PTR == map || NULL_PTR == key || NULL_PTR == value)
    {
        return TBOX_E_INVALID_PARAM;
    }

    TBOX_HASHMAP_NODE_PTR node = tbox_hashmap_find_node(map, key);
    if(NULL_PTR == node)
    {
        if(tbox_hashmap_queue_isempty(&map->free_queue))
        {
            return TBOX_E_NOMEMORY;
        }
        node = tbox_hashmap_queue_pop(&map->free_queue);
        node->key = (VOID *)key;
        node->next = NULL_PTR;
        UINT8 *ptr = (UINT8 *)node + sizeof(TBOX_HASHMAP_NODE);
        memcpy(ptr, value, map->ele_size);

        UINT32 hash_value = map->hash_func(key);
        UINT32 bucket_num = tbox_hashmap_capacity_to_bucket_num(map->capacity);
        UINT32 index = hash_value % bucket_num;
        if(NULL_PTR == map->buckets[index])
        {
            map->buckets[index] = node;
        }
        else
        {
            TBOX_HASHMAP_NODE_PTR prev = map->buckets[index];
            for(UINT32 i = 0U; i < map->capacity; i++)
            {
                if(NULL == prev->next)
                {
                    prev->next = node;
                    break;
                }
                prev = prev->next;
            }
        }
        map->count++;
    }
    else
    {
        UINT8 *ptr = (UINT8 *)node + sizeof(TBOX_HASHMAP_NODE);
        memcpy(ptr, value, map->ele_size);
    }

    return TBOX_E_OK;
}

VOID tbox_hashmap_remove(TBOX_HASHMAP *map, const VOID *key)
{
    if(NULL_PTR == map || NULL_PTR == key)
    {
        return;
    }

    TBOX_HASHMAP_NODE_PTR node = tbox_hashmap_find_node(map, key);
    if(NULL_PTR == node)
    {
        return;
    }
    UINT32 hash_value = map->hash_func(key);
    UINT32 bucket_num = tbox_hashmap_capacity_to_bucket_num(map->capacity);
    UINT32 index = hash_value % bucket_num;
    if(node == map->buckets[index])
    {
        map->buckets[index] = node->next;
    }
    else
    {
        TBOX_HASHMAP_NODE_PTR prev = map->buckets[index];
        for(UINT32 i = 0U; i < map->capacity; i++)
        {
            if(NULL_PTR == prev)
            {
                break;
            }
            if(node == prev->next)
            {
                prev->next = node->next;
                break;
            }
            prev = prev->next;
        }
    }
    node->key = NULL_PTR;
    node->next = NULL_PTR;
    UINT8 *ptr = (UINT8 *)node + sizeof(TBOX_HASHMAP_NODE); 
    memset(ptr, 0, map->ele_size);    
    tbox_hashmap_queue_push(&map->free_queue, node);
    map->count--;
}

INT32 tbox_hashmap_remove_one(TBOX_HASHMAP *map, VOID *value)
{
    if(NULL_PTR == map || NULL_PTR == value)
    {
        return TBOX_E_INVALID_PARAM;
    }
    
    UINT32 bucket_num = tbox_hashmap_capacity_to_bucket_num(map->capacity);
    for(UINT32 i = 0U; i < bucket_num; i++)
    {
        TBOX_HASHMAP_NODE_PTR node = map->buckets[i];
        if(NULL_PTR != node)
        {
            map->buckets[i] = node->next;
            node->key = NULL_PTR;
            node->next = NULL_PTR;
            UINT8 *ptr = (UINT8 *)node + sizeof(TBOX_HASHMAP_NODE);
            memcpy(value, ptr, map->ele_size);
            memset(ptr, 0, map->ele_size);  
            tbox_hashmap_queue_push(&map->free_queue, node);
            map->count--;
            return TBOX_E_OK;
        }
    }

    return TBOX_E_FAILED;
}

UINT32 tbox_hashmap_get_count(TBOX_HASHMAP *map)
{
    if(NULL_PTR == map)
    {
        return 0U;
    }
    return map->count;
}

UINT32 tbox_hashmap_default_hash_func(const VOID *key, UINT32 size)
{
    if(NULL_PTR == key || 0U == size)
    {
        return 0U;
    }

    UINT8 *data = (UINT8 *)key;
	UINT32 nblocks = size / 8U;
	UINT64 hash = TBOX_HASHMAP_INT_HASH_VALUE;
	for (UINT32 i = 0U; i < nblocks; ++i)
	{
		hash ^= ((UINT64)data[0] << 0ULL) | 
                ((UINT64)data[1] << 8ULL) |
			    ((UINT64)data[2] << 16ULL) | 
                ((UINT64)data[3] << 24ULL) |
                ((UINT64)data[4] << 32ULL) | 
                ((UINT64)data[5] << 40ULL) |
			    ((UINT64)data[6] << 48ULL) | 
                ((UINT64)data[7] << 56ULL);
		hash *= 0xbf58476d1ce4e5b9ULL;
		data += 8U;
	}

	UINT64 last = (UINT64)(size & 0xFFU);
	switch (size % 8U)
	{
	case 7U:
		last |= ((UINT64)data[6] << 56ULL); /* fallthrough */
	case 6U:
		last |= ((UINT64)data[5] << 48ULL); /* fallthrough */
	case 5U:
		last |= ((UINT64)data[4] << 40ULL); /* fallthrough */
	case 4:
		last |= ((UINT64)data[3] << 32ULL); /* fallthrough */
	case 3:
		last |= ((UINT64)data[2] << 24ULL); /* fallthrough */
	case 2:
		last |= ((UINT64)data[1] << 16ULL); /* fallthrough */
	case 1:
		last |= ((UINT64)data[0] << 8ULL); /* fallthrough */
		hash ^= last;
		hash *= 0xd6e8feb86659fd93ULL;
	}

	return (UINT32)(hash ^ hash >> 32ULL);    
}
