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

#define HPM_PROTOCOL_VERSION 0x13
#define HPM_UNIQUE_CODE_LENGTH 20
#define HPM_DATA_BUFF_LENGTH 512

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

static INT32 hpm_get_imei(UINT8 *buf)
{
    UINT8 imei[IF_4G_MAX_IMEI_LEN] = {0};
    UINT8 len = IF_4G_MAX_IMEI_LEN;
    UINT8 i = 0;

    if_4g_get_imei(imei, &len);
    for (; i < len; i++)
        buf[i] = imei[i] + 48;
    for (; i < 20; i++)
        buf[i] = 0;

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

static INT32 hpm_get_iccid(UINT8 *buf)
{
    UINT8 iccid[IF_4G_MAX_ICCID_LEN] = {0};
    UINT8 len = IF_4G_MAX_ICCID_LEN;

    if_4g_get_iccid(iccid, &len);
    memcpy(buf, iccid, 20);

    return 20;
}

static INT32 hpm_get_vin(UINT8 *buf)
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
    UINT8 *data;
    UINT16 len;

    data = mempool_alloc(HPM_DATA_BUFF_LENGTH);
    if (NULL == data)
    {
        MODULE_LOG_E(HPM, "malloc data buf failed");
        return 0;
    }

    len = 0;
    len += hpm_sesion_get_data_seq(data + len);
    data[len++] = 0x02; // 数据数量
    memcpy(data + len, pack->data, pack->len);
    len += pack->len;
    len = hpm_pack((HPM_CMD_TYPE)pack->type, HPM_COMPRESS_NONE, HPM_ENCRYPT_NONE, len, data, buf);

    mempool_free(data);

    return len;
}

INT32 hpm_pack_common_resp(UINT8 *buf, UINT8 *res, UINT16 ret_len)
{
    UINT8 len = 0;
    len = hpm_pack(HPM_CMD_TBOX_COMMON_ACK, HPM_COMPRESS_NONE, HPM_ENCRYPT_NONE, ret_len, res, buf);
    return len;
}

INT32 hpm_pack_unpack(UINT8 *in, UINT16 inlen, HPM_PACK_FRAME_T *parsebuf, UINT16 *parselen)
{
    HPM_PACK_FRAME_T *pos = NULL;
    UINT8 cs = 0;
    UINT16 i = 0;
    UINT16 datalen = 0;
    INT32 readlen = 0;

    if (NULL == in || NULL == parsebuf || NULL == parselen)
        return 0;

    for (i = 0; i < inlen; i++)
    {
        pos = (HPM_PACK_FRAME_T *)(in + i);
        if (0x4C53 == pos->sof)
        {
            datalen = ((pos->lenh << 8) & 0xFF00) | (pos->lenl & 0x00FF);
            cs = xor_checksum(&pos->ver, 4 + datalen);
            if (cs == *(pos->data + datalen))
            {
                readlen = 28 + datalen; // 除数据域外，固定字节长度为26
                if (readlen <= *parselen)
                {
                    *parselen = readlen;
                }
                memcpy(parsebuf, pos, *parselen);
                break;
            }
        }
    }

    readlen = readlen + i;
    return readlen;
}
