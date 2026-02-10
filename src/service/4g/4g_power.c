#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_power.h"
#include "4g_data.h"
#include "4g_sharememery.h"
#include "4g_at_transmit.h"
#include "4g_sequence_mgr.h"
#include "4g_mgr.h"
#include "4g_ftp.h"

#define POWER_4G_STOP_TIMEOUT                 (10U*1000)/PERIODIC_UNIT_4G
#define POWER_4G_WAIT_STOPDEPEND_TIMEOUT      (10U*1000U)/PERIODIC_UNIT_4G
#define POWER_4G_SEQ_TIMEOUT                  (15u*1000u)/PERIODIC_UNIT_4G
#define POWER_4G_RETRY_TIMEOUT                (1000U)/PERIODIC_UNIT_4G
#define POWER_4G_SHUTDOWN_SLEEP_DELAY         (5*1000)/PERIODIC_UNIT_4G//关机需3100ms彻底关机等待1000ms
#define POWER_4G_SLEEP_DELAY                  (5*1000)/PERIODIC_UNIT_4G	//关机等待5s
#define POWER_4G_DEPEND_FUN_NUM               (4U)
#define POWER_4G_POWER_SLEEP_RETRY_MAX_COUNT  (5U)

typedef VOID (*POWER_4G_STATE_PROC)(VOID);

typedef enum
{
    POWER_4G_STATE_IDLE = 0,
    POWER_4G_STATE_DOSTOP_WAIT_DEPEND,
    POWER_4G_STATE_DOSTOP,
    POWER_4G_STATE_DOSLEEP,
    POWER_4G_STATE_WAITSLEEP_RES,
    POWER_4G_STATE_SLEEP,
    POWER_4G_STATE_DOSTOP_WAIT_DEPEND_AND_WAKEUP,
    POWER_4G_STATE_DOSTOP_AND_WAKEUP,
    POWER_4G_STATE_DOSLEEP_AND_WAKEUP,
    POWER_4G_STATE_WAITSLEEP_RES_AND_WAKEUP,
    POWER_4G_STATE_MAX
}POWER_4G_STATE;

typedef enum
{
    POWER_4G_EVENT_WAKEUP = 0,
    POWER_4G_EVENT_SLEEP,
    POWER_4G_EVENT_TMOUT,
    POWER_4G_EVENT_MAX
}POWER_4G_EVENT;

typedef struct
{
    UINT8 is_used;
    IF_4G_SLEEP_DEPEND_FUN fun;
}POWER_4G_DEPAND;

static inline VOID power_4g_set_state(UINT8 state);
static VOID power_4g_at_sleep(VOID);
static UINT8 power_4g_at_sleep_resp(VOID *context, UINT8 *data, UINT16 *len);
static VOID power_4g_at_wakeup(VOID);
static UINT8 power_4g_at_wakeup_resp(VOID *context, UINT8 *data, UINT16 *len);

static VOID power_4g_wakeup_in_idle(VOID);
static VOID power_4g_sleep_in_idle(VOID);
static VOID power_4g_tmout_in_idle(VOID);
static VOID power_4g_wakeup_in_dostopwaitdepend(VOID);
static VOID power_4g_sleep_in_dostopwaitdepend(VOID);
static VOID power_4g_tmout_in_dotopwaitdepend(VOID);
static VOID power_4g_wakeup_in_dostop(VOID);
static VOID power_4g_sleep_in_dostop(VOID);
static VOID power_4g_tmout_in_dostop(VOID);
static VOID power_4g_wakeup_in_dosleep(VOID);
static VOID power_4g_sleep_in_dosleep(VOID);
static VOID power_4g_tmout_in_dosleep(VOID);
static VOID power_4g_wakeup_in_waitsleepres(VOID);
static VOID power_4g_sleep_in_waitsleepres(VOID);
static VOID power_4g_tmout_in_waitsleepres(VOID);
static VOID power_4g_wakeup_in_sleep(VOID);
static VOID power_4g_sleep_in_sleep(VOID);
static VOID power_4g_tmout_in_sleep(VOID);
static VOID power_4g_wakeup_in_dostopwaitdependandwakeup(VOID);
static VOID power_4g_sleep_in_dostopwaitdependandwakeup(VOID);
static VOID power_4g_tmout_in_dostopwaitdependandwakeup(VOID);
static VOID power_4g_wakeup_in_dostopandwakeup(VOID);
static VOID power_4g_sleep_in_dostopandwakeup(VOID);
static VOID power_4g_tmout_in_dostopandwakeup(VOID);
static VOID power_4g_wakeup_in_dosleepandwakeup(VOID);
static VOID power_4g_sleep_in_dosleepandwakeup(VOID);
static VOID power_4g_tmout_in_dosleepandwakeup(VOID);
static VOID power_4g_wakeup_in_waitsleepresandwakeup(VOID);
static VOID power_4g_sleep_in_waitsleepresandwakeup(VOID);
static VOID power_4g_tmout_in_waitsleepresandwakeup(VOID);

static uint8 power_4g_sleepseq_resp(uint8 cmd, uint8 result);

static SEQ_4G power_4g_sleep_seq = {SEQ_4G_IDLE,
                                    POWER_4G_SEQ_TIMEOUT/PERIODIC_UNIT_4G,
                                    NULL,
                                    NULL,
                                    power_4g_sleepseq_resp
                                    };

static POWER_4G_STATE_PROC power_4g_state_proc[POWER_4G_STATE_MAX][POWER_4G_EVENT_MAX] =
{
    /*POWER_4G_EVENT_WAKEUP                POWER_4G_EVENT_SLEEP                  POWER_4G_EVENT_TMOUT*/
    {power_4g_wakeup_in_idle,              power_4g_sleep_in_idle,               power_4g_tmout_in_idle},              /*POWER_4G_STATE_IDLE*/
    {power_4g_wakeup_in_dostopwaitdepend,  power_4g_sleep_in_dostopwaitdepend,   power_4g_tmout_in_dotopwaitdepend},    /*POWER_4G_STATE_DOSTOP_WAIT_DEPEND*/
    {power_4g_wakeup_in_dostop,            power_4g_sleep_in_dostop,             power_4g_tmout_in_dostop},            /*POWER_4G_STATE_DOSTOP*/
    {power_4g_wakeup_in_dosleep,           power_4g_sleep_in_dosleep,            power_4g_tmout_in_dosleep},           /*POWER_4G_STATE_DOSLEEP*/
    {power_4g_wakeup_in_waitsleepres,      power_4g_sleep_in_waitsleepres,       power_4g_tmout_in_waitsleepres},      /*POWER_4G_STATE_WAITSLEEP_RES*/
    {power_4g_wakeup_in_sleep,             power_4g_sleep_in_sleep,              power_4g_tmout_in_sleep},             /*POWER_4G_STATE_SLEEP*/
    {power_4g_wakeup_in_dostopwaitdependandwakeup, power_4g_sleep_in_dostopwaitdependandwakeup, power_4g_tmout_in_dostopwaitdependandwakeup}, /*POWER_4G_STATE_DOSTOP_WAIT_DEPEND_AND_WAKEUP*/
    {power_4g_wakeup_in_dostopandwakeup,   power_4g_sleep_in_dostopandwakeup,    power_4g_tmout_in_dostopandwakeup},   /*POWER_4G_STATE_DOSTOP_AND_WAKEUP*/
    {power_4g_wakeup_in_dosleepandwakeup,  power_4g_sleep_in_dosleepandwakeup,   power_4g_tmout_in_dosleepandwakeup},  /*POWER_4G_STATE_DOSLEEP_AND_WAKEUP*/
    {power_4g_wakeup_in_waitsleepresandwakeup, power_4g_sleep_in_waitsleepresandwakeup, power_4g_tmout_in_waitsleepresandwakeup} /*POWER_4G_STATE_WAITSLEEP_RES_AND_WAKEUP*/
};

static UINT16 power_4g_tmvalue;
static UINT8  power_4g_state;
static POWER_4G_DEPAND power_4g_depends[POWER_4G_DEPEND_FUN_NUM];

void power_4g_init(void)
{
    UINT8 index;
 
    power_4g_state = POWER_4G_STATE_IDLE;
    power_4g_tmvalue = 0U;

    for(index = 0U; index < POWER_4G_DEPEND_FUN_NUM; index++)
    {
        power_4g_depends[index].is_used = 0U;
        power_4g_depends[index].fun = NULL_PTR;
    }
}

void power_4g_reset(void)
{
    power_4g_tmvalue = 0U;
}

void power_4g_periodic(void)
{
    if(power_4g_state < POWER_4G_STATE_MAX)
    {
        power_4g_state_proc[power_4g_state][POWER_4G_EVENT_TMOUT]();
    }
}

void power_4g_sleep(void)
{
    MODULE_LOG_I(TBOX4G, "power 4g sleep enter");
    if(power_4g_state < POWER_4G_STATE_MAX)
    {
        power_4g_state_proc[power_4g_state][POWER_4G_EVENT_SLEEP]();
    }
}

void power_4g_wakeup(void)
{
    MODULE_LOG_I(TBOX4G, "power 4g wakeup enter, state:%d", power_4g_state);
    if(power_4g_state < POWER_4G_STATE_MAX)
    {
        power_4g_state_proc[power_4g_state][POWER_4G_EVENT_WAKEUP]();
    }
}

static inline VOID power_4g_set_state(UINT8 state)
{
    MODULE_LOG_I(TBOX4G, "power_4g_state:%d -> %d", power_4g_state, state);
    power_4g_state = state;
}

static VOID power_4g_at_sleep(VOID)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QSCLK=1" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        MODULE_LOG_E(TBOX4G, "alloc sharemem failed");
        return;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_SLEEP;
    cmd.retry_count = 2U;
    cmd.tick = 3000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = power_4g_at_sleep_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        MODULE_LOG_E(TBOX4G, "failed to put at cmd to queue");
        sharemem_4g_free(index);
    }
}

static UINT8 power_4g_at_sleep_resp(VOID *context, UINT8 *data, UINT16 *len)
{
    MODULE_LOG_I(TBOX4G, "at sleep resp, code:%02x", ((AT_4G_CMD_RESP *)context)->resp_code);

    return at_4g_common_resp(AT_4G_SLEEP, (AT_4G_CMD_RESP *)context, data, len);
}

static VOID power_4g_at_wakeup(VOID)
{
    AT_4G_CMD cmd;
    INT8 index = -1;
    CHAR *at_str = "AT+QSCLK=0" AT_4G_REQ_SUFFIX;
    sharemem_4g_ele *ele = NULL;

    ele = SHARMEM_4G_ALLOC(index, SHARMMEM_4G_16BYTE);
    if(NULL == ele)
    {
        MODULE_LOG_E(TBOX4G, "alloc sharemem failed");
        return;
    }

    ele->len = strlen(at_str);
    memcpy(ele->mem_ptr, at_str, ele->len);

    cmd.cmd_id = AT_4G_WAKEUP;
    cmd.retry_count = 1U;
    cmd.tick = 3000U;
    cmd.sharm_index = index;
    cmd.state = AT_CMD_SEND_REQ_WAITRESP;
    cmd.resp = power_4g_at_wakeup_resp;
    if(0 != at_4g_transmit_putcmd(AT_4G_CMD_HIGH, &cmd))
    {
        MODULE_LOG_E(TBOX4G, "failed to put at cmd to queue");
        sharemem_4g_free(index);
    }
}

static UINT8 power_4g_at_wakeup_resp(VOID *context, UINT8 *data, UINT16 *len)
{
    MODULE_LOG_I(TBOX4G, "at wakeup resp, code:%02x", ((AT_4G_CMD_RESP *)context)->resp_code);

    return at_4g_common_resp(AT_4G_WAKEUP, (AT_4G_CMD_RESP *)context, data, len);
}

static VOID power_4g_wakeup_in_idle(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_sleep_in_idle(VOID)
{
    power_4g_tmvalue = POWER_4G_WAIT_STOPDEPEND_TIMEOUT;
    power_4g_set_state(POWER_4G_STATE_DOSTOP_WAIT_DEPEND);
}

static VOID power_4g_tmout_in_idle(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_wakeup_in_dostopwaitdepend(VOID)
{
    power_4g_tmvalue = 0U;
    power_4g_set_state(POWER_4G_STATE_DOSTOP_WAIT_DEPEND_AND_WAKEUP);
}

static VOID power_4g_sleep_in_dostopwaitdepend(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_tmout_in_dotopwaitdepend(VOID)
{
    UINT8 index;
    
    if(power_4g_tmvalue > 0U)
    {
        power_4g_tmvalue--;
    }

    for(index = 0U; index < POWER_4G_DEPEND_FUN_NUM; index++)
    {
        if(1U == power_4g_depends[index].is_used &&
           NULL_PTR != power_4g_depends[index].fun)
        {
            if(FALSE == power_4g_depends[index].fun())
            {
                break;
            }
        }
    }
    if(index >= POWER_4G_DEPEND_FUN_NUM)
    {
        MODULE_LOG_I(TBOX4G, "all the depend task are sleep ok");

        seqmgr_4g_abort_allseq(0U);
        power_4g_tmvalue = POWER_4G_STOP_TIMEOUT;
        mgr_4g_do_stop();
        power_4g_set_state(POWER_4G_STATE_DOSTOP);        
    }
    else
    {
        if(0U == power_4g_tmvalue)
        {
            MODULE_LOG_I(TBOX4G, "wait for sleep timeout");
            seqmgr_4g_abort_allseq(0U);
            power_4g_tmvalue = POWER_4G_STOP_TIMEOUT;
            mgr_4g_do_stop();
            power_4g_set_state(POWER_4G_STATE_DOSTOP);  
        }
    } 
}

static VOID power_4g_wakeup_in_dostop(VOID)
{
    power_4g_set_state(POWER_4G_STATE_DOSTOP_AND_WAKEUP);
}

static VOID power_4g_sleep_in_dostop(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_tmout_in_dostop(VOID)
{
    if(power_4g_tmvalue > 0U)
    {
        power_4g_tmvalue--;
    }

    if(MGR_4G_SEQ_STOPFINISH == mgr_4g_get_state())
    {
        if(TBOX_PM_SLEEPPOST_ACTION_SLEEP == tbox_pm_get_sleeppost_action())
        {
            UINT8 ret;

            MODULE_LOG_I(TBOX4G, "stop finish, go to sleep");

            seqmgr_4g_abort_allseq(0U);
            power_4g_sleep_seq.state = SEQ_4G_IDLE;
            power_4g_sleep_seq.timeout = POWER_4G_SEQ_TIMEOUT/PERIODIC_UNIT_4G;
            ret = seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &power_4g_sleep_seq, SEQ_4G_CONFLICT_ABORT);
            if(0U == ret)
            {
                power_4g_set_state(POWER_4G_STATE_DOSLEEP);
            }
            else
            {
                power_4g_tmvalue = POWER_4G_RETRY_TIMEOUT;
            } 
        }
        else
        {
            MODULE_LOG_I(TBOX4G, "stop finish and enter sleep state");
            data_4g_init();
            seqmgr_4g_init();
            sharemem_4g_reinit();
            at_4g_transmit_reset();
            mgr_4g_handle_when_sleep();
            power_4g_set_state(POWER_4G_STATE_SLEEP);
        }
    }
    else
    {
        if(0U == power_4g_tmvalue)
        {
            if(TBOX_PM_SLEEPPOST_ACTION_SLEEP == tbox_pm_get_sleeppost_action())
            {
                UINT8 ret;

                MODULE_LOG_I(TBOX4G, "stop timeout, go to sleep");

                seqmgr_4g_abort_allseq(0U);
                power_4g_sleep_seq.state = SEQ_4G_IDLE;
                power_4g_sleep_seq.timeout = POWER_4G_SEQ_TIMEOUT/PERIODIC_UNIT_4G;
                ret = seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &power_4g_sleep_seq, SEQ_4G_CONFLICT_ABORT);
                if(0U == ret)
                {
                    power_4g_set_state(POWER_4G_STATE_DOSLEEP);
                }
                else
                {
                    power_4g_tmvalue = POWER_4G_RETRY_TIMEOUT;
                }  
            }
            else
            {
                MODULE_LOG_I(TBOX4G, "stop timeout and enter sleep state");
                data_4g_init();
                seqmgr_4g_init();
                sharemem_4g_reinit();
                at_4g_transmit_reset();
                mgr_4g_handle_when_sleep();
                power_4g_set_state(POWER_4G_STATE_SLEEP);    
            }          
        }
    }
}

static VOID power_4g_wakeup_in_dosleep(VOID)
{
    power_4g_tmvalue = 0U;
    power_4g_set_state(POWER_4G_STATE_DOSLEEP_AND_WAKEUP);
}

static VOID power_4g_sleep_in_dosleep(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_tmout_in_dosleep(VOID)
{
    power_4g_at_sleep();
    power_4g_tmvalue = POWER_4G_SEQ_TIMEOUT;
    power_4g_set_state(POWER_4G_STATE_WAITSLEEP_RES);
}

static VOID power_4g_wakeup_in_waitsleepres(VOID)
{
    power_4g_set_state(POWER_4G_STATE_WAITSLEEP_RES_AND_WAKEUP);
}

static VOID power_4g_sleep_in_waitsleepres(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_tmout_in_waitsleepres(VOID)
{
    if(power_4g_tmvalue > 0U)
    {
        power_4g_tmvalue--;
    }
    if(0U == power_4g_tmvalue)
    {
        MODULE_LOG_I(TBOX4G, "wait for enable sleep mode timeout, go to sleep");
        data_4g_init();
        seqmgr_4g_init();
        sharemem_4g_reinit();
        at_4g_transmit_reset();
        mgr_4g_handle_when_sleep();
        tbox_pm_io_mpu_sleep();
        power_4g_set_state(POWER_4G_STATE_SLEEP);
    }
}

static VOID power_4g_wakeup_in_sleep(VOID)
{
    tbox_pm_io_mpu_wake();
    mgr_4g_handle_when_wakeup();
    power_4g_at_wakeup();
    power_4g_set_state(POWER_4G_STATE_IDLE);
}

static VOID power_4g_sleep_in_sleep(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_tmout_in_sleep(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_wakeup_in_dostopwaitdependandwakeup(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_sleep_in_dostopwaitdependandwakeup(VOID)
{
    power_4g_set_state(POWER_4G_STATE_DOSTOP_WAIT_DEPEND);
}

static VOID power_4g_tmout_in_dostopwaitdependandwakeup(VOID)
{
    tbox_pm_io_mpu_wake();
    power_4g_tmvalue = 0U;
    power_4g_set_state(POWER_4G_STATE_IDLE);
}

static VOID power_4g_wakeup_in_dostopandwakeup(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_sleep_in_dostopandwakeup(VOID)
{
    power_4g_set_state(POWER_4G_STATE_DOSTOP);
}

static VOID power_4g_tmout_in_dostopandwakeup(VOID)
{
    if(power_4g_tmvalue > 0U)
    {
        power_4g_tmvalue--;
    }

    if(MGR_4G_SEQ_STOPFINISH == mgr_4g_get_state())
    {
        MODULE_LOG_I(TBOX4G, "stop finish and wakeup");
        tbox_pm_io_mpu_wake();
        mgr_4g_handle_when_wakeup();
        power_4g_set_state(POWER_4G_STATE_IDLE);
    }
    else
    {
        if(0U == power_4g_tmvalue)
        {
            MODULE_LOG_I(TBOX4G, "stop timeout and wakeup");

            tbox_pm_io_mpu_wake();
            mgr_4g_handle_when_wakeup();
            power_4g_set_state(POWER_4G_STATE_IDLE);          
        }
    }    
}

static VOID power_4g_wakeup_in_dosleepandwakeup(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_sleep_in_dosleepandwakeup(VOID)
{
    power_4g_set_state(POWER_4G_STATE_DOSLEEP);
}

static VOID power_4g_tmout_in_dosleepandwakeup(VOID)
{
    tbox_pm_io_mpu_wake();
    mgr_4g_handle_when_wakeup();
    power_4g_set_state(POWER_4G_STATE_IDLE);      
}

static VOID power_4g_wakeup_in_waitsleepresandwakeup(VOID)
{
    /*DO NOTHING*/
}

static VOID power_4g_sleep_in_waitsleepresandwakeup(VOID)
{
    power_4g_set_state(POWER_4G_STATE_WAITSLEEP_RES);
}

static VOID power_4g_tmout_in_waitsleepresandwakeup(VOID)
{
    if(power_4g_tmvalue > 0U)
    {
        power_4g_tmvalue--;
    }
    if(0U == power_4g_tmvalue)
    {
        MODULE_LOG_I(TBOX4G, "wait for sleep timeout and wakeup");
        tbox_pm_io_mpu_wake();
        power_4g_at_wakeup();
        mgr_4g_handle_when_wakeup();
        power_4g_set_state(POWER_4G_STATE_IDLE);
    }
}

static uint8 power_4g_sleepseq_resp(uint8 cmd, uint8 result)
{
    MODULE_LOG_I(TBOX4G, "receive cmd:%d result:%d state:%d", cmd, result, power_4g_state);

    if(AT_4G_SLEEP != cmd)
    {
        return SEQ_4G_CMD_NOMATCH;
    }

    if(POWER_4G_STATE_WAITSLEEP_RES == power_4g_state)
    {
        mgr_4g_handle_when_sleep();
        tbox_pm_io_mpu_sleep();
        power_4g_set_state(POWER_4G_STATE_SLEEP);            
    }
    else
    {
        tbox_pm_io_mpu_wake();
        power_4g_at_wakeup();
        mgr_4g_handle_when_wakeup();
        power_4g_set_state(POWER_4G_STATE_IDLE);            
    }

    return SEQ_4G_CMD_FINISH;
}

int power_4g_reg_depend_fun(IF_4G_SLEEP_DEPEND_FUN fun)
{
    uint8 index;

    for(index = 0U; index < POWER_4G_DEPEND_FUN_NUM; index++)
    {
        if(0U == power_4g_depends[index].is_used)
        {
            power_4g_depends[index].is_used = 1U;
            power_4g_depends[index].fun = fun;
            break;
        }
    }
    if(index >= POWER_4G_DEPEND_FUN_NUM)
    {
        MODULE_LOG_E(TBOX4G, "regiter 4g depend fun failed, no space");
        return (int)TBOX_E_NOMEMORY;
    }

    return (int)TBOX_E_OK;
}
