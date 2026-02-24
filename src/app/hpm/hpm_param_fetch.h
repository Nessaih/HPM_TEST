#ifndef __HPM_PARAM_FECTH_H__
#define __HPM_PARAM_FECTH_H__

#include <stdint.h>

#define HPM_PARAM_MAX_CAN_TYPE1_SINGLE (32U)
#define HPM_PARAM_MAX_CAN_TYPE2_COMPLEX (4U)
#define HPM_PARAM_MAX_CAN_TYPE3_PGN_BC (4U)
#define HPM_PARAM_MAX_CAN_TYPE4_PGN_REQ (4U)
#define HPM_PARAM_MAX_CAN_TYPE5_UDS (4U)

int hpm_param_init(UINT8 seq);

int hpm_param_download_req(UINT8* in_data, UINT16 in_len, UINT8* res, UINT16* res_len);

void hpm_param_timeout(void);

int8_t hpm_param_canid_type1_index_find(uint32_t canid);

int8_t hpm_param_canid_type2_index_find(uint32_t canid, uint8_t *frame_num, uint8_t *indexpos);

int8_t hpm_param_pgn_type3_index_find(uint32_t pgn);

int8_t hpm_param_pgn_type4_index_find(uint32_t pgn);

int8_t hpm_param_j1939_req(void);

int8_t hpm_param_uds_req(void);

int8_t hpm_param_get_uds_did_index(void);

uint32_t hpm_param_get_uds_did(void);

void hpm_param_show_cfg(void);

uint32_t hpm_param_get_id(void);

int8_t hpm_param_uds_did_index_add(void);

#endif //__hpm_CFG_H__
