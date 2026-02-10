#ifndef __HPM_CFG_H__
#define __HPM_CFG_H__

INT32 hpm_cfg_init(UINT8 seq);

VOID hpm_cfg_wake(VOID);

VOID hpm_cfg_sleep(VOID);

VOID hpm_cfg_changed_handle(TBOX_MSG_DATA *data);

INT32 hpm_cfg_get_param(UINT8 *in_data, UINT16 in_len,UINT8 *out_data, UINT16 *out_len);

INT32 hpm_cfg_set_param(UINT8 *in_data, UINT16 in_len,UINT8 *out_data, UINT16 *out_len);

INT32 hpm_cfg_get_devid(UINT8 *data, INT32 len);

INT32 hpm_cfg_get_tracecode(UINT8 *data, INT32 len);

INT32 hpm_cfg_get_vin(UINT8 *data, INT32 len);

INT32 hpm_cfg_get_murl(UINT8 *data, INT32 len);

INT32 hpm_cfg_get_mip(UINT8 *data, INT32 len);
	
INT32 hpm_cfg_get_surl(UINT8 *data, INT32 len);

INT32 hpm_cfg_get_sip(UINT8 *data, INT32 len);

INT32 hpm_cfg_get_mport(UINT32 *data, INT32 len);

INT32 hpm_cfg_get_sport(UINT32 *data, INT32 len);

INT32 hpm_cfg_get_htbt(UINT32 *data, INT32 len);

#endif
