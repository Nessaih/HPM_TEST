#ifndef DRV_LOG_H
#define DRV_LOG_H

#include "tbox_type.h"
#include "tbox_log.h"

#ifdef __cplusplus
extern "C" {
#endif

INT32 drv_log_output(const CHAR *drv_name, TBOX_LOG_LEVEL level, const CHAR *fun, const INT32 line, const CHAR *fmt, ...);
VOID drv_log_set_level(TBOX_LOG_LEVEL level);

#define DRV_LOG_F(drv_name, ...) drv_log_output(#drv_name, LOG_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__)
#define DRV_LOG_E(drv_name, ...) drv_log_output(#drv_name, LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#define DRV_LOG_W(drv_name, ...) drv_log_output(#drv_name, LOG_LEVEL_WARN, __FUNCTION__, __LINE__, __VA_ARGS__)
#define DRV_LOG_I(drv_name, ...) drv_log_output(#drv_name, LOG_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#define DRV_LOG_D(drv_name, ...) drv_log_output(#drv_name, LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* DRV_LOG_H */