#ifndef __HPM_CONTROL_H__
#define __HPM_CONTROL_H__

#define HPM_FOTA_URL_MAX_LEN	(128)
#define HPM_FOTA_VER_MAX_LEN	(64)
#define HPM_FOTA_MD5_MAX_LEN	(16)

typedef enum
{
    HPM_CTRL_DATA_REQ 	= 0x01,
    HPM_CTRL_CFG_READ 	= 0x02,
    HPM_CTRL_CFG_SET 	= 0x03,
    HPM_CTRL_LOCK_ENB 	= 0x04,
    HPM_CTRL_LOCK_RELAY = 0x05,
    HPM_CTRL_LOCK_CTRL 	= 0x06,
    HPM_CTRL_CAN_FILE 	= 0x0A,
    HPM_CTRL_RESET 		= 0x0B,
    HPM_CTRL_REBOOT 	= 0x0C,
	HPM_CTRL_UPGRADE 	= 0x0D,
	HPM_CTRL_TRANSMIS 	= 0x0E,
} HPM_CTRL_SUB_TYPE_E;

typedef enum
{
	HPM_CTRL_RES_NG	= 0x00,
	HPM_CTRL_RES_OK	= 0x01,
}HPM_CONTROL_RESULT_E;

typedef enum
{
	HPM_FOTA_STA_CMD_OK	= 0x00,
	HPM_FOTA_STA_CMD_NG	= 0x01,
	HPM_FOTA_STA_DWL_OK	= 0x02,
	HPM_FOTA_STA_DWL_NG	= 0x03,
	HPM_FOTA_STA_CHK_OK	= 0x04,
	HPM_FOTA_STA_CHK_NG	= 0x05,
	HPM_FOTA_STA_OTA_OK	= 0x06,
	HPM_FOTA_STA_OTA_NG	= 0x07,
}HPM_CTRL_FOTA_ERR_CODE_E;

typedef enum
{
	HPM_CTRL_FOTA_IDLE	= 0x00,
	HPM_CTRL_FOTA_BUSY	= 0x01,
}HPM_CTRL_FOTA_STATE_E;

typedef enum
{
	HPM_CTRL_FOTA_TBOX	= 0x00,
	HPM_CTRL_FOTA_ECU	= 0x01,
}HPM_CTRL_FOTA_TYPE_E;

typedef struct
{
	UINT32 magic;
	UINT16 seq;
	UINT8  cmd;
	UINT8  cmd_sub;
	BOOL   result;
	UINT8  type;
	UINT8  fota_state;
	UINT8  err_code;
	UINT8  fota_url[HPM_FOTA_URL_MAX_LEN];
	UINT8  fota_ver[HPM_FOTA_VER_MAX_LEN];
	UINT8  fota_md5[HPM_FOTA_MD5_MAX_LEN];
}HPM_CTRL_FOTA_INFO_T;

VOID hpm_control_cmd_handle(UINT16 cmd, UINT8 *data, UINT16 len);

VOID hpm_control_get_fota_info(HPM_CTRL_FOTA_INFO_T *fota_info);

VOID hpm_control_reset_fota_info(VOID);

VOID hpm_control_process(VOID);

#endif
