#ifndef TBOX_PHM_MODULE_H
#define TBOX_PHM_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

INT32 tbox_phm_module_init(VOID);
VOID tbox_phm_module_start(VOID);
VOID tbox_phm_module_stop(VOID);
VOID tbox_phm_module_period(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PHM_MODULE_H */