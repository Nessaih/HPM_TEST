

#include <string.h>
#include "fal.h"
#include "fal_cfg.h"
#include "tbox_core.h"

#define FAL_PART_MAGIC_WORD   0x45503130
#define FAL_PART_MAGIC_WORD_H 0x4550L
#define FAL_PART_MAGIC_WORD_L 0x3130L
#define FAL_PART_MAGIC_WROD   0x45503130

struct part_flash_info {
    const fal_flash_dev_t *flash_dev;
};

#ifdef FAL_PART_HAS_TABLE_CFG

#if !defined(FAL_PART_TABLE)
#error "You must defined FAL_PART_TABLE on 'fal_cfg.h'"
#endif

static const fal_partition_t  partition_table_def[] = FAL_PART_TABLE;
static const fal_partition_t *partition_table       = NULL;
static struct part_flash_info
    part_flash_cache[sizeof(partition_table_def) / sizeof(partition_table_def[0])] = {0};

#else
#if !defined(FAL_PART_TABLE_FLASH_DEV_NAME)
#error "You must defined FAL_PART_TABLE_FLASH_DEV_NAME on 'fal_cfg.h'"
#endif

#if !defined(FAL_PART_TABLE_END_OFFSET)
#error "You must defined FAL_PART_TABLE_END_OFFSET on 'fal_cfg.h'"
#endif

static fal_partition_t        *partition_table  = NULL;
static struct part_flash_info *part_flash_cache = NULL;
#endif
static uint8_t  init_ok             = 0;
static uint32_t partition_table_len = 0;

void fal_show_partition(void)
{
    const char            *item1 = "name", *item2 = "flash_dev";
    uint32_t               i, part_name_max = strlen(item1), flash_dev_name_max = strlen(item2);
    const fal_partition_t *part;

    if(partition_table_len) {
        for(i = 0; i < partition_table_len; i++) {
            part = &partition_table[i];
            if(strlen(part->name) > part_name_max) {
                part_name_max = strlen(part->name);
            }
            if(strlen(part->flash_name) > flash_dev_name_max) {
                flash_dev_name_max = strlen(part->flash_name);
            }
        }
    }
    LOG_PRINT("==================== FAL partition table ====================\n");
    LOG_PRINT("| %-*.*s | %-*.*s |   offset   |    length  |\n", part_name_max, FAL_DEV_NAME_MAX, item1,
                flash_dev_name_max, FAL_DEV_NAME_MAX, item2);
    LOG_PRINT("-------------------------------------------------------------\n");
    for(i = 0; i < partition_table_len; i++) {

#ifdef FAL_PART_HAS_TABLE_CFG
        part = &partition_table[i];
#else
        part = &partition_table[partition_table_len - i - 1];
#endif

        LOG_PRINT("| %-*.*s | %-*.*s | 0x%08lx | 0x%08x |\n", part_name_max, FAL_DEV_NAME_MAX,
                    part->name, flash_dev_name_max, FAL_DEV_NAME_MAX, part->flash_name, part->offset,
                    part->len);
    }
    LOG_PRINT("=============================================================\n");
}

static int32_t check_and_update_part_cache(const fal_partition_t *table, uint32_t len)
{
    const fal_flash_dev_t *flash_dev = NULL;
    uint32_t               i;

#ifndef FAL_PART_HAS_TABLE_CFG
    if(part_flash_cache) {
        rt_free(part_flash_cache);
    }
    part_flash_cache = rt_malloc(len * sizeof(struct part_flash_info));
    if(part_flash_cache == NULL) {
        MODULE_LOG_E(TBOXSVR, "Initialize failed! No memory for partition table cache");
        return -2;
    }
#endif

    for(i = 0; i < len; i++) {
        flash_dev = fal_find_flash(table[i].flash_name);
        if(flash_dev == NULL) {
            MODULE_LOG_I(TBOXSVR, "Warning: Do NOT found the flash device(%s).", table[i].flash_name);
            continue;
        }

        if(table[i].offset >= (long)flash_dev->len) {
           MODULE_LOG_E(TBOXSVR, "Initialize failed! Partition(%s) offset address(%ld) out of flash bound(<%d).",
                   table[i].name, table[i].offset, flash_dev->len);
            partition_table_len = 0;

            return -1;
        }

        part_flash_cache[i].flash_dev = flash_dev;
    }

    return 0;
}

int32_t fal_partition_init(void)
{
    if(init_ok) {
        return (int32_t)partition_table_len;
    }

#ifdef FAL_PART_HAS_TABLE_CFG
    partition_table     = &partition_table_def[0];
    partition_table_len = sizeof(partition_table_def) / sizeof(partition_table_def[0]);
#else
    long                   part_table_offset = FAL_PART_TABLE_END_OFFSET;
    uint32_t               table_num = 0, table_item_size = 0;
    uint8_t                part_table_find_ok = 0;
    uint32_t               read_magic_word;
    fal_partition_t       *new_part = NULL;
    uint32_t               i;
    const fal_flash_dev_t *flash_dev = NULL;

    flash_dev = fal_find_flash(FAL_PART_TABLE_FLASH_DEV_NAME);
    if(flash_dev == NULL) {
        MODULE_LOG_E(TBOXSVR, "Initialize failed! Flash device (%s) NOT found.", FAL_PART_TABLE_FLASH_DEV_NAME);
        goto _exit;
    }

    if(part_table_offset < 0 || part_table_offset >= (long)flash_dev->len) {
        MODULE_LOG_E(TBOXSVR, "Setting partition table end offset address(%ld) out of flash bound(<%d).",
               part_table_offset, flash_dev->len);
        goto _exit;
    }

    table_item_size = sizeof(fal_partition_t);
    new_part        = (fal_partition_t *)rt_malloc(table_item_size);
    if(new_part == NULL) {
        MODULE_LOG_E(TBOXSVR, "Initialize failed! No memory for table buffer.");
        goto _exit;
    }

    {
        uint8_t read_buf[64];

        part_table_offset -= sizeof(read_buf);
        while(part_table_offset >= 0) {
            if(flash_dev->ops.read(part_table_offset, read_buf, sizeof(read_buf)) > 0) {
                for(i = 0; i < sizeof(read_buf) - sizeof(read_magic_word) + 1; i++) {
                    read_magic_word = read_buf[0 + i] + (read_buf[1 + i] << 8) +
                                      (read_buf[2 + i] << 16) + (read_buf[3 + i] << 24);
                    if(read_magic_word == ((FAL_PART_MAGIC_WORD_H << 16) + FAL_PART_MAGIC_WORD_L)) {
                        part_table_find_ok = 1;
                        part_table_offset += i;
                        MODULE_LOG_I(TBOXSVR, "Find the partition table on '%s' offset @0x%08lx.",
                               FAL_PART_TABLE_FLASH_DEV_NAME, part_table_offset);
                        break;
                    }
                }
            } else {
                break;
            }

            if(part_table_find_ok) {
                break;
            } else {
                if(part_table_offset >= (long)sizeof(read_buf)) {
                    part_table_offset -= sizeof(read_buf);
                    part_table_offset += (sizeof(read_magic_word) - 1);
                } else if(part_table_offset != 0) {
                    part_table_offset = 0;
                } else {
                    break;
                }
            }
        }
    }

    while(part_table_find_ok) {
        memset(new_part, 0x00, table_num);
        if(flash_dev->ops.read(part_table_offset - table_item_size * (table_num),
                               (uint8_t *)new_part, table_item_size) < 0) {
            MODULE_LOG_E(TBOXSVR, "Initialize failed! Flash device (%s) read error!", flash_dev->name);
            table_num = 0;
            break;
        }

        if(new_part->magic_word != ((FAL_PART_MAGIC_WORD_H << 16) + FAL_PART_MAGIC_WORD_L)) {
            break;
        }

        partition_table =
            (fal_partition_t *)rt_realloc(partition_table, table_item_size * (table_num + 1));
        if(partition_table == NULL) {
            MODULE_LOG_E(TBOXSVR, "Initialize failed! No memory for partition table");
            table_num = 0;
            break;
        }

        memcpy(partition_table + table_num, new_part, table_item_size);

        table_num++;
    };

    if(table_num == 0) {
        MODULE_LOG_E(TBOXSVR, "Partition table NOT found on flash: %s (len: %d) from offset: 0x%08x.",
               FAL_PART_TABLE_FLASH_DEV_NAME, FAL_DEV_NAME_MAX, FAL_PART_TABLE_END_OFFSET);
        goto _exit;
    } else {
        partition_table_len = table_num;
    }
#endif
    if(check_and_update_part_cache(partition_table, partition_table_len) != 0) {
        goto _exit;
    }

    init_ok = 1;

_exit:

#ifdef FAL_USING_DEBUG
    fal_show_partition();
#endif
#ifndef FAL_PART_HAS_TABLE_CFG
    if(new_part) {
        rt_free(new_part);
    }
#endif
    return (int32_t)partition_table_len;
}

const fal_partition_t *fal_find_partition(const char *name)
{
    if(!init_ok)
        return NULL;

    uint32_t i;

    for(i = 0; i < partition_table_len; i++) {
        if(!strcmp(name, partition_table[i].name)) {
            return &partition_table[i];
        }
    }

    return NULL;
}

static const fal_flash_dev_t *flash_device_find_by_part(const fal_partition_t *part)
{
    return part_flash_cache[part - partition_table].flash_dev;
}

int32_t fal_read(const fal_partition_t *part, uint32_t addr, uint8_t *buf, uint32_t size)
{
    int32_t                ret       = 0;
    const fal_flash_dev_t *flash_dev = NULL;

    // RT_ASSERT(part);
    // RT_ASSERT(buf);

    if(addr + size > part->len) {
        MODULE_LOG_E(TBOXSVR, "Partition read error! Partition(%s) address(0x%08x) out of bound(0x%08x).",
               part->name, addr + size, part->len);
        return -1;
    }

    flash_dev = flash_device_find_by_part(part);
    if(flash_dev == NULL) {
        MODULE_LOG_E(TBOXSVR, "Partition read error! Don't found flash device(%s) of the partition(%s).",
               part->flash_name, part->name);
        return -1;
    }

    ret = flash_dev->ops.read(part->offset + addr, buf, size);
    if(ret < 0) {
        MODULE_LOG_E(TBOXSVR, "Partition read error! Flash device(%s) read error!", part->flash_name);
    }

    return ret;
}

int32_t fal_write(const fal_partition_t *part, uint32_t addr, const uint8_t *buf, uint32_t size)
{
    int32_t                ret       = 0;
    const fal_flash_dev_t *flash_dev = NULL;

    if(addr + size > part->len) {
        MODULE_LOG_E(TBOXSVR, "Partition write error! Partition address out of bound.");
        return -1;
    }

    flash_dev = flash_device_find_by_part(part);
    if(flash_dev == NULL) {
       MODULE_LOG_E(TBOXSVR, "Partition write error!  Don't found flash device(%s) of the partition(%s).",
               part->flash_name, part->name);
        return -1;
    }

    ret = flash_dev->ops.write(part->offset + addr, buf, size);
    if(ret < 0) {
        MODULE_LOG_E(TBOXSVR, "Partition write error! Flash device(%s) write error!", part->flash_name);
    }

    return ret;
}

int32_t fal_erase(const fal_partition_t *part, uint32_t addr, uint32_t size)
{
    int32_t                ret       = 0;
    const fal_flash_dev_t *flash_dev = NULL;

    // RT_ASSERT(part);

    if(addr + size > part->len) {
        MODULE_LOG_E(TBOXSVR, "Partition erase error! Partition address out of bound.");
        return -1;
    }

    flash_dev = flash_device_find_by_part(part);
    if(flash_dev == NULL) {
       MODULE_LOG_E(TBOXSVR, "Partition erase error! Don't found flash device(%s) of the partition(%s).",
               part->flash_name, part->name);
        return -1;
    }

    ret = flash_dev->ops.erase(part->offset + addr, size);
    if(ret < 0) {
       MODULE_LOG_E(TBOXSVR, "Partition erase error! Flash device(%s) erase error!", part->flash_name);
    }

    return ret;
}

int32_t fal_erase_whole(const fal_partition_t *part)
{
    return fal_erase(part, 0, part->len);
}
