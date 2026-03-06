#include "tbox_common.h"
#include "tbox_core.h"
#include "time_if.h"
#include "4g_if.h"
#include "tbox_cfg_if.h"
#include "flash_common.h"
#include "drv_flash_mcu.h"
#include "hpm_param_parse.h"
#include "hpm_param_fetch.h"
#include "tbox_config.h"
#include "hpm_can.h"
#include "j1939_if.h"
#include "md5.h"

#define HPM_PARAM_FETCH_NAME "HPM_PARAM_FETCH_CONFIG"
#define HPM_PARAM_MAGIC_NUM (0x43464546)
#define HPM_FECTCH_DOWNLOAD_TIMEOUT (60U) // 60s
#define HPM_FECTCH_DOWNLOAD_DELAY (3U)    // 3s
#define HPM_FETCH_FLASH_READ_SIZE (256UL)
#define HPM_FETCH_FLASH_PARSE_SIZE (512UL)

#define HPM_PARAM_LOCK() xSemaphoreTake(hpm_param_task_mutex, portMAX_DELAY)
#define HPM_PARAM_UNLOCK() xSemaphoreGive(hpm_param_task_mutex)

#define HPM_PARAM_FTP_MIN_SIZE (22UL)
#define HPM_PARAM_FTP_SIZE_MAX (128UL)

typedef enum
{
    HPM_FETCH_FTP_INIT = 0,
    HPM_FETCH_FTP_START = 1,
    HPM_FETCH_FTP_DOWNLOAD = 2,
    HPM_FETCH_FTP_FINISH = 3
} hpm_fetch_ftp_e;

typedef struct
{
    UINT32 file_size : 16;
    UINT32 download_size : 16;
    UINT32 state : 4;
    UINT32 timeout : 12;
    UINT32 url_len : 16;
    UINT32 sequence;
    UINT8 md5[16];
    UINT8 *url;
} hpm_fetch_ftp_t;

typedef struct
{
    hpm_fetch_node_t *node;
    UINT8 data[HPM_FECTCH_SINGLE_DATA_LEN];
} hpm_fetch_can_single_t;

typedef struct
{
    hpm_fetch_node_t *node;
    UINT8 data[HPM_FECTCH_MULTI_DATA_LEN];
} hpm_fetch_can_multi_t;

static SemaphoreHandle_t hpm_param_task_mutex;
static hpm_fetch_ftp_t hpm_fetch_ftp;
static hpm_fetch_config_t hpm_fetch_config;

static hpm_fetch_can_single_t hpm_fetch_can_single[HPM_FETCH_SINGLE_COUNT];
static hpm_fetch_can_multi_t hpm_fetch_can_multi[HPM_FETCH_MULTI_COUNT];

static BOOL hpm_fetch_register_node(VOID);
static VOID hpm_fetch_unregister_all(VOID);
static INT32 hpm_fetch_save_config(VOID);
static INT32 hpm_fetch_load_config(VOID);
static VOID hpm_fetch_set_param(hpm_fetch_config_t *config);

static INT32 hpm_ftech_parse_file(const UINT8 *data, UINT16 len, hpm_fetch_parse_t *parse, hpm_fetch_config_t *config)
{
    if ((NULL_PTR == data) || (0 == len) || (NULL_PTR == parse) || (NULL_PTR == config))
    {
        MODULE_LOG_E(HPM, "invalid parameter");
        return -1;
    }

    if (parse->pos + len >= HPM_FETCH_FLASH_PARSE_SIZE)
    {
        MODULE_LOG_E(HPM, "parse buffer overflow");
        return -1;
    }

    char line[64] = {0};

    memcpy(parse->data + parse->pos, data, len);
    parse->len += len;

    const char *base = (const char *)parse->data; /* start of buffer */
    const char *p = base + parse->pos;
    const char *end = base + parse->len;
    while (p < end)
    {
        if (NULL == strstr(p, "\n"))
        {
            break;
        }

        hpm_param_skip_blank_line(&p);

        hpm_param_get_line(&p, line, sizeof(line));

        hpm_fetch_line_type_e type = hpm_param_get_line_type(line);

        switch (parse->status)
        {
        case HPM_FETCH_PARSE_STATUS_INIT:
            if (HPM_FETCH_LINE_INVALID != type)
            {
                parse->status = HPM_FETCH_PARSE_STATUS_LINE;
                parse->type = type;
            }
            break;
        case HPM_FETCH_PARSE_STATUS_LINE:
            if (HPM_FETCH_LINE_INVALID == type)
            {
                hpm_param_parse_config(config, line, parse->type);
            }
            else
            {
                parse->type = type;
            }
            break;
        default:
            break;
        }
    }

    UINT32 consumed = (UINT32)(p - base);
    if (consumed > parse->len)
    {
        consumed = parse->len; /* should not happen, but be safe */
    }

    UINT32 remain = parse->len - consumed;
    if (remain > 0)
    {
        memmove(parse->data, parse->data + consumed, remain);
    }

    parse->pos = remain;
    parse->len = remain;

    return 0;
}

static INT32 hpm_fetch_ftp_check_file(VOID)
{
    INT32 ret = 0;

    UINT8 *data = mempool_alloc(HPM_FETCH_FLASH_READ_SIZE);
    if (NULL_PTR == data)
    {
        MODULE_LOG_E(HPM, "alloc memory failed");
        return -1;
    }

    UINT8 *parse_buf = mempool_alloc(HPM_FETCH_FLASH_PARSE_SIZE);
    if (NULL_PTR == parse_buf)
    {
        MODULE_LOG_E(HPM, "alloc memory failed");
        mempool_free(data);
        return -1;
    }

    hpm_fetch_unregister_all();
    memset(&hpm_fetch_config, 0, sizeof(hpm_fetch_config));

    MD5_CTX ctx;
    MD5Init(&ctx);

    hpm_fetch_parse_t hpm_fetch_parse;
    memset(&hpm_fetch_parse, 0, sizeof(hpm_fetch_parse));
    hpm_fetch_parse.data = parse_buf;

    UINT32 addr = FLASH_MCU_ADD_HPM_CFG;
    UINT16 read_len = 0;
    while (read_len < hpm_fetch_ftp.file_size)
    {
        UINT16 len = hpm_fetch_ftp.file_size - read_len;
        if (len > HPM_FETCH_FLASH_READ_SIZE)
        {
            len = HPM_FETCH_FLASH_READ_SIZE;
        }
        drv_flash_mcu_read(addr, data, len);

        if (0 != hpm_ftech_parse_file(data, len, &hpm_fetch_parse, &hpm_fetch_config))
        {
            break;
        }

        MODULE_LOG_DUMP(HPM, "config data", data, len);

        MD5Update(&ctx, data, len);
        read_len += len;
        addr += len;
        drv_wdg_feed();
    }

    if (read_len == hpm_fetch_ftp.file_size)
    {
        UINT8 md5[16] = {0};
        MD5Final(&ctx, md5);

        if (0 != memcmp(md5, hpm_fetch_ftp.md5, sizeof(hpm_fetch_ftp.md5)))
        {
            MODULE_LOG_DUMP(HPM, "file md5", md5, sizeof(md5));
            MODULE_LOG_DUMP(HPM, "expected md5", hpm_fetch_ftp.md5, sizeof(hpm_fetch_ftp.md5));
            ret = 0;
        }
    }
    else
    {
        MODULE_LOG_E(HPM, "parse file error, read len %d, file size %d.", read_len, hpm_fetch_ftp.file_size);
        ret = -1;
    }

    mempool_free(data);
    mempool_free(parse_buf);

    return ret;
}

static VOID hpm_fetch_ftp_init(VOID)
{
    memset(&hpm_fetch_ftp, 0, sizeof(hpm_fetch_ftp));
    hpm_fetch_ftp.url = NULL_PTR;
}

static VOID hpm_fetch_ftp_state_set(hpm_fetch_ftp_e state)
{
    HPM_PARAM_LOCK();
    hpm_fetch_ftp.state = state;
    HPM_PARAM_UNLOCK();
}

static UINT32 hpm_fetch_ftp_state_get(VOID)
{
    UINT32 state = HPM_FETCH_FTP_INIT;
    HPM_PARAM_LOCK();
    state = hpm_fetch_ftp.state;
    HPM_PARAM_UNLOCK();
    return state;
}

static VOID hpm_fetch_erase_flash(VOID)
{
    UINT32 addr = FLASH_MCU_ADD_HPM_CFG;
    while (addr < FLASH_MCU_ADD_HPM_CFG + FLASH_MCU_SIZE_HPM_CFG)
    {
        drv_flash_mcu_erase(addr, FLASH_MCU_SIZE_FOTA_PAGE);
        addr += FLASH_MCU_SIZE_FOTA_PAGE;
        drv_wdg_feed();
    }
}

static INT32 hpm_fetch_ftp_write_flash(UINT32 addr, UINT8 *data, UINT16 len)
{
    if ((addr - FLASH_MCU_ADD_HPM_CFG + len) > (UINT32)FLASH_MCU_SIZE_HPM_CFG)
    {
        MODULE_LOG_E(HPM, "addr %x, len %d, overflow", addr, len);
        return -1;
    }

    for (UINT8 i = 0; i < 3; i++)
    {
        if (0 == drv_flash_mcu_write(addr, data, len))
        {
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(100U));
    }

    return -1;
}

static UINT8 hpm_fetch_ftp_callback(UINT8 notify, UINT8 *data, UINT16 len)
{
    if (HPM_FETCH_FTP_DOWNLOAD != hpm_fetch_ftp_state_get())
    {
        MODULE_LOG_E(HPM, "download error, state %d.", hpm_fetch_ftp_state_get());
        return IF_FTP_4G_CALLBACK_RET_ABORT;
    }

    switch (notify)
    {
    case IF_FTP_4G_NOTIFY_PROCESS:
        if (time_if_get_systick_s() - hpm_fetch_ftp.timeout > HPM_FECTCH_DOWNLOAD_TIMEOUT)
        {
            MODULE_LOG_E(HPM, "download timeout");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }

        if (0 != hpm_fetch_ftp_write_flash(hpm_fetch_ftp.download_size + FLASH_MCU_ADD_HPM_CFG, data, len))
        {
            MODULE_LOG_E(HPM, "write flash failed");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }
        hpm_fetch_ftp.download_size += len;

        if (hpm_fetch_ftp.download_size > hpm_fetch_ftp.file_size)
        {
            MODULE_LOG_E(HPM, "download error, data overflow.");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }
        break;
    case IF_FTP_4G_NOTIFY_ERROR:
        MODULE_LOG_E(HPM, "download error, code %hu.", len);
        return IF_FTP_4G_CALLBACK_RET_ABORT;
        break;
    case IF_FTP_4G_NOTIFY_FINISH:
        if (hpm_fetch_ftp.download_size != hpm_fetch_ftp.file_size)
        {
            MODULE_LOG_E(HPM, "download error, data not match.");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }
        else
        {
            MODULE_LOG_E(HPM, "download finish");
            hpm_fetch_ftp_state_set(HPM_FETCH_FTP_FINISH);
        }
        break;
    default:
        break;
    }
    return IF_FTP_4G_CALLBACK_RET_OK;
}

static VOID hpm_fetch_ftp_handle_init(VOID)
{
    /*do nothing*/
}

static VOID hpm_fetch_ftp_handle_start(VOID)
{
    if ((NULL_PTR == hpm_fetch_ftp.url) || (0 == hpm_fetch_ftp.url_len))
    {
        hpm_fetch_ftp_state_set(HPM_FETCH_FTP_INIT);
        return;
    }

    if (time_if_get_systick_s() - hpm_fetch_ftp.timeout < HPM_FECTCH_DOWNLOAD_DELAY)
    {
        return;
    }

    MODULE_LOG_E(HPM, "can config start.");

    hpm_fetch_erase_flash();
    if_ftp_4g_download(hpm_fetch_ftp.url, hpm_fetch_ftp.url_len, IF_4G_PUBLIC_APN, hpm_fetch_ftp_callback);
    hpm_fetch_ftp_state_set(HPM_FETCH_FTP_DOWNLOAD);
    hpm_fetch_ftp.timeout = time_if_get_systick_s();
    if (NULL_PTR != hpm_fetch_ftp.url)
    {
        mempool_free(hpm_fetch_ftp.url);
        hpm_fetch_ftp.url = NULL_PTR;
    }
}

static VOID hpm_fetch_ftp_handle_download(VOID)
{
    if (time_if_get_systick_s() - hpm_fetch_ftp.timeout >= HPM_FECTCH_DOWNLOAD_TIMEOUT)
    {
        MODULE_LOG_E(HPM, "download timeout");
        hpm_fetch_ftp_state_set(HPM_FETCH_FTP_INIT);
        return;
    }
}

static VOID hpm_fetch_ftp_handle_finish(VOID)
{
    if (0 != hpm_fetch_ftp_check_file())
    {
        MODULE_LOG_E(HPM, "file parse error.");
        hpm_fetch_unregister_all();
        hpm_fetch_load_config();
        hpm_fetch_register_node();
    }
    else
    {
        hpm_fetch_save_config();
        hpm_fetch_unregister_all();
        hpm_fetch_load_config();
        hpm_fetch_register_node();
        hpm_fetch_set_param(&hpm_fetch_config);
    }
    hpm_fetch_ftp_state_set(HPM_FETCH_FTP_INIT);
}

INT32 hpm_param_fetch_ftp_start(UINT8 *data, UINT16 len, UINT8 *resp, UINT16 *resp_len)
{
    if ((NULL_PTR == resp) || (NULL_PTR == resp_len) || (NULL_PTR == data))
    {
        MODULE_LOG_E(HPM, "invalid param.");
        return -1;
    }

    UINT16 out_len = 0;
    resp[out_len++] = 0x00;
    resp[out_len++] = 0x00;
    *resp_len = out_len;

    if (HPM_FETCH_FTP_INIT != hpm_fetch_ftp_state_get())
    {
        MODULE_LOG_E(HPM, "ftp state error, state %d.", hpm_fetch_ftp_state_get());
        return -1;
    }

    if ((len < HPM_PARAM_FTP_MIN_SIZE) ||
        (len > HPM_PARAM_FTP_MIN_SIZE + HPM_PARAM_FTP_SIZE_MAX))
    {
        MODULE_LOG_E(HPM, "data len error, len %d.", len);
        return -1;
    }

    UINT16 pos = 0;
    memcpy((UINT8 *)&hpm_fetch_ftp.sequence, data + pos, sizeof(hpm_fetch_ftp.sequence));
    pos += sizeof(hpm_fetch_ftp.sequence);

    hpm_fetch_ftp.url = mempool_alloc(HPM_PARAM_FTP_SIZE_MAX);
    if (NULL_PTR == hpm_fetch_ftp.url)
    {
        MODULE_LOG_E(HPM, "alloc memory failed for url");
        return -1;
    }

    for (UINT16 i = 0; (i < HPM_PARAM_FTP_SIZE_MAX) && (pos < len); i++)
    {
        hpm_fetch_ftp.url[i] = data[pos++];
        hpm_fetch_ftp.url_len++;
        if ('\0' == hpm_fetch_ftp.url[i])
        {
            break;
        }
    }

    if (pos + sizeof(UINT16) > len)
    {
        MODULE_LOG_E(HPM, "file size len error, len %d.", len);
        mempool_free(hpm_fetch_ftp.url);
        hpm_fetch_ftp.url = NULL_PTR;
        return -1;
    }
    else
    {
        hpm_fetch_ftp.file_size = (UINT32)(data[pos++] << 8) & 0xFF00;
        hpm_fetch_ftp.file_size |= (UINT32)(data[pos++] & 0x00FF);
    }

    if (pos + sizeof(hpm_fetch_ftp.md5) != len)
    {
        MODULE_LOG_E(HPM, "md5 len error, len %d.", pos);
        mempool_free(hpm_fetch_ftp.url);
        hpm_fetch_ftp.url = NULL_PTR;
        return -1;
    }

    memcpy(hpm_fetch_ftp.md5, data + pos, sizeof(hpm_fetch_ftp.md5));

    MODULE_LOG_E(HPM, "ftp start, url %s, file size %d.", hpm_fetch_ftp.url, hpm_fetch_ftp.file_size);

    hpm_fetch_ftp_state_set(HPM_FETCH_FTP_START);

    hpm_fetch_ftp.timeout = time_if_get_systick_s();

    return 0;
}

static VOID hpm_fetch_ftp_handle_process(VOID)
{
    hpm_fetch_ftp_e state = (hpm_fetch_ftp_e)hpm_fetch_ftp_state_get();
    switch (state)
    {
    case HPM_FETCH_FTP_INIT:
        hpm_fetch_ftp_handle_init();
        break;
    case HPM_FETCH_FTP_START:
        hpm_fetch_ftp_handle_start();
        break;
    case HPM_FETCH_FTP_DOWNLOAD:
        hpm_fetch_ftp_handle_download();
        break;
    case HPM_FETCH_FTP_FINISH:
        hpm_fetch_ftp_handle_finish();
        break;
    default:
        break;
    }
}

static INT32 hpm_fetch_save_config(VOID)
{
    UINT32 addr = (UINT32)FLASH_MCU_ADDR_HPM_NODE;
    hpm_fetch_config.magic_num = HPM_PARAM_MAGIC_NUM;
    drv_flash_mcu_erase(addr, sizeof(hpm_fetch_config));
    if (0 != drv_flash_mcu_write(addr, (UINT8 *)&hpm_fetch_config, sizeof(hpm_fetch_config)))
    {
        MODULE_LOG_E(HPM, "save config error.");
        return -1;
    }
    return 0;
}

static INT32 hpm_fetch_load_config(VOID)
{
    UINT32 addr = (UINT32)FLASH_MCU_ADDR_HPM_NODE;
    memset(&hpm_fetch_config, 0, sizeof(hpm_fetch_config));
    drv_flash_mcu_read(addr, (UINT8 *)&hpm_fetch_config, sizeof(hpm_fetch_config));
    if (HPM_PARAM_MAGIC_NUM != hpm_fetch_config.magic_num)
    {
        memset(&hpm_fetch_config, 0, sizeof(hpm_fetch_config));
        hpm_fetch_save_config();
    }
    return 0;
}

static VOID hpm_fetch_can_data_clear(VOID)
{
    for (UINT16 i = 0; i < HPM_FETCH_NODE_COUNT; i++)
    {
        hpm_fetch_node_t *node = &hpm_fetch_config.node[i];
        HPM_PARAM_LOCK();
        node->obtained = (UINT32)HPM_FETCH_STATUS_UNOBTAINED;
        node->len = 0;
        HPM_PARAM_UNLOCK();
    }
}

static VOID hpm_fetch_handle_single_msg(const can_msg_t *msg)
{
    HPM_PARAM_LOCK();
    for (UINT32 i = 0; i < HPM_FETCH_SINGLE_COUNT; i++)
    {
        hpm_fetch_can_single_t *p = &hpm_fetch_can_single[i];
        if (NULL_PTR == p->node)
        {
            continue;
        }
        if ((p->node->canid == (msg->id & 0x7FFFFFFF)) &&
            (p->node->type == (UINT32)HPM_FETCH_NODE_SINGLE) &&
            ((UINT8)p->node->channel == msg->ins))
        {
            memcpy(p->data, msg->data, HPM_FECTCH_SINGLE_DATA_LEN);
            p->node->obtained = (UINT32)HPM_FETCH_STATUS_OBTAINED;
        }
    }
    HPM_PARAM_UNLOCK();
}

static BOOL hpm_fetch_regiseter_single(hpm_fetch_node_t *node)
{
    for (UINT32 i = 0; i < HPM_FETCH_SINGLE_COUNT; i++)
    {
        hpm_fetch_can_single_t *p = &hpm_fetch_can_single[i];
        if (NULL_PTR == p->node)
        {
            p->node = node;
            hpm_fetch_config.registered |= (1U << HPM_FETCH_NODE_SINGLE);
            hpm_fetch_config.count++;
            return TRUE;
        }
    }
    return FALSE;
}

static hpm_fetch_can_multi_t *hpm_fetch_can_consecutive_get(const can_msg_t *msg)
{
    for (UINT32 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        if (NULL_PTR == p->node)
        {
            continue;
        }
        if ((p->node->canid == (msg->id & 0x7FFFFFFF)) &&
            (p->node->type == (UINT32)HPM_FETCH_NODE_CONSECTIVE) &&
            (p->node->channel == msg->ins))
        {
            return p;
        }
    }
    return NULL_PTR;
}

static BOOL hpm_fetch_regiseter_consecutive(hpm_fetch_node_t *node)
{
    for (UINT32 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        if (NULL_PTR == hpm_fetch_can_multi[i].node)
        {
            hpm_fetch_can_multi[i].node = node;
            hpm_fetch_config.registered |= (1U << HPM_FETCH_NODE_CONSECTIVE);
            hpm_fetch_config.count++;
            return TRUE;
        }
    }
    return FALSE;
}

static VOID hpm_fetch_handle_consecutive_msg(const can_msg_t *msg)
{
    hpm_fetch_can_multi_t *p = hpm_fetch_can_consecutive_get(msg);
    if ((NULL_PTR == p) || (NULL_PTR == p->node))
    {
        return;
    }

    UINT8 seq = msg->data[p->node->offset];
    UINT8 insert_pos = 0;
    for (UINT8 i = 0; i < HPM_FECTCH_MULTI_DATA_LEN; i += HPM_FECTCH_SINGLE_DATA_LEN)
    {
        UINT8 cur_seq = msg->data[p->node->offset + i];
        if (cur_seq == seq)
        {
            HPM_PARAM_LOCK();
            memcpy(p->data + i, msg->data + p->node->offset + 1, HPM_FECTCH_SINGLE_DATA_LEN - 1);
            p->node->obtained = (UINT32)HPM_FETCH_STATUS_OBTAINED;
            HPM_PARAM_UNLOCK();
            return;
        }
        else if (cur_seq > seq)
        {
            insert_pos = i;
            break;
        }
        insert_pos = i + HPM_FECTCH_SINGLE_DATA_LEN;
    }

    if (insert_pos < p->node->len)
    {
        HPM_PARAM_LOCK();
        if (p->node->len + HPM_FECTCH_SINGLE_DATA_LEN <= HPM_FECTCH_MULTI_DATA_LEN)
        {
            memmove(p->data + insert_pos + HPM_FECTCH_SINGLE_DATA_LEN,
                    p->data + insert_pos, p->node->len - insert_pos);
            memcpy(p->data + insert_pos, msg->data, HPM_FECTCH_SINGLE_DATA_LEN);
            p->node->len += HPM_FECTCH_SINGLE_DATA_LEN;
        }
        HPM_PARAM_UNLOCK();
    }
    else
    {
        HPM_PARAM_LOCK();
        if (p->node->len + HPM_FECTCH_SINGLE_DATA_LEN <= HPM_FECTCH_MULTI_DATA_LEN)
        {
            memcpy(p->data + p->node->len, msg->data, HPM_FECTCH_SINGLE_DATA_LEN);
            p->node->len += HPM_FECTCH_SINGLE_DATA_LEN;
        }
        HPM_PARAM_UNLOCK();
    }
}

static VOID hpm_fetch_j1939_pgn_callback(UINT8 *msg, UINT16 len, UINT8 src_addr, UINT32 pgn)
{
    if (NULL_PTR == msg || 0 == pgn)
    {
        MODULE_LOG_E(HPM, "invalid parameter");
        return;
    }

    if (len > HPM_FECTCH_MULTI_DATA_LEN)
    {
        MODULE_LOG_E(HPM, "data len %d for pgn 0x%X overflow.", len, pgn);
        return;
    }

    for (UINT8 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        if (NULL_PTR == p->node)
        {
            continue;
        }
        if ((p->node->pgn == pgn) &&
            (p->node->sa == src_addr) &&
            ((p->node->type == (UINT32)HPM_FETCH_NODE_MULTI ||
              p->node->type == (UINT32)HPM_FETCH_NODE_PGN)))
        {
            HPM_PARAM_LOCK();
            memcpy(p->data, msg, len);
            p->node->len = (UINT32)len;
            p->node->obtained = (UINT32)HPM_FETCH_STATUS_OBTAINED;
            HPM_PARAM_UNLOCK();
        }
    }
}

static BOOL hpm_fetch_regiseter_multi(hpm_fetch_node_t *node)
{
    for (UINT32 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        if (NULL_PTR == p->node)
        {
            p->node = node;
            hpm_fetch_config.registered |= (1U << HPM_FETCH_NODE_MULTI);
            hpm_fetch_config.count++;
            j1939_al_subscribe(node->sa, node->pgn, hpm_fetch_j1939_pgn_callback);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL hpm_fetch_regiseter_pgn(hpm_fetch_node_t *node)
{
    for (UINT32 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        if (NULL_PTR == p->node)
        {
            p->node = node;
            hpm_fetch_config.registered |= (1U << HPM_FETCH_NODE_PGN);
            hpm_fetch_config.count++;
            j1939_pgn_req(node->channel, node->pgn, node->ta, node->sa);
            j1939_al_subscribe(node->sa, node->pgn, hpm_fetch_j1939_pgn_callback);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL hpm_fetch_regiseter_uds(hpm_fetch_node_t *node)
{
    for (UINT32 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        if (NULL_PTR == p->node)
        {
            p->node = node;
            hpm_fetch_config.registered |= (1U << HPM_FETCH_NODE_UDS);
            hpm_fetch_config.count++;
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL hpm_fetch_register_node(VOID)
{
    hpm_fetch_config.count = 0;

    if ((0 == hpm_fetch_config.sequence) || (0xFFFFFFFF == hpm_fetch_config.sequence))
    {
        MODULE_LOG_I(HPM, "config sequence is 0 or 0xFFFFFFFF.");
        return FALSE;
    }

    for (UINT8 i = 0; i < HPM_FETCH_NODE_COUNT; i++)
    {
        hpm_fetch_node_t *node = &hpm_fetch_config.node[i];
        switch (node->type)
        {
        case HPM_FETCH_NODE_SINGLE:
            if (FALSE == hpm_fetch_regiseter_single(node))
            {
                MODULE_LOG_E(HPM, "register single node failed, slot overflow");
            }
            break;
        case HPM_FETCH_NODE_CONSECTIVE:
            if (FALSE == hpm_fetch_regiseter_consecutive(node))
            {
                MODULE_LOG_E(HPM, "register consecutive node failed, slot overflow");
            }
            break;
        case HPM_FETCH_NODE_MULTI:
            if (FALSE == hpm_fetch_regiseter_multi(node))
            {
                MODULE_LOG_E(HPM, "register multi node failed, slot overflow");
            }
            break;
        case HPM_FETCH_NODE_PGN:
            if (FALSE == hpm_fetch_regiseter_pgn(node))
            {
                MODULE_LOG_E(HPM, "register pgn node failed, slot overflow");
            }
            break;
        case HPM_FETCH_NODE_UDS:
            if (FALSE == hpm_fetch_regiseter_uds(node))
            {
                MODULE_LOG_E(HPM, "register uds node failed, slot overflow");
            }
            break;
        default:
            break;
        }
    }
    return TRUE;
}

static VOID hpm_fetch_unregister_all(VOID)
{
    UINT32 i = 0;

    for (i = 0; i < HPM_FETCH_SINGLE_COUNT; i++)
    {
        hpm_fetch_can_single_t *p = &hpm_fetch_can_single[i];
        p->node = NULL_PTR;
    }

    for (i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        p->node = NULL_PTR;
    }

    for (i = 0; i < HPM_FETCH_NODE_COUNT; i++)
    {
        hpm_fetch_node_t *node = &hpm_fetch_config.node[i];
        node->type = (UINT32)HPM_FETCH_NODE_INVALID;
        node->obtained = (UINT32)HPM_FETCH_STATUS_UNUSED;
        node->len = 0;
    }

    hpm_fetch_config.registered = 0;
    hpm_fetch_config.count = 0;
}

static VOID hpm_fetch_can_recv_cb(can_msg_t *msgs, UINT32 count)
{
    for (UINT32 i = 0; i < count; i++)
    {
        can_msg_t *msg = &msgs[i];
        if (NULL_PTR != msg)
        {
            hpm_fetch_handle_single_msg(msg);
            hpm_fetch_handle_consecutive_msg(msg);
        }
    }
}

INT32 hpm_param_fetch_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        break;
    case MODULE_INIT_SEQ_STORAGE:
        break;
    case MODULE_INIT_SEQ_MODULE:
        hpm_param_task_mutex = xSemaphoreCreateMutex();
        hpm_can_register(hpm_fetch_can_recv_cb);
        hpm_fetch_ftp_init();
        hpm_fetch_load_config();
        hpm_fetch_register_node();
        break;
    default:
        break;
    }
    return 0;
}

VOID hpm_param_fetch_deinit(VOID)
{
    if (NULL_PTR != hpm_param_task_mutex)
    {
        vSemaphoreDelete(hpm_param_task_mutex);
        hpm_param_task_mutex = NULL_PTR;
    }
}

VOID hpm_param_fetch_process(VOID)
{
    hpm_fetch_ftp_handle_process();
}

VOID hpm_param_fetch_wakeup(VOID)
{
    hpm_fetch_can_data_clear();
    hpm_fetch_ftp_state_set(HPM_FETCH_FTP_INIT);
}

VOID hpm_param_fetch_sleep(VOID)
{
    hpm_fetch_can_data_clear();
}

static INT32 hpm_param_fetch_report_single(UINT8 *data, INT32 remain_size)
{
    if (remain_size < 3)
    {
        MODULE_LOG_E(HPM, "remain size %d is too small for single node data.", remain_size);
        return -1;
    }
    UINT16 len = 0;
    data[len++] = (UINT8)(HPM_FETCH_NODE_SINGLE);
    UINT8 *len_ptr = data + len;
    data[len++] = 0x00;
    data[len++] = 0x00;
    remain_size -= 3;

    BOOL no_data = TRUE;

    for (UINT8 i = 0; i < HPM_FETCH_SINGLE_COUNT; i++)
    {
        hpm_fetch_can_single_t *p = &hpm_fetch_can_single[i];
        HPM_PARAM_LOCK();
        if ((NULL_PTR != p->node) && (p->node->obtained == (UINT32)HPM_FETCH_STATUS_OBTAINED))
        {
            if (remain_size < (INT32)(p->node->len + 5))
            {
                MODULE_LOG_E(HPM, "remain size %d is too small for single node data.", remain_size);
                HPM_PARAM_UNLOCK();
                return -1;
            }
            no_data = FALSE;
            data[len++] = (UINT8)(p->node->canid >> 24) & 0xFF;
            data[len++] = (UINT8)(p->node->canid >> 16) & 0xFF;
            data[len++] = (UINT8)(p->node->canid >> 8) & 0xFF;
            data[len++] = (UINT8)(p->node->canid) & 0xFF;
            data[len++] = (UINT8)(HPM_FECTCH_SINGLE_DATA_LEN);
            memcpy(data + len, p->data, HPM_FECTCH_SINGLE_DATA_LEN);
            len += HPM_FECTCH_SINGLE_DATA_LEN;
            remain_size -= (HPM_FECTCH_SINGLE_DATA_LEN + 5);
        }
        HPM_PARAM_UNLOCK();
    }

    if (TRUE == no_data)
    {
        return 0;
    }

    len_ptr[0] = (UINT8)((len - 3) >> 8) & 0xFF;
    len_ptr[1] = (UINT8)((len - 3) & 0xFF);

    return (INT32)len;
}

static INT32 hpm_param_fetch_report_consecutive(UINT8 *data, INT32 remain_size)
{
    if (remain_size < 3)
    {
        MODULE_LOG_E(HPM, "remain size %d is too small for single node data.", remain_size);
        return -1;
    }

    UINT16 len = 0;
    data[len++] = (UINT8)(HPM_FETCH_NODE_CONSECTIVE);
    UINT8 *len_ptr = data + len;
    data[len++] = 0x00;
    data[len++] = 0x00;
    remain_size -= 3;

    BOOL no_data = TRUE;

    for (UINT8 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        HPM_PARAM_LOCK();
        if ((NULL_PTR != p->node) &&
            (p->node->obtained == (UINT32)HPM_FETCH_STATUS_OBTAINED) &&
            (p->node->type == (UINT32)HPM_FETCH_NODE_CONSECTIVE))
        {
            if (remain_size < (INT32)(p->node->len + 5))
            {
                MODULE_LOG_E(HPM, "remain size %d is too small for consecutive node data.", remain_size);
                HPM_PARAM_UNLOCK();
                return -1;
            }

            no_data = FALSE;
            data[len++] = (UINT8)(p->node->canid >> 24) & 0xFF;
            data[len++] = (UINT8)(p->node->canid >> 16) & 0xFF;
            data[len++] = (UINT8)(p->node->canid >> 8) & 0xFF;
            data[len++] = (UINT8)(p->node->canid) & 0xFF;

            if (p->node->len > (p->node->frame * (HPM_FECTCH_SINGLE_DATA_LEN)))
            {
                p->node->len = p->node->frame * (HPM_FECTCH_SINGLE_DATA_LEN);
            }
            data[len++] = (UINT8)(p->node->len);
            memcpy(data + len, p->data, p->node->len);
            len += p->node->len;
            remain_size -= (p->node->len + 5);
        }
        HPM_PARAM_UNLOCK();
    }

    if (TRUE == no_data)
    {
        return 0;
    }

    len_ptr[0] = (UINT8)((len - 3) >> 8) & 0xFF;
    len_ptr[1] = (UINT8)((len - 3) & 0xFF);

    return (INT32)len;
}

static INT32 hpm_param_fetch_report_multi(UINT8 *data, INT32 remain_size)
{
    if (remain_size < 3)
    {
        MODULE_LOG_E(HPM, "remain size %d is too small for multi node data.", remain_size);
        return -1;
    }

    UINT16 len = 0;
    UINT8 *len_ptr = data + len;
    data[len++] = (UINT8)(HPM_FETCH_NODE_MULTI);
    data[len++] = 0x00;
    data[len++] = 0x00;
    remain_size -= 3;

    BOOL no_data = TRUE;

    for (UINT8 i = 0; i < HPM_FETCH_MULTI_COUNT; i++)
    {
        hpm_fetch_can_multi_t *p = &hpm_fetch_can_multi[i];
        HPM_PARAM_LOCK();
        if ((NULL_PTR != p->node) &&
            (p->node->obtained == (UINT32)HPM_FETCH_STATUS_OBTAINED) &&
            (p->node->type == (UINT32)HPM_FETCH_NODE_MULTI))
        {
            if (remain_size < (INT32)(p->node->len + 5))
            {
                MODULE_LOG_E(HPM, "remain size %d is too small for multi node data.", remain_size);
                HPM_PARAM_UNLOCK();
                return -1;
            }

            no_data = FALSE;
            data[len++] = (UINT8)(p->node->canid >> 24) & 0xFF;
            data[len++] = (UINT8)(p->node->canid >> 16) & 0xFF;
            data[len++] = (UINT8)(p->node->canid >> 8) & 0xFF;
            data[len++] = (UINT8)(p->node->canid) & 0xFF;
            if (p->node->len > HPM_FECTCH_MULTI_DATA_LEN)
            {
                p->node->len = HPM_FECTCH_MULTI_DATA_LEN;
            }
            data[len++] = (UINT8)(p->node->len);
            memcpy(data + len, p->data, p->node->len);
            len += p->node->len;
            remain_size -= (p->node->len + 5);
        }
    }

    if (TRUE == no_data)
    {
        return 0;
    }

    len_ptr[0] = (UINT8)((len - 2) >> 8) & 0xFF;
    len_ptr[1] = (UINT8)((len - 2) & 0xFF);

    return (INT32)len;
}

INT32 hpm_param_fetch_report(UINT8 *data, INT32 remain_size)
{
    if (NULL_PTR == data || remain_size <= 6)
    {
        MODULE_LOG_E(HPM, "invalid parameter");
        return -1;
    }

    hpm_fetch_config_t *p = &hpm_fetch_config;

    UINT16 len = 0;
    UINT8 *len_ptr = data + len;

    data[len++] = 0x00;
    data[len++] = 0x00;
    data[len++] = (p->sequence >> 24) & 0xFF;
    data[len++] = p->sequence >> 16;
    data[len++] = (p->sequence >> 8) & 0xFF;
    data[len++] = p->sequence & 0xFF;

    remain_size -= len;

    if (p->registered & (1U << HPM_FETCH_NODE_SINGLE))
    {
        INT32 single_len = hpm_param_fetch_report_single(data + len, remain_size);
        if (single_len < 0)
        {
            MODULE_LOG_E(HPM, "single node data overflow remain : %d.", remain_size);
            return -1;
        }
        len += single_len;
        remain_size -= single_len;
    }

    if (p->registered & (1U << HPM_FETCH_NODE_CONSECTIVE))
    {
        if (remain_size <= 0)
        {
            MODULE_LOG_E(HPM, "remain size %d is too small for consecutive node data.", remain_size);
            return -1;
        }

        INT32 consecutive_len = hpm_param_fetch_report_consecutive(data + len, remain_size);
        if (consecutive_len < 0)
        {
            MODULE_LOG_E(HPM, "consecutive node data overflow remain : %d.", remain_size);
            return -1;
        }
        len += consecutive_len;
        remain_size -= consecutive_len;
    }

    if (p->registered & (1U << HPM_FETCH_NODE_MULTI))
    {
        if (remain_size <= 0)
        {
            MODULE_LOG_E(HPM, "remain size %d is too small for multi node data.", remain_size);
            return -1;
        }

        INT32 multi_len = hpm_param_fetch_report_multi(data + len, remain_size);
        if (multi_len < 0)
        {
            MODULE_LOG_E(HPM, "multi node data overflow remain : %d.", remain_size);
            return -1;
        }
        len += multi_len;
    }

    len_ptr[0] = (UINT8)((len - 2) >> 8) & 0xFF;
    len_ptr[1] = (UINT8)((len - 2) & 0xFF);

    hpm_fetch_can_data_clear();

    return len;
}

static VOID hpm_fetch_set_param(hpm_fetch_config_t *config)
{
    if (NULL_PTR == config)
    {
        return;
    }

    TBOX_CFG_ID cfg_id;
    UINT32 baud = hpm_param_get_baudrate((hpm_fetch_can_rate_e)config->can_rate1);

    if (0 != baud)
    {
        TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
        tbox_cfg_write(cfg_id, &baud);
    }

    baud = hpm_param_get_baudrate((hpm_fetch_can_rate_e)config->can_rate2);
    if (0 != baud)
    {
        TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
        tbox_cfg_write(cfg_id, &baud);
    }

    baud = hpm_param_get_baudrate((hpm_fetch_can_rate_e)config->can_rate3);
    if (0 != baud)
    {
        TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
        tbox_cfg_write(cfg_id, &baud);
    }

    UINT32 report_intv = config->interval;
    if (0 != report_intv)
    {
        TBOX_CFG_ID_GET(HPMCYCON, cfg_id);
        tbox_cfg_write(cfg_id, &report_intv);
    }

    // obd protocol, do nothing currently
}

VOID hpm_param_show_cfg(VOID)
{
    tbox_log_print("\r\n-------------------------------------------------------------\r\n");
    tbox_log_print("%-24s : 0X%08X\r\n", "id", hpm_fetch_config.sequence);
    tbox_log_print("%-24s : %d\r\n", "interval", hpm_fetch_config.interval);
    tbox_log_print("%-24s : %d\r\n", "protocol", hpm_fetch_config.protocol);
    tbox_log_print("%-24s : %d\r\n", "can1 baudrate", hpm_param_get_baudrate((hpm_fetch_can_rate_e)hpm_fetch_config.can_rate1));
    tbox_log_print("%-24s : %d\r\n", "can2 baudrate", hpm_param_get_baudrate((hpm_fetch_can_rate_e)hpm_fetch_config.can_rate2));
    tbox_log_print("%-24s : %d\r\n", "can3 baudrate", hpm_param_get_baudrate((hpm_fetch_can_rate_e)hpm_fetch_config.can_rate3));
    tbox_log_print("%-24s : %d\r\n", "count", hpm_fetch_config.count);

    for (UINT8 i = 0; i < HPM_FETCH_NODE_COUNT; i++)
    {
        hpm_fetch_node_t *node = &hpm_fetch_config.node[i];
        if (node->type == (UINT32)HPM_FETCH_NODE_INVALID)
        {
            continue;
        }

        tbox_log_print("Node %d: Type %d, CANID 0x%08X, Channel %d, Obtained %s, Len %d\r\n",
                       i, node->type, node->canid, node->channel,
                       node->obtained == HPM_FETCH_STATUS_OBTAINED ? "Yes" : "No", node->len);
    }
}