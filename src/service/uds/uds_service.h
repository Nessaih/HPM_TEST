#ifndef UDS_SERVICE_H
#define UDS_SERVICE_H

#include "tbox_common.h"
#include "can_if.h"
#include "uds_if.h"

#ifdef __cplusplus
extern "C" {
#endif

INT32 uds_register_can_id(UINT8 can_node, UINT32 can_id, uds_handle_t handle);

VOID uds_unregister_can_id(uds_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* UDS_SERVICE_H */
