#include "tbox_common.h"
#include "tbox_core.h"
#include "cqueue.h"

#include "hpm_data_recv.h"
#include "hpm_content.h"

#define HPM_DATA_RECV_BUFFER_SIZE 1024

static UINT8 	 hpm_data_recv_buffer[HPM_DATA_RECV_BUFFER_SIZE];
static cqueue_t  hpm_data_recv_queue;

static VOID hpm_data_recv_queue_init(VOID)
{
	hpm_data_recv_queue.buffer = hpm_data_recv_buffer;
    hpm_data_recv_queue.size   = HPM_DATA_RECV_BUFFER_SIZE;
    hpm_data_recv_queue.in     = 0;
    hpm_data_recv_queue.out    = 0;
}

INT32 hpm_data_recv_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            hpm_data_recv_queue_init();
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

BOOL hpm_data_recv_empty(VOID)
{
	UINT16 data_len;

	HPM_MUTEX_LOCK();
    data_len = cqueue_datalen(&hpm_data_recv_queue);
	HPM_MUTEX_UNLOCK();

    if (0 == data_len)
        return true;
    return false;
}

INT32 hpm_data_recv_put(UINT8 *data, UINT16 len)
{
	UINT16 put_len;
	
	HPM_MUTEX_LOCK();
    put_len = cqueue_put(&hpm_data_recv_queue, data, len);
	HPM_MUTEX_UNLOCK();

    if (put_len != len)
    {
        return -1;
    }

    return 0;
}

INT32 hpm_data_recv_get(UINT8 *data, UINT16 *len)
{
    UINT16 get_len = *len;

	HPM_MUTEX_LOCK();
    get_len = cqueue_get(&hpm_data_recv_queue, data, get_len);
	HPM_MUTEX_UNLOCK();

    if (0 == get_len)
    {
        return -1;
    }
    *len = get_len;
    return 0;
}

