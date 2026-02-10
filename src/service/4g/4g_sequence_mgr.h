#ifndef TBOX_4G_SEQUENCE_MGR_H
#define TBOX_4G_SEQUENCE_MGR_H

#define SEQMGR_4G_CMD_EXE_OK      0
#define SEQMGR_4G_CMD_EXE_FAILED  1
#define SEQMGR_4G_CMD_EXE_TIMEOUT 2
#define SEQMGR_4G_CMD_EXE_ABORT   3

#define SEQ_4G_CONFLICT_ABORT     0
#define SEQ_4G_CONFLICT_SUSPEND   1
#define SEQ_4G_CONFLICT_NOCONFICT 2

#define SEQ_4G_CMD_ISMATCH       0
#define SEQ_4G_CMD_NOMATCH       1
#define SEQ_4G_CMD_FINISH      2

typedef void (*SEQ_4G_BEGIN)(void);
typedef void (*SEQ_4G_PERIODIC)(void);
typedef uint8 (*SEQ_4G_RESP)(uint8 cmd, uint8 result);

typedef enum
{
    SEQ_4G_PRI_LOW  = 0,
    SEQ_4G_PRI_HIGH,
    SEQ_4G_PRI_MAX
}SEQ_4G_PRIORITY;

typedef enum
{
    SEQ_4G_IDLE = 0,
    SEQ_4G_RUNNING,
    SEQ_4G_ABORT,
    SEQ_4G_SUSPENDING,
    SEQ_4G_SUSPENDED
}SEQ_4G_STATE;

typedef struct
{
    uint8  state;
    uint16 timeout;
    SEQ_4G_BEGIN begin;
    SEQ_4G_PERIODIC periodic;
    SEQ_4G_RESP resp;
}SEQ_4G;

void seqmgr_4g_init(void);

void seqmgr_4g_abort_allseq(uint16 timeout);

boolean seqmgr_4g_is_allseq_idle(void);

void seqmgr_4g_resetseq(uint8 pri);

uint8 seqmgr_4g_doseq(uint8 pri, SEQ_4G *seq, uint8 conflict_type);

void seqmgr_4g_periodic(void);

void seqmgr_4g_handle_cmd_exe_result(uint8 cmd, uint8 result);

#endif /* TBOX_4G_SEQUENCE_MGR_H */
