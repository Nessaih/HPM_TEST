#ifndef TBOX_MODULE_INNER_H
#define TBOX_MODULE_INNER_H

#include "tbox_module.h"
#include "tbox_message.h"

#ifdef __cplusplus
extern "C" {
#endif

INT32 tbox_module_init(VOID);
VOID tbox_module_exit(VOID);
VOID tbox_module_init_all_regmodule(VOID);
INT32 tbox_module_send_message(TBOX_ID module_id,                         
                               CHAR *msg_name,
                               TBOX_MSG_HADNLER msg_handle,  
                               UINT16 msg_len,
                               UINT8 *msg_data);

#if __cplusplus
}
#endif

#endif /* TBOX_MODULE_INNER_H */