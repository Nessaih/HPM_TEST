#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_log.h"
#include "flash_common.h"
#include "version.h"


#include "hpm_content.h"
#include "hpm_cfg.h"


#define HPM_CFG_RUN_INFO_MIGC (0x11223344)

typedef INT32 (*hpm_para_get_handle)(UINT8 *data, UINT16 *out_len);
typedef INT32 (*hpm_para_set_handle)(UINT8 *data, UINT16 in_len);
typedef INT32 (*hpm_para_check_handle)(UINT8 *data, UINT16 in_len);

typedef struct HPM_CFG_TSP_ITEM
{
	UINT16   	optcode;
    hpm_para_get_handle  get;
    hpm_para_set_handle  set;	
    hpm_para_check_handle  check;		
} HPM_CFG_TSP_ITEM;


typedef struct
{
	UINT32 migc;
	UINT32 gps_odo;
	UINT32 acc_tim;
}HPM_CFG_RUN_INFO_T;

static HPM_CFG_RUN_INFO_T hpm_cfg_run_info;


static VOID hpm_cfg_run_info_write(VOID)
{
	INT32 ret = 0;
	hpm_cfg_run_info.migc = HPM_CFG_RUN_INFO_MIGC;
	drv_flash_nor_erase(FLASH_NOR_ADDR_HPM_RUN_INFO, 1);
	ret = drv_flash_nor_write(FLASH_NOR_ADDR_HPM_RUN_INFO, (uint8_t *)&hpm_cfg_run_info, sizeof(hpm_cfg_run_info));
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg write run info failed, ret: %d", ret);
	}
}

static VOID hpm_cfg_run_info_read(VOID)
{
	INT32 ret = 0;
	ret = drv_flash_nor_read(FLASH_NOR_ADDR_HPM_RUN_INFO, (uint8_t *)&hpm_cfg_run_info, sizeof(hpm_cfg_run_info));
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg run info read failed, ret: %d", ret);
		hpm_cfg_run_info_write();
		return;
	}

	if(HPM_CFG_RUN_INFO_MIGC != hpm_cfg_run_info.migc)
	{
		hpm_cfg_run_info_write();
	}

	return;
}

VOID hpm_cfg_changed_handle(TBOX_MSG_DATA *data)
{
	//TODO: cfg changed
	return;
}

INT32 hpm_cfg_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            memset(&hpm_cfg_run_info, 0, sizeof(hpm_cfg_run_info));			
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:           
			hpm_cfg_run_info_read();
            break;
            
        default:
            break;
    }

	return 0;
}

VOID hpm_cfg_wake(VOID)
{
	return;
}

VOID hpm_cfg_sleep(VOID)
{
	//写hpm企标参数
	//gps里程,acc时间,obdtype,休眠模式
	hpm_cfg_run_info_write();
	return;
}


UINT32 hpm_cfg_get_run_acc_time(VOID)
{
	UINT32 acc_time;
	
	HPM_MUTEX_LOCK();
	acc_time = hpm_cfg_run_info.acc_tim;
	HPM_MUTEX_UNLOCK();

	return acc_time;
}

VOID hpm_cfg_set_run_acc_time(UINT32 time)
{
	HPM_MUTEX_LOCK();
	hpm_cfg_run_info.acc_tim = time;
	HPM_MUTEX_UNLOCK();
}

UINT32 hpm_cfg_get_run_gps_odo(VOID)
{
	UINT32 odo;
	HPM_MUTEX_LOCK();
	odo = hpm_cfg_run_info.gps_odo;
	HPM_MUTEX_UNLOCK();
	
	return odo;
}

VOID hpm_cfg_set_run_gps_odo(UINT32 odo)
{	
	HPM_MUTEX_LOCK();
	hpm_cfg_run_info.gps_odo = odo;
	HPM_MUTEX_UNLOCK();
}

INT32 hpm_cfg_get_devid(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(DEVICEID, cfg_id);
	CHAR devid[TBOX_CFG_DEVICEID_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_DEVICEID_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get device id failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, devid);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get device id failed, ret: %d", ret);
		return -1;
	}
	
	memcpy(data, devid, len);
	return 0;
}

INT32 hpm_cfg_get_tracecode(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(TRACECODE, cfg_id);
	CHAR tcode[TBOX_CFG_TRACECODE_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_TRACECODE_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get traceode failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, tcode);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get traceode failed, ret: %d", ret);
		return -1;
	}
	
	memcpy(data, tcode, len);
	return 0;
}

INT32 hpm_cfg_get_vin(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(VIN, cfg_id);
	CHAR vin[TBOX_CFG_VIN_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_VIN_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get vin failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, vin);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get vin failed, ret: %d", ret);
		return -1;
	}
	
	memcpy(data, vin, len);
	return 0;
}


INT32 hpm_cfg_get_murl(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	CHAR url[TBOX_CFG_URL_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_URL_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get murl failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, url);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get murl failed, ret: %d", ret);
		return -1;
	}

	if(strlen(url) <= 0)
	{
		return -1;
	}
	
	memcpy(data, url, len);
	return 0;
}

INT32 hpm_cfg_get_mip(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMMIP, cfg_id);
	CHAR url[TBOX_CFG_IP_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_IP_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get mip failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, url);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get mip failed, ret: %d", ret);
		return -1;
	}

	if(strlen(url) <= 0)
	{
		return -1;
	}
	
	memcpy(data, url, len);
	return 0;
}


INT32 hpm_cfg_get_surl(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMSURL, cfg_id);
	CHAR url[TBOX_CFG_URL_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_URL_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get surl failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, url);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get surl failed, ret: %d", ret);
		return -1;
	}

	if(strlen(url) <= 0)
	{
		return -1;
	}
	
	memcpy(data, url, len);
	return 0;
}


INT32 hpm_cfg_get_sip(UINT8 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMSIP, cfg_id);
	CHAR url[TBOX_CFG_IP_LEN] = {0};

	if(NULL == data || len < TBOX_CFG_IP_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get sip failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, url);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get sip failed, ret: %d", ret);
		return -1;
	}

	if(strlen(url) <= 0)
	{
		return -1;
	}
	
	memcpy(data, url, len);
	return 0;
}


INT32 hpm_cfg_get_mport(UINT32 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMMPORT, cfg_id);
	UINT16 port = 0;

	if(NULL == data || len < TBOX_CFG_PORT_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get mport failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, &port);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get mport failed, ret: %d", ret);
		return -1;
	}

	if(port > 0)
	{
		*data = port;
		return 0;
	}
	
	return -1;
}

INT32 hpm_cfg_get_sport(UINT32 *data, INT32 len)
{	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMSPORT, cfg_id);
	UINT16 port = 0;

	if(NULL == data || len < TBOX_CFG_PORT_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get sport failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, &port);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get sport failed, ret: %d", ret);
		return -1;
	}
	
	if(port > 0)
	{
		*data = port;
		return 0;
	}
	
	return -1;
}

INT32 hpm_cfg_get_htbt(UINT32 *data, INT32 len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(HPMHTBT, cfg_id);
	UINT16 htbt = 0;

	if(NULL == data || len < TBOX_CFG_REPOT_INTV_LEN)
	{
		MODULE_LOG_E(HPM, "hpm cfg get htbt failed", ret);
		return -1;
	}

	ret = tbox_cfg_read(cfg_id, &htbt);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm cfg get sport failed, ret: %d", ret);
		return -1;
	}
	
	if(htbt > 0)
	{
		*data = htbt;
		return 0;
	}
	
	return -1;
}

/*────────────────────────────────────────DEFINE HPM Platform────────────────────────────────────────*/
static INT32 hpm_cfg_tsp_get_apn(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(PUBAPN, cfg_id);
	CHAR apn[TBOX_CFG_APN_LEN] = {0};
	
    ret = tbox_cfg_read(cfg_id, apn);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	strncpy((char *)data, apn, strlen(apn));
	*out_len = strlen((char *)data);
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_apn(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(in_len >= TBOX_CFG_APN_LEN || in_len <= 0)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_apn len:%d", in_len);
		return 0;
	}
	return 1;
}

static INT32 hpm_cfg_tsp_set_apn(UINT8 *data, UINT16 in_len)
{
	INT32 ret = 0;
	CHAR apn[TBOX_CFG_APN_LEN] = {0};
	memcpy(apn, data, in_len);

	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(PUBAPN, cfg_id);
    ret = tbox_cfg_write(cfg_id, apn);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_main_ip(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR ip[TBOX_CFG_IP_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMMIP, cfg_id);
    ret = tbox_cfg_read(cfg_id, ip);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	if (sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &data[0], &data[1], &data[2], &data[3]) != 4)
	{
		return HPM_CFG_RESP_NG;
	}
	
	*out_len = 4;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_main_ip(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(in_len != 4)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_main_ip len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_main_ip(UINT8 *data, UINT16 in_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR ip[TBOX_CFG_IP_LEN] = {0};

	sprintf(ip, "%hhu.%hhu.%hhu.%hhu", data[0], data[1], data[2], data[3]);
	
	TBOX_CFG_ID_GET(HPMMIP, cfg_id);
    ret = tbox_cfg_write(cfg_id, ip);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_main_url(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR ip[TBOX_CFG_URL_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	ret = tbox_cfg_read(cfg_id, ip);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	strncpy((char *)data, ip, strlen(ip));
	*out_len = strlen((char *)data);
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_mian_url(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(in_len >= TBOX_CFG_URL_LEN)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_mian_url len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_main_url(UINT8 *data, UINT16 in_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR url[TBOX_CFG_URL_LEN] = {0};
	memcpy(url, data, in_len);
	
	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	ret = tbox_cfg_write(cfg_id, url);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}
static INT32 hpm_cfg_tsp_get_slaver_ip(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR ip[TBOX_CFG_IP_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMSIP, cfg_id);
    ret = tbox_cfg_read(cfg_id, ip);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	if (sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &data[0], &data[1], &data[2], &data[3]) != 4)
	{
		return HPM_CFG_RESP_NG;
	}
	
	*out_len = 4;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_slaver_ip(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(in_len != 4)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_main_ip len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_slaver_ip(UINT8 *data, UINT16 in_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR ip[TBOX_CFG_IP_LEN] = {0};

	sprintf(ip, "%hhu.%hhu.%hhu.%hhu", data[0], data[1], data[2], data[3]);
	
	TBOX_CFG_ID_GET(HPMSIP, cfg_id);
    ret = tbox_cfg_write(cfg_id, ip);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_slaver_url(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR ip[TBOX_CFG_URL_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	ret = tbox_cfg_read(cfg_id, ip);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	strncpy((char *)data, ip, strlen(ip));
	*out_len = strlen((char *)data);
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_slaver_url(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(in_len >= TBOX_CFG_URL_LEN)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_mian_url len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_slaver_url(UINT8 *data, UINT16 in_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR url[TBOX_CFG_URL_LEN] = {0};
	memcpy(url, data, in_len);
	
	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	ret = tbox_cfg_write(cfg_id, url);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}


static INT32 hpm_cfg_tsp_get_main_port(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 port = 0;
	
	TBOX_CFG_ID_GET(HPMMPORT, cfg_id);
	ret = tbox_cfg_read(cfg_id, &port);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	data[0] = port >> 8;
	data[1] = port;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_mian_port(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(2 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_mian_port len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_main_port(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);

	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 port = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(HPMMPORT, cfg_id);
	ret = tbox_cfg_read(cfg_id, &port);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_slaver_port(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 port = 0;
	
	TBOX_CFG_ID_GET(HPMSPORT, cfg_id);
	ret = tbox_cfg_read(cfg_id, &port);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = port >> 8;
	data[1] = port;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_slaver_port(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(2 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_tsp_check_slaver_port len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_slaver_port(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 port = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(HPMSPORT, cfg_id);
	ret = tbox_cfg_read(cfg_id, &port);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_heartbeat_intv(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 intv = 0;
	
	TBOX_CFG_ID_GET(HPMHTBT, cfg_id);
	ret = tbox_cfg_read(cfg_id, &intv);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = intv;
	
	*out_len = 1;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_heartbeat_intv(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(1 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_heartbeat_intv len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_heartbeat_intv(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 intv = data[0];
	
	TBOX_CFG_ID_GET(HPMHTBT, cfg_id);
	ret = tbox_cfg_read(cfg_id, &intv);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_sleep_delay(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 intv = 0;
	
	TBOX_CFG_ID_GET(HPMSLPDY, cfg_id);
	ret = tbox_cfg_read(cfg_id, &intv);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = intv >> 8;
	data[1] = intv;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_sleep_delay(UINT8 *data, UINT16 in_len)
{
	UINT32 sleep_delay = 0;
	if(2 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_sleep_delay len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}
	sleep_delay = (data[0] << 8) + data[1];
	if(sleep_delay <= 20)
	{
		MODULE_LOG_E(HPM, "sleep time must be bigger than 20,time:%d",(INT32)sleep_delay);	
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_sleep_delay(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 intv = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(HPMSLPDY, cfg_id);
	ret = tbox_cfg_read(cfg_id, &intv);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_report_intv(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 intv = 0;
	
	TBOX_CFG_ID_GET(HPMCYCON, cfg_id);
	ret = tbox_cfg_read(cfg_id, &intv);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = intv >> 8;
	data[1] = intv;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_report_intv(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(4 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_report_intv len:%d", in_len);		
		return 0;
	}

	return 1;
}

static INT32 hpm_cfg_tsp_set_report_intv(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT16 intv = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(HPMCYCON, cfg_id);
	ret = tbox_cfg_read(cfg_id, &intv);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_soft_ver(UINT8 *data, UINT16* out_len)
{
	const CHAR *ver = version_get(VERSION_TYPE_APP);
	strcpy((char *)data, ver);

	*out_len = strlen(ver);
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_hard_ver(UINT8 *data, UINT16* out_len)
{
	data[0] = '3';
	data[1] = '0';
	data[2] = 0;

	*out_len = 3;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_acc_time(UINT8 *data, UINT16* out_len)
{
	UINT32 acc_time = hpm_cfg_get_run_acc_time();
	data[0] = acc_time >> 24;
	data[1] = acc_time >> 16;	
	data[2] = acc_time >> 8;
	data[3] = acc_time;	
	*out_len = 4;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_acc_time(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(4 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_acc_time len:%d", in_len);		
		return 0;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_acc_time(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);

	UINT32 acc_time =  (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
	hpm_cfg_set_run_acc_time(acc_time);
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_gps_odo(UINT8 *data, UINT16* out_len)
{
	UINT32 gps_odo = hpm_cfg_get_run_gps_odo();
	data[0] = gps_odo >> 24;
	data[1] = gps_odo >> 16;	
	data[2] = gps_odo >> 8;
	data[3] = gps_odo;	
	*out_len = 4;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_gps_odo(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);
	if(4 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_gps_odo len:%d", in_len);		
		return 0;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_gps_odo(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);

	UINT32 gps_odo =  (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
	hpm_cfg_set_run_gps_odo(gps_odo);

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_sleep_mode(UINT8 *data, UINT16* out_len)
{
#if 0
	UINT8 sleep_mode = 0;
	dev_cfg_get(CFG_ITEM_SLEEP_MODE,(UINT8 *)&sleep_mode);
	data[0] = sleep_mode;

	*out_len = 1;
	return HPM_CFG_RESP_OK;
#else
	return HPM_CFG_RESP_NG;
#endif

}

static INT32 hpm_cfg_tsp_check_sleep_mode(UINT8 *data, UINT16 in_len)
{
#if 0
	UNUSED(data);

	if(1 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_sleep_mode len:%d", in_len);
		return 0;
	}

	return HPM_CFG_RESP_OK;
#else
	return HPM_CFG_RESP_NG;
#endif
}

static INT32 hpm_cfg_tsp_set_sleep_mode(UINT8 *data, UINT16 in_len)
{
#if 0
	UNUSED(in_len);

	UINT8 sleep_mode =  data[0];


	return ret;
#else
	return HPM_CFG_RESP_NG;
#endif
}

static INT32 hpm_cfg_tsp_get_time_zone(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT8 time_zone = 0;
	
	TBOX_CFG_ID_GET(TIMEZONE, cfg_id);
	ret = tbox_cfg_read(cfg_id, &time_zone);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = time_zone;
	
	*out_len = 1;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_time_zone(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);

	if(1 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_time_zone len:%d", in_len);
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_time_zone(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT8 time_zone = data[0];
	
	TBOX_CFG_ID_GET(TIMEZONE, cfg_id);
	ret = tbox_cfg_read(cfg_id, &time_zone);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_vin(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR vin[TBOX_CFG_VIN_LEN] = {0};
	
	TBOX_CFG_ID_GET(VIN, cfg_id);
	ret = tbox_cfg_read(cfg_id, vin);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}

	strncpy((char *)data, vin, strlen(vin));
	*out_len = strlen((char *)data);
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_vin(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);

	if(17 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_vin len:%d", in_len);		
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_vin(UINT8 *data, UINT16 in_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	CHAR vin[TBOX_CFG_VIN_LEN] = {0};
	memcpy(vin, data, in_len);
	
	TBOX_CFG_ID_GET(VIN, cfg_id);
	ret = tbox_cfg_write(cfg_id, vin);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_obd_type(UINT8 *data, UINT16* out_len)
{
#if 0
	UINT8 obd_type = 0;
	dev_cfg_get(CFG_ITEM_HPM_OBD_TYPE,(UINT8 *)&obd_type);
	data[0] = obd_type;

	*out_len = 1;
	return HPM_CFG_RESP_OK;
#else
	return HPM_CFG_RESP_OK;
#endif
}

static INT32 hpm_cfg_tsp_check_obd_type(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);

	if(0 != data[0] && 1 != data[0] && 2 != data[0] && 3 != data[0])
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_obd_type len:%d", in_len);
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_obd_type(UINT8 *data, UINT16 in_len)
{
#if 0
	UNUSED(in_len);

	UINT8 obd_type =  data[0];

	INT32 ret =  dev_cfg_set(CFG_ITEM_HPM_OBD_TYPE, (UINT8 *)&obd_type);

	return ret;
#else
	return HPM_CFG_RESP_OK;
#endif
}

static INT32 hpm_cfg_tsp_get_baud1(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 baud = 0;
	
	TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
	ret = tbox_cfg_read(cfg_id, &baud);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = baud >> 8;
	data[1] = baud;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;

}

static INT32 hpm_cfg_tsp_check_baud1(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);

	if(2 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_baud1 len:%d", in_len);	
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_baud1(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 baud = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
	ret = tbox_cfg_read(cfg_id, &baud);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_baud2(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 baud = 0;
	
	TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
	ret = tbox_cfg_read(cfg_id, &baud);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = baud >> 8;
	data[1] = baud;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_baud2(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);

	if(2 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_baud2 len:%d", in_len);		
		return 0;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_baud2(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 baud = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
	ret = tbox_cfg_read(cfg_id, &baud);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_baud3(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 baud = 0;
	
	TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
	ret = tbox_cfg_read(cfg_id, &baud);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = baud >> 8;
	data[1] = baud;
	
	*out_len = 2;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_baud3(UINT8 *data, UINT16 in_len)
{
	UNUSED(data);

	if(2 != in_len)
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_baud3 len:%d", in_len);		
		return HPM_CFG_RESP_NG;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_baud3(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 baud = (data[0] << 8) + data[1];
	
	TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
	ret = tbox_cfg_read(cfg_id, &baud);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_get_gps_mode(UINT8 *data, UINT16* out_len)
{
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 mode = 0;
	
	TBOX_CFG_ID_GET(GPSMODE, cfg_id);
	ret = tbox_cfg_read(cfg_id, &mode);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	data[0] = mode;
	
	*out_len = 1;
	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_check_gps_mode(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);

	if((data[0] > 7) || (in_len != 1))
	{
        MODULE_LOG_E(HPM, "hpm_cfg_check_gps_mode len:%d", in_len);
		return 0;
	}

	return HPM_CFG_RESP_OK;
}

static INT32 hpm_cfg_tsp_set_gps_mode(UINT8 *data, UINT16 in_len)
{
	UNUSED(in_len);
	
	INT32 ret = 0;
	TBOX_CFG_ID cfg_id;
	UINT32 mode = data[0];
	
	TBOX_CFG_ID_GET(GPSMODE, cfg_id);
	ret = tbox_cfg_read(cfg_id, &mode);
	if(0 != ret)
	{
		return HPM_CFG_RESP_NG;
	}
	
	return HPM_CFG_RESP_OK;
	
#if 0
	UNUSED(in_len);
	UINT8 gps_mode =  data[0];
	INT32 ret =  dev_cfg_set(CFG_ITEM_HPM_GPS_MODE, (UINT8 *)&gps_mode);
	switch(gps_mode)
	{
		case 1:
		{
			//GPS
			gnss_dev_send("$PCAS04,1*18\r\n", strlen("$PCAS04,1*18\r\n"));
			break;
		}
		case 2:
		{
			//BDS
			gnss_dev_send("$PCAS04,2*1B\r\n", strlen("$PCAS04,2*1B\r\n"));			
			break;
		}
		case 3:
		{
			//GPS + BDS
			gnss_dev_send("$PCAS04,3*1A\r\n", strlen("$PCAS04,3*1A\r\n"));				
			break;
		}
		case 4:
		{
			//GLONASS
			gnss_dev_send("$PCAS04,4*1D\r\n", strlen("$PCAS04,4*1D\r\n"));			
			break;
		}
		case 5:
		{
			//GPS + GLONASS
			gnss_dev_send("$PCAS04,5*1C\r\n", strlen("$PCAS04,5*1C\r\n"));				
			break;
		}
		case 6:
		{
			//BDS + GLONASS
			gnss_dev_send("$PCAS04,6*1F\r\n", strlen("$PCAS04,6*1F\r\n"));			
			break;
		}
		case 7:
		{
			//BDS + GPS + GLONASS
			gnss_dev_send("$PCAS04,7*1E\r\n", strlen("$PCAS04,7*1E\r\n"));			
			break;
		}
		default:
			break;
	}

	//gnss_dev_send("$PCAS00*01\r\n", strlen("$PCAS00*01\r\n"));
	
#endif
}

static HPM_CFG_TSP_ITEM hpm_cfg_tsp_table[] = {
	{HPM_CFG_TSP_PARA_APN ,				hpm_cfg_tsp_get_apn,			hpm_cfg_tsp_set_apn,			hpm_cfg_tsp_check_apn			},
	{HPM_CFG_TSP_PARA_MAIN_IP , 		hpm_cfg_tsp_get_main_ip,		hpm_cfg_tsp_set_main_ip,		hpm_cfg_tsp_check_main_ip		},
	{HPM_CFG_TSP_PARA_MAIN_URL , 		hpm_cfg_tsp_get_main_url,		hpm_cfg_tsp_set_main_url,		hpm_cfg_tsp_check_mian_url		},
	{HPM_CFG_TSP_PARA_SLAVER_IP ,		hpm_cfg_tsp_get_slaver_ip,		hpm_cfg_tsp_set_slaver_ip,		hpm_cfg_tsp_check_slaver_ip		},
	{HPM_CFG_TSP_PARA_SLAVER_URL ,		hpm_cfg_tsp_get_slaver_url,		hpm_cfg_tsp_set_slaver_url,		hpm_cfg_tsp_check_slaver_url 	},
	{HPM_CFG_TSP_PARA_MAIN_PORT ,		hpm_cfg_tsp_get_main_port, 		hpm_cfg_tsp_set_main_port, 		hpm_cfg_tsp_check_mian_port 	},
	{HPM_CFG_TSP_PARA_SLAVER_PORT , 	hpm_cfg_tsp_get_slaver_port,	hpm_cfg_tsp_set_slaver_port,	hpm_cfg_tsp_check_slaver_port 	},
	{HPM_CFG_TSP_PARA_HEART_BEAT_INTV ,	hpm_cfg_tsp_get_heartbeat_intv,	hpm_cfg_tsp_set_heartbeat_intv,	hpm_cfg_tsp_check_heartbeat_intv},
	{HPM_CFG_TSP_PARA_ENTER_SLEEP_TIME ,hpm_cfg_tsp_get_sleep_delay, 	hpm_cfg_tsp_set_sleep_delay, 	hpm_cfg_tsp_check_sleep_delay	},
	{HPM_CFG_TSP_PARA_REAL_DATA_INTV , 	hpm_cfg_tsp_get_report_intv,	hpm_cfg_tsp_set_report_intv,	hpm_cfg_tsp_check_report_intv	},
	{HPM_CFG_TSP_PARA_SOFT_VER ,		hpm_cfg_tsp_get_soft_ver,		NULL,						NULL								},
	{HPM_CFG_TSP_PARA_HARD_VER ,		hpm_cfg_tsp_get_hard_ver,		NULL,						NULL								},
	{HPM_CFG_TSP_PARA_ACC_TOTAL_TIME ,	hpm_cfg_tsp_get_acc_time,		hpm_cfg_tsp_set_acc_time,		hpm_cfg_tsp_check_acc_time		},
	{HPM_CFG_TSP_PARA_ACC_TOTAL_DIS ,	hpm_cfg_tsp_get_gps_odo,		hpm_cfg_tsp_set_gps_odo,		hpm_cfg_tsp_check_gps_odo		},
	{HPM_CFG_TSP_PARA_SLEEP_MODE_ENBLE ,hpm_cfg_tsp_get_sleep_mode,		hpm_cfg_tsp_set_sleep_mode,		hpm_cfg_tsp_check_sleep_mode	},
	{HPM_CFG_TSP_PARA_TIME_ZONE ,		hpm_cfg_tsp_get_time_zone, 		hpm_cfg_tsp_set_time_zone, 		hpm_cfg_tsp_check_time_zone		},
	{HPM_CFG_TSP_PARA_VIN ,				hpm_cfg_tsp_get_vin,			hpm_cfg_tsp_set_vin,			hpm_cfg_tsp_check_vin 			},
	{HPM_CFG_TSP_PARA_OBD_TYPE , 	    hpm_cfg_tsp_get_obd_type,		hpm_cfg_tsp_set_obd_type,		hpm_cfg_tsp_check_obd_type		},
	{HPM_CFG_TSP_PARA_CAN1_BAUD ,		hpm_cfg_tsp_get_baud1,			hpm_cfg_tsp_set_baud1,			hpm_cfg_tsp_check_baud1			},
	{HPM_CFG_TSP_PARA_CAN2_BAUD ,		hpm_cfg_tsp_get_baud2,			hpm_cfg_tsp_set_baud2,			hpm_cfg_tsp_check_baud2 		},
	{HPM_CFG_TSP_PARA_CAN3_BAUD ,		hpm_cfg_tsp_get_baud3,			hpm_cfg_tsp_set_baud3,			hpm_cfg_tsp_check_baud3 		},
	{HPM_CFG_TSP_PARA_GPS_MODE ,		hpm_cfg_tsp_get_gps_mode,		hpm_cfg_tsp_set_gps_mode,		hpm_cfg_tsp_check_gps_mode 		},
};


INT32 hpm_cfg_tsp_get_param(UINT8 *in_data, UINT16 in_len,UINT8 *out_data, UINT16 *out_len)
{
	UINT16 r_len = 0;
	UINT16 w_len = 0;	
	UINT16 index = 0;
	UINT16 one_len = 0;
	UINT16 one_cfg = 0;
	UINT8 tmp_pos = 0;
	INT32 ret = 0;

	out_data[w_len++] = 0; //len
	out_data[w_len++] = 0;
	
	for(r_len = 0;r_len < in_len ;r_len += 2)
	{
		one_cfg = (in_data[r_len] << 8)+in_data[r_len+1];
		if(one_cfg >= HPM_CFG_TSP_PARA_MIN && one_cfg <= HPM_CFG_TSP_PARA_MAX)
		{
			for(index = 0;index < sizeof(hpm_cfg_tsp_table)/sizeof(HPM_CFG_TSP_ITEM);index ++)
			{
				if(one_cfg == hpm_cfg_tsp_table[index].optcode && NULL != hpm_cfg_tsp_table[index].get)
				{
					out_data[w_len++] = one_cfg >> 8;
					out_data[w_len++] = one_cfg;
					tmp_pos = w_len;
					w_len ++;
					ret = hpm_cfg_tsp_table[index].get(out_data + w_len,&one_len);
					if(HPM_CFG_RESP_OK != ret)
					{
						break;
					}
					out_data[tmp_pos] = one_len;
					w_len += one_len;
				}
			}
		}
	}

	if(HPM_CFG_RESP_OK != ret)
	{
		out_data[0] = 0x00;
		out_data[1] = 0x00;
		*out_len = 2;
	}
	else
	{
		out_data[0] = w_len >> 8;
		out_data[1] = w_len;
		*out_len = w_len;
	}
	
	return ret;
}

INT32 hpm_cfg_tsp_set_param(UINT8 *in_data, UINT16 in_len,UINT8 *out_data, UINT16 *out_len)
{
	UINT16 r_len;
	UINT16 index = 0;
	UINT16 one_len = 0;
	UINT16 one_cfg = 0;
	INT32 check_flag = 0;//0:failed 1:success

	for(r_len = 0;r_len < in_len ;r_len += one_len)
	{
		one_cfg = (in_data[r_len] << 8)+in_data[r_len+1];	
		r_len += 2;
		one_len = in_data[r_len++];
		if(one_cfg >= HPM_CFG_TSP_PARA_MIN && one_cfg <= HPM_CFG_TSP_PARA_MAX)
		{
			for(index = 0;index < sizeof(hpm_cfg_tsp_table)/sizeof(HPM_CFG_TSP_ITEM);index ++)
			{
				if(one_cfg == hpm_cfg_tsp_table[index].optcode && NULL != hpm_cfg_tsp_table[index].check)
				{
					check_flag = hpm_cfg_tsp_table[index].check(in_data + r_len,one_len);
					if(HPM_CFG_RESP_OK != check_flag)
						break;
				}
			}
		}	
		if(HPM_CFG_RESP_OK != check_flag)
			break;		
	}

	if(HPM_CFG_RESP_OK == check_flag)
	{
		for(r_len = 0;r_len < in_len ;r_len += one_len)
		{
			one_cfg = (in_data[r_len] << 8)+in_data[r_len+1];	
			r_len += 2;	
			one_len = in_data[r_len++];
			if(one_cfg >= HPM_CFG_TSP_PARA_MIN && one_cfg <= HPM_CFG_TSP_PARA_MAX)
			{
				for(index = 0;index < sizeof(hpm_cfg_tsp_table)/sizeof(HPM_CFG_TSP_ITEM);index ++)
				{
					if(one_cfg == hpm_cfg_tsp_table[index].optcode && NULL != hpm_cfg_tsp_table[index].set)
					{
						hpm_cfg_tsp_table[index].set(in_data + r_len,one_len);
					}
				}
			}		
		}
	}

	out_data[0] = 0x00;	//len
	out_data[1] = 0x00;
	*out_len = 2;

	return check_flag;
}
/*────────────────────────────────────────ENDDEF HPM Platform────────────────────────────────────────*/

