#include "log.h"
#include <stdarg.h>
#include <stdio.h>

#if defined(UDS_SYS) && (UDS_SYS == UDS_SYS_CUSTOM)
#include "../../service/uds/uds_config.h"
#include "tbox_log.h"
#include "FreeRTOS.h"
#include "task.h"

extern TBOX_ID tbox_UDS_id;
#endif

#if UDS_LOG_LEVEL > UDS_LOG_NONE
void UDS_LogWrite(UDS_LogLevel_t level, const char *tag, const char *format, ...) {
    va_list list;
    va_start(list, format);
    
#if defined(UDS_SYS) && (UDS_SYS == UDS_SYS_CUSTOM)
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, list);
    
    TBOX_LOG_LEVEL tbox_level;
    switch (level) {
        case UDS_LOG_ERROR:   tbox_level = LOG_LEVEL_ERROR; break;
        case UDS_LOG_WARN:    tbox_level = LOG_LEVEL_WARN;  break;
        case UDS_LOG_INFO:    tbox_level = LOG_LEVEL_INFO;  break;
        case UDS_LOG_DEBUG:   tbox_level = LOG_LEVEL_DEBUG; break;
        case UDS_LOG_VERBOSE: tbox_level = LOG_LEVEL_DEBUG; break;
        default:              tbox_level = LOG_LEVEL_DEBUG; break;
    }
    
    tbox_log_output(tbox_UDS_id, tbox_level, tag, 0, "%s", buffer);
#else
    (void)level;
    (void)tag;
    vprintf(format, list);
#endif
    
    va_end(list);
}

void UDS_LogSDUInternal(UDS_LogLevel_t level, const char *tag, const uint8_t *buffer,
                        size_t buff_len, UDSSDU_t *info) {
    (void)info;
    for (unsigned i = 0; i < buff_len; i++) {
        UDS_LogWrite(level, tag, "%02x ", buffer[i]);
    }
    UDS_LogWrite(level, tag, "\n");
}
#endif
