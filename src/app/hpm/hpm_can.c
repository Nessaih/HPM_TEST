#include "tbox_common.h"
#include "tbox_core.h"
#include "task.h"
#include "can_if.h"
#include "hpm_can.h"
#include "hpm_param_fetch.h"

#define HPM_CAN_DATA_LEN (8U)
#define HPM_CAN_FETCH_DATA_LEN (512U)
#define HPM_CAN_LOCK() xSemaphoreTake(hpm_can_mutex, portMAX_DELAY)
#define HPM_CAN_UNLOCK() xSemaphoreGive(hpm_can_mutex)

typedef enum
{
    HPM_CAN_TYPE_SINGLE = 1,
    HPM_CAN_TYPE_COMPLEX = 2,
    HPM_CAN_TYPE_PGN_BC = 3,
    HPM_CAN_TYPE_PGN_REQ = 4,
    HPM_CAN_TYPE_DID = 5,
} HPM_DATA_TYPE;

typedef struct
{
    uint32_t canid;
    uint8_t data[HPM_CAN_DATA_LEN];
} HPM_CAN_TYPE1_SINGLE;

typedef struct
{
    uint8_t flag;
    uint16_t len;
    uint8_t data[HPM_CAN_FETCH_DATA_LEN];
} HPM_CAN_DATA;

static VOID hpm_can_event_handler(UINT32 event, VOID *para);

static HPM_CAN_TYPE1_SINGLE hpm_can1_single[HPM_PARAM_MAX_CAN_TYPE1_SINGLE];
static HPM_CAN_DATA hpm_fetch_data;
static SemaphoreHandle_t hpm_can_mutex = NULL;

static VOID hpm_can_fetch_data_reset(VOID)
{
    memset(hpm_can1_single, 0, sizeof(hpm_can1_single));
    for (INT32 i = 0; i < HPM_PARAM_MAX_CAN_TYPE1_SINGLE; i++)
    {
        hpm_can1_single[i].canid = 0xFFFFFFFF;
    }
}

static VOID hpm_can_recv_can1(UINT32 id, UINT8 *data)
{
    INT8 index = hpm_param_canid_type1_index_find(id);
    if (index < 0)
    {
        return;
    }
    HPM_CAN_LOCK();
    hpm_can1_single[index].canid = id;
    memcpy(hpm_can1_single[index].data, data, HPM_CAN_DATA_LEN);
    HPM_CAN_UNLOCK();
}

VOID hpm_can_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        hpm_can_mutex = xSemaphoreCreateMutex();
        break;
    case MODULE_INIT_SEQ_STORAGE:
        break;
    case MODULE_INIT_SEQ_MODULE:
        can_if_regcb(hpm_can_event_handler);
        hpm_can_fetch_data_reset();
        memset(&hpm_fetch_data, 0, sizeof(hpm_fetch_data));
        break;
    default:
        break;
    }
}

VOID hpm_can_deinit(VOID)
{
    if (NULL != hpm_can_mutex)
    {
        vSemaphoreDelete(hpm_can_mutex);
        hpm_can_mutex = NULL;
    }
}

static VOID hpm_can_event_handler(UINT32 event, VOID *para)
{
    switch (event)
    {
    case CAN_IF_EVENT_RECEIVED:
    {
        can_msg_t *msg = (can_msg_t *)para;
        if (msg)
        {
            UINT32 canid = msg->id & 0x7FFFFFFFU;
            hpm_can_recv_can1(canid, msg->data);
        }
        break;
    }
    default:
        break;
    }
}

static INT32 hpm_can_single_report(UINT8 *data)
{
    UINT16 len = 0;
    data[len++] = HPM_CAN_TYPE_SINGLE;
    UINT8 *len_pos = &data[len];

    data[len++] = 0x00;
    data[len++] = 0x00;

    for (INT32 i = 0; i < HPM_PARAM_MAX_CAN_TYPE1_SINGLE; i++)
    {
        if (0xFFFFFFFF != hpm_can1_single[i].canid)
        {
            data[len++] = (UINT8)(hpm_can1_single[i].canid >> 24);
            data[len++] = (UINT8)(hpm_can1_single[i].canid >> 16);
            data[len++] = (UINT8)(hpm_can1_single[i].canid >> 8);
            data[len++] = (UINT8)(hpm_can1_single[i].canid >> 0);
            data[len++] = HPM_CAN_DATA_LEN;
            memcpy(&data[len], hpm_can1_single[i].data, HPM_CAN_DATA_LEN);
            len += HPM_CAN_DATA_LEN;
        }
    }

    len_pos[0] = (UINT8)((len - 3) >> 8);
    len_pos[1] = (UINT8)((len - 3) >> 0);

    return len;
}

INT32 hpm_can_report(UINT8 *data)
{
    UINT16 len = 0;
    UINT8 *data_len = &data[len];
    data[len++] = 0;
    data[len++] = 0;
    UINT32 id = hpm_param_get_id();
    data[len++] = (UINT8)(id >> 24);
    data[len++] = (UINT8)(id >> 16);
    data[len++] = (UINT8)(id >> 8);
    data[len++] = (UINT8)(id >> 0);
    len += hpm_can_single_report(&data[len]);
    data_len[0] = (UINT8)((len - 2) >> 8);
    data_len[1] = (UINT8)((len - 2) >> 0);
    hpm_can_fetch_data_reset();
    return len;
}