#include "tbox_config.h"
#include "tbox_common.h"
#include "tbox_dlist.h"

VOID dlist_init(DLIST_NODE *dlist)
{
    if(NULL == dlist)
    {
        return;
    }
    dlist->next = dlist;
    dlist->prev = dlist;
}

BOOL dlist_empty(const DLIST_NODE *dlist)
{
    if(NULL == dlist || NULL == dlist->next || NULL == dlist->prev)
    {
        return TRUE;
    }
    return (dlist->next == dlist || dlist->prev == dlist) ? TRUE : FALSE;
}

VOID dlist_add_head(DLIST_NODE *new, DLIST_NODE *dlist)
{
    if(NULL == new || NULL == dlist || 
       NULL == dlist->next || NULL == dlist->prev)
    {
        return;
    }
    new->next         = dlist->next;
    new->prev         = dlist;
    dlist->next->prev = new;
    dlist->next       = new;
}

VOID dlist_add_tail(DLIST_NODE *new, DLIST_NODE *dlist)
{
    if(NULL == new || NULL == dlist ||
       NULL == dlist->next || NULL == dlist->prev)
    {
        return;
    }
    new->prev         = dlist->prev;
    new->next         = dlist;
    dlist->prev->next = new;
    dlist->prev       = new;
}

VOID dlist_del_entry(DLIST_NODE *entry)
{
    if(NULL == entry || 
      NULL == entry->next || 
      NULL == entry->prev)
    {
        return;
    }
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

VOID dlist_replace(DLIST_NODE *old, DLIST_NODE *new)
{
    if (NULL == old || NULL == new)
    {
        return;
    }
    new->next       = old->next;
    new->next->prev = new;
    new->prev       = old->prev;
    new->prev->next = new;
}

VOID dlist_move(DLIST_NODE *entry, DLIST_NODE *dlist)
{
    if(NULL == entry || NULL == dlist || 
       NULL == dlist->next || NULL == dlist->prev)
    {
        return;
    }
    dlist_del_entry(entry);
    dlist_add_head(entry, dlist);
}

VOID dlist_move_tail(DLIST_NODE *entry, DLIST_NODE *dlist)
{
    if(NULL == entry || NULL == dlist ||
       NULL == dlist->next || NULL == dlist->prev)
    {
        return;
    }
    dlist_del_entry(entry);
    dlist_add_tail(entry, dlist);
}

UINT32 dlist_count(const DLIST_NODE *dlist)
{
    struct dlist_node *node;
    UINT32           cnt = 0;
    if(NULL == dlist || NULL == dlist->next || NULL == dlist->prev)
    {
        return 0;
    }
    for (node = dlist->next; node != dlist; node = node->next)
    {
        cnt++;
    }

    return cnt;
}

DLIST_NODE *dlist_pop_first_node(DLIST_NODE *dlist)
{
    struct dlist_node *ret = NULL;
    if(NULL == dlist || NULL == dlist->next || NULL == dlist->prev)
    {
        return NULL;
    }
    if (!dlist_empty(dlist))
    {
        ret = dlist->next;
        dlist_del_entry(ret);
    }
    return ret;
}

DLIST_NODE *dlist_pop_last_node(DLIST_NODE *dlist)
{
    DLIST_NODE *ret = NULL;
    if(NULL == dlist || NULL == dlist->next || NULL == dlist->prev)
    {
        return NULL;
    }
    if (!dlist_empty(dlist))
    {
        ret = dlist->prev;
        dlist_del_entry(ret);
    }
    return ret;
}

BOOL dlist_has_node(const DLIST_NODE *dlist, const DLIST_NODE *node)
{
    if(NULL == dlist || NULL == node || dlist->next == NULL || dlist->prev == NULL)
    {
        return FALSE;
    }
    DLIST_NODE *tmp = dlist->next;
    while (tmp != dlist)
    {
        if (tmp == node)
        {
            return TRUE;
        }
        tmp = tmp->next;
    }
    return FALSE;
}