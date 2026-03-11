#ifndef __MEM_COMMON_H__
#define __MEM_COMMON_H__

#include "flash_def.h"

/*===================================  MCU Flash Memory Map ================================== */
/*────────────────────────────────────────DEFINE BLOCK3────────────────────────────────────────*/
#define FLASH_MCU_ADDR_FOTA_DATA    (FLASH_APP2_ADDR)
#define FLASH_MCU_SIZE_FOTA_DATA    (0x00080000U)
#define FLASH_MCU_SIZE_FOTA_PAGE    (FLASH_PPAGE_SIZE)
/*────────────────────────────────────────ENDDEF BLOCK3────────────────────────────────────────*/

/*────────────────────────────────────────DEFINE BLOCK7────────────────────────────────────────*/
#define FLASH_MCU_ADD_HPM_CFG       (FLASH_USER_ADDR)
#define FLASH_MCU_SIZE_HPM_CFG      (0x00008000U) //32K
#define FLASH_MCU_ADDR_HPM_NODE     (FLASH_MCU_ADD_HPM_CFG + FLASH_MCU_SIZE_HPM_CFG)
#define FLASH_MCU_SIZE_HPM_NODE     (0x00002000U) // 8K
/*────────────────────────────────────────ENDDEF BLOCK7────────────────────────────────────────*/

/*===================================  MCU Flash Memory Map ===================================*/



/*===================================  Extern Flash Memory Map ===================================*/
/*────────────────────────────────────────DEFINE BLOCK2────────────────────────────────────────*/
#define FLASH_ADDR_HPM_MGR (FLASH_EXT_ADDR_BLOCK2)
#define FLASH_SIZE_HPM_MGR (0x00001000U)

/*────────────────────────────────────────ENDDEF BLOCK2────────────────────────────────────────*/

/*────────────────────────────────────────DEFINE BLOCK4────────────────────────────────────────*/
#define FLASH_ADDR_REISSUE_DATA (FLASH_EXT_ADDR_BLOCK4)
#define FLASH_SIZE_REISSUE_DATA (FLASH_EXT_SIZE_BLOCK4)

#define FLASH_ADDR_HPM_DATA     (FLASH_ADDR_REISSUE_DATA)
#define FLASH_SIZE_HPM_DATA     (0x01000000U) // 16M

/*────────────────────────────────────────ENDDEF BLOCK4────────────────────────────────────────*/
/*===================================  Extern Flash Memory Map ===================================*/

#endif //__MEM_COMMON_H__
