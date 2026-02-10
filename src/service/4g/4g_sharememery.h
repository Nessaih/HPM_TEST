#ifndef TBOX_4G_SHAREMEMERY_H
#define TBOX_4G_SHAREMEMERY_H

#define SHARMEM_4G_ALLOC(INDEX, MEM_TYPE) sharemem_4g_getmem(INDEX = sharemem_4g_alloc(MEM_TYPE))

typedef struct
{
    uint8* mem_ptr;
    uint16 len;
}sharemem_4g_ele;

void sharemem_4g_init(void);

void sharemem_4g_reinit(void);

INT8 sharemem_4g_alloc(SHARMMEM_4G_TYPE type);

sharemem_4g_ele* sharemem_4g_getmem(INT8 index);

void sharemem_4g_free(INT8 index);

#endif /* TBOX_4G_SHAREMEMERY_H */
