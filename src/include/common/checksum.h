#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#ifdef __cpluscplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

uint16_t fletcher16_checksum(uint8_t *data, uint16_t len);
uint16_t fletcher16_verify(uint8_t *data, uint16_t len);
uint8_t  xor_checksum(uint8_t *data, uint16_t len);
uint8_t  sum_checksum(uint8_t *data, uint16_t len);
uint32_t crc32(const uint8_t *buf, uint32_t len, uint32_t init);
uint16_t crc16(uint8_t *data, uint16_t len);

#ifdef __cpluscplus
}
#endif

#endif //__CHECKSUM_H__