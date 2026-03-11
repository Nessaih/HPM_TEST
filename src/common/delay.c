

#include "ac7840x.h"
#include "delay.h"

void delay_us(int32_t tick)
{
    tick = tick * 23;
    while (tick > 0)
    {
        __NOP();
        --tick;
    }
}

void delay_ms(int32_t tick)
{
    tick = tick * 24000;
    while (tick > 0)
    {
        __NOP();
        --tick;
    }
}