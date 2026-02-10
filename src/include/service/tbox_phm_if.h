#ifndef TBOX_PHM_IF_H
#define TBOX_PHM_IF_H

#ifdef __cplusplus
extern "C" {
#endif

INT32 tbox_phm_add_apn_monitor(UINT8 apn_index);
VOID tbox_phm_remove_apn_monitor(UINT8 apn_index);
INT32 tbox_phm_add_connect_monitor(UINT8 conn_no);
VOID tbox_phm_remove_connect_monitor(UINT8 conn_no);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PHM_IF_H */