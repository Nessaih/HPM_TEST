#ifndef __HPM_CONTROL_H__
#define __HPM_CONTROL_H__

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
} HPM_CTRL_SUB_TYPE;

VOID hpm_control_cmd_handle(UINT16 cmd, UINT8 *data, UINT16 len);

#endif
