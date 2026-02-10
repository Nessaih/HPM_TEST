#ifndef UTEST_VECS_ST_H
#define UTEST_VECS_ST_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdint.h>
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define MAX_TAP 4
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

 /***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/
/**
 * @brief Hash test vector structure.
 * @note Used for hash algorithm test cases.
 */
struct hash_testvec {
    const char *key;                /**< Key for hash operation */
    const char *plaintext;          /**< Plaintext input */
    const char *digest;             /**< Expected digest output */
    unsigned int psize;             /**< Plaintext size */
    unsigned short ksize;           /**< Key size */
    int setkey_error;               /**< Error code for setkey operation */
    int digest_error;               /**< Error code for digest operation */
    unsigned short tap[MAX_TAP];    /**< Tap values */
    unsigned short np;              /**< Number of tap values */
    const char *iv;                 /**< Initialization vector (only for GMAC) */
    unsigned int iv_len;            /**< IV length (only for GMAC) */
};

/**
 * @brief KDF test vector structure.
 * @note Used for key derivation function test cases.
 */
struct kdf_testvec {
    const char *salt;               /**< Salt value */
    unsigned int salt_size;         /**< Salt size */
    const char *plaintext;          /**< Plaintext input */
    unsigned int psize;             /**< Plaintext size */
    const char *shared_info;        /**< Shared info value */
    unsigned int shared_info_size;  /**< Shared info size */
    const char *key;                /**< Derived key */
    unsigned int ksize;             /**< Key size */
};

/**
 * @brief Cipher test vector structure.
 * @note Used for symmetric cipher algorithm test cases.
 */
struct cipher_testvec {
    const char *key;                /**< Key for cipher operation */
    const char *iv;                 /**< Initialization vector */
    const char *iv_out;             /**< Output IV */
    const char *ptext;              /**< Plaintext input */
    const char *ctext;              /**< Ciphertext output */
    unsigned char wk;               /**< Weak key flag */
    unsigned short klen;            /**< Key length */
    unsigned int len;               /**< Data length */
    int fips_skip;                  /**< FIPS skip flag */
    int setkey_error;               /**< Error code for setkey operation */
    int crypt_error;                /**< Error code for crypt operation */
    unsigned int clen;              /**< Ciphertext length (only for stream cipher digest) */
};

/**
 * @brief AEAD test vector structure.
 * @note Used for AEAD algorithm test cases.
 */
struct aead_testvec {
    const char *key;                /**< Key for AEAD operation */
    const char *iv;                 /**< Initialization vector */
    const char *ptext;              /**< Plaintext input */
    const char *assoc;              /**< Associated data */
    const char *ctext;              /**< Ciphertext output */
    unsigned char novrfy;           /**< No verify flag */
    unsigned char wk;               /**< Weak key flag */
    unsigned char klen;             /**< Key length in bytes */
    unsigned int plen;              /**< Plaintext length */
    unsigned int clen;              /**< Ciphertext length */
    unsigned int alen;              /**< Associated data length */
    unsigned int ivlen;             /**< IV length */
    int setkey_error;               /**< Error code for setkey operation */
    int setauthsize_error;          /**< Error code for setauthsize operation */
    int crypt_error;                /**< Error code for crypt operation */
};

/**
 * @brief Asymmetric cipher test vector structure.
 * @note Used for asymmetric cipher algorithm test cases.
 */
struct akcipher_testvec {
    const unsigned char *key;               /**< Public/private key data */
    const unsigned char *params;            /**< Random parameters for signature */
    const unsigned char *m;                 /**< Original message */
    const unsigned char *c;                 /**< Signature (r, s) mode */
    unsigned int key_len;                   /**< Key size in bytes */
    unsigned int param_len;                 /**< Parameter size in bytes */
    unsigned int m_size;                    /**< Message size in bytes */
    unsigned int c_size;                    /**< Signature size in bytes */
    unsigned char public_key_vec;           /**< Public key only flag */
    unsigned char siggen_sigver_test;       /**< Signature/verification test flag */
    unsigned char hash_alg;                 /**< Hash algorithm */
};

/**
 * @brief Test vector structure for key agreement and shared secret.
 * @note Used for KPP (Key Pair and Key Agreement) algorithm test cases.
 */
struct kpp_testvec {
    const unsigned char *secret;                /**< Local private key */
    const unsigned char *b_secret;              /**< Remote private key */
    const unsigned char *b_public;              /**< Remote public key */
    const unsigned char *expected_a_public;     /**< Expected local public key */
    const unsigned char *expected_ss;           /**< Expected shared secret */
    unsigned short secret_size;                 /**< Size of local private key */
    unsigned short b_secret_size;               /**< Size of remote private key */
    unsigned short b_public_size;               /**< Size of remote public key */
    unsigned short expected_a_public_size;      /**< Size of local public key */
    unsigned short expected_ss_size;            /**< Size of shared secret */
    int genkey;                                /**< Key generation flag */
    const unsigned char *p;                     /**< DH parameter p */
    const unsigned char *q;                     /**< DH parameter q */
    const unsigned char *g;                     /**< DH parameter g */
    unsigned short p_size;                      /**< Size of p */
    unsigned short q_size;                      /**< Size of q */
    unsigned short g_size;                      /**< Size of g */
};

/**
 * @brief RSA cipher test vector structure.
 * @note Used for RSA cipher algorithm test cases.
 */
struct rsacipher_testvec {
    const unsigned char *n;             /**< RSA modulus n */
    const unsigned char *e;             /**< RSA public exponent e */
    const unsigned char *d;             /**< RSA private exponent d */
    const unsigned char *p;             /**< RSA prime p */
    const unsigned char *q;             /**< RSA prime q */
    const unsigned char *dp;            /**< RSA dp */
    const unsigned char *dq;            /**< RSA dq */
    const unsigned char *u;             /**< RSA coefficient u */
    unsigned int n_byte_size;           /**< Size of n in bytes */
    unsigned int e_byte_size;           /**< Size of e in bytes */
    unsigned int d_byte_size;           /**< Size of d in bytes */
    unsigned int p_byte_size;           /**< Size of p in bytes */
    unsigned int q_byte_size;           /**< Size of q in bytes */
    unsigned int dp_byte_size;          /**< Size of dp in bytes */
    unsigned int dq_byte_size;          /**< Size of dq in bytes */
    unsigned int u_byte_size;           /**< Size of u in bytes */
    const unsigned char *m;             /**< Message */
    unsigned int m_size;                /**< Message size */
    const unsigned char *c;             /**< Ciphertext */
    unsigned int c_size;                /**< Ciphertext size */
    unsigned int is_crt_mode;           /**< CRT mode flag */
    unsigned char public_key_vec;       /**< Public key only flag */
    unsigned char siggen_sigver_test;   /**< Signature/verification test flag */
    unsigned char hash_alg;             /**< Hash algorithm */
    const unsigned char *salt;          /**< Salt for RSASSA-PSS */
    unsigned int salt_byte_size;        /**< Salt size in bytes */
};

/**
 * @brief SM9 cipher test vector structure.
 * @note Used for SM9 cipher algorithm test cases.
 */
struct sm9cipher_testvec {
    unsigned char hid;                  /**< SM9 hid parameter */
    const unsigned char *Ppub;          /**< SM9 public parameter */
    const unsigned char *priv;          /**< SM9 private key */
    const unsigned char *id;            /**< SM9 identity */
    unsigned int id_sz;                 /**< Identity size */
    const unsigned char *m;             /**< Message */
    unsigned int m_sz;                  /**< Message size */
    const unsigned char *r;             /**< Random value */
    unsigned int r_sz;                  /**< Random value size */
    const unsigned char *c;             /**< Ciphertext */
    unsigned int c_sz;                  /**< Ciphertext size */
    const unsigned char *h;             /**< Hash value */
    const unsigned char *sig;           /**< Signature */
    unsigned char enc_typde;            /**< Encryption type */
    unsigned int K2len;                 /**< K2 length */
    unsigned char padding;              /**< Padding flag */
    unsigned char siggen_sigver_test;   /**< Signature/verification test flag */
};

/**
 * @brief ECIES test vector structure.
 * @note Used for ECIES algorithm test cases.
 */
struct ecies_testvec {
    unsigned char curve_id;                     /**< Curve ID */
    unsigned char *msg;                         /**< Message */
    unsigned int msg_bytes;                     /**< Message size in bytes */
    unsigned char *shared_info1;                /**< Shared info 1 */
    unsigned int shared_info1_bytes;            /**< Shared info 1 size */
    unsigned char *shared_info2;                /**< Shared info 2 */
    unsigned int shared_info2_bytes;            /**< Shared info 2 size */
    const unsigned char *receiver_pri_key;      /**< Receiver private key */
    unsigned int receiver_pri_key_sz;           /**< Receiver private key size */
    const unsigned char *receiver_pub_key;      /**< Receiver public key */
    unsigned int receiver_pub_key_sz;           /**< Receiver public key size */
    unsigned char point_form;                   /**< Point form */
    const unsigned char *sender_tmp_pri_key;    /**< Ephemeral private key in encrypting */
    unsigned int sender_tmp_pri_key_sz;         /**< Ephemeral private key size */
    unsigned char kdf_hash_alg;                 /**< KDF hash algorithm */
    unsigned char mac_hash_alg;                 /**< MAC hash algorithm */
    unsigned int mac_k_bytes;                   /**< MAC key bytes */
    unsigned char *cipher_part1;                /**< Cipher part 1 */
    unsigned char *cipher_part2_part3_without_s1_s2; /**< Cipher part 2 and 3 without shared_info1 and shared_info2 */
    unsigned char *cipher_part2_part3_with_s1;       /**< Cipher part 2 and 3 with shared_info1 */
    unsigned char *cipher_part2_part3_with_s2;       /**< Cipher part 2 and 3 with shared_info2 */
    unsigned char *cipher_part2_part3_with_s1_s2;    /**< Cipher part 2 and 3 with shared_info1 and shared_info2 */
};

static const char zeroed_string[48];
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#endif
