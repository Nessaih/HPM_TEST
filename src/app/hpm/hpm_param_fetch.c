#include "tbox_common.h"
#include "tbox_string.h"
#include "tbox_core.h"
#include "time_if.h"
#include "tbox_cfg_if.h"
#include "flash_common.h"
#include "drv_flash_mcu.h"
#include "4g_if.h"
#include "hpm_param_fetch.h"
#include "hpm_can.h"

#define HPM_PARAM_FETCH_NAME "HPM_PARAM_FETCH_CONFIG"
#define HPM_PARAM_MAGIC_NUM (0x4346) // "CF"

#define HPM_PARAM_LOCK() xSemaphoreTake(hpm_param_task_mutex, portMAX_DELAY)
#define HPM_PARAM_UNLOCK() xSemaphoreGive(hpm_param_task_mutex)

#ifndef HPM_PARAM_J1939
typedef void (*J1939_PGN_CALLBACK_T)(uint8_t *msg, uint16_t len, uint32_t *pgn);
#endif

#define HPM_PARAM_DOWNLOAD_TIMEOUT (300U) // 300s
#define HPM_PARAM_READ_SIZE (64U)
#define HPM_PARAM_BUFFER_SIZE (256U)

typedef enum
{
    HPM_PARAM_INIT = 0,
    HPM_PARAM_START_DOWNLOAD = 1,
    HPM_PARAM_DOWNLOADING = 2,
    HPM_PARAM_READ_CFG = 3,
} HPM_PARAM_STATE;

typedef struct
{
    uint32_t param_id;
    uint32_t file_size;
    uint8_t url[128];
    uint8_t md5[16];
} HPM_FTP_DOWNLOAD_INFO;

typedef struct
{
    uint8_t can_channel;
    uint32_t canID;
} HPM_PARAM_CFG_TYPE1_SINGLE;

typedef struct
{
    uint8_t can_channel;
    uint8_t frame_num;
    uint8_t index;
    uint32_t canID;
} HPM_PARAM_CFG_TYPE2_COMPLEX;

typedef struct
{
    uint8_t can_channel;
    uint16_t sa;
    uint16_t pgn;
    J1939_PGN_CALLBACK_T fun_cb;
} HPM_PARAM_CAN_TYPE3_PGN_BC;

typedef struct
{
    uint8_t can_channel;
    uint16_t sa;
    uint16_t ta;
    uint16_t pgn;
    J1939_PGN_CALLBACK_T fun_cb;
} HPM_PARAM_CAN_TYPE4_PGN_REQ;

typedef struct
{
    uint8_t can_channel;
    uint32_t req_id;
    uint32_t resp_id;
    uint32_t did;
} HPM_PARAM_CAN_TYPE5_UDS;

typedef struct
{
    uint32_t id;
    uint16_t intv;
    uint16_t baud1;
    uint16_t baud2;
    uint8_t obd;
    uint8_t can1_num;
    uint8_t can2_num;
    uint8_t can3_num;
    uint8_t can4_num;
    uint8_t can5_num;
    HPM_PARAM_CFG_TYPE1_SINGLE can1_single[HPM_PARAM_MAX_CAN_TYPE1_SINGLE];
    HPM_PARAM_CFG_TYPE2_COMPLEX can2_complex[HPM_PARAM_MAX_CAN_TYPE2_COMPLEX];
    HPM_PARAM_CAN_TYPE3_PGN_BC can3_pgn_bc[HPM_PARAM_MAX_CAN_TYPE3_PGN_BC];
    HPM_PARAM_CAN_TYPE4_PGN_REQ can4_pgn_req[HPM_PARAM_MAX_CAN_TYPE4_PGN_REQ];
    HPM_PARAM_CAN_TYPE5_UDS can5_uds[HPM_PARAM_MAX_CAN_TYPE5_UDS];
} HPM_PARAM_CFG_INFO;

typedef struct
{
    uint8_t flag;
    uint32_t param_id;
    uint32_t file_size;
    uint16_t magic_num;
} HPM_PARAM_RECORD_INFO;

static SemaphoreHandle_t hpm_param_task_mutex;
static HPM_PARAM_RECORD_INFO hpm_param_record_info;
static uint8_t hpm_param_state;
static time_t hpm_param_req_time;

static HPM_FTP_DOWNLOAD_INFO hpm_download_info;
static uint32_t hpm_param_write_addr;
static HPM_PARAM_CFG_INFO hpm_param_cfg_info;
static HPM_PARAM_CFG_INFO hpm_param_info_tmp;
static int8_t hpm_uds_handle;
static int8_t hpm_pgn_req_index;
static int8_t hpm_did_req_index;
static uint8_t hpm_param_buffer[HPM_PARAM_BUFFER_SIZE];

static uint8_t hpm_param_read_download_cfg(uint32_t file_size);
static uint8_t hpm_param_read_cfg_failed(void);
static uint8_t hpm_param_read_cfg_success(void);
static void hpm_param_cfg_info_record_success(void);
static void hpm_param_cfg_info_record_failed(void);

int hpm_param_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        hpm_param_task_mutex = xSemaphoreCreateMutex();
        if (NULL_PTR == hpm_param_task_mutex)
        {
            MODULE_LOG_E(HPM, "param fetch mutex create failed");
            return -1;
        }
        break;
    case MODULE_INIT_SEQ_MODULE:
    {
        memset(&hpm_param_record_info, 0, sizeof(hpm_param_record_info));
        tbox_cfg_getkv(HPM_PARAM_FETCH_NAME, &hpm_param_record_info, sizeof(hpm_param_record_info));
        if (HPM_PARAM_MAGIC_NUM != hpm_param_record_info.magic_num)
        {
            MODULE_LOG_E(HPM, "do not find param cfg !!!!!!!");
            memset(&hpm_param_record_info, 0, sizeof(hpm_param_record_info));
            hpm_param_record_info.magic_num = HPM_PARAM_MAGIC_NUM;
            tbox_cfg_setkv(HPM_PARAM_FETCH_NAME, &hpm_param_record_info, sizeof(hpm_param_record_info));
        }

        MODULE_LOG_I(HPM, "read cfg flag:%d", hpm_param_record_info.flag);

        hpm_param_state = HPM_PARAM_INIT;
        hpm_param_req_time = 0;
        memset(&hpm_download_info, 0, sizeof(hpm_download_info));
        hpm_param_write_addr = 0;
        hpm_uds_handle = -1;
        hpm_pgn_req_index = -1;
        hpm_did_req_index = -1;
        memset(&hpm_param_cfg_info, 0, sizeof(hpm_param_cfg_info));
        memset(&hpm_param_info_tmp, 0, sizeof(hpm_param_info_tmp));
        if (1 == hpm_param_record_info.flag)
        {
            if (0 == hpm_param_read_download_cfg(hpm_param_record_info.file_size))
            {
                hpm_param_read_cfg_success();
            }
            else
            {
                hpm_param_read_cfg_failed();
                hpm_param_cfg_info_record_failed();
            }
            MODULE_LOG_I(HPM, "cfg flag:%d", hpm_param_record_info.flag);
        }
        memset(hpm_param_buffer, 0, sizeof(hpm_param_buffer));
    }
    default:
        break;
    }

    return 0;
}

static uint8_t hpm_param_get_state(void)
{
    uint8_t state = 0;

    HPM_PARAM_LOCK();
    state = hpm_param_state;
    HPM_PARAM_UNLOCK();

    return state;
}

static void hpm_param_set_state(uint8_t state)
{
    HPM_PARAM_LOCK();
    hpm_param_state = state;
    HPM_PARAM_UNLOCK();
}

static uint16_t hpm_param_calc_diff_secs(void)
{
    DEV_TIME local_time;
    time_t local_secs = time_if_get(&local_time);

    if (hpm_param_req_time > local_secs)
    {
        return 0;
    }
    return (local_secs - hpm_param_req_time);
}

static uint8_t hpm_param_write_flash(uint8_t *buff, uint16_t len)
{
    uint8_t *temp_addr;
    uint16_t temp_len, write_len;

    if ((hpm_param_write_addr + len) >= (FLASH_MCU_ADD_HPM_CFG + FLASH_MCU_SIZE_HPM_CFG))
    {
        MODULE_LOG_E(HPM, "the data is too long1 len:%d", (int)len);
        return 1;
    }

    temp_len = len;
    temp_addr = buff;

    MODULE_LOG_DUMP(HPM, "cfg data:", buff, len);
    while (temp_len > 0)
    {
        write_len = 256 - (hpm_param_write_addr % 256);
        if (temp_len <= write_len)
        {
            write_len = temp_len;
        }

        if (0 != drv_flash_mcu_write(hpm_param_write_addr, temp_addr, write_len))
        {
            return 1;
        }

        temp_addr += write_len;
        hpm_param_write_addr += write_len;
        temp_len -= write_len;

        MODULE_LOG_E(HPM, "hpm write param cfg addr:%02x", hpm_param_write_addr);
    }

    return 0;
}

static void hpm_param_cfg_info_record_success(void)
{
    memset(&hpm_param_record_info, 0, sizeof(hpm_param_record_info));
    hpm_param_record_info.flag = 1;
    hpm_param_record_info.file_size = hpm_download_info.file_size;
    hpm_param_record_info.param_id = hpm_download_info.param_id;
    hpm_param_record_info.magic_num = HPM_PARAM_MAGIC_NUM;
    if (tbox_cfg_setkv(HPM_PARAM_FETCH_NAME, &hpm_param_record_info, sizeof(hpm_param_record_info)) < 0)
    {
        MODULE_LOG_E(HPM, "write param info failed!!!");
    }
    else
    {
        MODULE_LOG_I(HPM, "write cfg flag:%d", hpm_param_record_info.flag);
    }
}

static void hpm_param_cfg_info_record_failed(void)
{
    memset(&hpm_param_record_info, 0, sizeof(hpm_param_record_info));
    hpm_param_record_info.flag = 0;
    hpm_param_record_info.file_size = 0;
    hpm_param_record_info.param_id = 0;
    hpm_param_record_info.magic_num = HPM_PARAM_MAGIC_NUM;
    if (tbox_cfg_setkv(HPM_PARAM_FETCH_NAME, &hpm_param_record_info, sizeof(hpm_param_record_info)) < 0)
    {
        MODULE_LOG_E(HPM, "write param info failed!!!");
    }
    else
    {
        MODULE_LOG_I(HPM, "write cfg flag:%d", hpm_param_record_info.flag);
    }
}

static void hpm_param_bubble_sort(void)
{
    int i, j;
    HPM_PARAM_CFG_TYPE1_SINGLE tmp1;
    HPM_PARAM_CFG_TYPE2_COMPLEX tmp2;

    for (i = 0; i < HPM_PARAM_MAX_CAN_TYPE1_SINGLE; i++)
    {
        for (j = i + 1; j < hpm_param_cfg_info.can1_num; j++)
        {
            if (hpm_param_cfg_info.can1_single[j].canID < hpm_param_cfg_info.can1_single[i].canID)
            {
                tmp1 = hpm_param_cfg_info.can1_single[i];
                hpm_param_cfg_info.can1_single[i] = hpm_param_cfg_info.can1_single[j];
                hpm_param_cfg_info.can1_single[j] = tmp1;
            }
        }
    }

    for (i = 0; i < HPM_PARAM_MAX_CAN_TYPE2_COMPLEX; i++)
    {
        for (j = i + 1; j < hpm_param_cfg_info.can2_num; j++)
        {
            if (hpm_param_cfg_info.can2_complex[j].canID < hpm_param_cfg_info.can2_complex[i].canID)
            {
                tmp2 = hpm_param_cfg_info.can2_complex[i];
                hpm_param_cfg_info.can2_complex[i] = hpm_param_cfg_info.can2_complex[j];
                hpm_param_cfg_info.can2_complex[j] = tmp2;
            }
        }
    }
}

static void hpm_param_uds_open(uint8_t index)
{
#if 0
    UDS_CLIENT uds;
    int16_t ret;

    uds.instance = hpm_param_cfg_info.can5_uds[index].can_channel - 1;
    uds.send_id = hpm_param_cfg_info.can5_uds[index].req_id;
    uds.recv_id = hpm_param_cfg_info.can5_uds[index].resp_id;
    uds.state_ind = hpm_can_uds_state_cb;

    ret = uds_client_open(&uds);
    if (ret < 0)
    {
        MODULE_LOG_E(HPM, "uds open err:%d", ret);
        return;
    }
    hpm_uds_handle = ret;
    // MODULE_LOG_I(HPM, "uds open OK,handle:%d channel:%d req id:0x%x  resp id:0x%x", ret,uds.instance,uds.send_id,uds.recv_id);
#endif
}

static void hpm_param_uds_close(void)
{
#if 0
    if (hpm_did_req_index >= 0)
    {
        if (-1 != hpm_uds_handle)
        {
            uds_client_close(hpm_uds_handle);
            hpm_uds_handle = -1;
        }
    }
#endif
}

static uint8_t hpm_param_read_cfg_success(void)
{
    uint8_t i = 0;
    MODULE_LOG_I(HPM, "hpm_param_read_cfg_success !!!!!!!");
    HPM_PARAM_LOCK();
    memcpy(&hpm_param_cfg_info, &hpm_param_info_tmp, sizeof(hpm_param_cfg_info));
    hpm_param_bubble_sort();
    if (hpm_param_info_tmp.can4_num > 0)
    {
        hpm_pgn_req_index = 0;
    }
    if (hpm_param_info_tmp.can5_num > 0)
    {
        hpm_did_req_index = 0;
    }
    for (i = 0; i < hpm_param_info_tmp.can3_num; i++)
    {
        MODULE_LOG_I(HPM, "add pgn:0x%x", hpm_param_info_tmp.can3_pgn_bc[i].pgn);
        // j1939_pgn_add(hpm_param_info_tmp.can3_pgn_bc[i].pgn);
        // j1939_al_subscribe(hpm_param_info_tmp.can3_pgn_bc[i].pgn, hpm_can_j1939_can3_cb);
    }
    for (i = 0; i < hpm_param_info_tmp.can4_num; i++)
    {
        MODULE_LOG_I(HPM, "add pgn:0x%x", hpm_param_info_tmp.can4_pgn_req[i].pgn);
        // j1939_pgn_add(hpm_param_info_tmp.can4_pgn_req[i].pgn);
        // j1939_al_subscribe(hpm_param_info_tmp.can4_pgn_req[i].pgn, hpm_can_j1939_can4_cb);
    }

    HPM_PARAM_UNLOCK();

    return 0;
}

static uint8_t hpm_param_read_cfg_failed(void)
{
    MODULE_LOG_E(HPM, "hpm_param_read_cfg_failed !!!!!!!");

    HPM_PARAM_LOCK();
    memset(&hpm_param_cfg_info, 0, sizeof(hpm_param_cfg_info));
    hpm_pgn_req_index = -1;
    hpm_did_req_index = -1;

    HPM_PARAM_UNLOCK();

    return 0;
}

static uint8_t hpm_param_set_common_cfg(void)
{
    TBOX_CFG_ID cfg_id = CFG_ID_INVALID;
    if (0 != hpm_param_cfg_info.baud1 && 0xFFFF != hpm_param_cfg_info.baud1)
    {
        TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
        if (CFG_ID_INVALID != cfg_id)
        {
            tbox_cfg_write(cfg_id, (uint8_t *)&hpm_param_cfg_info.baud1);
        }
    }
    if (0 != hpm_param_cfg_info.baud2 && 0xFFFF != hpm_param_cfg_info.baud2)
    {
        TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
        if (CFG_ID_INVALID != cfg_id)
        {
            tbox_cfg_write(cfg_id, (uint8_t *)&hpm_param_cfg_info.baud2);
        }
    }

#if 0
    if (0 != hpm_param_cfg_info.intv && 0xFFFF != hpm_param_cfg_info.intv)
    {
        TBOX_CFG_ID_GET(DATAPERIODACCON, cfg_id);
        if (CFG_ID_INVALID != cfg_id)
        {
            tbox_cfg_write(cfg_id, (uint8_t *)&hpm_param_cfg_info.intv);
        }
    }
    if (0 != hpm_param_cfg_info.obd && 0xFF != hpm_param_cfg_info.obd)
    {
        TBOX_CFG_ID_GET(HPMOBDTYPE, cfg_id);
        if (CFG_ID_INVALID != cfg_id)
        {
            tbox_cfg_write(cfg_id, (uint8_t *)&hpm_param_cfg_info.obd);
        }
    }
#endif
    return 0;
}

static uint8_t hpm_param_read_download_cfg(uint32_t file_size)
{
    MODULE_LOG_I(HPM, "start param read cfg file size:%d", (int)file_size);
    uint32_t read_addr = FLASH_MCU_ADD_HPM_CFG;
    uint32_t read_file_len_total = 0; // total read file len
    uint32_t read_file_len_once = (HPM_PARAM_READ_SIZE <= file_size) ? HPM_PARAM_READ_SIZE : file_size;
    uint32_t read_data_len_total = 0; // read data len total
    uint32_t read_data_len_once = 0;  // read data len once
    uint8_t *start_line = NULL;
    uint8_t *end_line = NULL;
    uint32_t line_len;
    unsigned short fetch_type;
    uint16_t index;
    uint8_t end_len = 2;
    uint8_t tmp_buffer[16] = {0};
    uint8_t tmp_len = 0;
    uint8_t tmp_idx = 0;
    // read [CO]
    memset(&hpm_param_info_tmp, 0, sizeof(hpm_param_info_tmp));
    memset(hpm_param_buffer, 0, sizeof(hpm_param_buffer));

    if (0 != drv_flash_mcu_read(read_addr, hpm_param_buffer, read_file_len_once))
    {
        MODULE_LOG_E(HPM, "hpm param cfg read buffer failed");
        return 1;
    }

    MODULE_LOG_DUMP(HPM, "cfg read:", hpm_param_buffer, read_file_len_once);

    read_file_len_total += read_file_len_once;
    read_data_len_once = 0;
    read_data_len_total = 0;

    MODULE_LOG_I(HPM, "read file total size:%u,read file once size:%u", (unsigned int)read_file_len_total, (unsigned int)read_file_len_once);
    start_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "[CO]");
    if (NULL == start_line)
    {
        MODULE_LOG_E(HPM, "hpm param cfg read [CO] failed");
        return 1;
    }

    end_len = 2;
    end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\r\n");
    if (NULL == end_line)
    {
        end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\n");
        if (NULL == end_line)
        {
            MODULE_LOG_E(HPM, "hpm param cfg read [CO] end line failed");
            return 1;
        }
        end_len = 1;
    }

    line_len = end_line - start_line + end_len;
    read_data_len_once += line_len;
    read_data_len_total += line_len;

    // read CO info
    start_line = hpm_param_buffer + read_data_len_once;
    end_len = 2;
    end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\r\n");
    if (NULL == end_line)
    {
        end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\n");
        if (NULL == end_line)
        {
            MODULE_LOG_E(HPM, "hpm param cfg read [CO] info end line failed");
            return 1;
        }
        end_len = 1;
    }
    line_len = end_line - start_line + end_len;
    read_data_len_once += line_len;
    read_data_len_total += line_len;

    sscanf((char *)start_line, "%u,%hu,[%[^]]],%hu", (unsigned int *)&hpm_param_info_tmp.id, (unsigned short *)&hpm_param_info_tmp.intv,
           tmp_buffer, (unsigned short *)&hpm_param_info_tmp.obd);

    tmp_len = strlen((char *)tmp_buffer);
    if (tmp_len > 0)
    {
        if (NULL == tbox_string_get_substring(tmp_buffer, tmp_len, ":"))
        {
            sscanf((char *)tmp_buffer, "%d", (int *)&hpm_param_info_tmp.baud1);
        }
        else
        {
            sscanf((char *)tmp_buffer, "%d:%d", (int *)&hpm_param_info_tmp.baud1, (int *)&hpm_param_info_tmp.baud2);
        }
    }

    MODULE_LOG_I(HPM, "id:0x%x intv:%d baud1:%d baud2:%d obd:%d", (unsigned int)hpm_param_info_tmp.id, hpm_param_info_tmp.intv, hpm_param_info_tmp.baud1,
                 hpm_param_info_tmp.baud2, hpm_param_info_tmp.obd);

    // read [ES]
    start_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "[ES]");
    if (NULL == start_line)
    {
        MODULE_LOG_E(HPM, "hpm param cfg read [ES] failed");
        return 1;
    }

    end_len = 2;
    end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\r\n");
    if (NULL == end_line)
    {
        end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\n");
        if (NULL == end_line)
        {
            MODULE_LOG_E(HPM, "hpm param cfg read [ES] end line failed");
            return 1;
        }
        end_len = 1;
    }
    line_len = end_line - start_line + end_len;
    read_data_len_once += line_len;
    read_data_len_total += line_len;

    // read ES info
    for (index = 0; read_data_len_total < file_size; index++)
    {
        start_line = hpm_param_buffer + read_data_len_once;
        end_len = 2;
        end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\r\n");
        if (NULL == end_line)
        {
            end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\n");
            if (NULL == end_line)
            {
                if (read_file_len_total < file_size && read_data_len_total < file_size)
                {
                    read_file_len_total = read_data_len_total;
                    read_file_len_once = (HPM_PARAM_READ_SIZE <= file_size - read_file_len_total) ? HPM_PARAM_READ_SIZE : file_size - read_file_len_total;
                    read_addr = FLASH_MCU_ADD_HPM_CFG + read_file_len_total;
                    drv_flash_mcu_read(read_addr, hpm_param_buffer, read_file_len_once);
                    read_file_len_total += read_file_len_once;
                    read_data_len_once = 0;

                    // trace_i(hpm_task_handle, "read file total size:%u,read file once size:%u\r\n",(unsigned int)read_file_len_total,(unsigned int)read_file_len_once);
                    // trace_dumphex(hpm_task_handle, "cfg read:", hpm_param_buffer, read_file_len_once);

                    start_line = hpm_param_buffer + read_data_len_once;
                    end_len = 2;
                    end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\r\n");
                    if (NULL == end_line)
                    {
                        end_line = tbox_string_get_substring(hpm_param_buffer + read_data_len_once, read_file_len_once - read_data_len_once, "\n");
                        if (NULL == end_line)
                        {
                            MODULE_LOG_E(HPM, "hpm param cfg read [ES] info end line failed1,line:%hu", index);
                            return 1;
                        }
                        end_len = 1;
                    }
                }
                else
                {
                    MODULE_LOG_E(HPM, "hpm param cfg read [ES] info end line failed2,line:%hu", index);
                    return 1;
                }
            }
            end_len = 1;
        }

        line_len = end_line - start_line + end_len;
        read_data_len_once += line_len;
        read_data_len_total += line_len;

        sscanf((char *)start_line, "%hu,", &fetch_type);
        switch (fetch_type)
        {
        case 1:
        {
            if (hpm_param_info_tmp.can1_num >= HPM_PARAM_MAX_CAN_TYPE1_SINGLE)
            {
                MODULE_LOG_E(HPM, "can type1 num over flow");
                break;
            }
            if (2 != sscanf((char *)start_line + 2, "%hu,%08x", (unsigned short *)&(hpm_param_info_tmp.can1_single[hpm_param_info_tmp.can1_num].can_channel),
                            (unsigned int *)&(hpm_param_info_tmp.can1_single[hpm_param_info_tmp.can1_num].canID)))
            {
                MODULE_LOG_E(HPM, "get can type1 failed channel:%d	canid:0x%x",
                             hpm_param_info_tmp.can1_single[hpm_param_info_tmp.can1_num].can_channel,
                             (unsigned int)hpm_param_info_tmp.can1_single[hpm_param_info_tmp.can1_num].canID);
                break;
            }

            hpm_param_info_tmp.can1_num++;
            break;
        }
        case 2:
        {
            if (hpm_param_info_tmp.can2_num >= HPM_PARAM_MAX_CAN_TYPE2_COMPLEX)
            {
                MODULE_LOG_E(HPM, "can type2 num over flow");
                break;
            }
            if (3 != sscanf((char *)start_line + 2, "%hu,%*x,%hu,%hu",
                            (unsigned short *)&hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].can_channel,
                            (unsigned short *)&hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].frame_num,
                            (unsigned short *)&hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].index))
            {
                MODULE_LOG_E(HPM, "get can type2 failed,channel:%d	canid:0x%x fram:%d index:%d",
                             (int)hpm_param_info_tmp.can1_single[hpm_param_info_tmp.can1_num].can_channel,
                             (unsigned int)hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].canID,
                             (int)hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].frame_num,
                             (int)hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].index);

                break;
            }

            tmp_len = 0;
            memset(tmp_buffer, 0, sizeof(tmp_buffer));
            for (tmp_idx = 0; tmp_idx < 9; tmp_idx++)
            {
                if (*(start_line + 4 + tmp_idx) == ',')
                {
                    break;
                }

                tmp_len++;
            }

            strncpy((char *)tmp_buffer, (char *)start_line + 4, tmp_len);
            hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].canID = strtoul((char *)tmp_buffer, NULL, 16);

            if (hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].index >= 8)
            {
                MODULE_LOG_E(HPM, "get can type2 failed,index:%d", hpm_param_info_tmp.can2_complex[hpm_param_info_tmp.can2_num].index);
                break;
            }

            hpm_param_info_tmp.can2_num++;
            break;
        }
        case 3:
        {
            if (hpm_param_info_tmp.can3_num >= HPM_PARAM_MAX_CAN_TYPE3_PGN_BC)
            {
                MODULE_LOG_E(HPM, "can type3 num over flow");
                break;
            }
            if (3 != sscanf((char *)start_line + 2, "%hu,%x,%x", (unsigned short *)&hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].can_channel,
                            (unsigned int *)&hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].sa,
                            (unsigned int *)&hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].pgn))
            {
                MODULE_LOG_E(HPM, "get can type3 failed,channel:%d sa:0x%x,pgn:0x%x",
                             (int)hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].can_channel,
                             (unsigned int)hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].sa,
                             (unsigned int)hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].pgn);
                break;
            }
            // hpm_param_info_tmp.can3_pgn_bc[hpm_param_info_tmp.can3_num].fun_cb = hpm_can_j1939_can3_cb;
            hpm_param_info_tmp.can3_num++;
            break;
        }
        case 4:
        {
            if (hpm_param_info_tmp.can4_num >= HPM_PARAM_MAX_CAN_TYPE4_PGN_REQ)
            {
                MODULE_LOG_E(HPM, "can type4 num over flow");
                break;
            }
            if (4 != sscanf((char *)start_line + 2, "%hu,%x,%x,%x", (unsigned short *)&hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].can_channel,
                            (unsigned int *)&hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].sa,
                            (unsigned int *)&hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].ta,
                            (unsigned int *)&hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].pgn))
            {
                MODULE_LOG_E(HPM, "get can type4 failed,chanel:%d sa:0x%x ta:0x%x pgn:0x%x",
                             (int)hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].can_channel,
                             (unsigned int)hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].sa,
                             (unsigned int)hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].ta,
                             (unsigned int)hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].pgn);
                break;
            }
            // hpm_param_info_tmp.can4_pgn_req[hpm_param_info_tmp.can4_num].fun_cb = hpm_can_j1939_can4_cb;
            hpm_param_info_tmp.can4_num++;
            break;
        }

        case 5:
        {
            if (hpm_param_info_tmp.can5_num >= HPM_PARAM_MAX_CAN_TYPE5_UDS)
            {
                MODULE_LOG_E(HPM, "can type5 num over flowd");
                break;
            }
            if (4 != sscanf((char *)start_line + 2, "%hu,%x,%x,%x", (unsigned short *)&hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].can_channel,
                            (unsigned int *)&hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].req_id,
                            (unsigned int *)&hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].resp_id,
                            (unsigned int *)&hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].did))
            {
                MODULE_LOG_E(HPM, "get can type5 failed,chanel:%d reqid:0x%x respid:0x%x did:0x%x",
                             (int)hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].can_channel,
                             (unsigned int)hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].req_id,
                             (unsigned int)hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].resp_id,
                             (unsigned int)hpm_param_info_tmp.can5_uds[hpm_param_info_tmp.can5_num].did);
                break;
            }
            hpm_param_info_tmp.can5_num++;
            break;
        }
        default:
        {
            MODULE_LOG_E(HPM, "can type error type:%d", fetch_type);
            break;
        }
        }
        // printf("\r\n index:%u read data len:%d read file len:%d file\r\n",index,(int)read_data_len_total,(int)read_file_len_total);
    }

    return 0;
}

static uint8_t hpm_param_ftp_download_call_back(uint8 notify_code, uint8 *data, uint16 len)
{
    if (hpm_param_get_state() != HPM_PARAM_DOWNLOADING)
    {
        MODULE_LOG_E(HPM, "download callback faild,state:%d", hpm_param_get_state());
        return IF_FTP_4G_CALLBACK_RET_OK;
    }
    if (IF_FTP_4G_NOTIFY_FINISH == notify_code)
    {
        hpm_param_set_state(HPM_PARAM_READ_CFG);
        MODULE_LOG_E(HPM, "hpm param cfg download finish");
    }
    else if (IF_FTP_4G_NOTIFY_ERROR == notify_code)
    {
        MODULE_LOG_E(HPM, "download timeout");
        hpm_param_set_state(HPM_PARAM_INIT);
        return IF_FTP_4G_CALLBACK_RET_ABORT;
    }
    else
    {
        if (hpm_param_calc_diff_secs() >= HPM_PARAM_DOWNLOAD_TIMEOUT)
        {
            MODULE_LOG_E(HPM, "download timeout 1");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }
        if (0 != hpm_param_write_flash(data, len))
        {
            MODULE_LOG_E(HPM, "failed to write flash");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }
    }
    return IF_FTP_4G_CALLBACK_RET_OK;
}

int hpm_param_download_req(UINT8* in_data, UINT16 in_len, UINT8* res, UINT16* res_len)
{
    unsigned short r_len = 0;
    uint8_t *url;
    DEV_TIME time;
    uint32_t param_id;
    uint16_t pos = 0;

    UNUSED(in_len);

    res[pos++] = 0x00; // 应答长度
    res[pos++] = 0x00;
    *res_len = pos;

    if (HPM_PARAM_INIT != hpm_param_get_state())
    {
        MODULE_LOG_E(HPM, "param donload faild,state:%u", (unsigned int)hpm_param_get_state());
        return -1;
    }
    param_id = (in_data[0] << 24) + (in_data[1] << 16) + (in_data[2] << 8) + in_data[3];
    if (param_id == hpm_param_record_info.param_id)
    {
        MODULE_LOG_E(HPM, "param id not change,id:%u", (unsigned int)param_id);
        return -1;
    }
    memset(&hpm_download_info, 0, sizeof(hpm_download_info));
    hpm_download_info.param_id = param_id;
    r_len += 4;
    url = in_data + r_len;
    r_len += strlen((char *)url);
    r_len++; //\0
    hpm_download_info.file_size = (in_data[r_len] << 8) + in_data[r_len + 1];
    r_len += 2;
    memcpy(hpm_download_info.md5, in_data + r_len, 16);
    memcpy(hpm_download_info.url, url, strlen((char *)url));
    MODULE_LOG_E(HPM, "param id:0x%x,url:%s,cfg_size:%d", (unsigned int)hpm_download_info.param_id, url, (int)hpm_download_info.file_size);

    time_t current_time = time_if_get(&time);

    HPM_PARAM_LOCK();
    hpm_param_req_time = current_time;
    HPM_PARAM_UNLOCK();

    hpm_param_set_state(HPM_PARAM_START_DOWNLOAD);

    return 0;
}

void hpm_param_timeout(void)
{
    uint8_t param_state = hpm_param_get_state();
    switch (param_state)
    {
    case HPM_PARAM_START_DOWNLOAD:
    {
        uint32_t tmp_addr;
        tmp_addr = hpm_param_write_addr = FLASH_MCU_ADD_HPM_CFG;
        while (1)
        {
            drv_flash_mcu_erase(tmp_addr, 1);
            tmp_addr += 0x00001000;
            if (tmp_addr - FLASH_MCU_ADD_HPM_CFG >= FLASH_MCU_SIZE_HPM_CFG)
            {
                break;
            }
        }

        if_ftp_4g_download(hpm_download_info.url, strlen((char *)hpm_download_info.url),
                           IF_4G_PUBLIC_APN, hpm_param_ftp_download_call_back);
        hpm_param_set_state(HPM_PARAM_DOWNLOADING);

        break;
    }
    case HPM_PARAM_DOWNLOADING:
    {
        if (hpm_param_calc_diff_secs() >= HPM_PARAM_DOWNLOAD_TIMEOUT)
        {
            MODULE_LOG_E(HPM, "download timeout,set state init");
            hpm_param_set_state(HPM_PARAM_INIT);
        }
        break;
    }
    case HPM_PARAM_READ_CFG:
    {
        if (if_4g_is_downloading())
        {
            MODULE_LOG_I(HPM, "ftp is downloading, wait ftp idle");
            break;
        }

        if (0 == hpm_param_read_download_cfg(hpm_download_info.file_size))
        {
            hpm_param_read_cfg_success();
            hpm_param_cfg_info_record_success();
            hpm_param_set_common_cfg();
            // hpm_can_data_reset();
        }
        else
        {
            hpm_param_read_cfg_failed();
            hpm_param_cfg_info_record_failed();
        }

        hpm_param_set_state(HPM_PARAM_INIT);

        break;
    }

    default:
        break;
    }
}

int8_t hpm_param_canid_type1_index_find(uint32_t canid)
{
    int8_t i = 0;
    for (i = 0; i < hpm_param_cfg_info.can1_num; i++)
    {
        if (hpm_param_cfg_info.can1_single[i].canID == canid)
        {
            break;
        }
    }
    // printf("\r\n index:%d \r\n",i);
    if (i < hpm_param_cfg_info.can1_num)
    {
        return i;
    }
    else
    {
        return -1;
    }
#if 0
    uint8_t low, high, mid;
    
    low = 0;
    //HPM_PARAM_LOCK();	
    high = hpm_param_cfg_info.can1_num - 1;

    while(low<=high)
    {
        mid = (low+high)/2;
        if(hpm_param_cfg_info.can1_single[mid].canID == canid)
        {
            //HPM_PARAM_UNLOCK();       
            return mid;
        }
        if(hpm_param_cfg_info.can1_single[mid].canID > canid)
        {
            high = mid-1;
        }
        if(hpm_param_cfg_info.can1_single[mid].canID < canid)
        {
            low = mid+1;
        }
    }
    
    //HPM_PARAM_UNLOCK();

    return -1;
#endif
}

int8_t hpm_param_canid_type2_index_find(uint32_t canid, uint8_t *frame_num, uint8_t *indexpos)
{
    int8_t i = 0;
    for (i = 0; i < hpm_param_cfg_info.can2_num; i++)
    {
        if (hpm_param_cfg_info.can2_complex[i].canID == canid)
        {
            *frame_num = hpm_param_cfg_info.can2_complex[i].frame_num;
            *indexpos = hpm_param_cfg_info.can2_complex[i].index;
            break;
        }
    }
    // printf("\r\n index:%d \r\n",i);
    if (i < hpm_param_cfg_info.can2_num)
    {
        return i;
    }
    else
    {
        return -1;
    }

#if 0
    uint8_t low, high, mid;
    
    low = 0;
    
    HPM_PARAM_LOCK();	
    high = hpm_param_cfg_info.can2_num - 1;
    
    while(low<=high)
    {
        mid = (low+high)/2;
        if(hpm_param_cfg_info.can2_complex[mid].canID == canid)
        {
            *frame_num = hpm_param_cfg_info.can2_complex[mid].frame_num;
            *indexpos = hpm_param_cfg_info.can2_complex[mid].index;
            HPM_PARAM_UNLOCK();    
            return mid;
        }
        if(hpm_param_cfg_info.can2_complex[mid].canID > canid)
        {
            high = mid-1;
        }
        if(hpm_param_cfg_info.can2_complex[mid].canID < canid)
        {
            low = mid+1;
        }
    }
    
    HPM_PARAM_UNLOCK();
    return -1;
#endif
}

int8_t hpm_param_pgn_type3_index_find(uint32_t pgn)
{
    int8_t i = 0;

    // HPM_PARAM_LOCK();
    for (i = 0; i < hpm_param_cfg_info.can3_num; i++)
    {
        if (pgn == hpm_param_cfg_info.can3_pgn_bc[i].pgn)
        {
            // HPM_PARAM_UNLOCK();
            return i;
        }
    }
    // HPM_PARAM_UNLOCK();
    return -1;
}

int8_t hpm_param_pgn_type4_index_find(uint32_t pgn)
{
    int8_t i = 0;

    // HPM_PARAM_LOCK();
    for (i = 0; i < hpm_param_cfg_info.can4_num; i++)
    {
        if (pgn == hpm_param_cfg_info.can4_pgn_req[i].pgn)
        {
            // HPM_PARAM_UNLOCK();
            return i;
        }
    }
    // HPM_PARAM_UNLOCK();
    return -1;
}

int8_t hpm_param_j1939_req(void)
{
    HPM_PARAM_LOCK();

    if (hpm_param_cfg_info.can4_num > 0 && hpm_pgn_req_index != -1)
    {
        // j1939_pgn_req(hpm_param_cfg_info.can4_pgn_req[hpm_pgn_req_index].pgn,
        //               hpm_param_cfg_info.can4_pgn_req[hpm_pgn_req_index].ta,
        //               hpm_param_cfg_info.can4_pgn_req[hpm_pgn_req_index].sa);
        hpm_pgn_req_index++;
        if (hpm_pgn_req_index >= hpm_param_cfg_info.can4_num)
        {
            hpm_pgn_req_index = 0;
        }
    }

    HPM_PARAM_UNLOCK();
    return 0;
}

int8_t hpm_param_uds_req(void)
{
    HPM_PARAM_LOCK();

    hpm_param_uds_close();

    if (hpm_param_info_tmp.can5_num > 0 && hpm_did_req_index >= 0)
    {
        hpm_param_uds_open(hpm_did_req_index);
    }

    if (hpm_uds_handle >= 0)
    {
        // uds_client_readdatabydid(hpm_uds_handle, hpm_param_cfg_info.can5_uds[hpm_did_req_index].did);
    }

    HPM_PARAM_UNLOCK();
    return 0;
}

int8_t hpm_param_get_uds_did_index(void)
{
    int8_t index;
    HPM_PARAM_LOCK();
    index = hpm_did_req_index;
    HPM_PARAM_UNLOCK();
    return index;
}

int8_t hpm_param_uds_did_index_add(void)
{
    int8_t index;
    HPM_PARAM_LOCK();
    hpm_did_req_index++;
    if (hpm_did_req_index >= hpm_param_info_tmp.can5_num)
    {
        hpm_did_req_index = 0;
    }
    index = hpm_did_req_index;
    HPM_PARAM_UNLOCK();

    return index;
}

uint32_t hpm_param_get_uds_did(void)
{
    uint32_t did;
    HPM_PARAM_LOCK();
    did = hpm_param_cfg_info.can5_uds[hpm_did_req_index].did;
    HPM_PARAM_UNLOCK();
    return did;
}

void hpm_param_show_cfg(void)
{
    uint8_t i = 0;
    tbox_log_print("\r\n\r\n-------------------------------------------------------------\r\n");

    tbox_log_print(" %-24s : 0x%x\r\n", "id", (unsigned int)hpm_param_cfg_info.id);
    tbox_log_print(" %-24s : %d\r\n", "intv", hpm_param_cfg_info.intv);
    tbox_log_print(" %-24s : %d\r\n", "baud1", hpm_param_cfg_info.baud1);
    tbox_log_print(" %-24s : %d\r\n", "baud2", hpm_param_cfg_info.baud2);
    tbox_log_print(" %-24s : %d\r\n", "obd type", hpm_param_cfg_info.obd);

    tbox_log_print(" %-24s : %d\r\n", "can type1 num", hpm_param_cfg_info.can1_num);
    for (i = 0; i < hpm_param_cfg_info.can1_num; i++)
    {
        tbox_log_print(" channel:%d	canid:0x%x\r\n", hpm_param_cfg_info.can1_single[i].can_channel, (unsigned int)hpm_param_cfg_info.can1_single[i].canID);
    }

    tbox_log_print(" %-24s : %d\r\n", "can type2 num", hpm_param_cfg_info.can2_num);
    for (i = 0; i < hpm_param_cfg_info.can2_num; i++)
    {
        tbox_log_print(" channel:%d,	canid:0x%x, fram num:%d, fram index:%d\r\n", hpm_param_cfg_info.can2_complex[i].can_channel,
                       (unsigned int)hpm_param_cfg_info.can2_complex[i].canID, hpm_param_cfg_info.can2_complex[i].frame_num,
                       hpm_param_cfg_info.can2_complex[i].index);
    }

    tbox_log_print(" %-24s : %d\r\n", "can type3 num", hpm_param_cfg_info.can3_num);
    for (i = 0; i < hpm_param_cfg_info.can3_num; i++)
    {
        tbox_log_print(" channel:%d,	SA:0x%x, PGN:0x%x\r\n", hpm_param_cfg_info.can3_pgn_bc[i].can_channel,
                       (unsigned int)hpm_param_cfg_info.can3_pgn_bc[i].sa, (unsigned int)hpm_param_cfg_info.can3_pgn_bc[i].pgn);
    }

    tbox_log_print(" %-24s : %d\r\n", "can type4 num", hpm_param_cfg_info.can4_num);
    for (i = 0; i < hpm_param_cfg_info.can4_num; i++)
    {
        tbox_log_print(" channel:%d,	SA:0x%x, TA:0x%x, PGN:0x%x\r\n", hpm_param_cfg_info.can4_pgn_req[i].can_channel,
                       (unsigned int)hpm_param_cfg_info.can4_pgn_req[i].sa, (unsigned int)hpm_param_cfg_info.can4_pgn_req[i].ta,
                       (unsigned int)hpm_param_cfg_info.can4_pgn_req[i].pgn);
    }

    tbox_log_print(" %-24s : %d\r\n", "can type5 num", hpm_param_cfg_info.can5_num);
    for (i = 0; i < hpm_param_cfg_info.can5_num; i++)
    {
        tbox_log_print(" channel:%d,	req_id:0x%x, resp_id:0x%x, did:0x%x\r\n", hpm_param_cfg_info.can5_uds[i].can_channel,
                       (unsigned int)hpm_param_cfg_info.can5_uds[i].req_id, (unsigned int)hpm_param_cfg_info.can5_uds[i].resp_id,
                       (unsigned int)hpm_param_cfg_info.can5_uds[i].did);
    }

    tbox_log_print("\r\n\r\n-------------------------------------------------------------\r\n");
}

uint32_t hpm_param_get_id(void)
{
    return hpm_param_cfg_info.id;
}
