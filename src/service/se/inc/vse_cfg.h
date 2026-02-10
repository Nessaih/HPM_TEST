#ifndef _VSE_CFG_H_
#define _VSE_CFG_H_


#include "vse_type.h"
#include <stdarg.h>


/* Should check align. */
typedef struct {
    uint64_t sec;
    uint32_t msec; /* 0 ~ 999 */
} vse_time_t;


void    vse_gpio_reset(void);
void    vse_gpio_wakeup(void);
void    vse_delay_us(uint16_t usec);
void    vse_delay_ms(uint16_t msec);
int32_t vse_get_time(vse_time_t *time);

int32_t vse_init(void);
int32_t vse_deinit(void);
int32_t vse_send(uint8_t *data, uint16_t length);
int32_t vse_recv(uint8_t *data, uint16_t length);
void    vse_printf(const char *fmt, ...);
 

#endif /* _VSE_CFG_H_ */
