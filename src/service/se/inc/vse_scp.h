#ifndef _VSE_SCP_H_
#define _VSE_SCP_H_


#include "vse_type.h"


#define SCP_KEY_SIZE        (16)
#define SCP_MAC_SIZE        ( 8)
#define SCP_RDM_SIZE        ( 8)


typedef enum {
    SCP_LEVEL_PLAIN = 0,
    SCP_LEVEL_P_MAC, /* Plain+MAC */
    SCP_LEVEL_CRYPT,
    SCP_LEVEL_C_MAC, /* Crypt+MAC */

    SCP_LEVEL_MAX
} scp_level_t;

typedef enum {
    SCP_KEY_ENC = 0,
    SCP_KEY_MAC,
    SCP_KEY_DEK,

    SCP_KEY_MAX
} scp_key_t;

typedef struct {
    uint8_t key[SCP_KEY_MAX][SCP_KEY_SIZE];
    uint8_t mac[SCP_MAC_SIZE];
} scp_data_t;

typedef struct {
    uint8_t key_data[10];
    uint8_t kmc_ver;
    uint8_t scp_flg;
    uint8_t sn_cnt[2];
    uint8_t se_rdm[6];
    uint8_t crypt[8];
} scpi_data_t;


int32_t scp_init(uint8_t random[8], uint8_t *data, uint16_t *length);
int32_t scp_auth(uint8_t random[8], uint8_t sn_cnt[2], uint8_t se_rdm[6],
                 scp_level_t level, scp_data_t *data);


#endif /* _VSE_SCP_H_ */
