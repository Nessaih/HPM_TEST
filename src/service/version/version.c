#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "mcu_resource_map.h"
#include "version.h"

#ifdef __BOOTLOADER__
#define CURRENT_VERSION_TYPE VERSION_TYPE_BOOT
#define MOUDLE_TYPE          "BTL"
#define PROJECT_CODE         "HPM60XXXX001"
#else
#define CURRENT_VERSION_TYPE VERSION_TYPE_APP
#define MOUDLE_TYPE          "VPU"
#define PROJECT_CODE         "HPM30HAIW001"
#endif

// clang-format off
#define SOFTWARE_VER  "00"
#define DEBUG_VER     "11"
#define RELEASE_DATE  "20260310"
#define HARDWARE_VER  "93"
#define APP_VERSION   MOUDLE_TYPE "." PROJECT_CODE "." SOFTWARE_VER "." DEBUG_VER "." RELEASE_DATE "." HARDWARE_VER
// clang-format on

static const CHAR app_version[VERION_MAX_LEN] @".ver" = APP_VERSION "\0" __TIMESTAMP__;

const CHAR *version_get(UINT8 type)
{
    const char *ver = NULL;

    if (CURRENT_VERSION_TYPE == type) 
    {
        ver = app_version;
    } 
    else 
    {
#ifdef __BOOTLOADER__
        ver = (const char *)(FLASH_APP_ADDR + 0X200U);
#else
        ver = (const char *)(FLASH_BOOT_ADDR + 0X200U);
#endif
    }

    return ver;
}

VOID  version_print(UINT8 type)
{
    const CHAR *name[] = {"BOOT", "APP"};
    CHAR print_buff[64] = "\0";
    
    strncpy(print_buff, "\n-------------------------------------------------\n", 64U);
    tbox_log_raw_output(print_buff, strlen(print_buff));
    snprintf(print_buff, 64U, "%4s Version: %s", name[type], version_get(type));
    tbox_log_raw_output(print_buff, strlen(print_buff));
    strncpy(print_buff, "\n-------------------------------------------------\n", 64U);  
    tbox_log_raw_output(print_buff, strlen(print_buff));
}

VOID  version_show(VOID)
{
    version_print(CURRENT_VERSION_TYPE);
}
