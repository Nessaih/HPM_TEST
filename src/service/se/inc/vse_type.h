#ifndef _VSE_TYPE_H_
#define _VSE_TYPE_H_


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tbox_log.h"

#ifndef NULL
#define NULL                    ((void *)0)
#endif

#ifndef TRUE
#define TRUE                    (1)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif

/* Common micros. */
#define MIN(X, Y)               ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)               ((X) > (Y) ? (X) : (Y))

#define MWD(hi, lo)             ((uint16_t)(hi << 8) | lo)
#define HSB(x)                  ((uint8_t)(x >> 8))
#define LSB(x)                  ((uint8_t)x)

#define MOFFSET(type, member)   ((int)&(((type *)0) -> member))	

/* SE state code. */
#define VSE_UC_DISCON           (7)
#define VSE_ERROR_SIZE          (6)
#define VSE_UNFND_FILE          (5)
#define VSE_UNFND_KID           (4)
#define VSE_WAIT_RESP           (3)
#define VSE_RETRY_LAST          (2)
#define VSE_MORE_DATA           (1)
#define VSE_SUCCESS             (0)
#define VSE_ERROR               (-1)
#define VSE_E_PARAM             (-2)
#define VSE_E_BUFF              (-3)


#endif /* _VSE_TYPE_H_ */
