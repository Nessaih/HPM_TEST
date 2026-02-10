#ifndef _VSE_SM2ZA_H_
#define _VSE_SM2ZA_H_


#include "vse_type.h"


#define SM2ZA_UID_SIZE          (16)
#define SM2ZA_PUB_SIZE          (64)
#define SM2ZA_DATA_SIZE         (32)


int32_t sm2za(uint8_t *uid, uint16_t uid_len, uint8_t *pub, uint16_t pub_len,
              uint8_t *za, uint16_t *za_len);


#endif /* _VSE_SM2ZA_H_ */
