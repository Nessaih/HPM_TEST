#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_pm_if.h"
#include "tbox_phm_module.h"

#define TBOX_PHM_MODULE_ABONORMAL_MAX_COUNT       3U
#define TBOX_PHM_MODULE_PERIOD_VALUE              5U //5S

typedef struct
{
    BOOL used;
    BOOL is_handle;
    TBOX_ID module_id;
    TBOX_ID task_id;
    UINT8 abonormal_count;
    MODULE_HANDLE handle;
}TBOX_PHM_MODULE;

static VOID tbox_phm_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_phm_handle_memory_abnormal(const CHAR *name, TBOX_MSG_DATA *data);

static volatile UINT8 tbox_phm_module_start_flag;
static UINT8 tbox_phm_module_period_value;
static SemaphoreHandle_t tbox_phm_mutex;
static TBOX_PHM_MODULE tbox_phm_module_info[TBOX_MODULE_MAX_NUM];

INT32 tbox_phm_module_init(VOID)
{
    tbox_phm_module_start_flag = 0U;
    tbox_phm_module_period_value = 0U;
    for(UINT8 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
    {
        tbox_phm_module_info[i].used = FALSE;
        tbox_phm_module_info[i].is_handle = FALSE;
        tbox_phm_module_info[i].module_id = TBOX_ID_INVALID;
        tbox_phm_module_info[i].task_id = TBOX_ID_INVALID;
        tbox_phm_module_info[i].abonormal_count = 0U;
        tbox_phm_module_info[i].handle = NULL_PTR;
    }
    tbox_phm_mutex = xSemaphoreCreateMutex();
    if(NULL_PTR == tbox_phm_mutex)
    {
        MODULE_LOG_E(TBOXPHM, "failed to create mutex");
        return (INT32)TBOX_E_FAILED_INIT;
    }

    tbox_supervice_set_abnormal_process(MODULE_ABNORMAL_MEMORY_OVERFLOW, MODULE_ABNORMAL_PROCESS_WARNING);
    tbox_supervice_set_abnormal_process(MODULE_ABNORMAL_MEMORY_UNDERFLOW, MODULE_ABNORMAL_PROCESS_WARNING);
    tbox_supervice_set_abnormal_process(MODULE_ABNORMAL_TASK_BLOCKED, MODULE_ABNORMAL_PROCESS_RESET);
    tbox_supervice_set_abnormal_process(MODULE_ABNORMAL_TASK_DELETED, MODULE_ABNORMAL_PROCESS_RESET);
    tbox_supervice_set_abnormal_process(MODULE_ABNORMAL_TASKSTACK_SMALL, MODULE_ABNORMAL_PROCESS_RESET);
    tbox_supervice_set_abnormal_process(MODULE_ABNORMAL_STACK_OVERFLOW, MODULE_ABNORMAL_PROCESS_FORBID);

    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXPHM, module_id);
    tbox_message_subscribe(TBOX_CORE_TASK_ABNORMAL_NOTIFY, module_id, tbox_phm_handle_task_abnormal);
    tbox_message_subscribe(TBOX_CORE_MEMORY_ABNORMAL_NOTIFY, module_id, tbox_phm_handle_memory_abnormal);

    return (INT32)TBOX_E_OK;
}

VOID tbox_phm_module_start(VOID)
{
    tbox_phm_module_start_flag = 1U;
    tbox_phm_module_period_value = 0U;
}

VOID tbox_phm_module_stop(VOID)
{
    tbox_phm_module_start_flag = 0U;
}

VOID tbox_phm_module_period(VOID)
{
    if(tbox_phm_module_period_value > 0U)
    {
        tbox_phm_module_period_value--;
    }
    if(tbox_phm_module_period_value > 0U)
    {
        return;
    }
    tbox_phm_module_period_value = TBOX_PHM_MODULE_PERIOD_VALUE;
    if(1U != tbox_phm_module_start_flag)
    {
        return;
    }

    xSemaphoreTake(tbox_phm_mutex, portMAX_DELAY);
    {
        for(UINT8 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
        {
            if(FALSE == tbox_phm_module_info[i].used || 
               TRUE == tbox_phm_module_info[i].is_handle ||
               TBOX_ID_INVALID == tbox_phm_module_info[i].module_id ||
               tbox_phm_module_info[i].abonormal_count <= TBOX_PHM_MODULE_ABONORMAL_MAX_COUNT)
            {
                continue;
            }
            xSemaphoreGive(tbox_phm_mutex);

            MODULE_LOG_E(TBOXPM, "the module[%d]'s abnormal count has more the %d, disable this module", 
                         tbox_phm_module_info[i].module_id, TBOX_PHM_MODULE_ABONORMAL_MAX_COUNT);
            tbox_module_enable(tbox_phm_module_info[i].module_id, FALSE);

            xSemaphoreTake(tbox_phm_mutex, portMAX_DELAY);
            tbox_phm_module_info[i].is_handle = TRUE;
        }
    }
    xSemaphoreGive(tbox_phm_mutex);
}

static VOID tbox_phm_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_ABNORMAL_INFO *info = (TBOX_CORE_TASK_ABNORMAL_INFO *)data->data;
    if(0U != strncmp(TBOX_CORE_TASK_ABNORMAL_NOTIFY, name, strlen(TBOX_CORE_TASK_ABNORMAL_NOTIFY)) ||
       NULL_PTR == data)
    {
        return;
    }

    if(data->size < sizeof(TBOX_CORE_TASK_ABNORMAL_INFO))
    {
        return;
    }
    if(NULL_PTR == info)
    {
        return;
    }

    MODULE_LOG_E(TBOXPHM, "task abnormal, task_id:%d, module_id:%d, handle:%p", info->task_id, info->module_id, info->task_handle);
    
    if(TBOX_ID_INVALID == info->module_id)
    {
        return;
    }

    xSemaphoreTake(tbox_phm_mutex, portMAX_DELAY);
    {
        UINT8 index;
        for(index = 0U; index < TBOX_MODULE_MAX_NUM; index++)
        {
            if(TRUE == tbox_phm_module_info[index].used && 
              tbox_phm_module_info[index].module_id == info->module_id &&
              tbox_phm_module_info[index].is_handle == FALSE)
            {
                tbox_phm_module_info[index].abonormal_count++;
                xSemaphoreGive(tbox_phm_mutex);
                return;
            }
        }
        for(index = 0U; index < TBOX_MODULE_MAX_NUM; index++)
        {
            if(FALSE == tbox_phm_module_info[index].used)
            {
                tbox_phm_module_info[index].used = TRUE;
                tbox_phm_module_info[index].module_id = info->module_id;
                tbox_phm_module_info[index].task_id = info->task_id;
                tbox_phm_module_info[index].abonormal_count = 1U;
                tbox_phm_module_info[index].handle = info->task_handle;
                xSemaphoreGive(tbox_phm_mutex);
                return;
            }
        }
    }
    xSemaphoreGive(tbox_phm_mutex);
}

static VOID tbox_phm_handle_memory_abnormal(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_MEMORY_ABNORMAL_INFO *info = (TBOX_CORE_MEMORY_ABNORMAL_INFO *)data->data;

    if(0U != strncmp(TBOX_CORE_MEMORY_ABNORMAL_NOTIFY, name, strlen(TBOX_CORE_MEMORY_ABNORMAL_NOTIFY)) ||
       NULL_PTR == data)
    {
        return;
    }

    if(data->size < sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO))
    {
        return;
    }
    if(NULL_PTR == info)
    {
        return;
    }

    MODULE_LOG_E(TBOXPHM, "memeory abnormal, addr:%p", info->addr);
}
