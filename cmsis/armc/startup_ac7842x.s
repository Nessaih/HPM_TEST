;/* Copyright Statement:
; *
; * This software/firmware and related documentation ("AutoChips Software") are
; * protected under relevant copyright laws. The information contained herein is
; * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
; * the prior written permission of AutoChips inc. and/or its licensors, any
; * reproduction, modification, use or disclosure of AutoChips Software, and
; * information contained herein, in whole or in part, shall be strictly
; * prohibited.
; *
; * AutoChips Inc. (C) 2024. All rights reserved.
; *
; * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
; * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
; * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
; * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
; * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
; * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
; * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
; * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
; * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
; * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
; * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
; * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
; * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
; * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
; * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
; * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
; * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
; * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
; * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
; */
 
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/

; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>


Stack_Size      EQU     0x00002000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp               ; Top of Stack
                DCD     Reset_Handler              ; Reset Handler
                DCD     NMI_Handler                ; NMI Handler
                DCD     HardFault_Handler          ; Hard Fault Handler
                DCD     MemManage_Handler          ; MPU Fault Handler
                DCD     BusFault_Handler           ; Bus Fault Handler
                DCD     UsageFault_Handler         ; Usage Fault Handler
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     SVC_Handler                ; SVCall Handler
                DCD     DebugMon_Handler           ; Debug Monitor Handler
                DCD     0                          ; Reserved
                DCD     PendSV_Handler             ; PendSV Handler
                DCD     SysTick_Handler            ; SysTick Handler

                ;External Interrupts
                DCD     DMA0_Channel0_IRQHandler            ;DMA channel 0 Interrupt
                DCD     DMA0_Channel1_IRQHandler            ;DMA channel 1 Interrupt
                DCD     DMA0_Channel2_IRQHandler            ;DMA channel 2 Interrupt
                DCD     DMA0_Channel3_IRQHandler            ;DMA channel 3 Interrupt
                DCD     DMA0_Channel4_IRQHandler            ;DMA channel 4 Interrupt
                DCD     DMA0_Channel5_IRQHandler            ;DMA channel 5 Interrupt
                DCD     DMA0_Channel6_IRQHandler            ;DMA channel 6 Interrupt
                DCD     DMA0_Channel7_IRQHandler            ;DMA channel 7 Interrupt
                DCD     DMA0_Channel8_IRQHandler            ;DMA channel 8 Interrupt
                DCD     DMA0_Channel9_IRQHandler            ;DMA channel 9 Interrupt
                DCD     DMA0_Channel10_IRQHandler           ;DMA channel 10 Interrupt
                DCD     DMA0_Channel11_IRQHandler           ;DMA channel 11 Interrupt
                DCD     DMA0_Channel12_IRQHandler           ;DMA channel 12 Interrupt
                DCD     DMA0_Channel13_IRQHandler           ;DMA channel 13 Interrupt
                DCD     DMA0_Channel14_IRQHandler           ;DMA channel 14 Interrupt
                DCD     DMA0_Channel15_IRQHandler           ;DMA channel 15 Interrupt
                DCD     PORTA_IRQHandler                    ;Port A pin detect interrupt
                DCD     PORTB_IRQHandler                    ;Port B pin detect interrupt
                DCD     PORTC_IRQHandler                    ;Port C pin detect interrupt
                DCD     PORTD_IRQHandler                    ;Port D pin detect interrupt
                DCD     PORTE_IRQHandler                    ;Port E pin detect interrupt
                DCD     UART0_IRQHandler                    ;UART0 Transmit / Receive Interrupt
                DCD     UART1_IRQHandler                    ;UART1 Transmit / Receive Interrupt
                DCD     UART2_IRQHandler                    ;UART2 Transmit / Receive Interrupt
                DCD     UART3_IRQHandler                    ;UART3 Transmit / Receive Interrupt
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     SPI0_IRQHandler                     ;SPI0 Interrupt
                DCD     SPI1_IRQHandler                     ;SPI1 Interrupt
                DCD     SPI2_IRQHandler                     ;SPI2 Interrupt
                DCD     SPI3_IRQHandler                     ;SPI3 Interrupt
                DCD     I2C0_IRQHandler                     ;I2C0 Interrupt
                DCD     I2C1_IRQHandler                     ;I2C1 Interrupt
                DCD     EIO_IRQHandler                      ;EIO Interrupt
                DCD     CAN0_IRQHandler                     ;CAN0 Interrupt
                DCD     CAN0_Wakeup_IRQHandler              ;CAN0 Wakeup Interrupt
                DCD     CAN1_IRQHandler                     ;CAN1 Interrupt
                DCD     CAN1_Wakeup_IRQHandler              ;CAN1 Wakeup Interrupt
                DCD     CAN2_IRQHandler                     ;CAN2 Interrupt
                DCD     CAN2_Wakeup_IRQHandler              ;CAN2 Wakeup Interrupt
                DCD     CAN3_IRQHandler                     ;CAN3 Interrupt
                DCD     CAN3_Wakeup_IRQHandler              ;CAN3 Wakeup Interrupt
                DCD     CAN4_IRQHandler                     ;CAN4 Interrupt
                DCD     CAN4_Wakeup_IRQHandler              ;CAN4 Wakeup Interrupt
                DCD     CAN5_IRQHandler                     ;CAN5 Interrupt
                DCD     CAN5_Wakeup_IRQHandler              ;CAN5 Wakeup Interrupt
                DCD     PDT0_IRQHandler                     ;PDT0 Interrupt
                DCD     PDT1_IRQHandler                     ;PDT1 Interrupt
                DCD     ADC0_IRQHandler                     ;ADC0 Interrupt
                DCD     ADC1_IRQHandler                     ;ADC1 Interrupt
                DCD     ACMP0_IRQHandler                    ;ACMP0 Interrupt
                DCD     WDG_IRQHandler                      ;WDG Interrupt
                DCD     EWDG_IRQHandler                     ;EWDG Interrupt
                DCD     MCM_IRQHandler                      ;MCM Interrupt
                DCD     LVD_IRQHandler                      ;LVD Interrupt
                DCD     SPM_IRQHandler                      ;SPM Interrupt
                DCD     RCM_IRQHandler                      ;RCM Interrupt
                DCD     PWM0_Overflow_IRQHandler            ;PWM0 Overflow Interrupt
                DCD     PWM0_Channel_IRQHandler             ;PWM0 Channel Interrupt
                DCD     PWM0_Fault_IRQHandler               ;PWM0 Fault Interrupt
                DCD     PWM1_Overflow_IRQHandler            ;PWM1 Overflow Interrupt
                DCD     PWM1_Channel_IRQHandler             ;PWM1 Channel Interrupt
                DCD     PWM1_Fault_IRQHandler               ;PWM1 Fault Interrupt
                DCD     PWM2_Overflow_IRQHandler            ;PWM2 Overflow Interrupt
                DCD     PWM2_Channel_IRQHandler             ;PWM2 Channel Interrupt
                DCD     PWM2_Fault_IRQHandler               ;PWM2 Fault Interrupt
                DCD     PWM3_Overflow_IRQHandler            ;PWM3 Overflow Interrupt
                DCD     PWM3_Channel_IRQHandler             ;PWM3 Channel Interrupt
                DCD     PWM3_Fault_IRQHandler               ;PWM3 Fault Interrupt
                DCD     PWM4_Overflow_IRQHandler            ;PWM4 Overflow Interrupt
                DCD     PWM4_Channel_IRQHandler             ;PWM4 Channel Interrupt
                DCD     PWM4_Fault_IRQHandler               ;PWM4 Fault Interrupt
                DCD     PWM5_Overflow_IRQHandler            ;PWM5 Overflow Interrupt
                DCD     PWM5_Channel_IRQHandler             ;PWM5 Channel Interrupt
                DCD     PWM5_Fault_IRQHandler               ;PWM5 Fault Interrupt
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     RTC_IRQHandler                      ;RTC Interrupt
                DCD     PCT_IRQHandler                      ;PCT Interrupt
                DCD     TIMER_Channel0_IRQHandler           ;TIMER Channel0 Interrupt
                DCD     TIMER_Channel1_IRQHandler           ;TIMER Channel1 Interrupt
                DCD     TIMER_Channel2_IRQHandler           ;TIMER Channel2 Interrupt
                DCD     TIMER_Channel3_IRQHandler           ;TIMER Channel3 Interrupt
                DCD     CSE_IRQHandler                      ;CSE Interrupt
                DCD     FLASH_ECC_IRQHandler                ;Flash ECC Interrupt
                DCD     FLASH_IRQHandler                    ;Flash Interrupt
                DCD     FLASH_Collision_IRQHandler          ;Flash Collision Interrupt
                DCD     ECC_1BIT_IRQHandler                 ;ECC 1BIT Interrupt
                DCD     ECC_2BIT_IRQHandler                 ;ECC 2BIT Interrupt
                DCD     SENT_Channel0_IRQHandler            ;SENT Channel 0 Interrupt
                DCD     SENT_Channel1_IRQHandler            ;SENT Channel 1 Interrupt
                DCD     SMU_IRQHandler                      ;SMU Interrupt
                DCD     STB_WU_IRQHandler                   ;Standby Wakeup Interrupt

__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; Reset handler
Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                MRS     R10, PRIMASK    ; Save PRIMASK
                CPSID   I               ; Mask interrupts

                IMPORT  SystemInitRam
                IMPORT  SystemInit
                IMPORT  __main

                ; Init Main stack
                LDR     R1, =__initial_sp
                LDR     R0, =Stack_Size
                SUB     R0, R1, R0
                MOV     R2, #0
                MOV     R3, R2
STACK_INIT_LOOP
                STMIA   R0!, {R2, R3}
                CMP     R0, R1
                BLT     STACK_INIT_LOOP

                ; Init ECC SRAM
                LDR     R0, =Stack_Size
                SUB     R0, R1, R0
                LDR     R3, =SystemInitRam
                BLX     R3

                LDR     R0, =SystemInit
                BLX     R0

                MSR     PRIMASK, R10    ; Restore PRIMASK
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                       [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler                 [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler                 [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler                  [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler                [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                       [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler                  [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler                    [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler                   [WEAK]
                B       .
                ENDP
Default_Handler PROC
                EXPORT  DMA0_Channel0_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel1_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel2_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel3_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel4_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel5_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel6_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel7_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel8_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel9_IRQHandler          [WEAK]
                EXPORT  DMA0_Channel10_IRQHandler         [WEAK]
                EXPORT  DMA0_Channel11_IRQHandler         [WEAK]
                EXPORT  DMA0_Channel12_IRQHandler         [WEAK]
                EXPORT  DMA0_Channel13_IRQHandler         [WEAK]
                EXPORT  DMA0_Channel14_IRQHandler         [WEAK]
                EXPORT  DMA0_Channel15_IRQHandler         [WEAK]
                EXPORT  PORTA_IRQHandler                  [WEAK]
                EXPORT  PORTB_IRQHandler                  [WEAK]
                EXPORT  PORTC_IRQHandler                  [WEAK]
                EXPORT  PORTD_IRQHandler                  [WEAK]
                EXPORT  PORTE_IRQHandler                  [WEAK]
                EXPORT  UART0_IRQHandler                  [WEAK]
                EXPORT  UART1_IRQHandler                  [WEAK]
                EXPORT  UART2_IRQHandler                  [WEAK]
                EXPORT  UART3_IRQHandler                  [WEAK]
                EXPORT  SPI0_IRQHandler                   [WEAK]
                EXPORT  SPI1_IRQHandler                   [WEAK]
                EXPORT  SPI2_IRQHandler                   [WEAK]
                EXPORT  SPI3_IRQHandler                   [WEAK]
                EXPORT  I2C0_IRQHandler                   [WEAK]
                EXPORT  I2C1_IRQHandler                   [WEAK]
                EXPORT  EIO_IRQHandler                    [WEAK]
                EXPORT  CAN0_IRQHandler                   [WEAK]
                EXPORT  CAN0_Wakeup_IRQHandler            [WEAK]
                EXPORT  CAN1_IRQHandler                   [WEAK]
                EXPORT  CAN1_Wakeup_IRQHandler            [WEAK]
                EXPORT  CAN2_IRQHandler                   [WEAK]
                EXPORT  CAN2_Wakeup_IRQHandler            [WEAK]
                EXPORT  CAN3_IRQHandler                   [WEAK]
                EXPORT  CAN3_Wakeup_IRQHandler            [WEAK]
                EXPORT  CAN4_IRQHandler                   [WEAK]
                EXPORT  CAN4_Wakeup_IRQHandler            [WEAK]
                EXPORT  CAN5_IRQHandler                   [WEAK]
                EXPORT  CAN5_Wakeup_IRQHandler            [WEAK]
                EXPORT  PDT0_IRQHandler                   [WEAK]
                EXPORT  PDT1_IRQHandler                   [WEAK]
                EXPORT  ADC0_IRQHandler                   [WEAK]
                EXPORT  ADC1_IRQHandler                   [WEAK]
                EXPORT  ACMP0_IRQHandler                  [WEAK]
                EXPORT  WDG_IRQHandler                    [WEAK]
                EXPORT  EWDG_IRQHandler                   [WEAK]
                EXPORT  MCM_IRQHandler                    [WEAK]
                EXPORT  LVD_IRQHandler                    [WEAK]
                EXPORT  SPM_IRQHandler                    [WEAK]
                EXPORT  RCM_IRQHandler                    [WEAK]
                EXPORT  PWM0_Overflow_IRQHandler          [WEAK]
                EXPORT  PWM0_Channel_IRQHandler           [WEAK]
                EXPORT  PWM0_Fault_IRQHandler             [WEAK]
                EXPORT  PWM1_Overflow_IRQHandler          [WEAK]
                EXPORT  PWM1_Channel_IRQHandler           [WEAK]
                EXPORT  PWM1_Fault_IRQHandler             [WEAK]
                EXPORT  PWM2_Overflow_IRQHandler          [WEAK]
                EXPORT  PWM2_Channel_IRQHandler           [WEAK]
                EXPORT  PWM2_Fault_IRQHandler             [WEAK]
                EXPORT  PWM3_Overflow_IRQHandler          [WEAK]
                EXPORT  PWM3_Channel_IRQHandler           [WEAK]
                EXPORT  PWM3_Fault_IRQHandler             [WEAK]
                EXPORT  PWM4_Overflow_IRQHandler          [WEAK]
                EXPORT  PWM4_Channel_IRQHandler           [WEAK]
                EXPORT  PWM4_Fault_IRQHandler             [WEAK]
                EXPORT  PWM5_Overflow_IRQHandler          [WEAK]
                EXPORT  PWM5_Channel_IRQHandler           [WEAK]
                EXPORT  PWM5_Fault_IRQHandler             [WEAK]
                EXPORT  RTC_IRQHandler                    [WEAK]
                EXPORT  PCT_IRQHandler                    [WEAK]
                EXPORT  TIMER_Channel0_IRQHandler         [WEAK]
                EXPORT  TIMER_Channel1_IRQHandler         [WEAK]
                EXPORT  TIMER_Channel2_IRQHandler         [WEAK]
                EXPORT  TIMER_Channel3_IRQHandler         [WEAK]
                EXPORT  CSE_IRQHandler                    [WEAK]
                EXPORT  FLASH_ECC_IRQHandler              [WEAK]
                EXPORT  FLASH_IRQHandler                  [WEAK]
                EXPORT  FLASH_Collision_IRQHandler        [WEAK]
                EXPORT  ECC_1BIT_IRQHandler               [WEAK]
                EXPORT  ECC_2BIT_IRQHandler               [WEAK]
                EXPORT  SENT_Channel0_IRQHandler          [WEAK]
                EXPORT  SENT_Channel1_IRQHandler          [WEAK]
                EXPORT  SMU_IRQHandler                    [WEAK]
                EXPORT  STB_WU_IRQHandler                 [WEAK]
DMA0_Channel0_IRQHandler
DMA0_Channel1_IRQHandler
DMA0_Channel2_IRQHandler
DMA0_Channel3_IRQHandler
DMA0_Channel4_IRQHandler
DMA0_Channel5_IRQHandler
DMA0_Channel6_IRQHandler
DMA0_Channel7_IRQHandler
DMA0_Channel8_IRQHandler
DMA0_Channel9_IRQHandler
DMA0_Channel10_IRQHandler
DMA0_Channel11_IRQHandler
DMA0_Channel12_IRQHandler
DMA0_Channel13_IRQHandler
DMA0_Channel14_IRQHandler
DMA0_Channel15_IRQHandler
PORTA_IRQHandler
PORTB_IRQHandler
PORTC_IRQHandler
PORTD_IRQHandler
PORTE_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
UART3_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
SPI3_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
EIO_IRQHandler
CAN0_IRQHandler
CAN0_Wakeup_IRQHandler
CAN1_IRQHandler
CAN1_Wakeup_IRQHandler
CAN2_IRQHandler
CAN2_Wakeup_IRQHandler
CAN3_IRQHandler
CAN3_Wakeup_IRQHandler
CAN4_IRQHandler
CAN4_Wakeup_IRQHandler
CAN5_IRQHandler
CAN5_Wakeup_IRQHandler
PDT0_IRQHandler
PDT1_IRQHandler
ADC0_IRQHandler
ADC1_IRQHandler
ACMP0_IRQHandler
WDG_IRQHandler
EWDG_IRQHandler
MCM_IRQHandler
LVD_IRQHandler
SPM_IRQHandler
RCM_IRQHandler
PWM0_Overflow_IRQHandler
PWM0_Channel_IRQHandler
PWM0_Fault_IRQHandler
PWM1_Overflow_IRQHandler
PWM1_Channel_IRQHandler
PWM1_Fault_IRQHandler
PWM2_Overflow_IRQHandler
PWM2_Channel_IRQHandler
PWM2_Fault_IRQHandler
PWM3_Overflow_IRQHandler
PWM3_Channel_IRQHandler
PWM3_Fault_IRQHandler
PWM4_Overflow_IRQHandler
PWM4_Channel_IRQHandler
PWM4_Fault_IRQHandler
PWM5_Overflow_IRQHandler
PWM5_Channel_IRQHandler
PWM5_Fault_IRQHandler
RTC_IRQHandler
PCT_IRQHandler
TIMER_Channel0_IRQHandler
TIMER_Channel1_IRQHandler
TIMER_Channel2_IRQHandler
TIMER_Channel3_IRQHandler
CSE_IRQHandler
FLASH_ECC_IRQHandler
FLASH_IRQHandler
FLASH_Collision_IRQHandler
ECC_1BIT_IRQHandler
ECC_2BIT_IRQHandler
SENT_Channel0_IRQHandler
SENT_Channel1_IRQHandler
SMU_IRQHandler
STB_WU_IRQHandler

                B       .

                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                 IF      :DEF:__MICROLIB

                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit

                 ELSE

                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap

__user_initial_stackheap

                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF

                 END

;****************END OF FILE******************************************
