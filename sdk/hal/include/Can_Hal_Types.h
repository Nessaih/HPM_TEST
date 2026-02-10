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
 * @file Can_Hal_Types.h
 * @brief This file provides can hal types header.
 */

#ifndef CAN_HAL_TYPES_H
#define CAN_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/*#define CAN_PERFORMANCE_TEST*/

#ifndef BIT
#define BIT(x) (1UL << (uint32)(x))
#endif

#if defined (AC7840X) || defined (AC7842X)
/*!< CAN common irq all supports enable masks. */
#define CAN_COMMON_IRQ_EN_MASKS (CAN_CTRL1_EIE_Msk | CAN_CTRL1_TSIE_Msk | CAN_CTRL1_TPIE_Msk |\
                                 CAN_CTRL1_RAFIE_Msk | CAN_CTRL1_RFIE_Msk | CAN_CTRL1_ROIE_Msk |\
                                 CAN_CTRL1_RIE_Msk | CAN_CTRL1_EPIE_Msk | CAN_CTRL1_ALIE_Msk |\
                                 CAN_CTRL1_BEIE_Msk)

/*!< CAN common irq all supports flag masks. */
#define CAN_COMMON_IRQ_FLAG_MASKS (CAN_CTRL1_EIF_Msk | CAN_CTRL1_BEIF_Msk | CAN_CTRL1_AIF_Msk |\
                                   CAN_CTRL1_RAFIF_Msk | CAN_CTRL1_RFIF_Msk | CAN_CTRL1_ROIF_Msk |\
                                   CAN_CTRL1_RIF_Msk | CAN_CTRL1_EPIF_Msk | CAN_CTRL1_ALIF_Msk |\
                                   CAN_CTRL1_TSIF_Msk | CAN_CTRL1_TPIF_Msk)
/*!< CAN ecc irq all supports enable masks. */
#define CAN_ECC_IRQ_EN_MASKS            (CAN_VERMEM_MDWIE_Msk | CAN_VERMEM_MDEIE_Msk)

/*!< CAN ecc irq all supports flag masks. */
#define CAN_ECC_IRQ_FLAG_MASKS          (CAN_VERMEM_MDWIF_Msk | CAN_VERMEM_MDEIF_Msk)

#define CAN_MAX_S_SEG_1    (0xFFUL)        /*!< CAN max slow bitrate seg 1 */
#define CAN_MAX_S_SEG_2    (0x7FUL)        /*!< CAN max slow bitrate seg 2 */
#define CAN_MAX_S_SJW      (0x7FUL)        /*!< CAN max slow bitrate sjw */
#define CAN_MAX_S_PRESC    (0xFFUL)       /*!< CAN max slow bitrate prescaler */
#define CAN_MAX_F_SEG_1    (0x1FUL)        /*!< CAN max fast bitrate seg 1 */
#define CAN_MAX_F_SEG_2    (0xFUL)         /*!< CAN max fast bitrate seg 2 */
#define CAN_MAX_F_SJW      (0xFUL)         /*!< CAN max fast bitrate sjw */
#define CAN_MAX_F_PRESC    (0xFFU)        /*!< CAN max fast bitrate prescaler */

#define CAN_RX_FIFO_NUM_MAX (1UL)

#elif defined (AC7843X)
#define CAN_COMMON_IRQ_EN_MASKS         (0x3FFFFFFFUL)
/*!< Enable all DMU interrupt mask */
#define DMU_IRQ_ALL_ENABLE_MSK          (0x601FDF00UL)

#define CAN_MAX_S_SEG_1    (0xFFUL)        /*!< CAN max slow bitrate seg 1 */
#define CAN_MAX_S_SEG_2    (0x7FUL)        /*!< CAN max slow bitrate seg 2 */
#define CAN_MAX_S_SJW      (0x7FUL)        /*!< CAN max slow bitrate sjw */
#define CAN_MAX_S_PRESC    (0x1FFUL)       /*!< CAN max slow bitrate prescaler */
#define CAN_MAX_F_SEG_1    (0x1FUL)        /*!< CAN max fast bitrate seg 1 */
#define CAN_MAX_F_SEG_2    (0xFUL)         /*!< CAN max fast bitrate seg 2 */
#define CAN_MAX_F_SJW      (0xFUL)         /*!< CAN max fast bitrate sjw */
#define CAN_MAX_F_PRESC    (0x1FUL)        /*!< CAN max fast bitrate prescaler */

#define CAN_DEDICATED_BUFFER_DEPTH_NUM (0x1U) /*!< CAN dedicated rx/tx buffer depth num */

#define CAN_STD_FILTER_NUM_MAX (128UL)
#define CAN_EXT_FILTER_NUM_MAX (64UL)
#define CAN_FILTER_NUM_MAX (CAN_STD_FILTER_NUM_MAX + CAN_STD_FILTER_NUM_MAX)

#define CAN_TX_FIFO_NUM_MAX (1UL)
#define CAN_TX_BUF_NUM_MAX (32UL)
#define CAN_TX_EVENT_FIFO_DEPTH_MAX (32UL)

#define CAN_RX_FIFO_NUM_MAX (2UL)
#define CAN_RX_FIFO_DEPTH_MAX (64UL)
#define CAN_RX_DEDICATED_BUFFER_NUM_MAX (64UL)
#define CAN_RX_BUF_NUM_MAX (66UL)
#endif
/*===================================================ENUMS==========================================*/
/*!
 * @brief CAN error mask type enumeration.
 */
typedef enum
{
    CAN_BUSERR_BIT_MASK = 0x1, /*!< can bus error: bit error */
    CAN_BUSERR_FORM_MASK = 0x2, /*!< can bus error: form error */
    CAN_BUSERR_STUFF_MASK = 0x4, /*!< can bus error: stuff error */
    CAN_BUSERR_ACK_MASK = 0x8, /*!< can bus error: ack error */
    CAN_BUSERR_CRC_MASK = 0x10, /*!< can bus error: crc error */
    CAN_BUSERR_OTHER_MASK = 0x20,/*!< can bus error: others error */
    CAN_ARBIT_ERR_MASK = 0x40, /*!< can arbitrate error */
    CAN_OVERFLOW_ERR_MASK = 0x80,/*!< can receive fifo overflow error */
    CAN_ECC_WARN_MASK = 0x100, /*!< can ecc warning, refer to CAN_VERMEM_MDWIF_Msk */
    CAN_ECC_ERR_MASK = 0x200, /*!< can ecc error, refer to CAN_VERMEM_MDEIF_Msk */
} Can_ErrorMaskType;

/*!
 * @brief CAN frame RTR type type enumeration.
 */
typedef enum
{
    CAN_CTRL_DATA_FRAME = 0x0U, /*!< can frame RTR: data frame */
    CAN_CTRL_REMOTE_FRAME, /*!< can frame RTR: remote frame */
} Can_CtrlFrameType;

/*!
 * @brief CAN frame DLC value type enumeration.
 */
typedef enum
{
    CAN_MSG_DLC_12_BYTES = 0x09U,/*!< can frame DLC value, valid data len 12 bytes */
    CAN_MSG_DLC_16_BYTES, /*!< can frame DLC value, valid data len 16 bytes */
    CAN_MSG_DLC_20_BYTES, /*!< can frame DLC value, valid data len 20 bytes */
    CAN_MSG_DLC_24_BYTES, /*!< can frame DLC value, valid data len 24 bytes */
    CAN_MSG_DLC_32_BYTES, /*!< can frame DLC value, valid data len 32 bytes */
    CAN_MSG_DLC_48_BYTES, /*!< can frame DLC value, valid data len 48 bytes */
    CAN_MSG_DLC_64_BYTES, /*!< can frame DLC value, valid data len 64 bytes */
} Can_MsgDlcType;

/*!
 * @brief CAN extend mode type enumeration.
 */
typedef enum
{
    CAN_EXTMODE_OFF = 0x0U, /*!< can extend mode off */
    CAN_EXTMODE_LISTENING, /*!< can extend mode listening */
    CAN_EXTMODE_LOOPBACK_INTERNEL,/*!< can extend mode internal */
    CAN_EXTMODE_LOOPBACK_EXTERNEL,/*!< can extend mode external */
} Can_ExtendModeType;

/*!
 * @brief CAN error state type enumeration.
 */
typedef enum
{
    CAN_ERR_STATE_ACTIVE = 0x0U, /*!< error state active */
    CAN_ERR_STATE_PASSIVE, /*!< error state passive */
    CAN_ERR_STATE_BUSOFF, /*!< error state busoff */
} Can_DevErrorStateType;

/*!
 * @brief CAN work state type enumeration.
 */
typedef enum
{
    CAN_STATE_STOP = 0x0U, /*!< state stop */
    CAN_STATE_RUNNING, /*!< state running */
    CAN_STATE_STANDBY, /*!< state standby */
} Can_HalStateType;

/*!
 * @brief CAN frame type enumeration.
 */
typedef enum
{
    CAN_STANDARD_MSG = 0x0U, /*!< standard frame */
    CAN_EXTENDED_MSG, /*!< extend frame */
} Can_MessageIdType;

/*!
 * @brief CAN filter type enumeration.
 */
typedef enum
{
    STD_FRAME = 0U, /*!< Only receive standard frame id */
    EXT_FRAME, /*!< Only receive extend frame id */
#if defined (AC7840X) || defined (AC7842X)
    STD_EXT_FRAME, /*!< receive standard/extend frame id */
#endif
} Can_FilterFrameType;

#if defined (AC7840X) || defined (AC7842X)
/*!
 * @brief CAN tx buf type enumeration.
 */
typedef enum
{
    CAN_TX_BUF_PRIMARY = 0U, /*!< Tx primary buffer */
    CAN_TX_BUF_SECONDARY     /*!< Tx secondary buffer */
} Can_TxBufType;

/*!
 * @brief CAN tx secondary buf amount.
 */
typedef enum
{
    CAN_TX_SEC_ONE = 0U,    /*!< Transmit secondary one message  */
    CAN_TX_SEC_ALL          /*!< Transmit secondary all messages */
} Can_TxSecAmountType;
#endif

#if defined (AC7843X)
/*!
 * @brief CAN frame data size enumeration.
 */
typedef enum
{
    CAN_DATA_SIZE_8 = 0U, /*!< frame data size 8 bytes*/
    CAN_DATA_SIZE_12,     /*!< frame data size 12 bytes*/
    CAN_DATA_SIZE_16,     /*!< frame data size 16 bytes*/
    CAN_DATA_SIZE_20,     /*!< frame data size 20 bytes*/
    CAN_DATA_SIZE_24,     /*!< frame data size 24 bytes*/
    CAN_DATA_SIZE_32,     /*!< frame data size 32 bytes*/
    CAN_DATA_SIZE_48,     /*!< frame data size 48 bytes*/
    CAN_DATA_SIZE_64,     /*!< frame data size 64 bytes*/
} Can_DataSizeType;

/*!
 * @brief Rx FIFO mode enumeration.
 */
typedef enum
{
    BLOCK_MODE = 0U, /*!< Blocking mode */
    OVERWRITE_MODE /*!< Overwrite mode */
} Can_RxFifoModeType;

/*!
 * @brief CAN MRAM type struct.
 */
typedef enum
{
    DEDICATED_RX_BUFFER, /*!< Rx buffer dedicated */
    RX_FIFO0, /*!< Rx buffer fifo0  */
    RX_FIFO1, /*!< Rx buffer fifo1  */
    TX_BUFFER, /*!< Tx buffer */
    TX_EVENT_FIFO, /*!< Tx event fifo */
    STD_FILTER, /*!< Standard id filter */
    EXT_FILTER /*!< Extend id filter */
} Can_MramType;

/*!
 * @brief CAN tx buffer type enumeration.
 */
typedef enum
{
    TX_BUF_TYPE_DEDICATED, /*!< Dedicated tx buffer */
    TX_BUF_TYPE_FIFO, /*!< Tx fifo */
} Can_TxBufferType;

/*!
 * @brief CAN tx buffer type enumeration.
 */
typedef enum
{
    RX_BUF_TYPE_DEDICATED, /*!< Rx dedicated buffer */
    RX_BUF_TYPE_FIFO0, /*!< Rx fifo 0 */
    RX_BUF_TYPE_FIFO1, /*!< Rx fifo 1 */
} Can_RxBufferType;

/*!
 * @brief CAN filter type enumeration.
 */
typedef enum
{
    FILTER_RANGE = 0U, /*!< Range filter */
    FILTER_DUAL, /*!< Dual ID filter */
    FILTER_CLASSIC, /*!< Classic filter */
    STD_DISABLE_OR_EXT_RANGE /*!< Filter disable(STD), Range filter with XIDAM mask not applied(EXT)*/
} Can_FilterType;

/*!
 * @brief Filter element configuration.
 */
typedef enum
{
    DISABLE_FILTER = 0U, /*!< Filter disable */
    STORE_IN_RXFIFO0, /*!< Store in Rx FIFO 0 if filter matches */
    STORE_IN_RXFIFO1, /*!< Store in Rx FIFO 1 if filter matches */
    REJECT_MATCH_ID, /*!< Reject ID if filter matches */
    SET_PRIORITY, /*!< Set priority if filter matches */
    SET_PRIORITY_STORE_IN_RXFIFO0, /*!< Set priority and store in FIFO 0 if filter matches */
    SET_PRIORITY_STORE_IN_RXFIFO1, /*!< Set priority and store in FIFO 1 if filter matches */
    STORE_IN_RXBUFFER /*!< Store into Rx Buffer or as debug message */
} Can_FilterElementCfgType;

/*!
 * @brief Rx filter no-match handler struct.
 */
typedef enum
{
    ACCEPT_RXFIFO0 = 0U, /*!< Store in rx fifo 0 */
    ACCEPT_RXFIFO1, /*!< Store in rx fifo 1 */
    REJECT_NOMATCH_ID /*!< Reject message */
} Can_NoMacthFramesType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*!
 * @brief High priority message status struct.
 */
typedef struct
{
    uint8 FLST; /*!< Filter list */
    uint8 FIDX; /*!< Filter index */
    uint8 MSI; /*!< Message storage indicator */
    uint8 BIDX; /*!< Buffer index */
} Can_HpmsStatusType;

/*!
 * @brief Message RAM configuration struct.
 */
typedef struct
{
    boolean useDefault; /*!< Use default config */
    uint8 txFifoNumber; /*!< Tx fifo number */
    uint8 txDbufferNumber; /*!< Dedicate tx buffer number */
    uint8 stdFilterNumber; /*!< Standard filter number */
    uint8 extFilterNumber; /*!< Extened filter number */
    uint8 rxFifo0Number; /*!< Rx fifo 0 number */
    uint8 rxFifo1Number; /*!< Rx fifo 1 number */
    uint8 rxDbufferNumber; /*!< Dedicate rx buffer number */
    uint8 txEventFifoNumber; /*!< Tx event Fifo number */
    uint8 rxDbufferDataSize; /*!< Dedicate rx buffer data size */
    uint8 rxFifo0DataSize; /*!< Rx fifo 0 data size */
    uint8 rxFifo1DataSize; /*!< Rx fifo 1 data size */
    uint8 txBufferDataSize; /*!< Tx buffer data size */
} Can_MramConfigType;
#endif
/*!
 * @brief Message RAM start address struct.
 */
typedef struct
{
    uint32 rxFifo0StartAddress; /*!< Rx Fifo 0 start address */
    uint32 rxFifo1StartAddress; /*!< Rx Fifo 1 start Aaddress */
    uint32 rxDbufferStartAddress; /*!< Dedicate rx buffer start address */
    uint32 txBufferStartAddress; /*!< Tx Buffer Start Address */
    uint32 stdFilterStartAddress; /*!< Std Filter Start Address */
    uint32 extFilterStartAddress; /*!< Ext Filter Start Address */
    uint32 txEventfifoStartAddress; /*!< Ex Event fifo Start Address */
    uint32 endAddress;
} Can_MramAddressType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*!
 * @brief CAN tx/rx frame information structure.
 */
typedef struct
{
    boolean MsgFdf; /*!< FD format indicator(FDF) */
    boolean MsgRtr; /*!< Remote transmission request(RTR) */
    boolean MsgBrs; /*!< Bit rate switch(BRS) */
    Can_MessageIdType MsgIdType; /*!< frame type, standard frame or extended frame */
    uint8 MsgDataLen; /*!< frame data len */
    uint8 *MsgDataPtr; /*!< frame data ptr*/
    uint32 MsgRts; /*!< Receive time stamp(RTS), rx only*/
    uint32 MsgId; /*!< CAN identifier */
    uint8 FdPaddingValue; /*!< CAN FD padding value, tx only*/
#if defined (AC7843X)
    boolean EfcAnmf; /*!< event fifo control(EFC) when tx, indicates whether to store send event in  tx event fifo. */
                     /* accepted non-matching frame(ANMF) when rx, */
                     /*indicates the matching status between the received frame and the filters */
    uint8 MmFidx; /*!< message marker(MM) when tx, tx msg cfg it, can controler copy it to tx event fifo when msg sended, */
                  /* used to indicates send msg state */
                  /* Filter Index(FIDX) when rx, indicates the frame matched fileter index*/
#endif
} Can_MessageInfoType;

/*!
 * @brief CAN IRQ callback args structure.
 */
typedef struct
{
    uint8 Instance; /*!< can hw unit index */
    uint32 CommonIrqFlagMasks; /*!< can common irq flag masks, refer to CAN_COMMON_IRQ_FLAG_MASKS*/
#if defined (AC7840X) || defined (AC7842X)
    uint32 EccIrqFlagMasks; /*!< can ecc irq flag masks, refer to CAN_ECC_IRQ_FLAG_MASKS*/
#elif defined (AC7843X)
    uint32 RxNewData1; /*!< can rx new data1 */
    uint32 RxNewData2; /*!< can rx new data2 */
    uint32 TxBTO; /*!< can tx TXBTO reg val */
#endif
} Can_IrqCBParams;

/*!
 * @brief CAN DMU IRQ callback args structure.
 */
typedef struct
{
    uint8 Instance; /*!< can hw unit index */
    uint32 DmuIrqFlagMasks; /*!< can dmu irq flag masks*/
} Can_DmuIrqCBParams;

/*!
 * @brief CAN timestamp config information structure.
 */
typedef struct
{
    boolean En; /*!< can timestamp enable, affect if TTS/RTS is valid*/
    boolean ExtClkSrc; /*!< can timestamp use external clk source or not*/
    boolean PosEnd; /*!< can timestamp mark postion select, false:SOF true:EOF*/
    uint8 ExtClkDiv; /*!< can timestamp external clk source division, 0:clk/1 ... 7:clk/8*/
} Can_TimeStampType;

#if defined (AC7843X)
/*!
 * @brief CAN global filter configuration struct.
 */
typedef struct
{
    boolean rejectStdRemote; /*!< Reject remote frames standard */
    boolean rejectExtRemote; /*!< Reject remote frames extended */
    Can_NoMacthFramesType std; /*!< Accept non-matching frames standard */
    Can_NoMacthFramesType ext; /*!< Accept non-matching frames extended */
} Can_GlobalFilterConfigType;

/*!
 * @brief can device args struct type
 */
typedef struct
{
    uint8 BufferType; /*!< Buffer type */
    uint8 BufferIndex; /*!< Buffer index */
} Can_BufferInfoType;

/*!
 * @brief CAN tx buffer config args structure.
 */
typedef struct
{
    uint8 DepthNum; /*!< can FIFO depth num*/
    boolean TxCmpIrqEn; /*!< 1 bit of TXBTIE*/
    boolean TxCnlIrqEn; /*!< 1 bit of TXBCIE*/
} Can_TxBufferConfigType;

/*!
 * @brief CAN rx buffer config args structure.
 */
typedef struct
{
    uint8 DepthNum; /*!< can FIFO depth num */
} Can_RxBufferConfigType;

/*!
 * @brief CAN frame data size config args structure.
 */
typedef struct
{
    Can_DataSizeType txBufferDataSize; /*!< Tx buffer data size */
    Can_DataSizeType rxDbufferDataSize; /*!< Dedicate rx buffer data size */
    Can_DataSizeType rxFifo0DataSize; /*!< Rx fifo 0 data size */
    Can_DataSizeType rxFifo1DataSize; /*!< Rx fifo 1 data size */
} Can_DataSizeConfigType;

/*!
 * @brief CAN tx event fifo element information structure.
 */
typedef struct
{
    uint8 ESI; /*!< Error state indicator */
    uint8 XTD; /*!< Extended identifier */
    uint8 RTR; /*!< Remote transmission request */
    uint8 MM; /*!< Message marker */
    uint8 EFC; /*!< Event type */
    uint8 FDF; /*!< FD format */
    uint8 BRS; /*!< Bit rate switch */
    uint8 DLC; /*!< Data length code */
    uint16 TXTS; /*!< Tx timestamp */
    uint32 ID; /*!< Identifier */
} Can_MsgEventType;
#endif

/*!
 * @brief CAN filter config args structure.
 */
typedef struct
{
    uint32 ID1; /*!< filter ID1 or Code*/
    uint32 ID2; /*!< filter ID2 or Mask*/
    Can_FilterFrameType FrameType;/*!< filter accept frame type*/
#if defined (AC7843X)
    Can_FilterType Type; /*!< filter type */
    Can_FilterElementCfgType FEC; /*!< filter element configuration */
#endif
} Can_FilterParamsType;

typedef struct
{
    uint8 FiltersNum; /*!< can standard filters num */
    Can_FilterParamsType *FiltersParamsPtr; /*!< can extended filters args array ptr*/
#if defined (AC7843X)
    Can_GlobalFilterConfigType GlobalFilter;/*!< Global filter config */
    uint32 XIDAM; /*!< extended id acceptance Mask */
#endif
} Can_FiltersConfigType;

/*!
 * @brief CAN bitrate config args structure.
 */
typedef struct
{
    uint16 Presc; /*!< bitrate args: Presc(BRP) 0:clk/1 ... 255:clk/256 */
    uint8 Seg1; /*!< bitrate args: Seg1 0 ~ 0xFF*/
    uint8 Seg2; /*!< bitrate args: Seg2 0 ~ 0x7F*/
    uint8 Sjw; /*!< bitrate args: Sjw 0 ~ 0x7F*/
} Can_BitrateParamsType;

/*!
 * @brief CAN baudrate config args structure.
 */
typedef struct
{
    Can_BitrateParamsType NormalBitrate; /*!< Can normal bitrate */
    Can_BitrateParamsType DataBitrate; /*!< Can data bitrate */
    uint8 SspOffset; /*!< Can SSPOFF:0 disable others:1~0x7F */
#if defined (AC7843X)
    uint8 TDCFWL; /*!< Can transmitter delay compensation filter window length */
#endif
} Can_BaudrateConfigType;

/*!
 * @brief CAN baudrate config args structure.
 */
typedef struct
{
    uint8 FifoId; /*!< Can rx fifo id */
    uint32 DmaDstAddr; /*!< Can dma dest address, it's length must one frame info size */
    Hal_CallbackType DmaCb; /*!< Can dma callback, user can swap DmaDstAddr and start dma again */
    void *DmaCbArgs; /*!< Can dma callback args, if needed */
} Can_RxFifoBufDmaConfigType;

/*!
 * @brief CAN baudrate config args structure.
 */
typedef struct
{
    boolean EccEn; /*!< Can ECC enable */
    boolean FdEn; /*!< Can FD enable */
    boolean FdIsoEn; /*!< Can FD ISO mode enable*/
    boolean RxOverWrite; /*!< Can rx overwrite enable, true: overwrite old frame when rx fifo overflow,*/
                         /*false: new frame ignore when rx fifo overflow */
    uint8 RxFifoBufDmaConfigNum; /*!< Can rx fifo buf dam config num */
    Can_RxFifoBufDmaConfigType *RxFifoBufDmaConfigPtr; /*!< Can rx fifo buf dam config args */
    Can_BaudrateConfigType *BaudrateConfigPtr; /*!< Can Baudrate config args */
    Can_FiltersConfigType Filters; /*!< Can Filters config args */
#if defined (AC7843X)
    uint8 RxBuffersNum; /*!< can rx buffers num*/
    uint8 TxBuffersNum; /*!< can tx buffers num*/
    Can_RxBufferConfigType *RxBufferConfigPtr; /*!< Can rx buffer config args */
    Can_TxBufferConfigType *TxBufferConfigPtr; /*!< Can tx buffer config args */
    Can_BufferInfoType *RxBuffersInfoPtr; /*!< User alloc memory to hal and cannot release it after init,*/
                                          /* args fill by hal, user no need cfg it */
    Can_BufferInfoType *TxBuffersInfoPtr; /*!< User alloc memory to hal and cannot release it after init,*/
                                          /*args fill by hal, user no need cfg it */
    Can_DataSizeConfigType DataSize; /*!< Frame data size args */
    uint8 TxEventFifoNum; /*!< Tx event Fifo number */
    uint8 TxEventFifoWm;  /*!< Tx event Fifo watermark, EFWM */
#endif
    Hal_CallbackType WakeupIrqCallback; /*!< Can wakeup irq callback, NULL not enable irq */
    Hal_CallbackType IrqCallback; /*!< Can irq callback(inlude common irq masks && ecc irq masks),*/
                                  /*NULL not enable irq */
    uint32 CommonIrqEnMasks; /*!< common irq enable masks, refer to CAN_COMMON_IRQ_EN_MASKS */
#if defined (AC7840X) || defined (AC7842X)
    boolean BOREC;                  /*!<busoff auto recovery disable*/
    Can_TxSecAmountType TxSecAmount;/*!<CAN tx secondary buf amount*/
    uint32 EccIrqEnMasks; /*!< ecc irq enable masks, Only EccEn Enable need cfg, refer to CAN_ECC_IRQ_EN_MASKS*/
#else
    uint32 DmuIrqEnMasks; /*!< dmu irq enable masks, refer to reg:DMUIE */
    Hal_CallbackType DmuIrqCallback; /*!< DmuIrqEnMasks != 0U must dmu irq callback != NULL */
#endif
} Can_HalConfigType;

#if defined CAN_PERFORMANCE_TEST
typedef struct
{
    uint32 rxAllCount;
    uint32 rxCurrentCount;
    uint32 txAllCount;
    uint32 txCurrentCount;
    uint32 txPeriod0;
    uint32 txPeriod1;
    uint32 txPeriod2;
    uint32 MsgId; /*!< CAN identifier */
    uint32 txPFCnt0;
    uint32 txPFCnt1;
    uint32 txPFCnt2;
    uint64 TimerCount;
    uint32 CountPeriod;
    uint8 abortTask;
    uint64 TimeTxMin;
    uint64 TimeTxMax;
    uint64 TimeTxAvg;
    uint64 TimeRxMin;
    uint64 TimeRxMax;
    uint64 TimeRxAvg;
} Can_PerformaceTestType;
extern Can_PerformaceTestType CanPerformaceTest;
extern uint64 OsIf_GetCurrentTimeUS(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
