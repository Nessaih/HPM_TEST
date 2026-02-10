#ifndef __MEM_HPM_H__
#define __MEM_HPM_H__

#include "mem_def.h"

/*────────────────────────────────────────DEFINE BLOCK2────────────────────────────────────────*/
#define FLASH_NOR_ADDR_HPM_RUN_INFO (FLASH_NOR_ADDR_BLOCK2)
#define FLASH_NOR_SIZE_HPM_RUN_INFO (0x00001000)

#define FLASH_NOR_ADDR_HPM_SES_INFO (FLASH_NOR_ADDR_HPM_RUN_INFO + FLASH_NOR_SIZE_HPM_RUN_INFO)
#define FLASH_NOR_SIZE_HPM_SES_INFO (0x00001000)

/*────────────────────────────────────────ENDDEF BLOCK2────────────────────────────────────────*/

/*────────────────────────────────────────DEFINE BLOCK4────────────────────────────────────────*/
#define FLASH_NOR_ADDR_REISSUE_DATA (FLASH_NOR_ADDR_BLOCK4)
#define FLASH_NOR_SIZE_REISSUE_DATA (FLASH_NOR_SIZE_BLOCK4)

#define FLASH_NOR_ADDR_HPM_MGR      (FLASH_NOR_ADDR_REISSUE_DATA)
#define FLASH_NOR_SIZE_HPM_MGR      (0x00001000)

#define FLASH_NOR_ADDR_HPM_DATA     (FLASH_NOR_ADDR_HPM_MGR + 0x00010000)
#define FLASH_NOR_SIZE_HPM_DATA     (0x00180000) // 1.5M

/*────────────────────────────────────────ENDDEF BLOCK4────────────────────────────────────────*/

#endif //__MEM_HPM_H__