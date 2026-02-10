#ifndef __UDS_IF_H__
#define __UDS_IF_H__

#include "service_id.h"

typedef enum {
    UDS_MSG_SERVER_IND = SERVICE_ID_UDS + 100,
    UDS_MSG_SERVER_REQ,
    UDS_MSG_SERVER_CON,

    UDS_MSG_CLIENT_IND,
    UDS_MSG_CLIENT_REQ,
    UDS_MSG_CLIENT_CON,

    UDS_MSG_CLT_CHN_SET_ACK,
    UDS_MSG_CLT_CHN_SET_REQ,

    UDS_MSG_DOWNLOAD_REQ,
    UDS_MSG_DOWNLOAD_RES,

    UDS_MSG_CTRL_CAN,
} UDS_MSG_ID;

#endif
