/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2023. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

/*!
 * @file Debugout_AC784xx.c
 *
 * @brief This file provides debug information output integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Debugout_AC784xx.h"
#include "AC784xx_Uart_Reg.h"
#include "Gpio_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

#include <stdio.h>
#include <stdarg.h>    //PRQA S 5130 # use standard header file.

/* ============================================  DEFINES AND MACROS  ============================================ */
/** @brief use atc log output implement, if not use,need define to 0 */
#ifndef ATC_DEBUG_OUT_INFO
#define ATC_DEBUG_OUT_INFO  1
#endif

/** @brief define uart max receive buffer size */
#define MAX_DEBUG_BUFF_SIZE      128U
/** @brief define debug uart for debug output  */
#define DEBUG_UART               UART0
/** @brief define debug uart interrupt vector  */
#define DEBUG_UART_IRQ           UART0_IRQn
/** @brief select debug uart clock  */
#define DEBUG_UART_CLK           CKGEN_UART0_CLK
/** @brief select debug uart bus clock  */
#define DEBUG_UART_BUS_CLK       CKGEN_UART0_BUS_CLK
/** @brief set debug uart smp */
#define DEBUG_UART_SMP           16.0F
/** @brief set debug uart baudrate  */
#define DEBUG_UART_BAUDRATE      115200.0F

/** @brief define select debug uart tx gpio */
#define DEBUG_UART_TX_PIN        PORTID_C, 3U
/** @brief define select debug uart rx gpio */
#define DEBUG_UART_RX_PIN        PORTID_C, 2U
/** @brief define select debug uart tx pinmux */
#define DEBUG_UART_TX_FUNC       PORT_MUX_ALT2
/** @brief define select debug uart rx pinmux */
#define DEBUG_UART_RX_FUNC       PORT_MUX_ALT2

/* ============================================= TYPEDEFS ================================================ */
/** @brief debug init flag,zero means not init, non zero means inited */
static uint8 s_debugInit = 0U;

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* ======================================  Functions define  ======================================== */
/*!
 * @brief uart output debug out is valid.
 * @note  Function ID: DES_DBG_API_003
 * @param[in] Ch: output character
 * @return  output character
 */
static uint8 Uart_Putc(uint8 Ch)
{
    if (s_debugInit != 0U)
    {
        while (Uart_Reg_GetStatusFlag(DEBUG_UART, UART_TX_DATA_NOT_FULL) == FALSE)
        {
            ;
        }
        Uart_Reg_PutChar(DEBUG_UART, Ch);
    }
    return Ch;
}

/*!
 * @brief Init debug out, and set the debug out is valid.
 * @note  Function ID: DES_DBG_API_000
 * @return void
 */
void Debug_Init(void)
{
    uint32 Freq = 0U;
#ifdef DEBUG_CMD_INTERRUPT
    Core_Hal_SetIrqPriority(DEBUG_UART_IRQ, 3U);
    Core_Hal_ClearPendingIrq(DEBUG_UART_IRQ);
    Core_Hal_EnableIrq(DEBUG_UART_IRQ);
#endif /* DEBUG_CMD_INTERRUPT */
    /* enable debug uart clock */
    (void)Ckgen_Hal_SetPeriphClkMux(DEBUG_UART_CLK, CKGEN_HSI_DIV2_CLK);
    (void)Ckgen_Hal_EnablePeriphClk(DEBUG_UART_BUS_CLK, TRUE);
    (void)Ckgen_Hal_GetFreq(CKGEN_HSI_DIV2_CLK, &Freq);
    /* PC8/PC9 debug uart */
    (void)Gpio_Hal_SetMuxMode(DEBUG_UART_TX_PIN, DEBUG_UART_TX_FUNC);
    (void)Gpio_Hal_SetMuxMode(DEBUG_UART_RX_PIN, DEBUG_UART_RX_FUNC);
    /* Set the baudrate */
    Uart_Reg_SetBaudRateDivisor(DEBUG_UART, (float)Freq / DEBUG_UART_SMP / DEBUG_UART_BAUDRATE); //PRQA S 5209 # use of basictype 'float' for debug.
    Uart_Reg_SetBitCountPerChar(DEBUG_UART, UART_8_BITS_PER_CHAR);
    Uart_Reg_SetStopBitCount(DEBUG_UART, UART_ONE_STOP_BIT);
    Uart_Reg_SetTransmitterCmd(DEBUG_UART, TRUE);
    Uart_Reg_SetReceiverCmd(DEBUG_UART, TRUE);
    Uart_Reg_SetFIFO(DEBUG_UART, STD_ACTIVE);
#ifdef DEBUG_CMD_INTERRUPT
    Uart_Reg_SetInterruptEn(DEBUG_UART, 9);
#endif /* DEBUG_CMD_INTERRUPT */
    s_debugInit = 1U;
}

/*!
 * @brief Set the debug out is invalid.
 * @note  Function ID: DES_DBG_API_001
 * @param[in] IsCloseDevice: whether close uart
 * @return void
 */
void Debug_DeInit(boolean IsCloseDevice)
{
    s_debugInit = 0U;
    if (TRUE == IsCloseDevice)
    {
        /* disable uart clock */
        Rcm_Hal_SetResetState(RCM_RESET_ID_UART0, RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(RCM_RESET_ID_UART0, RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_UART0_BUS_CLK, FALSE);
        /* Disable UART NVIC interrupt. */
        Core_Hal_DisableIrq(DEBUG_UART_IRQ);
        Core_Hal_ClearPendingIrq(DEBUG_UART_IRQ);
    }
}

/*!
 * @brief Init debug out, and set the debug out is valid.
 * @note  Function ID: DES_DBG_API_002
 * @param[in] format: need output character string
 * @return void
 */
void Debug_Printf(const char *format, ...) //PRQA S 1337 # variafble numer of parameters of print.
{
#if ATC_DEBUG_OUT_INFO
    va_list list;
    uint8 len, i;
    /* store send message */
    uint8 s_logMessage[MAX_DEBUG_BUFF_SIZE] = {0};
    if (format != NULL)
    {
        va_start(list, format); //PRQA S 5140 # use of identifier:va_* for debug.
        len = vsnprintf((char *)&s_logMessage[0], MAX_DEBUG_BUFF_SIZE, format, list); //PRQA S 4434 # convert signed type to unsigned type for debug .
        for (i = 0; i < len; i++)
        {
            (void)Uart_Putc(s_logMessage[i]);
        }
        va_end(list);
    }
#endif
}

/* =============================================  EOF  ============================================== */
