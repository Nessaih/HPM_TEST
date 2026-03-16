#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "checksum.h"

#define GNSS_CTRL_SEND_BUFF_SIZE  (128)

typedef enum
{
	GNSS_CTRL_RATE_1HZ = 1,
	GNSS_CTRL_RATE_2HZ = 2,
	GNSS_CTRL_RATE_4HZ = 4,
	GNSS_CTRL_RATE_5HZ = 5,
	GNSS_CTRL_RATE_10HZ = 10,
}GNSS_CTRL_RATE_E;

typedef enum
{
	GNSS_CTRL_MODE_GPS 				= 1,
	GNSS_CTRL_MODE_BDS				= 2,
	GNSS_CTRL_MODE_GPS_BDS			= 3,
	GNSS_CTRL_MODE_GLONASS			= 4,
	GNSS_CTRL_MODE_GPS_GLONASS		= 5,
	GNSS_CTRL_MODE_BDS_GLONASS		= 6,
	GNSS_CTRL_MODE_GPS_BDS_GLONASS	= 7,
}GNSS_CTRL_MODE_E;


static VOID gnss_control_send_data(UINT8 *data, UINT32 len)
{
	INT32 ret = 0;
	MODULE_LOG_I(GNSS, "len: %d, data: %s", len, data);
	
	ret = drv_eio_write(data, len);
	if(ret < 0)
	{
		MODULE_LOG_E(HPM, "gnss send data failed, ret: %d", ret);
	}
}

static VOID gnss_control_set_mode(VOID)
{
	INT32 ret = 0;
	UINT8 mode = 0;
	UINT8 data[GNSS_CTRL_SEND_BUFF_SIZE] = {0};
	TBOX_CFG_ID cfg_id;
	UINT8 checksum = 0;
	
    TBOX_CFG_ID_GET(GPSMODE, cfg_id);
    ret = tbox_cfg_read(cfg_id, &mode);
	if(0 != ret)
	{
		MODULE_LOG_E(GNSS, "gnss get gps mode failed, ret: %d", ret);
		return;
	}

	switch (mode)
	{
		case GNSS_CTRL_MODE_GPS:
		case GNSS_CTRL_MODE_BDS:
		case GNSS_CTRL_MODE_GPS_BDS:
		case GNSS_CTRL_MODE_GLONASS:
		case GNSS_CTRL_MODE_GPS_GLONASS:
		case GNSS_CTRL_MODE_BDS_GLONASS:
		case GNSS_CTRL_MODE_GPS_BDS_GLONASS:
			MODULE_LOG_I(GNSS, "gnss mode set: %d", mode);
			break;
		default:
			MODULE_LOG_E(GNSS, "gnss mode error, mode: %d", mode);
			return;
	}

	ret = snprintf((char *)data, GNSS_CTRL_SEND_BUFF_SIZE, "$PCAS04,%u", mode);
	if(ret < 0 || ret >= GNSS_CTRL_SEND_BUFF_SIZE)
	{
		MODULE_LOG_E(GNSS, "gnss format mode command failed, ret: %d", ret);
		return;
	}

	checksum = xor_checksum(&data[1], strlen((char *)data) - 1);

	MODULE_LOG_D(GNSS, "NMEA command: %s, checksum: %02X", data, checksum);

	snprintf((char *)data + strlen((char *)data), 
			 GNSS_CTRL_SEND_BUFF_SIZE - strlen((char *)data), 
			 "*%02X\r\n", checksum);

	gnss_control_send_data(data, strlen((char *)data));
}

static VOID gnss_control_set_rate(VOID)
{
	INT32 ret = 0;
	UINT16 rate = 0;
	UINT16 intv_ms = 0;
	UINT8 data[GNSS_CTRL_SEND_BUFF_SIZE] = {0};
	TBOX_CFG_ID cfg_id;
	UINT8 checksum = 0;
	
    TBOX_CFG_ID_GET(GPSRATE, cfg_id);
    ret = tbox_cfg_read(cfg_id, &rate);
	if(0 != ret)
	{
		MODULE_LOG_E(GNSS, "gnss get gps rate failed, ret: %d", ret);
		return;
	}

	switch (rate)
	{
		case GNSS_CTRL_RATE_1HZ:
		case GNSS_CTRL_RATE_2HZ:
		case GNSS_CTRL_RATE_4HZ:
		case GNSS_CTRL_RATE_5HZ:
		case GNSS_CTRL_RATE_10HZ:
			break;
		default:
			MODULE_LOG_E(GNSS, "gnss rate error, rate: %d", rate);
			return;
	}

	if(rate == 0)
	{
		MODULE_LOG_E(GNSS, "gnss rate is 0");
		return;
	}
	
	intv_ms = 1000 / rate;
	
	MODULE_LOG_I(GNSS, "gnss rate set: %dHz, interval: %dms", rate, intv_ms);
	
	ret = snprintf((char *)data, GNSS_CTRL_SEND_BUFF_SIZE, "$PCAS02,%u", intv_ms);
	if(ret < 0 || ret >= GNSS_CTRL_SEND_BUFF_SIZE)
	{
		MODULE_LOG_E(GNSS, "gnss format rate command failed, ret: %d", ret);
		return;
	}

	checksum = xor_checksum(&data[1], strlen((char *)data) - 1);

	MODULE_LOG_D(GNSS, "NMEA command: %s, checksum: %02X", data, checksum);

	snprintf((char *)data + strlen((char *)data), 
			 GNSS_CTRL_SEND_BUFF_SIZE - strlen((char *)data), 
			 "*%02X\r\n", checksum);

	gnss_control_send_data(data, strlen((char *)data));
}

static VOID gnss_control_save_flash(VOID)
{
	INT32 ret = 0;
	UINT8 checksum = 0;
	UINT8 data[GNSS_CTRL_SEND_BUFF_SIZE] = {0};

	ret = snprintf((char *)data, GNSS_CTRL_SEND_BUFF_SIZE, "$PCAS00");
	if(ret < 0 || ret >= GNSS_CTRL_SEND_BUFF_SIZE)
	{
		MODULE_LOG_E(GNSS, "gnss format save flash command failed, ret: %d", ret);
		return;
	}

	checksum = xor_checksum(&data[1], strlen((char *)data) - 1);

	MODULE_LOG_D(GNSS, "Save flash command: %s, checksum: %02X", data, checksum);

	snprintf((char *)data + strlen((char *)data), 
			 GNSS_CTRL_SEND_BUFF_SIZE - strlen((char *)data), 
			 "*%02X\r\n", checksum);

	gnss_control_send_data(data, strlen((char *)data));
}

VOID gnss_control_handle_cfg_change(TBOX_MSG_DATA *data)
{
	TBOX_CFG_CHANGE_INFO *info;

    if (NULL_PTR == data)
    {
        MODULE_LOG_E(GNSS, "invalid param");
        return;
    }

    info = (TBOX_CFG_CHANGE_INFO *)data->data;
    if (NULL_PTR == info)
    {
        MODULE_LOG_E(GNSS, "invalid param");
        return;
    }

	if (0 == strncmp(info->name, "GPSMODE", strlen(info->name)))
	{
		gnss_control_set_mode();
	}
	else if (0 == strncmp(info->name, "GPSRATE", strlen(info->name)))
	{
		gnss_control_set_rate();
	}
	
	
}


INT32 gnss_control_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			gnss_control_set_mode();
			gnss_control_set_rate();
            break;
            
        default:
            break;
    }
	
	return 0;
}

VOID gnss_control_wake(VOID)
{
}

VOID gnss_control_sleep(VOID)
{
	gnss_control_save_flash();
}

