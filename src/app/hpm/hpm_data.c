#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_dlist.h"
#include "tbox_log.h"

#include "hpm_data.h"
#include "hpm_flash.h"
#include "hpm_pack.h"


#define HPM_PACK_NUM  6

static HPM_PACKET hpm_data_mem[HPM_PACK_NUM];

static DLIST_NODE   hpm_free_list;
static DLIST_NODE   hpm_realtm_list;
static DLIST_NODE   hpm_delay_list;
static DLIST_NODE   hpm_trans_list;

static DLIST_NODE * hpm_get_node_from_free_list(VOID)
{
    DLIST_NODE *node;
    if ((node = dlist_pop_first_node(&hpm_free_list)) == NULL)
    {
       if ((node = dlist_pop_first_node(&hpm_delay_list)) == NULL &&
          (node = dlist_pop_first_node(&hpm_realtm_list)) == NULL)
       {
          MODULE_LOG_E(HPM, "no buffer to use, reset");
          while(1);
       }
    }

    return node;
}

static UINT8 hpm_data_save_one_pack(VOID)
{
    HPM_PACKET *pack = hpm_data_get_report_pack_noflash();
    if(NULL == pack)
    {
       MODULE_LOG_E(HPM, "no data to save flash");
       return 1;
    }

	if(0 != hpm_flash_save_pack(pack->data, pack->len, pack->type))
	{
		hpm_data_put_back_list_with_samepos(pack);
		MODULE_LOG_E(HPM, "failed to save packet");
		return 1;
	}

    hpm_data_put_to_list(HPM_FREE_LIST, pack);

    return 0;
}

static UINT8 hpm_data_load_one_pack(VOID)
{
    uint16 len = 0;
    UINT8 cmd = 0;
    HPM_PACKET *pack = hpm_data_get_from_list(HPM_FREE_LIST);
    if(NULL == pack)
    {
        MODULE_LOG_E(HPM, "failed to load packet from list");
        return 1;
    }

    len = sizeof(pack->data);
    if(0 != hpm_flash_load_pack(pack->data, &len,&cmd))
    {
        //MODULE_LOG_E(HPM, "failed to load packet from flash");
        hpm_data_put_to_list(HPM_FREE_LIST, pack);
        return 1;
    }
    pack->len = len;
    pack->type = cmd;

	MODULE_LOG_I(HPM, "Loaded packet: length=%u, command=%u", len, cmd);

    hpm_data_put_to_list(HPM_DELAY_LIST, pack);

    return 0;
}

static VOID hpm_data_flush_one_realtm_data(VOID)
{
    DLIST_NODE *node;
    HPM_PACKET *pack;
    INT32 delay_cmdid;
    INT8  count = dlist_count(&hpm_realtm_list);

    if(0 == count )
    {
        return;
    }

    while(count > 0 && (node = dlist_pop_first_node(&hpm_realtm_list)) != NULL)
    {
        pack = TBOX_CONTAINER(node, HPM_PACKET, link);
        delay_cmdid = HPM_CMD_REISSUE_DATA;//hpm_cv_com_get_delay_cmdid(pack->type);
        if (delay_cmdid)
        {
            MODULE_LOG_E(HPM, "need to reissue, save it[reissue cmdid=0x%x, real_cmdid: 0x%x]", delay_cmdid, pack->type);
            pack->list = &hpm_delay_list;
            pack->type = delay_cmdid;
            dlist_add_tail(&pack->link, &hpm_delay_list);
            break;
        }
        else
        {
            MODULE_LOG_E(HPM, "no need to reissue, delete it[reissue cmdid=0x%x, real_cmdid: 0x%x]", delay_cmdid, pack->type);
            dlist_add_tail(node, &hpm_realtm_list);
        }
        count--;
    }
}

UINT8 hpm_data_init(VOID)
{
    UINT8 index = 0;

    dlist_init(&hpm_realtm_list);
    dlist_init(&hpm_delay_list);
    dlist_init(&hpm_trans_list);
    dlist_init(&hpm_free_list);

    for (index = 0; index < HPM_PACK_NUM; index++)
    {
       dlist_add_tail(&hpm_data_mem[index].link, &hpm_free_list);
    }

    return 0;
}

VOID hpm_data_format_memery(VOID)
{
    UINT8 index = 0;

    dlist_init(&hpm_realtm_list);
    dlist_init(&hpm_delay_list);
    dlist_init(&hpm_trans_list);
    dlist_init(&hpm_free_list);

    for (index = 0; index < HPM_PACK_NUM; index++)
    {
        dlist_add_tail(&hpm_data_mem[index].link, &hpm_free_list);
    }
}

VOID hpm_data_save_to_realtm_list(UINT8 cmdid, UINT8* data, uint16 len)
{
    HPM_PACKET* pack;
    DLIST_NODE* node;

    if (NULL == data)
    {
        return;
    }

    if (len > HPM_PACK_BUFF_LEN)
    {
        MODULE_LOG_E(HPM, "data is overfow,save data failed.");
        return;
    }

    if (dlist_empty(&hpm_free_list))
    {
        hpm_data_flush_one_realtm_data();
        if (0 != hpm_data_save_one_pack())
        {
            return;
        }
    }

    node = hpm_get_node_from_free_list();
    if (NULL == node)
    {
        MODULE_LOG_E(HPM, "failed to load node from list");
        return;
    }

    pack = TBOX_CONTAINER(node, HPM_PACKET, link);
    memcpy(pack->data, data, len);
    pack->len = len;
    pack->list = &hpm_realtm_list;
    pack->type = cmdid;

    dlist_add_tail(node, &hpm_realtm_list);
}

VOID hpm_data_flush_realtm_data(VOID)
{
    DLIST_NODE *node;
    HPM_PACKET *pack;
    INT32 delay_cmdid;
    INT8  count = dlist_count(&hpm_realtm_list);

    if(0 == count )
    {
        MODULE_LOG_I(HPM, "real list is  empty");
        return;
    }

    while(count > 0 && (node = dlist_pop_first_node(&hpm_realtm_list)) != NULL)
    {
        pack = TBOX_CONTAINER(node, HPM_PACKET, link);
        delay_cmdid = HPM_CMD_REISSUE_DATA;//hpm_cv_com_get_delay_cmdid(pack->type);
        if (delay_cmdid)
        {
            MODULE_LOG_E(HPM, "need to reissue, save it[reissue cmdid=0x%x, real_cmdid: 0x%x]", delay_cmdid, pack->type);
            pack->type = delay_cmdid;
            pack->list = &hpm_delay_list;
            dlist_add_tail(&pack->link, &hpm_delay_list);
        }
        else
        {
            MODULE_LOG_E(HPM, "no need to reissue, delete it[reissue cmdid=0x%x, real_cmdid: 0x%x]", delay_cmdid, pack->type);
            dlist_add_tail(node, &hpm_realtm_list);
        }
		count--;
    }
}

VOID hpm_data_flush_trans_list(VOID)
{
    DLIST_NODE* node;
    HPM_PACKET* pack;

    while ((node = dlist_pop_last_node(&hpm_trans_list)) != NULL)
    {
        pack = TBOX_CONTAINER(node, HPM_PACKET, link);

        dlist_add_tail(&pack->link, pack->list);
    }
}

UINT8 hpm_data_no_report_data(VOID)
{
    return ((dlist_empty(&hpm_realtm_list) == 1 && dlist_empty(&hpm_delay_list) == 1) ? TRUE : FALSE);
}

UINT8 hpm_data_no_sending_data(VOID)
{
    return ((dlist_empty(&hpm_trans_list) == 1) ? TRUE : FALSE);
}

HPM_PACKET *hpm_data_get_report_pack(VOID)
{
    DLIST_NODE *node;

    if ((node = dlist_pop_first_node(&hpm_realtm_list)) == NULL)
    {
        if(dlist_empty(&hpm_delay_list))
        {
            if(0 != hpm_data_load_one_pack())
            {
                return NULL;
            }
        }

        node = dlist_pop_first_node(&hpm_delay_list);
    }

    return ((node == NULL) ? NULL : TBOX_CONTAINER(node, HPM_PACKET, link));
}

VOID hpm_data_put_back_report_pack(HPM_PACKET* pack)
{
    dlist_add_head(&pack->link, pack->list);
}

VOID hpm_data_put_trans_list(HPM_PACKET* pack)
{
    dlist_add_tail(&pack->link, &hpm_trans_list);
}

VOID hpm_data_ack_pack(VOID)
{
    DLIST_NODE* node;

    if ((node = dlist_pop_first_node(&hpm_trans_list)) != NULL)
    {
        dlist_add_tail(node, &hpm_free_list);
    }
}

HPM_PACKET *hpm_data_get_report_pack_noflash(VOID)
{
    DLIST_NODE *node;

    if ((node = dlist_pop_first_node(&hpm_delay_list)) == NULL)
    {
         node = dlist_pop_first_node(&hpm_realtm_list);
    }

    return ((node == NULL) ? NULL : TBOX_CONTAINER(node, HPM_PACKET, link));
}

VOID hpm_data_put_back_list_with_samepos(HPM_PACKET *pack)
{
    dlist_add_tail(&pack->link, pack->list);
}

VOID hpm_data_put_to_list(HPM_LIST_TYPE list_type, HPM_PACKET *pack)
{
    DLIST_NODE *node = &pack->link;
    DLIST_NODE *list;

    switch(list_type)
    {
       case HPM_FREE_LIST:
         list = &hpm_free_list;
         break;

       case HPM_REPORT_LIST:
         list = &hpm_realtm_list;
         break;

       case HPM_DELAY_LIST:
         pack->list = &hpm_delay_list;
         list = &hpm_delay_list;
         break;

       case HPM_TRANS_LIST:
           list = &hpm_trans_list;
           break;

       default:
         list = NULL;
         break;
    }

    if(NULL == list)
    {
       return;
    }

    dlist_add_tail(node, list);
}

HPM_PACKET *hpm_data_get_from_list(HPM_LIST_TYPE list_type)
{
    DLIST_NODE *node;
    DLIST_NODE *list;

    switch(list_type)
    {
       case HPM_FREE_LIST:
         list = &hpm_free_list;
         break;

       case HPM_REPORT_LIST:
         list = &hpm_realtm_list;
         break;

       case HPM_DELAY_LIST:
         list = &hpm_delay_list;
         break;

       case HPM_TRANS_LIST:
           list = &hpm_trans_list;
           break;

       default:
         list = NULL;
         break;
    }

    if(NULL == list)
    {
       return NULL;
    }

    node = dlist_pop_first_node(list);
    if(NULL == node)
    {
       return NULL;
    }

    return TBOX_CONTAINER(node, HPM_PACKET, link);
}

UINT8 hpm_data_flush_alldata(VOID)
{
    if (FALSE == hpm_data_no_sending_data())
    {
        hpm_data_flush_trans_list();
    }

	hpm_data_flush_realtm_data();

    while(FALSE == hpm_data_no_report_data())
    {
        if(0 != hpm_data_save_one_pack())
        {
            break;
        }
    }

    return 0;
}
