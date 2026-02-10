/**
 *******************************************************************************
 * @file       .\sources\algorithm\circle_queue\cqueue.c
 * @brief      description
 * @record
 * Change Logs:
 * Date             Author          Notes
 * 2025/12/23       David             First version
 * 2025/02/11       David             Add cqueue_init function.
 *******************************************************************************
 */
#include "cqueue.h"
#include "string.h"

/**
 *@brief   Find the largest power of 2 not greater than the input value
 *@param  val Input unsigned 32-bit integer
 *@retval Largest power of 2 not greater than val (returns 0 if val is 0)
 *@date   2025-12-23
 *@note   Fix MISRAC2012-Rule-14.3_b : Remove parameter check because the only caller has already checked
 */
static inline uint32_t align_to_power(uint32_t val)
{
    /*
    Fix MISRAC2012-Rule-14.3_b: Remove redundant check, already verified in the only caller cqueue_init
    if (val == 0U) {
        return 0U;
    }
    */

    // Find the most significant bit (MSB) through bit operations
    uint32_t power = 0x80000000U;
    while (power > val) {
        power >>= 1U;
    }

    return power;
}

/**
 *@brief   Get a minimum value not less than zero.
 *@param   int a
 *@param   int a
 *@retval  uint32_t  len        minimum value not less than zero.
 *@date    2025/12/23
 *@note    1. fix MISRAC2012-Rule-14.3_b : Parameter type changed to avoid implicit conversion.
 *         2. Remove invalid checks, simplify logic, improve efficiency.
 */

static inline uint32_t min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

/**
 *@brief   Put data to queue
 *@param   cqueue_t   *queue
 *@param   uint8_t  *buffer
 *@param   uint32_t  len
 *@retval  uint32_t  len        The actual length of data put in.
 *@date    2025/12/23
 */
uint32_t cqueue_put(cqueue_t *queue, const uint8_t *buffer, uint32_t len)
{
    uint32_t l;

    len = min(len, queue->size - queue->in + queue->out);
    l   = min(len, queue->size - (queue->in & (queue->size - 1U)));
    memcpy(queue->buffer + (queue->in & (queue->size - 1U)), buffer, l);
    memcpy(queue->buffer, &buffer[l], len - l);
    queue->in += len;
    return len;
}

/**
 *@brief   Get data out of queue
 *@param   cqueue_t   *queue
 *@param   uint8_t  *buffer
 *@param   uint32_t  len
 *@retval  uint32_t  len        The actual length of the fetched data.
 *@date    2025/12/23
 */
uint32_t cqueue_get(cqueue_t *queue, uint8_t *buffer, uint32_t len)
{
    uint32_t l;

    len = min(len, queue->in - queue->out);
    l   = min(len, queue->size - (queue->out & (queue->size - 1U)));
    memcpy(buffer, queue->buffer + (queue->out & (queue->size - 1U)), l);
    memcpy(&buffer[l], queue->buffer, len - l);
    queue->out += len;

    return len;
}
/* discard some data of queue */

/**
 *@brief   Discard some data of queue.
 *@param   cqueue_t   *queue
 *@param   uint32_t  len
 *@retval  none
 *@date    2025/02/11
 */
uint32_t cqueue_discard(cqueue_t *queue, uint32_t len)
{
    len = min(len, queue->in - queue->out);
    queue->out += len;
    return len;
}

/**
 *@brief   Get the surplus space length of queue
 *@param   cqueue_t  *queue
 *@retval  uint32_t  len        The surplus space length of queue.
 *@date    2025/12/23
 */
uint32_t cqueue_surplus(cqueue_t *queue)
{
    return (queue->size - queue->in + queue->out);
}

/**
 *@brief   Get the data length of queue
 *@param   cqueue_t   *queue
 *@retval  uint32_t  len        The data length of queue.
 *@date    2025/12/23
 */
uint32_t cqueue_datalen(cqueue_t *queue)
{
    return (queue->in - queue->out);
}

/**
 *@brief   Read the data from the queue, but the data in the queue is not fetched.
 *@param   cqueue_t   *queue
 *@retval  uint32_t  len        The data length of queue.
 *@date    2025/12/23
 */
uint32_t cqueue_read(cqueue_t *queue, uint8_t *buffer, uint32_t len)
{
    uint32_t l;

    len = min(len, queue->in - queue->out);
    l   = min(len, queue->size - (queue->out & (queue->size - 1U)));
    memcpy(buffer, queue->buffer + (queue->out & (queue->size - 1U)), l);
    memcpy(&buffer[l], queue->buffer, len - l);
    return len;
}

uint32_t cqueue_init(cqueue_t *queue, uint8_t *buffer, uint32_t size)
{
    if (size == 0U) {
        return 0U;
    }

    queue->buffer = buffer;
    queue->size   = align_to_power(size);
    queue->in     = 0U;
    queue->out    = 0U;

    return queue->size;
}
