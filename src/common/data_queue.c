#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "data_queue.h"

VOID dataqueue_init(DATA_QUEUE_PTR data_queue)
{
    data_queue->buf_ptr = NULL_PTR;
    data_queue->len = 0U;
    data_queue->head = data_queue->tail = 0U;
}

UINT8 dataqueue_enqueue(DATA_QUEUE_PTR data_queue,
                        DATA_ELEMENT_HEADER_PTR element_header,
                        UINT8 *element_body)
{
    UINT16 idle_size = 0U;
    UINT16 need_size = 0U;
    UINT16 write_size = 0U;
    UINT16 index = 0U;
    UINT8 *temp_ptr;

    if(DATA_QUEUE_MAGICNO != element_header->magic_no
     || element_header->element_len <= 0U
     || element_header->element_len > data_queue->len)
    {
       return (UINT8)DATA_QUEUE_RET_INVALIDPARAM;
    }

    need_size = sizeof(DATA_ELEMENT_HEADER) + element_header->element_len;
    idle_size = (data_queue->head - data_queue->tail + data_queue->len - 1U) % data_queue->len;
    if(idle_size < need_size)
    {
      return (UINT8)DATA_QUEUE_RET_OVERFLOW;
    }

    write_size = sizeof(DATA_ELEMENT_HEADER);
    temp_ptr = (uint8 *)element_header;
    for(index = 0; index < write_size; index++)
    {
        data_queue->buf_ptr[data_queue->tail] = temp_ptr[index];
        data_queue->tail = (data_queue->tail + 1) % data_queue->len;
    }

    write_size = element_header->element_len;
    temp_ptr = element_body;
    for(index = 0; index < write_size; index++)
    {
        data_queue->buf_ptr[data_queue->tail] = temp_ptr[index];
        data_queue->tail = (data_queue->tail + 1) % data_queue->len;
    }

    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

UINT8 dataqueue_recover(DATA_QUEUE_PTR data_queue,
                        UINT16 lenght)
{
    if(lenght <= 0U)
    {
      return (UINT8)DATA_QUEUE_RET_INVALIDPARAM;
    }

    data_queue->tail = (data_queue->tail - lenght + data_queue->len) % data_queue->len;

    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

UINT8 dataqueue_dequeue_header(DATA_QUEUE_PTR data_queue,
                               DATA_ELEMENT_HEADER_PTR element_header)
{
    UINT16 need_size = 0U;
    UINT16 used_size = 0U;
    UINT16 index = 0U;
    UINT8* temp_ptr = NULL_PTR;

    need_size = sizeof(DATA_ELEMENT_HEADER);
    used_size = (data_queue->tail - data_queue->head + data_queue->len) % data_queue->len;
    if(used_size < need_size)
    {
       return (UINT8)DATA_QUEUE_RET_UNDERFLOW;
    }

    temp_ptr = (uint8 *)element_header;
    for(index = 0U; index < need_size; index++ )
    {
        temp_ptr[index] = data_queue->buf_ptr[data_queue->head];
        data_queue->head = (data_queue->head + 1) % data_queue->len;
    }

    if(DATA_QUEUE_MAGICNO != element_header->magic_no)
    {
       return (UINT8)DATA_QUEUE_RET_INVALIDDATA;
    }

    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

UINT8 dataqueue_get_header(DATA_QUEUE_PTR data_queue,
                           DATA_ELEMENT_HEADER_PTR element_header)
{
    UINT16 need_size = 0U;
    UINT16 used_size = 0U;
    UINT16 index = 0U;
    UINT16 temp_head = data_queue->head;
    UINT8* temp_ptr;

    need_size = sizeof(DATA_ELEMENT_HEADER);
    used_size = (data_queue->tail - temp_head + data_queue->len) % data_queue->len;
    if(used_size < need_size)
    {
       return (UINT8)DATA_QUEUE_RET_UNDERFLOW;
    }

    temp_ptr = (uint8 *)element_header;
    for(index = 0; index < need_size; index++ )
    {
        temp_ptr[index] = data_queue->buf_ptr[temp_head];
        temp_head = (temp_head + 1) % data_queue->len;
    }

    if(DATA_QUEUE_MAGICNO != element_header->magic_no)
    {
       return (UINT8)DATA_QUEUE_RET_INVALIDDATA;
    }

    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

UINT8 dataqueue_dequeue(DATA_QUEUE_PTR data_queue, UINT16 len)
{
    UINT16 used_size = 0U;

    used_size = (data_queue->tail - data_queue->head + data_queue->len) % data_queue->len;
    if(used_size < len)
    {
       return (UINT8)DATA_QUEUE_RET_UNDERFLOW;
    }

    while(len-- > 0U)
    {
        data_queue->head = (data_queue->head + 1) % data_queue->len;
    }
    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

UINT8 dataqueue_dequeue_body(DATA_QUEUE_PTR data_queue,
                             UINT16 lenght,
                             UINT8* element_body)
{
    UINT16 used_size = 0U;
    UINT16 index = 0U;

    if(lenght <= 0U)
    {
       return (UINT8)DATA_QUEUE_RET_INVALIDPARAM;
    }

    used_size = (data_queue->tail - data_queue->head + data_queue->len) % data_queue->len;
    if(used_size < lenght)
    {
       return (UINT8)DATA_QUEUE_RET_UNDERFLOW;
    }

    for(index = 0U; index < lenght; index++ )
    {
        element_body[index] = data_queue->buf_ptr[data_queue->head];
        data_queue->head = (data_queue->head + 1) % data_queue->len;
    }

    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

UINT8 dataqueue_get_data(DATA_QUEUE_PTR data_queue,
                         UINT16 lenght,
                         UINT8* element_body)
{
    UINT16 used_size = 0U;
    UINT16 index = 0U;
    UINT16 temp_head = data_queue->head;

    if(lenght <= 0U)
    {
       return (UINT8)DATA_QUEUE_RET_INVALIDPARAM;
    }

    used_size = (data_queue->tail - temp_head + data_queue->len) % data_queue->len;
    if(used_size < sizeof(DATA_ELEMENT_HEADER))
    {
       return (UINT8)DATA_QUEUE_RET_UNDERFLOW;
    }
    for(index = 0U; index < sizeof(DATA_ELEMENT_HEADER); index++ )
    {
        temp_head = (temp_head + 1) % data_queue->len;
    }

    used_size = (data_queue->tail - temp_head + data_queue->len) % data_queue->len;
    if(used_size < lenght)
    {
       return (UINT8)DATA_QUEUE_RET_UNDERFLOW;
    }
    for(index = 0U; index < lenght; index++ )
    {
        element_body[index] = data_queue->buf_ptr[temp_head];
        temp_head = (temp_head + 1) % data_queue->len;
    }

    return (UINT8)DATA_QUEUE_RET_SUCCESS;
}

BOOL dataqueue_can_put_data(DATA_QUEUE_PTR data_queue, UINT16 lenght, UINT8 slice_num)
{
    UINT16 need_size = (slice_num * sizeof(DATA_ELEMENT_HEADER)) + lenght;
    UINT16 idle_size = (data_queue->head - data_queue->tail + data_queue->len - 1U) % data_queue->len;
    if (idle_size < need_size)
    {
        return FALSE;
    }
    return TRUE;
}