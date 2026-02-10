#ifndef __C_LIST_H
#define __C_LIST_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define OFFSET(TYPE, MEMBER)   ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) (type *)((char *)ptr -OFFSET(type,member))

#define DLIST_POISON1  ((void *) 0x00100100)
#define DLIST_POISON2  ((void *) 0x00200200)

#define dlist_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define DLIST_HEAD_INIT(name) { &(name), &(name) }

#define DLIST_HEAD(name) \
    struct dlist_head name = DLIST_HEAD_INIT(name)

#define dlist_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
struct dlist_head {
    struct dlist_head *next, *prev;
};
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static inline void INIT_DLIST_HEAD(struct dlist_head *list) {
    list->next = list;
    list->prev = list;
}

static inline void __dlist_add(struct dlist_head *new,
                              struct dlist_head *prev,
                              struct dlist_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void dlist_add(struct dlist_head *new, struct dlist_head *head) {
    __dlist_add(new, head, head->next);
}

static inline void __dlist_del(struct dlist_head * prev, struct dlist_head * next) {
    next->prev = prev;
    prev->next = next;
}

static inline void dlist_del(struct dlist_head *entry) {
    __dlist_del(entry->prev, entry->next);
    entry->next = DLIST_POISON1;
    entry->prev = DLIST_POISON2;
}

static inline int dlist_empty(const struct dlist_head *head) {
    return head->next == head;
}

#define dlist_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = (pos)->next; pos != (head); \
    pos = n, n = (pos)->next)

static inline void dlist_add_tail(struct dlist_head *new, struct dlist_head *head) {
    __dlist_add(new, head->prev, head);
}
#endif // __C_LIST_H
