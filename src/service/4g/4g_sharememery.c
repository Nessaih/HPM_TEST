#include "4g_depend_header.h"
#include "tbox_common.h"
#include "4g_content.h"
#include "4g_sharememery.h"

#define MEM_4G_TOTALCOUNT (SHAREMEM_4G_16_COUNT + \
                           SHAREMEM_4G_32_COUNT + \
                           SHAREMEM_4G_64_COUNT + \
                           SHAREMEM_4G_128_COUNT + \
                           SHAREMEM_4G_256_COUNT)

#define MEM_4G_MEM_LEN (SHAREMEM_4G_16_COUNT*16 + \
                        SHAREMEM_4G_32_COUNT*32 + \
                        SHAREMEM_4G_64_COUNT*64 + \
                        SHAREMEM_4G_128_COUNT*128 + \
                        SHAREMEM_4G_256_COUNT*256)

typedef struct
{
    sharemem_4g_ele ele;
    uint8 is_used;
}sharemem_4g_mgr_ele;

static uint8 *sharemem_4g = NULL_PTR;
static sharemem_4g_mgr_ele sharemem_4g_mgr[MEM_4G_TOTALCOUNT];

void sharemem_4g_init(void)
{
    if(NULL_PTR == sharemem_4g)
    {
        sharemem_4g = (uint8 *)mempool_alloc(MEM_4G_MEM_LEN);
    }
    
    sharemem_4g_reinit();
}

void sharemem_4g_reinit(void)
{
    uint8 index;
    uint8 count = 0;
    uint8 *temp_ptr;

    memset(sharemem_4g, 0U, MEM_4G_MEM_LEN);
    temp_ptr = sharemem_4g;
    for(index = 0; index < SHAREMEM_4G_16_COUNT; index++)
    {
        sharemem_4g_mgr[count].ele.mem_ptr = temp_ptr;
        sharemem_4g_mgr[count].ele.len = 0;
        sharemem_4g_mgr[count].is_used = 0;

        temp_ptr = temp_ptr + 16;
        count++;
    }
    for(index = 0; index < SHAREMEM_4G_32_COUNT; index++)
    {
        sharemem_4g_mgr[count].ele.mem_ptr = temp_ptr;
        sharemem_4g_mgr[count].ele.len = 0;
        sharemem_4g_mgr[count].is_used = 0;

        temp_ptr = temp_ptr + 32;
        count++;
    }
    for(index = 0; index < SHAREMEM_4G_64_COUNT; index++)
    {
        sharemem_4g_mgr[count].ele.mem_ptr = temp_ptr;
        sharemem_4g_mgr[count].ele.len = 0;
        sharemem_4g_mgr[count].is_used = 0;

        temp_ptr = temp_ptr + 64;
        count++;
    }
    for(index = 0; index < SHAREMEM_4G_128_COUNT; index++)
    {
        sharemem_4g_mgr[count].ele.mem_ptr = temp_ptr;
        sharemem_4g_mgr[count].ele.len = 0;
        sharemem_4g_mgr[count].is_used = 0;

        temp_ptr = temp_ptr + 128;
        count++;
    }
#if (SHAREMEM_4G_256_COUNT > 0)    
    for(index = 0; index < SHAREMEM_4G_256_COUNT; index++)
    {
        sharemem_4g_mgr[count].ele.mem_ptr = temp_ptr;
        sharemem_4g_mgr[count].ele.len = 0;
        sharemem_4g_mgr[count].is_used = 0;

        temp_ptr = temp_ptr + 256;
        count++;
    }
#endif
}

INT8 sharemem_4g_alloc(SHARMMEM_4G_TYPE type)
{
    uint8 index = 0, min, max;
    uint16 size = 0U;

    switch(type)
    {
        case SHARMMEM_4G_16BYTE:
            min = 0;
            max = SHAREMEM_4G_16_COUNT;
            size = 16U;
            break;

        case SHARMMEM_4G_32BYTE:
            min = SHAREMEM_4G_16_COUNT;
            max = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT;
            size = 32U;
            break;

        case SHARMMEM_4G_64BYTE:
            min = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT;
            max = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT;
            size = 64U;
            break;

        case SHARMMEM_4G_128BYTE:
            min = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT;
            max = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT;
            size = 128U;
            break;

        case SHARMMEM_4G_256BYTE:
            min = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT;
            max = SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT+SHAREMEM_4G_256_COUNT;
            size = 256U;
            break;

        default:
            return -1;
    }

    if (min >= max)
    {
        MODULE_LOG_E(TBOX4G, "the memory[%d] has no space", type);
        return -1;
    }

    for(index = min; index < max; index++)
    {
        if(0 == sharemem_4g_mgr[index].is_used)
        {
            sharemem_4g_mgr[index].ele.len = 0;
            sharemem_4g_mgr[index].is_used = 1;
            memset(sharemem_4g_mgr[index].ele.mem_ptr, 0U, size);
            break;
        }
    }
    if(index >= max)
    {
        MODULE_LOG_I(TBOX4G, "failed to alloc shame memory, type:%d index:%u", type, index);
        return -1;
    }

    MODULE_LOG_I(TBOX4G, "success to alloc shame memory, type:%d index:%u", type, index);

    return index;
}

sharemem_4g_ele* sharemem_4g_getmem(INT8 index)
{
    uint16 max_len = 0;

    if(index >= 0 && index < SHAREMEM_4G_16_COUNT)
    {
        max_len = 16;
    }
    else if(index >= SHAREMEM_4G_16_COUNT && index < (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT))
    {
        max_len = 32;
    }
    else if(index >= (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT)
        && index < (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT))
    {
        max_len = 64;
    }
    else if(index >= (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT)
        && index < (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT))
    {
        max_len = 128;
    }
    else if(index >= (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT)
        && index < (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT+SHAREMEM_4G_256_COUNT))
    {
        max_len = 256;
    }
    else
    {
        return NULL;
    }

    if(0 == sharemem_4g_mgr[index].is_used)
    {
        return NULL;
    }
    if(sharemem_4g_mgr[index].ele.len > max_len)
    {
        //MODULE_LOG_E(TBOX4G, "the length is too long");
        return NULL;
    }
    
    return &sharemem_4g_mgr[index].ele;
}

void sharemem_4g_free(INT8 index)
{
   // MODULE_LOG_I(TBOX4G, "free shame memory, index:%d", index);

    if(index < 0 ||
       index >= (SHAREMEM_4G_16_COUNT+SHAREMEM_4G_32_COUNT+SHAREMEM_4G_64_COUNT+SHAREMEM_4G_128_COUNT+SHAREMEM_4G_256_COUNT))
    {
        return;
    }
    if(1 == sharemem_4g_mgr[index].is_used)
    {
        sharemem_4g_mgr[index].ele.len = 0;
        sharemem_4g_mgr[index].is_used = 0;
    }
}

