#include "can_config.h"
#include "can_list.h"
#include "dlist.h"
#include "drv_can.h"

#define CAN_LIST_MAX_SIZE 100U

static can_node_t list_nodes[CAN_LIST_MAX_SIZE];
static can_list_t free_list;

static void can_free_list_init(void)
{
    static bool is_inited = FALSE;

    if (is_inited) {
        return;
    }

    LIST_INIT_HEAD(&free_list.head);

    for (size_t i = 0; i < CAN_LIST_MAX_SIZE; i++) {
        list_add_tail(&list_nodes[i].list, &free_list.head);
    }
    free_list.size = CAN_LIST_MAX_SIZE;
    is_inited      = TRUE;
}

uint32_t can_list_space(void)
{
    return free_list.size;
}

void can_list_init(can_list_t *list)
{
    LIST_INIT_HEAD(&list->head);
    list->size = 0;
    can_free_list_init();
}

void can_list_push(can_list_t *list, can_node_t *node)
{
    if (NULL == list || NULL == node) {
        return;
    }
    list_add_tail(&node->list, &list->head);
    ++list->size;
}

can_node_t *can_list_pop(can_list_t *list)
{
    can_node_t *node;

    if (NULL == list || 0u == list->size) {
        return NULL;
    }

    node = list_first_entry(&list->head, can_node_t, list);
    list_del(&list->head);
    --list->size;
    return node;
}

void can_list_free(can_node_t *node)
{
    if (NULL == node) {
        return;
    }
    list_add_tail(&node->list, &free_list.head);
    ++free_list.size;
}

can_node_t *can_list_malloc(void)
{
    can_node_t *node;

    if (0u == free_list.size) {
        return NULL;
    }

    node = list_first_entry(&free_list.head, can_node_t, list);
    list_del(&free_list.head);
    --free_list.size;
    return node;
}
