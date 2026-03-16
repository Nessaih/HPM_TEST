

#include "ac7840x.h"
#include "delay.h"

void delay_us(int32_t tick)
{
    int64_t count = (int64_t)tick * 23;
    while (count > 0)
    {
        __NOP();
        --count;
    }
}

void delay_ms(int32_t tick)
{
    int64_t count = (int64_t)tick * 24000;
    while (count > 0)
    {
        __NOP();
        --count;
    }
}