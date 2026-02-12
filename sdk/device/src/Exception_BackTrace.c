
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r13;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
    uint32_t depth;
    uint32_t buffer[32];
    char     name[32];
} stack_info_t;

stack_info_t stack;

#pragma section = ".text"

void backtrace(uint32_t handle_lr, uint32_t handle_sp, uint32_t handle_fpu)
{
    uint32_t   *spp = (uint32_t *)handle_sp;
    uint32_t   *pcp = spp + 6;
    const char *tn  = pxTaskName(NULL);
    uint32_t    sp  = handle_sp;
    uint32_t    pc  = handle_lr - 1;
    uint32_t    cs  = (uint32_t)__section_begin(".text");
    uint32_t    ce  = (uint32_t)__section_end(".text");
    uint32_t    st  = uxTaskStackTop(NULL);
    uint16_t    dp  = 0;

    stack.r0  = spp[0];
    stack.r1  = spp[1];
    stack.r2  = spp[2];
    stack.r3  = spp[3];
    stack.r13 = spp[4];
    stack.lr  = spp[5];
    stack.pc  = spp[6];
    stack.psr = spp[7];
    strncpy(stack.name, tn, 32 - 1);

    for (int i = 2; i > 0; --i)
    {
        if (*pcp > cs && *pcp < ce)
            stack.buffer[dp++] = *pcp;
        pcp--;
    }

    if (handle_fpu)
    {
        spp += 18;
    }

    sp = (uint32_t)spp;

    for (; sp < st; sp += sizeof(size_t))
    {
        pc = *((uint32_t *)sp) - sizeof(size_t);
        if (pc % 2 == 0)
            continue;
        pc = *((uint32_t *)sp) - 1;

        if (pc > cs && pc < ce && dp < 32)
        {
            uint16_t *pi = (uint16_t *)(pc);
            if (((pi[-1] & 0xFF00) == 0x4700) || (((pi[-1] & 0xF800) == 0xF800) && ((pi[-2] & 0xF800) == 0xF000)))
                stack.buffer[dp++] = pc;
        }
    }
    stack.depth = dp;
}