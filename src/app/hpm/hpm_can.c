#include "tbox_common.h"
#include "tbox_core.h"
#include "hpm_can.h"

static hpm_can_recv_cb_t hpm_can_recv_cb[HPM_CAN_MAX_CB];

static VOID hpm_can_callback_init(VOID)
{
    for (UINT16 i = 0; i < HPM_CAN_MAX_CB; i++)
    {
        hpm_can_recv_cb[i] = NULL;
    }
}

static INT32 hpm_can_event_handler(CAN_EVENT event, UINT32 arg1, UINT32 arg2)
{
    switch (event)
    {
    case CAN_EVENT_DATAIN:
    {
        can_msg_t *msgs = (can_msg_t *)arg1;
        UINT32 count = arg2;
        for (UINT16 i = 0; i < HPM_CAN_MAX_CB; i++)
        {
            if (NULL != hpm_can_recv_cb[i])
            {
                hpm_can_recv_cb[i](msgs, count);
            }
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

VOID hpm_can_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        break;
    case MODULE_INIT_SEQ_STORAGE:
        break;
    case MODULE_INIT_SEQ_MODULE:
        hpm_can_callback_init();
        can_if_reg_cb(hpm_can_event_handler);
        break;
    default:
        break;
    }
}

INT32 hpm_can_register(hpm_can_recv_cb_t cb)
{
    for (UINT16 i = 0; i < HPM_CAN_MAX_CB; i++)
    {
        if (NULL == hpm_can_recv_cb[i])
        {
            hpm_can_recv_cb[i] = cb;
            return i;
        }
    }
    return -1;
}
