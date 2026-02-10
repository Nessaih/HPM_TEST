#include "4g_depend_header.h"
#include "4g_content.h"
#include "4g_at.h"
#include "4g_network.h"
#include "4g_modem.h"
#include "4g_sim.h"
#include "4g_sms.h"
#include "4g_dial.h"
#include "4g_socket.h"
#include "4g_sequence_mgr.h"
#include "4g_if_inner.h"
#include "4g_ftp.h"
#include "4g_mgr.h"

#define SOCKET_4G_CONN_TIMEOUT      (uint32)(40*1000)/(uint32)PERIODIC_UNIT_4G  //40S
#define SOCKET_4G_DISCONN_TIMEOUT   (uint32)(15*1000)/(uint32)PERIODIC_UNIT_4G  //15S
#define SOCKET_4G_SEND_TIMEOUT      (uint32)(20*1000)/(uint32)PERIODIC_UNIT_4G  //20S
#define SOCKET_4G_SEND_PERIOD       200/PERIODIC_UNIT_4G                        //200MS
#define SOCKET_4G_FAILED_PERIOD     1000/PERIODIC_UNIT_4G                       //1S
#define SOCKET_SEND_BUFFER_LEN      1024
#define SOCKET_DATA_MAX_LEN         IF_4G_SOCKET_MAX_DATA_LEN

#define SOCKET_4G_CONN_RETRY_COUNT      10
#define SOCKET_4G_DISCONN_RETRY_COUNT   1

typedef enum
{
    SOCKET_4G_DO_IDLE = 0,
    SOCKET_4G_DOCONN_CANCEL_CONN,
    SOCKET_4G_WAITFOR_CANCEL_CONN_RES,
    SOCKET_4G_DOCONN,
    SOCKET_4G_WAITFOR_DOCONN_RES,
    SOCKET_4G_DODISCONN,
    SOCKET_4G_WAITFOR_DODISCONN_RES,
    SOCKET_4G_SEND_LEN,
    SOCKET_4G_WAITFOR_SEND_LEN_RES,
    SOCKET_4G_SEND_DATA,
    SOCKET_4G_WAITFOR_SEND_DATA_RES,
    SOCKET_4G_QUERY_SOCKET_STATE,
    SOCKET_4G_WAITFOR_SOCKET_STATE_RES
}SOCKET_4G_DO_STATE;

typedef enum
{
    SOCKET_4G_DOTYPE_NONE = 0,
    SOCKET_4G_DOTYPE_DISCONN,
    SOCKET_4G_DOTYPE_CONN,
    SOCKET_4G_DOTYPE_DOSEND,
    SOCKET_4G_DOTYPE_MAX
}SOCKET_4G_DO_TYPE;

typedef struct
{
    uint8 context_id;
    uint8 sockt_type;
    uint8 ip[SOCKET_4G_IPV4_LEN+1];
    uint32 port;
}SOCKET_4G_CONN_INFO;

typedef struct
{
    INT8  conn_id;
    uint8 conn_state;
    uint8 dostate;
    uint8 will_do;
    uint8 retry_count;
    uint8 send_pri;
    uint16 period;
    DATA_QUEUE send_queue;
    union
    {
        SOCKET_4G_CONN_INFO conn_info;
        uint8 send_buff[SOCKET_SEND_BUFFER_LEN];
    }data;
}SOCKET_4G_INFO;

typedef struct
{
    uint8 state;
    INT8 cur_index;
    SOCKET_4G_INFO info[SOCKET_4G_COUNT];
}SOCKET_4G_MGR;

static uint8 socket_4g_seq_resp(uint8 cmd, uint8 result);

static void socket_4g_periodic_in_idle(void);

static void socket_4g_periodic_in_busy(void);

static INT8 socket_4g_select_seq_in_idle(void);

static SOCKET_4G_MGR socket_4g_mgr;

static uint8 socket_4g_temp_buff[SOCKET_DATA_MAX_LEN];

static SEQ_4G socket_4g_seq = {SEQ_4G_IDLE,
                                 0,
                                 NULL,
                                 NULL,
                                 socket_4g_seq_resp
                               };

void socket_4g_init(void)
{
    uint8 index;

    socket_4g_mgr.cur_index = -1;
    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        memset(socket_4g_mgr.info[index].data.send_buff, 0, sizeof(socket_4g_mgr.info[index].data.send_buff));
        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
        socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
        socket_4g_mgr.info[index].conn_id = -1;
        socket_4g_mgr.info[index].period = 0;
        socket_4g_mgr.info[index].retry_count = 0;
        socket_4g_mgr.info[index].send_pri = IF_4G_SEND_PRI_LOW;
        socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
    }
}

uint8 socket_4g_connect(uint8 context_id,
                        uint8 conn_id,
                        uint8 sockt_type,
                        uint8 *ip,
                        uint32 port)
{
    uint8 index;
    INT8  state;

    state = dial_4g_get_callstate(context_id);
    if(DIAL_4G_STATE_CONNECTED != state)
    {
        MODULE_LOG_E(TBOX4G, "the context is not connect ");
        return 1;
    }

    MODULE_LOG_I(TBOX4G, "mgr state:%d index:%d", socket_4g_mgr.state, socket_4g_mgr.cur_index);
    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        MODULE_LOG_I(TBOX4G, "socket:%d do_state:%d will_do:%d conn_id:%d", socket_4g_mgr.info[index].conn_state,
            socket_4g_mgr.info[index].dostate,
            socket_4g_mgr.info[index].will_do,
            socket_4g_mgr.info[index].conn_id);
    }

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id == socket_4g_mgr.info[index].conn_id)
        {
            if(SOCKET_4G_DOTYPE_DISCONN == socket_4g_mgr.info[index].will_do)
            {
                MODULE_LOG_E(TBOX4G, "wait for the socket to be disconnect, index:%d ", index);
                return 1;
            }

            if(SOCKET_4G_STATE_CONNECTED == socket_4g_mgr.info[index].conn_state)
            {
                MODULE_LOG_E(TBOX4G, "the socket has been connect, index:%d ", index);
                return 0;
            }
            else if(SOCKET_4G_STATE_CONNECTING == socket_4g_mgr.info[index].conn_state)
            {
                MODULE_LOG_E(TBOX4G, "the socket is connecting, index:%d ", index);
                return 1;
            }
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        for(index = 0; index < SOCKET_4G_COUNT; index++)
        {
            if(-1 == socket_4g_mgr.info[index].conn_id)
            {
                break;
            }
        }
        if(index >= SOCKET_4G_COUNT)
        {
            return 1;
        }
    }

    MODULE_LOG_I(TBOX4G, "socket connect ip:%s port:%d index:%d", ip, port, index);

    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_CONN;
    socket_4g_mgr.info[index].period = 0;
    socket_4g_mgr.info[index].conn_id = conn_id;
    socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
    socket_4g_mgr.info[index].data.conn_info.context_id = context_id;
    socket_4g_mgr.info[index].data.conn_info.sockt_type = sockt_type;
    socket_4g_mgr.info[index].data.conn_info.port = port;
    socket_4g_mgr.info[index].retry_count = 0;
    memcpy(socket_4g_mgr.info[index].data.conn_info.ip, ip, SOCKET_4G_IPV4_LEN);
    socket_4g_mgr.info[index].data.conn_info.ip[SOCKET_4G_IPV4_LEN] = '\0';

    return 0;
}

uint8 socket_4g_close(uint8 conn_id)
{
    uint8 index;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id == socket_4g_mgr.info[index].conn_id)
        {
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        return 1;
    }

    MODULE_LOG_I(TBOX4G, "socket disconnect index:%d", index);

    if(SOCKET_4G_DOTYPE_DISCONN == socket_4g_mgr.info[index].will_do)
    {
        MODULE_LOG_E(TBOX4G, "the socket is disconnecting, index:%d ", index);
        return 1;
    }
   /* else
    {
        if(SOCKET_4G_DOTYPE_NONE == socket_4g_mgr.info[index].will_do &&
           SOCKET_4G_STATE_DISCONNECTED == socket_4g_mgr.info[index].conn_state)
        {
            return 0;
        }
    }*/
    socket_4g_mgr.info[index].retry_count = 0;
    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DISCONN;

    return 0;
}

void socket_4g_close_all(void)
{
    uint8 index;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(-1 == socket_4g_mgr.info[index].conn_id ||
           SOCKET_4G_DOTYPE_DISCONN == socket_4g_mgr.info[index].will_do ||
           SOCKET_4G_STATE_DISCONNECTED == socket_4g_mgr.info[index].conn_state)
        {
           continue;
        }
        socket_4g_mgr.info[index].retry_count = 0;
        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DISCONN;
    }

    return;
}

boolean socket_4g_isall_close(void)
{
    uint8 index;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(-1 == socket_4g_mgr.info[index].conn_id)
        {
            continue;
        }
        if(SOCKET_4G_STATE_DISCONNECTED != socket_4g_mgr.info[index].conn_state)
        {
            return FALSE;
        }
    }

    return TRUE;
}

INT8 socekt_4g_get_connid(uint8 index)
{
    if(index >= SOCKET_4G_COUNT)
    {
        return -1;
    }

    return socket_4g_mgr.info[index].conn_id;
}

uint8 socket_4g_get_conn_state(uint8 conn_id)
{
    uint8 index;
    uint8 ret = SOCKET_4G_STATE_DISCONNECTED;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id == socket_4g_mgr.info[index].conn_id)
        {
            ret = socket_4g_mgr.info[index].conn_state;
            break;
        }
    }

    return ret;
}

uint8 socket_4g_mgr_state(void)
{
    return socket_4g_mgr.state;
}

void socket_4g_set_conn_state(uint8 conn_id, uint8 state)
{
    uint8 index;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id == socket_4g_mgr.info[index].conn_id)
        {
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        return;
    }

    if(SOCKET_4G_STATE_CONNECTED == state)
    {
        if(SOCKET_4G_STATE_CONNECTED != socket_4g_mgr.info[index].conn_state)
        {
            socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_CONNECTED;

            if(SOCKET_4G_DOTYPE_DISCONN != socket_4g_mgr.info[index].will_do ||
               SOCKET_4G_DOCONN == socket_4g_mgr.info[index].dostate ||
               SOCKET_4G_DOCONN_CANCEL_CONN == socket_4g_mgr.info[index].dostate ||
               SOCKET_4G_WAITFOR_CANCEL_CONN_RES == socket_4g_mgr.info[index].dostate ||
               SOCKET_4G_WAITFOR_DOCONN_RES == socket_4g_mgr.info[index].dostate)
            {
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                socket_4g_mgr.info[index].period = 0;
                socket_4g_mgr.info[index].retry_count = 0;
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;

                memset(socket_4g_mgr.info[index].data.send_buff, 0, sizeof(socket_4g_mgr.info[index].data.send_buff));
                dataqueue_init(&socket_4g_mgr.info[index].send_queue);
                socket_4g_mgr.info[index].send_queue.buf_ptr = socket_4g_mgr.info[index].data.send_buff;
                socket_4g_mgr.info[index].send_queue.len = sizeof(socket_4g_mgr.info[index].data.send_buff);

                if(SOCKET_4G_MGR_BUSY == socket_4g_mgr.state)
                {
                    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                    seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
                }
            }
        }
    }
    else if(SOCKET_4G_STATE_DISCONNECTED == state)
    {
        if(SOCKET_4G_STATE_DISCONNECTED != socket_4g_mgr.info[index].conn_state)
        {
            socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;

            socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
            socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
            socket_4g_mgr.info[index].period = 0;
            socket_4g_mgr.info[index].retry_count = 0;
            socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
            socket_4g_mgr.cur_index = -1;

            if(SOCKET_4G_MGR_BUSY == socket_4g_mgr.state)
            {
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
            }
        }
    }
    else
    {
        /**/
    }
}

void socket_4g_set_all_disconnect(void)
{
    uint8 index;

    if(SOCKET_4G_MGR_BUSY == socket_4g_mgr.state)
    {
        seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
    }

    socket_4g_mgr.cur_index = -1;
    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        memset(socket_4g_mgr.info[index].data.send_buff, 0, sizeof(socket_4g_mgr.info[index].data.send_buff));
        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
        socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
        socket_4g_mgr.info[index].conn_id = -1;
        socket_4g_mgr.info[index].period = 0;
        socket_4g_mgr.info[index].retry_count = 0;
        socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
    }
}

uint8 socket_4g_send(uint8 pri, uint8 conn_id, uint8 *data, uint16 len)
{
    uint8 index;
    uint8 state = SOCKET_4G_STATE_DISCONNECTED;
    DATA_ELEMENT_HEADER header;

    if(TRUE == ftp_4g_isdownloading())
    {
        MODULE_LOG_E(TBOX4G, "the ftp is downloading");
        return 1;
    }

    if(len >= SOCKET_DATA_MAX_LEN)
    {
        MODULE_LOG_E(TBOX4G, "the data is too long");
        return 1;
    }

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(conn_id == socket_4g_mgr.info[index].conn_id)
        {
            state = socket_4g_mgr.info[index].conn_state;
            break;
        }
    }
    if(index >= SOCKET_4G_COUNT)
    {
        MODULE_LOG_E(TBOX4G, "4g count too big");    
        return 1;
    }
    if(SOCKET_4G_STATE_CONNECTED != state)
    {
        MODULE_LOG_E(TBOX4G, "the socket is not connect");
        return 1;
    }

    if(SOCKET_4G_DOTYPE_DISCONN == socket_4g_mgr.info[index].will_do ||
        SOCKET_4G_DOTYPE_CONN == socket_4g_mgr.info[index].will_do)
    {
        MODULE_LOG_E(TBOX4G, "the socket is connecting or disconnecting");
        return 1;
    }

    header.magic_no = DATA_QUEUE_MAGICNO;
    header.element_len = len;
    header.element_type = conn_id;
    if(DATA_QUEUE_RET_SUCCESS != dataqueue_enqueue(&socket_4g_mgr.info[index].send_queue, &header, data))
    {
        MODULE_LOG_E(TBOX4G, "failed to put data into queue");
        return 1;
    }

    socket_4g_mgr.info[index].send_pri = pri;
    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DOSEND;

    return 0;
}

void socket_4g_periodic(void)
{
    uint8 index;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(socket_4g_mgr.info[index].period > 0)
        {
            socket_4g_mgr.info[index].period--;
        }
    }

    if(SOCKET_4G_MGR_IDLE == socket_4g_mgr.state)
    {
        socket_4g_periodic_in_idle();
    }
    else
    {
        socket_4g_periodic_in_busy();
    }
}

#if 0
void socket_4g_close_cursock(void)
{
    INT8 index = socket_4g_mgr.cur_index;

    if(index < 0 ||
        SOCKET_4G_MGR_IDLE == socket_4g_mgr.state)
    {
        return;
    }
		
	if(SOCKET_4G_SEND_LEN == socket_4g_mgr.info[index].dostate ||
	  SOCKET_4G_WAITFOR_SEND_LEN_RES == socket_4g_mgr.info[index].dostate ||
	  SOCKET_4G_SEND_DATA == socket_4g_mgr.info[index].dostate ||
	  SOCKET_4G_WAITFOR_SEND_DATA_RES == socket_4g_mgr.info[index].dostate)
	{
		socket_4g_set_conn_state(socket_4g_mgr.info[index].conn_id, SOCKET_4G_STATE_DISCONNECTED);
	}
}
#endif

static uint8 socket_4g_seq_resp(uint8 cmd, uint8 result)
{
    uint8 ret = SEQ_4G_CMD_NOMATCH;
    INT8 index = socket_4g_mgr.cur_index;

    if(index < 0 ||
        SOCKET_4G_MGR_IDLE == socket_4g_mgr.state)
    {
        return ret;
    }

    MODULE_LOG_I(TBOX4G, "recv info, index:%d cmd:%d result:%d dostate:%d",
                   index, cmd, result, socket_4g_mgr.info[index].dostate);

    switch(socket_4g_mgr.info[index].dostate)
    {
        case SOCKET_4G_WAITFOR_CANCEL_CONN_RES:
            if(AT_4G_CLOSE_SOCKET == cmd)
            {
                if(SOCKET_4G_DOTYPE_DISCONN == socket_4g_mgr.info[index].will_do)
                {
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
                    socket_4g_mgr.info[index].retry_count = 0;
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                    socket_4g_mgr.cur_index = -1;
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                    ret = SEQ_4G_CMD_FINISH;
                }
                else
                {
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_DOCONN;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                socket_4g_mgr.info[index].period = 0;
            }
            break;

        case SOCKET_4G_WAITFOR_DOCONN_RES:
            if(AT_4G_OPEN_SOCKET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_CONNECTED;

                    memset(socket_4g_mgr.info[index].data.send_buff, 0, sizeof(socket_4g_mgr.info[index].data.send_buff));
                    dataqueue_init(&socket_4g_mgr.info[index].send_queue);
                    socket_4g_mgr.info[index].send_queue.buf_ptr = socket_4g_mgr.info[index].data.send_buff;
                    socket_4g_mgr.info[index].send_queue.len = sizeof(socket_4g_mgr.info[index].data.send_buff);

                    if(SOCKET_4G_DOTYPE_DISCONN != socket_4g_mgr.info[index].will_do)
                    {
                        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DOSEND;
                    }
                    socket_4g_mgr.info[index].period = 0;
                }
                else
                {
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                    socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
                }
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;

                ret = SEQ_4G_CMD_FINISH;
            }
            break;

        case SOCKET_4G_WAITFOR_DODISCONN_RES:
            if(AT_4G_CLOSE_SOCKET == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
                    socket_4g_mgr.info[index].retry_count = 0;
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                    socket_4g_mgr.info[index].period = 0;
                }
                else
                {
                    if(SOCKET_4G_DOTYPE_DISCONN == socket_4g_mgr.info[index].will_do)
                    {
                        socket_4g_mgr.info[index].retry_count++;
                        if(socket_4g_mgr.info[index].retry_count >= SOCKET_4G_DISCONN_RETRY_COUNT)
                        {
                            socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
                            socket_4g_mgr.info[index].retry_count = 0;
                            socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                        }
                    }
                    else
                    {
                        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                    }
                    socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
                }
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;

                ret = SEQ_4G_CMD_FINISH;
            }
            break;

        case SOCKET_4G_WAITFOR_SEND_LEN_RES:
            if(AT_4G_SOCKET_DATA_LEN == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_SEND_DATA;
                    ret = SEQ_4G_CMD_ISMATCH;
                }
                else /*failed to get ack, disconnect connect*/
                {
                    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                    socket_4g_mgr.cur_index = -1;
                    socket_4g_mgr.info[index].period = 0;
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DISCONN;
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case SOCKET_4G_WAITFOR_SEND_DATA_RES:
            if(AT_4G_SOCKET_DATA == cmd)
            {
                if(SEQMGR_4G_CMD_EXE_OK == result)
                {
                    DATA_ELEMENT_HEADER header;
                    IF_4G_SEND_CALLBACK callbak = NULL;
                    dataqueue_get_header(&socket_4g_mgr.info[index].send_queue, &header);
                    dataqueue_dequeue(&socket_4g_mgr.info[index].send_queue,
                                      header.element_len + sizeof(DATA_ELEMENT_HEADER));
                    callbak = if_4g_get_send_callback(socket_4g_mgr.info[index].conn_id);
                    if(NULL != callbak)
                    {
                        callbak(socket_4g_mgr.info[index].conn_id, IF_4G_SEND_OK);
                    }

                    if(SOCKET_4G_DOTYPE_DISCONN != socket_4g_mgr.info[index].will_do)
                    {
                        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DOSEND;
                    }
                    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                    socket_4g_mgr.cur_index = -1;
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                    ret = SEQ_4G_CMD_FINISH;
                }
                else /*failed to get ack, disconnect connect*/
                {
                    socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                    socket_4g_mgr.cur_index = -1;
                    socket_4g_mgr.info[index].period = 0;
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DISCONN;
                    ret = SEQ_4G_CMD_FINISH;
                }
            }
            break;

        case SOCKET_4G_WAITFOR_SOCKET_STATE_RES:
            if(AT_4G_QUERY_SOCKET_STATE == cmd)
            {
                if(NET_4G_SOCKET_DISCONNECT == ((result&0xf0) >> 4))
                {
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTED;
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                }
                else
                {
                    if(SOCKET_4G_DOTYPE_DISCONN != socket_4g_mgr.info[index].will_do)
                    {
                        socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_DOSEND;
                    }
                }

                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                ret = SEQ_4G_CMD_FINISH;
            }
            break;

        default:
          if(SEQMGR_4G_CMD_EXE_TIMEOUT == result ||
             SEQMGR_4G_CMD_EXE_ABORT == result) //abnormal condition,reset state
          {
              socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
              socket_4g_mgr.cur_index = -1;
              socket_4g_mgr.info[index].period = 0;
              socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
              ret = SEQ_4G_CMD_FINISH;
          }
          break;
    }

    return ret;
}

static void socket_4g_periodic_in_idle(void)
{
    DATA_ELEMENT_HEADER header;
    INT8 sel_index = socket_4g_select_seq_in_idle();
    if(sel_index < 0)
    {
        return;
    }

    //MODULE_LOG_I(TBOX4G, "socket begin to connect, index:%d", sel_index);

    switch(socket_4g_mgr.info[sel_index].will_do)
    {
        case SOCKET_4G_DOTYPE_CONN:
            socket_4g_seq.state = SEQ_4G_IDLE;
            socket_4g_seq.timeout = SOCKET_4G_CONN_TIMEOUT;
            if(0 != seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &socket_4g_seq, SEQ_4G_ABORT))
            {
                socket_4g_mgr.info[sel_index].period = SOCKET_4G_FAILED_PERIOD;
                return;
            }
            socket_4g_mgr.state = SOCKET_4G_MGR_BUSY;
            socket_4g_mgr.cur_index = sel_index;
            socket_4g_mgr.info[sel_index].dostate = SOCKET_4G_DOCONN_CANCEL_CONN;
            break;

        case SOCKET_4G_DOTYPE_DISCONN:
            /*if(DIAL_4G_STATE_DISCONNECTED == socket_4g_mgr.info[sel_index].conn_state)
            {
                socket_4g_mgr.info[sel_index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[sel_index].will_do = SOCKET_4G_DOTYPE_NONE;
                socket_4g_mgr.info[sel_index].period = 0;
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                return;
            }*/
            socket_4g_seq.state = SEQ_4G_IDLE;
            socket_4g_seq.timeout = SOCKET_4G_DISCONN_TIMEOUT;
            if(0 != seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &socket_4g_seq, SEQ_4G_ABORT))
            {
                return;
            }
            socket_4g_mgr.info[sel_index].dostate = SOCKET_4G_DODISCONN;
            socket_4g_mgr.info[sel_index].retry_count = 0;
            socket_4g_mgr.state = SOCKET_4G_MGR_BUSY;
            socket_4g_mgr.cur_index = sel_index;
            break;

        case SOCKET_4G_DOTYPE_DOSEND:
            if(DATA_QUEUE_RET_SUCCESS != dataqueue_get_header(&socket_4g_mgr.info[sel_index].send_queue, &header))
            {
                dataqueue_init(&socket_4g_mgr.info[sel_index].send_queue);
                socket_4g_mgr.info[sel_index].send_queue.buf_ptr = socket_4g_mgr.info[sel_index].data.send_buff;
                socket_4g_mgr.info[sel_index].send_queue.len = sizeof(socket_4g_mgr.info[sel_index].data.send_buff);

                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.info[sel_index].will_do = SOCKET_4G_DOTYPE_NONE;
                socket_4g_mgr.info[sel_index].dostate = SOCKET_4G_DO_IDLE;
                return;
            }
            socket_4g_seq.state = SEQ_4G_IDLE;
            socket_4g_seq.timeout = SOCKET_4G_SEND_TIMEOUT;
            if(0 != seqmgr_4g_doseq(SEQ_4G_PRI_HIGH, &socket_4g_seq, SEQ_4G_ABORT))
            {
                MODULE_LOG_E(TBOX4G, "failed to do sequence");
                socket_4g_mgr.info[sel_index].period = SOCKET_4G_FAILED_PERIOD;
                return;
            }
            socket_4g_mgr.state = SOCKET_4G_MGR_BUSY;
            socket_4g_mgr.cur_index = sel_index;
            socket_4g_mgr.info[sel_index].dostate = SOCKET_4G_SEND_LEN;
            break;

        default:
            break;
    }
}

static void socket_4g_periodic_in_busy(void)
{
    DATA_ELEMENT_HEADER header;
    INT8 index = socket_4g_mgr.cur_index;

    if(index < 0 || index >= SOCKET_4G_COUNT)
    {
        socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
        socket_4g_mgr.cur_index = -1;
        return;
    }

 //   MODULE_LOG_I(TBOX4G, "current index:%d dostate:%d", index, socket_4g_mgr.info[index].dostate);

    switch(socket_4g_mgr.info[index].dostate)
    {
        case SOCKET_4G_DOCONN_CANCEL_CONN:
            if(0 == net_4g_close_socket(AT_4G_CMD_LOW, socket_4g_mgr.info[index].conn_id))
            {
                socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_CANCEL_CONN_RES;
                socket_4g_mgr.info[index].period = 0;
                socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_CONNECTING;
            }
            else
            {
                socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
            }
            break;

        case SOCKET_4G_DOCONN:
            if(SOCKET_4G_SOCKT_UDP == socket_4g_mgr.info[index].data.conn_info.sockt_type)
            {
                if(0 == net_4g_open_socket(AT_4G_CMD_LOW,
                                           socket_4g_mgr.info[index].data.conn_info.context_id+1,
                                           socket_4g_mgr.info[index].conn_id,
                                           (uint8*)"UDP",
                                           socket_4g_mgr.info[index].data.conn_info.ip,
                                           socket_4g_mgr.info[index].data.conn_info.port))
                {
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_DOCONN_RES;
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_CONNECTING;
                }
                else
                {
                    socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
                }
            }
            else
            {
                if(0 == net_4g_open_socket(AT_4G_CMD_LOW,
                                           socket_4g_mgr.info[index].data.conn_info.context_id+1,
                                           socket_4g_mgr.info[index].conn_id,
                                           (uint8*)"TCP",
                                           socket_4g_mgr.info[index].data.conn_info.ip,
                                           socket_4g_mgr.info[index].data.conn_info.port))
                {
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_DOCONN_RES;
                    socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_CONNECTING;
                }
                else
                {
                    socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
                }
            }
            break;

        case SOCKET_4G_DODISCONN:
            if(0 == net_4g_close_socket(AT_4G_CMD_LOW, socket_4g_mgr.info[index].conn_id))
            {
                socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_DODISCONN_RES;
                socket_4g_mgr.info[index].period = 0;
                socket_4g_mgr.info[index].conn_state = SOCKET_4G_STATE_DISCONNECTEING;
            }
            else
            {
                socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
            }
            break;

        case SOCKET_4G_SEND_LEN:
            if(SOCKET_4G_STATE_DISCONNECTED == socket_4g_mgr.info[index].conn_state)
            {
                socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[index].period = 0;
                seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
                break;
            }

            if(DATA_QUEUE_RET_SUCCESS != dataqueue_get_header(&socket_4g_mgr.info[index].send_queue, &header))
            {
                if(SOCKET_4G_DOTYPE_DISCONN != socket_4g_mgr.info[index].will_do)
                {
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                }
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
                seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
            }
            else
            {
                MODULE_LOG_I(TBOX4G, "send data len[%d]", header.element_len);
                if(0 == net_4g_send_data_len(AT_4G_CMD_LOW, socket_4g_mgr.info[index].conn_id, header.element_len))
                {
                    socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_SEND_LEN_RES;
                }
                else
                {
                    socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
                }
            }
            break;

        case SOCKET_4G_SEND_DATA:
            if(SOCKET_4G_STATE_DISCONNECTED == socket_4g_mgr.info[index].conn_state)
            {
                socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[index].period = 0;
                seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
                break;
            }

            dataqueue_get_header(&socket_4g_mgr.info[index].send_queue, &header);
            if(DATA_QUEUE_RET_SUCCESS != dataqueue_get_data(&socket_4g_mgr.info[index].send_queue, header.element_len, socket_4g_temp_buff))
            {
                dataqueue_init(&socket_4g_mgr.info[index].send_queue);
                socket_4g_mgr.info[index].send_queue.buf_ptr = socket_4g_mgr.info[index].data.send_buff;
                socket_4g_mgr.info[index].send_queue.len = sizeof(socket_4g_mgr.info[index].data.send_buff);

                if(SOCKET_4G_DOTYPE_DISCONN != socket_4g_mgr.info[index].will_do)
                {
                    socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                }
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;

                seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
            }
            if(0 == net_4g_send_data(socket_4g_temp_buff, header.element_len))
            {
                socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_SEND_DATA_RES;
            }
            else
            {
                socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
            }
            break;

        case SOCKET_4G_QUERY_SOCKET_STATE:
            if(SOCKET_4G_STATE_DISCONNECTED == socket_4g_mgr.info[index].conn_state)
            {
                socket_4g_mgr.info[index].will_do = SOCKET_4G_DOTYPE_NONE;
                socket_4g_mgr.state = SOCKET_4G_MGR_IDLE;
                socket_4g_mgr.cur_index = -1;
                socket_4g_mgr.info[index].dostate = SOCKET_4G_DO_IDLE;
                socket_4g_mgr.info[index].period = 0;
                seqmgr_4g_resetseq(SEQ_4G_PRI_LOW);
                break;
            }

            if(0 == net_4g_query_socket_state(AT_4G_CMD_LOW, socket_4g_mgr.info[index].conn_id))
            {
                socket_4g_mgr.info[index].dostate = SOCKET_4G_WAITFOR_SOCKET_STATE_RES;
            }
            else
            {
                socket_4g_mgr.info[index].period = SOCKET_4G_FAILED_PERIOD;
            }
            break;
    }
}

static INT8 socket_4g_select_seq_in_idle(void)
{
    uint8 index, will_do = SOCKET_4G_DOTYPE_MAX;
    INT8 cur_index = socket_4g_mgr.cur_index;
    INT8 sel_index = -1;

    for(index = 0; index < SOCKET_4G_COUNT; index++)
    {
        if(socket_4g_mgr.info[index].period > 0)
        {
            continue;
        }
        if(SOCKET_4G_DOTYPE_NONE == socket_4g_mgr.info[index].will_do)
        {
            continue;
        }
        if(will_do > socket_4g_mgr.info[index].will_do)
        {
            will_do = socket_4g_mgr.info[index].will_do;
            sel_index = index;
        }
        else if(will_do == socket_4g_mgr.info[index].will_do)
        {
            if(SOCKET_4G_DOTYPE_DOSEND == will_do &&
               -1 != sel_index)
            {
                if(socket_4g_mgr.info[sel_index].send_pri > socket_4g_mgr.info[index].send_pri)
                {
                    will_do = socket_4g_mgr.info[index].will_do;
                    sel_index = index;
                }
            }
        }
        else
        {
            /**/
        }
    }
    if(sel_index != -1)
    {
        return sel_index;
    }

    for(index = cur_index+1; index < SOCKET_4G_COUNT; index++)
    {
        if(socket_4g_mgr.info[index].period > 0)
        {
            continue;
        }
        if(SOCKET_4G_DOTYPE_NONE == socket_4g_mgr.info[index].will_do)
        {
            continue;
        }
        sel_index = index;
        break;
    }
    if(sel_index != -1)
    {
        return sel_index;
    }

    for(index = 0; index < cur_index; index++)
    {
        if(socket_4g_mgr.info[index].period > 0)
        {
            continue;
        }
        if(SOCKET_4G_DOTYPE_NONE == socket_4g_mgr.info[index].will_do)
        {
            continue;
        }
        sel_index = index;
        break;
    }

    return sel_index;
}

