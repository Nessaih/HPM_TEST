#ifndef _VSE_ALGO_H_
#define _VSE_ALGO_H_


#include "vse_type.h"


#define ALGO_BLOCK_SIZE             (8)


typedef enum {
    ALGO_ENC_ECB = 0,
    ALGO_ENC_CBC,

    ALGO_ENC_MAX
} algo_enc_t;

typedef enum {
    ALGO_SIZE_1KEY = 8,
    ALGO_SIZE_2KEY = 16,
    ALGO_SIZE_3KEY = 24,
} algo_size_t;

typedef enum {
    ALGO_MODE_ENC = 0,
    ALGO_MODE_DEC,

    ALGO_MODE_MAX
} algo_mode_t;

typedef struct {
    uint8_t *msg;
    uint8_t *key;
    uint8_t *vec;
    uint16_t msg_len;
    uint16_t key_len;
    uint16_t vec_len;
} algo_para_t;


int32_t algo_des(algo_para_t *para, algo_enc_t type, uint8_t *cipher,
                 uint16_t *cipher_len);
int32_t algo_3des(algo_para_t *para, algo_enc_t type, algo_mode_t mode,
                  uint8_t *output, uint16_t *out_len);
int32_t algo_3mac(algo_para_t *para, uint8_t *mac, uint16_t *mac_len);


#endif /* _VSE_ALGO_H_ */
