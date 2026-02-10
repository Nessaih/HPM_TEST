
#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include "fal_def.h"

#define FAL_PART_HAS_TABLE_CFG 1

/* ===================== Flash device Configuration ========================= */
extern const fal_flash_dev_t onchip_pflash;
extern const fal_flash_dev_t onchip_dflash;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                                                        \
    {                                                                                              \
        &onchip_pflash,                                                                            \
        &onchip_dflash,                                                                            \
    }
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG

/* partition table */
#define FAL_PART_TABLE                                                                             \
    {                                                                                              \
        {FAL_PART_MAGIC_WORD, "bl",   "onchip_pflash", 0x00000000, 0x00018000, 0},                 \
        {FAL_PART_MAGIC_WORD, "app",  "onchip_pflash", 0x00018000, 0x00064000, 0},                 \
        {FAL_PART_MAGIC_WORD, "kv",   "onchip_dflash", 0x00000000, 0x00004000, 0},                 \
        {FAL_PART_MAGIC_WORD, "cfg",  "onchip_dflash", 0x00004000, 0x00004000, 0},                 \
    }

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
