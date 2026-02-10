/*
 * eHSM_Debug_Ip.h
 *
 */

#ifndef EHSM_DEBUG_IP_H_
#define EHSM_DEBUG_IP_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#if 1
#include "eHSM_IntCfg_Ip.h"
#include "stdio.h"
#include "string.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define filename(p) strrchr(p,'\\')? strrchr(p,'\\')+1 : p

extern void Debug_Printf(const char * format, ...);
#if 0
static void Debug_Printf_Test(const char * format, ...)
{
#if ATC_DEBUG_OUT_INFO
    va_list list;
    uint8 len, i;
    /* store send message */
    uint8 s_logMessage[MAX_DEBUG_BUFF_SIZE] = {0};

    if (format != NULL)
    {
        va_start(list, format);
        len = vsnprintf((char *)&s_logMessage[0], MAX_DEBUG_BUFF_SIZE, format, list);

        for (i = 0; i < len; i++)
        {
            (void)Uart_Putc(s_logMessage[i]);
        }

        va_end(list);
    }
#endif
}
#endif

/* #define CONFIG_HOST_DEBUG_ENABLE */

#define CONFIG_HOST_V_LOG_ERR   0
#define CONFIG_HOST_V_LOG_WARN  1
#define CONFIG_HOST_V_LOG_DEBUG 2
#define CONFIG_HOST_V_LOG_INFO  3
#define CONFIG_HOST_V_LOG_LEVEL CONFIG_HOST_V_LOG_INFO

#ifdef CONFIG_HOST_DEBUG_ENABLE
#define rt_kprintf Debug_Printf

#define HOST_LOG(level, fmt, ...)                                      \
    if ((level) <= CONFIG_HOST_V_LOG_LEVEL)                            \
    {                                                                  \
        switch(level)                                                  \
        {                                                              \
            case CONFIG_HOST_V_LOG_ERR:   rt_kprintf("[ehsm_err]:<%s %s %d>", filename(__FILE__), __FUNCTION__, __LINE__); break; \
            case CONFIG_HOST_V_LOG_WARN:  rt_kprintf("[ehsm_warn]:<%s %s %d>", filename(__FILE__), __FUNCTION__, __LINE__); break; \
            case CONFIG_HOST_V_LOG_DEBUG: rt_kprintf("[ehsm_debug]:<%s %s %d>", filename(__FILE__), __FUNCTION__, __LINE__); break; \
            case CONFIG_HOST_V_LOG_INFO:  rt_kprintf("[ehsm_info]:<%s %s %d>", filename(__FILE__), __FUNCTION__, __LINE__); break; \
            default: break;                                            \
        }                                                              \
        rt_kprintf(fmt, ##__VA_ARGS__);                                \
    }

#define PURE_LOG(level, fmt, ...)                                      \
    if ((level) <= CONFIG_HOST_V_LOG_INFO)                             \
    {                                                                  \
        rt_kprintf(fmt, ##__VA_ARGS__);                                \
    }
#else
#define HOST_LOG(level, fmt, ...)
#define PURE_LOG(level, fmt, ...)
#endif

/* judge case print or not */
#if (CONFIG_HOST_V_LOG_LEVEL >= CONFIG_HOST_V_LOG_ERR)
#define HOST_LOG_ERROR(...)   HOST_LOG(CONFIG_HOST_V_LOG_ERR, ##__VA_ARGS__)
#else
#define HOST_LOG_ERROR(...)
#endif

#if (CONFIG_HOST_V_LOG_LEVEL >= CONFIG_HOST_V_LOG_WARN)
#define HOST_LOG_WARN(...)  HOST_LOG(CONFIG_HOST_V_LOG_WARN, ##__VA_ARGS__)
#else
#define HOST_LOG_WARN(...)
#endif

#if (CONFIG_HOST_V_LOG_LEVEL >= CONFIG_HOST_V_LOG_DEBUG)
#define HOST_LOG_DEBUG(...) HOST_LOG(CONFIG_HOST_V_LOG_DEBUG, ##__VA_ARGS__)
#else
#define HOST_LOG_DEBUG(...)
#endif

#if (CONFIG_HOST_V_LOG_LEVEL >= CONFIG_HOST_V_LOG_INFO)
#define HOST_LOG_INFO(...)  HOST_LOG(CONFIG_HOST_V_LOG_INFO, ##__VA_ARGS__)
#else
#define HOST_LOG_INFO(...)
#endif

#if (CONFIG_HOST_V_LOG_LEVEL >= CONFIG_HOST_V_LOG_INFO)
#define HOST_PURE_LOG(...)  PURE_LOG(CONFIG_HOST_V_LOG_INFO, ##__VA_ARGS__)
#else
#define HOST_PURE_LOG(...)
#endif

/* module print control */

#define CONFIG_HOST_COMMON_DEBUG_ENABLE
#ifdef CONFIG_HOST_COMMON_DEBUG_ENABLE
#define  COMMON_LOG_ERROR(fmt, args...)    HOST_LOG_ERROR(fmt, ##args)
#define  COMMON_LOG_WARN(fmt, args...)     HOST_LOG_WARN(fmt, ##args)
#define  COMMON_LOG_DEBUG(fmt, args...)    HOST_LOG_DEBUG(fmt, ##args)
#define  COMMON_LOG_INFO(fmt, args...)     HOST_LOG_INFO(fmt, ##args)
#define  COMMON_PURE_LOG_DEBUG(fmt, args...)    HOST_PURE_LOG(fmt, ##args)
#else
#define  COMMON_LOG_ERROR(fmt, args...)
#define  COMMON_LOG_WARN(fmt, args...)
#define  COMMON_LOG_DEBUG(fmt, args...)
#define  COMMON_LOG_INFO(fmt, args...)
#define  COMMON_PURE_LOG_DEBUG(fmt, args...)
#endif

#define CONFIG_HOST_EVITA_DEBUG_ENABLE
    #ifdef CONFIG_HOST_EVITA_DEBUG_ENABLE
    #define  EVITA_LOG_ERROR(fmt, args...)    HOST_LOG_ERROR(fmt, ##args)
    #define  EVITA_LOG_WARN(fmt, args...)     HOST_LOG_WARN(fmt, ##args)
    #define  EVITA_LOG_DEBUG(fmt, args...)    HOST_LOG_DEBUG(fmt, ##args)
    #define  EVITA_LOG_INFO(fmt, args...)     HOST_LOG_INFO(fmt, ##args)
    #define  EVITA_PURE_LOG_DEBUG(fmt, args...)    HOST_PURE_LOG(fmt, ##args)
    #else
    #define  EVITA_LOG_ERROR(fmt, args...)
    #define  EVITA_LOG_WARN(fmt, args...)
    #define  EVITA_LOG_DEBUG(fmt, args...)
    #define  EVITA_LOG_INFO(fmt, args...)
    #define  EVITA_PURE_LOG_DEBUG(fmt, args...)
    #endif

#define CONFIG_HOST_AUTOSAR_DEBUG_ENABLE
#ifdef CONFIG_HOST_AUTOSAR_DEBUG_ENABLE
#define  AUTOSAR_LOG_ERROR(fmt, args...)    HOST_LOG_ERROR(fmt, ##args)
#define  AUTOSAR_LOG_WARN(fmt, args...)     HOST_LOG_WARN(fmt, ##args)
#define  AUTOSAR_LOG_DEBUG(fmt, args...)    HOST_LOG_DEBUG(fmt, ##args)
#define  AUTOSAR_LOG_INFO(fmt, args...)     HOST_LOG_INFO(fmt, ##args)
#define  AUTOSAR_PURE_LOG_DEBUG(fmt, args...)    HOST_PURE_LOG(fmt, ##args)
#else
#define  AUTOSAR_LOG_ERROR(fmt, args...)
#define  AUTOSAR_LOG_WARN(fmt, args...)
#define  AUTOSAR_LOG_DEBUG(fmt, args...)
#define  AUTOSAR_LOG_INFO(fmt, args...)
#define  AUTOSAR_PURE_LOG_DEBUG(fmt, args...)
#endif

#define CONFIG_HOST_SHE_DEBUG_ENABLE
#ifdef CONFIG_HOST_SHE_DEBUG_ENABLE
#define  SHE_LOG_ERROR(fmt, args...)    HOST_LOG_ERROR(fmt, ##args)
#define  SHE_LOG_WARN(fmt, args...)     HOST_LOG_WARN(fmt, ##args)
#define  SHE_LOG_DEBUG(fmt, args...)    HOST_LOG_DEBUG(fmt, ##args)
#define  SHE_LOG_INFO(fmt, args...)     HOST_LOG_INFO(fmt, ##args)
#define  SHE_PURE_LOG_DEBUG(fmt, args...)    HOST_PURE_LOG(fmt, ##args)
#else
#define  SHE_LOG_ERROR(fmt, args...)
#define  SHE_LOG_WARN(fmt, args...)
#define  SHE_LOG_DEBUG(fmt, args...)
#define  SHE_LOG_INFO(fmt, args...)
#define  SHE_PURE_LOG_DEBUG(fmt, args...)
#endif

#define CONFIG_HOST_CUSTOM_DEBUG_ENABLE
#ifdef CONFIG_HOST_CUSTOM_DEBUG_ENABLE
#define  CUSTOM_LOG_ERROR(fmt, args...)    HOST_LOG_ERROR(fmt, ##args)
#define  CUSTOM_LOG_WARN(fmt, args...)     HOST_LOG_WARN(fmt, ##args)
#define  CUSTOM_LOG_DEBUG(fmt, args...)    HOST_LOG_DEBUG(fmt, ##args)
#define  CUSTOM_LOG_INFO(fmt, args...)     HOST_LOG_INFO(fmt, ##args)
#define  CUSTOM_PURE_LOG_DEBUG(fmt, args...)    HOST_PURE_LOG(fmt, ##args)
#else
#define  CUSTOM_LOG_ERROR(fmt, args...)
#define  CUSTOM_LOG_WARN(fmt, args...)
#define  CUSTOM_LOG_DEBUG(fmt, args...)
#define  CUSTOM_LOG_INFO(fmt, args...)
#define  CUSTOM_PURE_LOG_DEBUG(fmt, args...)
#endif

#define CONFIG_HOST_PERFORMANCE_DEBUG_ENABLE
#ifdef CONFIG_HOST_PERFORMANCE_DEBUG_ENABLE
#define  PERFORMANCE_LOG_ERROR(fmt, args...)    HOST_LOG_ERROR(fmt, ##args)
#define  PERFORMANCE_LOG_WARN(fmt, args...)     HOST_LOG_WARN(fmt, ##args)
#define  PERFORMANCE_LOG_DEBUG(fmt, args...)    HOST_LOG_DEBUG(fmt, ##args)
#define  PERFORMANCE_LOG_INFO(fmt, args...)     HOST_LOG_INFO(fmt, ##args)
#define  PERFORMANCE_PURE_LOG_DEBUG(fmt, args...)    HOST_PURE_LOG(fmt, ##args)
#else
#define  PERFORMANCE_LOG_ERROR(fmt, args...)
#define  PERFORMANCE_LOG_WARN(fmt, args...)
#define  PERFORMANCE_LOG_DEBUG(fmt, args...)
#define  PERFORMANCE_LOG_INFO(fmt, args...)
#define  PERFORMANCE_PURE_LOG_DEBUG(fmt, args...)
#endif

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#endif
#endif /* APPLICATION_COMMON_INCLUDE_EHSM_DEBUG_H_ */
