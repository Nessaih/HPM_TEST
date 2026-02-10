#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_check_power.h"
#include "tbox_pm_io.h"
#include "analog_if.h"

#define TBOX_CHECK_POWER_PERIOD_MS         500U       /*500ms*/
#define TBOX_CHECK_POWER_BATMIN_VOLTAGE    3500U      /*3.5V*/
#define TBOX_CHECK_POWER_IS_NORMAL         0U
#define TBOX_CHECK_POWER_IS_UNDERVOLTAGE   1U
#define TBOX_CHECK_POWER_UNDERVOLTAGE_TIME (30000U/TBOX_CHECK_POWER_PERIOD_MS)    /*30s*/
#define TBOX_CHECK_POWER_RECOVERY_TIME     (5000U/TBOX_CHECK_POWER_PERIOD_MS)     /*5s*/

typedef enum
{
    TBOX_CHECK_POWER_STATE_NOCHECK = 0x00,
    TBOX_CHECK_POWER_STATE_DOCHECK_INNORMAL,
    TBOX_CHECK_POWER_STATE_DOCHECK_INUNDERVOLTAGE,
} DEV_MN_CHECK_POWERSUPPLY_STATE;

static volatile UINT8 tbox_check_power_state;
static volatile UINT8 tbox_check_power_count;
static volatile UINT8 tbox_check_power_flag;

VOID tbox_check_power_init(VOID)
{
    tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_NOCHECK;
    tbox_check_power_flag = TBOX_CHECK_POWER_IS_NORMAL;
    tbox_check_power_count = 0U;
}

VOID tbox_check_power_start(VOID)
{
    if(TBOX_CHECK_POWER_IS_UNDERVOLTAGE == tbox_check_power_flag)
    {
        tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_DOCHECK_INUNDERVOLTAGE;
    }
    else
    {
        tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_DOCHECK_INNORMAL;
    }
    tbox_check_power_count = 0U;
}

VOID tbox_check_power_stop(VOID)
{
    /*DO NOTING*/
    tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_NOCHECK;
    tbox_check_power_count = 0U; 
}

VOID tbox_check_period(VOID)
{
    switch (tbox_check_power_state)
    {
        case TBOX_CHECK_POWER_STATE_NOCHECK:
            /*DO NOTING*/
            break;

        case TBOX_CHECK_POWER_STATE_DOCHECK_INNORMAL:
            {
                /*如果主电在，则不进行电源供电检测 */
                if (TRUE == tbox_pm_io_mainpower_is_active())
                {
                    tbox_check_power_flag = TBOX_CHECK_POWER_IS_NORMAL;
                    tbox_check_power_count = 0U;
                    break;
                }
                /*检查电池电压是否高于3.5V*/
                if(analog_pwr_vtg() > TBOX_CHECK_POWER_BATMIN_VOLTAGE)
                {
                    if(TBOX_CHECK_POWER_IS_UNDERVOLTAGE == tbox_check_power_flag)
                    {
                        if(++tbox_check_power_count >= TBOX_CHECK_POWER_RECOVERY_TIME)
                        {
                            tbox_check_power_flag = TBOX_CHECK_POWER_IS_NORMAL;
                            tbox_check_power_count = 0U;
                            MODULE_LOG_I(TBOXMODE, "backup power is recoveryed...");
                        }
                    }
                }
                else
                {
                    if(++tbox_check_power_count >= TBOX_CHECK_POWER_UNDERVOLTAGE_TIME)
                    {
                        tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_DOCHECK_INUNDERVOLTAGE;
                        tbox_check_power_flag = TBOX_CHECK_POWER_IS_UNDERVOLTAGE;
                        tbox_check_power_count = 0U;
                        MODULE_LOG_E(TBOXMODE, "backup power is undervoltage...");
                    }
                }              
            }
            break;

        case TBOX_CHECK_POWER_STATE_DOCHECK_INUNDERVOLTAGE:
            {
                if (TRUE == tbox_pm_io_mainpower_is_active())
                {
                    tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_DOCHECK_INNORMAL;    
                    tbox_check_power_flag = TBOX_CHECK_POWER_IS_NORMAL;
                    tbox_check_power_count = 0U;
                    MODULE_LOG_I(TBOXMODE, "main power is recoveryed...");
                    break;
                }
                if(TBOX_CHECK_POWER_IS_UNDERVOLTAGE != tbox_check_power_flag)
                {
                    tbox_check_power_flag = TBOX_CHECK_POWER_IS_UNDERVOLTAGE;
                    tbox_check_power_count = 0U;                    
                }
                if(analog_pwr_vtg() > TBOX_CHECK_POWER_BATMIN_VOLTAGE)
                {
                    if(++tbox_check_power_count >= TBOX_CHECK_POWER_RECOVERY_TIME)
                    {
                        tbox_check_power_state = (UINT8)TBOX_CHECK_POWER_STATE_DOCHECK_INNORMAL;
                        tbox_check_power_flag = TBOX_CHECK_POWER_IS_NORMAL;
                        tbox_check_power_count = 0U;
                        MODULE_LOG_I(TBOXMODE, "backup power is recoveryed...");
                    }
                }
            }
            break;

        default:
            break;
    }
}

BOOL tbox_check_power_isundervoltage(VOID)
{
    if(TRUE == tbox_pm_io_mainpower_is_active())
    {
        return FALSE;
    }

    if(TBOX_CHECK_POWER_IS_NORMAL == tbox_check_power_flag)
    {
        return FALSE;
    }

    return TRUE;
}
