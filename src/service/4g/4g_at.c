#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_modem.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"

uint8 at_4g_common_resp(uint8 cmd, AT_4G_CMD_RESP *resp, uint8 *data, uint16 *len)
{
    UNUSED(data);
    UNUSED(len);

    if(cmd != resp->cmd.cmd_id)
    {
        return AT_4G_DECODE_NOMATCH;
    }

    if(AT_4G_RESP_UNKNOWN == resp->resp_code)
    {
        return AT_4G_DECODE_NOMATCH;
    }
    else if(AT_4G_RESP_OK == resp->resp_code)
    {
        seqmgr_4g_handle_cmd_exe_result(cmd, SEQMGR_4G_CMD_EXE_OK);
        return AT_4G_DECODE_ISMATCH;
    }
     else if(AT_4G_RESP_TIMEOUT == resp->resp_code ||
             AT_4G_RESP_ABORT == resp->resp_code   ||
             (resp->resp_code >= AT_4G_RESP_MIN_ERROR &&
              resp->resp_code <= AT_4G_RESP_MAX_ERROR))
     {
         seqmgr_4g_handle_cmd_exe_result(cmd, SEQMGR_4G_CMD_EXE_FAILED);
         return AT_4G_DECODE_ISMATCH;
     }
     else
     {
         /**/
     }

     return AT_4G_DECODE_NOMATCH;
}

