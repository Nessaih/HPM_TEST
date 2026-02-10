

#include "ac7840x.h"
#include "delay.h"

void delay_us(int32_t tick)
{
    uint8_t i;

    while (tick > 0) {
        for (i = 16; i > 0u; --i)
            __NOP();
        tick -= 10;
    }
}

void delay_ms(int32_t tick)
{
    uint16_t i;

    while (tick > 0) {
        for (i = 17000; i > 0u; --i)
            __NOP();
        tick -= 1;
    }
}