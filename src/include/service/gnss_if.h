#ifndef __GNSS_IF_H__
#define __GNSS_IF_H__

#include "tbox_common.h"
#include "time_if.h"

typedef enum
{
    GNSS_POS_STATE_UNFIX,
    GNSS_POS_STATE_FIX,
} GNSS_POS_STATE;

typedef struct
{
    DOUBLE longitude;
    DOUBLE latitude;
    UINT8  is_east;
    UINT8  is_north;
} GNSS_POSITION_DATA;

typedef enum
{
    GNSS_FLAG_INVALID = 0,
    GNSS_FLAG_NORMAL,
    GNSS_FLAG_ERROR
}GNSS_ERROR_FLAG;

extern double         gnss_get_altitude(void);
extern double         gnss_get_speed(void);
extern double         gnss_get_direction(void);
extern GNSS_POS_STATE gnss_get_fix_state(void);
extern int32_t        gnss_get_satellites(void);
extern void           gnss_get_position(GNSS_POSITION_DATA *pos);
extern int32_t        gnss_get_time(DEV_TIME *time);
extern GNSS_ERROR_FLAG gnss_wdg_get_error_flag(void);

#endif
