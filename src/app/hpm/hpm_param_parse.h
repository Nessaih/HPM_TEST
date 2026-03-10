#ifndef HPM_PARAM_PARSE_H
#define HPM_PARAM_PARSE_H

#include "tbox_common.h"

typedef enum
{
    HPM_FETCH_PROTOCOL_UDS = 0,
    HPM_FETCH_PROTOCOL_27145 = 1,
    HPM_FETCH_PROTOCOL_J1939 = 2,
    HPM_FETCH_PROTOCOL_15031 = 3,
} hpm_fetch_protocol_e;

typedef enum
{
    HPM_FETCH_CAN_RATE_0 = 0,
    HPM_FETCH_CAN_RATE_250 = 1,
    HPM_FETCH_CAN_RATE_500 = 2,
    HPM_FETCH_CAN_RATE_1000 = 3,
} hpm_fetch_can_rate_e;

typedef enum
{
    HPM_FETCH_NODE_INVALID = 0,
    HPM_FETCH_NODE_SINGLE = 1,     // 车辆主动广播单帧数据
    HPM_FETCH_NODE_CONSECTIVE = 2, // 车辆主动广播多帧企业自定义广播
    HPM_FETCH_NODE_BDPGN = 3,      // 车辆主动广播多帧按照J1939 BAM广播
    HPM_FETCH_NODE_PGN = 4,        // 终端按照J1939协议请求
    HPM_FETCH_NODE_UDS = 5,        // 终端按照UDS协议请求
} hpm_fetch_node_type_e;

typedef enum
{
    HPM_FETCH_LINE_INVALID = 0,
    HPM_FETCH_LINE_CO = 1,
    HPM_FETCH_LINE_ES = 2,
    HPM_FETCH_LINE_NS4 = 3,
    HPM_FETCH_LINE_NS6 = 4,
} hpm_fetch_line_type_e;

typedef enum
{
    HPM_FETCH_STATUS_UNUSED = 0,     // 未使用状态
    HPM_FETCH_STATUS_OBTAINED = 1,   // 已获取到数据
    HPM_FETCH_STATUS_UNOBTAINED = 2, // 未获取到数据
} hpm_fetch_status_e;

typedef struct
{
    union
    {
        UINT32 canid;
        struct
        {
            UINT32 ta : 8;
            UINT32 pgn : 16;
            UINT32 sa : 8;
        };
    };
    UINT32 respid;
    UINT32 did;
    UINT32 channel : 2;
    UINT32 obtained : 2;
    UINT32 type : 4;
    UINT32 offset : 4;
    UINT32 frame : 4;
    UINT32 len : 16;
} hpm_fetch_node_t;

#define HPM_FETCH_SINGLE_COUNT (64UL)
#define HPM_FETCH_MULTI_COUNT (10UL)

#define HPM_FETCH_NODE_COUNT (HPM_FETCH_SINGLE_COUNT + HPM_FETCH_MULTI_COUNT)

#define HPM_FECTCH_SINGLE_DATA_LEN (8UL)
#define HPM_FECTCH_MULTI_DATA_LEN (128UL)

typedef struct
{
    UINT32 magic_num;
    UINT32 sequence;
    UINT32 interval : 16;
    UINT32 can_rate1 : 4;
    UINT32 can_rate2 : 4;
    UINT32 can_rate3 : 4;
    UINT32 protocol : 4;
    UINT32 registered;
    UINT32 count;
    hpm_fetch_node_t node[HPM_FETCH_NODE_COUNT];
} hpm_fetch_config_t;

typedef enum
{
    HPM_FETCH_PARSE_STATUS_INIT,
    HPM_FETCH_PARSE_STATUS_LINE,
    HPM_FETCH_PARSE_STATUS_FIELD,
    HPM_FETCH_PARSE_STATUS_FINISH,
} hpm_fetch_parse_status_e;

typedef struct
{
    hpm_fetch_line_type_e type;
    hpm_fetch_parse_status_e status;
    UINT32 pos : 16;
    UINT32 len : 16;
    UINT8 *data;
} hpm_fetch_parse_t;

UINT32 hpm_param_get_baudrate(hpm_fetch_can_rate_e rate);
VOID hpm_param_skip_white_space(const char **p);
VOID hpm_param_skip_blank_line(const char **p);
UINT16 hpm_param_get_line(const char **p, char *line, UINT16 len);
INT32 hpm_param_split_str(const char **src, char *dst, UINT16 len, char delim);
hpm_fetch_line_type_e hpm_param_get_line_type(const char *line);
INT32 hpm_param_parse_config(hpm_fetch_config_t *config, const char *line, hpm_fetch_line_type_e type);
INT32 hpm_param_parse_node(hpm_fetch_node_t *node, const char *line);

#endif
