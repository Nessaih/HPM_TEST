#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include "tbox_common.h"

#define DATA_QUEUE_MAGICNO 0x0AA0

typedef enum
{
  DATA_QUEUE_RET_SUCCESS = 0U,
  DATA_QUEUE_RET_INVALIDPARAM,
  DATA_QUEUE_RET_INVALIDDATA,
  DATA_QUEUE_RET_UNDERFLOW,
  DATA_QUEUE_RET_OVERFLOW,
  DATA_QUEUE_RET_UNKNOWN
}DATA_QEUEU_RET_CODE;

typedef struct
{
  UINT16 magic_no;
  UINT8  element_type;
  UINT8  param;
  UINT16 element_len;
}DATA_ELEMENT_HEADER, *DATA_ELEMENT_HEADER_PTR;

typedef struct
{
  UINT16 head;
  UINT16 tail;
  UINT8 *buf_ptr;
  UINT16 len;
}DATA_QUEUE, *DATA_QUEUE_PTR;

VOID dataqueue_init(DATA_QUEUE_PTR data_queue);

UINT8 dataqueue_enqueue(DATA_QUEUE_PTR data_queue,
                        DATA_ELEMENT_HEADER_PTR element_header,
                        UINT8 *element_body);

UINT8 dataqueue_recover(DATA_QUEUE_PTR data_queue,
                        UINT16 lenght);

UINT8 dataqueue_dequeue_header(DATA_QUEUE_PTR data_queue,
                               DATA_ELEMENT_HEADER_PTR element_header);

UINT8 dataqueue_get_header(DATA_QUEUE_PTR data_queue,
                           DATA_ELEMENT_HEADER_PTR element_header);

UINT8 dataqueue_dequeue(DATA_QUEUE_PTR data_queue, UINT16 len);

UINT8 dataqueue_dequeue_body(DATA_QUEUE_PTR data_queue,
                             UINT16 lenght,
                             UINT8* element_body);

UINT8 dataqueue_get_data(DATA_QUEUE_PTR data_queue,
                         UINT16 lenght,
                         UINT8* element_body);

BOOL dataqueue_can_put_data(DATA_QUEUE_PTR data_queue, UINT16 lenght, UINT8 slice_num);

#endif /* DATA_QUEUE_H */
