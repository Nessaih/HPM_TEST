#ifndef TBOX_4G_AT_H
#define TBOX_4G_AT_H

#include "tbox_common.h"
#include "4g_content.h"

#define AT_4G_RECV_END_OK_STRING "\r\n\r\nOK\r\n"

#define AT_4G_REQ_SUFFIX         "\r\n"
#define AT_4G_RESP_PREFIX        "\r\n"
#define AT_4G_RESP_SUFFIX        "\r\n"

typedef uint8 (*AT_CMD_RESP)(void *context, uint8 *data, uint16 *len);

typedef enum
{
    AT_4G_RESP_UNKNOWN = 0,  /*initialize value*/
    AT_4G_RESP_SENDCMP,
    AT_4G_RESP_TIMEOUT,    /*response timeout*/
    AT_4G_RESP_ABORT,      /*sending the high priority data*/
    AT_4G_RESP_OK,         /*"OK"*/
    AT_4G_RESP_BUSY,       /*"BUSY"*/
    AT_4G_RESP_NO_ANSWER,  /*"NO ANSWER"*/
    AT_4G_RESP_NO_CARRIER, /*"NO CARRIER"*/
    AT_4G_RESP_CONNECT,    /*"CONNECT"*/
    AT_4G_RESP_ERROR,      /*"ERROR"*/
    AT_4G_RESP_MIN_ERROR = AT_4G_RESP_ERROR,
    AT_4G_RESP_CMEERR_NOALLOWED,     /*"+CME ERROR:3"*/
    AT_4G_RESP_CMEERR_NOSUPPORT,     /*"+CME ERROR:4"*/
    AT_4G_RESP_CMEERR_SIMNOINSERT,   /*"+CME ERROR:10"*/
    AT_4G_RESP_CMEERR_PINREQ,        /*"+CME ERROR:11"*/
    AT_4G_RESP_CMEERR_PUKREQ,        /*"+CME ERROR:12"*/
    AT_4G_RESP_CMEERR_SIMFAILED,     /*"+CME ERROR:13"*/
    AT_4G_RESP_CMEERR_SIMBUSY,       /*"+CME ERROR:14"*/
    AT_4G_RESP_CMEERR_SIMWRONG,      /*"+CME ERROR:15"*/
    AT_4G_RESP_CMEERR_MEMFULL,       /*"+CME ERROR:20"*/
    AT_4G_RESP_CMEERR_INVALID,       /*"+CME ERROR:21"*/
    AT_4G_RESP_CMEERR_NOFIND,        /*"+CME ERROR:22"*/
    AT_4G_RESP_CMEERR_TOOLONG,       /*"+CME ERROR:24"*/
    AT_4G_RESP_CMEERR_NONETSERVICE,  /*"+CME ERROR:30"*/
    AT_4G_RESP_CMEERR_NETTIMEOUT,    /*"+CME ERROR:31"*/
    AT_4G_RESP_CMEERR_OTHER,         /*"other cme error"*/
    AT_4G_RESP_CMSERR_NOALLOWED,     /*"+CMS ERROR:302"*/
    AT_4G_RESP_CMSERR_NOSUPPORT,     /*"+CMS ERROR:303"*/
    AT_4G_RESP_CMSERR_INVALID_PDU,   /*"+CMS ERROR:304"*/
    AT_4G_RESP_CMSERR_INVALID_TXT,   /*"+CMS ERROR:305"*/
    AT_4G_RESP_CMSERR_SIMNOINSERT,   /*"+CMS ERROR:310"*/
    AT_4G_RESP_CMSERR_SIMFAILED,     /*"+CMS ERROR:313"*/
    AT_4G_RESP_CMSERR_SIMBUSY,       /*"+CMS ERROR:314"*/
    AT_4G_RESP_CMSERR_MEMFULL,       /*"+CMS ERROR:322"*/
    AT_4G_RESP_CMSERR_NONET,         /*"+CMS ERROR:331"*/
    AT_4G_RESP_CMSERR_NETTIMEOUT,    /*"+CMS ERROR:332"*/
    AT_4G_RESP_CMSERR_SIMNOTREADY,   /*+CMS ERROR:512*/
    AT_4G_RESP_CMSERR_INVALIDPARAM,  /*"+CMS ERROR:514"*/
    AT_4G_RESP_CMSERR_INVALIDSVCMOD, /*"+CMS ERROR:517"*/
    AT_4G_RESP_MAX_ERROR = AT_4G_RESP_CMSERR_INVALIDSVCMOD,
    AT_4G_RESP_CPIN_READY,           /*"+CPIN:READY"*/
    AT_4G_RESP_CPIN_PIN,             /*"+CPIN:*PIN"*/
    AT_4G_RESP_CPIN_PUK,             /* +CPIN:*PUK*/
    AT_4G_RESP_CPIN_NOINSERT,        /* +CPIN:NOT INSERTED*/
    AT_4G_RESP_READYTOSEND,          /*">"*/
    AT_4G_RESP_SENDOK,               /*"SEND OK"*/
    AT_4G_RESP_RING,                 /*"RING"*/
    AT_4G_RESP_SIM_INIT,             /*"+QINISTAT:0"*/
    AT_4G_RESP_SIM_READY,            /*"+QINISTAT:1"*/
    AT_4G_RESP_SIM_SMSDONE,          /*"+QINISTAT:2"*/
    AT_4G_RESP_SIM_PBDONE,           /*"+QINISTAT:4"*/
    AT_4G_RESP_SIM_READY_SMSDONE,    /*"+QINISTAT:3"*/
    AT_4G_RESP_SIM_ALL,              /*"+QINISTAT:7"*/
    AT_4G_RESP_ME_READY,             /*"RDY"*/
    AT_4G_RESP_FUN_MINI,             /*"+CFUN:0"*/
    AT_4G_RESP_FUN_FULL,             /*"+CFUN:1"*/
    AT_4G_RESP_FUN_DISABLE_TRANS,    /*"+CFUN:4"*/
    AT_4G_RESP_SMS_DONE,             /*"+QIND: SMS DONE"*/
    AT_4G_RESP_PB_DONE,              /*"+QIND: PB DONE"*/
    AT_4G_RESP_IURC_CLOSE,           /*+QIURC: "closed",<connectID>*/
    AT_4G_RESP_IURC_FULL,            /*+QIURC: "incoming full"*/
    AT_4G_RESP_IURC_DEACTIVE,        /*+QIURC: "pdpdeact",<contextID>*/
}AT_4G_RESP_CODE;

typedef enum
{
    AT_4G_UNKNOWN_CMD = 0,
    AT_4G_TESTALIVE_CMD,            /*AT*/
    AT_4G_READ_BAUD,                /*AT+IPR?*/
    AT_4G_WRITE_BAUD,               /*AT+IPR=<rate>*/
    AT_4G_WRITE_ECHOMODE,           /*ATE<value>*/
    AT_4G_ENABLE_ERR_NUM,           /*AT+CMEE=1*/
    AT_4G_SET_AIRMODE,              /*AT+CFUN=0*/
    AT_4G_SET_NORMALMODE,           /*AT+CFUN=1*/
    AT_4G_QUERY_FUNMODE,            /*AT+CFUN?*/
    AT_4G_CONFIG_URCPORT,           /*AT+QURCCFG="urcport","uart1"\r\n*/
    AT_4G_PWROFF,                   /*AT+QPOWD*/
    AT_4G_RESET,                    /*AT+CFUN=15*/
    AT_4G_SLEEP,                    /*AT+QSCLK=1*/
    AT_4G_WAKEUP,                   /*AT+QSCLK=0*/
    AT_4G_QUERY_IMEI,               /*AT+GSN*/
    AT_4G_QUERY_INITSTATE,          /*AT+QINISTAT*/
    AT_4G_QUERY_PINSTATE,           /*AT+CPIN?*/
    AT_4G_QUERY_ICCID,              /*AT+QCCID*/
    AT_4G_QUERY_IMSI,               /*AT+CIMI*/
    AT_4G_ENABLE_SETNUM,            /*AT+CPBS="SM"*/
    AT_4G_QUERY_NUM,                /*AT+CNUM*/
    AT_4G_SET_NUM,                  /*AT+CPBW=1,<number>*/
    AT_4G_CREG,                     /*AT+CREG?*/
    AT_4G_GPRS_REG,                 /*AT+CGREG?*/
    AT_4G_CEREG,                    /*AT+CEREG?*/
    AT_4G_QUERY_SIGNALSTRENGTH,     /*AT+CSQ*/
    AT_4G_QUERY_PS_ATTACH_STATE,    /*AT+CGATT?*/
    AT_4G_CONFIG_AUTO_SCAN,         /*AT+COPS=0*/
    AT_4G_QUERY_COPS,               /*AT+COPS?*/
    AT_4G_CONFIG_LOCKAT_2G,         /*AT+QCFG=\"NWSCANMODE\",1*/
    AT_4G_GET_LACCELLID,            /*AT+CGREG=2 & AT+CREG?*/
    AT_4G_GET_OPERATOR,             /*AT+COPS?*/
    AT_4G_CONFIG_APN,               /*AT+QICSGP=<contextID>*/
    AT_4G_CONFIG_APN_AUTH,          /*AT+QICSGP=1,1,"username","password"*/
    AT_4G_QUERY_APN,                /*+QICSGP: (1-16),(1-3),<APN>,<username>,<password>,(0-3),(0-1)*/
    AT_4G_ACTIVE_APN,               /*AT+QIACT=<contextID>*/
    AT_4G_DEACTIVE_APN,             /*AT+QIDEACT=<contextID>*/
    AT_4G_OPEN_SOCKET,              /*AT+QIOPEN=<contextID>,<connectID>,<service_type>,<IP_address>/<domain_name>,<remote_port>[,<local_port>[,<access_mode>]]*/
    AT_4G_CLOSE_SOCKET,             /*AT+QICLOSE=<connectID>[,<timeout>]*/
    AT_4G_QUERY_SOCKET_STATE,       /*AT+QISTATE=<query_type>,<connectID>*/
    AT_4G_SOCKET_DATA_LEN,          /*AT+QISEND*/
    AT_4G_SOCKET_DATA,              /**/
    AT_4G_QUERY_CENTERNUM,          /*AT+CSCA?*/
    AT_4G_SET_CENTERNUM,            /*AT+CSCA=\"+8613800731500\"*/
    AT_4G_UPDATA_TZ,                /*AT+CTZU=1*/
    AT_4G_SMS_SETPDUMODE,           /*AT+CMGF=0*/
    AT_4G_SMS_SETTXTMODE,           /*AT+CMGF=1*/
    AT_4G_SMS_SENDLEN,              /*AT+CMGS=<length>*/
    AT_4G_SMS_SENDDATA,             /*sms send data*/
    AT_4G_SMS_DELALL,               /*AT+CMGD=1,4*/
    AT_4G_FTP_TRANS_SET,            /*AT+QFTPCFG*/
    AT_4G_FTP_LOGIN,                /*AT+QFTPOPEN*/
    AT_4G_FTP_CWD,                  /*AT+QFTPCWD*/
    AT_4G_FTP_GET_SIZE,             /*AT+QFTPSIZE*/
    AT_4G_FTP_DOWNLOAD,             /*AT+QFTPGET*/
    AT_4G_FTP_QUIT,                 /*AT+QFTPCLOSE*/
    AT_4G_FTP_STAT,                 /*AT+QFTPSTAT*/
    AT_4G_TIM_CCLK,                 /*AT+CCLK?*/
    AT_4G_TIM_CCLK_SETTIME,         /*AT+CCLK=<time>*/
    AT_4G_TIM_NTP,                  /*AT+QNTP*/
    AT_4G_CONFIG_URC_OTHER_OFF,		/*AT+QCFG="urc/ri/other","off"*/
    AT_4G_CONFIG_RING,				/*AT+QCFG="urc/ri/ring","pulse",120,1*/
    AT_4G_CONFIG_SMSINCOMING,		/*AT+QCFG="urc/ri/smsincoming","pulse",120,1*/
    AT_4G_QUERY_TEMP,               /*AT+QTEMP*/
    AT_4G_QUERY_MTID,               /*AT+CGMM*/
    AT_4G_QUERY_TAID,               /*AT+CGMR*/
    AT_4G_QUERY_SIMSTAE,            /*AT+QSIMSTAT?*/
}AT_4G_REQ_CMD;

typedef struct
{
    uint8  cmd_id;
    uint8  retry_count;
    uint8  state;
    INT8   sharm_index;
    uint16 tick;
    uint16 org_tick;
    AT_CMD_RESP resp;
}AT_4G_CMD;

typedef struct
{
    AT_4G_CMD cmd;
    AT_4G_CMD_PRIORITY  pri;
    uint8  bak_state;
    uint8 resp_code;
}AT_4G_CMD_RESP;

uint8 at_4g_common_resp(uint8 cmd, AT_4G_CMD_RESP *resp, uint8 *data, uint16 *len);

#endif /* TBOX_4G_AT_H*/

