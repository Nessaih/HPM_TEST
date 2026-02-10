#ifndef __SE_IF_H__
#define __SE_IF_H__

#include "tbox_common.h"

#define SE_PUBKEY_MAX_LEN	(64)
#define SE_CHIPID_MAX_LEN	(16)
#define SE_SINGN_MAX_LEN	(64)

INT32 se_get_uid(UINT8 *data, UINT16 len);

INT32 se_set_uid(UINT8 *data, UINT16 len);

INT32 se_get_version(UINT8 *ver, UINT16 *data_len);

INT32 se_get_pubkey(UINT8 *data, UINT16 *len);

INT32 se_get_real_chip_id(UINT8 *data, UINT16 *len);

INT32 se_get_signature(UINT8 *idata, INT32 ilen, UINT8 *odata, UINT16 *olen);

INT32 se_verify_signature(UINT8 *idata, INT32 ilen, UINT8 *sdata);

#endif //__SE_IF_H__

