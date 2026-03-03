#ifndef __FOTA_IF_H__
#define __FOTA_IF_H__

#include "tbox_type.h"

typedef enum
{
    FOTA_RST_OK 	= 0x00,
    FOTA_RST_NG		= 0x01,
}FOTA_RESULT_E;

typedef enum
{
	FOTA_STA_CMD_OK	= 0x00,
	FOTA_STA_CMD_NG	= 0x01,
	FOTA_STA_DWL_OK	= 0x02,
	FOTA_STA_DWL_NG	= 0x03,
	FOTA_STA_CHK_OK	= 0x04,
	FOTA_STA_CHK_NG	= 0x05,
	FOTA_STA_OTA_OK	= 0x06,
	FOTA_STA_OTA_NG	= 0x07,
}FOTA_STATE_TYPE_E;

typedef struct
{
    INT32  id;
	UINT16 seq;
	INT8   rst;
	INT8   err_code;
}FOTA_RESULT_INFO_T;

#define FOTA_EVENT_STATE_CHANGE     "FOTA_STATE_CHANGE"

INT32 fota_do_upgrade(UINT8 *url, UINT16 url_len, INT32 id, UINT16 seq);

INT32 fota_do_upgrade_with_info(UINT8 *url, UINT16 url_len, INT32 id, UINT16 seq, CHAR *ver, INT32 file_sz);

#endif
