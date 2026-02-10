#ifndef __CAN_LIST_H__
#define __CAN_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dlist.h"
#include "can_types.h"

typedef struct can_list
{
    struct list_head head;
    uint32_t         size;
} can_list_t;

typedef struct can_node
{
    can_msg_t        msg;
    struct list_head list;
} can_node_t;

extern void        can_list_init(can_list_t *list);
extern uint32_t    can_list_space(void);
extern void        can_list_push(can_list_t *list, can_node_t *node);
extern can_node_t *can_list_pop(can_list_t *list);
extern void        can_list_free(can_node_t *node);
extern can_node_t *can_list_malloc(void);

#ifdef __cplusplus
}
#endif

#endif 