#ifndef UDS_CLIENT_API_H
#define UDS_CLIENT_API_H

#include <stdint.h>
#include <stdbool.h>
#include "uds_if.h"  /* 对外接口定义 */

#ifdef __cplusplus
extern "C" {
#endif

struct UDSClient;

struct UDSClient* uds_get_client(uds_handle_t handle);

void uds_client_poll_all(void);

#ifdef __cplusplus
}
#endif

#endif /* UDS_CLIENT_API_H */
