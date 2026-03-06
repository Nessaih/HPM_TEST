#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_mgr.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_sms.h"
#include "4g_dial.h"
#include "4g_socket.h"
#include "4g_dev.h"
#include "4g_at_transmit.h"
#include "4g_data.h"
#include "4g_info.h"
#include "4g_power.h"
#include "4g_at_startup.h"
#include "4g_sequence_mgr.h"
#include "4g_sharememery.h"
#include "4g_if_inner.h"
#include "4g_ftp.h"
#include "4g_ftp.h"
#include "4g_time.h"
#include "4g_shell.h"

#define MGR_4G_SEQ_INTERVAL       (5*1000)/PERIODIC_UNIT_4G
#define MGR_4G_CMD_INTERVAL       (100)/PERIODIC_UNIT_4G
#define MGR_4G_MONITOR_INTERVAL   (uint32)(300*1000)/(uint32)PERIODIC_UNIT_4G //300S

#define MGR_4G_STATE_RUN    0
#define MGR_4G_STATE_SLEEP  1

static void mgr_4g_periodic(void);

static void mgr_4g_periodic_do_seq(void);

static void mgr_4g_periodic_do_stopseq(void);

static uint8 mgr_4g_datain_callback(void *context, uint8 *data, uint16 *len);

static uint8 mgr_4g_statein_callback(void *context, uint8 *data, uint16 *len);

/*static INT32 mgr_4g_set_phonenum(int argc, char **argv);

static INT32 mgr_4g_test_ntp(int argc, char **argv);

static INT32 mgr_4g_test_cclk(int argc, char **argv);

static INT32 mgr_4g_test_data(int argc, char **argv);*/

static MGR_4G_SEQ_STATE		mgr_4g_seq_state;
static uint16 				mgr_4g_period;
static uint8 				mgr_4g_runstate;

void mgr_4g_all_period(void)
{
static uint8 is_doing = 0;

    if(MGR_4G_STATE_RUN != mgr_4g_runstate)
    {
	   is_doing = 0;
       return;
    }

    if(1 == is_doing)
    {
        return;
    }
	
    is_doing = 1;
	
    at_4g_transmit_check_recv();
    at_4g_transmit_check_timeout();
    at_4g_transmit_monitor_queue();

    if_4g_periodic();
    power_4g_periodic();
    at_4g_periodic();
    dial_4g_periodic();
    info_4g_periodic();
    socket_4g_periodic();
    mgr_4g_periodic();
    seqmgr_4g_periodic();
    ftp_4g_period();
    at_4g_transmit_check_send();

    is_doing = 0;
}

void mgr_4g_init(void)
{
    mgr_4g_seq_state = MGR_4G_SEQ_IDLE;
    mgr_4g_period = 0;
    mgr_4g_runstate = MGR_4G_STATE_RUN;
	
    data_4g_init();
    seqmgr_4g_init();
    sharemem_4g_init();
    at_4g_transmit_init();
    at_4g_startup_init();
    info_4g_init();
    modem_4g_init();
    net_4g_init();
    power_4g_init();
    sim_4g_init();
    sms_4g_init();
    socket_4g_init();
    if_4g_init();
    ftp_4g_init();
    time_4g_init();

    at_4g_transmit_reg_callback(AT_4G_DECODE_DATAIN_IND, mgr_4g_datain_callback);
    at_4g_transmit_reg_callback(AT_4G_DECODE_STATE_IND, mgr_4g_statein_callback);

    dial_4g_init();
    shell_4g_init();
}

uint8 mgr_4g_get_state(void)
{
    return mgr_4g_seq_state;
}

boolean mgr_4g_is_stopped(void)
{
    return (mgr_4g_runstate == (uint8)MGR_4G_STATE_SLEEP) ? TRUE : FALSE;
}

void mgr_4g_do_stop(void)
{
    mgr_4g_seq_state = MGR_4G_SEQ_DOSTOP;
    mgr_4g_period = 0;
}

void mgr_4g_handle_when_sleep(void)
{
    mgr_4g_runstate = MGR_4G_STATE_SLEEP;

    socket_4g_set_all_disconnect();
    dial_4g_set_all_disconnect();

    MODULE_LOG_I(TBOX4G, "4g sleep finish");
}

void mgr_4g_handle_when_wakeup(void)
{
    mgr_4g_runstate = MGR_4G_STATE_RUN;
    mgr_4g_seq_state = MGR_4G_SEQ_IDLE;
    mgr_4g_period = 0;

    mgr_4g_reset();

    MODULE_LOG_I(TBOX4G, "receive wakeup indication");
}

void mgr_4g_reset_sequence(void)
{
    mgr_4g_runstate = MGR_4G_STATE_RUN;
    mgr_4g_seq_state = MGR_4G_SEQ_IDLE;
    mgr_4g_period = 0;
    mgr_4g_reset();
}

/*wakeup and reset*/
void mgr_4g_reset(void)
{
    data_4g_init();
    seqmgr_4g_init();
    sharemem_4g_reinit();
    at_4g_transmit_reset();
    at_4g_startup_init();
    info_4g_init();
    modem_4g_reinit();
    net_4g_init();
    power_4g_reset();
    sim_4g_reinit();
    sms_4g_init();
    dial_4g_reset();
    socket_4g_init();
    time_4g_init();
}

static uint8 mgr_4g_datain_callback(void *context, uint8 *data, uint16 *len)
{
    uint8 conn_id = *(uint8*)context;
    IF_4G_RECV_CALLBAKC callback = NULL;

    if(*len == 0)
    {
        return 0;
    }
    callback = if_4g_get_recv_callback(conn_id);
    if(callback != NULL)
    {
        callback(conn_id, data, *len);
    }

    return 0;
}

static uint8 mgr_4g_statein_callback(void *context, uint8 *data, uint16 *len)
{
    uint8 resp = *(uint8*)context;

    UNUSED(len);

    switch(resp)
    {
        case AT_4G_RESP_CPIN_READY:
            if(SIM_4G_PINSTATE_READY != sim_4g_get_pinstate())
            {
                MODULE_LOG_I(TBOX4G, "pin ready");
                sim_4g_set_pinstate(SIM_4G_PINSTATE_READY);
            }
            break;

        case AT_4G_RESP_CPIN_PIN:
            MODULE_LOG_I(TBOX4G, "it is need to enter pin code");
            sim_4g_set_pinstate(SIM_4G_PINSTATE_NEEDPIN);
            break;

        case AT_4G_RESP_CPIN_PUK:
            MODULE_LOG_I(TBOX4G, "it is need to enter puk code");
            sim_4g_set_pinstate(SIM_4G_PINSTATE_NEEDPUK);
            break;

        case AT_4G_RESP_CPIN_NOINSERT:
            if(SIM_4G_PINSTATE_NOINSERT != sim_4g_get_pinstate())
            {
                MODULE_LOG_I(TBOX4G, "pin not insert");
                if(SIM_4G_PINSTATE_UNKNOWN != sim_4g_get_pinstate())
                {
                    mgr_4g_reset();
                }
                sim_4g_set_pinstate(SIM_4G_PINSTATE_NOINSERT);
            }
            break;

        case AT_4G_RESP_SMS_DONE:
            MODULE_LOG_I(TBOX4G, "sms done");
            sim_4g_set_initstate(SIM_4G_INITSTATE_SMSDONE);
            break;

        case AT_4G_RESP_PB_DONE:
            MODULE_LOG_I(TBOX4G, "pb done");
            sim_4g_set_initstate(SIM_4G_INITSTATE_PBDONE);
            break;

        case AT_4G_RESP_IURC_CLOSE:
            {
                uint8 conn_id = *(uint8 *)data;
                MODULE_LOG_I(TBOX4G, "the connect close, conn_id:%d", conn_id);
                socket_4g_set_conn_state(conn_id, SOCKET_4G_STATE_DISCONNECTED);
            }
            break;

        case AT_4G_RESP_IURC_DEACTIVE:
            {
                MODULE_LOG_I(TBOX4G, "the context deactive, cotext_id:%d", *(uint8 *)data);
                socket_4g_close_all();
                dial_4g_stopcall();
            }
            break;

        default:
            break;
    }

    return 0;
}

static void mgr_4g_periodic(void)
{
    if(mgr_4g_seq_state < MGR_4G_SEQ_DOSTOP)
    {
        mgr_4g_periodic_do_seq();
    }
    else
    {
        mgr_4g_periodic_do_stopseq();
    }
}

static void mgr_4g_periodic_do_seq(void)
{
#define MGR_4G_REFETCH_ICCID_COUNT ((uint8)3)
    uint8 state;
    static uint8 suc_refetch_count = 0;

    if(mgr_4g_period > 0)
    {
        mgr_4g_period--;
        if(mgr_4g_period > 0)
        {
           return;
        }
    }

    switch(mgr_4g_seq_state)
    {
        case MGR_4G_SEQ_IDLE:
            state = socket_4g_mgr_state();
            if(SOCKET_4G_MGR_BUSY == state)
            {
                break;
            }
            state = at_4g_get_start_state();
            if(AT_4G_STARTUP_IDLE == state)
            {
                MODULE_LOG_I(TBOX4G, "begin to do startup sequence");
                at_4g_dostartup();
                mgr_4g_seq_state = MGR_4G_SEQ_DO_STARTUP;
            }
            else if(AT_4G_STARTUP_SEQ_FINISH == state)
            {
                info_4g_doquery();
                mgr_4g_seq_state = MGR_4G_SEQ_DO_QUERYINFO;
                MODULE_LOG_I(TBOX4G, "begin to do query information sequence");
            }
            else
            {
                mgr_4g_seq_state = MGR_4G_SEQ_DO_STARTUP;
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DO_STARTUP:
            state = at_4g_get_start_state();
            if(AT_4G_STARTUP_IDLE == state)
            {
                at_4g_dostartup();
            }
            else if(AT_4G_STARTUP_SEQ_FINISH == state)
            {
                MODULE_LOG_I(TBOX4G, "begin to do query information sequence");
                info_4g_doquery();
                mgr_4g_seq_state = MGR_4G_SEQ_DO_QUERYINFO;
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DO_QUERYINFO:
            state = info_4g_get_state();
            if(INFO_4G_IDLE == state ||
               INFO_4G_FINISH == state)
            {
                state = dial_4g_get_mgr_state();
                if(DIAL_4G_MGR_INIT == state ||
                   DIAL_4G_MGR_CALL_FINISH == state)
                {
                    /*
                     * <<20251016 lgc m1>>
                     * 如果连续3次获取iccid或者imei失败，就直接进行拨号流程；如果获取iccid和imei成功，将重试计算器重置为0。
                     */
                    if(FALSE == modem_4g_info.imei_is_valid ||
                       FALSE == sim_4g_info.iccid_is_valid)
                    {
                        if(suc_refetch_count < MGR_4G_REFETCH_ICCID_COUNT)
                        {
                            info_4g_doquery();
                            mgr_4g_period = MGR_4G_CMD_INTERVAL;
                            mgr_4g_seq_state = MGR_4G_SEQ_DO_QUERYINFO;
                            suc_refetch_count++;
                            return;
                        }
                    }
                    else
                    {
                        suc_refetch_count = 0;
                    }

                    if(TRUE == dial_4g_is_all_connect())
                    {
                        mgr_4g_seq_state = MGR_4G_SEQ_IDLE;
                        mgr_4g_period = 3* MGR_4G_SEQ_INTERVAL;// 15S
                        MODULE_LOG_I(TBOX4G, "all connect");
                        return;
                    }
                    MODULE_LOG_I(TBOX4G, "begin to do dail sequence");
                    dial_4g_docall();
                    mgr_4g_seq_state = MGR_4G_SEQ_DO_DAIL;
                }
                else if(DIAL_4G_MGR_STOP_FINISH == state)
                {
                    if(dial_4g_is_all_disconnect())
                    {
                        MODULE_LOG_I(TBOX4G, "begin to do dail sequence");
                        dial_4g_docall();
                        mgr_4g_seq_state = MGR_4G_SEQ_DO_DAIL;
                    }
                    else
                    {
                       dial_4g_stopcall();
                       mgr_4g_seq_state = MGR_4G_SEQ_DO_DAIL;
                    }
                }
                else
                {
                    mgr_4g_seq_state = MGR_4G_SEQ_DO_DAIL;
                }
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DO_DAIL:
            state = dial_4g_get_mgr_state();
            if(DIAL_4G_MGR_INIT == state ||
               DIAL_4G_MGR_CALL_FINISH == state ||
               DIAL_4G_MGR_STOP_FINISH == state)
            {
                mgr_4g_seq_state = MGR_4G_SEQ_IDLE;
                if(TRUE == dial_4g_is_all_connect())
                {
                    mgr_4g_period = 3U* MGR_4G_SEQ_INTERVAL; // 15s
                }
                else
                {
                    mgr_4g_period = MGR_4G_SEQ_INTERVAL;
                }
            }
            else
            {
                mgr_4g_period = MGR_4G_CMD_INTERVAL;
            }
            break;

        default:
            break;
    }
}

static void mgr_4g_periodic_do_stopseq(void)
{
    uint8 state;

    if(mgr_4g_period > 0)
    {
        mgr_4g_period--;
        if(mgr_4g_period > 0)
        {
           return;
        }
    }

    switch(mgr_4g_seq_state)
    {
        case MGR_4G_SEQ_DOSTOP:
            at_4g_dostop_startup();
            mgr_4g_seq_state = MGR_4G_SEQ_DOSTOP_STARTUP;
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DOSTOP_STARTUP:
            state = at_4g_get_start_state();
            if(AT_4G_STARTUP_SEQ_STOP == state)
            {
                info_4g_stop_query();
                mgr_4g_seq_state = MGR_4G_SEQ_DOSTOP_QUERYINFO;
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DOSTOP_QUERYINFO:
            state = info_4g_get_state();
            if(INFO_4G_STOP == state)
            {
                socket_4g_close_all();
                mgr_4g_seq_state = MGR_4G_SEQ_DOSTOP_SOCKET;
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DOSTOP_SOCKET:
            if(TRUE == socket_4g_isall_close())
            {
                dial_4g_stopcall();
                mgr_4g_seq_state = MGR_4G_SEQ_DOSTOP_DIAL;
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_DOSTOP_DIAL:
            if(TRUE == dial_4g_is_all_disconnect())
            {
                mgr_4g_seq_state = MGR_4G_SEQ_STOPFINISH;
            }
            mgr_4g_period = MGR_4G_CMD_INTERVAL;
            break;

        case MGR_4G_SEQ_STOPFINISH:
            /*DO NOTING*/
            break;

        default:
            break;
    }
}

void mgr_4g_dump_4ginfo(void)
{
    uint8 index = 0;

    LOG_PRINT("\r\n >>>>>>>>>>>>>>> SHOW 4G INFORMATION >>>>>>>>>>>>>>\r\n");
    LOG_PRINT("\r\n temperature : %d\r\n", modem_4g_info.modem_4g_temp);
    LOG_PRINT("\r\n chipid : %s \r\n", (char *)modem_4g_info.modem_4g_mtid);
    LOG_PRINT("\r\n fwversion : %s \r\n", (char *)modem_4g_info.modem_4g_taid);
    LOG_PRINT("\r\n dial[%d] state : %d \r\n", IF_4G_PUBLIC_APN, IF_4G_STATE_CONNECTED == dial_4g_get_callstate(IF_4G_PUBLIC_APN) ? 1 : 0);
	LOG_PRINT("\r\n hpm socket[%d] state : %d \r\n", 1, IF_4G_STATE_CONNECTED == socket_4g_get_conn_state(IF_4G_HPM_CONN_ID) ? 1 : 0);
    LOG_PRINT("\r\n imei: ");
    for(index = 0; index < MODEM_4G_IMEI_MAX; index++)
    {
        LOG_PRINT("%d", modem_4g_info.modem_4g_imei[index]);
    }
	
    LOG_PRINT("\r\n signalstrength:%d \r\n", net_4g_info.signalstrength);
    LOG_PRINT("\r\n iccid:%s \r\n", sim_4g_info.sim_4g_iccid);
    LOG_PRINT("\r\n imsi:%s \r\n", sim_4g_info.sim_4g_imsi);
    LOG_PRINT("\r\n phone number:%s \r\n", sim_4g_info.sim_4g_num);
}

#if 0
static INT32 mgr_4g_set_phonenum(int argc, char **argv)
{

    char  tel_no[MCU_CFG_HI_PHONE_NUM_LEN+1] = {'\0'};

    UNUSED(argc);
    UNUSED(argv);

    if(1 != argc)
    {
        printf("\r\n the phone number need one param \r\n");
        return 0;
    }

    if(strlen(*argv) > 16 || 0 == strlen(*argv))
    {
        printf("\r\n the phone number lenght is invalid \r\n");
        return 0;
    }

    strcpy(tel_no, *argv);
    cfg_set_param(MCU_CFG_ID_TEL_NO, (unsigned char*)tel_no);

    seqmgr_4g_abort_allseq(1);
    if(0 != sim_4g_set_num_ex(AT_4G_CMD_HIGH, tel_no, 3))
    {
        printf("\r\n failed to set phone number \r\n");
        return 0;
    }

    if_4g_query_phone_num();

    return 0;
}

static INT32 mgr_4g_test_ntp(int argc, char **argv)
{
    unsigned char  ip[64] = {0};
    unsigned short port = 0;

    if(argc != 2)
    {
        log_print("\r\n test time need two param\r\n");
        return -1;
    }

    snprintf((char*)ip, 64, "%s", argv[0]);
    if(sscanf(argv[1], "%hu", &port) != 1)
    {
        log_print("\r\n port format error\r\n");
        return -1;
    }

    time_4g_ntp(AT_4G_CMD_LOW, IF_4G_PUBLIC_APN, (uint8*)ip, port, NULL);

    return 0;
}

static INT32 mgr_4g_test_cclk(int argc, char **argv)
{
    int  type;

    if(argc != 1)
    {
        log_print("\r\n test time need one param\r\n");
        return -1;
    }

    if(sscanf(argv[0], "%d", &type) != 1)
    {
        log_print("\r\n type format error\r\n");
        return -1;
    }

    if(0 == type) // read
    {
        time_4g_cclk(AT_4G_CMD_LOW, NULL);
    }
    else if(1 == type) //write
    {
        time_4g_cclk_settime(AT_4G_CMD_LOW, (uint8*)"10/08/11,10:00:00+00");
    }
    else
    {
        log_print("\r\n the type is invalid \r\n");
    }

    return 0;
}

static INT32 mgr_4g_test_data(int argc, char **argv)
{
    uint8 test_data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8 delay_count = 50;
    UNUSED(argc);
    UNUSED(argv);

    log_print("\r\n socket connect \r\n");
    socket_4g_connect(IF_4G_PUBLIC_APN,
                      IF_4G_GB4_CONN_ID,
                      IF_4G_SOCKT_TCP,
                      (uint8*)"122.96.140.107",
                      10000);

    while(--delay_count > 0)
    {
        if(SOCKET_4G_STATE_CONNECTED == socket_4g_get_conn_state(IF_4G_GB4_CONN_ID))
        {
            OSIF_TimeDelay(100);

            log_print("\r\n socket send data \r\n");
            socket_4g_send(IF_4G_SEND_PRI_LOW, IF_4G_GB4_CONN_ID, test_data, sizeof(test_data));
            while(--delay_count > 0)
            {
                OSIF_TimeDelay(100);
            }
            break;
        }
        OSIF_TimeDelay(100);
    }

    log_print("\r\n socket close \r\n");
    socket_4g_close(IF_4G_GB4_CONN_ID);
    return 0;
}
#endif
