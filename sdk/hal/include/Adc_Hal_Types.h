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
 * AutoChips Inc. (C) 2024. All rights reserved.
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
 * @file Adc_Hal_Types.h
 * @brief This file provides adc hal types header.
 */

#ifndef ADC_HAL_TYPES_H
#define ADC_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
#ifndef BIT
#define BIT(x) (1UL << (uint32)(x))
#endif

/*!< ADC regular group sequence length. */
#define ADC_REGULAR_SEQUENCE_LENGTH         (24UL)

/*!< ADC injected group sequence length. */
#define ADC_INJECT_SEQUENCE_LENGTH          (4UL)

/*!< get ADC average count number, x should be Adc_AverageType */
#define ADC_GET_AVERAGE_NUM(x)              (4UL << (x))

/*!< ADC regular group trigger source count number in CTU */
#define ADC_CTU_REGULAR_SOURCE_COUNT        (4UL)
/*!< ADC injection group trigger source count number in CTU */
#define ADC_CTU_INJECT_SOURCE_COUNT         (4UL)

/*===================================================ENUMS==========================================*/
/**
 * @brief Type of clock prescaler factor.
 */
typedef enum
{
    ADC_CLK_DIVIDE_1 = 0UL, /*!< Input clock divided by 1. */
    ADC_CLK_DIVIDE_2, /*!< Input clock divided by 2. */
    ADC_CLK_DIVIDE_3, /*!< Input clock divided by 3. */
    ADC_CLK_DIVIDE_4, /*!< Input clock divided by 4. */
    ADC_CLK_DIVIDE_5, /*!< Input clock divided by 5. */
    ADC_CLK_DIVIDE_6, /*!< Input clock divided by 6. */
    ADC_CLK_DIVIDE_7, /*!< Input clock divided by 7. */
    ADC_CLK_DIVIDE_8, /*!< Input clock divided by 8. */
    ADC_CLK_DIVIDE_9, /*!< Input clock divided by 9. */
    ADC_CLK_DIVIDE_10, /*!< Input clock divided by 10.*/
    ADC_CLK_DIVIDE_11, /*!< Input clock divided by 11.*/
    ADC_CLK_DIVIDE_12, /*!< Input clock divided by 12.*/
    ADC_CLK_DIVIDE_13, /*!< Input clock divided by 13.*/
    ADC_CLK_DIVIDE_14, /*!< Input clock divided by 14.*/
    ADC_CLK_DIVIDE_15, /*!< Input clock divided by 15.*/
    ADC_CLK_DIVIDE_16 /*!< Input clock divided by 16.*/
} Adc_PrescaleType;

/**
 * @brief Type of Channel Resolution in number of bits.
 */
typedef enum
{
    ADC_RESOLUTION_12BIT = 0U, /*!< 12-bit Resolution mode */
    ADC_RESOLUTION_10BIT, /*!< 10-bit Resolution mode */
    ADC_RESOLUTION_8BIT, /*!< 8-bit Resolution mode */
    ADC_RESOLUTION_MAX = 3U /*!< Invalid Resolution mode */
} Adc_ResolutionType;

/*!
 * @brief Voltage reference selection
 */
typedef enum
{
    ADC_VOLTAGEREF_VREF = 0U, /*!< VrefH and VrefL as Voltage reference. */
    ADC_VOLTAGEREF_VDDA /*!< VDDA as Voltage reference. */
} Adc_VoltageReferenceType;

/**
 * @brief Type of sampling time, i.e. the time during which the Value is sampled, (in clock-
 * cycles).
 */
typedef enum
{
    ADC_SPT_CLK_5 = 0U, /*!< ADC Sample time 5 Clock Cycle */
    ADC_SPT_CLK_10, /*!< ADC Sample time 10 Clock Cycle */
    ADC_SPT_CLK_15, /*!< ADC Sample time 15 Clock Cycle */
    ADC_SPT_CLK_23, /*!< ADC Sample time 23 Clock Cycle */
    ADC_SPT_CLK_35, /*!< ADC Sample time 35 Clock Cycle */
    ADC_SPT_CLK_45, /*!< ADC Sample time 45 Clock Cycle */
    ADC_SPT_CLK_85, /*!< ADC Sample time 85 Clock Cycle */
    ADC_SPT_CLK_185 = 7U, /*!< ADC Sample time 185 Clock Cycle */
    ADC_SPT_CLK_MAX = 8U /*!< Invalid ADC Sample time */
} Adc_SamplingTimeType;

#ifndef ADC_SDK_NON_EXTENDED_API
/*!
 * @brief ADC conversion group type enumeration.
 */
typedef enum
{
    ADC_REGULAR_GROUP = 0U, /*!< ADC regular group */
    ADC_INJECT_GROUP, /*!< ADC inject group */
    ADC_GROUP_MAX /*!< ADC group max Value, should be less than this Value */
} Adc_GroupConvType;
#endif

/*!
 * @brief ADC sequence type enumeration.
 */
typedef enum
{
    ADC_RSEQ_0 = 0U, /*!< ADC rseq 0 */
    ADC_RSEQ_1, /*!< ADC rseq 1 */
    ADC_RSEQ_2, /*!< ADC rseq 2 */
    ADC_RSEQ_3, /*!< ADC rseq 3 */
    ADC_RSEQ_4, /*!< ADC rseq 4 */
    ADC_RSEQ_5, /*!< ADC rseq 5 */
    ADC_RSEQ_6, /*!< ADC rseq 6 */
    ADC_RSEQ_7, /*!< ADC rseq 7 */
    ADC_RSEQ_8, /*!< ADC rseq 8 */
    ADC_RSEQ_9, /*!< ADC rseq 9 */
    ADC_RSEQ_10, /*!< ADC rseq 10 */
    ADC_RSEQ_11, /*!< ADC rseq 11 */
    ADC_RSEQ_12, /*!< ADC rseq 12 */
    ADC_RSEQ_13, /*!< ADC rseq 13 */
    ADC_RSEQ_14, /*!< ADC rseq 14 */
    ADC_RSEQ_15, /*!< ADC rseq 15 */
    ADC_RSEQ_16, /*!< ADC rseq 16 */
    ADC_RSEQ_17, /*!< ADC rseq 17 */
    ADC_RSEQ_18, /*!< ADC rseq 18 */
    ADC_RSEQ_19, /*!< ADC rseq 19 */
    ADC_RSEQ_20, /*!< ADC rseq 20 */
    ADC_RSEQ_21, /*!< ADC rseq 21 */
    ADC_RSEQ_22, /*!< ADC rseq 22 */
    ADC_RSEQ_23, /*!< ADC rseq 23 */
    ADC_RSEQ_24, /*!< ADC rseq 23 */
    ADC_RSEQ_25, /*!< ADC rseq 23 */
    ADC_RSEQ_26, /*!< ADC rseq 23 */
    ADC_RSEQ_27, /*!< ADC rseq 23 */
    ADC_RSEQ_28, /*!< ADC rseq 23 */
    ADC_RSEQ_29, /*!< ADC rseq 23 */
    ADC_RSEQ_30, /*!< ADC rseq 23 */
    ADC_RSEQ_31, /*!< ADC rseq 23 */
    ADC_ISEQ_0, /*!< ADC iseq 0 */
    ADC_ISEQ_1, /*!< ADC iseq 1 */
    ADC_ISEQ_2, /*!< ADC iseq 2 */
    ADC_ISEQ_3, /*!< ADC iseq 3 */
    ADC_ISEQ_MAX /*!< MAX Seq index */
} Adc_SequenceType;

/*!
 * @brief ADC Channel type enumeration.
 */
typedef enum
{
    ADC_CH_0 = 0U, /*!< ADC Channel 0 */
    ADC_CH_1, /*!< ADC Channel 1 */
    ADC_CH_2, /*!< ADC Channel 2 */
    ADC_CH_3, /*!< ADC Channel 3 */
    ADC_CH_4, /*!< ADC Channel 4 */
    ADC_CH_5, /*!< ADC Channel 5 */
    ADC_CH_6, /*!< ADC Channel 6 */
    ADC_CH_7, /*!< ADC Channel 7 */
    ADC_CH_8, /*!< ADC Channel 8 */
    ADC_CH_9, /*!< ADC Channel 9 */
    ADC_CH_10, /*!< ADC Channel 10 */
    ADC_CH_11, /*!< ADC Channel 11 */
    ADC_CH_12, /*!< ADC Channel 12 */
    ADC_CH_13, /*!< ADC Channel 13 */
    ADC_CH_14, /*!< ADC Channel 14 */
    ADC_CH_15, /*!< ADC Channel 15 */
    ADC_CH_16, /*!< ADC Channel 16 */
    ADC_CH_17, /*!< ADC Channel 17 */
    ADC_CH_18, /*!< ADC Channel 18 */
    ADC_CH_19, /*!< ADC Channel 19 */
    ADC_CH_20, /*!< ADC Channel 20 */
    ADC_CH_21, /*!< ADC Channel 21 */
    ADC_CH_22, /*!< ADC Channel 22 */
    ADC_CH_23, /*!< ADC Channel 23 */
#if defined (AC7840X) || defined (AC7843X)
    ADC_CH_24, /*!< ADC Channel 24 */
    ADC_CH_25, /*!< ADC Channel 25 */
    ADC_CH_26, /*!< ADC Channel 26 */
    ADC_CH_27, /*!< ADC Channel 27 */
    ADC_CH_28, /*!< ADC Channel 28 */
    ADC_CH_29, /*!< ADC Channel 29 */
    ADC_CH_30, /*!< ADC Channel 30 */
    ADC_CH_31, /*!< ADC Channel 31 */
    ADC_CH_BANDGAP = 33U,  /*!< ADC internal Channel Bandgap */
    ADC_CH_TSENSOR = 34U,  /*!< ADC internal Channel T-Sensor */
    ADC_CH_GEOE_CAL = 35U, /*!< ADC GE OE calibration Channel */
    ADC_CH_SUPPLY = 37U,   /*!< ADC internal Channel Supply */
#if defined (AC7843X)
    ADC_CH_INTERLEAVE38,   /*!< ADC interleave channel 38 */
    ADC_CH_INTERLEAVE39,   /*!< ADC interleave channel 39 */
#endif
#elif defined (AC7842X)
    ADC_CH_GEOE_CAL = 24U,  /*!< ADC GE OE calibration Channel */
    ADC_CH_INTERNAL = 25U,  /*!< ADC internal Channel Supply */
#endif
    ADC_CH_MAX,            /*!< Invalid Channel */
    ADC_CH_DISABLE = 0x3FU /*!< Disabled Channel */
} Adc_InputChannelType;

#if defined (AC7842X) || defined (AC7843X)
/*!
 * @brief Analog bg output selest type enumeration.
 */
typedef enum
{
    ANA_OUTPUT_SELECT_BG1 = 3U,            /*!< OUTPUT select BG1 */
    ANA_OUTPUT_SELECT_BG0,                 /*!< OUTPUT select BG0 */
} Ana_BgOutputType;

/*!
 * @brief Analog tensor output selest type enumeration.
 */
typedef enum
{
    ANA_OUTPUT_SELECT_TSENSOR_OFF = 0U,    /*!< OUTPUT select TSENSOR OFF*/
    ANA_OUTPUT_SELECT_TSENSOR_ON = 3U      /*!< OUTPUT select TSENSOR ON*/
} Ana_TensorOutputType;
#endif

#if defined (AC7840X) || defined (AC7843X)
/*!
 * @brief ADC internal channel source type enumeration.
 */
typedef enum
{
    ADC_INTL_CH_SRC_VDD = 0U,                /*!< ADC internal Channel VDD Supply */
    ADC_INTL_CH_SRC_VDDA,                    /*!< ADC internal Channel VDDA Supply */
    ADC_INTL_CH_SRC_REFH,                    /*!< ADC reference positive input */
    ADC_INTL_CH_SRC_OSC_LDO,                 /*!< ADC OSC LDO Supply */
    ADC_INTL_CH_SRC_FLASH_LDO,               /*!< ADC flash LDO Supply */
    ADC_INTL_CH_SRC_DIGITAL_LDO,             /*!< ADC digital LDO Supply */
    ADC_INTL_CH_SRC_MAX = 6U                 /*!< ADC Supply max Value */
} Adc_InternalChannelSrcType;
#elif defined (AC7842X)
/*!
 * @brief ADC internal channel source type enumeration.
 */
typedef enum
{
    ADC_INTL_CH_SRC_DIGLDO_SUPPLY = 0U,   /*!< ANA internal AMUX channel DIGLDO supply */
    ADC_INTL_CH_SRC_FLHLDO_SUPPLY,        /*!< ADC internal AMUX channel FLASHLDO supply */
    ADC_INTL_CH_SRC_BG0,                  /*!< ADC internal AMUX channel BG0 */
    ADC_INTL_CH_SRC_BG1 = 6U,             /*!< ADC internal AMUX channel BG1 */
    ADC_INTL_CH_SRC_TSENSOR0,             /*!< ADC internal AMUX channel TSENSOR0 */
    ADC_INTL_CH_SRC_TSENSOR1,             /*!< ADC internal AMUX channel TSENSOR1 */
    ADC_INTL_CH_SRC_VDDA = 10U,           /*!< ANA internal AMUX channel AVDD */
    ADC_INTL_CH_SRC_VDD,                  /*!< ADC internal AMUX channel VDD */
    ADC_INTL_CH_SRC_XOSC = 15U,           /*!< ADC internal AMUX channel XOSC */
    ADC_INTL_CH_SRC_MAX                   /*!< ADC ana max value */
} Adc_InternalChannelSrcType;
#endif

#if defined (AC7843X)
/*!
 * @brief Analog ADC internal AMUX channel type enumeration.
 */
typedef enum
{
    ANA_CHANNEL_AMUX_BG0 = 4U,             /*!< ANA internal AMUX channel BG0 */
    ANA_CHANNEL_AMUX_BG1,                  /*!< ADC internal AMUX channel BG1 */
    ANA_CHANNEL_AMUX_TSENSOR0,             /*!< ADC internal AMUX channel TSENSOR0 */
    ANA_CHANNEL_AMUX_TSENSOR1,             /*!< ADC internal AMUX channel TSENSOR1 */
    ANA_CHANNEL_AMUX_MAX = 8U              /*!< ADC supply max value */
} Ana_AmuxChannelType;
#endif

/*!
 * @brief Hardware average selection
 */
typedef enum
{
    ADC_AVERAGE_4 = 0U, /*!< Hardware average of 4 samples. */
    ADC_AVERAGE_8 = 1U, /*!< Hardware average of 8 samples. */
    ADC_AVERAGE_16 = 2U, /*!< Hardware average of 16 samples. */
    ADC_AVERAGE_32 = 3U /*!< Hardware average of 32 samples. */
} Adc_AverageType;

/*!
 * @brief Hardware average selection
 */
typedef enum
{
    ADC_INTERLEAVE_DISABLE = 0U, /*!< ADC interleave disable. */
    ADC_INTERLEAVE_0 = 1U, /*!< ADC0/1 interleave PB13/PB0. */
    ADC_INTERLEAVE_1 = 3U, /*!< ADC0/1 interleave PB14/PB1. */
} Adc_InterleaveType;

/**
 * @brief Type for Alignment of ADC raw results in ADC result buffer (left/right Alignment).
 */
typedef enum
{
    ADC_ALIGN_RIGHT = 0x00U, /*!< AMO Level trigger */
    ADC_ALIGN_LEFT, /*!< AMO Level trigger */
} Adc_ResultAlignmentType;

/*!
 * @brief ADC analog monitor trigger mode enumeration.
 */
typedef enum
{
    ADC_AMO_TRIGGER_LEVEL = 0U, /*!< AMO Level trigger */
    ADC_AMO_TRIGGER_EDGE /*!< AMO Edge trigger */
} Adc_AmoTriggerModeType;

/**
 * @brief the trigger source for an ADC Channel group.
 */
typedef enum
{
    ADC_TRIGG_SRC_SW = 0U, /*!< Internal software trigger */
    ADC_TRIGG_SRC_HW /*!< External signal trigger */
} Adc_TriggerSourceType;

/*!
 * @brief Events which can trigger ADC Callback
 */
typedef enum
{
#ifndef ADC_SDK_NON_EXTENDED_API
    ADC_EVENT_AMO = ADC_STR_AMO_Msk, /*!< Abnormal Event in AMO level mode */
    ADC_EVENT_AAMO = ADC_STR_AAMO_Msk, /*!< Abnormal Event in AMO edge mode */
    ADC_EVENT_NAMO = ADC_STR_NAMO_Msk, /*!< Normal Event in AMO edge mode */
#endif
    ADC_EVENT_EOC = 0x80U, /*!< End of convertion evnet */
    ADC_EVENT_IEOC = 0x8000U, /*!< End of inject convertion evnet */
} Adc_EventType; //PRQA S 1535 # members are used.*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*!
 * @brief ADC hardware average structure
 */
typedef struct
{
    boolean Enable; /*!< Enable hardware averaging */
    Adc_AverageType Value; /*!< Selection for number of samples used for hardware averaging */
} Adc_Average_ConfigType;

/*!
 * @brief ADC interrupt information structure
 */
typedef struct
{
    uint8 Instance; /*!< ADC Instance number */
    uint32 Event; /*!< adc Event bit mask, combination of Adc_EventType */
    Adc_SequenceType sequence; /*!< Group seqeunce index */
} Adc_InterruptInfoType;

/*!
 * @brief ADC interrupt handler type
 */
typedef void (*Adc_CallbackType)(const Adc_InterruptInfoType *Info);

/*!
 * @brief ADC genernal convertion config
 */
typedef struct //PRQA S 3630 # the defined external structure or union does not need to be hidden.*/
{
    Adc_Average_ConfigType HwAverage; /*!< ADC HW average setting */
    Adc_InterleaveType Interleave; /*!< ADC Interleave setting */
    Adc_PrescaleType ClockDivide; /*!< ADC clock divide */
    Adc_ResolutionType Resolution; /*!< ADC Resolution setting */
    Adc_ResultAlignmentType Alignment; /*!< ADC result Alignment setting */
    Adc_VoltageReferenceType VoltageRef; /*!< ADC reference source selection */
    boolean DmaEnable; /*!< Enable/disable ADC DMA support function */
    boolean PowerEn; /*!< Enable/disbale ADC analog module power Supply */
} Adc_ConverterConfigType;

/*!
 * @brief Defines the control Channel configuration
 */
typedef struct //PRQA S 3630 # the defined external structure or union does not need to be hidden.*/
{
    Adc_InputChannelType Channel; /*!< Selection of input Channel for measurement */
    Adc_SamplingTimeType Spt; /*!< Sample time */
    Adc_SequenceType SeqIndex; /*!< Group seqeunce index */
    boolean InterruptEn; /*!< Enable/disbale EOC irq for the seqeunce index */
    Adc_InternalChannelSrcType InternalChannelSrc; /*!< slecttion of internal channel srouce*/
} Adc_ChanConfigType;

/*!
 * @brief Defines the control group configuration
 */
typedef struct //PRQA S 3630 # the defined external structure or union does not need to be hidden.*/
{
    uint32 DmaDstAddr;
    Adc_TriggerSourceType RegularTrigger; /*!< regular group trigger source type */
    Adc_TriggerSourceType InjectTrigger; /*!< injection group trigger source type */
    uint8 RegularSequenceLength; /*!< Regular sequence length */
    uint8 InjectSequenceLength; /*!< Injected sequence length */
    uint8 RegularDiscontinuousNum; /*!< Regular discontinuous mode number */
    boolean ScanModeEn; /*!< Enable/disable scan mode for regular and injected group */
    boolean ContinuousModeEn; /*!< Enable/disable continuous mode */
    boolean RegularDiscontinuousModeEn; /*!< Enable/disable discontinuous mode for regular group */
    boolean InjectDiscontinuousModeEn; /*!< Enable/disable discontinuous mode for injected group */
    boolean InjectAutoModeEn; /*!< Enable/disable antomatic mode for injected group */
    boolean IntervalModeEn; /*!< Enable/disable interval mode for mode3/5 */
    Hal_CallbackType DmaCallback; /*!< Dma callback */
    void *DmaArgs; /*!< Dma callback args */
} Adc_GroupConfigType;

/*!
 * @brief ADC analog monitor(AMO) config structure
 */
typedef struct //PRQA S 3630 # the defined external structure or union does not need to be hidden.*/
{
    Adc_AmoTriggerModeType AmoTriggerMode; /*!< 0:level trigger; 1:edge trigger */
    boolean AmoInterruptEn; /*!< Enable/disable AMO interrupt */
    boolean AmoRegularEn; /*!< Enable/disable analog monitor regular group mode */
    boolean AmoInjectEn; /*!< Enable/disable analog monitor injected group mode */
    boolean AmoSingleModeEn; /*!< Enable/disable analog monitor single Channel mode */
    Adc_InputChannelType AmoSingleChannel; /*!< Select analog monitor single Channel */
    uint16 AmoUpThreshold; /*!< Set analog monitor upper Threshold */
    uint16 AmoLowThreshold; /*!< Set analog monitor lower Threshold */
    uint16 AmoUpOffset; /*!< Set analog monitor upper Offset */
    uint16 AmoLowOffset; /*!< Set analog monitor lower Offset */
} Adc_AmoConfigType;

/*!
 * @brief ADC interrupt struct
 */
typedef struct
{
    Adc_CallbackType Callback; /*!< ADC irq callback */
} Adc_InterruptType;

/*!
 * @brief ADC init config args structure.
 */
typedef struct //PRQA S 3630 # the defined external structure or union does not need to be hidden.*/
{
    Adc_InterruptType Interrupt; /*!< ADC irq args */
} Adc_InitConfigType;

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
