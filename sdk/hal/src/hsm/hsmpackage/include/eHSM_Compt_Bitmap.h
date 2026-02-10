#ifndef _EHSM_BITMAP_H_
#define _EHSM_BITMAP_H_

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_Types_Ip.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct bitmap{
    ehsm_uint32_t bit_max;
    ehsm_uint32_t bit_bytes;
    ehsm_uint8_t bit_map[0];
}bitmap_st;
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

/**
* @brief create a bitmap 
* @bit_max [in] bits of the bitmap 
*
* @return Failure when return NULL, otherwise success
*/
bitmap_st *ehsm_bitmap_create(ehsm_uint32_t bit_max);

/**
* @brief initialize a bitmap
* @bit_max [in] bits of the bitmap
* @bit_ptr [in] memory for bitmap_st
*
*
* @return Failure when return NULL, otherwise success
*/
bitmap_st *ehsm_bitmap_init(void *bit_ptr, ehsm_uint32_t bit_max);

/**
* @brief destroy a bitmap 
* @bmap [in] bitmap the will be destroyed 
*
*/
void ehsm_bitmap_destroy(bitmap_st *bmap);

/**
* @brief clear all the bits of the bitmap 
* @bmap [in] bitmap that will be reset
*
*/
void ehsm_bitmap_reset(bitmap_st *bmap);

/**
* @brief set a bit of the bitmap 
* @bmap [in] bitmap that will be set 
*
*/
void ehsm_bitmap_set(bitmap_st *bmap, ehsm_uint32_t bit);

/**
* @brief clear a bit of the bitmap 
* @bmap [in] bitmap that will be clear 
*
*/
void ehsm_bitmap_clr(bitmap_st *bmap, ehsm_uint32_t bit);

/**
* @brief get the bits number that has been set
* @bmap [in] bitmap object 
*
* @return number of bits that has been set
*/
ehsm_uint32_t ehsm_bitmap_count(bitmap_st *bmap);

/**
* @brief get the first bits index that have been set
* @bmap [in] bitmap object 
*
* @return the first bits index that have been set, 
* no bit has been when returen 0
*/
ehsm_uint32_t ehsm_bitmap_first(bitmap_st *bmap);

/**
* @brief get the last bits index that have been set
* @bmap [in] bitmap object 
*
* @return the last bits index that have been set, 
* no bit has been when returen 0
*/
ehsm_uint32_t ehsm_bitmap_last(bitmap_st *bmap);

#endif
