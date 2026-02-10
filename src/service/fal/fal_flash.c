#include <string.h>
#include "fal_cfg.h"
#include "fal_def.h"
#include "tbox_core.h"

#if !defined(FAL_FLASH_DEV_TABLE)
#error "You must defined flash device table (FAL_FLASH_DEV_TABLE) on 'fal_cfg.h'"
#endif

static const fal_flash_dev_t *const device_table[] = FAL_FLASH_DEV_TABLE;
static const uint32_t device_table_len             = sizeof(device_table) / sizeof(device_table[0]);
static uint8_t        init_ok                      = 0;

// clang-format off
int32_t fal_flash_init(void)
{
    uint32_t i, j, offset;

    if(init_ok) {
        return 0;
    }

    for(i = 0; i < device_table_len; i++) {

        if(device_table[i]->ops.init) {
            device_table[i]->ops.init();
        }

        MODULE_LOG_I(TBOXSVR, "Flash device | %*.*s | addr: 0x%08lx | len: 0x%08x | blk_size: 0x%08x |initialized finish.", FAL_DEV_NAME_MAX, FAL_DEV_NAME_MAX,
                    device_table[i]->name, device_table[i]->addr, device_table[i]->len, device_table[i]->blk_size);

        offset = 0;
        for(j = 0; j < FAL_DEV_BLK_MAX; j++) {
            const struct flash_blk *blk     = &device_table[i]->blocks[j];
            uint32_t                blk_len = blk->count * blk->size;
            if(blk->count == 0 || blk->size == 0)
                break;

            if(offset > device_table[i]->len) {
                MODULE_LOG_I(TBOXSVR, "Flash device %*.*s: add block failed, offset %d > len %d.", FAL_DEV_NAME_MAX, FAL_DEV_NAME_MAX,
                            device_table[i]->name, device_table[i]->addr, offset, device_table[i]->len);
                break;
            }

            MODULE_LOG_I(TBOXSVR, "                  blk%2d | addr: 0x%08lx | len: 0x%08x | blk_size: 0x%08x |initialized finish.", j,
                        device_table[i]->addr + offset, blk_len, blk->size);

            offset += blk_len;
        }
    }

    init_ok = 1;
    return 0;
}

// clang-format on
const fal_flash_dev_t *fal_find_flash(const char *name)
{
    if (0 == strcmp(name, "onchip_pflash"))
        return &onchip_pflash;

    if (0 == strcmp(name, "onchip_dflash"))
        return &onchip_dflash;

    return NULL;
}
