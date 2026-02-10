#ifndef _HPM_DATA_H_
#define _HPM_DATA_H_

#include "tbox_dlist.h"

#define HPM_PACK_BUFF_LEN 512

typedef struct
{
    uint8  data[HPM_PACK_BUFF_LEN];
    uint16 len;
    uint8  type;
    unsigned short seq;

    DLIST_NODE *list;
    DLIST_NODE  link;
}HPM_PACKET;

typedef enum
{
   HPM_FREE_LIST = 0,
   HPM_REPORT_LIST,
   HPM_DELAY_LIST,
   HPM_TRANS_LIST,
}HPM_LIST_TYPE;


UINT8 hpm_data_init(VOID);

VOID hpm_data_format_memery(VOID);

VOID hpm_data_save_to_realtm_list(UINT8 cmdid, UINT8* data, uint16 len);

VOID hpm_data_flush_realtm_data(VOID);

VOID hpm_data_flush_trans_list(VOID);

UINT8 hpm_data_no_report_data(VOID);

UINT8 hpm_data_no_sending_data(VOID);

HPM_PACKET* hpm_data_get_report_pack(VOID);

VOID hpm_data_put_back_report_pack(HPM_PACKET* pack);

VOID hpm_data_put_trans_list(HPM_PACKET* pack);

VOID hpm_data_ack_pack(VOID);

HPM_PACKET* hpm_data_get_report_pack_noflash(VOID);

VOID hpm_data_put_back_list_with_samepos(HPM_PACKET* pack);

VOID hpm_data_put_to_list(HPM_LIST_TYPE list_type, HPM_PACKET* pack);

HPM_PACKET* hpm_data_get_from_list(HPM_LIST_TYPE list_type);

UINT8 hpm_data_flush_alldata(VOID);

#endif
