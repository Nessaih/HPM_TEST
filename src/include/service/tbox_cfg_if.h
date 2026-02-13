#ifndef TBOX_CFG_IF_H
#define TBOX_CFG_IF_H

#include "tbox_common.h"
#include "tbox_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_DONOT_ERASE  0U
#define CFG_CAN_ERASE    1U

#define CFG_ID_INVALID -1
typedef UINT32 TBOX_CFG_ID;

/*定义默认配置项的长度*/
#define TBOX_CFG_DEVICEID_LEN           12U
#define TBOX_CFG_TRACECODE_LEN          32U
#define TBOX_CFG_SEID_LEN               20U
#define TBOX_CFG_BATTYPE_LEN            1U
#define TBOX_CFG_SLEEPMODE_LEN          1U
#define TBOX_CFG_SLEEPDELAY_LEN         4U
#define TBOX_CFG_RTCENABLE_LEN          1U
#define TBOX_CFG_RTCINTV_LEN            4U
#define TBOX_CFG_CANBAUD_LEN            2U
#define TBOX_CFG_TIMEZONE_LEN           1U
#define TBOX_CFG_APN_LEN                32U
#define TBOX_CFG_SMSCENTER_LEN          20U
#define TBOX_CFG_GPSMODE_LEN            1U
#define TBOX_CFG_GPSRATE_LEN            2U
#define TBOX_CFG_RESETCOUNT_LEN         4U
#define TBOX_CFG_VIN_LEN                18U /*实际长度为17字节，多出1字节用于字符串结束符*/
#define TBOX_CFG_URL_LEN          		64U
#define TBOX_CFG_IP_LEN          		16U
#define TBOX_CFG_PORT_LEN            	2U
#define TBOX_CFG_REPOT_INTV_LEN         2U
#define TBOX_CFG_RTCPOR_LEN             4U  /*RTC上电标志*/
typedef enum
{
    CFG_TYPE_NUMBER = 0,     /*10机制数字类型，定义默认值是10进展数字字符串*/
    CFG_TYPE_NUMBER16,       /*16机制数字类型，定义默认值是16进展数字字符串*/ 
    CFG_TYPE_STRING,         /*字符串类型*/
    CFG_TYPE_BYTE,           /*字节类型， 定义默认值是16进制字符串，每个字节用空格隔开，如"0x12 0x34 0x56"*/
    CFG_TYPE_INVALID
}TBOX_CFG_TYPE;

typedef struct
{
    CHAR *name;
    CHAR *def_value;
    UINT8 type;
    UINT8 earse_flag;
    UINT8 len;
}TBOX_CFG_INFO;

typedef struct
{
    CHAR *name;
    TBOX_CFG_ID id;
}TBOX_CFG_CHANGE_INFO;

#define TBOX_CFG_DEFINE(cfg_name, cfg_type, earse, cfg_def_value, cfg_len)\
    TBOX_CFG_ID cfg_##cfg_name##_id = CFG_ID_INVALID;\
    void cfg_##cfg_name##_reg(void);\
    void cfg_##cfg_name##_reg(void)\
    {\
        TBOX_CFG_INFO item = {#cfg_name, cfg_def_value, cfg_type, earse, cfg_len};\
        cfg_##cfg_name##_id = tbox_cfg_reg(&item);\
    }

#define TBOX_CFG_REG(cfg_name)\
    {void cfg_##cfg_name##_reg(void);\
     cfg_##cfg_name##_reg();}

#define TBOX_CFG_ID_GET(cfg_name, cfg_id) {extern TBOX_CFG_ID cfg_##cfg_name##_id; cfg_id = cfg_##cfg_name##_id;}

/*set default 和 value change 事件*/
#define TBOX_CFG_EVENT_SET_DEFAULT      "CFG_SET_DEFAULT"
#define TBOX_CFG_EVENT_VALUE_CHANGE     "CFG_VALUE_CHANGE"

INT32 tbox_cfg_reset(VOID);
INT32 tbox_cfg_reg(TBOX_CFG_INFO *item);
INT32 tbox_cfg_write(TBOX_CFG_ID id, VOID *buf);
INT32 tbox_cfg_read(TBOX_CFG_ID id, VOID *buf);
INT32 tbox_cfg_set_default(TBOX_CFG_ID id);
INT32 tbox_cfg_setkv(CHAR *key, VOID *value, UINT16 value_len);
INT32 tbox_cfg_getkv(CHAR *key, VOID *value, UINT16 value_len);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_CFG_IF_H */
