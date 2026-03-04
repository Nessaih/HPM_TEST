#include "tbox_common.h"
#include "tbox_core.h"

#include "hpm_data_recv.h"
#include "hpm_content.h"
#include "hpm_control.h"

#define HPM_DATA_RECV_BUFFER_SIZE (1024UL)

typedef struct
{
    UINT16 head;
    UINT16 tail;
    UINT16 count;
    UINT8 *data;
} hpm_data_recv_queue_t;

static UINT8 hpm_data_recv_buffer[HPM_DATA_RECV_BUFFER_SIZE];
static hpm_data_recv_queue_t hpm_data_recv_queue;
static hpm_pack_recv_t hpm_data_recv_info;

UINT16 hpm_data_recv_count(VOID)
{
    UINT16 count = 0;
    count = hpm_data_recv_queue.count;
    return count;
}

BOOL hpm_data_recv_empty(VOID)
{
    BOOL empty = TRUE;
    empty = (hpm_data_recv_queue.count == 0);
    return empty;
}

VOID hpm_data_recv_clear(VOID)
{
    hpm_data_recv_queue.head = 0;
    hpm_data_recv_queue.tail = 0;
    hpm_data_recv_queue.count = 0;
    memset(&hpm_data_recv_info, 0, sizeof(hpm_pack_recv_t));
}

UINT16 hpm_data_recv_push(UINT8 *data, UINT16 size)
{
    if ((NULL_PTR == data) || (0 == size) || (size > HPM_DATA_RECV_BUFFER_SIZE))
    {
        return 0;
    }

    for (UINT16 i = 0; i < size; i++)
    {
        if (hpm_data_recv_queue.count == HPM_DATA_RECV_BUFFER_SIZE)
        {
            hpm_data_recv_queue.head = (hpm_data_recv_queue.head + 1) & (HPM_DATA_RECV_BUFFER_SIZE - 1);
        }
        else
        {
            hpm_data_recv_queue.count++;
        }
        hpm_data_recv_queue.data[hpm_data_recv_queue.tail] = data[i];
        hpm_data_recv_queue.tail = (hpm_data_recv_queue.tail + 1) & (HPM_DATA_RECV_BUFFER_SIZE - 1);
    }
    return size;
}

UINT16 hpm_data_recv_pop(UINT16 size)
{
    if ((0 == size) || (size > HPM_DATA_RECV_BUFFER_SIZE))
    {
        return 0;
    }

    if (TRUE == hpm_data_recv_empty())
    {
        return 0;
    }

    UINT16 require = 0;
    require = (size > hpm_data_recv_queue.count) ? hpm_data_recv_queue.count : size;
    for (UINT16 i = 0; i < require; i++)
    {
        hpm_data_recv_queue.head = (hpm_data_recv_queue.head + 1) & (HPM_DATA_RECV_BUFFER_SIZE - 1);
        hpm_data_recv_queue.count--;
    }
    return require;
}

UINT16 hpm_data_recv_peek(UINT8 *data, UINT16 size)
{
    if ((NULL_PTR == data) || (0 == size) || (size > HPM_DATA_RECV_BUFFER_SIZE))
    {
        return 0;
    }

    if (TRUE == hpm_data_recv_empty())
    {
        return 0;
    }

    UINT16 require = 0;
    require = (size > hpm_data_recv_queue.count) ? hpm_data_recv_queue.count : size;
    for (UINT16 i = 0; i < require; i++)
    {
        UINT16 pos = (hpm_data_recv_queue.head + i) & (HPM_DATA_RECV_BUFFER_SIZE - 1);
        data[i] = hpm_data_recv_queue.data[pos];
    }
    return require;
}

hpm_pack_recv_t *hpm_data_recv_info_get(VOID)
{
    return &hpm_data_recv_info;
}

INT32 hpm_data_recv_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        hpm_data_recv_queue.head = 0;
        hpm_data_recv_queue.tail = 0;
        hpm_data_recv_queue.count = 0;
        hpm_data_recv_queue.data = hpm_data_recv_buffer;
        memset(&hpm_data_recv_info, 0, sizeof(hpm_pack_recv_t));
        break;

    case MODULE_INIT_SEQ_STORAGE:
        break;

    case MODULE_INIT_SEQ_MODULE:
        break;

    default:
        break;
    }

    return 0;
}

static BOOL hpm_data_recv_is_server_req(UINT8 cmd)
{
    if (cmd == (UINT8)HPM_CMD_CONTROL)
    {
        return TRUE;
    }

    if (cmd == (UINT8)HPM_CMD_CONTROL_CAR)
    {
        return TRUE;
    }

    return FALSE;
}

VOID hpm_data_recv_process(VOID)
{
    hpm_pack_recv_t *recv = &hpm_data_recv_info;
    memset(recv, 0, sizeof(hpm_pack_recv_t));

    if (TRUE == hpm_data_recv_empty())
    {
        return;
    }

    UINT8 *data = mempool_alloc(HPM_SESSION_RECV_MEM_SIZE);
    if (NULL == data)
    {
        MODULE_LOG_E(HPM, "memalloc pack head buf failed");
        return;
    }

    UINT16 parselen = HPM_SESSION_RECV_MEM_SIZE;
    UINT16 size = hpm_data_recv_peek(data, HPM_SESSION_RECV_MEM_SIZE);
    INT32 ret = hpm_pack_unpack(data, size, recv, &parselen);
    if (HPM_PACK_PARSE_OK != ret)
    {
        if (HPM_PACK_PARSE_NOT_COMPLETE != ret)
        {
            hpm_data_recv_pop(parselen);
        }
    }
    else
    {
        hpm_data_recv_pop(parselen);
    }

    if (parselen > 0)
    {
        MODULE_LOG_DUMP(HPM, "hpm receive info", data, parselen);
    }

    if (TRUE == hpm_data_recv_is_server_req(recv->cmd))
    {
        hpm_control_cmd_handle(recv->cmd, recv->data, recv->len);
        recv->cmd = (UINT8)HPM_CMD_INVALID;
        recv->len = 0;
    }

    mempool_free(data);
}