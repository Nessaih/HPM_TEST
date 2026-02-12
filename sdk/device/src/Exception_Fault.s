
    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8
    PRESERVE8


    IMPORT backtrace
    EXPORT HardFault_Handler

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

Fault_Loop
    BL      Fault_Loop              ;while(1)

    END

