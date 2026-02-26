#ifndef __J1939_LIST_H__
#define __J1939_LIST_H__

#define list_define(name, type, size)                                                                                                                \
    struct name                                                                                                                                      \
    {                                                                                                                                                \
        type         buffer[size];                                                                                                                   \
        unsigned int in;                                                                                                                             \
        unsigned int out;                                                                                                                            \
        unsigned int mask;                                                                                                                           \
    }

#define list_init(list)                                                                                                                              \
    do                                                                                                                                               \
    {                                                                                                                                                \
        (list)->in   = 0;                                                                                                                            \
        (list)->out  = 0;                                                                                                                            \
        (list)->mask = sizeof((list)->buffer) / sizeof((list)->buffer[0]) - 1;                                                                       \
    } while (0)

#define list_is_empty(list) (((list)->in - (list)->out) & (list)->mask) == 0

#define list_is_full(list)  (((list)->in - (list)->out) & (list)->mask) == (list)->mask

#define list_len(list)      (((list)->in - (list)->out) & (list)->mask)

#define list_put(list, value)                                                                                                                        \
    do                                                                                                                                               \
    {                                                                                                                                                \
        (list)->buffer[(list)->in & (list)->mask] = value;                                                                                           \
        (list)->in++;                                                                                                                                \
    } while (0)

#define list_get(list, value)                                                                                                                        \
    do                                                                                                                                               \
    {                                                                                                                                                \
        *(value) = (list)->buffer[(list)->out & (list)->mask];                                                                                       \
        (list)->out++;                                                                                                                               \
    } while (0)

#endif //__J1939_LIST_H__