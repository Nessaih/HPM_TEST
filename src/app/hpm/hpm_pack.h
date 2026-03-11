#ifndef __HPM_PACK_H__
#define __HPM_PACK_H__

#include "hpm_data.h"
#include "hpm_content.h"

typedef enum
{
    HPM_CMD_INVALID = 0x00,
    HPM_CMD_LOGIN = 0x01,
    HPM_CMD_LIVE_DATA = 0x02,
    HPM_CMD_REISSUE_DATA = 0x03,
    HPM_CMD_LOGOUT = 0x04,
    HPM_CMD_HEART_BEAT = 0x05,
    HPM_CMD_TBOX_COMMON_ACK = 0x06,
    HPM_CMD_TBOX_STATUS = 0x07,
    HPM_CMD_CONTROL = 0x81,
    HPM_CMD_TSP_COMMON_ACK = 0x82,
    HPM_CMD_CONTROL_CAR = 0x83,
    HPM_CMD_UPGRADE_ACK = 0x89,
} HPM_CMD_TYPE;

typedef enum
{
    HPM_PACK_PARSE_OK = 0,
    HPM_PACK_PARSE_NOT_COMPLETE = 1,
    HPM_PACK_PARSE_INVALID_PACKET = 2,
    HPM_PACK_PARSE_INVALID_PARAM = 3,
    HPM_PACK_PARSE_CHECKSUM_ERROR = 4,
    HPM_PACK_PARSE_OVERFLOW = 5
} hpm_pack_parse_result_e;

typedef enum
{
    HPM_PARSE_POS_PERFIX0 = 0,
    HPM_PARSE_POS_PERFIX1 = 1,
    HPM_PARSE_POS_CMD = 2,
    HPM_PARSE_POS_LENH = 25,
    HPM_PARSE_POS_LENL = 26,
    HPM_PARSE_POS_DATA = 27,
} hpm_parse_recv_pos_e;

typedef struct
{
    UINT16 seq;
    UINT8 cmd;
    UINT8 result;
} hpm_pack_common_resp_t;

typedef struct
{
    UINT8 cmd;
    UINT16 len;
    union
    {
        hpm_pack_common_resp_t common_resp;
        UINT8 data[HPM_SESSION_RECV_MEM_SIZE];
    };
} hpm_pack_recv_t;

typedef enum
{
    HPM_RECV_RESP_FAILED = 0,
    HPM_RECV_RESP_SUCCESS = 1,
    HPM_RECV_RESP_MAX
} HPM_RECV_RESP_FLAG;

INT32 hpm_get_imei(UINT8* buf);

INT32 hpm_get_time(UINT8* buf);

INT32 hpm_get_iccid(UINT8* buf);

INT32 hpm_get_vin(UINT8* buf);

UINT8 hpm_get_delay_cmdid(UINT8 cmd);

INT32 hpm_pack_login(UINT8 *buf);

INT32 hpm_pack_logout(UINT8 *buf);

INT32 hpm_pack_heartbeat(UINT8 *buf);

INT32 hpm_pack_report_data(HPM_PACKET *pack, UINT8 *buf);

INT32 hpm_pack_common_resp(UINT8 *buf, UINT8 *res, UINT16 ret_len);

INT32 hpm_pack_unpack(UINT8 *data, UINT16 len, hpm_pack_recv_t *pack, UINT16 *parse_len);

#endif
