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

/**
 * @file System_AC784xx.c
 *
 * @brief This file provides system clock config integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"
#include "Ckgen_Hal.h"
#include "OsIf_Critical.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
/** @brief Sram ECC read enable. */
#define SRAM_ECC_READ_ENABLE     (1U)

/** @brief Sram ECC 2bit error reset enable. */
#define SRAM_ECC_ERR_RST_ENABLE  (0U)

#ifndef WDG_DISABLE
/** @brief Watchdog disable.one means disable, zero means enable */
#define WDG_DISABLE              (1U)
#endif

/** @brief ICache disable */
#define ICACHE_DISABLE            (0U)

/** @brief ICache line 16byte (default is 32byte) */
#define ICACHE_LINE_16BYTE        (1U)

/** @brief DCache disable */
#define DCACHE_DISABLE            (1U)

/** @brief Don't initialize sram after system reset. */
#define NOT_INIT_SRAM_AFTER_RESET (1U)

/** @brief Default system clock. */
#define DEFAULT_SYSTEM_CLOCK     (48000000U)

/** @brief Standby retention base address. */
#define SRAM_RETENTION_BASE      (SRAM_U_BASE - SRAM_RETENTION_SIZE)

/* ============================================= TYPEDEFS ================================================ */
/**
 * @brief The type used to copy from source to destination.
 */
typedef struct
{
    uint8 *DstStart; /**< Start address of destination */
    uint8 *SrcStart; /**< Start address of source */
    uint8 *SrcEnd; /**< End address of source */
} Sys_CopyType;

/**
 * @brief The type used to zero initialization.
 */
typedef struct
{
    uint8 *DstStart; /**< Start address of destination */
    uint8 *DstEnd; /**< End address of destination */
} Sys_ZeroType;

/* =========================================== LOCAL VARIABLES ============================================== */
/** @brief Externals declaration. */
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
extern uint32 __Vectors;
#endif

static FlashDeviceType g_dFlashDevice = NON_DEV;
/** @brief Core clock. */
uint32 SystemCoreClock = DEFAULT_SYSTEM_CLOCK;

#if defined (COMPILER_GCC) || defined (COMPILER_GHS)
/*PRQA S 3684 ++ # allows definition of unsized arrays.*/
extern uint32 __DATA_ROM[];
extern uint32 __DATA_RAM[];
extern uint32 __DATA_END[];

extern uint32 __CODE_RAM[];
extern uint32 __CODE_ROM[];
extern uint32 __CODE_END[];

extern uint32 __BSS_START[];
extern uint32 __BSS_END[];
/*PRQA S 3684 -- # allows definition of unsized arrays.*/

/** @brief Copy to RAM table. */
/*PRQA S 3218 ++ # Variables inside functions that occupy a large of stack space are allowed to be defined as static global variables.*/
static const Sys_CopyType SystemCopyTable[] =
{
    {(uint8 *)__DATA_RAM, (uint8 *)__DATA_ROM, (uint8 *)__DATA_END},
    {(uint8 *)__CODE_RAM, (uint8 *)__CODE_ROM, (uint8 *)__CODE_END},
#ifdef VECT_TAB_SRAM
    {
        (uint8 *)VECTOR_TABLE_SRAM_ADDR,
        (uint8 *)(&__Vectors),
        (uint8 *)((uint32)(&__Vectors) + ((MAX_IRQn + 16U) << 2U))
    },
#endif
};

/** @brief Zero RAM table. */
static const Sys_ZeroType SystemZeroTable[] =
{
    {(uint8 *)__BSS_START, (uint8 *)__BSS_END},
};
#else

/** @brief Copy to RAM table. */
static const Sys_CopyType SystemCopyTable[] =
{
#ifdef VECT_TAB_SRAM
    {
        (uint8 *)VECTOR_TABLE_SRAM_ADDR,
        (uint8 *)(&__Vectors),
        (uint8 *)((uint32)(&__Vectors) + ((MAX_IRQn + 16U) << 2U))
    },
#else
    {(uint8 *)0U, (uint8 *)0U, (uint8 *)0U},
#endif
};

/** @brief Zero RAM table. */
static const Sys_ZeroType SystemZeroTable[] =
{
    {(uint8 *)0U, (uint8 *)0U},
};
#endif
/*PRQA S 3218 -- # Variables inside functions that occupy a large of stack space are allowed to be defined as static global variables.*/

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* ======================================  Functions define  ======================================== */
#ifdef ATC_DEVICE_ASSERT
extern void Debug_Printf(const char *format, ...);

/** @brief for circular to waiting debug.
 * @param[in] File: assert file name.
 * @param[in] Line: assert line in file.
 * @return void
 */
__attribute__((weak)) void WaitingForDebug(const char *File, uint32 Line)
{
    Debug_Printf("[ASSERT]Detect error on %s %ld.\r\n", File, Line);
    for (;;) {}; //PRQA S 2880 # code for debugging.
}
#endif

/**
 * @brief Update system core clock.
 * @note  Function ID: DES_BOOT_API_001
 * @return void
 */
void SystemCoreClockUpdate(void)
{
    (void)Ckgen_Hal_GetFreq(CKGEN_CORE_CLK, &SystemCoreClock);
}

/**
 * @brief Setup the microcontroller system. Initialize the System.
 * @note  Function ID: DES_BOOT_API_000
 * @return void
 */
void SystemInit(void)
{
    /* Disable watchdog */
#if (WDG_DISABLE)
    WDG->CNT = WDG_UNLOCK_FIRST_VALUE;
    WDG->CNT = WDG_UNLOCK_SECOND_VALUE;
    WDG->CS0 &= ~WDG_CS0_EN_Msk;
#endif
#if defined (AC7840X) || defined (AC7842X)
    /* Sram L&U ECC read enable */
#if (SRAM_ECC_READ_ENABLE)
    MCM->MLMDR0 |= MCM_MLMDR0_LREEN_Msk | MCM_MLMDR0_UREEN_Msk;
    /* Sram ECC 2bit error reset disable */
#if (SRAM_ECC_ERR_RST_ENABLE)
    /*bit16 be setted in sraminit ,so can not clear it*/
    CKGEN->RCM_EN |= 0x000007EFU;
#else
    CKGEN->RCM_EN |= 0x000003EFU;
#endif
    CKGEN->RCM_CTRL = 0x83000005U;
#endif  /*SRAM_ECC_READ_ENABLE*/
#elif defined (AC7843X)
#if (ICACHE_DISABLE || ICACHE_LINE_16BYTE || DCACHE_DISABLE)
    uint32 en = CKGEN->RCM_EN;
    CKGEN->RCM_EN = en & 0xFFFFFC10U;
#endif
    /* Disable iCache */
#if (ICACHE_DISABLE || ICACHE_LINE_16BYTE)
    MCM->ICACHE_CFG &= ~MCM_ICACHE_CFG_EN_Msk;
    while ((MCM->ICACHE_CFG & MCM_ICACHE_CFG_CLRDONE_Msk) == 0U)
    {
    }
#endif
    /* Set iCache line to 16byte */
#if (!ICACHE_DISABLE && ICACHE_LINE_16BYTE)
    MCM->CPUCACHE_PROT = 0x78430001U;
    MCM->ICACHE_CFG = 0x28100010U;
    MCM->ICACHE_CFG |= MCM_ICACHE_CFG_EN_Msk;
#endif
    /* Disable dCache */
#if DCACHE_DISABLE
    MCM->DCACHE_CFG &= ~MCM_DCACHE_CFG_EN_Msk;
#endif
#if (ICACHE_DISABLE || ICACHE_LINE_16BYTE || DCACHE_DISABLE)
    CKGEN->RCM_EN = en | CKGEN_RCM_EN_EXT_RST_EN_Msk;
#else
    CKGEN->RCM_EN |= CKGEN_RCM_EN_EXT_RST_EN_Msk;
#endif
#endif   /*AC7840X or AC7842X or AC7843X*/
    /* Set CP10 and CP11 Full Access */
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    SCB->CPACR |= 0xF00000U;
#endif
    /* Relocate vector table */
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
#ifdef VECT_TAB_SRAM
    SCB->VTOR = VECTOR_TABLE_SRAM_ADDR;
#else
    SCB->VTOR = (uint32)&__Vectors;
#endif  /*VECT_TAB_SRAM*/
#endif  /*__VTOR_PRESENT */
#if defined (AC7843X)
    /* Check SPLL status */
    if ((SPM->PWR_MGR_CFG1 & (SPM_PWR_MGR_CFG1_SPLL_RDY_Msk | SPM_PWR_MGR_CFG1_SPLL_EN_Msk)) \
            == SPM_PWR_MGR_CFG1_SPLL_RDY_Msk)
    {
        SPM->PWR_MGR_CFG1 |= SPM_PWR_MGR_CFG1_SPLL_EN_Msk;
        SPM->PWR_MGR_CFG1 &= ~SPM_PWR_MGR_CFG1_SPLL_EN_Msk;
    }
#endif  /*AC7843X*/
}

/*!
 * @brief Initialize 8 byte aligned Mem from [Start] to [End] with Zero.
 * @note  Function ID: DES_BOOT_API_003
 * @param[in] Start: Mem start address.
 * @param[in] End: Mem end address.
 * @return void
 */
static void SystemMemZero_Aligned8(uint64 *Start, const uint64 *End)
{
    volatile uint64 *d = (volatile uint64 *)Start; /* Avoid optimized into build-in function. */
#if defined (COMPILER_GCC) || defined (COMPILER_GHS)
    /*enable fpu for gcc compilrer optimized*/
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    SCB->CPACR |= 0xF00000U;
#endif
#endif
    while (d < End)
    {
        *d = 0x0UL;
        d++;
    }
#if defined (COMPILER_GCC) || defined (COMPILER_GHS)
    /*restore fpu old status*/
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    SCB->CPACR &= ~0xF00000U;
#endif
#endif
}

/*!
 * @brief Initialize Mem from [Start] to [End] with Zero.
 * @note  Function ID: DES_BOOT_API_004
 * @param[in] Start: Mem start address.
 * @param[in] End: Mem end address.
 * @return void
 */
static void SystemMemZero(uint8 *Start, const uint8 *End)
{
    uint8 *d = Start;
    uint8 *rem = (uint8 *)((uint32)End - ((uint32)End % 8U));
    /* Zero first unaligned bytes. */
    while ((((uint32)d % 8U) != 0U) && (d < End))
    {
        *d = 0x00U;
        d++;
    }
    /* Zero align bytes. */
    /*PRQA S 3305,0310 ++ # d&m need aligned with 8 bytes*/
    SystemMemZero_Aligned8((uint64 *)d, (uint64 *)rem);
    /*PRQA S 3305,0310 -- # d&m need aligned with 8 bytes*/
    /* Zero remaining unaligned bytes. */
    d = rem;
    while (d < End)
    {
        *d = 0x00U;
        d++;
    }
}

/*!
 * @brief Copy Mem to [DstStart] from [SrcStart] to [SrcEnd].
 * @note  Function ID: DES_BOOT_API_005
 * @param[in] DstStart: Destination mem start address.
 * @param[in] SrcStart: Source mem end address.
 * @param[in] SrcEnd: Source mem value.
 * @return void
 */
static void SystemMemCpy(uint8 *DstStart, const uint8 *SrcStart, const uint8 *SrcEnd)
{
    const uint8 *s = SrcStart;
    uint8 *d = DstStart;
    while (s < SrcEnd)
    {
        *d = *s;
        d++;
        s++;
    }
}

/*PRQA S 2995 ++ # The Stack Base and End is fix in one enviroment */
/*!
 * @brief Initialize the SRAM to 0 for ECC.
 * @note  Function ID: DES_BOOT_API_006
 * @param[in] StackBase: Main stack start address.
 * @param[in] StackEnd:  Main stack end address.
 * @return void
 */
static void SystemInitAllRam(uint32 StackBase, uint32 StackEnd)
{
#if defined (AC7840X) || defined (AC7842X)
    /* Not init RAM when system reset by check boot reason is POR| LVR | standby wakeup,
     * or CKGEN_RCM_EN_NOT_INIT_SRAM_Msk.
     */
#if (NOT_INIT_SRAM_AFTER_RESET)
#if defined (AC7840X)
    if ((0U != (CKGEN->RCM_STATUS & (CKGEN_RCM_STATUS_POR_RST_FLAG_Msk | CKGEN_RCM_STATUS_LVR_RST_FLAG_Msk)))
            || (0U != (SPM->STB_WP_STATUS & 0x1FFFFU)))
#elif defined (AC7842X)
    if ((0U == (CKGEN->RCM_EN & CKGEN_RCM_EN_NOT_INIT_SRAM_Msk))
            || (0U != (SPM->STB_WP_STATUS & 0x1FFFFU)))
#endif
#endif /* NOT_INIT_SRAM_AFTER_RESET */
    {
        /* Initialize SRAM. */
        if (0U == (SPM->STB_WP_STATUS & 0x1FFFFU)) /* POR. */
        {
            SystemMemZero_Aligned8((uint64 *)SRAM_L_BASE, (uint64 *)StackBase);
            SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)SRAM_U_END);
        }
        else /* Standby wakeup skip the retention SRAM. */
        {
            if (StackEnd <= SRAM_RETENTION_BASE)
            {
                /* Stack locate in SRAM_L but not in retention. */
                SystemMemZero_Aligned8((uint64 *)SRAM_L_BASE, (uint64 *)StackBase);
                SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)SRAM_U_END);
            }
            else if (StackBase >= SRAM_U_BASE)
            {
                /* Stack locate in SRAM_U. */
                SystemMemZero_Aligned8((uint64 *)SRAM_L_BASE, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)StackBase);
                SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)SRAM_U_END);
            }
            else if ((StackBase < SRAM_RETENTION_BASE) && (StackEnd > SRAM_RETENTION_BASE))
            {
                /* Stack location in SRAM_L and retention. */
                SystemMemZero_Aligned8((uint64 *)SRAM_L_BASE, (uint64 *)StackBase);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)SRAM_U_END);
            }
            else if ((StackBase < SRAM_U_BASE) && (StackEnd > SRAM_U_BASE))
            {
                /* Stack location in SRAM_U and retention. */
                SystemMemZero_Aligned8((uint64 *)SRAM_L_BASE, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)SRAM_U_END);
            }
            else
            {
                /* Stack location in retention fullly. */
                SystemMemZero_Aligned8((uint64 *)SRAM_L_BASE, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)SRAM_U_END);
            }
        }
        /* Initialize FlexRAM if not used by CSE. */
        if (READ_BIT32(FLASH->PART, FLASH_PART_EPART_Msk) == FLASH_PART_EPART_Msk)
        {
            SystemMemZero_Aligned8((uint64 *)FLEXRAM_BASE, (uint64 *)(FLEXRAM_BASE + FLEX_RAM_SIZE));
        }
    }
#else
    /* BootROM initialize RAM AC7843 */
    (void)StackBase;
    (void)StackEnd;
#endif
#if NOT_INIT_SRAM_AFTER_RESET
#if defined (AC7842X) || defined (AC7843X)
    /* Write the no init sram flag */
    CKGEN->RCM_EN |= CKGEN_RCM_EN_NOT_INIT_SRAM_Msk;
#endif
#endif
}
/*PRQA S 2995 -- # The Stack Base and End is fix in one enviroment */

/*!
 * @brief Initialize the SRAM.
 * @note  Function ID: DES_BOOT_API_002
 * @param[in] StackBase: Main stack start address.
 * @param[in] StackEnd:  Main stack end address.
 * @return void
 */
void SystemInitRam(uint32 StackBase, uint32 StackEnd) //PRQA S 1503 # it is used by startup.
{
    uint8 i;

    /* Initialize the SRAM to 0 for ECC. */
    SystemInitAllRam(StackBase, StackEnd);
    /* Copy initialized data from ROM to RAM. */
    for (i = 0; i < (sizeof(SystemCopyTable) / sizeof(SystemCopyTable[0])); i++)
    {
        SystemMemCpy(SystemCopyTable[i].DstStart, SystemCopyTable[i].SrcStart, SystemCopyTable[i].SrcEnd);
    }
    /*PRQA S 2877 ++ # allow executed not more than once*/
    /* Initialize bss to zero. */
    for (i = 0; i < (sizeof(SystemZeroTable) / sizeof(SystemZeroTable[0])); i++)
    {
        SystemMemZero(SystemZeroTable[i].DstStart, SystemZeroTable[i].DstEnd);
    }
    /*PRQA S 2877 -- # allow executed not more than once*/
}

/*!
 * @brief DFlash Lock.
 * @note  Function ID: DES_BOOT_API_010
 * @param[in] Device_Type: Device for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceTryLock(FlashDeviceType Device_Type)
{
    FlashDeviceType Ret = NON_DEV;

    OSIF_ENTER_CRITICAL(FLS_HAL_ID2);
    if (g_dFlashDevice == NON_DEV)
    {
        g_dFlashDevice = Device_Type;
        Ret = g_dFlashDevice;
    }
    OSIF_EXIT_CRITICAL(FLS_HAL_ID2);

    return Ret;
}

/*!
 * @brief DFlash UnLock.
 * @note  Function ID: DES_BOOT_API_011
 * @param[in] Device_Type: Device for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceUnlock(FlashDeviceType Device_Type)
{
    FlashDeviceType Ret = NON_DEV;

    OSIF_ENTER_CRITICAL(FLS_HAL_ID2);
    if (g_dFlashDevice != Device_Type)
    {
        Ret = g_dFlashDevice;
    }
    g_dFlashDevice = NON_DEV;
    OSIF_EXIT_CRITICAL(FLS_HAL_ID2);

    return Ret;
}

/*!
 * @brief DFlash Free Lock.
 * @note  Function ID: DES_BOOT_API_012
 * @param[in] Device_Type: Device for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceFreelock(FlashDeviceType Device_Type)
{
    FlashDeviceType Ret = NON_DEV;

    OSIF_ENTER_CRITICAL(FLS_HAL_ID2);
    if (g_dFlashDevice == Device_Type)
    {
        g_dFlashDevice = NON_DEV;
    }
    Ret = g_dFlashDevice;
    OSIF_EXIT_CRITICAL(FLS_HAL_ID2);

    return Ret;
}

/**
 * @brief copy src addr to dest addr
 * @note  Function ID : DES_BOOT_API_013
 * @param[in] Dest: dest  addr
 * @param[in] Src: src addr
 * @param[in] N: copy len
 * @return dest address
 */
void *System_Memcpy(void *Dest, const void *Src, uint32 N)
{
    uint8 *D = (uint8 *)Dest;
    const uint8 *S = (const uint8 *)Src;
    void *Ret = Dest;

    if ((NULL_PTR == Dest) || (NULL_PTR == Src))
    {
        Ret = NULL_PTR;
    }
    else
    {
        /* deal address no align case */
        while ((((uint32)S % sizeof(uint32)) != 0U) && (N > 0U))
        {
            *D++ = *S++;
            N--;
        }

        /*copy order by 4 bytes*/
        uint32 *Wd = (uint32*)D;
        const uint32 *Ws = (const uint32 *)S;
        while (N >= 4U)
        {
            *Wd++ = *Ws++;
            N -=4;
        }

        /*deal with no align data */
        D = (uint8 *)Wd;
        S = (const uint8 *)Ws;
        while (N--) {
            *D++ = *S++;
        }

        /*Data memory barrier ensure access with order*/
        __DMB();
    }

    return Ret;
}
/**
 * @brief set a memory to value
 * @note  Function ID : DES_BOOT_API_014
 * @param[in] Addr: dest memory addr
 * @param[in] Val: set val
 * @param[in] N: len
 * @return dest address
 */
void *System_Memset(void *Addr, uint8 Val, uint32 N)
{
    uint8 *D = (uint8 *)Addr;

    if (Addr != NULL_PTR)
    {
        /*  deal address no align case  */
        while ((((uint32)D % sizeof(uint32)) != 0U) && (N > 0U))
        {
            *D++ = Val;
            N--;
        }

        /*set order by 4 bytes*/
        uint32 Fill_Val = ((Val << 24U) | (Val << 16) | (Val << 8) | Val);
        uint32 *Wd = (uint32 *)D;
        while (N >= 4U) {
            *Wd++ = Fill_Val;
            N -= 4;
        }

        /*deal with no align data*/
        D = (uint8 *)Wd;
        while(N--)
        {
            *D++ = Val;
        }
        /*Data memory barrier ensure access with order*/
        __DMB();
    }
    return Addr;
}

/* =============================================  EOF  ============================================== */
