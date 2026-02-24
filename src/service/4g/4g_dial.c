#include "4g_depend_header.h"
#include "tbox_phm_if.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_sms.h"
#include "4g_dial.h"
#include "4g_mgr.h"
#include "4g_sequence_mgr.h"

#define DIAL_4G_CALL_TIMEOUT            (360*1000)     //360S
#define DIAL_4G_STOP_TIMEOUT            (40*1000)      //40S
#define DIAL_4G_ABORTCALL_TIMEOUT       (5*1000)       //5S
#define DIAL_4G_RETRY_PERIOD            (1000)         //1S
#define DIAL_4G_CONFIGAPN_TIMEOUT       (5000)         //5S
#define DIAL_4G_RESET4G_MAXCOUNT        (2)
#define DIAL_4G_REFETCH_REG_MAXCOUNT    ((uint8)3)

typedef enum
{
    DIAL_4G_CALL_IDLE = 0,
    DIAL_4G_CALL_DO_CALL,
    DIAL_4G_CALL_QUERY_APN,
    DIAL_4G_CALL_WAITFOR_QUERY_APN_RES,
    DIAL_4G_CALL_CONFIG_APN,
    DIAL_4G_CALL_WAITFOR_CONFIG_APN_RES,
    DIAL_4G_CALL_RESET4G,
    DIAL_4G_CALL_QUERY_REG_STATE,
    DIAL_4G_CALL_DEAVTIVE_APN,
    DIAL_4G_CALL_WAITFOR_DEACTIVE_RES,
    DIAL_4G_CALL_ACTIVE_APN,
    DIAL_4G_CALL_WAITFOR_ACTIVE_APN_RES,
    DIAL_4G_CALL_FINISH,
}DIAL_4G_CALL_STATE;

typedef enum
{
    DIAL_4G_STOP_IDLE = DIAL_4G_CALL_FINISH+1,
    DIAL_4G_STOP_DO_STOP,
    DIAL_4G_STOP_DEACTIVE_APN,
    DIAL_4G_STOP_WAITFOR_DEACTIVE_APN_RES,
    DIAL_4G_STOP_FINISH,
}DIAL_4G_STOP_STATE;

typedef struct
{
    uint8 mgr_state;
    uint8 do_state;
    uint8 cur_index;
    uint16 period;
    uint8 need_abort_call;
    DIAL_4G_NET net[DIAL_4G_APN_MAX_INDEX];
}DIAL_4G_MGR;

static uint8 dial_4g_seq_resp(uint8 cmd, uint8 result);

static boolean dial_4g_has_apn_need_call(INT8 cur_index, uint8 *next_call_index);

static boolean dial_4g_has_apn_need_stop(INT8 cur_index, uint8 *next_call_index);

static void dial_4g_periodic_do_call(void);

static void dial_4g_periodic_do_stop(void);

static void dial_4g_handle_notify_when_abort(void);

static void dial_4g_handle_resptimeout(void);

static void dial_4g_cfg_change(const char *name, TBOX_MSG_DATA *data);

static void dial_4g_cfg_setdefault(const char *name, TBOX_MSG_DATA *data);

static DIAL_4G_MGR dial_4g_mgr;

static uint8 dial_4g_resetcount;

static uint8 dial_4g_apn_current_index;

static uint8 dial_4g_fetchcount;

static SEQ_4G dial_4g_seq = {SEQ_4G_IDLE,
                             0,
                             NULL,
                             NULL,
                             dial_4g_seq_resp
                            };

void dial_4g_init(void)
{
    uint8 index;
	TBOX_CFG_ID cfg_id;

    dial_4g_resetcount = 0;
    dial_4g_fetchcount = 0;

    dial_4g_mgr.mgr_state = DIAL_4G_MGR_INIT;
    dial_4g_mgr.do_state  = 0;
    dial_4g_mgr.cur_index = 0;
    dial_4g_mgr.period = 0;
    dial_4g_mgr.need_abort_call = 0;
    for(index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        dial_4g_mgr.net[index].state = DIAL_4G_STATE_DISCONNECTED;
        if(DIAL_4G_PUBLIC_APN == index)
        {
        	TBOX_CFG_ID_GET(PUBAPN, cfg_id);
        	tbox_cfg_read(cfg_id, dial_4g_mgr.net[index].apn);
            memset(dial_4g_mgr.net[index].usrname, 0, sizeof(dial_4g_mgr.net[index].usrname));
            memset(dial_4g_mgr.net[index].password, 0, sizeof(dial_4g_mgr.net[index].password));
            tbox_phm_add_apn_monitor(DIAL_4G_PUBLIC_APN);
            MODULE_LOG_D(TBOX4G,"pub apn: %s \r\n", dial_4g_mgr.net[index].apn);
        }
        else
        {
            memset(dial_4g_mgr.net[index].apn, 0, sizeof(dial_4g_mgr.net[index].apn));
            memset(dial_4g_mgr.net[index].usrname, 0, sizeof(dial_4g_mgr.net[index].usrname));
            memset(dial_4g_mgr.net[index].password, 0, sizeof(dial_4g_mgr.net[index].password));
        }
    }

    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOX4G, module_id);
    tbox_message_subscribe(TBOX_CFG_EVENT_VALUE_CHANGE, module_id, dial_4g_cfg_change);
    tbox_message_subscribe(TBOX_CFG_EVENT_SET_DEFAULT, module_id, dial_4g_cfg_setdefault);   
}

void dial_4g_reset(void)
{
    uint8 index;

    dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
    dial_4g_fetchcount = 0;

    dial_4g_mgr.mgr_state = DIAL_4G_MGR_INIT;
    dial_4g_mgr.do_state  = 0;
    dial_4g_mgr.cur_index = 0;
    dial_4g_mgr.period = 0;
    dial_4g_mgr.need_abort_call = 0;

    TBOX_4G_MUTEX_LOCK();
    for(index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        dial_4g_mgr.net[index].state = DIAL_4G_STATE_DISCONNECTED;
    }
    TBOX_4G_MUTEX_UNLOCK();
}

void dial_4g_set_apn(uint8 index,
                     uint8 *apn,
                     uint8 *username,
                     uint8 *password)
{
    if(index >= DIAL_4G_APN_MAX_INDEX ||
       apn == NULL)
    {
        return;
    }

    strncpy((char*)dial_4g_mgr.net[index].apn, (char*)apn, DIAL_4G_APN_LEN - 1);
    if(NULL != username)
    {
        strncpy((char*)dial_4g_mgr.net[index].usrname, (char*)username, DIAL_4G_USERNAME_LEN - 1);
    }
    if(NULL != password)
    {
        strncpy((char*)dial_4g_mgr.net[index].password, (char*)password, DIAL_4G_PASSWORD_LEN - 1);
    }
}

/*
 *拨号流程：
  A、先查询模块APN信息。
  B、如果APN存在，就查询注网情况，注网OK，就进行拨号。
  C、如果APN不存在，间隔10秒配置APN，配置APN后，间隔10秒复位4G模块。
  D、复位4G模块次数限定为3次，如果超过3次，就不复位4G模块，为了避免APN设置失败，一直复位4G模块
  E、如果连续3次获取2g/3g/4g的注网状态为未注网，就直接进行拨号；如果获取注网状态为注网上，将注网状态重试次数重置为0。 <<20251016 lgc m1>>
 */
uint8 dial_4g_docall(void)
{
    uint8 next_index = 0;

    if(DIAL_4G_MGR_INIT != dial_4g_mgr.mgr_state &&
       DIAL_4G_MGR_CALL_FINISH != dial_4g_mgr.mgr_state &&
       DIAL_4G_MGR_STOP_FINISH != dial_4g_mgr.mgr_state)
    {
        MODULE_LOG_E(TBOX4G, "state is invalid, state:%d", dial_4g_mgr.mgr_state);
        return 1;
    }

    dial_4g_mgr.mgr_state = DIAL_4G_MGR_DOCALL;
    dial_4g_mgr.do_state = DIAL_4G_CALL_IDLE;
    dial_4g_mgr.cur_index = DIAL_4G_PUBLIC_APN;
    dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
    dial_4g_mgr.period = 0;
    dial_4g_mgr.need_abort_call = 0;
    if(FALSE == dial_4g_has_apn_need_call(-1, &next_index))
    {
        MODULE_LOG_E(TBOX4G, "it is not need to dial");
        dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
        return 0;
    }

    dial_4g_mgr.cur_index = next_index;
    TBOX_4G_MUTEX_LOCK();
    dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_CONNECTING;
    TBOX_4G_MUTEX_UNLOCK();
    dial_4g_seq.state = SEQ_4G_IDLE;
    dial_4g_seq.timeout = (uint32)DIAL_4G_CALL_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
    if(0 == seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &dial_4g_seq, (uint8)SEQ_4G_CONFLICT_ABORT))
    {
        dial_4g_mgr.do_state = DIAL_4G_CALL_QUERY_APN;
    }
    else
    {
        dial_4g_mgr.do_state = DIAL_4G_CALL_DO_CALL;
        dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
    }
    
    MODULE_LOG_I(TBOX4G, "do dial, mgr state:%d do state:%d", dial_4g_mgr.mgr_state, dial_4g_mgr.do_state);
    
    return 0;
}

uint8 dial_4g_stopcall(void)
{
    uint8 next_index = 0;

    dial_4g_resetcount = 0;

    if(DIAL_4G_MGR_INIT == dial_4g_mgr.mgr_state ||
       DIAL_4G_MGR_DOSTOP == dial_4g_mgr.mgr_state)
    {
        MODULE_LOG_E(TBOX4G, "state is invalid, state:%d", dial_4g_mgr.mgr_state);
        return 1;
    }

    if(DIAL_4G_MGR_DOCALL == dial_4g_mgr.mgr_state)
    {
        MODULE_LOG_I(TBOX4G, "it is dialing, abort call");
        dial_4g_mgr.mgr_state = DIAL_4G_MGR_ABORT_CALL;
        dial_4g_mgr.period = (uint32)DIAL_4G_ABORTCALL_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
    }
    else
    {
        dial_4g_mgr.mgr_state = DIAL_4G_MGR_DOSTOP;
        dial_4g_mgr.do_state = DIAL_4G_STOP_IDLE;
        dial_4g_mgr.cur_index = DIAL_4G_PUBLIC_APN;
        dial_4g_mgr.period = 0;
        if(FALSE == dial_4g_has_apn_need_stop((INT8)(-1), &next_index))
        {
            MODULE_LOG_I(TBOX4G, "it is not need to stop dial");
            TBOX_4G_MUTEX_LOCK();
            for(uint8 index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
            {
                dial_4g_mgr.net[index].state = DIAL_4G_STATE_DISCONNECTED;
            }
            TBOX_4G_MUTEX_UNLOCK();            
            dial_4g_mgr.mgr_state = DIAL_4G_MGR_STOP_FINISH;
            return 0;
        }

        dial_4g_mgr.cur_index = next_index;
        TBOX_4G_MUTEX_LOCK();
        dial_4g_mgr.net[next_index].state = DIAL_4G_STATE_DISCONNECTING;
        TBOX_4G_MUTEX_UNLOCK();

        dial_4g_seq.state = SEQ_4G_IDLE;
        dial_4g_seq.timeout = (uint32)DIAL_4G_STOP_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
        if(0 == seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &dial_4g_seq, (uint8)SEQ_4G_CONFLICT_ABORT))
        {
            dial_4g_mgr.do_state = DIAL_4G_STOP_DEACTIVE_APN;
        }
        else
        {
            dial_4g_mgr.do_state = DIAL_4G_STOP_DO_STOP;
            dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
        }
    }

    return 0;
}

uint8 dial_4g_get_mgr_state(void)
{
    return dial_4g_mgr.mgr_state;
}

void dial_4g_set_all_disconnect(void)
{
    uint8 index;

    if(DIAL_4G_MGR_INIT != dial_4g_mgr.mgr_state)
    {
        seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
    }

    dial_4g_mgr.mgr_state = DIAL_4G_MGR_INIT;
    dial_4g_mgr.do_state  = 0;
    dial_4g_mgr.cur_index = 0;
    dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
    dial_4g_mgr.period = 0;
    dial_4g_mgr.need_abort_call = 0;
    TBOX_4G_MUTEX_LOCK();
    for(index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        dial_4g_mgr.net[index].state = DIAL_4G_STATE_DISCONNECTED;
    }
    TBOX_4G_MUTEX_UNLOCK();
}

boolean dial_4g_is_all_connect(void)
{
    uint8 index;

    for(index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        if('\0' == dial_4g_mgr.net[index].apn[0])
        {
            continue;
        }
        if(DIAL_4G_STATE_CONNECTED != dial_4g_mgr.net[index].state)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean dial_4g_is_all_disconnect(void)
{
    uint8 index;

    for(index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        if('\0' == dial_4g_mgr.net[index].apn[0])
        {
            continue;
        }
        if(DIAL_4G_STATE_DISCONNECTED != dial_4g_mgr.net[index].state)
        {
            return FALSE;
        }
    }

    return TRUE;
}

sint8 dial_4g_get_callstate(uint8 index)
{
    uint8 state;

    if(index >= DIAL_4G_APN_MAX_INDEX)
    {
        return -1;
    }

    if('\0' == dial_4g_mgr.net[index].apn[0])
    {
        return -1;
    }

    TBOX_4G_MUTEX_LOCK();
    state = dial_4g_mgr.net[index].state;
    TBOX_4G_MUTEX_UNLOCK();

    return state;
}

void dial_4g_periodic(void)
{
    if(dial_4g_mgr.period > 0)
    {
        dial_4g_mgr.period--;
        if(dial_4g_mgr.period > 0)
        {
            return;
        }
    }

    /*abort call timeout*/
    if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state)
    {
        dial_4g_handle_notify_when_abort();
    }
    else if(DIAL_4G_MGR_DOCALL == dial_4g_mgr.mgr_state)
    {
        dial_4g_periodic_do_call();
    }
    else if(DIAL_4G_MGR_DOSTOP == dial_4g_mgr.mgr_state)
    {
        dial_4g_periodic_do_stop();
    }
    else
    {
        /**/
    }
}

static uint8 dial_4g_seq_resp(uint8 cmd, uint8 result)
{
    uint8 ret = SEQ_4G_CMD_NOMATCH;

    MODULE_LOG_I(TBOX4G, "mgrstate:%d dostate:%d cmd:%d result:%d", dial_4g_mgr.mgr_state,
                   dial_4g_mgr.do_state, cmd, result);

    if(SEQMGR_4G_CMD_EXE_TIMEOUT == result ||
       SEQMGR_4G_CMD_EXE_ABORT == result)
    {
        dial_4g_handle_resptimeout();
        return SEQ_4G_CMD_FINISH;
    }

    if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state ||
       DIAL_4G_MGR_DOCALL == dial_4g_mgr.mgr_state)
    {
        switch(dial_4g_mgr.do_state)
        {
            case DIAL_4G_CALL_WAITFOR_QUERY_APN_RES:
                  if(AT_4G_QUERY_APN == cmd)
                  {
                      if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state)
                      {
                          TBOX_4G_MUTEX_LOCK();
                          dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
                          TBOX_4G_MUTEX_UNLOCK();
                          dial_4g_handle_notify_when_abort();
                          ret = SEQ_4G_CMD_FINISH;
                      }
                      else
                      {
                          dial_4g_mgr.do_state = DIAL_4G_CALL_QUERY_APN;
                          dial_4g_apn_current_index++;
                          ret = SEQ_4G_CMD_ISMATCH;
                      }
                  }
                  break;

            case DIAL_4G_CALL_WAITFOR_DEACTIVE_RES:
                if(AT_4G_DEACTIVE_APN == cmd)
                {
                    if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state)
                    {
                        TBOX_4G_MUTEX_LOCK();
                        dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
                        TBOX_4G_MUTEX_UNLOCK();
                        dial_4g_handle_notify_when_abort();
                        ret = SEQ_4G_CMD_FINISH;
                    }
                    else
                    {
                        dial_4g_mgr.do_state = DIAL_4G_CALL_ACTIVE_APN;
                        ret = SEQ_4G_CMD_ISMATCH;
                    }
                }
                break;

            case DIAL_4G_CALL_WAITFOR_CONFIG_APN_RES:
                if(AT_4G_CONFIG_APN == cmd || AT_4G_CONFIG_APN_AUTH == cmd)
                {
                    if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state)
                    {
                        dial_4g_handle_notify_when_abort();
                        ret = SEQ_4G_CMD_FINISH;
                    }
                    else
                    {
                        if(SEQMGR_4G_CMD_EXE_OK == result)
                        {
                            uint8 next_index = 0;
                            if(TRUE == dial_4g_has_apn_need_call(dial_4g_apn_current_index, &next_index))
                            {
                                dial_4g_mgr.do_state = DIAL_4G_CALL_CONFIG_APN;
                                dial_4g_apn_current_index = next_index;
                                dial_4g_mgr.period = 0;
                                ret = SEQ_4G_CMD_ISMATCH;
                            }
                            else
                            {
                                dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
                                if(dial_4g_resetcount >= DIAL_4G_RESET4G_MAXCOUNT)
                                {
                                    dial_4g_mgr.do_state = DIAL_4G_CALL_QUERY_REG_STATE;
                                    ret = SEQ_4G_CMD_ISMATCH;
                                    MODULE_LOG_E(TBOX4G, "it is don't reset 4g when success to config apn, because the 4g reset count > 2");
                                }
                                else
                                {
                                    TBOX_4G_MUTEX_LOCK();
                                    dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
                                    TBOX_4G_MUTEX_UNLOCK();
                                    dial_4g_mgr.period = DIAL_4G_CONFIGAPN_TIMEOUT/PERIODIC_UNIT_4G;
                                    dial_4g_mgr.do_state = DIAL_4G_CALL_RESET4G;
                                    ret = SEQ_4G_CMD_FINISH;
                                }
                            }
                        }
                        else
                        {
                            uint8 next_index = 0;

                            MODULE_LOG_E(TBOX4G, "failed to config apn[%d]", dial_4g_apn_current_index);
                            if(TRUE == dial_4g_has_apn_need_call(dial_4g_apn_current_index, &next_index))
                            {
                                dial_4g_mgr.do_state = DIAL_4G_CALL_CONFIG_APN;
                                dial_4g_apn_current_index = next_index;
                                dial_4g_mgr.period = 0;
                                ret = SEQ_4G_CMD_ISMATCH;
                            }
                            else
                            {
                                TBOX_4G_MUTEX_LOCK();
                                dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
                                TBOX_4G_MUTEX_UNLOCK();
                                dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
                                dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
                                dial_4g_mgr.do_state = DIAL_4G_CALL_IDLE;
                                dial_4g_mgr.cur_index = 0;
                                dial_4g_mgr.period = 0;
                                ret = SEQ_4G_CMD_FINISH;
                            }
                        }
                    }
                }
                break;

            case DIAL_4G_CALL_WAITFOR_ACTIVE_APN_RES:
                if(AT_4G_ACTIVE_APN == cmd)
                {
                    uint8 next_index = 0;

                    if(SEQMGR_4G_CMD_EXE_OK == result)
                    {
                        dial_4g_mgr.do_state = DIAL_4G_CALL_FINISH;
                        TBOX_4G_MUTEX_LOCK();
                        dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_CONNECTED;
                        TBOX_4G_MUTEX_UNLOCK();

                        if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state)
                        {
                            dial_4g_handle_notify_when_abort();
                            ret = SEQ_4G_CMD_FINISH;
                        }
                        else
                        {
                            if(TRUE == dial_4g_has_apn_need_call(dial_4g_mgr.cur_index, &next_index))
                            {
                                dial_4g_mgr.do_state = DIAL_4G_CALL_DEAVTIVE_APN;
                                dial_4g_mgr.cur_index = next_index;
                                dial_4g_mgr.period = 0;
                                TBOX_4G_MUTEX_LOCK();
                                dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_CONNECTING;
                                TBOX_4G_MUTEX_UNLOCK();
                                ret = SEQ_4G_CMD_ISMATCH;
                            }
                            else
                            {
                                dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
                                dial_4g_mgr.cur_index = 0;
                                dial_4g_mgr.period = 0;
                                ret = SEQ_4G_CMD_FINISH;
                            }
                        }
                    }
                    else
                    {
                        MODULE_LOG_E(TBOX4G, "failed to active apn");

                        dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
                        if(TRUE == dial_4g_has_apn_need_call(dial_4g_mgr.cur_index, &next_index))
                        {
                            dial_4g_mgr.do_state = DIAL_4G_CALL_DEAVTIVE_APN;
                            dial_4g_mgr.cur_index = next_index;
                            TBOX_4G_MUTEX_LOCK();
                            dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_CONNECTING;
                            TBOX_4G_MUTEX_UNLOCK();
                            dial_4g_mgr.period = 0;
                            ret = SEQ_4G_CMD_ISMATCH;
                        }
                        else
                        {
                            dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
                            dial_4g_mgr.do_state = DIAL_4G_CALL_FINISH;
                            dial_4g_mgr.cur_index = 0;
                            dial_4g_mgr.period = 0;
                            ret = SEQ_4G_CMD_FINISH;
                        }
                    }
                }
                break;

            default:
                break;
        }
    }
    else if(DIAL_4G_MGR_DOSTOP == dial_4g_mgr.mgr_state)
    {
        switch(dial_4g_mgr.do_state)
        {
            case DIAL_4G_STOP_WAITFOR_DEACTIVE_APN_RES:
                if(AT_4G_DEACTIVE_APN == cmd)
                {
                    uint8 next_index = 0;
                    TBOX_4G_MUTEX_LOCK();
                    dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
                    TBOX_4G_MUTEX_UNLOCK();
                    if(TRUE == dial_4g_has_apn_need_stop(dial_4g_mgr.cur_index, &next_index))
                    {
                        dial_4g_mgr.do_state = DIAL_4G_STOP_DEACTIVE_APN;
                        dial_4g_mgr.cur_index = next_index;
                        TBOX_4G_MUTEX_LOCK();
                        dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTING;
                        TBOX_4G_MUTEX_UNLOCK();
                        dial_4g_mgr.period = 0;
                        ret = SEQ_4G_CMD_ISMATCH;
                    }
                    else
                    {
                        dial_4g_mgr.do_state = DIAL_4G_STOP_FINISH;
                        dial_4g_mgr.mgr_state = DIAL_4G_MGR_STOP_FINISH;
                        dial_4g_mgr.cur_index = 0;
                        dial_4g_mgr.period = 0;
                        ret = SEQ_4G_CMD_FINISH;
                    }
                }
                break;

            default:
                break;
        }
    }
    else
    {
        /**/
    }

    return ret;
}

static boolean  dial_4g_has_apn_need_call(INT8 cur_index, uint8 *next_call_index)
{
    uint8 index;

    for(index = DIAL_4G_PUBLIC_APN; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        if(index == cur_index)
        {
            continue;
        }

        if('\0' == dial_4g_mgr.net[index].apn[0])
        {
            continue;
        }

        if(DIAL_4G_STATE_CONNECTED == dial_4g_mgr.net[index].state)
        {
            continue;
        }

        *next_call_index = index;
        break;
    }
    if(index >= DIAL_4G_APN_MAX_INDEX)
    {
        return FALSE;
    }

    return TRUE;
}

static boolean dial_4g_has_apn_need_stop(INT8 cur_index, uint8 *next_call_index)
{
    uint8 index;

    for(index = DIAL_4G_PUBLIC_APN; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        if(index == cur_index)
        {
            continue;
        }

        if('\0' == dial_4g_mgr.net[index].apn[0])
        {
            continue;
        }

        if(DIAL_4G_STATE_CONNECTED == dial_4g_mgr.net[index].state)
        {
            *next_call_index = index;
            break;
        }
    }
    if(index >= DIAL_4G_APN_MAX_INDEX)
    {
        return FALSE;
    }

    return TRUE;
}


static void dial_4g_periodic_do_call(void)
{
    /*MODULE_LOG_I(TBOX4G, "do call mgrstate:%d dostate:%d ", dial_4g_mgr.mgr_state,
                   dial_4g_mgr.do_state);*/

    switch(dial_4g_mgr.do_state)
    {
        case DIAL_4G_CALL_DO_CALL:
            dial_4g_seq.state = SEQ_4G_IDLE;
            dial_4g_seq.timeout = (uint32)DIAL_4G_CALL_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
            if(0 == seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &dial_4g_seq, (uint8)SEQ_4G_CONFLICT_ABORT))
            {
                dial_4g_mgr.do_state = DIAL_4G_CALL_QUERY_APN;
            }
            else
            {
                dial_4g_mgr.do_state = DIAL_4G_CALL_DO_CALL;
                dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case DIAL_4G_CALL_QUERY_APN:
            {
                uint8 index = dial_4g_apn_current_index;
                for(; index < DIAL_4G_APN_MAX_INDEX; index++)
                {
                    if(0 == strlen((char*)dial_4g_mgr.net[index].apn))
                    {
                        continue;
                    }
                    break;
                }
                if(index >= DIAL_4G_APN_MAX_INDEX) /*查询APN完成*/
                {
                    for(index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
                    {
                        if(0 == strlen((char*)dial_4g_mgr.net[index].apn))
                        {
                            continue;
                        }
                        if(FALSE == net_4g_apn_isalready_config(index+1, dial_4g_mgr.net[index].apn))
                        {
                            break;
                        }
                    }
                    if(index >= DIAL_4G_APN_MAX_INDEX) /*不需要配置APN*/
                    {
                        dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
                        dial_4g_mgr.do_state = DIAL_4G_CALL_QUERY_REG_STATE;
                    }
                    else
                    {
                        dial_4g_apn_current_index = index;
                        dial_4g_mgr.period = DIAL_4G_CONFIGAPN_TIMEOUT/PERIODIC_UNIT_4G;
                        dial_4g_mgr.do_state = DIAL_4G_CALL_CONFIG_APN;
                    }
                    
                    MODULE_LOG_I(TBOX4G, "no need query apn index:%d do_state:%d", dial_4g_apn_current_index, dial_4g_mgr.do_state);
                }
                else
                {
                    dial_4g_apn_current_index = index;
                    net_4g_set_apn(index+1, dial_4g_mgr.net[index].apn);
                    if(0 == net_4g_apn_query_apn(AT_4G_CMD_LOW, index+1)) /*查询APN*/
                    {
                        dial_4g_mgr.do_state = DIAL_4G_CALL_WAITFOR_QUERY_APN_RES;
                    }
                    else
                    {
                        dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
                    }
                    MODULE_LOG_I(TBOX4G, "query apn index:%d do_state:%d", dial_4g_apn_current_index, dial_4g_mgr.do_state);
                }
            }
            break;

        case DIAL_4G_CALL_CONFIG_APN:
            if(dial_4g_apn_current_index >= DIAL_4G_APN_MAX_INDEX)
            {
                dial_4g_apn_current_index = DIAL_4G_PUBLIC_APN;
                dial_4g_mgr.do_state = DIAL_4G_CALL_QUERY_REG_STATE;
                break;
            }
            if(0 == net_4g_config_apn(AT_4G_CMD_LOW, dial_4g_apn_current_index+1,
                                      dial_4g_mgr.net[dial_4g_apn_current_index].apn,
                                      dial_4g_mgr.net[dial_4g_apn_current_index].usrname,
                                      dial_4g_mgr.net[dial_4g_apn_current_index].password))
            {
                dial_4g_mgr.do_state = DIAL_4G_CALL_WAITFOR_CONFIG_APN_RES;
            }
            else
            {
                dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case DIAL_4G_CALL_RESET4G:
            MODULE_LOG_I(TBOX4G, "after config apn, reset 4g");
            dial_4g_mgr.period = 0;
            dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
            dial_4g_mgr.do_state = DIAL_4G_CALL_IDLE;
            tbox_pm_reboot(TBOX_PM_REBOOT_4G);
            vTaskDelay(pdMS_TO_TICKS(100U));
            mgr_4g_reset_sequence();
            dial_4g_resetcount++;
            break;

        case DIAL_4G_CALL_QUERY_REG_STATE:
            if((NET_4G_REGISTERED == net_4g_info.ereg_state ||
                NET_4G_ROAMING == net_4g_info.ereg_state ||
                NET_4G_REGISTERED == net_4g_info.gprs_reg_state ||
                NET_4G_ROAMING == net_4g_info.gprs_reg_state))
            {
                dial_4g_mgr.do_state = DIAL_4G_CALL_DEAVTIVE_APN;
            }
            else
            {
                MODULE_LOG_I(TBOX4G, "cereg state:%d, gprs state:%d retry:%d", net_4g_info.ereg_state, 
                                        net_4g_info.gprs_reg_state, dial_4g_fetchcount);
                if(dial_4g_fetchcount < DIAL_4G_REFETCH_REG_MAXCOUNT)
                {
                    dial_4g_fetchcount++;
                    dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
                    dial_4g_mgr.do_state = DIAL_4G_CALL_IDLE;
                    dial_4g_mgr.cur_index = 0;
                    dial_4g_mgr.period = 0;
                }
                else
                {
                    dial_4g_mgr.do_state = DIAL_4G_CALL_DEAVTIVE_APN;
                }
            }
            break;

        case DIAL_4G_CALL_DEAVTIVE_APN:
            if(dial_4g_mgr.cur_index >= DIAL_4G_APN_MAX_INDEX)
            {
                uint8 temp = 0;
                if(FALSE == dial_4g_has_apn_need_call(-1, &temp))
                {
                    dial_4g_mgr.cur_index = 0;
                    dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
                    dial_4g_mgr.do_state = DIAL_4G_CALL_FINISH;
                    return;
                }
                else
                {
                    dial_4g_mgr.cur_index = temp;
                }
            }
            TBOX_4G_MUTEX_LOCK();
            dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_CONNECTING;
            TBOX_4G_MUTEX_UNLOCK();
            if(0 == net_4g_deactive_apn(AT_4G_CMD_LOW, (uint8)(dial_4g_mgr.cur_index+1)))
            {
                dial_4g_mgr.do_state = DIAL_4G_CALL_WAITFOR_DEACTIVE_RES;
            }
            else
            {
                dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case DIAL_4G_CALL_ACTIVE_APN:
            if(0 == net_4g_active_apn(AT_4G_CMD_LOW, (uint8)(dial_4g_mgr.cur_index+1)))
            {
                dial_4g_mgr.do_state = DIAL_4G_CALL_WAITFOR_ACTIVE_APN_RES;
            }
            else
            {
                dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        default:
            break;
    }
}

static void dial_4g_periodic_do_stop(void)
{
    MODULE_LOG_I(TBOX4G, "do stop mgrstate:%d dostate:%d ", dial_4g_mgr.mgr_state,
                   dial_4g_mgr.do_state);

    switch(dial_4g_mgr.do_state)
    {
        case DIAL_4G_STOP_DO_STOP:
            dial_4g_seq.state = SEQ_4G_IDLE;
            dial_4g_seq.timeout = (uint32)DIAL_4G_STOP_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
            TBOX_4G_MUTEX_LOCK();
            dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTING;
            TBOX_4G_MUTEX_UNLOCK();
            if(0 == seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &dial_4g_seq, (uint8)SEQ_4G_CONFLICT_ABORT))
            {
                dial_4g_mgr.do_state = DIAL_4G_STOP_DEACTIVE_APN;
            }
            else
            {
                dial_4g_mgr.do_state = DIAL_4G_STOP_DO_STOP;
                dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case DIAL_4G_STOP_DEACTIVE_APN:
            if(0 == net_4g_deactive_apn(AT_4G_CMD_LOW, (uint8)(dial_4g_mgr.cur_index+1)))
            {
                dial_4g_mgr.do_state = DIAL_4G_STOP_WAITFOR_DEACTIVE_APN_RES;
            }
            else
            {
                dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        default:
            break;
    }
}

static void dial_4g_handle_notify_when_abort(void)
{
    uint8 next_index = 0;

    dial_4g_mgr.mgr_state = DIAL_4G_MGR_DOSTOP;
    dial_4g_mgr.do_state = DIAL_4G_STOP_IDLE;
    dial_4g_mgr.cur_index = DIAL_4G_PUBLIC_APN;
    dial_4g_mgr.period = 0;
    if(FALSE == dial_4g_has_apn_need_stop((INT8)(-1), &next_index))
    {
        TBOX_4G_MUTEX_LOCK();
        for(uint8 index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
        {
            dial_4g_mgr.net[index].state = DIAL_4G_STATE_DISCONNECTED;
        }
        TBOX_4G_MUTEX_UNLOCK();         
        dial_4g_mgr.mgr_state = DIAL_4G_MGR_STOP_FINISH;
        return;
    }

    dial_4g_mgr.cur_index = next_index;
    TBOX_4G_MUTEX_LOCK();
    dial_4g_mgr.net[next_index].state = DIAL_4G_STATE_DISCONNECTING;
    TBOX_4G_MUTEX_UNLOCK();

    dial_4g_seq.state = SEQ_4G_IDLE;
    dial_4g_seq.timeout = (uint32)DIAL_4G_STOP_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
    if(0 == seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &dial_4g_seq, (uint8)SEQ_4G_CONFLICT_ABORT))
    {
        dial_4g_mgr.do_state = DIAL_4G_STOP_DEACTIVE_APN;
    }
    else
    {
        dial_4g_mgr.do_state = DIAL_4G_STOP_DO_STOP;
        dial_4g_mgr.period = DIAL_4G_RETRY_PERIOD/PERIODIC_UNIT_4G;
    }
}

static void dial_4g_handle_resptimeout(void)
{
    if(DIAL_4G_MGR_DOCALL == dial_4g_mgr.mgr_state)
    {
        if(dial_4g_mgr.cur_index >= DIAL_4G_APN_MAX_INDEX)
        {
            return;
        }
        TBOX_4G_MUTEX_LOCK();
        dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
        TBOX_4G_MUTEX_UNLOCK();
        dial_4g_mgr.mgr_state = DIAL_4G_MGR_CALL_FINISH;
        dial_4g_mgr.do_state = DIAL_4G_CALL_IDLE;
        dial_4g_mgr.cur_index = 0;
        dial_4g_mgr.period = 0;
    }
    else if(DIAL_4G_MGR_ABORT_CALL == dial_4g_mgr.mgr_state ||
            DIAL_4G_MGR_DOSTOP == dial_4g_mgr.mgr_state)
    {
        if(dial_4g_mgr.cur_index >= DIAL_4G_APN_MAX_INDEX)
        {
            return;
        }
        TBOX_4G_MUTEX_LOCK();
        dial_4g_mgr.net[dial_4g_mgr.cur_index].state = DIAL_4G_STATE_DISCONNECTED;
        TBOX_4G_MUTEX_UNLOCK();
        dial_4g_mgr.do_state = DIAL_4G_STOP_FINISH;
        dial_4g_mgr.mgr_state = DIAL_4G_MGR_STOP_FINISH;
        dial_4g_mgr.cur_index = 0;
        dial_4g_mgr.period = 0;
    }
    else
    {
        /**/
    }
}

static void dial_4g_cfg_change(const char *name, TBOX_MSG_DATA *data)
{
    char *new_apn_buffer;
    char *apn_name[DIAL_4G_APN_MAX_INDEX] = {"PUBAPN", "PRIAPN","OTAAPN"};

    TBOX_CFG_CHANGE_INFO *change_info = NULL_PTR;
    if(0U != strncmp(TBOX_CFG_EVENT_VALUE_CHANGE, name, strlen(TBOX_CFG_EVENT_VALUE_CHANGE)) ||
       NULL_PTR == data)
    {
        return;
    }
    if(data->size < sizeof(TBOX_CFG_CHANGE_INFO))
    {
        return;
    }
    change_info = (TBOX_CFG_CHANGE_INFO *)data->data;
    if(NULL_PTR == change_info)
    {
        return;
    }

    new_apn_buffer = (char *)mempool_alloc(TBOX_CFG_APN_LEN+1U);
    if(NULL_PTR == new_apn_buffer)
    {
        MODULE_LOG_E(TBOX4G, "failed to alloc memory for apn");
        return;
    }
    memset(new_apn_buffer, 0U, TBOX_CFG_APN_LEN);

    for(uint8 index = 0; index < DIAL_4G_APN_MAX_INDEX; index++)
    {
        if(0 != strcmp(apn_name[index], change_info->name))
        {
            continue;
        }

        tbox_cfg_read(change_info->id, new_apn_buffer);
        if(0U == strlen(new_apn_buffer))
        {
            MODULE_LOG_E(TBOX4G, "apn is empty");
            break;
        }
        if(0 == strncmp(new_apn_buffer, (char*)dial_4g_mgr.net[index].apn, DIAL_4G_APN_LEN))
        {
            break;
        }
        
        MODULE_LOG_I(TBOX4G, "apn[%d] cfg change old:%s new:%s", index, dial_4g_mgr.net[index].apn, new_apn_buffer);
        strncpy((char*)dial_4g_mgr.net[index].apn, new_apn_buffer, DIAL_4G_APN_LEN);
        dial_4g_stopcall();
        break;
    }

    mempool_free(new_apn_buffer);
}

static void dial_4g_cfg_setdefault(const char *name, TBOX_MSG_DATA *data)
{
    UNUSED(name);
    UNUSED(data);

    char *new_apn_buffer;
	TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(PUBAPN, cfg_id);

    new_apn_buffer = (char *)mempool_alloc(TBOX_CFG_APN_LEN+1U);
    if(NULL_PTR == new_apn_buffer)
    {
        MODULE_LOG_E(TBOX4G, "failed to alloc memory for apn");
        return;
    }
    memset(new_apn_buffer, 0U, TBOX_CFG_APN_LEN);

    tbox_cfg_read(cfg_id, new_apn_buffer);
    if(0U == strlen(new_apn_buffer))
    {
        MODULE_LOG_W(TBOX4G, "apn is empty");
        mempool_free(new_apn_buffer);  
        return;
    }
    if(0 == strncmp(new_apn_buffer, (char*)dial_4g_mgr.net[DIAL_4G_PUBLIC_APN].apn, DIAL_4G_APN_LEN))
    {
        MODULE_LOG_W(TBOX4G, "the apn is same as default");
        mempool_free(new_apn_buffer);  
        return;
    }

    MODULE_LOG_I(TBOX4G, "public apn cfg change old:%s new:%s", dial_4g_mgr.net[DIAL_4G_PUBLIC_APN].apn, new_apn_buffer);
    strncpy((char*)dial_4g_mgr.net[DIAL_4G_PUBLIC_APN].apn, new_apn_buffer, DIAL_4G_APN_LEN);
    dial_4g_stopcall();

    mempool_free(new_apn_buffer);
}