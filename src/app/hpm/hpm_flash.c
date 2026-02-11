#include "tbox_common.h"
#include "drv_flash_nor.h"
#include "checksum.h"
#include "tbox_log.h"
#include "tbox_memory.h"
#include "flash_common.h"

#include "hpm_flash.h"

#define HPM_REPO_FLASH_MAGICNO    (0x0909)
#define HPM_REPO_FLASH_VER        (0x1002)
#define HPM_REPO_FLASH_MAX_SECTOR (0x0180)   /*384*4K*/
#define HPM_REPO_FLASH_PACKET_LEN (512)    /*header+data*/
#define HPM_REPO_FLASH_DATA_LEN   (HPM_REPO_FLASH_PACKET_LEN-sizeof(HPM_REPO_FLASH_HEADER))
#define HPM_REPO_FLASH_MAX_PACKET (8)      /*4K=8(PACKET)*512*/
#define HPM_REPO_FLASH_SECTOR_BIT (48)      /*48*8=384, 0:no erase 1:erased*/
#define HPM_REPO_CALC_ADDR(SECTOR, PACKET) (FLASH_NOR_ADDR_HPM_DATA+SECTOR*4*1024+PACKET*512)
#define HPM_REPO_GET_SECTOR_BIT(SECTOR) (hpm_flash_mgrinfo.sector_bit[SECTOR/8] & (1 << (SECTOR%8)))
#define HPM_REPO_SET_SECTOR_BIT(SECTOR) (hpm_flash_mgrinfo.sector_bit[SECTOR/8] |= (1 << (SECTOR%8)))
#define HPM_REPO_CLEAR_SECTOR_BIT(SECTOR) (hpm_flash_mgrinfo.sector_bit[SECTOR/8] &= ~(1 << (SECTOR%8)))
#define HPM_REPO_CALC_SECTOR_ADDR(SECTOR) (FLASH_NOR_ADDR_HPM_DATA+SECTOR*4*1024)

typedef struct
{
    UINT16 sector_num;
    UINT8  packet_num;
}HPM_REPO_FLASH_POS;

typedef struct
{
    UINT16 magic_no;
    UINT16 ver;
    HPM_REPO_FLASH_POS read_pos;
    HPM_REPO_FLASH_POS write_pos;
    UINT8 sector_bit[HPM_REPO_FLASH_SECTOR_BIT];
}HPM_REPO_FLASH_MGR;

typedef struct
{
    UINT16 len;
    UINT8 cmd;
    UINT8 check_sum;
}HPM_REPO_FLASH_HEADER;

typedef enum
{
    HPM_FLASH_ERR_OK = 0x00,
    HPM_FLASH_ERR_NG
}GB4_ERR_CODE;

static HPM_REPO_FLASH_MGR hpm_flash_mgrinfo;
static HPM_REPO_FLASH_MGR hpm_flash_mgrinfo_tmp;
static UINT8  hpm_flash_write_flag;
static UINT32 hpm_flash_tick = 0;

static UINT8 hpm_flash_load_mgrinfo(VOID)
{
    UINT8* data = (UINT8*)&hpm_flash_mgrinfo;
    UINT16 len = sizeof(hpm_flash_mgrinfo);

    if (HPM_FLASH_ERR_OK != drv_flash_nor_read(FLASH_NOR_ADDR_HPM_MGR, data, len))
    {
        MODULE_LOG_E(HPM, "read flash info error ");
        return HPM_FLASH_ERR_NG;
    }

    return HPM_FLASH_ERR_OK;
}

static VOID hpm_flash_reset_mgrinfo(VOID)
{
    memset(&hpm_flash_mgrinfo, 0, sizeof(hpm_flash_mgrinfo));
    hpm_flash_mgrinfo.magic_no = HPM_REPO_FLASH_MAGICNO;
    hpm_flash_mgrinfo.ver = HPM_REPO_FLASH_VER;
}

static UINT8 hpm_flash_save_mgrinfo(VOID)
{
    UINT8* data = (UINT8*)&hpm_flash_mgrinfo;
    UINT16 len = sizeof(hpm_flash_mgrinfo);

    if (0 != drv_flash_nor_erase(FLASH_NOR_ADDR_HPM_MGR, 1))
    {
        MODULE_LOG_E(HPM, "failed erase sector");
        return HPM_FLASH_ERR_NG;
    }

    if (HPM_FLASH_ERR_OK != drv_flash_nor_write(FLASH_NOR_ADDR_HPM_MGR, data, len))
    {
        MODULE_LOG_E(HPM, "hpm flash write info error ");
        return HPM_FLASH_ERR_NG;
    }

    return HPM_FLASH_ERR_OK;
}

static VOID  hpm_flash_move_next_read_sector(VOID)
{
    hpm_flash_mgrinfo.read_pos.packet_num = 0;
    hpm_flash_mgrinfo.read_pos.sector_num = hpm_flash_mgrinfo.read_pos.sector_num + 1;
    if (hpm_flash_mgrinfo.read_pos.sector_num >= HPM_REPO_FLASH_MAX_SECTOR)
    {
        hpm_flash_mgrinfo.read_pos.sector_num = 0;
    }

    MODULE_LOG_I(HPM, " read sector:(%d sector_num, %d packet_num)",
            hpm_flash_mgrinfo.read_pos.sector_num, hpm_flash_mgrinfo.read_pos.packet_num);
}

static VOID hpm_flash_move_next_read_pos(VOID)
{
    hpm_flash_mgrinfo.read_pos.packet_num = hpm_flash_mgrinfo.read_pos.packet_num + 1;
    if (hpm_flash_mgrinfo.read_pos.packet_num >= HPM_REPO_FLASH_MAX_PACKET)
    {
        hpm_flash_mgrinfo.read_pos.packet_num = 0;
        hpm_flash_mgrinfo.read_pos.sector_num = hpm_flash_mgrinfo.read_pos.sector_num + 1;
        if (hpm_flash_mgrinfo.read_pos.sector_num >= HPM_REPO_FLASH_MAX_SECTOR)
        {
            hpm_flash_mgrinfo.read_pos.sector_num = 0;
        }
    }
    MODULE_LOG_I(HPM, " read pos:(%d sector_num, %d packet_num)",
            hpm_flash_mgrinfo.read_pos.sector_num, hpm_flash_mgrinfo.read_pos.packet_num);
}

static VOID hpm_flash_move_next_write_pos(VOID)
{
    hpm_flash_mgrinfo.write_pos.packet_num = hpm_flash_mgrinfo.write_pos.packet_num + 1;
    if (hpm_flash_mgrinfo.write_pos.packet_num >= HPM_REPO_FLASH_MAX_PACKET)
    {
        hpm_flash_mgrinfo.write_pos.packet_num = 0;
        hpm_flash_mgrinfo.write_pos.sector_num = hpm_flash_mgrinfo.write_pos.sector_num + 1;
        if (hpm_flash_mgrinfo.write_pos.sector_num >= HPM_REPO_FLASH_MAX_SECTOR)
        {
            hpm_flash_mgrinfo.write_pos.sector_num = 0;
        }
    }
    MODULE_LOG_I(HPM, " write pos:(%d sector_num, %d packet_num)",
            hpm_flash_mgrinfo.write_pos.sector_num, hpm_flash_mgrinfo.write_pos.packet_num);
}

static UINT8 hpm_flash_read(UINT8* data, UINT16 len)
{
    uint32 addr = HPM_REPO_CALC_ADDR(hpm_flash_mgrinfo.read_pos.sector_num, hpm_flash_mgrinfo.read_pos.packet_num);

    if (addr >= (FLASH_NOR_ADDR_HPM_DATA + FLASH_NOR_SIZE_HPM_DATA))
    {
        MODULE_LOG_E(HPM, "the upmsg manager is overflow, read data from (0 sector_num, 0 packet_num)");
        hpm_flash_mgrinfo.read_pos.packet_num = 0;
        hpm_flash_mgrinfo.read_pos.sector_num = 0;
        addr = HPM_REPO_CALC_ADDR(hpm_flash_mgrinfo.read_pos.sector_num, hpm_flash_mgrinfo.read_pos.packet_num);
    }

    if (0 != drv_flash_nor_read(addr, data, len))
    {
        MODULE_LOG_E(HPM, "failed to read data from flash");
        return 1;
    }

    return 0;
}

static UINT8 hpm_flash_write(UINT8* data, UINT16 len)
{
    uint32 addr = HPM_REPO_CALC_ADDR(hpm_flash_mgrinfo.write_pos.sector_num, hpm_flash_mgrinfo.write_pos.packet_num);
    UINT8 secort_bit = HPM_REPO_GET_SECTOR_BIT(hpm_flash_mgrinfo.write_pos.sector_num);

    if (addr >= (FLASH_NOR_ADDR_HPM_DATA + FLASH_NOR_SIZE_HPM_DATA))
    {
        MODULE_LOG_I(HPM, "the upmsg manager is overflow, write data from (0 sector_num, 0 packet_num)");
        hpm_flash_mgrinfo.write_pos.packet_num = 0;
        hpm_flash_mgrinfo.write_pos.sector_num = 0;
        addr = HPM_REPO_CALC_ADDR(hpm_flash_mgrinfo.write_pos.sector_num, hpm_flash_mgrinfo.write_pos.packet_num);
        secort_bit = HPM_REPO_GET_SECTOR_BIT(hpm_flash_mgrinfo.write_pos.sector_num);
    }

    if (0 == secort_bit && 0 == hpm_flash_mgrinfo.write_pos.packet_num)
    {
        if (0 != drv_flash_nor_erase(HPM_REPO_CALC_SECTOR_ADDR(hpm_flash_mgrinfo.write_pos.sector_num), 1))
        {
            MODULE_LOG_E(HPM, "failed erase sector");
            return 1;
        }

        HPM_REPO_SET_SECTOR_BIT(hpm_flash_mgrinfo.write_pos.sector_num);
        if (hpm_flash_mgrinfo.read_pos.sector_num == hpm_flash_mgrinfo.write_pos.sector_num &&
            hpm_flash_mgrinfo.read_pos.packet_num > hpm_flash_mgrinfo.write_pos.packet_num)
        {
            hpm_flash_move_next_read_sector();
        }
    }

    if (0 != drv_flash_nor_write(addr, data, len))
    {
        MODULE_LOG_E(HPM, "failed to write data to flash");
        return 1;
    }

    if (hpm_flash_mgrinfo.write_pos.packet_num == (HPM_REPO_FLASH_MAX_PACKET - 1))
    {
        HPM_REPO_CLEAR_SECTOR_BIT(hpm_flash_mgrinfo.write_pos.sector_num);
    }

    hpm_flash_move_next_write_pos();

    if (hpm_flash_mgrinfo.read_pos.sector_num == hpm_flash_mgrinfo.write_pos.sector_num &&
        hpm_flash_mgrinfo.read_pos.packet_num == hpm_flash_mgrinfo.write_pos.packet_num)
    {
        hpm_flash_move_next_read_pos();
    }

    return 0;
}

VOID hpm_flash_deinit(VOID)
{
    hpm_flash_save_mgrinfo();
    hpm_flash_tick = 0;
}

UINT8 hpm_flash_save_pack(UINT8* data, UINT16 len, UINT8 cmd)
{
    UINT8 ret = HPM_FLASH_ERR_OK;
    HPM_REPO_FLASH_HEADER header;

    if (hpm_flash_mgrinfo.write_pos.sector_num >= HPM_REPO_FLASH_MAX_SECTOR ||
        hpm_flash_mgrinfo.write_pos.packet_num >= HPM_REPO_FLASH_MAX_PACKET)
    {
        MODULE_LOG_E(HPM, "the write pos is invalid");
        hpm_flash_reset_mgrinfo();
    }

    if (len + sizeof(HPM_REPO_FLASH_HEADER) > HPM_REPO_FLASH_PACKET_LEN)
    {
        MODULE_LOG_E(HPM, "the write buf is overflow,ignore.");
        return HPM_FLASH_ERR_NG;
    }

    UINT8* buf = mempool_alloc(HPM_REPO_FLASH_PACKET_LEN);
    if (NULL == buf)
    {
        MODULE_LOG_E(HPM, "hpm flash malloc data failded.");
        return HPM_FLASH_ERR_NG;
    }
	memset(buf, 0xFF, HPM_REPO_FLASH_PACKET_LEN);

    header.check_sum = xor_checksum(data, len);
    header.len = len;
    header.cmd = cmd;
    memcpy(buf, &header, sizeof(header));
    memcpy(buf + sizeof(header), data, len);

    if (HPM_FLASH_ERR_OK != hpm_flash_write(buf, HPM_REPO_FLASH_PACKET_LEN))
    {
        ret = HPM_FLASH_ERR_NG;
    }
    else
    {
        ret = HPM_FLASH_ERR_OK;
        //hpm_flash_write_flag = 1;
    }

    mempool_free(buf);

    return ret;
}

UINT8 hpm_flash_load_pack(UINT8* data, UINT16 *len, UINT8* cmd)
{
    UINT8 ret = HPM_FLASH_ERR_OK;
    HPM_REPO_FLASH_HEADER *header;

    if (hpm_flash_mgrinfo.write_pos.sector_num >= HPM_REPO_FLASH_MAX_SECTOR ||
        hpm_flash_mgrinfo.write_pos.packet_num >= HPM_REPO_FLASH_MAX_PACKET)
    {
        MODULE_LOG_E(HPM, "the read pos is invalid");
        hpm_flash_reset_mgrinfo();
        hpm_flash_write_flag = 1;
        return HPM_FLASH_ERR_NG;
    }

    if (hpm_flash_mgrinfo.write_pos.sector_num == hpm_flash_mgrinfo.read_pos.sector_num &&
        hpm_flash_mgrinfo.write_pos.packet_num == hpm_flash_mgrinfo.read_pos.packet_num)
    {
        //MODULE_LOG_E(HPM, "no data in flash");
        return HPM_FLASH_ERR_NG;
    }

    UINT8* buf = mempool_alloc(HPM_REPO_FLASH_PACKET_LEN);
    if (NULL == buf)
    {
        MODULE_LOG_E(HPM, "hpm flash malloc data failded.");
        return HPM_FLASH_ERR_NG;
    }
	memset(buf, 0xFF, HPM_REPO_FLASH_PACKET_LEN);

    if (HPM_FLASH_ERR_OK != hpm_flash_read(buf, HPM_REPO_FLASH_PACKET_LEN))
    {
        MODULE_LOG_E(HPM, "failed to read data");
        mempool_free(buf);
        return HPM_FLASH_ERR_NG;
    }

    header = (HPM_REPO_FLASH_HEADER*)buf;
    if (header->len > (HPM_REPO_FLASH_PACKET_LEN - sizeof(HPM_REPO_FLASH_HEADER)))
    {
        MODULE_LOG_E(HPM, "the data is invalid");
        hpm_flash_move_next_read_pos();
        hpm_flash_write_flag = 1;
        mempool_free(buf);
        return HPM_FLASH_ERR_NG;
    }

    if (header->check_sum != xor_checksum(buf + sizeof(HPM_REPO_FLASH_HEADER), header->len))
    {
        MODULE_LOG_E(HPM, "the check sum is invalid");
        hpm_flash_move_next_read_pos();
        hpm_flash_write_flag = 1;
        mempool_free(buf);
        return HPM_FLASH_ERR_NG;
    }

    *cmd = header->cmd;
    *len = header->len;
    memcpy(data, buf + sizeof(HPM_REPO_FLASH_HEADER), header->len);

    mempool_free(buf);
    hpm_flash_move_next_read_pos();
//    hpm_flash_write_flag = 1;

    return ret;
}

VOID hpm_flash_init(VOID)
{
    hpm_flash_write_flag = 0;

    if(HPM_FLASH_ERR_OK == hpm_flash_load_mgrinfo())
    {
        if (HPM_REPO_FLASH_MAGICNO != hpm_flash_mgrinfo.magic_no
            || HPM_REPO_FLASH_VER != hpm_flash_mgrinfo.ver)
        {
            hpm_flash_reset_mgrinfo();
            hpm_flash_write_flag = 1;
        }
    }
    else
    {
        hpm_flash_reset_mgrinfo();
        hpm_flash_write_flag = 1;
    }
}

UINT8 hpm_flash_clear_info(VOID)
{
    hpm_flash_mgrinfo.read_pos.sector_num = 0;
    hpm_flash_mgrinfo.read_pos.packet_num = 0;
    hpm_flash_mgrinfo.write_pos.sector_num = 0;
    hpm_flash_mgrinfo.write_pos.packet_num = 0;
    hpm_flash_write_flag = 1;
    return 0;
}

VOID hpm_flash_period(VOID)
{
    if (1 == hpm_flash_write_flag)
    {
        if(HPM_FLASH_ERR_OK == hpm_flash_save_mgrinfo())
        {
            hpm_flash_write_flag = 0;
            hpm_flash_tick = 0;
            return;
        }
        else
        {
            MODULE_LOG_E(HPM, "hpm flash save mgrinfo failed.");
            return;
        }
    }

    if (hpm_flash_mgrinfo_tmp.magic_no != hpm_flash_mgrinfo.magic_no)
    {
        hpm_flash_mgrinfo_tmp = hpm_flash_mgrinfo;
    }

    hpm_flash_tick++;
    if (hpm_flash_tick >= 600) //10min
    {
        hpm_flash_tick = 0;
        if ((hpm_flash_mgrinfo_tmp.read_pos.sector_num != hpm_flash_mgrinfo.read_pos.sector_num) ||
           (hpm_flash_mgrinfo_tmp.read_pos.packet_num != hpm_flash_mgrinfo.read_pos.packet_num) ||
            (hpm_flash_mgrinfo_tmp.write_pos.sector_num != hpm_flash_mgrinfo.write_pos.sector_num) ||
            (hpm_flash_mgrinfo_tmp.write_pos.packet_num != hpm_flash_mgrinfo.write_pos.packet_num))
        {
            hpm_flash_write_flag = 1;
            hpm_flash_mgrinfo_tmp = hpm_flash_mgrinfo;
        }
    }
}

VOID hpm_flash_print_info(VOID)
{
    UINT16 write_sect_index = hpm_flash_mgrinfo.write_pos.sector_num;
    UINT8 write_pack_index = hpm_flash_mgrinfo.write_pos.packet_num;
    uint32 write_pos = write_sect_index * HPM_REPO_FLASH_MAX_PACKET + write_pack_index;

    UINT16 read_sect_index = hpm_flash_mgrinfo.read_pos.sector_num;
    UINT8 read_pack_index = hpm_flash_mgrinfo.read_pos.packet_num;
    uint32 read_pos = read_sect_index * HPM_REPO_FLASH_MAX_PACKET + read_pack_index;

    tbox_log_print("hpm_flash_write_pos  : %u\r\n", write_pos);
    tbox_log_print("hpm_flash_read_pos   : %u\r\n", read_pos);

    return;
}

