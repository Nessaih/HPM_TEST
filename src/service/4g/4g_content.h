#ifndef TBOX_4G_CONTENT_H
#define TBOX_4G_CONTENT_H

#include "4g_depend_header.h"

#define MGR_4G_TIMOUE_INTV		  (10U)

typedef enum
{
    SHARMMEM_4G_16BYTE = 0U,
    SHARMMEM_4G_32BYTE,
    SHARMMEM_4G_64BYTE,
    SHARMMEM_4G_128BYTE,
    SHARMMEM_4G_256BYTE
}SHARMMEM_4G_TYPE;

typedef enum
{
    AT_4G_CMD_LOW = 0U,
    AT_4G_CMD_MID,
    AT_4G_CMD_HIGH,
    AT_4G_CMD_DIRECT
}AT_4G_CMD_PRIORITY;

#define PERIODIC_UNIT_4G 	MGR_4G_TIMOUE_INTV

#define GBC_CONTEXT_ID    	IF_4G_PUBLIC_APN

#define SHAREMEM_4G_16_COUNT  8U
#define SHAREMEM_4G_32_COUNT  4U
#define SHAREMEM_4G_64_COUNT  2U
#define SHAREMEM_4G_128_COUNT 1U
#define SHAREMEM_4G_256_COUNT 1U

#define DEV_4G_INSTANCE       1U
#define DEV_4G_BAUDRATE       115200U
#define DEV_4G_RXBUF_SIZE     128U

#define AT_CMD_STATE_IDLE       (0U)
#define AT_CMD_STATE_REQ        (1U<<0U)
#define AT_CMD_STATE_WAITSEND   (1U<<1U)
#define AT_CMD_STATE_WAITRESP   (1U<<2U)
#define AT_CMD_SEND_REQ_NORESP    (AT_CMD_STATE_REQ|AT_CMD_STATE_WAITSEND)
#define AT_CMD_SEND_REQ_WAITRESP  (AT_CMD_STATE_REQ|AT_CMD_STATE_WAITSEND|AT_CMD_STATE_WAITRESP)

#define SIM_4G_ICCID_MAX       (21U)
#define SIM_4G_IMSI_MAX        (16U)
#define SIM_4G_NUM_MAX         (16U)
#define SIM_4G_LACCELLID_MAX   (21U)
#define NET_OPERATOR_NAME_MAX  (16U)
#define NET_APN_NAME_MAX       (32U)
#define SMS_4G_CENTERNUM_MAX   (21U)
#define MODEM_4G_IMEI_MAX      (15U)
#define SOCKET_4G_COUNT        (IF_4G_CONN_MAX)
#define SOCKET_4G_IPV4_LEN     (64U)
#define MODEM_4G_MTID_LEN      (32U)
#define MODEM_4G_TAID_LEN      (32U)

extern SemaphoreHandle_t tbox_4g_mutex;
#define TBOX_4G_MUTEX_LOCK()     xSemaphoreTake(tbox_4g_mutex, portMAX_DELAY)
#define TBOX_4G_MUTEX_UNLOCK()   xSemaphoreGive(tbox_4g_mutex)

UINT8 tbox_4g_get_state(VOID);

VOID tbox_4g_reset(VOID);

#endif /* TBOX_4G_CONTENT_H */
