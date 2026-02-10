#ifndef TBOX_4G_SIM_H
#define TBOX_4G_SIM_H

#include "4g_if.h"

#define SIM_4G_INITSTATE_DEFAULT     0
#define SIM_4G_INITSTATE_PINREADY   (1 << 0)
#define SIM_4G_INITSTATE_SMSDONE    (1 << 1)
#define SIM_4G_INITSTATE_PBDONE     (1 << 2)
#define SIM_4G_INITSTATE_ALLREADY   (SIM_4G_INITSTATE_PINREADY | SIM_4G_INITSTATE_SMSDONE | SIM_4G_INITSTATE_PBDONE)

typedef enum
{
    SIM_4G_PINSTATE_UNKNOWN  = 0U,
    SIM_4G_PINSTATE_NOINSERT,
    SIM_4G_PINSTATE_READY,
    SIM_4G_PINSTATE_NEEDPIN,
    SIM_4G_PINSTATE_NEEDPUK
}SIM_4G_PINSTATE;

typedef enum
{
    SIM_4G_SETNUM_UNKNOWN = 0,
    SIM_4G_DISABLE_SETNUM,
    SIM_4G_ENABLE_SETNUM
}SIM_4G_SETNUM_FLAG;

typedef struct
{
    uint8 sim_state;         /* SIM���״̬ */
    uint8 setnum_flag;
    boolean iccid_is_valid;
    uint8 sim_4g_iccid[SIM_4G_ICCID_MAX];
    boolean imsi_is_valid;
    uint8 sim_4g_imsi[SIM_4G_IMSI_MAX];
    boolean num_is_valid;
    uint8 sim_4g_num[SIM_4G_NUM_MAX+1];
}SIM_4G_INFO;

extern SIM_4G_INFO sim_4g_info;

void sim_4g_init(void);

void sim_4g_reinit(void);

uint8 sim_4g_query_initstate(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_query_pinstate(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_query_iccid(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_query_imsi(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_enable_setnum(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_query_num(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_query_num_ex(AT_4G_CMD_PRIORITY pri, uint8 retry);

uint8 sim_4g_set_num(AT_4G_CMD_PRIORITY pri, char* num);

uint8 sim_4g_set_num_ex(AT_4G_CMD_PRIORITY pri, char* num, uint8 retry);

uint8 sim_4g_query_state(AT_4G_CMD_PRIORITY pri);

uint8 sim_4g_get_initstate(void);

void sim_4g_set_initstate(uint8 state);

uint8 sim_4g_get_pinstate(void);

void sim_4g_set_pinstate(uint8 state);

#endif /* TBOX_4G_SIM_H */
