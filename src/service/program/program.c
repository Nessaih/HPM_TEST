
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
#include "tbox_pm_if.h"

#ifndef __BOOTLOADER__

#define drv_flash_read  drv_flash_mcu_read
#define drv_flash_write drv_flash_mcu_write
#define drv_flash_erase drv_flash_mcu_erase

#endif

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
    status = drv_flash_read(addr, data, l);
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
        status = drv_flash_read(addr + p, data, l);
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

int32_t check_struct(program_config_t *cfg)
{
    uint32_t crc;

    if (cfg == NULL)
    {
        return -1;
    }
    crc = Crc_Hal_CalculateCRC32((uint8_t *)cfg, FLASH_PCFG_LEN - 4, CRC_INITIAL_VALUE32, TRUE, CRC_TABLE_256_BYTE_MODE);

    if (crc != cfg->scrc)
    {
        return -2;
    }

    if (0 == cfg->isize || cfg->isize > FLASH_APP_LEN)
    {
        return -3;
    }

    return 0;
}

int32_t check_image(program_config_t *cfg)
{

    uint32_t crc;

    if (cfg == NULL)
    {
        return -1;
    }

    if (calc_crc32(cfg->iaddr, cfg->isize, &crc))
    {
        return -2;
    }

    if (crc != cfg->icrc)
    {
        return -3;
    }
    return 0;
}

int32_t check_version(char *ver)
{
    const char *cv = version_get(VERSION_TYPE_APP);
    const char *nv = ver;
    uint32_t    l  = strlen(cv);
    uint32_t    nl = strlen(nv);

    if (l != nl || l < 33 || nl < 33 || cv[33] != nv[33])
    {
        return -1;
    }

    if (0 == strcmp(cv, nv))
    {
        return -2;
    }

    return 0;
}

int32_t program_read_cfg(program_config_t *cfg)
{
    int32_t          status;
    program_config_t buf[2];
    uint32_t         addr[2]  = {FLASH_PCFG1_ADDR, FLASH_PCFG2_ADDR};
    uint8_t          valid[2] = {0, 0};
    uint32_t         pcfg;

    if (cfg == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < 2; i++)
    {
        status = drv_flash_read(addr[i], (uint8_t *)&buf[i], FLASH_PCFG_LEN);
        if (0 != status)
            continue;

        if (check_struct(&buf[i]) == 0)
        {
            valid[i] = 1;
        }
    }

    if (valid[0] && valid[1])
    {
        if (buf[0].snum > buf[1].snum)
            pcfg = 0;
        else
            pcfg = 1;
    }
    else if (valid[0])
    {
        pcfg = 0;
    }
    else if (valid[1])
    {
        pcfg = 1;
    }
    else
    {
        return -1;
    }

    memcpy(cfg, &buf[pcfg], sizeof(program_config_t));

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

    strncpy(cfg->iver, (char *)(cfg->iaddr + 0x200U), sizeof(cfg->iver) - 1);
    cfg->iver[sizeof(cfg->iver) - 1] = '\0';

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

    status = drv_flash_erase(cfg->saddr, FLASH_PCFG_LEN);
    if (0 != status)
    {
        return -3;
    }

    status = drv_flash_write(cfg->saddr, (uint8_t *)cfg, cfg->ssize);
    if (0 != status)
    {
        return -3;
    }

    if (num == 0xFFFFFFFFU)
    {
        status = drv_flash_erase(addr, FLASH_PCFG_LEN);
    }

    return status;
}

#ifdef __BOOTLOADER__

int32_t program_check_cfg(program_config_t *cfg)
{
    if (cfg == NULL)
    {
        return -1;
    }

    if (cfg->istate != 0x5A5A5A5A)
    {
        return -2;
    }

    if (check_struct(cfg))
    {
        return -3;
    }

    if (check_image(cfg))
    {
        return -4;
    }

    if (check_version(cfg->iver))
    {
        return -5;
    }

    return 0;
}

int32_t program_process(program_config_t *cfg)
{
    int32_t  status;
    uint8_t  data[256];
    uint32_t p = 0, l;
    uint32_t crc;

    status = drv_flash_erase(FLASH_APP1_ADDR, FLASH_APP_LEN);
    if (status != 0)
    {
        log_print("Erase app failed\n");
        return -1;
    }

    while (p < cfg->isize)
    {
        drv_wdg_feed();

        l      = MIN_VALUE(sizeof(data), cfg->isize - p);
        status = drv_flash_read(cfg->iaddr + p, data, l);
        if (status != 0)
        {
            return -2;
        }
        status = drv_flash_write(FLASH_APP1_ADDR + p, data, l);
        if (status != 0)
        {
            return -3;
        }
        p += l;
    }

    status = calc_crc32(FLASH_APP1_ADDR, cfg->isize, &crc);
    if (status != 0)
    {
        return -4;
    }

    if (crc != cfg->icrc)
    {
        return -5;
    }

    return 0;
}

int32_t program_exit(program_config_t *cfg)
{

    int32_t status;

    cfg->istate = 0x00;

    status = program_write_cfg(cfg);
    if (status)
    {
        return -1;
    }

    OsIf_UDelay(1000);
    NVIC_SystemReset();
    return 0;
}

void program_start(void)
{
    program_config_t cfg;

    if (program_read_cfg(&cfg))
    {
        goto __app_run;
    }

    drv_wdg_feed();

    if (program_check_cfg(&cfg))
    {
        goto __app_run;
    }

    drv_wdg_feed();

    if (!program_process(&cfg))
    {
        program_exit(&cfg);
        goto __app_run;
    }
    else
    {
        log_print("Boot Reset ...\n");
        log_flush();
        NVIC_SystemReset();
        while (1)
            ;
    }

__app_run:
    log_print("Start App ...\n");
    log_flush();
    driver_deinit();
    __set_MSP(*(uint32_t *)FLASH_APP1_ADDR);
    ((void (*)(void)) * (uint32_t *)(FLASH_APP1_ADDR + 4))();
}

void program_indicate(void)
{
    static uint8_t mode  = 0;
    static uint8_t count = 0;

    if (count < 2)
    {
        count++;
        return;
    }
    else
    {
        count = 0;
    }

    switch (mode++)
    {
    case 0:
        drv_pin_set_level(PIN_LED_TBOX_0, 1);
        drv_pin_set_level(PIN_LED_TBOX_1, 0);
        drv_pin_set_level(PIN_LED_TBOX_2, 0);
        drv_pin_set_level(PIN_LED_TBOX_3, 0);
        break;

    case 1:
        drv_pin_set_level(PIN_LED_TBOX_0, 1);
        drv_pin_set_level(PIN_LED_TBOX_1, 1);
        drv_pin_set_level(PIN_LED_TBOX_2, 0);
        drv_pin_set_level(PIN_LED_TBOX_3, 0);
        break;

    case 2:
        drv_pin_set_level(PIN_LED_TBOX_0, 1);
        drv_pin_set_level(PIN_LED_TBOX_1, 1);
        drv_pin_set_level(PIN_LED_TBOX_2, 1);
        drv_pin_set_level(PIN_LED_TBOX_3, 0);
        break;

    case 3:
        drv_pin_set_level(PIN_LED_TBOX_0, 1);
        drv_pin_set_level(PIN_LED_TBOX_1, 1);
        drv_pin_set_level(PIN_LED_TBOX_2, 1);
        drv_pin_set_level(PIN_LED_TBOX_3, 1);
        break;

    default:
        drv_pin_set_level(PIN_LED_TBOX_0, 0);
        drv_pin_set_level(PIN_LED_TBOX_1, 0);
        drv_pin_set_level(PIN_LED_TBOX_2, 0);
        drv_pin_set_level(PIN_LED_TBOX_3, 0);
        mode = 0;
        break;
    }
}

#else

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

    status = program_read_cfg(&cfg);
    if (status)
    {
        return -3;
    }

    cfg.iaddr  = addr;
    cfg.isize  = size;
    cfg.icrc   = crc;
    cfg.istate = 0x5A5A5A5A;

    status = program_write_cfg(&cfg);
    if (status)
    {
        return -4;
    }

    tbox_pm_reboot(TBOX_PM_REBOOT_MCU);

    return 0;
}

#endif