#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_sms.h"
#include "4g_info.h"
#include "4g_ftp.h"
#include "4g_sequence_mgr.h"
#include "4g_if.h"

#define INFO_4G_SEQ_TIMEOUT     (180*1000) //180s
#define INFO_4G_PERIOD          (1000)     //1s
#define INFO_4G_WAIT_FOR_TMOUT  (3000)     //3S

static uint8 info_4g_queryseq_resp(uint8 cmd, uint8 result);

static void  info_4g_hanlde_query(void);

static void  info_4g_handle_resptimeout(void);

static uint8  info_4g_state;
static uint16 info_4g_period;
static SEQ_4G info_4g_query_seq = {SEQ_4G_IDLE,
                                   (uint32)INFO_4G_SEQ_TIMEOUT/(uint32)PERIODIC_UNIT_4G,
                                   NULL,
                                   NULL,
                                   info_4g_queryseq_resp
                                  };

void info_4g_init(void)
{
    info_4g_state = INFO_4G_IDLE;
    info_4g_period = 0;
}

void info_4g_doquery(void)
{
    info_4g_period = 0;
    info_4g_state = INFO_4G_IDLE;

    /*FTP下载中，不查询4G信息*/
    if(TRUE == ftp_4g_isdownloading())
    {
        MODULE_LOG_I(TBOX4G, "ftp downloading, not query 4G info");
        return;
    }

    info_4g_query_seq.state   = SEQ_4G_IDLE;
    info_4g_query_seq.timeout = (uint32)INFO_4G_SEQ_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
    if(0 != seqmgr_4g_doseq(SEQ_4G_PRI_LOW, &info_4g_query_seq, SEQ_4G_CONFLICT_NOCONFICT))
    {
        return;
    }
    
    info_4g_state = INFO_4G_TESTALIVE;
    info_4g_hanlde_query();
}

void info_4g_stop_query(void)
{
    if(info_4g_state > INFO_4G_IDLE && info_4g_state < INFO_4G_FINISH)
    {
        seqmgr_4g_resetseq(SEQ_4G_PRI_HIGH);
    }
    info_4g_state = INFO_4G_STOP;
    info_4g_period = 0;
}

void info_4g_periodic(void)
{
    if(info_4g_period > 0)
    {
        info_4g_period--;
        if(info_4g_period > 0)
        {
            return;
        }
    }

    switch(info_4g_state)
    {
        case INFO_4G_DOQUERY:
            info_4g_query_seq.state   = SEQ_4G_IDLE;
            info_4g_query_seq.timeout = (uint32)INFO_4G_SEQ_TIMEOUT/(uint32)PERIODIC_UNIT_4G;
            if(0 != seqmgr_4g_doseq(SEQ_4G_PRI_LOW, &info_4g_query_seq, SEQ_4G_CONFLICT_NOCONFICT))
            {
                info_4g_period = 0;
                info_4g_state = INFO_4G_IDLE;
                break;
            }

            info_4g_state = INFO_4G_TESTALIVE;
            info_4g_hanlde_query();
            break;
        
        case INFO_4G_TESTALIVE:
            if(0 == modem_4g_test_alive(AT_4G_CMD_LOW, 1))
            {
                info_4g_state = INFO_4G_TESTALIVE_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_SIM_INITSTATE:
            if(0 == sim_4g_query_initstate(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_SIM_INITSTATE_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_IMEI:
            if(0 == modem_4g_query_imei(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_IMEI_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_IMSI:
            if(0 == sim_4g_query_imsi(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_IMSI_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_ICCID:
            if(0 == sim_4g_query_iccid(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_ICCID_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_CENTERNUM:
            if(0 == sms_4g_query_centernum(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_CENTERNUM_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_SELECT_PB_MEM:
            if(0 == sim_4g_enable_setnum(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_SELECT_PB_MEM_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_PHONENUM:
            if(0 == sim_4g_query_num(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_PHONENUM_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_CELLID:
            if(0 == net_4g_get_lac_cellid(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_CELLID_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_SET_SMS_PDUMODE:
            if(0 == sms_4g_set_pdumode(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_SET_SMS_PDUMODE_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_GET_OPERATOR:
            if(0 == net_4g_get_operator_info(AT_4G_CMD_MID))
            {
                info_4g_state = INFO_4G_WAITFOR_OPERATOR_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_SIGNALSTRENGTH:
            if(0 == net_4g_query_signalstrength(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_SIGNALSTRENGTH_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_CREG:
            if(0 == net_4g_creg(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_CREG_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;
        
        case INFO_4G_QUERY_CGREG:
            if(0 == net_4g_grps_reg(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_CGREG_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;
        
        case INFO_4G_QUERY_CEREG:
            if(0 == net_4g_ereg(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_CEREG_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;
        
        case INFO_4G_QUERY_TEMPERATURE:
            if(0U == modem_4g_query_temp(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_TEMPERATURE_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case INFO_4G_QUERY_MTID:
            if(0U == modem_4g_query_mtid(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_MTID_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;
        
        case INFO_4G_QUERY_TAID:
            if(0U == modem_4g_query_taid(AT_4G_CMD_LOW))
            {
                info_4g_state = INFO_4G_WAITFOR_QUERY_TAID_RES;
            }
            else
            {
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        default:
            break;
    }
}

uint8 info_4g_get_state(void)
{
    return info_4g_state;
}

static void  info_4g_hanlde_query(void)
{
    uint8  index;
    boolean need_handle = FALSE;

    for(index = INFO_4G_IDLE; index < INFO_4G_FINISH; index++)
    {
        switch(info_4g_state)
        {
            case INFO_4G_DOQUERY:
                need_handle = TRUE;
                info_4g_period = INFO_4G_PERIOD/PERIODIC_UNIT_4G;
                break;
            
            case INFO_4G_TESTALIVE:
                need_handle = TRUE;
                break;

            case INFO_4G_QUERY_SIM_INITSTATE:
                if(SIM_4G_INITSTATE_ALLREADY == sim_4g_get_initstate())
                {
                    info_4g_state = INFO_4G_QUERY_IMEI;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            case INFO_4G_QUERY_IMEI:
                if(TRUE == modem_4g_info.imei_is_valid)
                {
                    info_4g_state = INFO_4G_QUERY_IMSI;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            case INFO_4G_QUERY_IMSI:
                if(TRUE == sim_4g_info.imsi_is_valid)
                {
                    info_4g_state = INFO_4G_QUERY_ICCID;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            case INFO_4G_QUERY_ICCID:
                /*if(TRUE == sim_4g_info.iccid_is_valid)
                {
                    info_4g_state = INFO_4G_QUERY_CENTERNUM;
                }
                else
                {
                    need_handle = TRUE;
                }*/
                need_handle = TRUE;
                break;

            case INFO_4G_QUERY_CENTERNUM:
                if(TRUE == sms_4g_info.centernum_valid)
                {
                    info_4g_state = INFO_4G_SELECT_PB_MEM;
                }
                else
                {
                    if(SIM_4G_INITSTATE_SMSDONE != (SIM_4G_INITSTATE_SMSDONE & sim_4g_get_initstate()))
                    {
                        info_4g_state = INFO_4G_SELECT_PB_MEM;
                    }
                    else
                    {
                        need_handle = TRUE;
                    }
                }
                break;

            case INFO_4G_SELECT_PB_MEM:
                if(SIM_4G_ENABLE_SETNUM == sim_4g_info.setnum_flag)
                {
                    info_4g_state = INFO_4G_QUERY_PHONENUM;
                }
                else
                {
                    if(SIM_4G_INITSTATE_PBDONE != (SIM_4G_INITSTATE_PBDONE & sim_4g_get_initstate()))
                    {
                        info_4g_state = INFO_4G_QUERY_PHONENUM;
                    }
                    else
                    {
                        need_handle = TRUE;
                    }
                }
                break;

            case INFO_4G_QUERY_PHONENUM:
                need_handle = TRUE;
                break;

            case INFO_4G_QUERY_CELLID:
                if(TRUE == net_4g_info.laccellid_valid)
                {
                    info_4g_state = INFO_4G_SET_SMS_PDUMODE;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            case INFO_4G_SET_SMS_PDUMODE:
                if(SMS_4G_MODE_PDU == sms_4g_info.sms_mode)
                {
                    info_4g_state = INFO_4G_GET_OPERATOR;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            case INFO_4G_GET_OPERATOR:
                if(TRUE == net_4g_info.operator_valid)
                {
                    info_4g_state = INFO_4G_QUERY_SIGNALSTRENGTH;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            case INFO_4G_QUERY_SIGNALSTRENGTH:
                need_handle = TRUE;
                break;
            
            case INFO_4G_QUERY_CREG:
                need_handle = TRUE;
                break;
            
            case INFO_4G_QUERY_CGREG:
                need_handle = TRUE;
                break;
            
            case INFO_4G_QUERY_CEREG:
                need_handle = TRUE;
                break;
            
            case INFO_4G_QUERY_TEMPERATURE:
                need_handle = TRUE;
                break;
            
            case INFO_4G_QUERY_MTID:
                if(TRUE == modem_4g_info.mtid_is_valid)
                {
                    info_4g_state = INFO_4G_QUERY_TAID;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;
            
            case INFO_4G_QUERY_TAID:
                if(TRUE == modem_4g_info.taid_is_valid)
                {
                    info_4g_state = INFO_4G_FINISH;
                }
                else
                {
                    need_handle = TRUE;
                }
                break;

            default:
                break;
        }
        if(need_handle)
        {
            break;
        }
    }
}

static uint8 info_4g_queryseq_resp(uint8 cmd, uint8 result)
{
    uint8 ret = SEQ_4G_CMD_NOMATCH;

    UNUSED(result);

    MODULE_LOG_I(TBOX4G, "query sequence resp info_4g_state:%d cmd:%d result:%d", info_4g_state,
                   cmd, result);
    
    if(SEQMGR_4G_CMD_EXE_TIMEOUT == result ||
       SEQMGR_4G_CMD_EXE_ABORT == result)
    {
        info_4g_handle_resptimeout();
        return SEQ_4G_CMD_FINISH;
    }

    /*FTP下载中，终止查询4G信息*/
    if(TRUE == ftp_4g_isdownloading())
    {
        info_4g_state = INFO_4G_FINISH;
        info_4g_period = 0U;
        MODULE_LOG_I(TBOX4G, "ftp downloading, stop query 4G info");
        return SEQ_4G_CMD_FINISH;
    }

    switch(info_4g_state)
    {
        case INFO_4G_TESTALIVE_RES:
            if(AT_4G_TESTALIVE_CMD == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_SIM_INITSTATE;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_SIM_INITSTATE_RES:
            if(AT_4G_QUERY_INITSTATE == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_IMEI;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_IMEI_RES:
            if(AT_4G_QUERY_IMEI == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_IMSI;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_IMSI_RES:
            if(AT_4G_QUERY_IMSI == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_ICCID;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_ICCID_RES:
            if(AT_4G_QUERY_ICCID == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_CENTERNUM;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_CENTERNUM_RES:
            if(AT_4G_QUERY_CENTERNUM == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_SELECT_PB_MEM;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_SELECT_PB_MEM_RES:
            if(AT_4G_ENABLE_SETNUM == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_PHONENUM;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_PHONENUM_RES:
            if(AT_4G_QUERY_NUM == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_CELLID;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_CELLID_RES:
            if(AT_4G_GPRS_REG == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_SET_SMS_PDUMODE;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_SET_SMS_PDUMODE_RES:
            if(AT_4G_SMS_SETPDUMODE == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_GET_OPERATOR;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_OPERATOR_RES:
            if(AT_4G_GET_OPERATOR == cmd)
            {
                if(TRUE == net_4g_info.operator_valid)
                {
                    MODULE_LOG_I(TBOX4G, "network operator name:%s act:%d",
                        net_4g_info.operator.name, net_4g_info.operator.act);
                }
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_SIGNALSTRENGTH;
                info_4g_hanlde_query();
            }
            break;

        case INFO_4G_WAITFOR_QUERY_SIGNALSTRENGTH_RES:
            if(AT_4G_QUERY_SIGNALSTRENGTH == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_CREG;
                info_4g_hanlde_query();
            }
            break;
        
        case INFO_4G_WAITFOR_QUERY_CREG_RES:
            if(AT_4G_CREG == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_CGREG;
                info_4g_hanlde_query();
            }
            break;
        
        case INFO_4G_WAITFOR_QUERY_CGREG_RES:
            if(AT_4G_GPRS_REG == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_CEREG;
                info_4g_hanlde_query();
            }
            break;
        
        case INFO_4G_WAITFOR_QUERY_CEREG_RES:
            if(AT_4G_CEREG == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_TEMPERATURE;
                info_4g_hanlde_query();
            }
            break;
        
        case INFO_4G_WAITFOR_QUERY_TEMPERATURE_RES:
            if(AT_4G_QUERY_TEMP == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_MTID;
                info_4g_hanlde_query();
            }
            break;
        
        case INFO_4G_WAITFOR_QUERY_MTID_RES:
            if(AT_4G_QUERY_MTID == cmd)
            {
                ret = SEQ_4G_CMD_ISMATCH;
                info_4g_state = INFO_4G_QUERY_TAID;
                info_4g_hanlde_query();
            }
            break;
        
        case INFO_4G_WAITFOR_QUERY_TAID_RES:
            if(AT_4G_QUERY_TAID == cmd)
            {
                ret = SEQ_4G_CMD_FINISH;
                info_4g_state = INFO_4G_FINISH;
                info_4g_period = 0;
            }
            break;

        default:
            break;
    }
    if(INFO_4G_FINISH == info_4g_state && ret != SEQ_4G_CMD_FINISH)
    {
        ret = SEQ_4G_CMD_FINISH;
    }

    return ret;
}

static void  info_4g_handle_resptimeout(void)
{
    if(INFO_4G_IDLE == info_4g_state ||
       INFO_4G_FINISH == info_4g_state ||
       INFO_4G_STOP == info_4g_state)
    {
        return;
    }

    info_4g_state = INFO_4G_FINISH;
    info_4g_period = 0;
}
