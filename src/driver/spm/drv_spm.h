#ifndef __DRV_SPM_H__
#define __DRV_SPM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    extern int32_t  drv_spm_init(void);
    extern int32_t  drv_spm_sleep(void);
    extern void     drv_spm_clr_status(void);
    extern uint32_t drv_spm_get_status(void);

#ifdef __cplusplus
}
#endif

#endif //__DRV_SPM_H__