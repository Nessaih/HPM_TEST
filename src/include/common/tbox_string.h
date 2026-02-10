#ifndef TBOX_STRING_H
#define TBOX_STRING_H

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "tbox_type.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline VOID tbox_string_tolower(CHAR *str)
{
    UINT32 i;
    for (i = 0U; '\0' != str[i]; i++) 
    {
        str[i] = (CHAR)tolower(str[i]);
    }
}

static inline  VOID tbox_string_toupper(CHAR *str)
{
    UINT32 i;
    for (i = 0U; '\0' != str[i]; i++) 
    {
        str[i] = (CHAR)toupper(str[i]);
    }
}

static inline UINT32 tbox_string_to_bytes(const CHAR *str, UINT8 *buf, UINT32 len)
{
    UINT32 index;
    UINT32 has_tolen = 0u;
    UINT32 str_len = strlen(str);
    CHAR *endptr;
    UINT8 value;

    for(index = 0; index < len && has_tolen <= str_len; index++)
    {
        value = (UINT8)strtol(str+has_tolen, &endptr, 16U);
        if((str+has_tolen) == endptr)
        {
            break;
        }
        buf[index] = value;
        has_tolen += 3U;
    }
    
    return index;
}

static inline UINT8* tbox_string_get_substring(UINT8 *data, UINT16 size, CHAR* substr)
{
    UINT16 substr_len;
    UINT16 i = 0U, j = 0U;
    UINT8 *token = NULL;
    UINT8 *temp_ptr = NULL;

    if(NULL_PTR == data ||
       0U == size ||
       NULL_PTR == substr)
    {
        return NULL_PTR;
    }

    substr_len = strlen(substr);
    if(size < substr_len)
    {
        return NULL_PTR;
    }

    for ( j = 0U; j < substr_len; )
    {
        token = (UINT8*)(substr+j);
        for(; i < size; )
        {
            if(*token == *(data+i))
            {
                if(0U == j)
                {
                    temp_ptr = data+i;
                }
                j++;
                i++;
                break;
            }
            else
            {
                if(0U == j)
                {
                  i++;
                }
                j = 0U;
                break;
            }
        }
        if (i >= size && j < substr_len)
        {
          return NULL_PTR;
        }
    }
    if( j >= substr_len)
    {
        return temp_ptr;
    }

    return NULL_PTR;
}


static inline UINT8* tbox_string_get_subdata(UINT8 *data, UINT16 size, UINT8* substr, UINT16 sub_len)
{
    UINT16 i = 0U, j = 0U;
    UINT8 *token = NULL_PTR;
    UINT8 *temp_ptr = NULL_PTR;

    if(NULL_PTR == data ||
       0U == size ||
       NULL_PTR == substr)
    {
        return NULL_PTR;
    }

    if(size < sub_len)
    {
        return NULL_PTR;
    }

    for ( j = 0U; j < sub_len; )
    {
        token = (UINT8*)(substr+j);
        for(; i < size; )
        {
            if(*token == *(data+i))
            {
                if(0U == j)
                {
                    temp_ptr = data+i;
                }
                j++;
                i++;
                break;
            }
            else
            {
                if(0U == j)
                {
                  i++;
                }
                j = 0U;
                break;
            }
        }
        if (i >= size && j < sub_len)
        {
          return NULL_PTR;
        }
    }
    if( j >= sub_len)
    {
        return temp_ptr;
    }

    return NULL_PTR;
}

static inline UINT8 tbox_string_cmp_string_from_end(UINT8 *data, UINT16 size, CHAR *substr)
{
    UINT8 *s = data + size - 1;
    UINT8 *d = (UINT8*)substr + strlen(substr) - 1;

    for(; *s == *d && s >= data && d >= (UINT8*)substr; s--, d--);

    if(s < data || d < (UINT8*)substr)
    {
        return 0U;
    }

    return 1U;
}

static inline BOOL tbox_string_get_num(UINT8 *data, UINT8 end_char, UINT32 *value)
{
    UINT8 *temp = data;

    *value = 0U;

    if(*temp == end_char)
    {
        return FALSE;
    }
    while(*temp != end_char)
    {
        if(!isdigit(*temp))
        {
           return FALSE;
        }
        *value = (*value)*10U + (*temp - '0');
        temp++;
    }

    return TRUE;
}

static inline BOOL tbox_tbox_string_get_num_bylen(UINT8 *data, UINT16 len, UINT32 *value)
{
    UINT16 index;

    *value = 0U;
    if(0U == len)
    {
        return FALSE;
    }
    for(index = 0; index < len; index++)
    {
        if(!isdigit(data[index]))
        {
           return FALSE;
        }
        *value = (*value)*10U + (data[index] - '0');
    }

    return TRUE;
}

static inline BOOL tbox_string_extract_num(UINT8 *data, UINT16 len, UINT32 *value)
{
    UINT16 index, begin_index;

    for(index = 0; index < len; index++)
    {
        if(isdigit(data[index]))
        {
           begin_index = index;
           break;
        }
    }
    if(index >= len)
    {
        return FALSE;
    }

    for(index = begin_index; index < len; index++)
    {
        if(!isdigit(data[index]))
        {
           break;
        }
    }

    return tbox_tbox_string_get_num_bylen(data+begin_index, index-begin_index, value);
}

#ifdef __cplusplus
}
#endif

#endif /* TBOX_STRING_H */