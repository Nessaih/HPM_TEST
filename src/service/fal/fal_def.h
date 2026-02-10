
#ifndef __FAL_DEF_H__
#define __FAL_DEF_H__

#include <stdint.h>

/* FAL flash and partition device name max length */
#ifndef FAL_DEV_NAME_MAX
#define FAL_DEV_NAME_MAX 24
#endif

#ifndef FAL_DEV_BLK_MAX
#define FAL_DEV_BLK_MAX 6
#endif

struct flash_blk {
    uint32_t size;
    uint32_t count;
};

struct flash_ops {
    int (*init)(void);
    int (*read)(int32_t offset, uint8_t *buf, uint32_t size);
    int (*write)(int32_t offset, const uint8_t *buf, uint32_t size);
    int (*erase)(int32_t offset, uint32_t size);
};

struct fal_flash_dev {
    char             name[FAL_DEV_NAME_MAX];
    uint32_t         addr;       /* flash device start address  */
    uint32_t         len;        /* flash device start length  */
    uint32_t         blk_size;   /* the block size in the flash for erase minimum granularity */
    uint32_t         write_gran; /* write minimum granularity, unit: bit. */
    struct flash_ops ops;
    struct flash_blk blocks[FAL_DEV_BLK_MAX];
};

typedef struct fal_flash_dev fal_flash_dev_t;

struct fal_partition {
    uint32_t magic_word;
    char     name[FAL_DEV_NAME_MAX];       /* partition name */
    char     flash_name[FAL_DEV_NAME_MAX]; /* flash device name for partition */
    int32_t  offset;                       /* partition offset address on flash device */
    uint32_t len;
    uint32_t reserved;
};

typedef struct fal_partition fal_partition_t;

#endif /* __FAL_DEF_H__ */
