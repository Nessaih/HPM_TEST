#ifndef _VSE_RANDOM_H_
#define _VSE_RANDOM_H_


#include "vse_type.h"


#define RANDOM_LEN_MIN      ( 4)
#define RANDOM_LEN_MAX      (16)


int32_t random_gen(uint8_t *data, uint16_t *length);


#endif /* _VSE_RANDOM_H_ */
