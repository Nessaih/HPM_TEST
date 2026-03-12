#include <stddef.h>
#include "spi_sdcard.h"

#define SD_STATIC_CAPACITY 1
#define SD_SECTOR_SIZE     512 // 扇区大小（字节）

#if SD_STATIC_CAPACITY
#define SD_TOTAL_SECTORS 242688                              // SD卡总扇区数
#define SD_TOTAL_SIZE    (SD_TOTAL_SECTORS * SD_SECTOR_SIZE) // SD卡总大小（字节）
#else
#define SD_TOTAL_SIZE drv_flash_sd_sector_size
#endif

static uint32_t drv_flash_sd_sector_size;
static uint8_t  drv_flash_sd_buffer[SD_SECTOR_SIZE];
static uint8_t  drv_flash_sd_is_busy = 0;
static uint8_t  drv_flash_sd_is_init = 0;

int32_t drv_flash_sd_init(void)
{
    uint32_t count;

    if (drv_flash_sd_is_init)
        return 0;

    if (sd_init() != SD_OK)
        return -1;

    count = sd_get_sector_count();
    if (count == 0)
        return -2;

    drv_flash_sd_sector_size = count * SD_SECTOR_SIZE;
    drv_flash_sd_is_init     = 1;

    return 0;
}

int32_t drv_flash_sd_sleep(void)
{
    return 0;
}

int32_t drv_flash_sd_wake(void)
{
    return 0;
}

int32_t drv_flash_sd_get_id(uint32_t *id)
{
    uint8_t cid[16] = {0};

    if (SD_OK != sd_get_cid(cid))
        return -1;

    *id = cid[15] | cid[14] << 8 | cid[13] << 16;

    return 0;
}

int32_t drv_flash_sd_get_size(uint32_t *size)
{
    *size = drv_flash_sd_sector_size;
    return 0;
}

/**
 * @brief       读取SD卡数据，支持地址不对齐和跨扇区读写
 * @param       addr     : 起始地址
 * @param       data     : 数据缓冲区
 * @param       data_len : 数据长度
 * @retval      0        : 成功
 *              其他值  : 失败
 */
int32_t drv_flash_sd_read(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    int32_t ret = 0;

    // 检查FLASH是否忙

    if (!drv_flash_sd_is_init)
    {
        return -1; // FLASH未初始化
    }

    if (drv_flash_sd_is_busy)
    {
        return -2; // FLASH忙
    }

    // 参数合法性检查
    if (data == NULL)
    {
        return -3;
    }
    if (addr + data_len > SD_TOTAL_SIZE)
    {
        return -4;
    }

    // 标记FLASH为忙
    drv_flash_sd_is_busy = 1;

    uint32_t start_sector = addr / SD_SECTOR_SIZE;
    uint32_t end_sector   = (addr + data_len - 1) / SD_SECTOR_SIZE;
    uint32_t start_offset = addr % SD_SECTOR_SIZE;
    uint32_t end_offset   = (addr + data_len - 1) % SD_SECTOR_SIZE;
    uint32_t sector_count = end_sector - start_sector + 1;
    uint8_t *buffer       = drv_flash_sd_buffer;
    uint32_t read_len     = 0;
    uint32_t i;

    if (sector_count == 1)
    {
        // 单个扇区内读取
        if (sd_read_disk(buffer, start_sector, 1) != SD_OK)
        {
            ret = -5;
            goto exit;
        }
        for (i = 0; i < data_len; i++)
        {
            data[i] = buffer[start_offset + i];
        }
    }
    else
    {
        // 跨扇区读取
        // 读取第一个扇区
        if (sd_read_disk(buffer, start_sector, 1) != SD_OK)
        {
            ret = -6;
            goto exit;
        }
        for (i = 0; i < SD_SECTOR_SIZE - start_offset; i++)
        {
            data[read_len++] = buffer[start_offset + i];
        }

        // 读取中间的完整扇区
        if (end_sector - start_sector > 1)
        {
            if (sd_read_disk(data + read_len, start_sector + 1, end_sector - start_sector - 1) != SD_OK)
            {
                ret = -7;
                goto exit;
            }
            read_len += (end_sector - start_sector - 1) * SD_SECTOR_SIZE;
        }

        // 读取最后一个扇区
        if (sd_read_disk(buffer, end_sector, 1) != SD_OK)
        {
            ret = -8;
            goto exit;
        }
        for (i = 0; i <= end_offset; i++)
        {
            data[read_len++] = buffer[i];
        }
    }

    ret = 0;

exit:
    // 标记FLASH为空闲
    drv_flash_sd_is_busy = 0;
    return ret;
}

/**
 * @brief       写入SD卡数据，支持地址不对齐和跨扇区读写
 * @param       addr     : 起始地址
 * @param       data     : 数据缓冲区
 * @param       data_len : 数据长度
 * @retval      0        : 成功
 *              其他值  : 失败
 */
int32_t drv_flash_sd_write(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    int32_t ret = 0;

    if (!drv_flash_sd_is_init)
    {
        return -1; // FLASH未初始化
    }

    // 检查FLASH是否忙
    if (drv_flash_sd_is_busy)
    {
        return -2; // FLASH忙
    }

    // 参数合法性检查
    if (data == NULL)
    {
        return -3;
    }
    if (addr + data_len > SD_TOTAL_SIZE)
    {
        return -4;
    }

    // 标记FLASH为忙
    drv_flash_sd_is_busy = 1;

    uint32_t start_sector = addr / SD_SECTOR_SIZE;
    uint32_t end_sector   = (addr + data_len - 1) / SD_SECTOR_SIZE;
    uint32_t start_offset = addr % SD_SECTOR_SIZE;
    uint32_t end_offset   = (addr + data_len - 1) % SD_SECTOR_SIZE;
    uint32_t sector_count = end_sector - start_sector + 1;
    uint8_t *buffer       = drv_flash_sd_buffer;
    uint32_t write_len    = 0;
    uint32_t i;

    if (sector_count == 1)
    {
        // 单个扇区内写入
        if (sd_read_disk(buffer, start_sector, 1) != SD_OK)
        {
            ret = -5;
            goto exit;
        }
        for (i = 0; i < data_len; i++)
        {
            buffer[start_offset + i] = data[i];
        }
        if (sd_write_disk(buffer, start_sector, 1) != SD_OK)
        {
            ret = -6;
            goto exit;
        }
    }
    else
    {
        // 跨扇区写入
        // 写入第一个扇区
        if (sd_read_disk(buffer, start_sector, 1) != SD_OK)
        {
            ret = -7;
            goto exit;
        }
        for (i = 0; i < SD_SECTOR_SIZE - start_offset; i++)
        {
            buffer[start_offset + i] = data[write_len++];
        }
        if (sd_write_disk(buffer, start_sector, 1) != SD_OK)
        {
            ret = -8;
            goto exit;
        }

        // 写入中间的完整扇区
        if (end_sector - start_sector > 1)
        {
            if (sd_write_disk(data + write_len, start_sector + 1, end_sector - start_sector - 1) != SD_OK)
            {
                ret = -9;
                goto exit;
            }
            write_len += (end_sector - start_sector - 1) * SD_SECTOR_SIZE;
        }

        // 写入最后一个扇区
        if (sd_read_disk(buffer, end_sector, 1) != SD_OK)
        {
            ret = -10;
            goto exit;
        }
        for (i = 0; i <= end_offset; i++)
        {
            buffer[i] = data[write_len++];
        }
        if (sd_write_disk(buffer, end_sector, 1) != SD_OK)
        {
            ret = -11;
            goto exit;
        }
    }

    ret = 0;

exit:
    // 标记FLASH为空闲
    drv_flash_sd_is_busy = 0;
    return ret;
}