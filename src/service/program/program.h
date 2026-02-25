#ifndef __PROGRAM_APP_H__
#define __PROGRAM_APP_H__

#include <stdint.h>

void    program_init(void);
int32_t program_start(uint32_t addr, uint32_t size, uint32_t crc);

#endif //__PROGRAM_APP_H__