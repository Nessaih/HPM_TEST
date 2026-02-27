#ifndef TBOX_TYPE_H
#define TBOX_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL (0U)
#endif

#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

#ifndef TRUE
#define TRUE 1U
#endif

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef BOOL
#define BOOL uint8_t
#endif

#ifndef UINT8
#define UINT8 uint8_t
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef INT8
#define INT8 int8_t
#endif

#ifndef UINT16
#define UINT16 uint16_t
#endif

#ifndef INT16
#define INT16 int16_t
#endif

#ifndef UINT32
#define UINT32 uint32_t
#endif

#ifndef INT32
#define INT32 int32_t
#endif

#ifndef UINT64
#define UINT64 unsigned long long
#endif

#ifndef INT64
#define INT64 signed long long
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef FLOAT
#define FLOAT float
#endif

#ifndef DOUBLE
#define DOUBLE double
#endif

#ifndef SIZE
#define SIZE UINT32
#endif

#ifndef UNUSED
#define UNUSED(x) ((VOID)(x))
#endif

#ifndef TBOX_ID
#define TBOX_ID INT32
#define TBOX_ID_INVALID (-1)
#endif


typedef union
{
    UINT8 byte;

    struct
    {
        UINT8 B0    :1;
        UINT8 B1    :1;
        UINT8 B2    :1;
        UINT8 B3    :1;
        UINT8 B4    :1;
        UINT8 B5    :1;
        UINT8 B6    :1;
        UINT8 B7    :1;
    }bit;

    struct
    {
        UINT8 L :4;
        UINT8 H :4;
    }half;
}UNION8;


typedef union
{
	UINT16 word;

	struct
	{
		UINT16 B00 : 1;
		UINT16 B01 : 1;
		UINT16 B02 : 1;
		UINT16 B03 : 1;
		UINT16 B04 : 1;
		UINT16 B05 : 1;
		UINT16 B06 : 1;
		UINT16 B07 : 1;
		UINT16 B08 : 1;
		UINT16 B09 : 1;
		UINT16 B10 : 1;
		UINT16 B11 : 1;
		UINT16 B12 : 1;
		UINT16 B13 : 1;
		UINT16 B14 : 1;
		UINT16 B15 : 1;
	} bit;

	struct
	{
		UINT8 L;
		UINT8 H;
	} byte;
} UNION16;

typedef union
{
	UINT32 dword;

	struct
	{
		UINT32 B00 : 1;
		UINT32 B01 : 1;
		UINT32 B02 : 1;
		UINT32 B03 : 1;
		UINT32 B04 : 1;
		UINT32 B05 : 1;
		UINT32 B06 : 1;
		UINT32 B07 : 1;
		UINT32 B08 : 1;
		UINT32 B09 : 1;
		UINT32 B10 : 1;
		UINT32 B11 : 1;
		UINT32 B12 : 1;
		UINT32 B13 : 1;
		UINT32 B14 : 1;
		UINT32 B15 : 1;
		UINT32 B16 : 1;
		UINT32 B17 : 1;
		UINT32 B18 : 1;
		UINT32 B19 : 1;
		UINT32 B20 : 1;
		UINT32 B21 : 1;
		UINT32 B22 : 1;
		UINT32 B23 : 1;
		UINT32 B24 : 1;
		UINT32 B25 : 1;
		UINT32 B26 : 1;
		UINT32 B27 : 1;
		UINT32 B28 : 1;
		UINT32 B29 : 1;
		UINT32 B30 : 1;
		UINT32 B31 : 1;
	} bit;

	struct
	{
		UINT8 LL;
		UINT8 LH;
		UINT8 HL;
		UINT8 HH;
	} byte;

	struct
	{
		UINT16 L;
		UINT16 H;
	} word;

} UNION32;

typedef union
{
	FLOAT 	f;
	UINT8   bytes[4];
} UNION_FLOAT;

typedef union
{
	INT64 	d;
	UINT8   bytes[8];
} UNION_DOUBLE;

typedef enum tag_tbox_error_code
{
    TBOX_E_OK                 = 0,
    TBOX_E_FAILED             = -1,
    TBOX_E_NOINIT             = -2,
    TBOX_E_NOOPEN             = -3,
    TBOX_E_NOCREATE           = -4,
    TBOX_E_NOSUPPORT          = -5,
    TBOX_E_NOREADY            = -6,
    TBOX_E_NORESOURCES        = -7,
    TBOX_E_NOMEMORY           = -8,
    TBOX_E_NODATA             = -9,
    TBOX_E_NOEXISTS           = -10,
    TBOX_E_NOMATCH            = -11,
    TBOX_E_NOPRIVILEGE        = -12,
    TBOX_E_NOFOUND            = -13,
    TBOX_E_NOALLOC            = -14,
    TBOX_E_INVALID_PARAM      = -15,
    TBOX_E_INVALID_STATE      = -16,
    TBOX_E_INVALID_FORMAT     = -17,
    TBOX_E_INVALID_VALUE      = -18,
    TBOX_E_INVALID_HANDLE     = -19,
    TBOX_E_INVALID_INDEX      = -20,
    TBOX_E_INVALID_LENGTH     = -21,
    TBOX_E_INVALID_ADDRESS    = -22,
    TBOX_E_INVALID_DATA       = -23,
    TBOX_E_INVALID_POINTER    = -24,
    TBOX_E_INVALID_VALIDATION = -25,
    TBOX_E_INVALID_SIGNATURE  = -26,
    TBOX_E_INVALID_KEY        = -27,
    TBOX_E_INVALID_VERSION    = -28,
    TBOX_E_INVALID_CONFIG     = -29,
    TBOX_E_UNKNOWN_ID         = -30,
    TBOX_E_UNKNOWN_SERVICE    = -31,
    TBOX_E_UNKNOWN_NAME       = -32,
    TBOX_E_UNKNOWN_ADDRESS    = -33,
    TBOX_E_UNKNOWN_MESSAGE    = -34,
    TBOX_E_UNKNOWN_OBJECT     = -35,
    TBOX_E_IS_BUSY            = -36,
    TBOX_E_IS_FULL            = -37,
    TBOX_E_IS_EMPTY           = -38,
    TBOX_E_IS_TIMEOUT         = -39,
    TBOX_E_IS_CLOSED          = -40,
    TBOX_E_IS_DESTROYED       = -41,
    TBOX_E_IS_DISABLED        = -42,
    TBOX_E_IS_ENABLED         = -43,
    TBOX_E_IS_RUNNING         = -44,
    TBOX_E_IS_STOPPED         = -45,
    TBOX_E_IS_SUSPENDED       = -46,
    TBOX_E_FAILED_INIT        = -47,
    TBOX_E_FAILED_OPEN        = -48,
    TBOX_E_FAILED_CREATE      = -49,
    TBOX_E_FAILED_IO          = -50,
    TBOX_E_FAILED_ALLOC       = -51,
    TBOX_E_FAILED_LOCK        = -52,
    TBOX_E_FAILED_UNLOCK      = -53,
    TBOX_E_OVERFLOW           = -54,
    TBOX_E_UNDERFLOW          = -55,
    TBOX_E_TOOSMALL           = -56,
    TBOX_E_HASSTART           = -57,
    TBOX_E_HASINIT            = -58,
    TBOX_E_HASSTOP            = -59,
    TBOX_E_HASDESTROY         = -60,
    TBOX_E_HASEXIST           = -61,
}TBOX_ERROR_CODE;

#ifdef __cplusplus
}
#endif

#endif /* TBOX_TYPE_H */