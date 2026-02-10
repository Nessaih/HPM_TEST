#ifndef _VSE_FILE_H_
#define _VSE_FILE_H_


#include "vse_type.h"
#include "vse_scp.h"


#define FILE_SEID_LEN_MIN       (  1)
#define FILE_SEID_LEN_MAX       (251)


typedef enum {
    SKEY_SM2_PUB = 0,

    SKEY_TYPE_MAX
} skey_type_t;

typedef struct {
    skey_type_t type;
    uint8_t kid;
    uint8_t *data;
    uint16_t len;
} file_skey_t;

typedef enum {
    FILE_MODE_GEN = 0x00,
    FILE_MODE_GET = 0x01,
} file_mode_t;

typedef enum {
    FILE_ASYM_RSA_STD = 0x02,
    FILE_ASYM_RSA_CRT = 0x03,
    FILE_ASYM_SM2 = 0x04,
    FILE_ASYM_ECC = 0x0A,
} file_asym_t;

typedef enum {
    FILE_BITLEN_SM2 = 0x0100,
    FILE_BITLEN_ECC192 = 0x00C0,
    FILE_BITLEN_ECC224 = 0x00E0,
    FILE_BITLEN_ECC256 = 0x0100,
    FILE_BITLEN_ECC384 = 0x0180,
    FILE_BITLEN_RSA768 = 0x0300,
    FILE_BITLEN_RSA1024 = 0x0400,
    FILE_BITLEN_RSA2048 = 0x0800,
} file_bitlen_t;

typedef struct {
    file_asym_t type;
    uint8_t kid;
    file_mode_t mode;
    file_bitlen_t bitlen;
} file_gen_t;


int32_t file_store_seid(uint8_t *seid, uint16_t length, scp_level_t level,
                        scp_data_t *scp);
int32_t file_read_seid(uint8_t *seid, uint16_t *length);
int32_t file_store_skey(file_skey_t *skey, uint8_t *seid, uint16_t seid_len,
                        scp_level_t level, scp_data_t *scp);
int32_t file_gen_keypair(file_gen_t *gen, uint8_t *pub, uint16_t *length,
                         scp_level_t level, scp_data_t *scp);

#endif /* _VSE_FILE_H_ */
