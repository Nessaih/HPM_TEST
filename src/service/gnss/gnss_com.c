#include "tbox_common.h"
#include "tbox_core.h"
#include "drv_eio_gnss.h"
#include "tbox_log.h"

#include "gnss_parse.h"

#define GNSS_COM_MSG_DATAIN    (1 << 0)

static TaskHandle_t gnss_task_handle;

VOID gnss_com_task(VOID *param)
{
	UNUSED(param);	
    UINT32 notify_value;
	
	for(;;)
	{
		BaseType_t notify_status = xTaskNotifyWait(0U, 0xFFFFFFFFU, &notify_value, pdTICKS_TO_MS(1000U));
        if (notify_status == pdFALSE)
        {
            continue;
        }

		if (notify_value & GNSS_COM_MSG_DATAIN) 
		{			
			gnss_parse_periodic();
		}
	}
	
}

static VOID gnss_com_datain_callback(VOID)
{
	BaseType_t task_switch = pdFALSE;
	if(NULL_PTR == gnss_task_handle)
	{
		return;
	}
	
	xTaskNotifyFromISR(gnss_task_handle, GNSS_COM_MSG_DATAIN, eSetBits, &task_switch);	
}

INT32 gnss_com_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			gnss_task_handle = NULL_PTR;
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			GET_TBOX_MODULE_HANDLE(GNSS, gnss_task_handle);
			drv_eio_gnss_register(gnss_com_datain_callback);
            break;
            
        default:
            break;
    }
	
	return 0;
}


