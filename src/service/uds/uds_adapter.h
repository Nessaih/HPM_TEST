#ifndef UDS_ADAPTER_H
#define UDS_ADAPTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int isotp_user_send_can(uint32_t arbitration_id,
                        const uint8_t* data,
                        uint8_t size,
                        void* user_arg);

uint32_t isotp_user_get_us(void);

void isotp_user_debug(const char* message, ...);

uint32_t UDSMillis(void);

#ifdef __cplusplus
}
#endif

#endif /* UDS_ADAPTER_H */
