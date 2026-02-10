#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_sequence_mgr.h"
#include "4g_mgr.h"
#include "4g_at.h"

static SEQ_4G *seq_4g[SEQ_4G_PRI_MAX];

void seqmgr_4g_init(void)
{
    uint8 index;
    for(index = SEQ_4G_PRI_LOW; index < SEQ_4G_PRI_MAX; index++)
    {
        seq_4g[index] = NULL;
    }
}

void seqmgr_4g_abort_allseq(uint16 timeout)
{
    uint8 index;

    for(index = SEQ_4G_PRI_LOW; index < SEQ_4G_PRI_MAX; index++)
    {
        if(seq_4g[index] != NULL &&
           SEQ_4G_IDLE != seq_4g[index]->state)
        {
            if(0 == timeout)
            {
                seq_4g[index]->state = SEQ_4G_IDLE;
                seq_4g[index] = NULL;
            }
            else
            {
                seq_4g[index]->state = SEQ_4G_ABORT;
                seq_4g[index]->timeout = timeout;
            }
        }
    }
}

void seqmgr_4g_resetseq(uint8 pri)
{
    uint8 index;

    if(NULL != seq_4g[pri])
    {
        seq_4g[pri]->state = SEQ_4G_IDLE;
        seq_4g[pri] = NULL;
    }

    for(index = SEQ_4G_PRI_LOW; index < pri; index++)
    {
        if(seq_4g[index] != NULL &&
           SEQ_4G_IDLE != seq_4g[index]->state)
        {
            seq_4g[index]->state = SEQ_4G_RUNNING;
        }
    }
}

boolean seqmgr_4g_is_allseq_idle(void)
{
    uint8 index;
    for(index = SEQ_4G_PRI_LOW; index < SEQ_4G_PRI_MAX; index++)
    {
        if(seq_4g[index] != NULL &&
           SEQ_4G_IDLE != seq_4g[index]->state)
        {
            return FALSE;
        }
    }

    return TRUE;
}

uint8 seqmgr_4g_doseq(uint8 pri, SEQ_4G *seq, uint8 conflict_type)
{
    uint8 index;
    uint8 low_pri_has_run = 0;

    if(pri >= SEQ_4G_PRI_MAX)
    {
        MODULE_LOG_E(TBOX4G, "the pri is invalid");
        return 1;
    }

    for(index = pri; index < SEQ_4G_PRI_MAX; index++)
    {
        if(seq_4g[index] != NULL &&
           SEQ_4G_IDLE != seq_4g[index]->state)
        {
            MODULE_LOG_W(TBOX4G, "the sequence[pri%d] is busy", index);
            return 1;
        }
    }
    for(index = SEQ_4G_PRI_LOW; index < pri; index++)
    {
        if(seq_4g[index] != NULL &&
           SEQ_4G_IDLE != seq_4g[index]->state)
        {
            if(SEQ_4G_CONFLICT_ABORT == conflict_type)
            {
                seq_4g[index]->state = SEQ_4G_ABORT;
            }
            else if(SEQ_4G_CONFLICT_SUSPEND == conflict_type)
            {
                seq_4g[index]->state = SEQ_4G_SUSPENDING;
            }
            else
            {
                /**/
            }
            low_pri_has_run = 1;
        }
    }
    if(1 == low_pri_has_run)
    {
        MODULE_LOG_E(TBOX4G, "the low priority has run");
        return 1;
    }

    seq_4g[pri] = seq;

    if(NULL != seq_4g[pri]->begin)
    {
        seq_4g[pri]->begin();
    }
    seq_4g[pri]->state = SEQ_4G_RUNNING;

    return 0;
}

void seqmgr_4g_periodic(void)
{
    uint8 index;
    for(index = SEQ_4G_PRI_LOW; index < SEQ_4G_PRI_MAX; index++)
    {
        if(seq_4g[index] == NULL)
        {
            continue;
        }
        if(SEQ_4G_IDLE == seq_4g[index]->state)
        {
            seq_4g[index] = NULL;
            continue;
        }

    //    MODULE_LOG_I(TBOX4G, "index:%d state:%d", index, seq_4g[index]->state);

        if(SEQ_4G_RUNNING == seq_4g[index]->state)
        {
            seq_4g[index]->timeout--;
            if(seq_4g[index]->timeout == 0)
            {
                MODULE_LOG_E(TBOX4G, "the task run timeout");
                if(NULL != seq_4g[index]->resp)
                {
                    seq_4g[index]->resp(AT_4G_UNKNOWN_CMD, SEQMGR_4G_CMD_EXE_TIMEOUT);
                }
                seq_4g[index]->state = SEQ_4G_IDLE;
                seq_4g[index] = NULL;
                return;
            }
            if(NULL != seq_4g[index]->periodic)
            {
                seq_4g[index]->periodic();
            }
        }
        else if(SEQ_4G_ABORT == seq_4g[index]->state)
        {
            seq_4g[index]->timeout--;
            if(seq_4g[index]->timeout == 0)
            {
                MODULE_LOG_E(TBOX4G, "the task abort timeout, abort the sequence[%d]", index);
                if(NULL != seq_4g[index]->resp)
                {
                    seq_4g[index]->resp(AT_4G_UNKNOWN_CMD, SEQMGR_4G_CMD_EXE_ABORT);
                }
                seq_4g[index]->state = SEQ_4G_IDLE;
                seq_4g[index] = NULL;
                return;
            }
        }
    }
}

void seqmgr_4g_handle_cmd_exe_result(uint8 cmd, uint8 result)
{
    uint8 index;
    uint8 ret;
    for(index = SEQ_4G_PRI_LOW; index < SEQ_4G_PRI_MAX; index++)
    {
        if(seq_4g[index] == NULL ||
           SEQ_4G_IDLE == seq_4g[index]->state)
        {
            continue;
        }

        ret = SEQ_4G_CMD_NOMATCH;
        if(NULL != seq_4g[index]->resp)
        {
            ret = seq_4g[index]->resp(cmd, result);
            MODULE_LOG_I(TBOX4G, "index:%d cmd:%d result:%d ret:%d", index, cmd, result, ret);
        }

        if(SEQ_4G_CMD_NOMATCH == ret)
        {
            continue;
        }
        else
        {
            if(SEQ_4G_CMD_FINISH == ret)
            {
                uint8 tem_index;

                seq_4g[index]->state = SEQ_4G_IDLE;
                seq_4g[index] = NULL;
                for(tem_index = SEQ_4G_PRI_LOW; tem_index < index; tem_index++)
                {
                    if(seq_4g[tem_index] == NULL ||
                        SEQ_4G_SUSPENDED != seq_4g[tem_index]->state)
                    {
                        continue;
                    }
                    seq_4g[tem_index]->state = SEQ_4G_RUNNING;
                }
            }
            else
            {
                if(SEQ_4G_ABORT == seq_4g[index]->state)
                {
                    MODULE_LOG_I(TBOX4G, "abort the sequence[%d]", index);
                    if(NULL != seq_4g[index]->resp)
                    {
                       seq_4g[index]->resp(AT_4G_UNKNOWN_CMD, SEQMGR_4G_CMD_EXE_ABORT);
                    }
                    seq_4g[index]->timeout = 0;
                    seq_4g[index]->state = SEQ_4G_IDLE;
                    seq_4g[index] = NULL;
                }
                else if(SEQ_4G_SUSPENDING == seq_4g[index]->state)
                {
                    seq_4g[index]->state = SEQ_4G_SUSPENDED;
                }
            }
        }
    }
}
