#include "tbox_common.h"
#include "tbox_core.h"
#include "drv_flash_nor.h"
#include "drv_flash_sd.h"
#include "tbox_cfg_if.h"
#include "hpm_flash_adapter.h"

#define HPM_FLASH_TYPE_NAME "HPM_FLASH_ADAPTER_TYPE"
#define HPM_FLASH_TYPE_MAGIC (0x504D464C)

typedef enum
{
    HPM_FLASH_ADAPTER_TYPE_NONE = 0,
    HPM_FLASH_ADAPTER_TYPE_SD,
    HPM_FLASH_ADAPTER_TYPE_NOR,
    HPM_FLASH_ADAPTER_TYPE_MAX,
} hpm_flash_adapter_type_e;

typedef struct
{
    INT32 (*read)(UINT32 addr, UINT8 *buf, UINT32 len);
    INT32 (*write)(UINT32 addr, UINT8 *buf, UINT32 len);
    INT32 (*erase)(UINT32 addr, UINT16 len);
} hpm_flash_ops_t;

typedef struct
{
    UINT32 magic;
    hpm_flash_adapter_type_e type;
} hpm_flash_adapter_t;

static INT32 hpm_flash_sd_erase(UINT32 addr, UINT16 len)
{
    UNUSED(addr);
    UNUSED(len);
    return 0;
}

static hpm_flash_adapter_t hpm_flash_adapter;

static hpm_flash_ops_t hpm_flash_ops[HPM_FLASH_ADAPTER_TYPE_MAX] = {
    {drv_flash_sd_read, drv_flash_sd_write, hpm_flash_sd_erase},    // HPM_FLASH_ADAPTER_TYPE_NONE
    {drv_flash_sd_read, drv_flash_sd_write, hpm_flash_sd_erase},    // HPM_FLASH_ADAPTER_TYPE_SD
    {drv_flash_nor_read, drv_flash_nor_write, drv_flash_nor_erase}, // HPM_FLASH_ADAPTER_TYPE_NOR
};

static hpm_flash_adapter_type_e hpm_flash_get_type(VOID)
{
    hpm_flash_adapter_type_e type = HPM_FLASH_ADAPTER_TYPE_NONE;
    UINT32 id = 0;
    INT32 ret = drv_flash_sd_get_id(&id);
    if ((ret == 0) && (id != 0))
    {
        MODULE_LOG_I(HPM, "sd flash id: 0x%08X", id);
        type = HPM_FLASH_ADAPTER_TYPE_SD;
        return type;
    }

    UINT8 i = 0;
    for (i = 0; i < 3; i++)
    {
        ret = drv_flash_nor_get_id(&id);
        if ((ret == 0) && (id != 0))
        {
            MODULE_LOG_I(HPM, "nor flash id: 0x%08X", id);
            type = HPM_FLASH_ADAPTER_TYPE_NOR;
            return type;
        }
        vTaskDelay(pdMS_TO_TICKS(100U));
    }

    MODULE_LOG_E(HPM, "no flash type found");
    return type;
}

static VOID hpm_flash_info_save(VOID)
{
    hpm_flash_adapter.magic = HPM_FLASH_TYPE_MAGIC;
    tbox_cfg_setkv(HPM_FLASH_TYPE_NAME, (UINT8 *)&hpm_flash_adapter, sizeof(hpm_flash_adapter_t));
}

static VOID hpm_flash_info_load(VOID)
{
    memset(&hpm_flash_adapter, 0, sizeof(hpm_flash_adapter_t));
    tbox_cfg_getkv(HPM_FLASH_TYPE_NAME, (UINT8 *)&hpm_flash_adapter, sizeof(hpm_flash_adapter_t));
    if ((hpm_flash_adapter.magic != HPM_FLASH_TYPE_MAGIC) ||
        (hpm_flash_adapter.type == HPM_FLASH_ADAPTER_TYPE_NONE) ||
        (hpm_flash_adapter.type >= HPM_FLASH_ADAPTER_TYPE_MAX))
    {
        MODULE_LOG_E(HPM, "load flash adapter type failed, reset to default");
        hpm_flash_adapter.type = hpm_flash_get_type();
        hpm_flash_info_save();
    }
}

INT32 hpm_flash_adapter_init(VOID)
{
    hpm_flash_info_load();
    return 0;
}

INT32 hpm_flash_adapter_read(UINT32 addr, UINT8 *buf, UINT32 len)
{
    hpm_flash_adapter_type_e type = hpm_flash_adapter.type;

    if (type >= HPM_FLASH_ADAPTER_TYPE_MAX || hpm_flash_ops[type].read == NULL)
    {
        MODULE_LOG_I(HPM, "flash adapter type not support");
        return -1;
    }

    return hpm_flash_ops[type].read(addr, buf, len);
}

INT32 hpm_flash_adapter_write(UINT32 addr, UINT8 *buf, UINT32 len)
{
    hpm_flash_adapter_type_e type = hpm_flash_adapter.type;

    if (type >= HPM_FLASH_ADAPTER_TYPE_MAX || hpm_flash_ops[type].write == NULL)
    {
        MODULE_LOG_I(HPM, "flash adapter type not support");
        return -1;
    }

    return hpm_flash_ops[type].write(addr, buf, len);
}

INT32 hpm_flash_adapter_erase(UINT32 addr, UINT16 len)
{
    hpm_flash_adapter_type_e type = hpm_flash_adapter.type;

    if (type >= HPM_FLASH_ADAPTER_TYPE_MAX || hpm_flash_ops[type].erase == NULL)
    {
        MODULE_LOG_I(HPM, "flash adapter type not support");
        return -1;
    }
    return hpm_flash_ops[type].erase(addr, len);
}

VOID hpm_flash_adapter_show(VOID)
{
    const char *type_name[HPM_FLASH_ADAPTER_TYPE_MAX] = {
        "unknown",
        "sd",
        "nor",
    };
    tbox_log_print("hpm flash type: %s\r\n", type_name[hpm_flash_adapter.type]);
}
