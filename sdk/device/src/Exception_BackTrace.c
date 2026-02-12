
#include <stdint.h>

typedef struct
{
    uint32_t reg[8];
    uint32_t depth;
    uint32_t buffer[32];
    char     name[32];
} stack_info_t;

extern stack_info_t stack;
extern void         dump_print(const char *format, ...);

void print_stack_info(stack_info_t *stack_info)
{
    if (!stack_info)
        return;

    dump_print("\n=== Hard Fault Stack Information ===\n");
    dump_print("Task Name: %s\n", stack_info->name);
    dump_print("\nRegisters:\n");

    for (uint8_t i = 0; i < 8; i++)
    {
        dump_print("  r%u : 0x%08X\n", i, stack_info->reg[i]);
    }

    dump_print("\nBacktrace depth:%u\n", stack_info->depth);
    dump_print("Run command:addr2line -e .\\TZS100_VPU.out -afpiC");
    for (uint32_t i = 0; i < stack_info->depth; i++)
    {
        dump_print(" %X", stack_info->buffer[i]);
    }
    dump_print("\n====================================\n\n");
}