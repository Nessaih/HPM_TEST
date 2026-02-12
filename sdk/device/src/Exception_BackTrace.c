
#include <stdint.h>
#include <stdio.h>


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

extern stack_info_t stack;

void print_stack_info(stack_info_t *stack_info)
{
    if (stack_info == NULL)
    {
        return;
    }

    // Print register values
    printf("\n=== Hard Fault Stack Information ===\n");
    printf("Task Name: %s\n", stack_info->name);
    printf("\nRegisters:\n");
    printf("  r0: 0x%08X\n", stack_info->r0);
    printf("  r1: 0x%08X\n", stack_info->r1);
    printf("  r2: 0x%08X\n", stack_info->r2);
    printf("  r3: 0x%08X\n", stack_info->r3);
    printf("  r13(sp): 0x%08X\n", stack_info->r13);
    printf("  lr: 0x%08X\n", stack_info->lr);
    printf("  pc: 0x%08X\n", stack_info->pc);
    printf("  psr: 0x%08X\n", stack_info->psr);

    // Print backtrace buffer
    printf("\nBacktrace (depth: %u):\n", stack_info->depth);
    for (uint32_t i = 0; i < stack_info->depth; i++)
    {
        printf("  [%u]: 0x%08X\n", i, stack_info->buffer[i]);
    }
    printf("====================================\n\n");
}