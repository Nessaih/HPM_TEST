#ifndef __FCT_IF_H__
#define __FCT_IF_H__

#define FCT_TEST_FLAG (0x12abcded)

#if 0   //open version handle
typedef enum {

    FCT_MSG_ID_IUC_TEST_REQ = SERVICE_ID_FCT + 100,
    FCT_MSG_ID_IUC_TEST_RES,
    FCT_MSG_ID_IUC_TEST_SEND_CAN_MSG,
    FCT_MSG_ID_IUC_TEST_RING,
    FCT_MSG_ID_IUC_TEST_LISTEN,
    FCT_MSG_ID_IUC_TEST_SLEEP,
    FCT_MSG_ID_IUC_TEST_CFG_CAN_MSG,
    FCT_MSG_ID_IUC_TEST_IO_REQ,
    FCT_MSG_ID_IUC_TEST_IO_RES,
    FCT_MSG_ID_MAX,
} FCT_MSG_ID;

void fct_iuc_test_ring(uint8_t *data, uint16_t len);
void fct_iuc_test_listen(uint8_t *data, uint16_t len);
void fct_iuc_test_sleep(uint8_t *data, uint16_t len);
void fct_iuc_req_io_status(uint8_t *data, uint16_t len);
void fct_iuc_can_send(uint8_t *data, uint16_t len);
void fct_iuc_can_init(uint8_t *data, uint16_t len);
#endif

#endif //__FCT_IF_H__
