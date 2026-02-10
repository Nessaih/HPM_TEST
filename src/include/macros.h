#ifndef __MACROS_H__
#define __MACROS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIN_VALUE(a, b)                                           \
    ({                                                            \
        __typeof__(a) _a = (a);                                   \
        __typeof__(b) _b = (b);                                   \
        _a < _b ? _a : _b;                                        \
    })

#define MAX_VALUE(a, b)                                           \
    ({                                                            \
        __typeof__(a) _a = (a);                                   \
        __typeof__(b) _b = (b);                                   \
        _a > _b ? _a : _b;                                        \
    })

#define ARRAY_SIZE(arr)                  (sizeof(arr) / sizeof((arr)[0]))

#define IS_TIMEOUT(now, before, timeout) (((uint32_t)((now) - (before))) >= ((uint32_t)(timeout)))

#ifdef __cplusplus
}
#endif

#endif //__MACROS_H__