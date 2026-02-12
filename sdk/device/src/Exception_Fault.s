    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT pxTaskName
    IMPORT uxTaskStackTop
    IMPORT print_stack_info
    EXPORT HardFault_Handler
    EXPORT stack

    ; Define stack_info_t structure in memory - using IAR assembler syntax
    SECTION    .bss:DATA(3)
    ALIGNRAM   2, 0x00000000  ; Align to 2^2 = 4 bytes, fill with 0 (IAR syntax)
stack:                          ; Base address of stack_info_t structure
    DS8 320                     ; Allocate 320 bytes for the entire structure
                                ; 8 registers * 4 bytes = 32 bytes
                                ; 32 buffer entries * 4 bytes = 128 bytes
                                ; 32 bytes for task name

    SECTION    .text:CODE(2)

HardFault_Handler
    MOV     r0, lr
    MOVS    r1, #0x04
    MOVS    r2, #0x10

    TST     r0, r1
    ITE     ne
    MRSNE   r1, psp
    MRSEQ   r1, msp

    TST     r0, r2
    ITE     ne
    MOVSNE  r2, #0
    MOVSEQ  r2, #1

    BL      backtrace

    ; Call C function to print stack info - with interrupt protection
    CPSID   i                     ; Close all interrupts (IRQ and FIQ)
    LDR     r0, =stack
    BL      print_stack_info
    CPSIE   i                     ; Open all interrupts (IRQ and FIQ)

Fault_Loop
    BL      Fault_Loop              ;while(1)

backtrace
    ; r0 = handle_lr
    ; r1 = handle_sp
    ; r2 = handle_fpu
    PUSH    {r4-r11, lr}

    ; Save current registers
    MOV     r4, r0                  ; handle_lr
    MOV     r5, r1                  ; handle_sp
    MOV     r6, r2                  ; handle_fpu

    ; spp = (uint32_t *)handle_sp
    MOV     r7, r5                  ; spp

    ; stack.r0 = spp[0]
    LDR     r8, [r7, #0]
    LDR     r9, =stack
    STR     r8, [r9, #0]            ; stack.r0

    ; stack.r1 = spp[1]
    LDR     r8, [r7, #4]
    STR     r8, [r9, #4]            ; stack.r1

    ; stack.r2 = spp[2]
    LDR     r8, [r7, #8]
    STR     r8, [r9, #8]            ; stack.r2

    ; stack.r3 = spp[3]
    LDR     r8, [r7, #12]
    STR     r8, [r9, #12]           ; stack.r3

    ; stack.r13 = spp[4]
    LDR     r8, [r7, #16]
    STR     r8, [r9, #16]           ; stack.r13

    ; stack.lr = spp[5]
    LDR     r8, [r7, #20]
    STR     r8, [r9, #20]           ; stack.lr

    ; stack.pc = spp[6]
    LDR     r8, [r7, #24]
    STR     r8, [r9, #24]           ; stack.pc

    ; stack.psr = spp[7]
    LDR     r8, [r7, #28]
    STR     r8, [r9, #28]           ; stack.psr

    ; Get task name
    MOV     r0, #0                  ; NULL
    BL      pxTaskName

    ; Copy task name to stack.name
    MOV     r1, r0                  ; tn
    LDR     r0, =stack
    ADD     r0, r0, #160            ; stack.name offset: 8 registers * 4 + 32 buffer entries * 4 = 32 + 128 = 160
    MOV     r2, #31                 ; max length
    BL      strncpy

    ; Initialize depth
    MOV     r0, #0
    STR     r0, [r9, #32]           ; stack.depth offset: 8 registers * 4 = 32

    ; Get .text section boundaries
    ; cs = (uint32_t)__section_begin(".text")
    ; ce = (uint32_t)__section_end(".text")
    LDR     r8, =sfb(.text) + 4        ; cs
    LDR     r9, =sfe(.text) + 4        ; ce

    ; Equivalent of C code: for (int i = 2; i > 0; --i)
    ; { if (*pcp > cs && *pcp < ce) stack.buffer[dp++] = *pcp; pcp--; }
    LDR     r0, =stack                 ; Base address of stack structure
    MOV     r1, r7                     ; r1 = spp (handle_sp)
    ADD     r1, r1, #24                ; pcp = spp + 6 (spp[6] = pc)
    LDR     r2, [r0, #32]              ; dp = stack.depth
    MOV     r12, #2                    ; i = 2

loop_pcp:
    ; Check loop condition: i > 0
    CMP     r12, #0
    BLE     end_loop_pcp

    ; Load *pcp
    LDR     r3, [r1]

    ; Check if *pcp > cs && *pcp < ce
    CMP     r3, r8
    BLE     skip_pcp_store
    CMP     r3, r9
    BGE     skip_pcp_store

    ; Store *pcp to stack.buffer[dp]
    LSL     r4, r2, #2                 ; offset = dp * 4
    ADD     r4, r4, #36                ; buffer base offset (32 + 4)
    STR     r3, [r0, r4]

    ; Increment dp
    ADD     r2, r2, #1

skip_pcp_store:
    ; Decrement pcp
    SUB     r1, r1, #4

    ; Decrement i
    SUB     r12, r12, #1

    ; Repeat loop
    B       loop_pcp

end_loop_pcp:
    ; Save updated dp to stack.depth
    STR     r2, [r0, #32]

    ; Get stack top
    MOV     r0, #0                  ; NULL
    BL      uxTaskStackTop
    MOV     r10, r0                 ; st = uxTaskStackTop(NULL)

    ; Process stack - start with current dp value from first loop
    MOV     r0, r5                  ; sp = handle_sp
    MOV     r11, r2                 ; dp = current depth (don't reset to 0!)

    ; Check if FPU context exists
    CMP     r6, #1
    BEQ     fpu_context

stack_loop:
    ; Check if sp < st
    CMP     r0, r10
    BGE     stack_loop_end

    ; pc = *((uint32_t *)sp) - 1
    LDR     r1, [r0]
    SUB     r1, r1, #1

    ; Check if pc is within .text section
    CMP     r1, r8
    BLE     next_stack
    CMP     r1, r9
    BGE     next_stack

    ; Check if dp < 32
    CMP     r11, #32
    BGE     next_stack

    ; Check instruction pattern
    ; pi = (uint16_t *)(pc)
    ; Check if (pi[-1] & 0xFF00) == 0x4700
    SUB     r2, r1, #2
    LDRH    r3, [r2]
    AND     r3, r3, #0xFF00
    CMP     r3, #0x4700
    BEQ     save_pc

    ; Check if ((pi[-1] & 0xF800) == 0xF800) && ((pi[-2] & 0xF800) == 0xF000)
    SUB     r2, r1, #2
    LDRH    r3, [r2]
    AND     r3, r3, #0xF800
    CMP     r3, #0xF800
    BNE     next_stack

    SUB     r2, r1, #4
    LDRH    r3, [r2]
    AND     r3, r3, #0xF800
    CMP     r3, #0xF000
    BNE     next_stack

save_pc:
    ; stack.buffer[dp++] = pc
    LDR     r2, =stack
    ADD     r2, r2, #36             ; stack.buffer offset: 8 registers * 4 + 4 (depth) = 36
    LSL     r3, r11, #2             ; dp * 4
    ADD     r2, r2, r3
    STR     r1, [r2]
    ADD     r11, r11, #1            ; dp++

next_stack:
    ; sp += sizeof(size_t)
    ADD     r0, r0, #4
    B       stack_loop

fpu_context:
    ; spp += 18
    ADD     r0, r5, #72             ; 18 * 4
    B       stack_loop

stack_loop_end:
    ; Save depth
    LDR     r0, =stack
    STR     r11, [r0, #32]          ; stack.depth offset: 8 registers * 4 = 32

    POP     {r4-r11, pc}

; Simple implementation of strncpy for ARM assembly
; r0 = destination
; r1 = source
; r2 = max length
strncpy:
    PUSH    {r4-r6, lr}
    MOV     r4, r0                  ; dest
    MOV     r5, r1                  ; src
    MOV     r6, r2                  ; len

strncpy_loop:
    CMP     r6, #0
    BEQ     strncpy_null_terminate

    LDRB    r3, [r5]
    STRB    r3, [r4]

    CMP     r3, #0
    BEQ     strncpy_end

    ADD     r4, r4, #1
    ADD     r5, r5, #1
    SUB     r6, r6, #1
    B       strncpy_loop

strncpy_null_terminate:
    STRB    r3, [r4]

strncpy_end:
    MOV     r0, r4
    POP     {r4-r6, pc}

    END