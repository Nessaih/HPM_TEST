#ifndef _VSE_QUERY_H_
#define _VSE_QUERY_H_


#include "vse_type.h"


typedef enum {
    QUERY_VERSION = 0,
    QUERY_CHIP_SN,
    QUERY_LOAD_IF,

    QUERY_TYPE_MAX
} query_type_t;


int32_t query_get(query_type_t type, uint8_t *data, uint16_t length);


#endif /* _VSE_QUERY_H_ */
