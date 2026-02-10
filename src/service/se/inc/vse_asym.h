#ifndef _VSE_ASYM_H_
#define _VSE_ASYM_H_


#include "vse_type.h"


typedef enum {
    ASYM_RSA_NOPADDING = 0,
    ASYM_RSA_SHA1,
    ASYM_RSA_SHA256,
    ASYM_RSA_SHA384,
    ASYM_RSA_SHA512,
    ASYM_SM2_SM3,
    ASYM_ECDSA,
    ASYM_SM9_SM3,
    ASYM_SM9_SM4,

    ASYM_TYPE_MAX
} asym_type_t;

typedef struct {
    asym_type_t type;
    uint8_t kid;
    uint8_t *msg;
    uint16_t msg_len;
} asym_para_t;


int32_t asym_enc(asym_para_t *para, uint8_t *cipher, uint16_t *cipher_len);
int32_t asym_dec(asym_para_t *para, uint8_t *plain, uint16_t *plain_len);
int32_t asym_mac(asym_para_t *para, uint8_t *mac, uint16_t *mac_len);
int32_t asym_ver(asym_para_t *para, uint8_t *mac, uint16_t mac_len);


#endif /* _VSE_ASYM_H_ */
