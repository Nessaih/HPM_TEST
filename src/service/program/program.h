#ifndef __PROGRAM_APP_H__
#define __PROGRAM_APP_H__

#include <stdint.h>


#ifdef __BOOTLOADER__
    void program_start(void);
#else
    int32_t program_start(uint32_t addr, uint32_t size, uint32_t crc);
#endif

#endif //__PROGRAM_APP_H__