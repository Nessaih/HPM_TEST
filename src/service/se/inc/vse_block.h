#ifndef _VSE_BLOCK_H_
#define _VSE_BLOCK_H_


#include "vse_type.h"
#include "vse_scp.h"


#define APDU_DATA_SIZE          (255)
#define RESP_DATA_SIZE          (256)

/* Master send information block:
 * CLA | INS | P1 | P2 | [Lc field] | [Data field] | [Le field]
 */
typedef enum {
    APDU_ONLY_HEAD = 0, /* CLA INS P1 P2 */
    APDU_WITH_RESP, /* CLA INS P1 P2 Le */
    APDU_WITH_DATA, /* CLA INS P1 P2 Lc Data */
    APDU_BOTH_ALL, /* CLA INS P1 P2 Lc Data Le */

    APDU_MODE_MAX
} apdu_mode_t;

typedef union {
    struct {
        uint8_t cla;
        uint8_t ins;
        uint8_t p1;
        uint8_t p2;
        uint8_t lc;
        uint8_t data[APDU_DATA_SIZE];
        uint8_t le;
        uint8_t res; /* lrc */
    } para;
    uint8_t buff[APDU_DATA_SIZE+7];
} vse_apdu_t;

typedef union {
    struct {
        uint8_t data[RESP_DATA_SIZE];
        uint8_t sw1;
        uint8_t sw2;
        uint8_t res; /* lrc */
    } para;
    uint8_t buff[RESP_DATA_SIZE+3];
} vse_resp_t;


void    block_init(vse_apdu_t *apdu, uint8_t cla, uint8_t ins);
int32_t block_reset(void);
int32_t block_ratr(void);
int32_t block_delay(void);
int32_t block_retry(void);
int32_t block_process(vse_apdu_t *apdu, apdu_mode_t mode, scp_level_t level,
                      scp_data_t *scp, vse_resp_t *resp, uint16_t *length);

#endif /* _VSE_BLOCK_H_ */
