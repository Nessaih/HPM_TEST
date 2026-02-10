/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __DLIST_H__
#define __DLIST_H__

/*
 * Copied from include/linux/...
 */
#include <stddef.h>
#include <stdbool.h>
#include "tbox_common.h"

struct list_head
{
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name)      struct list_head name = LIST_HEAD_INIT(name)

#define LIST_INIT_HEAD(list)                                                                                           \
    do {                                                                                                               \
        (list)->next = (list);                                                                                         \
        (list)->prev = (list);                                                                                         \
    } while (0)

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:   the type of the struct this is
 * embedded in.
 * @member: the name of the list_head within the
 * struct.
 */
#define list_entry(ptr, type, member)      TBOX_CONTAINER(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)

/**
 * list_for_each_entry  -   iterate over list of
 * given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_head within the
 * struct.
 */
#define list_for_each_entry(pos, head, member)                                                                         \
    for (pos = list_entry((head)->next, typeof(*pos), member); &pos->member != (head);                                 \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of
 * given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary
 * storage
 * @head:   the head for your list.
 * @member: the name of the list_head within the
 * struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)                                                                 \
    for (pos = list_entry((head)->next, typeof(*pos), member), n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                                                                       \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline bool list_empty(const struct list_head *head)
{
    return head->next == head;
}

/*
 * Insert a new entry between two known
 * consecutive entries.
 *
 * This is only for internal list manipulation
 * where we know the prev/next entries already!
 */
static inline void __list_add(struct list_head *_new, struct list_head *prev, struct list_head *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next
 * entries point to each other.
 *
 * This is only for internal list manipulation
 * where we know the prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

// #define LIST_POISON1 ((void *)0x00100100)
// #define LIST_POISON2 ((void *)0x00200200)
#define LIST_POISON1 ((void *)0x00000000)
#define LIST_POISON2 ((void *)0x00000000)
/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return
 * true after this, the entry is in an undefined
 * state.
 */

static inline struct list_head *list_del(struct list_head *entry)
{
    struct list_head *node;

    if (list_empty(entry)) {
        return NULL;
    } else {
        node = entry->next;
        __list_del(entry, entry->next->next);
    }
    return node;
}

// static inline struct list *list_del(struct
// list_head *entry)
// {
//     __list_del(entry->prev, entry->next);
// }
#endif
