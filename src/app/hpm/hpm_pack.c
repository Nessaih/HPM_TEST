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

typedef UINT16 (*hpm_pack_func_t)(UINT8 *buf);

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
    INT32 i = 0;

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
    INT32 len = 0;
    DEV_TIME time;

    time_if_get(&time);
    buf[len++] = time.year;
    buf[len++] = time.month;
    buf[len++] = time.day;
    buf[len++] = time.hour;
    buf[len++] = time.min;
    buf[len++] = time.sec;
    return len;
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

static UINT16 hpm_make_header(HPM_CMD_TYPE cmd, UINT8 *buf)
{
    UINT16 len = 0;

    buf[len++] = HPM_CMD_PREFIX0;
    buf[len++] = HPM_CMD_PREFIX1;
    buf[len++] = cmd;
    len += hpm_get_imei(buf + len);
    buf[len++] = HPM_PROTOCOL_VERSION;
    buf[len++] = (HPM_COMPRESS_NONE << 0) | (HPM_ENCRYPT_NONE << 4);

    return len;
}

static INT32 hpm_pack(HPM_CMD_TYPE cmd, UINT16 datalen, const UINT8 *data, UINT8 *buf)
{
    INT32 len = 0;
    len = hpm_make_header(cmd, buf);

    buf[len++] = (datalen >> 8) & 0xFF;
    buf[len++] = datalen & 0xFF;

    if ((datalen > 0) && (data != NULL))
    {
        memcpy(&buf[len], data, datalen);
        len += datalen;
    }

    UINT8 cs = xor_checksum(&buf[HPM_UNIQUE_CODE_LENGTH + 3], datalen + 4);
    buf[len++] = cs;
    return len;
}

static INT32 hpm_pack_data(HPM_CMD_TYPE cmd, UINT8 *buf, hpm_pack_func_t cb)
{
    INT32 len = 0;
    len = hpm_make_header(cmd, buf);

    UINT16 len_pos = len;
    len += 2;

    UINT16 datalen = 0;
    if (cb != NULL)
    {
        datalen = cb(buf + len);
        len += datalen;
    }

    buf[len_pos] = (datalen >> 8) & 0xFF;
    buf[len_pos + 1] = datalen & 0xFF;

    UINT8 cs = xor_checksum(&buf[HPM_UNIQUE_CODE_LENGTH + 3], datalen + 4);
    buf[len++] = cs;
    return len;
}

static UINT16 hpm_pack_login_data_cb(UINT8 *buf)
{
    UINT16 len = 0;
    len += hpm_get_time(buf + len);
    len += hpm_sesion_get_login_seq(buf + len);
    len += hpm_get_iccid(buf + len);
    len += hpm_get_vin(buf + len);
    return len;
}

INT32 hpm_pack_login(UINT8 *buf)
{
    return hpm_pack_data(HPM_CMD_LOGIN, buf, hpm_pack_login_data_cb);
}

static UINT16 hpm_pack_logout_data_cb(UINT8 *buf)
{
    UINT16 len = 0;
    len += hpm_get_time(buf + len);
    len += hpm_session_get_logout_seq(buf + len);
    return len;
}

INT32 hpm_pack_logout(UINT8 *buf)
{
    return hpm_pack_data(HPM_CMD_LOGOUT, buf, hpm_pack_logout_data_cb);
}

INT32 hpm_pack_heartbeat(UINT8 *buf)
{
    INT32 len = 0;
    len = hpm_pack(HPM_CMD_HEART_BEAT, 0, NULL, buf);
    return len;
}

INT32 hpm_pack_report_data(HPM_PACKET *pack, UINT8 *buf)
{
    return hpm_pack((HPM_CMD_TYPE)pack->type, pack->len, pack->data, buf);
}

INT32 hpm_pack_common_resp(UINT8 *buf, UINT8 *res, UINT16 ret_len)
{
    INT32 len = 0;
    len = hpm_pack(HPM_CMD_TBOX_COMMON_ACK, ret_len, res, buf);
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