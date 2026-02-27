#ifndef __MEM_COMMON_H__
#define __MEM_COMMON_H__

#include "flash_def.h"

/*===================================  MCU Flash Memory Map ================================== */
/*────────────────────────────────────────DEFINE BLOCK3────────────────────────────────────────*/
#define FLASH_MCU_ADDR_FOTA_DATA	(FLASH_APP2_ADDR)
#define FLASH_MCU_SIZE_FOTA_DATA	(0x00080000U)
#define FLASH_MCU_SIZE_FOTA_PAGE	(FLASH_PPAGE_SIZE)
/*────────────────────────────────────────ENDDEF BLOCK3────────────────────────────────────────*/

/*────────────────────────────────────────DEFINE BLOCK7────────────────────────────────────────*/
#define FLASH_MCU_ADD_HPM_CFG       (FLASH_USER_ADDR)
#define FLASH_MCU_SIZE_HPM_CFG      (0x00008000) //32K
/*────────────────────────────────────────ENDDEF BLOCK7────────────────────────────────────────*/

/*===================================  MCU Flash Memory Map ===================================*/



/*===================================  NOR Flash Memory Map ===================================*/
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
/*===================================  NOR Flash Memory Map ===================================*/



#endif //__MEM_COMMON_H__
