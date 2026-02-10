#include <string.h>

#include "tbox_config.h"
#include "tbox_common.h"
#include "api_rtos.h"
#include "tbox_memory.h"
#include "tbox_hashmap.h"
#include "tbox_message_inner.h"
#include "tbox_module_inner.h"
#include "tbox_log_inner.h"

typedef struct tag_tbox_msg_handler_item
{
    TBOX_ID dst_id;
    TBOX_MSG_HADNLER  handler;
    struct tag_tbox_msg_handler_item *next;
}TBOX_MSG_HANDLER_ITEM;

typedef struct tag_tbox_msg_mgr_item
{
    UINT8 priority;
    UINT8 type;
    BOOL  enable;
    TBOX_MSG_HANDLER_ITEM *handler_list;
}TBOX_MSG_MGR_ITEM;

static volatile BOOL tbox_msg_is_init = FALSE;
static TBOX_HASHMAP *tbox_msg_hashmap = NULL;
static SemaphoreHandle_t tbox_msg_mutex;

static UINT32 tbox_msg_hashmap_string_hash_func(const VOID *key)
{
    CHAR *p_key = (CHAR *)key;
    return tbox_hashmap_default_hash_func(p_key, strlen(p_key));
}

static BOOL tbox_msg_hashmap_string_cmp_func(const VOID *key1, const VOID *key2)
{
    if(key1 == key2)
    {
        return TRUE;
    }
    return (strncmp((CHAR *)key1, (CHAR *)key2, strlen((CHAR *)key1)) == 0) ? TRUE : FALSE;
}

INT32 tbox_message_init(VOID)
{
#define TBOX_MESSAGE_MAP_MGR_SIZE ((TBOX_MESSAGE_MAX_NUM+48U)/48U)*512U

    if(TRUE == tbox_msg_is_init)
    {
        return (INT32)TBOX_E_HASINIT;
    }

    UINT32 size = sizeof(TBOX_MSG_MGR_ITEM)*TBOX_MESSAGE_MAX_NUM+TBOX_MESSAGE_MAP_MGR_SIZE;
    UINT8 *buffer = (UINT8 *)tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, size);
    if(NULL == buffer)
    {
        return (INT32)TBOX_E_FAILED_ALLOC;
    }

    memset(buffer, 0, size);
    tbox_msg_hashmap = tbox_hashmap_create(buffer, size, 
                                           TBOX_MESSAGE_MAX_NUM, 
                                           sizeof(TBOX_MSG_MGR_ITEM), 
                                           tbox_msg_hashmap_string_hash_func, 
                                           tbox_msg_hashmap_string_cmp_func);
    if(NULL == tbox_msg_hashmap)
    {
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, buffer);
        return (INT32)TBOX_E_FAILED_ALLOC;
    }

    tbox_msg_mutex = xSemaphoreCreateMutex();
    if(NULL == tbox_msg_mutex)
    {
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, buffer);
        tbox_hashmap_destroy(tbox_msg_hashmap);
        tbox_msg_hashmap = NULL_PTR;        
        return (INT32)TBOX_E_FAILED_CREATE;
    }

    tbox_msg_is_init = TRUE;

    return (INT32)TBOX_E_OK;
}

VOID tbox_message_deinit(VOID)
{
    if(FALSE == tbox_msg_is_init)
    {
        return;
    }

    if(NULL == tbox_msg_mutex)
    {
        MODULE_LOG_E(ICORE, "the mutext is null");
        return;
    }

    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL != tbox_msg_hashmap)
        {
            TBOX_MSG_MGR_ITEM item;
            UINT32 count = tbox_hashmap_get_count(tbox_msg_hashmap);
            for(UINT32 i = 0U; i < count; i++)
            {
                item.handler_list = NULL_PTR;
                tbox_hashmap_remove_one(tbox_msg_hashmap, &item);
                if(NULL != item.handler_list)
                {
                    TBOX_MSG_HANDLER_ITEM *handler_item = item.handler_list;
                    while(NULL != handler_item)
                    {
                        TBOX_MSG_HANDLER_ITEM *next = handler_item->next;
                        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, handler_item);
                        handler_item = next;
                    }
                }
            }
            tbox_hashmap_destroy(tbox_msg_hashmap);
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_msg_hashmap);
            tbox_msg_hashmap = NULL;
        }
    }
    xSemaphoreGive(tbox_msg_mutex);

    vSemaphoreDelete(tbox_msg_mutex);
    tbox_msg_mutex = NULL;

    tbox_msg_is_init = FALSE;

    MODULE_LOG_D(ICORE, "deinit the memssage module");    
}

VOID tbox_message_reset(VOID)
{
    /*DO NOTING*/
}

INT32 tbox_message_register(const TBOX_MSG_REGINFO *reginfo)
{
    if(NULL == reginfo || NULL == reginfo->name || 0 == strlen(reginfo->name))
    {
        MODULE_LOG_E(ICORE, "the reginfo is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(reginfo->priority >= TBOX_MSG_PRIORITY_MAX || reginfo->type >= TBOX_MSG_TYPE_MAX)
    {
        MODULE_LOG_E(ICORE, "the priority or type is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return (INT32)TBOX_E_NOINIT;
    }

    TBOX_MSG_MGR_ITEM item;
    item.priority = reginfo->priority;
    item.type = reginfo->type;
    item.enable = reginfo->enable;
    item.handler_list = NULL;
    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the hashmap is null");
            return (INT32)TBOX_E_NOINIT;
        }
        if(TBOX_E_OK != tbox_hashmap_put(tbox_msg_hashmap, (VOID *)reginfo->name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "put the item to the hashmap failed");
            return (INT32)TBOX_E_FAILED;
        }
    }
    xSemaphoreGive(tbox_msg_mutex);

    MODULE_LOG_I(ICORE, "register the message success, name:%s", reginfo->name);
    return (INT32)TBOX_E_OK;
}

VOID tbox_message_unregister(const CHAR *name)
{
    if(NULL == name || 0 == strlen(name))
    {
        MODULE_LOG_E(ICORE, "the name is invalid");
        return;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return;
    }

    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the hashmap is null");
            return;
        }
        
        TBOX_MSG_MGR_ITEM item;
        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "get the item from the hashmap failed");
            return;
        }
        TBOX_MSG_HANDLER_ITEM *handler_item = item.handler_list;
        while(NULL != handler_item)
        {
            TBOX_MSG_HANDLER_ITEM *next = handler_item->next;
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, handler_item);
            handler_item = next;
        }
        tbox_hashmap_remove(tbox_msg_hashmap, (VOID *)name);
    }
    xSemaphoreGive(tbox_msg_mutex);

    MODULE_LOG_I(ICORE, "unregister the message success, name:%s", name);
}

VOID tbox_message_enable(const CHAR *name, BOOL enable)
{
    if(NULL == name || 0 == strlen(name))
    {
        return;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return;
    }

    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            return;
        }

        TBOX_MSG_MGR_ITEM item;
        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            return;
        }
        item.enable = enable;
        tbox_hashmap_put(tbox_msg_hashmap, (VOID *)name, &item);
    }
    xSemaphoreGive(tbox_msg_mutex);

    MODULE_LOG_I(ICORE, "enable the message success, name:%s, enable:%d", name, enable);
}

BOOL tbox_message_is_enabled(const CHAR *name)
{
    BOOL enable = FALSE;
    if(NULL == name || 0 == strlen(name))
    {
        return enable;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return enable;
    }

    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            return enable;
        }

        TBOX_MSG_MGR_ITEM item;
        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            return enable;
        }
        enable = item.enable;
    }
    xSemaphoreGive(tbox_msg_mutex);

    return enable;
}

INT32 tbox_message_add_handler(const CHAR *name, TBOX_ID dst_id, TBOX_MSG_HADNLER handler)
{
    if(NULL == name || 0 == strlen(name) || NULL == handler)
    {
        MODULE_LOG_E(ICORE, "the name or handler is null");
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(dst_id <= TBOX_ID_INVALID)
    {
        MODULE_LOG_E(ICORE, "the dst_id is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return (INT32)TBOX_E_NOINIT;
    }

    TBOX_MSG_HANDLER_ITEM *handler_item = (TBOX_MSG_HANDLER_ITEM *)tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, sizeof(TBOX_MSG_HANDLER_ITEM));
    if(NULL == handler_item)
    {
        MODULE_LOG_E(ICORE, "alloc the handler_item failed");
        return (INT32)TBOX_E_FAILED_ALLOC;
    }
    handler_item->dst_id = dst_id;
    handler_item->handler = handler;
    handler_item->next = NULL;
    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, handler_item);
            MODULE_LOG_E(ICORE, "the hashmap is null");
            return (INT32)TBOX_E_NOINIT;
        }

        TBOX_MSG_MGR_ITEM item;
        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "get the item from the hashmap failed, name:%s", name);
            return (INT32)TBOX_E_FAILED;
        }
        if(TBOX_MSG_TYPE_MESSAGE != item.type)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the type is not message, name:%s", name);
            return (INT32)TBOX_E_INVALID_DATA;
        }

        TBOX_MSG_HANDLER_ITEM *item_head = item.handler_list;
        if(NULL == item_head)
        {
            item.handler_list = handler_item;
        }
        else
        {
            while(NULL != item_head)
            {
                if(item_head->dst_id == dst_id)
                {
                    break;
                }
                if(NULL == item_head->next)
                {
                    item_head->next = handler_item;
                    break;
                }
                item_head = item_head->next;
            }
        }
        tbox_hashmap_put(tbox_msg_hashmap, (VOID *)name, &item);
    }
    xSemaphoreGive(tbox_msg_mutex);

    MODULE_LOG_I(ICORE, "add the handler success, name:%s, dst_id:%d", name, dst_id)

    return (INT32)TBOX_E_OK;
}

INT32 tbox_message_send(const CHAR *name,  TBOX_ID src_id, TBOX_ID dst_id, TBOX_MSG_DATA *data)
{
    UINT16 size = 0U;

    if(NULL == name || 0 == strlen(name))
    {
        MODULE_LOG_E(ICORE, "the name or data is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(NULL_PTR != data)
    {
        size = data->size;
        if(size > 0U && NULL_PTR == data->data)
        {
            MODULE_LOG_E(ICORE, "the data is NULL_PTR");
            return (INT32)TBOX_E_INVALID_PARAM;            
        }
    }
    if(size > TBOX_MESSAGE_DATA_MAX_LEN)
    {
        MODULE_LOG_E(ICORE, "the data size is too large");
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(src_id <= TBOX_ID_INVALID || dst_id <= TBOX_ID_INVALID)
    {
        MODULE_LOG_E(ICORE, "the src_id or dst_id is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return (INT32)TBOX_E_NOINIT;
    }

    TBOX_MSG_HADNLER handle_func = NULL_PTR;
    TBOX_MSG_MGR_ITEM item;
    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the hashmap is null");
            return (INT32)TBOX_E_NOINIT;
        }
        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "get the item from the hashmap failed");
            return (INT32)TBOX_E_FAILED;
        }
        if(TBOX_MSG_TYPE_MESSAGE != item.type)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_W(ICORE, "the type is not message");    
            return (INT32)TBOX_E_INVALID_DATA;
        }
        if(FALSE == item.enable)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_W(ICORE, "the message is not enabled");
            return (INT32)TBOX_E_NOPRIVILEGE;
        }

        TBOX_MSG_HANDLER_ITEM *handler_item = item.handler_list;
        while(NULL != handler_item)
        {
            if(handler_item->dst_id == dst_id)
            {
                handle_func = handler_item->handler;
                break;
            }
            handler_item = handler_item->next;
        }
    }
    xSemaphoreGive(tbox_msg_mutex);

    if(NULL == handle_func)
    {
        MODULE_LOG_W(ICORE, "the message handler is not found");
        return (INT32)TBOX_E_NOEXISTS;
    }
    INT32 ret = (INT32)TBOX_E_OK;
    if(0U == size)
    {
        ret = tbox_module_send_message(dst_id, (CHAR *)name, handle_func, 0U, NULL_PTR);
    }
    else
    {
        ret = tbox_module_send_message(dst_id, (CHAR *)name, handle_func, size, data->data);
    }
    MODULE_LOG_D(ICORE, "send the message, name:%s, src_id:%d, dst_id:%d ret:%d", name, src_id, dst_id, ret);
    return ret;
}

INT32 tbox_message_subscribe(const CHAR *name, TBOX_ID dst_id, TBOX_MSG_HADNLER handler)
{
    if(NULL == name || 0 == strlen(name) || NULL == handler)
    {
        MODULE_LOG_E(ICORE, "the name or handler is null");
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(dst_id <= TBOX_ID_INVALID)
    {
        MODULE_LOG_E(ICORE, "the dst_id is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return (INT32)TBOX_E_NOINIT;
    }

    TBOX_MSG_HANDLER_ITEM *handler_item = (TBOX_MSG_HANDLER_ITEM *)tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, sizeof(TBOX_MSG_HANDLER_ITEM));
    if(NULL == handler_item)
    {
        MODULE_LOG_E(ICORE, "alloc the handler_item failed");
        return (INT32)TBOX_E_FAILED_ALLOC;
    }
    handler_item->dst_id = dst_id;
    handler_item->handler = handler;
    handler_item->next = NULL;
    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, handler_item);
            MODULE_LOG_E(ICORE, "the hashmap is null");
            return (INT32)TBOX_E_NOINIT;
        }

        TBOX_MSG_MGR_ITEM item;
        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "get the item from the hashmap failed");
            return (INT32)TBOX_E_FAILED;
        }
        if(TBOX_MSG_TYPE_TOPIC != item.type)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the type is not topic");
            return (INT32)TBOX_E_INVALID_DATA;
        }

        TBOX_MSG_HANDLER_ITEM *item_head = item.handler_list;
        if(NULL == item_head)
        {
            item.handler_list = handler_item;
        }
        else
        {
            while(NULL != item_head)
            {
                if(item_head->dst_id == dst_id)
                {
                    break;
                }
                if(NULL == item_head->next)
                {
                    item_head->next = handler_item;
                    break;
                }
                item_head = item_head->next;
            }
        }
        tbox_hashmap_put(tbox_msg_hashmap, (VOID *)name, &item);
    }
    xSemaphoreGive(tbox_msg_mutex);

    MODULE_LOG_I(ICORE, "subscribe the topic success, name:%s, dst_id:%d", name, dst_id)

    return (INT32)TBOX_E_OK;   
}

INT32 tbox_message_publish(const CHAR *name, TBOX_MSG_DATA *data)
{
    UINT16 size = 0U;

    if(NULL == name || 0 == strlen(name))
    {
        MODULE_LOG_E(ICORE, "the name or data is invalid");
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(NULL_PTR != data)
    {
        size = data->size;
        if(size > 0U && NULL_PTR == data->data)
        {
            MODULE_LOG_E(ICORE, "the data is NULL_PTR");
            return (INT32)TBOX_E_INVALID_PARAM;            
        }
    }
    if(size > TBOX_MESSAGE_DATA_MAX_LEN)
    {
        MODULE_LOG_E(ICORE, "the data size is too large");
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_msg_is_init)
    {
        MODULE_LOG_E(ICORE, "the message is not init");
        return (INT32)TBOX_E_NOINIT;
    }
    
    TBOX_MSG_MGR_ITEM item;
    xSemaphoreTake(tbox_msg_mutex, portMAX_DELAY);
    {
        if(NULL == tbox_msg_hashmap)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the hashmap is null");
            return (INT32)TBOX_E_NOINIT;
        }

        if(TBOX_E_OK != tbox_hashmap_get(tbox_msg_hashmap, (VOID *)name, &item))
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "get the item from the hashmap failed");
            return (INT32)TBOX_E_FAILED;
        }
        if(TBOX_MSG_TYPE_TOPIC != item.type)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the type is not topic"); 
            return (INT32)TBOX_E_INVALID_DATA;
        }
        if(FALSE == item.enable)
        {
            xSemaphoreGive(tbox_msg_mutex);
            MODULE_LOG_E(ICORE, "the topic is not enabled");
            return (INT32)TBOX_E_NOPRIVILEGE;
        }
    }
    xSemaphoreGive(tbox_msg_mutex);

    TBOX_MSG_HANDLER_ITEM *handler_item = item.handler_list;
    while(NULL != handler_item)
    {
        if(NULL == handler_item->handler || TBOX_ID_INVALID == handler_item->dst_id)
        {
            break;
        }
        
        MODULE_LOG_D(ICORE, "publish the message, name:%s, dst_id:%d", name, handler_item->dst_id);
        
        if(0U == size)
        {
            tbox_module_send_message(handler_item->dst_id, (CHAR *)name, handler_item->handler, 0U, NULL_PTR);
        }
        else
        {
            tbox_module_send_message(handler_item->dst_id, (CHAR *)name, handler_item->handler, size, data->data);
        }

        handler_item = handler_item->next;
    }

    return (INT32)TBOX_E_OK;
}