#ifndef __HPM_PACK_H__
#define __HPM_PACK_H__

typedef enum
{
    HPM_CMD_LOGIN        	= 0x01,
    HPM_CMD_LIVE_DATA    	= 0x02,
    HPM_CMD_REISSUE_DATA 	= 0x03,
    HPM_CMD_LOGOUT       	= 0x04,
    HPM_CMD_HEART_BEAT   	= 0x05,
    HPM_CMD_TBOX_COMMON_ACK = 0x06,
    HPM_CMD_CONTROL      	= 0x81,
    HPM_CMD_TSP_COMMON_ACK  = 0x82,    
    HPM_CMD_UPGRADE_ACK 	= 0x89,
} HPM_CMD_TYPE;

typedef struct
{
    UINT16 sof;
    UINT8  cmd;
    UINT8  imei[20];
    UINT8  ver;
    UINT8  ecp : 4;
    UINT8  cps : 4;
    UINT8  lenh;
    UINT8  lenl;
    UINT8  data[1];
} HPM_PACK_FRAME_T;

typedef enum
{
  HPM_RECV_RESP_FAILED = 0,
  HPM_RECV_RESP_SUCCESS = 1,  	
  HPM_RECV_RESP_MAX
}HPM_RECV_RESP_FLAG;

INT32 hpm_pack_login(UINT8 *buf);

INT32 hpm_pack_logout(UINT8 *buf);

INT32 hpm_pack_heartbeat(UINT8 *buf);

INT32 hpm_pack_unpack(UINT8 *in, UINT16 inlen, HPM_PACK_FRAME_T *parsebuf, UINT16 *parselen);

#endif
