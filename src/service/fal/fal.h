

#ifndef _FAL_H_
#define _FAL_H_

#include "fal_cfg.h"
#include "fal_def.h"

#ifdef __cplusplus
extern "C" {
#endif

const fal_flash_dev_t *fal_find_flash(const char *name);
const fal_partition_t *fal_find_partition(const char *name);
int32_t                fal_read(const fal_partition_t *part, uint32_t addr, uint8_t *buf, uint32_t size);
int32_t                fal_write(const fal_partition_t *part, uint32_t addr, const uint8_t *buf, uint32_t size);
int32_t                fal_erase(const fal_partition_t *part, uint32_t addr, uint32_t size);
int32_t                fal_erase_whole(const fal_partition_t *part);
int32_t                fal_init(void);
void                   fal_show_partition(void);

#ifdef __cplusplus
}
#endif

#endif