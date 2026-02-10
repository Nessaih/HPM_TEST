#ifndef EHSM_IF_EXT_TYPES_IP_H_
#define EHSM_IF_EXT_TYPES_IP_H_

#ifdef __cplusplus
extern "C"{
#endif

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Types_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*definition of default value*/
#define CONFIG_SM9_SIGN_DEFAULT_HID_VALUE   0x1
#define CONFIG_SM9_EXCHG_DEFAULT_HID_VALUE  0x2
#define CONFIG_SM9_ENC_DEFAULT_HID_VALUE    0x3

#define SM2_PUBLIC_KEY_SIZE         65U
#define SM2_S1_S2_SIZE              32U

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/* algorithm for ehsm support generate key */
#define EHSM_ALG_RANDOM              0U
#define EHSM_ALG_DES                 1U
#define EHSM_ALG_TDES_128            2U
#define EHSM_ALG_TDES_192            3U
#define EHSM_ALG_AES_128             4U
#define EHSM_ALG_AES_192             5U
#define EHSM_ALG_AES_256             6U
#define EHSM_ALG_SM4                 7U
#define EHSM_ALG_SM2                 8U
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024
#define EHSM_ALG_RSA_1024            9U
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048
#define EHSM_ALG_RSA_2048            10U
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024_CRT
#define EHSM_ALG_RSA_1024_CRT        13U
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048_CRT
#define EHSM_ALG_RSA_2048_CRT        14U
#endif
#define EHSM_ALG_DH                  17U
#define EHSM_ALG_BRAINPOOLP160R1     18U
#define EHSM_ALG_BRAINPOOLP192R1     19U
#define EHSM_ALG_BRAINPOOLP224R1     20U
#define EHSM_ALG_BRAINPOOLP256R1     21U
#define EHSM_ALG_BRAINPOOLP320R1     22U
#define EHSM_ALG_BRAINPOOLP384R1     23U
#define EHSM_ALG_BRAINPOOLP512R1     24U
#define EHSM_ALG_SECP192R1           25U
#define EHSM_ALG_SECP224R1           26U
#define EHSM_ALG_SECP256R1           27U
#define EHSM_ALG_SECP384R1           28U
#define EHSM_ALG_SECP521R1           29U
#define EHSM_ALG_ED25519             30U
#define EHSM_ALG_END                 31U

typedef ehsm_uint8_t ehsm_gen_key_alg_e;
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

#endif /* _EHSM_OTP_OPERATE_H_ */
