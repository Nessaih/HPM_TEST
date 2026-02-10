#ifndef TEST_INIT_CONFIG_H
#define TEST_INIT_CONFIG_H

#ifdef __cplusplus
extern "C"{
#endif

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

#ifdef CONFIG_EHSM_UNIT_TEST

    //#define CONFIG_EHSM_PTEST
    #ifndef CONFIG_EHSM_PTEST
		//#define CONFIG_EHSM_UNIT_TEST_CRYPTO_LIB

        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_CMD
        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_ASYM
        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_HASH
        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_MAC
        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_SYM
        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_K
        //#define CONFIG_EHSM_UNIT_TEST_AUTOSAR_RANDOM
        // #define CONFIG_EHSM_UNIT_TEST_AUTOSAR_CANCEL  // TODO: Test on RTT OS

//        #define CONFIG_EHSM_UNIT_TEST_SHE_AES
//        #define CONFIG_EHSM_UNIT_TEST_SHE_COMPRESS
//        #define CONFIG_EHSM_UNIT_TEST_SHE_MAC
//        #define CONFIG_EHSM_UNIT_TEST_SHE_AUTH
//        #define CONFIG_EHSM_UNIT_TEST_SHE_CMD
//        #define CONFIG_EHSM_UNIT_TEST_SHE_K
//        #define CONFIG_EHSM_UNIT_TEST_SHE_RETURN
//        #define CONFIG_EHSM_UNIT_TEST_SHE_RNG
//        #define CONFIG_EHSM_UNIT_TEST_SHE_SECURE_BOOT
//        #define CONFIG_EHSM_UNIT_TEST_SHE_STORAGE
        // #define CONFIG_EHSM_UNIT_TEST_SHE_CANCEL      // TODO: Test on RTT OS

//        #define CONFIG_EHSM_UNIT_TEST_EVITA_ASYM
//        #define CONFIG_EHSM_UNIT_TEST_EVITA_HASH
//        #define CONFIG_EHSM_UNIT_TEST_EVITA_COUNTER
//        #define CONFIG_EHSM_UNIT_TEST_EVITA_K
        #define CONFIG_EHSM_UNIT_TEST_EVITA_SYM
//        #define CONFIG_EHSM_UNIT_TEST_EVITA_RNG
        //#define CONFIG_EHSM_UNIT_TEST_EVITA_TIMER

        //#define CONFIG_EHSM_UNIT_TEST_CUSTOM_DEBUG_AUTH
        //#define CONFIG_EHSM_UNIT_TEST_JTAG_DEBUG_AUTH
        //#define CONFIG_EHSM_UNIT_TEST_FW_K_INSTALL
        //#define CONFIG_EHSM_UNIT_SYS_MGR
        //#define CONFIG_EHSM_UNIT_TEST_OTP
        //#define CONFIG_EHSM_UNIT_TEST_SENSOR_ALARM_RSP
        //#define CONFIG_EHSM_UNIT_FAST_CMAC
        //#define CONFIG_EHSM_UNIT_TEST_SM9_K
        //#define CONFIG_EHSM_UNIT_TEST_CUSTOM_BOOT
        //#define CONFIG_EHSM_UNIT_TEST_CUSTOM_EXTEND
        //#define CONFIG_EHSM_UNIT_TEST_CUSTOM_UPGRADE

        // !!! Donot open in normal test !!!
        //#define CONFIG_EHSM_UNIT_TEST_CUSTOM_LIFECYCLE

    #else // ifdef CONFIG_EHSM_PTEST
        #define CONFIG_EHSM_PTEST_TIME_COUNTING_SELF_TEST
        //#define CONFIG_EHSM_PTEST_FAST_CMAC
        #define CONFIG_EHSM_PTEST_ASR_RANDOM
        #define CONFIG_EHSM_PTEST_ASR_HASH
        #define CONFIG_EHSM_PTEST_ASR_SYM
        #define CONFIG_EHSM_PTEST_ASR_MAC
        #define CONFIG_EHSM_PTEST_ASR_ASYM
        //#define CONFIG_EHSM_PTEST_SM9
        //#define CONFIG_EHSM_PTEST_SECURE_BOOT
    #endif
#endif

/***********************************************************************************************************************
 *  Configuration for other feature
 *  named as CONFIG_EHSM_xxx if it's a feature configuration
 *  named as CONFIG_EHSM_V_ if it's a value configuration
 **********************************************************************************************************************/

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

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* TEST_INIT_CONFIG_H */
