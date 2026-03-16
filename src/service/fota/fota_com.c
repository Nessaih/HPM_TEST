#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_log.h"
#include "version.h"
#include "fota_if.h"
#include "time_if.h"
#include "flash_common.h"
#include "4g_if.h"
#include "Crc_Hal.h"
#include "Crc_Hal_Types.h"
#include "tbox_cfg_if.h"
#include "program.h"

#include "fota_content.h"
#include "fota_com.h"

typedef VOID (*fota_com_process_func)(VOID);

static VOID fota_com_write_context(VOID);


static FOTA_CONTEXT_T   fota_context;
static UINT32 			fota_write_addr;
static UINT32			fota_com_crc;
static BOOL				fota_first_crc_call;

static VOID fota_com_notify_result(FOTA_RESULT_INFO_T *result)
{
	TBOX_MSG_DATA data = {.data = (UINT8*)result, .size = sizeof(FOTA_RESULT_INFO_T)};
	tbox_message_publish(FOTA_EVENT_STATE_CHANGE, &data);
}

static INT32 fota_com_calc_diff_secs(VOID)
{
	INT32 cur_tick;
	cur_tick = time_if_get_systick_s();

	return (cur_tick - fota_context.tick);
}

static INT32 fota_com_write_flash(UINT8 *data, UINT16 len)
{
	UINT8* temp_addr;
    UINT16 temp_len , write_len;

    if ((fota_write_addr + len) >= (FLASH_MCU_ADDR_FOTA_DATA + FLASH_MCU_SIZE_FOTA_DATA))
    {
        MODULE_LOG_E(FOTA , "the data is too long");
        return -1;
    }

    if (((fota_write_addr - FLASH_MCU_ADDR_FOTA_DATA) + len) > FLASH_APP_LEN)
    {
        MODULE_LOG_E(FOTA , "the data is too long");
        return -1;
    }

    temp_len = len;
    temp_addr = data;
    while (temp_len > 0)
    {
        write_len = 256 - (fota_write_addr % 256);
        if (temp_len <= write_len)
        {
            write_len = temp_len;
        }

        if (0 != drv_flash_mcu_write(fota_write_addr , temp_addr , write_len))
        {
            return 1;
        }

        temp_addr += write_len;
        fota_write_addr += write_len;
        temp_len -= write_len;

        MODULE_LOG_I(FOTA, "fota_com_write_flash:%08x", fota_write_addr);
    }

    return 0;
}

static UINT8 fota_com_ftp_callback(UINT8 notify_code, UINT8 *data, UINT16 len)
{
	if (FOTA_COM_DOWNLOADING != fota_context.state)
    {
        return IF_FTP_4G_CALLBACK_RET_OK;
    }

    if (IF_FTP_4G_NOTIFY_FINISH == notify_code)
    {
		MODULE_LOG_I(FOTA , "download finish");		
		fota_context.timeout = 0;
		fota_context.state = FOTA_COM_DOWNLOAD_FINISH;    	
    }
    else if (IF_FTP_4G_NOTIFY_ERROR == notify_code)
    {
        MODULE_LOG_I(FOTA , "error notify_code:%d len:%d" , notify_code , len);
        fota_context.reslut.rst = FOTA_RST_NG;
		fota_context.reslut.err_code = FOTA_STA_DWL_NG;
		fota_com_notify_result(&fota_context.reslut);
		fota_com_reset();
    }
    else
    {
        if (fota_com_calc_diff_secs() >= FOTA_UPGRADE_TIMEOUT)
        {
			fota_context.reslut.rst = FOTA_RST_NG;
			fota_context.reslut.err_code = FOTA_STA_DWL_NG;
            MODULE_LOG_I(FOTA , "download timeout");
			
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }

        MODULE_LOG_I(FOTA , "%u %u" , fota_write_addr - FLASH_MCU_ADDR_FOTA_DATA , len);
        if (((fota_write_addr - FLASH_MCU_ADDR_FOTA_DATA) + len) >= fota_context.file_size)
        {
            if (len < 4)
            {
                MODULE_LOG_E(FOTA, "fota_com: final chunk too small to contain CRC (len=%u)", len);
                fota_context.reslut.rst = FOTA_RST_NG;
                fota_context.reslut.err_code = FOTA_STA_DWL_NG;
                return IF_FTP_4G_CALLBACK_RET_ABORT;
            }
            fota_com_crc = Crc_Hal_CalculateCRC32(data, len - 4 , fota_com_crc, fota_first_crc_call, CRC_TABLE_256_BYTE_MODE);
			fota_context.crc = (data[len-4]<<24) | (data[len-3]<<16) | (data[len-2]<<8) | data[len-1];
        }
        else
        {
            fota_com_crc = Crc_Hal_CalculateCRC32(data , len , fota_com_crc, fota_first_crc_call, CRC_TABLE_256_BYTE_MODE);
        }
        
        /* 第一次调用后，后续调用都使用FALSE */
        if (fota_first_crc_call)
        {
            fota_first_crc_call = FALSE;
        }

        if (0 != fota_com_write_flash(data , len))
        {
            MODULE_LOG_I(FOTA , "failed to write flash");
            return IF_FTP_4G_CALLBACK_RET_ABORT;
        }
    }

    return IF_FTP_4G_CALLBACK_RET_OK;
}

static VOID fota_com_proc_idle(VOID)
{
	return;
}

static VOID fota_com_proc_download(VOID)
{
	INT32 sec_addr = 0;
	
	fota_context.timeout++;
	if(fota_context.timeout == FOTA_WAIT_ERASE_INTV)
	{
		sec_addr = fota_write_addr = FLASH_MCU_ADDR_FOTA_DATA;
		while (1)
		{
			drv_flash_mcu_erase(sec_addr, FLASH_MCU_SIZE_FOTA_PAGE);
			sec_addr += FLASH_MCU_SIZE_FOTA_PAGE;
			if((sec_addr-FLASH_MCU_ADDR_FOTA_DATA) >= FLASH_MCU_SIZE_FOTA_DATA)
			{
				break;
			}
			drv_wdg_feed();
		}
	}

	if(fota_context.timeout >= FOTA_WAIT_DOWNL_INTV)
	{
		fota_context.state = FOTA_COM_DOWNLOADING;
		fota_context.timeout = 0;
		if_ftp_4g_download(fota_context.url, fota_context.url_len, IF_4G_PUBLIC_APN, fota_com_ftp_callback);
	}
}

static VOID fota_com_proc_downliading(VOID)
{
	fota_context.timeout++;
	if(fota_context.timeout >= FOTA_DOWNL_TIMEOUT)
	{
		fota_context.reslut.rst = FOTA_RST_NG;
		fota_context.reslut.err_code = FOTA_STA_DWL_NG;
		fota_com_notify_result(&fota_context.reslut);
		fota_context.timeout = 0;
		fota_com_reset();
	}
}

static VOID fota_com_proc_downliad_finish(VOID)
{	
	fota_context.timeout++;
	if(fota_context.timeout >= FOTA_DOWNL_FINISH)
	{
		fota_context.reslut.rst = FOTA_RST_OK;
		fota_context.reslut.err_code = FOTA_STA_DWL_OK;
		fota_com_notify_result(&fota_context.reslut);
		fota_context.state = FOTA_COM_CHECKSUM;
		fota_context.timeout = 0;
	}
	
}

static VOID fota_com_proc_checksum(VOID)
{	
	MODULE_LOG_I(FOTA , "download finish !!! flash data len:%d" , fota_write_addr - FLASH_MCU_ADDR_FOTA_DATA);
	MODULE_LOG_I(FOTA , "crc: %08x, %08x" , fota_context.crc, fota_com_crc);
	
	fota_context.state = FOTA_COM_HANDLE_RESET;
	if(fota_context.crc != fota_com_crc)
	{
		fota_context.reslut.rst = FOTA_RST_NG;
		fota_context.reslut.err_code = FOTA_STA_CHK_NG;
		fota_com_notify_result(&fota_context.reslut);
		fota_com_reset();
	}
	else
	{
		fota_context.reslut.rst = FOTA_RST_OK;
		fota_context.reslut.err_code = FOTA_STA_CHK_OK;
		fota_com_notify_result(&fota_context.reslut);
		fota_context.state = FOTA_COM_HANDLE_RESET;
		fota_context.timeout = 0;
	}
}

static VOID fota_com_proc_handle_reset(VOID)
{
	INT32 ret = 0;
	fota_context.timeout++;
	if(fota_context.timeout >= FOTA_RESET_DELAY)
	{
		fota_context.state = FOTA_COM_HANDLE_RESULT;		
		fota_context.timeout = 0;
		fota_com_write_context();
		ret = program_start(FLASH_MCU_ADDR_FOTA_DATA, fota_context.file_size-4, fota_context.crc);
		if(0 != ret)
		{
			MODULE_LOG_E(FOTA , "progream failed, ret: %d" , ret);
			fota_context.reslut.rst = FOTA_RST_NG;
			fota_context.reslut.err_code = FOTA_STA_OTA_NG;
			fota_com_notify_result(&fota_context.reslut);
			fota_com_reset();
		}
	}
}

static VOID fota_com_proc_handle_result(VOID)
{	
	fota_context.timeout++;
	if(fota_context.timeout >= FOTA_RESULT_DELAY)
	{
		fota_context.reslut.rst = FOTA_RST_OK;
		fota_context.reslut.err_code = FOTA_STA_OTA_OK;
		fota_com_notify_result(&fota_context.reslut);
		fota_context.timeout = 0;
		fota_com_reset();
	}
}

static fota_com_process_func fota_com_process[] =
{
	fota_com_proc_idle,
	fota_com_proc_download,
	fota_com_proc_downliading,
	fota_com_proc_downliad_finish,
	fota_com_proc_checksum,
	fota_com_proc_handle_reset,
	fota_com_proc_handle_result,
};

VOID fota_com_timeout_proc(VOID)
{
	MODULE_LOG_I(FOTA, "fota sta: %d \r\n", fota_context.state);
	fota_com_process[fota_context.state]();
}

static VOID fota_com_write_context(VOID)
{
	INT32 ret = 0;
	
	ret = tbox_cfg_setkv(FOTA_CONTEXT_NAME, &fota_context, sizeof(fota_context));
	if(0 != ret)
	{
		MODULE_LOG_E(FOTA, "fota write context info failed, ret: %d", ret);
	}
}

static VOID fota_com_read_context(VOID)
{
	INT32 ret = 0;
	ret = tbox_cfg_getkv(FOTA_CONTEXT_NAME, &fota_context, sizeof(fota_context));
	if(0 != ret)
	{
		fota_context.magic = FOTA_MAGIC_NO;
		fota_com_write_context();
	}
	else
	{
		if(FOTA_MAGIC_NO != fota_context.magic)
		{
			fota_context.magic     = FOTA_MAGIC_NO;			
			fota_com_write_context();
		}
	}
}

INT32 fota_com_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			fota_write_addr = 0;
			fota_com_crc = CRC_INITIAL_VALUE32;
			fota_first_crc_call = TRUE;
			memset(&fota_context, 0, sizeof(fota_context));
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			fota_com_read_context();
            break;
            
        default:
            break;
    }
	
	return 0;
}

VOID fota_com_wake(VOID)
{
	fota_com_reset();
}

VOID fota_com_sleep(VOID)
{
}

static INT32 fota_com_ftp_info_parse(const CHAR *url, CHAR *ver, INT32 ver_size, INT32 *file_sz)
{
	INT32 ret = 0;
	
	// 查找最后一个'/'位置
	const char *last_slash = strrchr(url, '/');
	if (last_slash == NULL)
	{
		return -1;
	}
	
	// 提取文件名（最后一个'/'后的内容）
	const char *filename = last_slash + 1;
	
	// 从文件名中提取文件大小（数字部分）
	const char *num_start = filename;
	const char *num_end = filename + strlen(filename);
	
	// 跳过非数字字符，找到第一个数字
	while (num_start < num_end && !isdigit(*num_start))
	{
		num_start++;
	}
	
	// 提取连续的数字序列
	const char *num_ptr = num_start;
	while (num_ptr < num_end && isdigit(*num_ptr))
	{
		num_ptr++;
	}
	
	if (num_start < num_ptr)
	{
		char file_sz_str[32] = {0};
		UINT16 sz_len = num_ptr - num_start;
		if (sz_len > 0 && sz_len < sizeof(file_sz_str))
		{
			memcpy(file_sz_str, num_start, sz_len);
			file_sz_str[sz_len] = '\0';
			*file_sz = atoi(file_sz_str);
		}
	}
	
	// 提取版本号（最后一个'/'前的内容）
	const char *ver_start = last_slash;
	const char *ver_end = last_slash;
	
	// 向前查找版本号开始位置（上一个'/'或字符串开头）
	while (ver_start > url && *(ver_start-1) != '/')
	{
		ver_start--;
	}
	
	if (ver_start < ver_end)
	{
		UINT16 ver_len = ver_end - ver_start;
		if (ver_len > 0 && ver_len < ver_size)
		{
			memcpy(ver, ver_start, ver_len);
			ver[ver_len] = '\0';
		}
		else
		{
			ret = -1;
		}
	}
	else
	{
		ret = -1;
	}
	
	return ret;
}


static BOOL fota_com_check_version(CHAR *ver)
{
	if(0 == strncmp(version_get(VERSION_TYPE_APP), ver, strlen(version_get(VERSION_TYPE_APP))))
	{
		return FALSE;
	}

	return TRUE;
}

INT32 fota_do_upgrade(UINT8 *url, UINT16 url_len, INT32 id, UINT16 seq)
{	
	INT32 ret = 0;
	char  ver[128] = {0};
	INT32 file_sz = 0;
	FOTA_RESULT_INFO_T fota_result_info;

	//解析版本号, 文件大小
	ret = fota_com_ftp_info_parse((CHAR *)url, ver, sizeof(ver), &file_sz);
	if(0 != ret)
	{
		MODULE_LOG_E(FOTA, "fota parse ftp info failed, ret: %d", ret);
		fota_result_info.id = id;
		fota_result_info.seq = seq;
	    fota_result_info.rst = FOTA_RST_NG;
	    fota_result_info.err_code = FOTA_STA_CMD_NG;
	    fota_com_notify_result(&fota_result_info);
		return -1;
	}
	
	if(FOTA_COM_IDLE != fota_context.state)
	{
		MODULE_LOG_E(FOTA, "fota is in the upgrading, state: %d", fota_context.state);		
	    fota_result_info.id = id;
		fota_result_info.seq = seq;
	    fota_result_info.rst = FOTA_RST_NG;
	    fota_result_info.err_code = FOTA_STA_CMD_NG;
	    fota_com_notify_result(&fota_result_info);
		return -1;
	}

	if(FALSE == fota_com_check_version(ver))
	{
		MODULE_LOG_E(FOTA, "version check is same, ver: %s", ver);
	    fota_result_info.id = id;
		fota_result_info.seq = seq;
	    fota_result_info.rst = FOTA_RST_OK;
	    fota_result_info.err_code = FOTA_STA_OTA_OK;
	    fota_com_notify_result(&fota_result_info);
		return -1;
	}

	memset(&fota_context, 0, sizeof(fota_context));
	fota_context.magic = FOTA_MAGIC_NO;
	fota_context.state = FOTA_COM_DOWNLOAD;
	fota_context.timeout = 0;
	fota_context.file_size = file_sz;
	fota_context.url_len = url_len;
	memcpy(fota_context.url, url, url_len);	
	fota_context.tick = time_if_get_systick_s();	
	fota_context.reslut.id  = id;
	fota_context.reslut.seq = seq;
	
	fota_com_crc = CRC_INITIAL_VALUE32;
	fota_first_crc_call = TRUE;
		
    return 0;
}

INT32 fota_do_upgrade_with_info(UINT8 *url, UINT16 url_len, INT32 id, UINT16 seq, CHAR *ver, INT32 file_sz)
{
	FOTA_RESULT_INFO_T fota_result_info;

	if((NULL == url) || (url_len <= 0) || (NULL == ver) || (file_sz <= 0))
	{
		MODULE_LOG_E(FOTA, "fota param err");		
		fota_result_info.id = id;
		fota_result_info.seq = seq;
	    fota_result_info.rst = FOTA_RST_NG;
	    fota_result_info.err_code = FOTA_STA_CMD_NG;
	    fota_com_notify_result(&fota_result_info);
	}
	
	if(FOTA_COM_IDLE != fota_context.state)
	{
		MODULE_LOG_E(FOTA, "fota is in the upgrading, state: %d", fota_context.state);		
	    fota_result_info.id = id;
		fota_result_info.seq = seq;
	    fota_result_info.rst = FOTA_RST_NG;
	    fota_result_info.err_code = FOTA_STA_CMD_NG;
	    fota_com_notify_result(&fota_result_info);
		return -1;
	}

	if(FALSE == fota_com_check_version(ver))
	{
		MODULE_LOG_E(FOTA, "version check is same, ver: %s", ver);
	    fota_result_info.id = id;
		fota_result_info.seq = seq;
	    fota_result_info.rst = FOTA_RST_OK;
	    fota_result_info.err_code = FOTA_STA_OTA_OK;
	    fota_com_notify_result(&fota_result_info);
		return -1;
	}

	memset(&fota_context, 0, sizeof(fota_context));
	fota_context.magic = FOTA_MAGIC_NO;
	fota_context.state = FOTA_COM_DOWNLOAD;
	fota_context.timeout = 0;
	fota_context.file_size = file_sz;
	fota_context.url_len = url_len;
	memcpy(fota_context.url, url, url_len);	
	fota_context.tick = time_if_get_systick_s();	
	fota_context.reslut.id  = id;
	fota_context.reslut.seq = seq;
	fota_context.reslut.rst = FOTA_RST_OK;
	fota_context.reslut.err_code = FOTA_STA_CMD_OK;	
	fota_com_notify_result(&fota_context.reslut);
	
	fota_com_crc = CRC_INITIAL_VALUE32;
	fota_first_crc_call = TRUE;
		
    return 0;
}


VOID fota_com_info_dump(VOID)
{
	tbox_log_print("=== FOTA Context Dump ===\r\n");
	tbox_log_print("magic: 0x%08X\r\n", fota_context.magic);
	tbox_log_print("state: %u\r\n", fota_context.state);
	tbox_log_print("crc: 0x%08X\r\n", fota_context.crc);
	tbox_log_print("timeout: %u\r\n", fota_context.timeout);
	tbox_log_print("tick: %u\r\n", fota_context.tick);
	tbox_log_print("file_size: %u\r\n", fota_context.file_size);
	tbox_log_print("url_len: %u\r\n", fota_context.url_len);
	tbox_log_print("url: %.*s\r\n", fota_context.url_len, (char *)fota_context.url);
	tbox_log_print("result.id: %d\r\n", fota_context.reslut.id);
	tbox_log_print("result.seq: %u\r\n", fota_context.reslut.seq);
	tbox_log_print("result.rst: %d\r\n", fota_context.reslut.rst);
	tbox_log_print("result.err_code: %d\r\n", fota_context.reslut.err_code);
}

VOID fota_com_reset(VOID)
{
	fota_write_addr = 0;
	fota_com_crc = 0xFFFFFFFF;
	fota_first_crc_call = TRUE;
	memset(&fota_context, 0, sizeof(fota_context));
	fota_context.magic = FOTA_MAGIC_NO;	
	fota_com_write_context();
}

INT32 fota_com_get_state(VOID)
{
	return fota_context.state;
}

