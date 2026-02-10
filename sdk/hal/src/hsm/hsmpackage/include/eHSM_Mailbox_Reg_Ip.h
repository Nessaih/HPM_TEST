#ifndef SOC_SEIP_REG_H
#define SOC_SEIP_REG_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
//Add ATC 2025.06.06 
#define HOST2HSM_ACCESS_PATH                    (*(volatile uint32 *)0xE0084018UL)

#define STATUS_BASE                (0x40010000)

#define HSM_STATUS_IN            *((volatile unsigned int *)(STATUS_BASE + 0x60))
#define HSM_STATUS_IN1            *((volatile unsigned int *)(STATUS_BASE + 0x64))

#define MBOX_SOCBASE            (0x400F0000)

#define rSOCMBOX_CMD_D0         *((volatile unsigned int *)(MBOX_SOCBASE + 0x00*4))
#define rSOCMBOX_CMD_D1         *((volatile unsigned int *)(MBOX_SOCBASE + 0x01*4))
#define rSOCMBOX_CMD_D2         *((volatile unsigned int *)(MBOX_SOCBASE + 0x02*4))
#define rSOCMBOX_CMD_D3         *((volatile unsigned int *)(MBOX_SOCBASE + 0x03*4))
#define rSOCMBOX_CMD_D4         *((volatile unsigned int *)(MBOX_SOCBASE + 0x04*4))
#define rSOCMBOX_CMD_D5         *((volatile unsigned int *)(MBOX_SOCBASE + 0x05*4))
#define rSOCMBOX_CMD_D6         *((volatile unsigned int *)(MBOX_SOCBASE + 0x06*4))
#define rSOCMBOX_CMD_D7         *((volatile unsigned int *)(MBOX_SOCBASE + 0x07*4))
#define rSOCMBOX_CMD_D8         *((volatile unsigned int *)(MBOX_SOCBASE + 0x08*4))
#define rSOCMBOX_CMD_D9         *((volatile unsigned int *)(MBOX_SOCBASE + 0x09*4))
#define rSOCMBOX_CMD_D10        *((volatile unsigned int *)(MBOX_SOCBASE + 0x0A*4))
#define rSOCMBOX_CMD_D11        *((volatile unsigned int *)(MBOX_SOCBASE + 0x0B*4))
#define rSOCMBOX_CMD_D12        *((volatile unsigned int *)(MBOX_SOCBASE + 0x0C*4))
#define rSOCMBOX_CMD_D13        *((volatile unsigned int *)(MBOX_SOCBASE + 0x0D*4))
#define rSOCMBOX_CMD_D14        *((volatile unsigned int *)(MBOX_SOCBASE + 0x0E*4))
#define rSOCMBOX_CMD_D15        *((volatile unsigned int *)(MBOX_SOCBASE + 0x0F*4))
#define rSOCMBOX_RSP_D0         *((volatile unsigned int *)(MBOX_SOCBASE + 0x80))
#define rSOCMBOX_RSP_D01        *((volatile unsigned int *)(MBOX_SOCBASE + 0x84))

#define MB_S2H_NOTE                *((volatile unsigned int *)(MBOX_SOCBASE + 0x100))
#define MB_H2S_NOTE                *((volatile unsigned int *)(MBOX_SOCBASE + 0x104))
#define MB_H2S_SOC_INT          *((volatile unsigned int *)(MBOX_SOCBASE + 0x120))//PZG add
#define MB_H2S_SOC_INT_EN       *((volatile unsigned int *)(MBOX_SOCBASE + 0x124))//PZG add
#define MB_HSM_STATUS0            *((volatile unsigned int *)(MBOX_SOCBASE + 0x200))
#define MB_HSM_STATUS1            *((volatile unsigned int *)(MBOX_SOCBASE + 0x210)) // 20230529
#define MB_S2H_SOC_INT          *((volatile unsigned int *)(MBOX_SOCBASE + 0x110))
#define MB_S2H_SOC_INT_EN       *((volatile unsigned int *)(MBOX_SOCBASE + 0x114))

#define MBOX_HOST2HSM_HOST_INT                    *((volatile unsigned int *)(MBOX_SOCBASE + 0x110UL))
#define MBOX_HOST2HSM_HOST_INT_EN                *((volatile unsigned int *)(MBOX_SOCBASE + 0x114UL))
#define MBOX_HOST2HSM_HSM_INT                    *((volatile unsigned int *)(MBOX_SOCBASE + 0x118UL))
#define MBOX_HOST2HSM_HSM_INT_EN                *((volatile unsigned int *)(MBOX_SOCBASE + 0x11CUL))

#define MBOX_HSM2HOST_HOST_INT                    *((volatile unsigned int *)(MBOX_SOCBASE + 0x120UL))
#define MBOX_HSM2HOST_HOST_INT_EN                *((volatile unsigned int *)(MBOX_SOCBASE + 0x124UL))
#define MBOX_HSMHOST_HSM_INT                    *((volatile unsigned int *)(MBOX_SOCBASE + 0x128UL))
#define MBOX_HSMHOST_HSM_INT_EN                 *((volatile unsigned int *)(MBOX_SOCBASE + 0x12CUL))


//#define rSOCMBOX_INT_STA        *((volatile unsigned int *)(MBOX_SOCBASE + 0x11*4))
//#define rSOCMBOX_INT_EN         *((volatile unsigned int *)(MBOX_SOCBASE + 0x12*4))

//#define rSOCMBOX_SOC2SE_STA     *((volatile unsigned int *)(MBOX_SOCBASE + 0x40*4))
//#define rSOCMBOX_SOC2SE_INT     *((volatile unsigned int *)(MBOX_SOCBASE + 0x41*4))
//#define rSOCMBOX_SE2SOC_STA     *((volatile unsigned int *)(MBOX_SOCBASE + 0x42*4))
//#define rSOCMBOX_SE2SOC_INT     *((volatile unsigned int *)(MBOX_SOCBASE + 0x43*4))
//#define rSOCMBOX_SE2SOC_MODE    *((volatile unsigned int *)(MBOX_SOCBASE + 0x44*4))

#define KMU_BASE                   (0xC21D4000) //0x021D_4000~0x021D_7fff
#define rKMU_CTRL               *((volatile unsigned int *)(KMU_BASE +0x00))
#define rKMU_STA                *((volatile unsigned int *)(KMU_BASE +0x01))
#define rKMU_INT_EN             *((volatile unsigned int *)(KMU_BASE +0x02))
#define rVER_D0                 *((volatile unsigned int *)(KMU_BASE +0x04))
#define rVER_D1                 *((volatile unsigned int *)(KMU_BASE +0x05))
#define rVER_D2                 *((volatile unsigned int *)(KMU_BASE +0x06))
#define rVER_D3                 *((volatile unsigned int *)(KMU_BASE +0x07))
#define rSN_D0                  *((volatile unsigned int *)(KMU_BASE +0x08))
#define rSN_D1                  *((volatile unsigned int *)(KMU_BASE +0x09))
#define rATTR_D0                *((volatile unsigned int *)(KMU_BASE +0x0A))
#define rATTR_D1                *((volatile unsigned int *)(KMU_BASE +0x0B))
#define rATTR_D2                *((volatile unsigned int *)(KMU_BASE +0x0C))
#define  KBUF_BASE               (rAHB_BASE + 0x14200 ) //0x021D_4200~0x021D_43ff
#define rKBUF                   *((volatile unsigned int *)(KBUF_BASE +0x00)) //0x021D_4200~0x021D_43ff

#define rERR_ST                *((volatile unsigned int *)(0x021D0000 + (0x0A<<2))) 

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

#endif
