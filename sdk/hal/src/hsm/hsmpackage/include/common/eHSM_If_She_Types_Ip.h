#ifndef EHSM_IF_SHE_TYPES_IP_H_
#define EHSM_IF_SHE_TYPES_IP_H_
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
#define EHSM_SHE_KEY_PROP_WR_PRT       (0x1 << 7)
#define EHSM_SHE_KEY_PROP_BOOT_PRT     (0x1 << 6)
#define EHSM_SHE_KEY_PROP_DEBUG_PRT    (0x1 << 5)
#define EHSM_SHE_KEY_PROP_KEY_USAGE    (0x1 << 4)
#define EHSM_SHE_KEY_PROP_WILDCARD     (0x1 << 3)
#define EHSM_SHE_KEY_MAC_VERIFY_ONLY   (0x1 << 2)

#define EHSM_SHE_M1_STD_SIZE      (16)
#define EHSM_SHE_M4_STD_SIZE      (32)

#define EHSM_SHE_KEY_SIZE (sizeof(ehsm_she_key_st))
#define EHSM_SHE_OTP_KEY_ATTR_SIZE  (8)   //4 byte counter + 1 byte secure + 3 byte reserved

#define EHSM_SHE_KEY_MAX_SIZE     (16)
#define EHSM_SHE_UID_MAX_SIZE     (15)

#define EHSM_SHE_NVM_KEY_NUM    50

/**
 * @brief No error has occurred and thecommand will be executed.
 */
#define ERC_NO_ERROR                    0x0U

/**
 * @brief This error code is returned if an error not covered by the error codes above is detected inside SHE.
 */
#define ERC_GENERAL_ERROR               0x1U

/**
 * @brief This error code is returned by SHE whenever the sequence of commands or subcommands is out of sequence, e.g.
 *        when a function is called while another function is still running.
 *
 * @note  This is not supported.
 */
#define ERC_SEQUENCE_ERROR              0x2U

/**
 * @brief This error code is returned whenever a function of SHE is  called while another function is still processing,
 *        i.e., when SREGBUSY = 1
 */
#define ERC_BUSY                        0x3U

/**
 * @brief This error code is returned if a key is locked due to failed boot measurement or an active debugger.
 */
#define ERC_KEY_NOT_AVAILABLE           0x4U

/**
 * @brief This error code is returned by SHE whenever a function is called to perform an operation with a key that is
 *        not allowed for the given operation.
 */
#define ERC_KEY_INVALID                 0x5U

/**
 * @brief This error code is returned by SHE if the application attempts to use a key that has not been initialized yet.
 */
#define ERC_KEY_EMPTY                   0x6U

#define ERC_NO_SECURE_BOOT              0x7U

/**
 * @brief This error is returned when a key update is attempted on a memory slot that has been write protected or when
 *        an attempt to active the debugger is started when a key is write-protected.
 */
#define ERC_KEY_WRITE_PROTECTED         0x8U

/**
 * @brief This error is returned when a key update did not succeed due to errors in verification of the messages.
 */
#define ERC_KEY_UPDATE_ERROR            0x9U

/**
 * @brief The error code is returned if internal debugging is not possible because the authentication with the
 *        challenge response protocol did not succeed.
 */
#define ERC_NO_DEBUGGING                0xAU

/**
 * @brief The error code is returned by CMD_RND and CMD_DEBUG if the seed has not been initialized before.
 *
 * @note  This is not supported.
 */
#define ERC_RNG_SEED                    0xBU

/**
 * @brief This error code can be returned if the underlying memory technology is able to detect physical errors, e.g.
 *        flipped bits etc., during memory read or write operations to notify the application.
 *
 * @note  This is not supported
 */
#define ERC_MEMORY_FAILURE              0xCU

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct
{
    ehsm_uint32_t counter;
    ehsm_uint8_t secure_flag;
    ehsm_uint8_t reserved[3];
    ehsm_uint8_t raw_key[EHSM_SHE_KEY_MAX_SIZE];
}ehsm_she_key_st;

/*define the she status register type*/
typedef enum ehsm_she_status_type_
{
    EHSM_SHE_STATUS_TYPE_BUSY          = (1 << 0),
    /*
     * In parallel mode, this is set if she_secure_boot is called.
     * In sequential mode, this is set by default.
     */
    EHSM_SHE_STATUS_TYPE_SECURE_BOOT   = (1 << 1),
    /*
     * This is not set by default.
     */
    EHSM_SHE_STATUS_TYPE_BOOT_INIT     = (1 << 2),
    /*
     * In parallel mode, this bit is set when the secure booting has been finished by calling either CMD_BOOT_FAILURE
     *   or CMD_BOOT_OK or if CMD_SECURE_BOOT failed in verifying BOOT_MAC.
     * In sequential mode, this is set by default.
     */
    EHSM_SHE_STATUS_TYPE_BOOT_FINISHED = (1 << 3),
    /*
     * In parallel mode, this bit is set if the secure booting (CMD_SECURE_BOOT) succeeded. If CMD_BOOT_FAILURE is
     *   called the bit is erased.
     * In sequential mode, this is set by default if boot success, otherwise, it's erased.
     */
    EHSM_SHE_STATUS_TYPE_BOOT_OK       = (1 << 4),
    EHSM_SHE_STATUS_TYPE_RND_INT       = (1 << 5),
    EHSM_SHE_STATUS_TYPE_EXT_DEBUGGER  = (1 << 6),
    EHSM_SHE_STATUS_TYPE_INT_DEBUGGER  = (1 << 7),
}ehsm_she_status_type_e;

typedef enum
{
    EHSM_KEY_SHE_RAM_KEY = 0,
    EHSM_KEY_SHE_MASTER_ECU_KEY,
    EHSM_KEY_SHE_END = EHSM_SHE_NVM_KEY_NUM + 1,
} ehsm_she_key_handle_e;

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
