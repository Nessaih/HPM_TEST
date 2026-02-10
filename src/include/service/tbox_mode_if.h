#ifndef TBOX_MODE_IF_H
#define TBOX_MODE_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#define TBOX_MODE_SWITCH_NOTIFY "MODE_SWITCH_NOTIFY"

typedef enum
{
    TBOX_MODE_FACTORY = 0x00,
    TBOX_MODE_NORMAL,
    TBOX_MODE_MAINTENANCE,
    TBOX_MODE_UNDERVOLTAGE,
    TBOX_MODE_EMERGENCY,
    TBOX_MODE_ABNORMAL,
    TBOX_MODE_MAX
}TBOX_MODE_TYPE;

INT32 tbox_mode_switch(TBOX_MODE_TYPE mode);
INT32 tbox_mode_get(TBOX_MODE_TYPE *mode);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_MODE_IF_H */