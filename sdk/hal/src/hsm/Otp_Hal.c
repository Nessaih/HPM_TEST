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
 * @file OTP_Hal.c
 *
 * @brief This file provides Hal OTP api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/
#if defined(HSM_HAL_SUPPORT_OTP_INTERFACE)
#include "Hsm_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
#define OTP_CTRL_CODE_UPGRADE_OR_VERIFY 0xFFFFFF00u
#define OTP_CTRL_KEY_DEC_MASK         ((uint32)0x03 << 12)
#define OTP_CTRL_KEY_DEC_ALGO_AES128  ((uint32)0x01<<12)
#define OTP_KEY_LEVEL_MASK              0x000000F0u
#define OTP_KEY_LEVEL_POS               4u
#define OTP_KEY_LEVEL1_TYPE             0xa
#define OTP_KEY_LEVEL2_TYPE             0x5

/* sha256 var*/
typedef struct {
    uint8 Data[64];
    uint32 DataLen;
    uint64 BitLen;
    uint32 State[8];
} Sha256Context;

#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

#define CH(x, y, z)   (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)        (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x)        (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x)       (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x)       (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

static const uint32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

typedef struct {
    uint32 State[8];
    uint32 DataLen;
    uint64 BitLen;
    uint8 Buffer[64];
} Sm3Context;

static uint32 Rotl32(uint32 x, uint32 n)
{
    n &= 31;
    return (x<<n) | (x>>(32 - n));
}

#define Ff0(x, y, z) ((x) ^ (y) ^ (z))
#define Ff1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define Gg0(x, y, z) ((x) ^ (y) ^ (z))
#define Gg1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))

#define P0(x) ((x) ^ Rotl32((x), 9) ^ Rotl32((x), 17))
#define P1(x) ((x) ^ Rotl32((x), 15) ^ Rotl32((x), 23))

static const uint32 TTable[64] = {
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a
};

static const uint8 Sm4Sbox[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7,
    0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
    0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3,
    0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a,
    0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95,
    0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
    0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba,
    0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
    0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b,
    0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2,
    0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
    0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52,
    0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
    0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5,
    0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
    0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55,
    0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
    0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60,
    0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
    0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f,
    0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
    0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f,
    0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
    0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd,
    0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
    0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e,
    0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
    0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20,
    0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
};

static const uint32 FK[4] = {
    0xa3b1bac6U, 0x56aa3350U, 0x677d9197U, 0xb27022dcU
};

static const uint32 CK[32] = {
    0x00070e15U, 0x1c232a31U, 0x383f464dU, 0x545b6269U,
    0x70777e85U, 0x8c939aa1U, 0xa8afb6bdU, 0xc4cbd2d9U,
    0xe0e7eef5U, 0xfc030a11U, 0x181f262dU, 0x343b4249U,
    0x50575e65U, 0x6c737a81U, 0x888f969dU, 0xa4abb2b9U,
    0xc0c7ced5U, 0xdce3eaf1U, 0xf8ff060dU, 0x141b2229U,
    0x30373e45U, 0x4c535a61U, 0x686f767dU, 0x848b9299U,
    0xa0a7aeb5U, 0xbcc3cad1U, 0xd8dfe6edU, 0xf4fb0209U,
    0x10171e25U, 0x2c333a41U, 0x484f565dU, 0x646b7279U
};

static uint8 const AesSbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static uint8  const Rcon[44] = {
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x00, 0x00,
    0x1b, 0x00, 0x00, 0x00,
    0x36, 0x00, 0x00, 0x00
};

static uint32 RoubdKey[32];

/**
 * @brief Specifies the OTP key level.
 * @note DES ID: DES_HSM_TYP_001
 */
typedef enum
{
    HSM_OTP_KEY_LEVEL1 = 1, /**< Key level maybe use HSM firmware */
    HSM_OTP_KEY_LEVEL2 = 2, /**< Key level maybe use host firmware */
}HSM_OtpKeyLevel;

/*============================================FUNCTION PROTOTYPES===================================*/
static void Sha256Transform(Sha256Context *Ctx, const uint8 *Data)
{
    uint32 A, B, C, D, E, F, G, H, T1, T2, M[64];
    sint32 I;

    for (I = 0; I < 16; ++I)
        M[I] = (Data[I * 4] << 24) | (Data[I * 4 + 1] << 16) | (Data[I * 4 + 2] << 8) | (Data[I * 4 + 3]);
    for (; I < 64; ++I)
        M[I] = SIG1(M[I - 2]) + M[I - 7] + SIG0(M[I - 15]) + M[I - 16];

    A = Ctx->State[0];
    B = Ctx->State[1];
    C = Ctx->State[2];
    D = Ctx->State[3];
    E = Ctx->State[4];
    F = Ctx->State[5];
    G = Ctx->State[6];
    H = Ctx->State[7];

    for (I = 0; I < 64; ++I) {
        T1 = H + EP1(E) + CH(E, F, G) + K[I] + M[I];
        T2 = EP0(A) + MAJ(A, B, C);
        H = G;
        G = F;
        F = E;
        E = D + T1;
        D = C;
        C = B;
        B = A;
        A = T1 + T2;
    }

    Ctx->State[0] += A;
    Ctx->State[1] += B;
    Ctx->State[2] += C;
    Ctx->State[3] += D;
    Ctx->State[4] += E;
    Ctx->State[5] += F;
    Ctx->State[6] += G;
    Ctx->State[7] += H;
}

static void Sha256Init(Sha256Context *Ctx)
{
    Ctx->DataLen = 0;
    Ctx->BitLen = 0;
    Ctx->State[0] = 0x6a09e667;
    Ctx->State[1] = 0xbb67ae85;
    Ctx->State[2] = 0x3c6ef372;
    Ctx->State[3] = 0xa54ff53a;
    Ctx->State[4] = 0x510e527f;
    Ctx->State[5] = 0x9b05688c;
    Ctx->State[6] = 0x1f83d9ab;
    Ctx->State[7] = 0x5be0cd19;
}

static void Sha256Update(Sha256Context *Ctx, const uint8 *Input, size_t Len)
{
    for (size_t I = 0; I < Len; ++I) {
        Ctx->Data[Ctx->DataLen] = Input[I];
        Ctx->DataLen++;
        if (Ctx->DataLen == 64) {
            Sha256Transform(Ctx, Ctx->Data);
            Ctx->BitLen += 512;
            Ctx->DataLen = 0;
        }
    }
}

static void Sha256Final(Sha256Context *Ctx, uint8 *Hash)
{
    uint32 I = Ctx->DataLen;

    // Pad
    if (Ctx->DataLen < 56) {
        Ctx->Data[I++] = 0x80;
        while (I < 56)
            Ctx->Data[I++] = 0x00;
    } else {
        Ctx->Data[I++] = 0x80;
        while (I < 64)
            Ctx->Data[I++] = 0x00;
        Sha256Transform(Ctx, Ctx->Data);
        (void)System_Memset(Ctx->Data, 0, 56);
    }

    // Append length
    Ctx->BitLen += Ctx->DataLen * 8;
    Ctx->Data[63] = Ctx->BitLen;
    Ctx->Data[62] = Ctx->BitLen >> 8;
    Ctx->Data[61] = Ctx->BitLen >> 16;
    Ctx->Data[60] = Ctx->BitLen >> 24;
    Ctx->Data[59] = Ctx->BitLen >> 32;
    Ctx->Data[58] = Ctx->BitLen >> 40;
    Ctx->Data[57] = Ctx->BitLen >> 48;
    Ctx->Data[56] = Ctx->BitLen >> 56;
    Sha256Transform(Ctx, Ctx->Data);

    // Output
    for (I = 0; I < 4; ++I) {
        Hash[I]      = (Ctx->State[0] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 4]  = (Ctx->State[1] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 8]  = (Ctx->State[2] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 12] = (Ctx->State[3] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 16] = (Ctx->State[4] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 20] = (Ctx->State[5] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 24] = (Ctx->State[6] >> (24 - I * 8)) & 0x000000ff;
        Hash[I + 28] = (Ctx->State[7] >> (24 - I * 8)) & 0x000000ff;
    }
}

void Hsm_Hal_Sha256HashBySoftWare(const uint8* Msg, uint32 MsgLen, uint8* HashBuf)
{
    Sha256Context Ctx;

    Sha256Init(&Ctx);
    Sha256Update(&Ctx, Msg, MsgLen);
    Sha256Final(&Ctx, HashBuf);
}

static void Sm3Transform(Sm3Context *Ctx, const uint8 Data[64])
{
    uint32 W[68], W1[64];
    uint32 A, B, C, D, E, F, G, H, Ss1, Ss2, Tt1, Tt2;
    sint32 i;

    for (i = 0; i < 16; i++) {
        W[i] = ((uint32)Data[4 * i] << 24) |
               ((uint32)Data[4 * i + 1] << 16) |
               ((uint32)Data[4 * i + 2] << 8) |
               ((uint32)Data[4 * i + 3]);
    }
    for (i = 16; i < 68; i++) {
        W[i] = P1(W[i - 16] ^ W[i - 9] ^ Rotl32(W[i - 3], 15)) ^
               Rotl32(W[i - 13], 7) ^ W[i - 6];
    }
    for (i = 0; i < 64; i++) {
        W1[i] = W[i] ^ W[i + 4];
    }

    A = Ctx->State[0];
    B = Ctx->State[1];
    C = Ctx->State[2];
    D = Ctx->State[3];
    E = Ctx->State[4];
    F = Ctx->State[5];
    G = Ctx->State[6];
    H = Ctx->State[7];

    for (i = 0; i < 64; i++) {
        Ss1 = Rotl32((Rotl32(A, 12) + E + Rotl32(TTable[i], i)) & 0xffffffff, 7);
        Ss2 = Ss1 ^ Rotl32(A, 12);
        Tt1 = ((i < 16) ? Ff0(A, B, C) : Ff1(A, B, C)) + D + Ss2 + W1[i];
        Tt2 = ((i < 16) ? Gg0(E, F, G) : Gg1(E, F, G)) + H + Ss1 + W[i];

        D = C;
        C = Rotl32(B, 9);
        B = A;
        A = Tt1;
        H = G;
        G = Rotl32(F, 19);
        F = E;
        E = P0(Tt2);
    }

    Ctx->State[0] ^= A;
    Ctx->State[1] ^= B;
    Ctx->State[2] ^= C;
    Ctx->State[3] ^= D;
    Ctx->State[4] ^= E;
    Ctx->State[5] ^= F;
    Ctx->State[6] ^= G;
    Ctx->State[7] ^= H;
}

void Sm3Init(Sm3Context *Ctx)
{
    Ctx->DataLen = 0;
    Ctx->BitLen = 0;
    Ctx->State[0] = 0x7380166f;
    Ctx->State[1] = 0x4914b2b9;
    Ctx->State[2] = 0x172442d7;
    Ctx->State[3] = 0xda8a0600;
    Ctx->State[4] = 0xa96f30bc;
    Ctx->State[5] = 0x163138aa;
    Ctx->State[6] = 0xe38dee4d;
    Ctx->State[7] = 0xb0fb0e4e;
}

static void Sm3Update(Sm3Context *Ctx, const uint8 *Input, size_t Len)
{
    for (size_t i = 0; i < Len; i++) {
        Ctx->Buffer[Ctx->DataLen++] = Input[i];
        if (Ctx->DataLen == 64) {
            Sm3Transform(Ctx, Ctx->Buffer);
            Ctx->BitLen += 512;
            Ctx->DataLen = 0;
        }
    }
}

static void Sm3Final(Sm3Context *Ctx, uint8 Hash[32])
{
    size_t i = Ctx->DataLen;

    Ctx->Buffer[i++] = 0x80;
    if (i > 56) {
        while (i < 64) Ctx->Buffer[i++] = 0x00;
        Sm3Transform(Ctx, Ctx->Buffer);
        i = 0;
    }
    while (i < 56) Ctx->Buffer[i++] = 0x00;

    Ctx->BitLen += Ctx->DataLen * 8;
    for (uint32 j = 0U; j < 8U; j++) {
        Ctx->Buffer[63U - j] = Ctx->BitLen >> (8U * j);
    }

    Sm3Transform(Ctx, Ctx->Buffer);

    for (i = 0; i < 8; i++) {
        Hash[4 * i + 0] = (Ctx->State[i] >> 24) & 0xff;
        Hash[4 * i + 1] = (Ctx->State[i] >> 16) & 0xff;
        Hash[4 * i + 2] = (Ctx->State[i] >> 8) & 0xff;
        Hash[4 * i + 3] = (Ctx->State[i]) & 0xff;
    }
}

void Hsm_Hal_Sm3HashBySoftWare(const uint8* Msg, uint32 MsgLen, uint8* HashBuf)
{
    Sm3Context Ctx;

    Sm3Init(&Ctx);
    Sm3Update(&Ctx, Msg, MsgLen);
    Sm3Final(&Ctx, HashBuf);
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_100
 * @return op status
 */
static uint32 HSM_Hal_RotL(uint32 XBytes, uint32 NBytes)
{
    return (XBytes << NBytes) | (XBytes >> (32U - NBytes));
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_101
 * @return op status
 */
static uint32 HSM_Hal_Tau(uint32 InBytes)
{
    uint8 TempBytes[4];
    TempBytes[0] = Sm4Sbox[(InBytes >> 24U)];
    TempBytes[1] = Sm4Sbox[(InBytes >> 16U) & 0xFFU];
    TempBytes[2] = Sm4Sbox[(InBytes >> 8U) & 0xFFU];
    TempBytes[3] = Sm4Sbox[InBytes & 0xFFU];
    return ((uint32)TempBytes[0] << 24U) | ((uint32)TempBytes[1] << 16U) |
           ((uint32)TempBytes[2] << 8U) | ((uint32)TempBytes[3]);
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_102
 * @return op status
 */
static uint32 HSM_Hal_L1(uint32 InBytes)
{
    return InBytes ^ HSM_Hal_RotL(InBytes, 2) ^ HSM_Hal_RotL(InBytes, 10) ^
                HSM_Hal_RotL(InBytes, 18) ^ HSM_Hal_RotL(InBytes, 24);
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_103
 * @return op status
 */
static uint32 HSM_Hal_L2(uint32 InBytes)
{
    return InBytes ^ HSM_Hal_RotL(InBytes, 13) ^ HSM_Hal_RotL(InBytes, 23);
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_104
 * @return op status
 */
static void HSM_Hal_Sm4SetKey(const uint8 *KeyBuf)
{
    uint32 K[36];
    for (uint8 i = 0; i < 4u; i++)
    {
        K[i] = ((uint32)KeyBuf[4u*i] << 24u) | ((uint32)KeyBuf[(4u*i)+1u] << 16u) |
               ((uint32)KeyBuf[(4u*i)+2u] << 8u) | ((uint32)KeyBuf[(4u*i)+3u]);
        K[i] ^= FK[i];
    }

    for (uint8 i = 0; i < 32U; i++)
    {
        uint32 tmp = K[i+1U] ^ K[i+2U] ^ K[i+3U] ^ CK[i];
        K[i+4U] = K[i] ^ HSM_Hal_L2(HSM_Hal_Tau(tmp));
        RoubdKey[i] = K[i+4U];
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_105
 * @return op status
 */
static void HSM_Hal_Sm4EncBySoftWare(const uint8 *InBuf, uint8 *OutBuf)
{
    uint32 X[36];
    uint8 i;
    uint32 TempBytes;
    for (i = 0; i < 4U; i++)
    {
        X[i] = ((uint32)InBuf[4U*i] << 24U) | ((uint32)InBuf[(4U*i)+1U] << 16U) |
               ((uint32)InBuf[(4U*i)+2U] << 8U) | ((uint32)InBuf[(4U*i)+3U]);
    }

    for (i = 0; i < 32U; i++)
    {
        TempBytes = X[i+1U] ^ X[i+2U] ^ X[i+3U] ^ RoubdKey[i];
        X[i+4U] = X[i] ^ HSM_Hal_L1(HSM_Hal_Tau(TempBytes));
    }

    for (i = 0; i < 4U; i++)
    {
        TempBytes = X[35U - i];
        OutBuf[4U*i] = (uint8)(TempBytes >> 24U);
        OutBuf[(4U*i)+1U] = (uint8)((TempBytes >> 16U) & 0xFFU);
        OutBuf[(4U*i)+2U] = (uint8)((TempBytes >> 8U) & 0xFFU);
        OutBuf[(4U*i)+3U] = (uint8)(TempBytes & 0xFFU);
    }
}

/*!
 * @brief Sm4 CBC encrypt by software.
 * @note  Function ID: DES_HSM_API_118
 * @return op status
 */
static void HSM_Hal_Sm4CBCEncBySoftWare(const uint8 *InBuf, uint8 *OutBuf, size_t InBufLen,
                const uint8 *KeyBuf, uint8 *Iv)
{
    uint8 Block[16];
    HSM_Hal_Sm4SetKey(KeyBuf);
    for (size_t i = 0; i < InBufLen; i += 16U)
    {
        for (uint8 j = 0; j < 16U; j++)
        {
            Block[j] = InBuf[i + j] ^ Iv[j];
        }
        HSM_Hal_Sm4EncBySoftWare(Block, &OutBuf[i]);
        (void)System_Memcpy(Iv, &OutBuf[i], 16U);
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_106
 * @return op status
 */
static uint8 *HSM_Hal_RotWord(uint8 *Word)
{
    uint8 Tmp[4];
   (void) (void)System_Memcpy(Tmp, Word, 4U);
    for (uint8 i = 0; i < 4U; i++)
    {
        Word[i] = Tmp[(i + 1U) % 4U];
    }
    return Word;
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_107
 * @return op status
 */
static uint8 *HSM_Hal_SubWord(uint8 *Word)
{
    for (uint8 i = 0; i < 4U; i++)
    {
        Word[i] = AesSbox[Word[i]];
    }
    return Word;
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_108
 * @return op status
 */
static uint8 HSM_Hal_Mul(uint8 A, uint8 B)
{
    uint8 TempBytesb[4];
    uint8 Out = 0;
    TempBytesb[0] = B;
    for (uint8 i = 1; i < 4U; i++)
    {
        TempBytesb[i] = TempBytesb[i - 1U] << 1U;
        if ((TempBytesb[i - 1U] & 0x80U) != 0U) {
            TempBytesb[i] ^= 0x1bU;
        }
    }
    for (uint8 i = 0; i < 4U; i++)
    {
        if (((A >> i) & 0x01U) != 0U) {
            Out ^= TempBytesb[i];
        }
    }
    return Out;
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_109
 * @return op status
 */
static void HSM_Hal_MixColumns(uint8 State[][4])
{
    uint8 Tmp[4];
    uint8 const MatMul[4][4] = {{0x02, 0x03, 0x01, 0x01}, {0x01, 0x02, 0x03, 0x01},
                    {0x01, 0x01, 0x02, 0x03},{0x03, 0x01, 0x01, 0x02}};
    for (uint8 col = 0U; col < 4U; col++)
    {
        for (uint8 row = 0U; row < 4U; row++)
        {
            Tmp[row] = State[col][row];
        }
        for (uint8 i = 0U; i < 4U; i++)
        {
            State[col][i] = 0x00U;
            for (uint8 j = 0U; j < 4U; j++)
            {
                State[col][i] ^= HSM_Hal_Mul(MatMul[i][j], Tmp[j]);
            }
        }
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_110
 * @return op status
 */
static void HSM_Hal_BlockXor(uint8 *Dst, const uint8 *A, const uint8 *B)
{
    for (uint8 i = 0; i < 16U; i++)
    {
        Dst[i] = A[i] ^ B[i];
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_111
 * @return op status
 */
static void HSM_Hal_AddRoundKey(uint8 State[][4], const uint8 *Key)
{
    for (uint8 row = 0; row < 4U; row++)
    {
        for (uint8 col = 0; col < 4U; col++)
        {
            State[col][row] ^= Key[(col * 4U) + row];
        }
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_112
 * @return op status
 */
static void HSM_Hal_SubBytes(uint8 State[][4])
{
    for (uint8 row = 0U; row < 4U; row++) {
        for (uint8 col = 0U; col < 4U; col++) {
            State[col][row] = AesSbox[State[col][row]];
        }
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_113
 * @return op status
 */
static void HSM_Hal_ShiftRows(uint8 State[][4])
{
    uint8 Tmp[4];
    for (uint8 row = 1; row < 4U; row++){
        for (uint8 col = 0; col < 4U; col++) {
            Tmp[col] = State[(row + col) % 4U][row];
        }
        for (uint8 col = 0; col < 4U; col++) {
            State[col][row] = Tmp[col];
        }
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_114
 * @return op status
 */
static void HSM_Hal_KeyExpansion(const uint8 *Key, uint8 *W, uint8 Nk, uint8 Nr)
{
    uint8 Tmp[4];
    (void)System_Memcpy(W, Key, 4U * (uint32)Nk);

    for (uint8 i = 4U * Nk; i <((4U * (Nr + 1U)) * 4U); i += 4U)
    {
        (void)System_Memcpy(Tmp, &W[i-4U], 4U);
        if ((i % (Nk * 4U)) == 0U)
        {
            (void)HSM_Hal_SubWord(HSM_Hal_RotWord(Tmp));
            for (uint8 j = 0U; j < 4U; j++)
            {
                Tmp[j] ^= Rcon[(i / Nk) + j];
            }
        }
        else if ((Nk > 6U) && ((i % (Nk * 4U)) == 16U))//PRQA S 2004 ++ #else
        {
            (void)HSM_Hal_SubWord(Tmp);
        }

        for (uint8 j = 0U; j < 4U; j++)
        {
           /* PRQA S 2853 ++ #Implicit conversion to a signed integer type of insufficient size. */
            W[i + j] = W[(i - (Nk * 4U)) + j] ^ Tmp[j];
           /* PRQA S 2853 -- #Implicit conversion to a signed integer type of insufficient size. */
        }
    }
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_115
 * @return op status
 */
static void HSM_Hal_Cipher(const uint8 *InBuf, uint8 *OutBuf, const uint8 *W, uint8 Nk, uint8 Nr)
{
    uint8 State[4][4];
    (void)System_Memcpy(&State[0][0], InBuf, 4U * (uint32)Nk);

    HSM_Hal_AddRoundKey(State, W);
    for (uint8 rod = 0; rod < Nr; rod++)
    {
        HSM_Hal_SubBytes(State);
        HSM_Hal_ShiftRows(State);
        if (rod != (Nr - 1u))
        {
            HSM_Hal_MixColumns(State);
        }
        HSM_Hal_AddRoundKey(State, (const uint8 *)(&W [((rod + 1u) * 16U)]));
    }
    (void)System_Memcpy(OutBuf,&State[0][0], ((uint32)Nk * 4u));
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_116
 * @return op status
 */
static void HSM_Hal_AesEncBySoftWare(const uint8 *InBuf, uint8 *OutBuf, const uint8 *Key)
{
    uint8 Nk = 4, Nr = 10;

    uint8 W[176] = {0};

    HSM_Hal_KeyExpansion(Key, W, Nk, Nr);
    HSM_Hal_Cipher(InBuf, OutBuf, W, Nk, Nr);
}

/*!
 * @brief AES128 CBC encrypt by software.
 * @note  Function ID: DES_HSM_API_117
 * @return op status
 */
static void HSM_Hal_AesCBCEncBySoftWare(const uint8* InBuf, uint8* OutBuf,
            uint32 InBufLen, const uint8* Key, const uint8 *Iv)
{
    uint8 BlockInBuf[16], BlockOutBuf[16];
    uint8 LocalIv[16] = {0};

    (void)System_Memcpy(LocalIv, Iv, 16);
    for (uint32 i=0U; i<InBufLen; i += 16U)
    {
        HSM_Hal_BlockXor(BlockInBuf, &InBuf[i], LocalIv);
        HSM_Hal_AesEncBySoftWare(BlockInBuf, BlockOutBuf, Key);
        (void)System_Memcpy(&OutBuf[i], BlockOutBuf, 16U);
        (void)System_Memcpy(LocalIv, BlockOutBuf, 16U);
    }
}

/*!
 * @brief crc function for MPEG-2 standard.
 * @note  Function ID: DES_HSM_API_119
 * @param[in] InBufAddr: Input data buffer.
 * @param[in] InBufLen: Input data buffer length.
 * @return Crc value
 */
static uint32 HSM_Hal_Crc32Mpeg2(const uint8 *InBufAddr, uint32 InBufLen)
{
    uint32 i, j;
    uint32 CrcValue = 0xFFFFFFFFU;
    uint32 TemLen = InBufLen;
    uint32 TempValue;

    for (j=0U; j < TemLen; j++)
    {
        TempValue = InBufAddr[j];
        /*PRQA S 3387 ++ #allow volatile modified variables to be ++ or --.*/
        CrcValue = CrcValue ^ (TempValue << 24U);
        /*PRQA S 3387 -- #allow volatile modified variables to be ++ or --.*/
        for (i = 0; i < 8U; i++)
        {
            if ((CrcValue & 0x80000000U) != 0U)
           {
                CrcValue = (CrcValue << 1U) ^ (0x04C11DB7U);
           }
           else
          {
                CrcValue <<= 1;
          }
        }
    }

    return CrcValue;
}

static Hal_StatusType HSM_Hal_SetOtpCtrl(HSM_OtpCtrlUpgradeAlgoType UpgradeAlgo,
        HSM_OtpCtrlVerifyAlgoType VerifyAlgo, HSM_ImageType Type)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint32 CtrlAddr = 0u;
    uint32 CtrlVal = 0u;
    if (Type == HSM_IMAGE_HSM_TYPE)
    {
        CtrlAddr = (uint32)OTP_FW_CTRL_FIELD_ADDR_H;
    }
    else if(Type == HSM_IMAGE_HOST_TYPE)
    {
        CtrlAddr = (uint32)OTP_HOST_CTRL_FIELD_ADDR_L;
    }
    Ret = HSM_Hal_OtpRead(CtrlAddr, 4U, (uint8 *)&CtrlVal);
    if (Ret == STATUS_SUCCESS)
    {
        if (CtrlVal == 0xFFFFFFFF)
        {
            CtrlVal = OTP_CTRL_CODE_UPGRADE_OR_VERIFY |
                    (UpgradeAlgo << OTP_CTRL_CODE_UPGRADE_POS) | (VerifyAlgo << OTP_CTRL_CODE_VERIFY_POS);
            Ret = HSM_Hal_OtpWrite(CtrlAddr, 4U, (uint8 *)&CtrlVal);
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }

    return Ret;
}

static Hal_StatusType HSM_Hal_GetRndKey(HSM_OtpKeyLevel Level, const uint8 *KeyOut)
{
    Hal_StatusType Ret;
    uint32 Error = 0U;

    if (KeyOut != NULL_PTR)
    {
        Error = HSM_Reg_GetRandKey((uint8)Level, HSM_RND_OTP_KEY_SYM, KeyOut);
        if (Error != HSM_SUCCESS)
        {
            Ret = HSM_WRONG_GET_RND_KEY_ERROR;
        }
        else
        {
            Ret = STATUS_SUCCESS;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief adjust otp key is valid or not
 * @note  Function ID: DES_HSM_API_143
 * @param [in] KeyId: keyindex index.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static boolean HSM_Hal_IsValidOtpKeyId(HSM_OtpKeyId KeyId)
{
    uint8 TempBuf[40] = {0};
    Hal_StatusType Ret;
    boolean Flag = TRUE;

    Ret = HSM_Hal_OtpRead((uint32)OTP_KEY_ADDR((uint32)KeyId), 40U, TempBuf);
    if (Ret == STATUS_SUCCESS)
    {
        for(uint8 i = 0; i<40U; i++)
        {
            if(TempBuf[i] != 0xFFU)
            {
                Flag = FALSE;
                break;
            }
        }
    }
    else
    {
        Flag = FALSE;
    }

    return Flag;
}

static Hal_StatusType HSM_Hal_EncryptKey(HSM_OtpKeyLevel Level, const uint8 *KeyIn, uint32 KeySize, const uint8 *KeyOut)
{
    Hal_StatusType Ret;
    uint32 Error = 0U;

    if (KeyOut != NULL_PTR)
    {
        Error = HSM_Reg_EncryptKey((uint8)Level, KeyIn, KeySize, KeyOut);
        if (Error != HSM_SUCCESS)
        {
            Ret = HSM_WRONG_ENCRYPT_KEY_ERROR;
        }
        else
        {
            Ret = STATUS_SUCCESS;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

static Hal_StatusType HSM_Hal_EncryptKeyByHsmFw(const uint8 *KeyIn, uint32 KeySize,
                HSM_OtpKeyId KeyId, HSM_ExternOtpKeyType KeyType)
{
    Hal_StatusType Ret;
    uint32 Error = 0U;
    uint32 KeySlot = 0u;

    if (KeyIn != NULL_PTR)
    {
        if ((KeyId == HOST_DEBUG_KEY)|| (KeyId == HOST_FW_VERIFY_KEY)
        || (KeyId == HOST_UPGRADE_ENCRYPT_KEY) || (KeyId == HOST_UPGRADE_VERIFY_KEY) || (KeyId == USER_AUTH_KEY))
        {
            if(HSM_Hal_IsValidOtpKeyId(KeyId) != 0U)
            {
                HSM_LifeCycleType Lc = HSM_Hal_GetLifeCycle();

                if ((HSM_LIFE_CYCLE_DEV_MODE == Lc) || (HSM_LIFE_CYCLE_MANU_MODE == Lc))
                {
                    switch (KeyId)
                    {
                    case HOST_DEBUG_KEY:
                        KeySlot = 1;
                        break;
                    case HOST_FW_VERIFY_KEY:
                        KeySlot = 2;
                        break;
                    case HOST_UPGRADE_ENCRYPT_KEY:
                        KeySlot = 4;
                        break;
                    case HOST_UPGRADE_VERIFY_KEY:
                        KeySlot = 8;
                        break;
                    case USER_AUTH_KEY:
                        KeySlot = 0x10;
                        break;
                    default:
                        Ret = STATUS_ERROR;
                        break;
                    }
                    Error = HSM_Reg_EncryptKeyByHsmFw(KeyIn, KeySize, KeySlot, KeyType);
                    if (Error != HSM_SUCCESS)
                    {
                        Ret = HSM_WRONG_ENCRYPT_KEY_ERROR;
                    }
                    else
                    {
                        Ret = STATUS_SUCCESS;
                    }
                }
                else
                {
                    Ret = STATUS_ERROR;
                }
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief Otp Hw Ctrl
 *
 * @param[in] HostVerifyPara: Pointer to host image verify parameter
 * @return operate status
 */
Hal_StatusType HSM_Hal_SetOtpKeyCipherAlgo(HSM_OtpKeyCipherAlgo Algo)
{
    Hal_StatusType Ret;
    uint32 KeyCipherAlgo = 0;
    Ret = HSM_Hal_OtpRead((uint32)OTP_HW_CTRL_FIELD_ADDR, 4U, (uint8 *)&KeyCipherAlgo);
    if (Ret == STATUS_SUCCESS)
    {
        if (KeyCipherAlgo == 0xFFFFFFFFU)
        {
            if ((Algo == OTP_KEY_CIPHER_AES) || (Algo == OTP_KEY_CIPHER_SM4))
            {
                if (Algo == OTP_KEY_CIPHER_AES)
                {
                    KeyCipherAlgo = (uint32)OTP_KEY_CIPHER_AES_CIPHER;
                }
                else
                {
                    KeyCipherAlgo = (uint32)OTP_KEY_CIPHER_SM4_CIPHER;
                }

                Ret = HSM_Hal_OtpWrite((uint32)OTP_HW_CTRL_FIELD_ADDR, 4U, (uint8 *)&KeyCipherAlgo);
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }

    return Ret;
}

/*!
 * @brief Set Otp Hw Ctrl for HSM
 *
 * @param[in] HostVerifyPara: Pointer to host image verify parameter
 * @return operate status
 */
Hal_StatusType HSM_Hal_SetHsmOtpCtrl(HSM_OtpCtrlUpgradeAlgoType UpgradeAlgo, HSM_OtpCtrlVerifyAlgoType VerifyAlgo)
{
    Hal_StatusType Ret;

    Ret = HSM_Hal_SetOtpCtrl(UpgradeAlgo, VerifyAlgo,  HSM_IMAGE_HSM_TYPE);

    return Ret;
}

/*!
 * @brief Set Otp Hw Ctrl for Soc
 *
 * @param[in] HostVerifyPara: Pointer to host image verify parameter
 * @return operate status
 */
Hal_StatusType HSM_Hal_SetHostOtpCtrl(HSM_OtpCtrlUpgradeAlgoType UpgradeAlgo, HSM_OtpCtrlVerifyAlgoType VerifyAlgo)
{
    Hal_StatusType Ret;

    Ret = HSM_Hal_SetOtpCtrl( UpgradeAlgo, VerifyAlgo,  HSM_IMAGE_HOST_TYPE);

    return Ret;
}

Hal_StatusType HSM_Hal_SetOtpRndKey(HSM_OtpKeyId KeyId, uint32 KeyAttr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 KeyBuf[40] = {0};
    HSM_OtpKeyLevel KeyLevel = HSM_OTP_KEY_LEVEL1;
    if ((KeyId == CHIP_ROOT_KEY) || (KeyId == DEVICE_ROOT_KEY)
    || (KeyId == USER_ROOT_KEY) || (KeyId == HSM_FW_VERIFY_KEY)
    || (KeyId == HOST_FW_VERIFY_KEY) || (KeyId == HOST_UPGRADE_VERIFY_KEY)
    || (KeyId == HSM_ENCRYPT_KEY) || (KeyId == HSM_SECRET_KEY) || (KeyId == HSM_PRIVATE_KEY)
    || (KeyId == HOST_ENCRYPT_KEY) || (KeyId == HOST_PRIVATE_KEY))
    {
        if (((KeyAttr & OTP_KEY_LEVEL_MASK) >> OTP_KEY_LEVEL_POS) == OTP_KEY_LEVEL1_TYPE)
        {
            KeyLevel = HSM_OTP_KEY_LEVEL1;
        }
        else if (((KeyAttr & OTP_KEY_LEVEL_MASK) >> OTP_KEY_LEVEL_POS) == OTP_KEY_LEVEL2_TYPE)
        {
            KeyLevel = HSM_OTP_KEY_LEVEL2;
        }
        else
        {
            Ret = STATUS_ERROR;
        }

        if (Ret == STATUS_SUCCESS)
        {
            Ret = HSM_Hal_GetRndKey(KeyLevel, (const uint8 *)&KeyBuf[4]);
            if(Ret == STATUS_SUCCESS)
            {
                /*PRQA S 3305 ++ #make sure there are no alignment issues.*/
                ((uint32 *)KeyBuf)[0] = KeyAttr;
                /*PRQA S 3305 -- #make sure there are no alignment issues.*/
                if(HSM_Hal_IsValidOtpKeyId(KeyId) != 0U)
                {
                    Ret = HSM_Hal_OtpWrite((uint32)OTP_KEY_ADDR((uint32)KeyId), 40U, KeyBuf);
                }
                else
                {
                    Ret = STATUS_ERROR;
                }
            }
        }
    }
    else
    {
        Ret = HSM_WRONG_KEY_HANDLE;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_SetOtpRndKeyByHsmFw(HSM_OtpKeyId KeyId)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 KeySlot = 0u;

    if ((KeyId == DEVICE_ROOT_KEY)|| (KeyId == USER_ROOT_KEY)
    || (KeyId == HOST_FW_VERIFY_KEY) || (KeyId == HOST_ENCRYPT_KEY) || (KeyId == HOST_PRIVATE_KEY))
    {
        if(HSM_Hal_IsValidOtpKeyId(KeyId) != 0U)
        {
            HSM_LifeCycleType Lc = HSM_Hal_GetLifeCycle();

            if ((HSM_LIFE_CYCLE_DEV_MODE == Lc) || (HSM_LIFE_CYCLE_MANU_MODE == Lc))
            {
                switch (KeyId)
                {
                case DEVICE_ROOT_KEY:
                    KeySlot = 1;
                    break;
                case HOST_FW_VERIFY_KEY:
                    KeySlot = 2;
                    break;
                case HOST_ENCRYPT_KEY:
                    KeySlot = 4;
                    break;
                case HOST_PRIVATE_KEY:
                    KeySlot = 8;
                    break;
                case USER_ROOT_KEY:
                    KeySlot = 10;
                    break;
                default:
                    Ret = STATUS_ERROR;
                    break;
                }
                uint32 RetCode = HSM_Reg_GetRandKeyByHsmFw(KeySlot, (uint32)HSM_RND_OTP_KEY_SYM);
                if (HSM_SUCCESS == RetCode)
                {
                    Ret = STATUS_SUCCESS;
                }
                else
                {
                    Ret = HSM_WRONG_OTP_WRITE_ERROR;
                }
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }
    else
    {
        Ret = HSM_WRONG_KEY_HANDLE;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_SetOtpExternalKey(HSM_OtpKeyId KeyId, const uint8 *ExternKey, uint32 ExternKeyLen,
                                    uint32 KeyAttr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    /*RTL Key fix in chip. never to change it.*/
    uint8 const HSM_KEK_KEY[16] = {0x02,0xEC,0x7F,0xAB,0x2B,0xBC,0x46,0x98,0x67,0xB9,0xAB,0x33,0x31,0x46,0x0A,0x1E};
    uint8 const SOC_KEK_KEY[16] = {0x5C,0x75,0xFA,0x87,0x53,0xEF,0x74,0x16,0x97,0xA3,0xEA,0xC4,0xEB,0x2C,0x6E,0xC4};
    uint8 KeyBuf[40] = {0};
    uint8 EncryptKeyBuf[48] = {0};
    uint8 EncryptKeyBufCipher[48] = {0};
    uint8 Iv[16] = {0};
    uint32 EncAlgo = 0;
    uint32 Crc;
    uint8 const *Key;
    HSM_OtpKeyLevel KeyLevel = HSM_OTP_KEY_LEVEL1;

    if (((KeyAttr & OTP_KEY_LEVEL_MASK) >> OTP_KEY_LEVEL_POS) == OTP_KEY_LEVEL1_TYPE)
    {
        KeyLevel = HSM_OTP_KEY_LEVEL1;
    }
    else if (((KeyAttr & OTP_KEY_LEVEL_MASK) >> OTP_KEY_LEVEL_POS) == OTP_KEY_LEVEL2_TYPE)
    {
        KeyLevel = HSM_OTP_KEY_LEVEL2;
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    if ((ExternKeyLen == 32U) && (Ret == STATUS_SUCCESS))
    {
        /* set attribute */
        /*PRQA S 3305 ++ #make sure there are no alignment issues.*/
        ((uint32 *)KeyBuf)[0] = KeyAttr;
        /*PRQA S 3305 -- #make sure there are no alignment issues.*/

        if(HSM_OTP_KEY_LEVEL1 == KeyLevel)
        {
            Key = HSM_KEK_KEY;
        }
        else
        {
            Key = SOC_KEK_KEY;
        }
        if (ExternKey != NULL_PTR)
        {
            Ret = HSM_Hal_OtpRead(OTP_HW_CTRL_FIELD_ADDR, 4, (uint8 *)&EncAlgo);
            if (Ret == STATUS_SUCCESS)
            {
                Crc = HSM_Hal_Crc32Mpeg2(ExternKey, 32);
                (void)System_Memcpy(EncryptKeyBuf, ExternKey, 32);
                (void)System_Memcpy(&EncryptKeyBuf[32U], (const uint8 *)&Crc, 4U);
                (void)System_Memset(&EncryptKeyBuf[36U], 0, 12U);
                if ((EncAlgo & OTP_CTRL_KEY_DEC_MASK) == OTP_CTRL_KEY_DEC_ALGO_AES128)
                {
                    /*AES CBC encrypt extern key*/
                    HSM_Hal_AesCBCEncBySoftWare(EncryptKeyBuf, EncryptKeyBufCipher, 48, Key, Iv);
                }
                else
                {
                    /*SM4 CBC encrypt extern key*/
                    HSM_Hal_Sm4CBCEncBySoftWare(EncryptKeyBuf, EncryptKeyBufCipher, 48, Key, Iv);
                }
                /*send cipher key to hsm, hsm will decrypt this and again encrypt  use root key*/
                Ret = HSM_Hal_EncryptKey(KeyLevel, EncryptKeyBufCipher, 48, &KeyBuf[4]);
                if(Ret == STATUS_SUCCESS)
                {
                    /* if otp slot value is not 0xff, will not write key to it*/
                    if (HSM_Hal_IsValidOtpKeyId(KeyId) != 0U)
                    {
                        //need use cmd , no dependence hsm
                        Ret = HSM_Hal_OtpWrite((uint32)OTP_KEY_ADDR((uint32)KeyId), 40U, KeyBuf);
                    }
                    else
                    {
                        Ret = STATUS_ERROR;
                    }
                }
            }
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_SetOtpExternalKeyByHsmFw(HSM_OtpKeyId KeyId, const uint8 *ExternKey,
                                                    HSM_ExternOtpKeyType KeyType)
{
    Hal_StatusType Ret;
    /*RTL Key fix in chip. never to change it.*/
    uint8 const SOC_KEK_KEY[16] = {0x5C,0x75,0xFA,0x87,0x53,0xEF,0x74,0x16,0x97,0xA3,0xEA,0xC4,0xEB,0x2C,0x6E,0xC4};
    uint8 EncryptKeyBuf[48] = {0};
    uint8 EncryptKeyBufCipher[48] = {0};
    uint8 Iv[16] = {0};
    uint32 EncAlgo = 0;
    uint32 Crc;

    if (ExternKey != NULL_PTR)
    {
        Ret = HSM_Hal_OtpRead(OTP_HW_CTRL_FIELD_ADDR, 4, (uint8 *)&EncAlgo);
        if (Ret == STATUS_SUCCESS)
        {
            Crc = HSM_Hal_Crc32Mpeg2(ExternKey, 32);
            (void)System_Memcpy(EncryptKeyBuf, ExternKey, 32);
            (void)System_Memcpy(&EncryptKeyBuf[32U], (const uint8 *)&Crc, 4U);
            (void)System_Memset(&EncryptKeyBuf[36U], 0, 12U);
            if ((EncAlgo & OTP_CTRL_KEY_DEC_MASK) == OTP_CTRL_KEY_DEC_ALGO_AES128)
            {
                /*AES CBC encrypt extern key*/
                HSM_Hal_AesCBCEncBySoftWare(EncryptKeyBuf, EncryptKeyBufCipher, 48, SOC_KEK_KEY, Iv);
            }
            else
            {
                /*SM4 CBC encrypt extern key*/
                HSM_Hal_Sm4CBCEncBySoftWare(EncryptKeyBuf, EncryptKeyBufCipher, 48, SOC_KEK_KEY, Iv);
            }
            /* write key */
            Ret = HSM_Hal_EncryptKeyByHsmFw(EncryptKeyBufCipher, 48, KeyId, KeyType);
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}
#endif
