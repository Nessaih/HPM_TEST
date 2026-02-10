#ifndef TBOX_LOG_H
#define TBOX_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tbox_common.h"
  
typedef enum tag_tbox_log_level 
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_NONE
}TBOX_LOG_LEVEL;

typedef struct tag_tbox_log_time
{
    UINT16 year;
    UINT8 month;
    UINT8 day;
    UINT8 hour;
    UINT8 minute;
    UINT8 second;
}TBOX_LOG_TIME;

VOID tbox_log_print(const CHAR *format, ...);
VOID tbox_log_output(TBOX_ID modlue_id, TBOX_LOG_LEVEL level, const CHAR *fun, const INT32 line, const CHAR *fmt, ...);
VOID tbox_log_dump(TBOX_ID modlue_id, const CHAR *tag, const UINT8 *data, const UINT16 len);

__weak VOID tbox_log_get_time(TBOX_LOG_TIME *time);
__weak VOID tbox_log_raw_output(const CHAR *str, UINT16 len);
__weak VOID tbox_log_flush(VOID);

#define LOG_PRINT(fmt, ...) tbox_log_print(fmt, ##__VA_ARGS__)
#define LOG_F(ID, ...) tbox_log_output(ID, LOG_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_E(ID, ...) tbox_log_output(ID, LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_W(ID, ...) tbox_log_output(ID, LOG_LEVEL_WARN, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_I(ID, ...) tbox_log_output(ID, LOG_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_D(ID, ...) tbox_log_output(ID, LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__)

#define MODULE_LOG_F(module, ...)\
    {\
        extern TBOX_ID tbox_##module##_id;\
        tbox_log_output(tbox_##module##_id, LOG_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }
#define MODULE_LOG_E(module, ...)\
    {\
        extern TBOX_ID tbox_##module##_id;\
        tbox_log_output(tbox_##module##_id, LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }
#define MODULE_LOG_W(module, ...)\
    {\
        extern TBOX_ID tbox_##module##_id;\
        tbox_log_output(tbox_##module##_id, LOG_LEVEL_WARN, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }
#define MODULE_LOG_I(module, ...)\
    {\
        extern TBOX_ID tbox_##module##_id;\
        tbox_log_output(tbox_##module##_id, LOG_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }
#define MODULE_LOG_D(module, ...)\
    {\
        extern TBOX_ID tbox_##module##_id;\
        tbox_log_output(tbox_##module##_id, LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }
#define MODULE_LOG_DUMP(module, tag, data, len)\
    {\
        extern TBOX_ID tbox_##module##_id;\
        tbox_log_dump(tbox_##module##_id, tag, data, len);\
    }
   
#ifdef __cplusplus
}
#endif

#endif /* TBOX_LOG_H */