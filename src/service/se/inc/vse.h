#ifndef _VSE_H_
#define _VSE_H_


#include "vse_type.h"


#define SM2_PUB_KEY_SIZE            ( 64u)
#define SM2_USER_ID_SIZE            ( 16u)
#define SM2_PREP_ZA_SIZE            ( 32u)
#define SM2_SIGNATURE_SIZE          ( 64u)
#define SM2_CIPHER_C1_SIZE          ( 64u)
#define SM2_CIPHER_C2_SIZE          (256u)
#define SM2_CIPHER_C3_SIZE          ( 32u)

#define VSE_DEVICE_ID_SIZE          ( 16u)
#define VSE_VERSION_SIZE            (  4u)


typedef enum {
    VERSION_TYPE_SDK = 0,
    VERSION_TYPE_COS,

    VERSION_TYPE_MAX
} version_type_t;

typedef struct {
    uint8_t c1[SM2_CIPHER_C1_SIZE];
    uint8_t c3[SM2_CIPHER_C3_SIZE];
    uint8_t c2[SM2_CIPHER_C2_SIZE];
    uint16_t c2_len;
} sm2_cipher_t;


uint8_t vse_is_support(void);
int32_t vse_connect(void);
int32_t vse_disconnect(void);
int32_t vse_get_sm2key(uint8_t *pub_key, uint16_t *length);
int32_t vse_set_sm2key(uint8_t *pub_key, uint16_t length);
int32_t vse_do_sm2enc(uint8_t *plain, uint16_t plain_len, sm2_cipher_t *cipher);
int32_t vse_do_sm2dec(sm2_cipher_t *cipher, uint8_t *plain, uint16_t *plain_len);
int32_t vse_do_sm2prep(uint8_t *uid, uint16_t uid_len, uint8_t *pub,
                       uint16_t pub_len, uint8_t *za, uint16_t *za_len);
int32_t vse_do_sm2sign(uint8_t *msg, uint16_t msg_len, uint8_t *sign,
                       uint16_t *sign_len);
int32_t vse_do_sm2verify(uint8_t *msg, uint16_t msg_len, uint8_t *sign,
                         uint16_t sign_len);
int32_t vse_set_did(uint8_t *did, uint16_t length);
int32_t vse_get_did(uint8_t *did, uint16_t *length);
int32_t vse_get_ver(uint8_t type, uint8_t *ver, uint16_t *length);


#endif /* _VSE_H_ */
