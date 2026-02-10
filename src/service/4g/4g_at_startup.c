#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_at_startup.h"
#include "4g_sequence_mgr.h"

#define AT_4G_STARTUP_SEQ_TIMEOUT  (90*1000) //90s
#define AT_4G_ABORT_SEQ_TIMEOUT    (10*1000) //10s
#define AT_4G_STARTUP_RETRY_PERIOD (1000)    //1s
#define AT_4G_STARTUP_TESTLIVE_RETRY_COUNT 20

static uint8 at_4g_startupseq_resp(uint8 cmd, uint8 result);
static void at_4g_startup_handle_resptimeout(void);

static uint8 at_4g_startup_state;
static uint16 at_4g_period;
static uint16 at_4g_abort_period;
static SEQ_4G at_4g_startup_seq = {SEQ_4G_IDLE,
                                   AT_4G_STARTUP_SEQ_TIMEOUT/PERIODIC_UNIT_4G,
                                   NULL,
                                   NULL,
                                   at_4g_startupseq_resp
                                   };

/*
 * 4G 模块启动AT交互时序:
 * 1、利用“AT”探测4G是否4G模块启动，并且AT是否可正常响应
 * 2、读和写波特率
 * 3、设置AT命令不回显
 * 4、设置上报CMEE错误
 * 5、设置搜网优先级
 * 6、打开4G模块全功能
 * 7、设置自动搜网
 * 8、通过NITZ更新时区
 * 9、配置通过串口输出URC
 */
void at_4g_startup_init(void)
{
    at_4g_startup_state = AT_4G_STARTUP_IDLE;
    at_4g_period = 0;
    at_4g_abort_period = 0;
}

void at_4g_dostartup(void)
{
    if(FALSE == seqmgr_4g_is_allseq_idle())
    {
        seqmgr_4g_abort_allseq(AT_4G_ABORT_SEQ_TIMEOUT/PERIODIC_UNIT_4G);
        at_4g_abort_period = AT_4G_ABORT_SEQ_TIMEOUT/PERIODIC_UNIT_4G;
        MODULE_LOG_E(TBOX4G, "need to do startup, abort other sequence");
        return;
    }
    at_4g_abort_period = 0;
    at_4g_period = 0;
    at_4g_startup_state = AT_4G_STARTUP_WAIRFOR_4GMODULE_STARTUP;
    at_4g_startup_seq.state = SEQ_4G_IDLE;
    at_4g_startup_seq.timeout = (AT_4G_STARTUP_SEQ_TIMEOUT/PERIODIC_UNIT_4G);
    seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &at_4g_startup_seq, SEQ_4G_CONFLICT_ABORT);
}

void at_4g_dostop_startup(void)
{
    if(at_4g_startup_state == AT_4G_STARTUP_SEQ_STOP)
    {
        return;
    }

    if(at_4g_startup_state != AT_4G_STARTUP_IDLE &&
       at_4g_startup_state != AT_4G_STARTUP_SEQ_FINISH)
    {
        seqmgr_4g_resetseq(SEQ_4G_PRI_HIGH);
    }
    if(at_4g_abort_period > 0)
    {
        at_4g_abort_period = 0;
    }
    at_4g_period = 0;
    at_4g_startup_state = AT_4G_STARTUP_SEQ_STOP;
}

void at_4g_periodic(void)
{
    if(at_4g_abort_period > 0)
    {
        at_4g_abort_period--;

        if(TRUE == seqmgr_4g_is_allseq_idle())
        {
            at_4g_abort_period = 0;
            at_4g_period = 0;
            at_4g_startup_state = AT_4G_STARTUP_WAIRFOR_4GMODULE_STARTUP;
            at_4g_startup_seq.state = SEQ_4G_IDLE;
            at_4g_startup_seq.timeout = (AT_4G_STARTUP_SEQ_TIMEOUT/PERIODIC_UNIT_4G);
            MODULE_LOG_I(TBOX4G, "abort finish, begin to do startup");
            seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &at_4g_startup_seq, SEQ_4G_CONFLICT_ABORT);
        }
        else
        {
            if(0 == at_4g_abort_period)
            {
                seqmgr_4g_abort_allseq(0);
            }
            at_4g_period = 0;
            at_4g_startup_state = AT_4G_STARTUP_WAIRFOR_4GMODULE_STARTUP;
            at_4g_startup_seq.state = SEQ_4G_IDLE;
            at_4g_startup_seq.timeout = (AT_4G_STARTUP_SEQ_TIMEOUT/PERIODIC_UNIT_4G);
            seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &at_4g_startup_seq, SEQ_4G_CONFLICT_ABORT);
        }
        if(at_4g_abort_period > 0)
        {
            return;
        }
    }

    if(at_4g_period > 0)
    {
        at_4g_period--;
        if(at_4g_period > 0)
        {
            return;
        }
    }

    switch(at_4g_startup_state)
    {
        case AT_4G_STARTUP_WAIRFOR_4GMODULE_STARTUP:
            at_4g_startup_state = AT_4G_STARTUP_TESTLIVE;
            at_4g_period = 0;
            break;

        case AT_4G_STARTUP_TESTLIVE:
            if(0 == modem_4g_test_alive(AT_4G_CMD_HIGH, 0))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_TESTLIVE_RES;
            }
            else
            {
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTUP_READBAUD:
            if(DEV_4G_BAUDRATE == modem_4g_get_baud_value())
            {
                at_4g_startup_state = AT_4G_STARTUP_DIABLE_ECHO;
            }
            else
            {
                if(0 == modem_4g_read_baud(AT_4G_CMD_HIGH, 0))
                {
                    at_4g_startup_state = AT_4G_STARTUP_WAITFOR_READBAUD_RES;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed  to read baud command");
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
            }
            break;

        case AT_4G_STARTUP_SETBAUD:
            if(DEV_4G_BAUDRATE == modem_4g_get_baud_value())
            {
                at_4g_startup_state = AT_4G_STARTUP_DIABLE_ECHO;
            }
            else
            {
                if(0 == modem_4g_set_baud(AT_4G_CMD_HIGH, DEV_4G_BAUDRATE))
                {
                    at_4g_startup_state = AT_4G_STARTUP_WAITFOR_SETBAUD_RES;
                }
                else
                {
                    MODULE_LOG_E(TBOX4G, "failed  to set baud command");
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
            }
            break;

        case AT_4G_STARTUP_DIABLE_ECHO:
            if(0 == modem_4g_echomode(AT_4G_CMD_HIGH, 0))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_DIABLE_ECHO_RES;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed  to send disable echo command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTUP_ENABLE_CMEE_ERR:
            if(0 == modem_4g_enable_errnum(AT_4G_CMD_HIGH))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_ENABLE_CMEE_ERR;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed  to send enable cmee eeror command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTUP_QUERY_FUN:
            modem_4g_query_funmode(AT_4G_CMD_HIGH);
            at_4g_startup_state = AT_4G_STARTUP_WAITFOR_QUERY_FUN_RES;
            break;

        case AT_4G_STARTUP_SET_FUN:
            if(0 == modem_4g_set_normalmode(AT_4G_CMD_HIGH))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_SET_FUN;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed  to set 4g function act command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTUP_QUERY_COPS:
            if(0 == net_4g_query_cops(AT_4G_CMD_HIGH))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_QUERY_COPS_RES;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed  to send query cops command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTUP_CONFIG_NETSCAN_MODE:
            if(0 == net_4g_config_auto_scan(AT_4G_CMD_HIGH))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_CONFIG_NETSCAN_MODE_RES;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed to send net scan auto mode command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTIP_ENABLE_UPDATA_TIMEZONE:
            if(0 == net_4g_enable_updata_timezone(AT_4G_CMD_HIGH))
            {
                at_4g_startup_state = AT_4G_STARTIP_WAITFOR_ENABLE_UPDATA_TIMEZONE_RES;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed to send enable time zone command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;

        case AT_4G_STARTUP_CONFIG_URC:
            if(0 == modem_4g_config_urc_port(AT_4G_CMD_HIGH))
            {
                at_4g_startup_state = AT_4G_STARTUP_WAITFOR_CONFIG_URC_RES;
            }
            else
            {
                MODULE_LOG_E(TBOX4G, "failed  to send disable echo command");
                at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
            }
            break;
			
		case AT_4G_STARTUP_URC_OTHER_OFF:
			if(0 == modem_4g_urc_other_off(AT_4G_CMD_HIGH))
			{
				at_4g_startup_state = AT_4G_STARTUP_URC_OTHER_OFF_RES;
			}
			else
			{
				MODULE_LOG_E(TBOX4G, "failed  to send urc other off command");
				at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
			}
			break;

		case AT_4G_STARTUP_URC_RING:
			if(0 == modem_4g_urc_ring(AT_4G_CMD_HIGH))
			{
				at_4g_startup_state = AT_4G_STARTUP_URC_RING_RES;
			}
			else
			{
                MODULE_LOG_E(TBOX4G, "failed  to send urc ring command");
				at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
			}
			break;
			
		case AT_4G_STARTUP_URC_SMSINCOMING:
			if(0 == modem_4g_urc_smsincoming(AT_4G_CMD_HIGH))
			{
				at_4g_startup_state = AT_4G_STARTUP_URC_SMSINCOMING_RES;
			}
			else
			{
				MODULE_LOG_E(TBOX4G, "failed  to send urc smsincoming command");
				at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
			}
			break;

        default:
            break;
    }
}

uint8 at_4g_get_start_state(void)
{
    return at_4g_startup_state;
}

static uint8 at_4g_startupseq_resp(uint8 cmd, uint8 result)
{
    uint8 ret = SEQ_4G_CMD_NOMATCH;

    if(SEQMGR_4G_CMD_EXE_TIMEOUT == result)
    {
        at_4g_startup_handle_resptimeout();
        return SEQ_4G_CMD_FINISH;
    }

    switch(at_4g_startup_state)
    {
        case AT_4G_STARTUP_WAITFOR_TESTLIVE_RES:
            if(AT_4G_TESTALIVE_CMD == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive test liver respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    if(DEV_4G_BAUDRATE == modem_4g_get_baud_value())
                    {
                        at_4g_startup_state = AT_4G_STARTUP_DIABLE_ECHO;
                    }
                    else
                    {
                        at_4g_startup_state = AT_4G_STARTUP_READBAUD;
                    }
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_TESTLIVE;
                    at_4g_period = 0;

                    /*at_4g_testlive_retry_count++;
                    if(at_4g_testlive_retry_count >= AT_4G_STARTUP_TESTLIVE_RETRY_COUNT)
                    {
                        at_4g_testlive_retry_count = 0;
                        mpuc_cmd(MPUC_CMD_STARTUP);
                        MODULE_LOG_E(TBOX4G, "test live timeout, startup 4g");
                    }*/
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_READBAUD_RES:
            if(AT_4G_READ_BAUD == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive read baud respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    if(DEV_4G_BAUDRATE == modem_4g_get_baud_value())
                    {
                        at_4g_startup_state = AT_4G_STARTUP_DIABLE_ECHO;
                    }
                    else
                    {
                        at_4g_startup_state = AT_4G_STARTUP_SETBAUD;
                    }
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_READBAUD;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_SETBAUD_RES:
            if(AT_4G_WRITE_BAUD == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive write baud respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    modem_4g_set_baud_value(DEV_4G_BAUDRATE);
                    at_4g_startup_state = AT_4G_STARTUP_DIABLE_ECHO;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_SETBAUD;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_DIABLE_ECHO_RES:
            if(AT_4G_WRITE_ECHOMODE == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive write echo mode respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_ENABLE_CMEE_ERR;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_DIABLE_ECHO;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_ENABLE_CMEE_ERR:
            if(AT_4G_ENABLE_ERR_NUM == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive enable cmee error respond, result:%d", result);
                at_4g_startup_state = AT_4G_STARTUP_QUERY_FUN;
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_QUERY_FUN_RES:
            if(AT_4G_QUERY_FUNMODE == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive query fun mode, funmode:%d result:%d", modem_4g_info.funmode, result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    if(1U != modem_4g_info.funmode)
                    {
                        at_4g_startup_state = AT_4G_STARTUP_SET_FUN;
                    }
                    else
                    {
                        at_4g_startup_state = AT_4G_STARTUP_QUERY_COPS;
                    }
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_QUERY_FUN;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_SET_FUN:
            if(AT_4G_SET_NORMALMODE == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive set 4g function, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_QUERY_COPS;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_SET_FUN;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_QUERY_COPS_RES:
            if(AT_4G_QUERY_COPS == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive query query cops, copsmode:%d result:%d", net_4g_info.cops_mode, result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    if(0 != net_4g_info.cops_mode)
                    {
                        at_4g_startup_state = AT_4G_STARTUP_CONFIG_NETSCAN_MODE;
                    }
                    else
                    {
                        at_4g_startup_state = AT_4G_STARTIP_ENABLE_UPDATA_TIMEZONE;
                    }
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_CONFIG_NETSCAN_MODE;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_CONFIG_NETSCAN_MODE_RES:
            if(AT_4G_CONFIG_AUTO_SCAN == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive config auto scan respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTIP_ENABLE_UPDATA_TIMEZONE;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_CONFIG_NETSCAN_MODE;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTIP_WAITFOR_ENABLE_UPDATA_TIMEZONE_RES:
            if(AT_4G_UPDATA_TZ == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive enable time zone respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_CONFIG_URC;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTIP_ENABLE_UPDATA_TIMEZONE;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                }
                ret = SEQ_4G_CMD_ISMATCH;
            }
            break;

        case AT_4G_STARTUP_WAITFOR_CONFIG_URC_RES:
            if(AT_4G_CONFIG_URCPORT == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive config urcport respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_URC_OTHER_OFF;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_CONFIG_URC;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
            }
            break;
			
		case AT_4G_STARTUP_URC_OTHER_OFF_RES:
            if(AT_4G_CONFIG_URC_OTHER_OFF == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive urc other off respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_URC_RING;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_URC_OTHER_OFF;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
            }
            break;
			
		case AT_4G_STARTUP_URC_RING_RES:
            if(AT_4G_CONFIG_RING == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive urc ring respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_URC_SMSINCOMING;
                    
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_URC_RING;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
            }
            break;

		case AT_4G_STARTUP_URC_SMSINCOMING_RES:
            if(AT_4G_CONFIG_SMSINCOMING == cmd)
            {
                MODULE_LOG_I(TBOX4G, "receive urc smsincoming respond, result:%d", result);
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    at_4g_startup_state = AT_4G_STARTUP_SEQ_FINISH;
                    ret = SEQ_4G_CMD_FINISH;
                }
                else
                {
                    at_4g_startup_state = AT_4G_STARTUP_URC_SMSINCOMING;
                    at_4g_period = AT_4G_STARTUP_RETRY_PERIOD/PERIODIC_UNIT_4G;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
            }
            break;
		

        default:
            break;
    }

    return ret;
}

static void at_4g_startup_handle_resptimeout(void)
{
    if(AT_4G_STARTUP_IDLE == at_4g_startup_state ||
       AT_4G_STARTUP_SEQ_STOP == at_4g_startup_state ||
       AT_4G_STARTUP_SEQ_FINISH == at_4g_startup_state)
    {
        return;
    }

    at_4g_startup_state = AT_4G_STARTUP_IDLE;
    at_4g_period = 0;
    at_4g_abort_period = 0;
}
