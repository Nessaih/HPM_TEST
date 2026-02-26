
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "tbox_type.h"
#include "Crc_Hal.h"
#include "Osif_Time.h"
#include "driver.h"
#include "drv_flash_mcu.h"
#include "drv_pin.h"
#include "drv_uart.h"
#include "drv_wdg.h"
#include "flash_def.h"
#include "macros.h"
#include "program.h"
#include "version.h"

struct program_config_t
{
    char     iver[64];
    uint32_t iaddr;
    uint32_t isize;
    uint32_t icrc;
    uint32_t istate;
    uint32_t rev[8];
    uint32_t snum;
    uint32_t saddr;
    uint32_t ssize;
    uint32_t scrc;
};

typedef struct program_config_t program_config_t;

int32_t calc_crc32(uint32_t addr, uint32_t size, uint32_t *crc32_buf)
{
    uint8_t  data[256];
    uint32_t p, l;
    uint32_t crc;
    int32_t  status;

    l      = MIN_VALUE(sizeof(data), size);
    status = drv_flash_mcu_read(addr, data, l);
    if (status != 0)
    {
        return -1;
    }

    p   = l;
    crc = Crc_Hal_CalculateCRC32(data, l, CRC_INITIAL_VALUE32, TRUE, CRC_TABLE_256_BYTE_MODE);
    while (p < size)
    {
        drv_wdg_feed();

        l      = MIN_VALUE(sizeof(data), size - p);
        status = drv_flash_mcu_read(addr + p, data, l);
        if (status != 0)
        {
            return -2;
        }
        crc = Crc_Hal_CalculateCRC32(data, l, crc, FALSE, CRC_TABLE_256_BYTE_MODE);
        p += l;
    }
    *crc32_buf = crc;
    return 0;
}

int32_t program_write_cfg(program_config_t *cfg)
{
    int32_t  status;
    uint32_t addr;
    uint32_t crc;
    uint32_t num;

    if (cfg == NULL)
    {
        return -1;
    }

    strncpy(cfg->iver, (char *)(cfg->iaddr + 0x200U), sizeof(cfg->iver));

    if (FLASH_PCFG1_ADDR == cfg->saddr)
    {
        addr       = FLASH_PCFG1_ADDR;
        cfg->saddr = FLASH_PCFG2_ADDR;
    }
    else
    {
        addr       = FLASH_PCFG2_ADDR;
        cfg->saddr = FLASH_PCFG1_ADDR;
    }

    num = cfg->snum;
    cfg->snum += 1;
    cfg->ssize = FLASH_PCFG_LEN;

    crc       = Crc_Hal_CalculateCRC32((uint8_t *)cfg, FLASH_PCFG_LEN - 4, CRC_INITIAL_VALUE32, TRUE, CRC_TABLE_256_BYTE_MODE);
    cfg->scrc = crc;

    status = drv_flash_mcu_erase(cfg->saddr, FLASH_PCFG_LEN);
    if (0 != status)
    {
        return -3;
    }

    status = drv_flash_mcu_write(cfg->saddr, (uint8_t *)cfg, cfg->ssize);
    if (0 != status)
    {
        return -3;
    }

    if (num == 0xFFFFFFFFU)
    {
        status = drv_flash_mcu_erase(addr, FLASH_PCFG_LEN);
    }

    return status;
}

int32_t program_start(uint32_t addr, uint32_t size, uint32_t crc)
{
    program_config_t cfg = {0};
    uint32_t         crcc;
    int32_t          status;

    status = calc_crc32(addr, size, &crcc);
    if (status)
    {
        return -1;
    }

    if (crcc != crc)
    {
        return -2;
    }

    cfg.iaddr  = addr;
    cfg.isize  = size;
    cfg.icrc   = crc;
    cfg.istate = 0x5A5A5A5A;

    status = program_write_cfg(&cfg);
    if (status)
    {
        return -3;
    }

    OsIf_UDelay(1000);

    NVIC_SystemReset();

    return 0;
}
