#ifndef __FCT_CMD_H__
#define __FCT_CMD_H__

typedef enum
{
    FCT_PM_ACTION_IDLE		= 0x00,
    FCT_PM_ACTION_LISTEN	= 0x01,
    FCT_PM_ACTION_SLEEP		= 0x02,
    FCT_PM_ACTION_CAN2		= 0x03,
} FCT_PM_ACTION_E;

INT32 fct_cmd_init(UINT8 seq);

VOID fct_cmd_timeout(VOID);

TBOX_ADSP_MATCH_RESULT fct_cmd_is_match(UINT8 *data, UINT32 len);

TBOX_ADSP_PROCESS_RESULT fct_cmd_callback(UINT8 *data, UINT32 len);

BOOL fct_cmd_is_exit(UINT8 *data, UINT32 len);

#endif

