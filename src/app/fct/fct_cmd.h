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

VOID fct_fctcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize);

VOID fct_eolcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize);

#endif

