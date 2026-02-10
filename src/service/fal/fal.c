
#include "fal.h"
#include "tbox_core.h"

static uint8_t init_ok = 0;

int32_t fal_init(void)
{
    extern int32_t fal_flash_init(void);
    extern int32_t fal_partition_init(void);

    int32_t result;

    result = fal_flash_init();

    if (result < 0) {
        goto __exit;
    }

    result = fal_partition_init();

__exit:

    if ((result > 0) && (!init_ok)) {
        init_ok = 1;
        MODULE_LOG_I(TBOXSVR, "Flash Abstraction Layer initialize success.");
    } else if (result <= 0) {
        init_ok = 0;
        MODULE_LOG_E(TBOXSVR, "Flash Abstraction Layer initialize failed.");
    } else {
        /* No action needed for non-matching, non-empty entries */
    }

    return result;
}

int32_t fal_init_check(void)
{
    return (int32_t)init_ok;
}
