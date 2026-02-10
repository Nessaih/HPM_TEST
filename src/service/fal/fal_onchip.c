#include <stdbool.h>
#include <string.h>
#include "delay.h"
#include "drv_flash_mcu.h"
#include "fal_cfg.h"
#include "fal_def.h"
#include "tbox_core.h"


static int init(void)
{
    return drv_flash_mcu_init();
}

static int pflash_read(int32_t offset, uint8_t *buf, uint32_t size)
{
    int32_t ret;
    ret = drv_flash_mcu_read(onchip_pflash.addr + offset, buf, size);
    if (0 != ret)
        return ret;

    return size;
}

static int pflash_write(int32_t offset, const uint8_t *buf, uint32_t size)
{
    int32_t ret;
    ret = drv_flash_mcu_write(onchip_pflash.addr + offset, buf, size);
    if (0 != ret)
        return ret;
    return size;
}

static int  pflash_erase(int32_t offset, uint32_t size)
{
    int32_t ret;
    ret = drv_flash_mcu_erase(onchip_pflash.addr + offset, size);
    if (0 != ret)
        return ret;
    return size;
}


static int dflash_read(int32_t offset, uint8_t *buf, uint32_t size)
{
    int32_t ret;
    ret = drv_flash_mcu_read(onchip_dflash.addr + offset, buf, size);
    if (0 != ret)
        return ret;

    return size;
}

static int dflash_write(int32_t offset, const uint8_t *buf, uint32_t size)
{
    int32_t ret;
    ret = drv_flash_mcu_write(onchip_dflash.addr + offset, buf, size);
    if (0 != ret)
        return ret;
    return size;
}

static int  dflash_erase(int32_t offset, uint32_t size)
{
    int32_t ret;
    ret = drv_flash_mcu_erase(onchip_dflash.addr + offset, size);
    if (0 != ret)
        return ret;
    return size;
}

const fal_flash_dev_t onchip_pflash = {
    .name       = "onchip_pflash",
    .addr       = 0x00000000,
    .len        = 0x00200000,
    .blk_size   = 0x00002000,
    .write_gran = 0x00000040,
    .ops        = {init, pflash_read, pflash_write, pflash_erase}
};

const fal_flash_dev_t onchip_dflash = {
    .name       = "onchip_dflash",
    .addr       = 0x01000000,
    .len        = 0x00010000,
    .blk_size   = 0x00000800,
    .write_gran = 0x00000040,
    .ops        = {NULL, dflash_read, dflash_write, dflash_erase}
};
