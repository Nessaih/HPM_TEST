#ifndef _VSE_SECURITY_H_
#define _VSE_SECURITY_H_


#include "vse_type.h"
#include "vse_scp.h"


#define STORE_SEID_LEN_MIN      (  1)
#define STORE_SEID_LEN_MAX      (251)


typedef enum {
    STORE_TYPE_SM2 = 0x04,
} store_type_t;

typedef enum {
    STORE_ELEM_SM2_W = 0x60,
} store_elem_t;

typedef struct {
    uint8_t *seid;
    uint8_t *skey;
    uint8_t  kid;
    store_type_t type;
    store_elem_t elem;
    uint16_t seid_len;
    uint16_t skey_len;
} store_cfg_t;

typedef enum {
    KEYPAIR_TYPE_SM2 = 0x04,
} keypair_type_t;

typedef enum {
    KEYPAIR_BITLEN_SM2 = 0x0100,
} keypair_bitlen_t;

typedef struct {
    keypair_type_t type;
    uint8_t kid;
    keypair_bitlen_t blen;
} keypair_cfg_t;


int32_t security_storage(store_cfg_t *store, scp_level_t level,
                         scp_data_t *scp);
int32_t security_get_seid(uint8_t *seid, uint16_t *length);
int32_t security_generate(keypair_cfg_t *keypair, scp_level_t level,
                          scp_data_t *scp, uint8_t *pub_key, uint16_t *pub_len);
int32_t security_get_pub(keypair_cfg_t *keypair, uint8_t *pub_key,
                         uint16_t *pub_len);

#endif /* _VSE_SECURITY_H_ */
