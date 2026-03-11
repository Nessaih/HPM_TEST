#include "tbox_common.h"
#include "tbox_core.h"
#include "time_if.h"
#include "4g_if.h"
#include "checksum.h"
#include "tbox_cfg_if.h"

#include "hpm_content.h"
#include "hpm_session.h"
#include "hpm_pack.h"
#include "hpm_cfg.h"

#define HPM_PROTOCOL_VERSION 0x14
#define HPM_UNIQUE_CODE_LENGTH 20
#define HPM_DATA_BUFF_LENGTH 512

#define HPM_CMD_PREFIX0 (0x53)
#define HPM_CMD_PREFIX1 (0x4C)
#define HPM_CHECKSUM_POS_START (23UL)

typedef enum
{
    HPM_COMPRESS_NONE = 0,
    HPM_COMPRESS_GZIP = 1,
} HPM_COMPRESS_E;

typedef enum
{
    HPM_ENCRYPT_NONE = 0,
    HPM_ENCRYPT_RSA = 1,
    HPM_ENCRYPT_SM2 = 1,
} HPM_ENCRYPT_E;

typedef enum
{
    HPM_PARSE_STEP_PERFIX0 = 0,
    HPM_PARSE_STEP_PERFIX1 = 1,
    HPM_PARSE_STEP_CMD = 2,
    HPM_PARSE_STEP_LEN = 3,
    HPM_PARSE_STEP_DATA = 4,
    HPM_PARSE_STEP_CHECKSUM = 5,
} hpm_parse_recv_step_e;

INT32 hpm_get_imei(UINT8 *buf)
{
    UINT8 imei[IF_4G_MAX_IMEI_LEN] = {0};
    UINT8 len = IF_4G_MAX_IMEI_LEN;
    UINT8 i = 0;

    if_4g_get_imei(imei, &len);
    for (; i < len; i++)
    {
        buf[i] = imei[i] + 48;
    }

    for (; i < 20; i++)
    {
        buf[i] = 0;
    }

    return i;
}

INT32 hpm_get_time(UINT8 *buf)
{
    DEV_TIME time;

    time_if_get(&time);
    buf[0] = time.year;
    buf[1] = time.month;
    buf[2] = time.day;
    buf[3] = time.hour;
    buf[4] = time.min;
    buf[5] = time.sec;
    return 6;
}

INT32 hpm_get_iccid(UINT8 *buf)
{
    UINT8 iccid[IF_4G_MAX_ICCID_LEN] = {0};
    UINT8 len = IF_4G_MAX_ICCID_LEN;

    if_4g_get_iccid(iccid, &len);
    memcpy(buf, iccid, 20);

    return 20;
}

INT32 hpm_get_vin(UINT8 *buf)
{
    INT32 ret = 0;
    UINT8 vin[TBOX_CFG_VIN_LEN] = {0};
    UINT8 len = TBOX_CFG_VIN_LEN;

    ret = hpm_cfg_get_vin(vin, len);
    if (0 == ret)
    {
        memcpy(buf, vin, 17);
    }

    return (TBOX_CFG_VIN_LEN - 1);
}

UINT8 hpm_get_delay_cmdid(UINT8 cmd)
{
    UINT8 delay = 0;
    switch (cmd)
    {
    case HPM_CMD_LIVE_DATA:
        delay = HPM_CMD_REISSUE_DATA;
        break;
    case HPM_CMD_REISSUE_DATA:
        delay = HPM_CMD_REISSUE_DATA;
        break;
    case HPM_CMD_TBOX_STATUS:
        delay = HPM_CMD_TBOX_STATUS;
        break;
    case HPM_CMD_TBOX_COMMON_ACK:
        delay = HPM_CMD_TBOX_COMMON_ACK;
        break;
    default:
        break;
    }
    return delay;
}

static INT32 hpm_pack(HPM_CMD_TYPE cmd, HPM_COMPRESS_E cps, HPM_ENCRYPT_E ecp, UINT16 datalen, UINT8 *data, UINT8 *buf)
{
    UINT16 len = 0;
    UINT8 cs;

    buf[len++] = 0x53;
    buf[len++] = 0x4C;
    buf[len++] = cmd;
    len += hpm_get_imei(buf + len);
    buf[len++] = HPM_PROTOCOL_VERSION;
    buf[len++] = (ecp << 0) | (cps << 4);
    buf[len++] = (datalen >> 8) & 0xFF;
    buf[len++] = (datalen >> 0) & 0xFF;

    memcpy(&buf[len], data, datalen);
    len += datalen;
    cs = xor_checksum(&buf[HPM_UNIQUE_CODE_LENGTH + 3], datalen + 4);
    buf[len++] = cs;
    return len;
}

INT32 hpm_pack_login(UINT8 *buf)
{
    UINT8 *data = NULL;
    INT32 len = 0;

    data = mempool_alloc(HPM_PACK_HEAD_MEM_SIZE);
    if (NULL == data)
    {
        MODULE_LOG_E(HPM, "memalloc pack head buf failed");
        return -1;
    }

    len = 0;
    len += hpm_get_time(data + len);
    len += hpm_sesion_get_login_seq(data + len);
    len += hpm_get_iccid(data + len);
    len += hpm_get_vin(data + len);
    len = hpm_pack(HPM_CMD_LOGIN, HPM_COMPRESS_NONE, HPM_ENCRYPT_NONE, len, data, buf);

    mempool_free(data);
    return len;
}

INT32 hpm_pack_logout(UINT8 *buf)
{
    UINT8 *data;
    UINT8 len;

    data = mempool_alloc(HPM_PACK_HEAD_MEM_SIZE);
    if (NULL == data)
    {
        MODULE_LOG_E(HPM, "memalloc pack head buf failed");
        return -1;
    }

    len = 0;
    len += hpm_get_time(data + len);
    len += hpm_session_get_logout_seq(data + len);
    len = hpm_pack(HPM_CMD_LOGOUT, HPM_COMPRESS_NONE, HPM_ENCRYPT_NONE, len, data, buf);

    mempool_free(data);
    return len;
}

INT32 hpm_pack_heartbeat(UINT8 *buf)
{
    UINT8 len = 0;
    len = hpm_pack(HPM_CMD_HEART_BEAT, HPM_COMPRESS_NONE, HPM_ENCRYPT_NONE, 0, NULL, buf);
    return len;
}

INT32 hpm_pack_report_data(HPM_PACKET *pack, UINT8 *buf)
{
    UINT16 len = 0;
    buf[len++] = 0x53;
    buf[len++] = 0x4C;
    buf[len++] = pack->type;
    len += hpm_get_imei(buf + len);
    buf[len++] = HPM_PROTOCOL_VERSION;
    buf[len++] = (HPM_COMPRESS_NONE << 0) | (HPM_ENCRYPT_NONE << 4);
    buf[len++] = (pack->len >> 8) & 0xFF;
    buf[len++] = (pack->len >> 0) & 0xFF;
    memcpy(buf + len, pack->data, pack->len);
    len += pack->len;
    UINT8 cs = xor_checksum(&buf[HPM_UNIQUE_CODE_LENGTH + 3], pack->len + 4);
    buf[len++] = cs;
    return len;
}

INT32 hpm_pack_common_resp(UINT8 *buf, UINT8 *res, UINT16 ret_len)
{
    UINT8 len = 0;
    len = hpm_pack(HPM_CMD_TBOX_COMMON_ACK, HPM_COMPRESS_NONE, HPM_ENCRYPT_NONE, ret_len, res, buf);
    return len;
}

INT32 hpm_pack_unpack(UINT8 *data, UINT16 len, hpm_pack_recv_t *pack, UINT16 *parse_len)
{
    if ((NULL_PTR == data) || (NULL_PTR == pack) ||
        (NULL_PTR == parse_len) || (0 == len))
    {
        MODULE_LOG_E(HPM, "invalid param");
        return HPM_PACK_PARSE_INVALID_PARAM;
    }

    hpm_parse_recv_step_e step = HPM_PARSE_STEP_PERFIX0;
    UINT16 body_len = 0;

    for (UINT16 pos = 0; pos < len; pos++)
    {
        switch (step)
        {
        case HPM_PARSE_STEP_PERFIX0:
            if (HPM_PARSE_POS_PERFIX0 == pos)
            {
                if (HPM_CMD_PREFIX0 == data[pos])
                {
                    step = HPM_PARSE_STEP_PERFIX1;
                }
                else
                {
                    *parse_len = 0;
                    return HPM_PACK_PARSE_INVALID_PACKET;
                }
            }
            break;
        case HPM_PARSE_STEP_PERFIX1:
            if (HPM_PARSE_POS_PERFIX1 == pos)
            {
                if (HPM_CMD_PREFIX1 == data[pos])
                {
                    step = HPM_PARSE_STEP_CMD;
                }
                else
                {
                    *parse_len = 0;
                    return HPM_PACK_PARSE_INVALID_PACKET;
                }
            }
            break;
        case HPM_PARSE_STEP_CMD:
            if (HPM_PARSE_POS_CMD == pos)
            {
                pack->cmd = data[pos];
                step = HPM_PARSE_STEP_LEN;
            }
            break;
        case HPM_PARSE_STEP_LEN:
            if (HPM_PARSE_POS_LENH == pos)
            {
                pack->len = (UINT16)(data[pos] << 8) & 0xFF00;
            }
            else if (HPM_PARSE_POS_LENL == pos)
            {
                pack->len |= (UINT16)(data[pos] << 0);
                if (pack->len >= HPM_DATA_BUFF_LENGTH)
                {
                    *parse_len = 0;
                    return HPM_PACK_PARSE_OVERFLOW;
                }
                step = HPM_PARSE_STEP_DATA;
            }
            break;
        case HPM_PARSE_STEP_DATA:
            pack->data[body_len++] = data[pos];
            if (body_len == pack->len)
            {
                step = HPM_PARSE_STEP_CHECKSUM;
            }
            break;
        case HPM_PARSE_STEP_CHECKSUM:
        {
            UINT8 cs = xor_checksum(&data[HPM_CHECKSUM_POS_START], body_len + 4);
            if (cs != data[pos])
            {
                *parse_len = 0;
                return HPM_PACK_PARSE_INVALID_PACKET;
            }
            else
            {
                *parse_len = pos + 1;
                return HPM_PACK_PARSE_OK;
            }
            break;
        }
        default:
            *parse_len = 0;
            return HPM_PACK_PARSE_INVALID_PACKET;
        }
    }

    *parse_len = 0;
    return HPM_PACK_PARSE_NOT_COMPLETE;
}