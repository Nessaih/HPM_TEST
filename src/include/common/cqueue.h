
#ifndef _FIFO_H_
#define _FIFO_H_

#include <stdint.h>

// #pragma pack (1)
typedef struct {
    uint8_t *buffer; /* the buffer holding the data */
    uint32_t size;   /* the size of the allocated buffer (should be set as: 2 ^ n) */
    uint32_t in;     /* data is added at offset (in % size) */
    uint32_t out;    /* data is extracted from off. (out % size) */
} cqueue_t;
// #pragma pack ()

extern uint32_t cqueue_put(cqueue_t *queue, const uint8_t *buffer, uint32_t len);
extern uint32_t cqueue_get(cqueue_t *queue, uint8_t *buffer, uint32_t len);
extern uint32_t cqueue_discard(cqueue_t *queue, uint32_t len);
extern uint32_t cqueue_surplus(cqueue_t *queue);
extern uint32_t cqueue_datalen(cqueue_t *queue);
extern uint32_t cqueue_read(cqueue_t *queue, uint8_t *buffer, uint32_t len);
extern uint32_t cqueue_init(cqueue_t *queue, uint8_t *buffer, uint32_t size);

#endif
