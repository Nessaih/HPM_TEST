#ifndef TBOX_DLIST_H
#define TBOX_DLIST_H

/*******************************************************************************
  * FileName    : tbox_dlist.h
  * Date        : 2025/12/09 10:01:48
  * Author      : lgc
  * Version     : v1.0
  * Decription  : 实现双向链表，每个链表在进行插入或者删除之前，都需要调用dlist_init初始化，不然就会dump
  * History:
  *   Date        Author      Version     Description
  *   2025/12/09  lgc         v1.0        创建文件，实现双向链表的基本操作
 *******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*the doubly linked list*/
typedef struct dlist_node
{
    struct dlist_node *prev;
    struct dlist_node *next;
} DLIST_NODE;

VOID dlist_init(DLIST_NODE *dlist);

BOOL dlist_empty(const DLIST_NODE *dlist);

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
VOID dlist_add_head(DLIST_NODE *new, DLIST_NODE *dlist);

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
VOID dlist_add_tail(DLIST_NODE *new, DLIST_NODE *dlist);

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
VOID dlist_del_entry(DLIST_NODE *entry);

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
VOID dlist_replace(DLIST_NODE *old, DLIST_NODE *new);

/**
 * list_move - delete from one list and add as another's head
 * @entry: the entry to move
 * @list: the head that will precede our entry
 */
VOID dlist_move(DLIST_NODE *entry, DLIST_NODE *dlist);

/**
 * list_move_tail - delete from one list and add as another's tail
 * @entry: the entry to move
 * @list: the head that will follow our entry
 */
VOID dlist_move_tail(DLIST_NODE *entry, DLIST_NODE *dlist);

UINT32 dlist_count(const DLIST_NODE *dlist);

DLIST_NODE *dlist_pop_first_node(DLIST_NODE *dlist);

DLIST_NODE *dlist_pop_last_node(DLIST_NODE *dlist);

BOOL dlist_has_node(const DLIST_NODE *dlist, const DLIST_NODE *node);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_DLIST_H */