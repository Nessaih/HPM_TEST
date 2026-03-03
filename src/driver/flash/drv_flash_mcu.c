
#include <stdbool.h>
#include <string.h>
#include "Device_Register.h"
#include "Fls_Hal.h"
#include "drv_flash_mcu.h"
#include "macros.h"

#define FLASH_UNIT_BUF_SIZE (8U)

typedef Hal_StatusType (*flash_erase_func_t)(uint32 offset);
typedef Hal_StatusType (*flash_read_func_t)(uint32 offset, uint32 length, uint8 *data);
typedef Hal_StatusType (*flash_write_func_t)(uint32 offset, uint32 length, const uint8 *data);

typedef struct
{
    uint32_t           base_addr;
    uint32_t           end_addr;
    uint32_t           page_size;
    uint32_t           unit_size;
    flash_erase_func_t erase;
    flash_read_func_t  read;
    flash_write_func_t write;
} flash_ops_t;

typedef enum {
    ADDR_IN_PFLASH,
    ADDR_IN_DFLASH,
    ADDR_INVALID,
} addr_type_t;


static const Flash_Config flash_cfg = {
    .Write_Timeout = 0xFFFFFFFFU, 
    .Erase_Timeout = 0xFFFFFFFFU, 
    .LVD_Enable    = TRUE,        
};


static const flash_ops_t flash_ops[] = {
    [ADDR_IN_PFLASH] = {
        .base_addr = PFLASH_BASE,
        .end_addr  = PFLASH_BASE + PFLASH_BLOCK_SIZE,
        .page_size = PFLASH_PAGE_SIZE,
        .unit_size = PFLASH_WRITE_UNIT_SIZE,
        .erase     = PFlash_Hal_PageErase,
        .read      = PFlash_Hal_Read,
        .write     = PFlash_Hal_PageWrite
    },
    [ADDR_IN_DFLASH] = {
        .base_addr = DFLASH_BASE,
        .end_addr  = DFLASH_BASE + DFLASH_BLOCK_SIZE,
        .page_size = DFLASH_PAGE_SIZE,
        .unit_size = DFLASH_WRITE_UNIT_SIZE,
        .erase     = DFlash_Hal_PageErase,
        .read      = DFlash_Hal_Read,
        .write     = DFlash_Hal_PageWrite
    },
};
static bool flash_is_busy = true;

static inline uint32_t align_down(uint32_t val, uint32_t align)
{
    uint32_t mask;

    mask = ~(align - 1U);
    val &= mask;

    return val;
}

static inline addr_type_t get_addr_type(uint32_t addr)
{
    const flash_ops_t *ops;

    ops = &flash_ops[ADDR_IN_PFLASH];
    if (ops->base_addr <= addr && addr < ops->end_addr)
    {
        return ADDR_IN_PFLASH;
    }

    ops = &flash_ops[ADDR_IN_DFLASH];
    if (ops->base_addr <= addr && addr < ops->end_addr)
    {
        return ADDR_IN_DFLASH;
    }

    return ADDR_INVALID;
}

int32_t drv_flash_mcu_init(void)
{
    flash_is_busy = true;
    Flash_Hal_Init(&flash_cfg);
    flash_is_busy = false;
    return 0;
}

int32_t drv_flash_mcu_erase(uint32_t addr, uint32_t size)
{
    const flash_ops_t *ops;
    Hal_StatusType     status;
    addr_type_t        addr_type;
    uint32_t           align_addr;
    uint32_t           end_addr;
    uint32_t           offset;
    int32_t            result = 0;

    if (flash_is_busy)
    {
        result = -1;
        goto __exit;
    }
    else
    {
        flash_is_busy = true;
    }

    addr_type = get_addr_type(addr);
    if (ADDR_INVALID == addr_type)
    {
        result = -2;
        goto __exit;
    }
    else
    {
        ops = &flash_ops[addr_type];
    }

    end_addr = addr + size;
    if (end_addr > ops->end_addr)
    {
        result = -3;
        goto __exit;
    }

    align_addr = align_down(addr, ops->page_size);

    while (align_addr < end_addr)
    {
        offset = align_addr - ops->base_addr;
        status = ops->erase(offset);
        if (STATUS_SUCCESS != status)
        {
            result = -4;
            goto __exit;
        }
        align_addr += ops->page_size;
    }

__exit:
    flash_is_busy = false;

    return result;
}

int32_t drv_flash_mcu_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    const flash_ops_t *ops;
    Hal_StatusType     status;
    addr_type_t        addr_type;
    uint32_t           end_addr;
    uint32_t           offset;
    int32_t            result = 0;

    if (flash_is_busy)
    {
        result = -1;
        goto __exit;
    }
    else
    {
        flash_is_busy = true;
    }

    addr_type = get_addr_type(addr);
    if (ADDR_INVALID == addr_type)
    {
        result = -2;
        goto __exit;
    }
    else
    {
        ops = &flash_ops[addr_type];
    }

    end_addr = addr + len;
    if (end_addr > ops->end_addr)
    {
        result = -3;
        goto __exit;
    }

    offset = addr - ops->base_addr;
    status = ops->read(offset, len, data);

    if (STATUS_SUCCESS != status)
    {
        result = -4;
        goto __exit;
    }

__exit:
    flash_is_busy = false;

    return result;
}

int32_t drv_flash_mcu_write(uint32_t addr, const uint8_t *data, uint32_t len)
{
    const flash_ops_t *ops;
    Hal_StatusType     status;
    addr_type_t        addr_type;
    uint8_t            unit_buf[FLASH_UNIT_BUF_SIZE];
    uint32_t           align_addr;
    uint32_t           end_addr;
    uint32_t           offset;
    uint32_t           write_len;
    int32_t            result = 0;

    if (flash_is_busy)
    {
        result = -1;
        goto __exit;
    }
    else
    {
        flash_is_busy = true;
    }

    addr_type = get_addr_type(addr);
    if (ADDR_INVALID == addr_type)
    {
        result = -2;
        goto __exit;
    }
    else
    {
        ops = &flash_ops[addr_type];
    }

    end_addr = addr + len;
    if (end_addr > ops->end_addr)
    {
        result = -3;
        goto __exit;
    }

    align_addr = align_down(addr, ops->unit_size);

    if (addr != align_addr)
    {

        memset(unit_buf, 0xFF, sizeof(unit_buf));

        offset = align_addr - ops->base_addr;
        status = ops->read(offset, ops->unit_size, unit_buf);
        if (STATUS_SUCCESS != status)
        {
            result = -4;
            goto __exit;
        }

        offset    = addr - align_addr;
        write_len = ops->unit_size - offset;
        write_len = MIN_VALUE(write_len, len);
        memcpy(unit_buf + offset, data, write_len);

        offset = align_addr - ops->base_addr;
        status = ops->write(offset, ops->unit_size, unit_buf);
        if (STATUS_SUCCESS != status)
        {
            result = -5;
            goto __exit;
        }

        data = &data[write_len];
        len -= write_len;
        align_addr += ops->unit_size;
    }

    if (len >= ops->unit_size)
    {
        offset    = align_addr - ops->base_addr;
        write_len = align_down(len, ops->unit_size);
        status    = ops->write(offset, write_len, data);
        if (STATUS_SUCCESS != status)
        {
            result = -6;
            goto __exit;
        }

        data = &data[write_len];
        len -= write_len;
        align_addr += write_len;
    }

    if (len > 0U)
    {
        memset(unit_buf, 0xFF, sizeof(unit_buf));

        offset = align_addr - ops->base_addr;
        status = ops->read(offset, ops->unit_size, unit_buf);
        if (STATUS_SUCCESS != status)
        {
            result = -7;
            goto __exit;
        }

        write_len = len;
        memcpy(unit_buf, data, write_len);

        offset = align_addr - ops->base_addr;
        status = ops->write(offset, ops->unit_size, unit_buf);
        if (STATUS_SUCCESS != status)
        {
            result = -8;
            goto __exit;
        }

        align_addr += write_len;
    }

    // DEVICE_ASSERT(align_addr == end_addr);

__exit:
    flash_is_busy = false;

    return result;
}
