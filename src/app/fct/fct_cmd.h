#ifndef __FCT_CMD_H__
#define __FCT_CMD_H__

typedef enum
{
    FCT_PM_ACTION_LISTEN = 0,
    FCT_PM_ACTION_SLEEP,
    FCT_PM_ACTION_CAN2,
    FCT_PM_ACTION_INVALID = 0xFF,
} FCT_PM_ACTION;

VOID fct_cmd_init(VOID);

VOID fct_cmd_timeout(VOID);

VOID fct_fctcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize);

VOID fct_eolcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize);

#endif

