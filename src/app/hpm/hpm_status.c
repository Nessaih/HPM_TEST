#include <string.h>
#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_pm_io.h"
#include "gnss_if.h"
#include "time_if.h"
#include "analog_if.h"
#include "can_if.h"
#include "4g_if.h"
#include "version.h"
#include "hpm_pack.h"
#include "hpm_data.h"
#include "hpm_session.h"
#include "hpm_param_fetch.h"
#include "tbox_cfg_if.h"
#include "hpm_status.h"

#define HPM_STATUS_PACK_BUFF_LEN (512UL)
#define HPM_STATUS_TBOX (1)
#define HPM_STATUS_SIM_NUM_LEN (15UL)
#define HPM_STATUS_PARAM_SIZE (128UL)
#define HPM_STATUS_WAIT_TIMEOUT (45000UL) // 45s

typedef enum
{
    HPM_STATUS_STATE_INIT = 0,
    HPM_STATUS_STATE_ACCOFF,
    HPM_STATUS_STATE_ACCON,
    HPM_STATUS_STATE_IDLE,
    HPM_STATUS_STATE_WAKEUP,
    HPM_STATUS_STATE_MAX
} hpm_status_state_e;

typedef enum
{
    HPM_STATUS_EV_NONE = 0,
    HPM_STATUS_EV_ACCOFF = 0,
    HPM_STATUS_EV_ACCON,
    HPM_STATUS_EV_WAKEUP,
    HPM_STATUS_EV_MAX
} hpm_status_ev_e;

typedef enum
{
    HPM_STATUS_PARAM_POSITION_STATUS = 0x00,
    HPM_STATUS_PARAM_LONGITUDE = 0x01,
    HPM_STATUS_PARAM_LATITUDE = 0x02,
    HPM_STATUS_PARAM_SATELLITES = 0x03,
    HPM_STATUS_PARAM_ANTENNA_STATUS = 0x04,
    HPM_STATUS_PARAM_GNSS_ADC_LOW = 0x05,
    HPM_STATUS_PARAM_GNSS_ADC_HIGH = 0x06,
    HPM_STATUS_PARAM_SIGNAL_STRENGTH = 0x07,
    HPM_STATUS_PARAM_ICCID = 0x08,
    HPM_STATUS_PARAM_IMEI = 0x09,
    HPM_STATUS_PARAM_SIM_NUMBER = 0x0A,
    HPM_STATUS_PARAM_FLASH_STATUS = 0x0B,
    HPM_STATUS_PARAM_SIX_AXIS_STATUS = 0x0C,
    HPM_STATUS_PARAM_BLUETOOTH_STATUS = 0x0D,
    HPM_STATUS_PARAM_ACC_STATUS = 0x0E,
    HPM_STATUS_PARAM_LINK_STATUS = 0x0F,
    HPM_STATUS_PARAM_TAMPER_STATUS = 0x10,
    HPM_STATUS_PARAM_MAIN_VOLTAGE = 0x11,
    HPM_STATUS_PARAM_BACKUP_VOLTAGE = 0x12,
    HPM_STATUS_PARAM_MCU_RESET_COUNT = 0x13,
    HPM_STATUS_PARAM_MPU_RESET_COUNT = 0x14,
    HPM_STATUS_PARAM_CAN1_BAUD = 0x15,
    HPM_STATUS_PARAM_CAN2_BAUD = 0x16,
    HPM_STATUS_PARAM_CAN3_BAUD = 0x17,
    HPM_STATUS_PARAM_CAN1_AUTO = 0x18,
    HPM_STATUS_PARAM_CAN2_AUTO = 0x19,
    HPM_STATUS_PARAM_CAN3_AUTO = 0x1A,
    HPM_STATUS_PARAM_CAN1_STATUS = 0x1B,
    HPM_STATUS_PARAM_CAN2_STATUS = 0x1C,
    HPM_STATUS_PARAM_CAN3_STATUS = 0x1D,
    HPM_STATUS_PARAM_VIN = 0x1E,
    HPM_STATUS_PARAM_DBC_NAME = 0x1F,
    HPM_STATUS_PARAM_DEVICEID = 0x20,
    HPM_STATUS_PARAM_TRACECODE = 0x21,
    HPM_STATUS_PARAM_MPU_VERSION = 0x22,
    HPM_STATUS_PARAM_MCU_VERSION = 0x23,
    HPM_STATUS_PARAM_FIRMWARE_VERSION = 0x24,
    HPM_STATUS_PARAM_OBD_DIAG = 0x25,
    HPM_STATUS_PARAM_BASE_STATION_ID = 0x26,
    HPM_STATUS_PARAM_PUB_APN = 0x27,
    HPM_STATUS_PARAM_PRI_APN = 0x28,
    HPM_STATUS_PARAM_OTA_APN = 0x29,
    HPM_STATUS_PARAM_SMS_CENTER = 0x2A,
    HPM_STATUS_PARAM_MAX
} hpm_status_param_e;

static hpm_status_state_e hpm_status_state;
static UINT32 hpm_status_ticks;

static UINT8 hpm_status_get_param_position_status(UINT8 *buf)
{
    UINT8 len = 0;
    UINT8 status = 0;
    GNSS_POSITION_DATA pos;
    gnss_get_position(&pos);

    if (GNSS_POS_STATE_FIX != gnss_get_fix_state())
    {
        status |= 0x01; // Bit0: 无效定位
    }

    if (!pos.is_north)
    {
        status |= (1U << 1); // 南纬
    }

    if (!pos.is_east)
    {
        status |= (1U << 2); // 西经
    }

    buf[len++] = status;
    return len;
}

static UINT8 hpm_status_get_param_longitude(UINT8 *buf)
{
    UINT8 len = 0;
    GNSS_POSITION_DATA pos;
    gnss_get_position(&pos);
    UINT32 longitude = (UINT32)(pos.longitude * 1000000); // 精度0.000001°/bit
    buf[len++] = (UINT8)(longitude >> 24);
    buf[len++] = (UINT8)(longitude >> 16);
    buf[len++] = (UINT8)(longitude >> 8);
    buf[len++] = (UINT8)longitude;
    return len;
}

static UINT8 hpm_status_get_param_latitude(UINT8 *buf)
{
    UINT8 len = 0;
    GNSS_POSITION_DATA pos;
    gnss_get_position(&pos);
    UINT32 latitude = (UINT32)(pos.latitude * 1000000); // 精度0.000001°/bit
    buf[len++] = (UINT8)(latitude >> 24);
    buf[len++] = (UINT8)(latitude >> 16);
    buf[len++] = (UINT8)(latitude >> 8);
    buf[len++] = (UINT8)latitude;
    return len;
}

static UINT8 hpm_status_get_param_satellites(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = gnss_get_satellites();
    return len;
}

static UINT8 hpm_status_get_param_antenna_status(UINT8 *buf)
{
    UINT32 status = 0;
    UINT8 len = 0;
    ant_status_t lte_ant = analog_lte_ant_status();
    if (ANT_OPEN == lte_ant)
    {
        status |= (1U << 0); // 4G天线开路
    }
    else if (ANT_SHORT == lte_ant)
    {
        status |= (1U << 1); // 4G天线短路
    }

    ant_status_t gps_ant = analog_gps_ant_status();
    if (ANT_OPEN == gps_ant)
    {
        status |= (1U << 2); // GNSS天线开路
    }
    else if (ANT_SHORT == gps_ant)
    {
        status |= (1U << 3); // GNSS天线短路
    }

    if (IF_4G_STATE_CONNECTED == if_4g_get_call_state(IF_4G_PUBLIC_APN))
    {
        status |= (1U << 4); // 4G已连接
    }

    buf[len++] = (UINT8)status;
    return len;
}

static UINT8 hpm_status_get_param_gnss_adc_low(UINT8 *buf)
{
    UNUSED(buf);
    return 0;
}

static UINT8 hpm_status_get_param_gnss_adc_high(UINT8 *buf)
{
    UNUSED(buf);
    return 0;
}

static UINT8 hpm_status_get_param_signal_strength(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = if_4g_get_signal_signalstrength();
    return len;
}

static UINT8 hpm_status_get_param_iccid(UINT8 *buf)
{
    hpm_get_iccid(buf);
    return 20;
}

static UINT8 hpm_status_get_param_imei(UINT8 *buf)
{
    hpm_get_imei(buf);
    return 15;
}

static UINT8 hpm_status_get_param_sim_number(UINT8 *buf)
{
    UINT8 sim_number[IF_4G_MAX_PHONE_NUM_LEN] = {0};
    UINT8 len = IF_4G_MAX_PHONE_NUM_LEN;
    if_4g_get_phone_num(sim_number, &len);
    memcpy(buf, sim_number, HPM_STATUS_SIM_NUM_LEN);
    return (UINT8)HPM_STATUS_SIM_NUM_LEN;
}

static UINT8 hpm_status_get_param_flash_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = 0;
    return len;
}

static UINT8 hpm_status_get_param_six_axis_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = 0;
    return len;
}

static UINT8 hpm_status_get_param_bluetooth_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = 0;
    return len;
}

static UINT8 hpm_status_get_param_acc_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = tbox_pm_io_acc_is_active() ? 1 : 0;
    return len;
}

static UINT8 hpm_status_get_param_link_status(UINT8 *buf)
{
    UINT8 len = 0;
    UINT8 status = 0;
    for (UINT8 i = 0; i < IF_4G_CONN_MAX; i++)
    {
        if (IF_4G_STATE_CONNECTED == if_4g_get_socket_conn_state(i))
        {
            status |= (1U << i);
        }
    }
    buf[len++] = status;
    return len;
}

static UINT8 hpm_status_get_param_record_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = 0;
    return len;
}

static UINT8 hpm_status_get_param_main_voltage(UINT8 *buf)
{
    UINT8 len = 0;
    UINT16 vol = (UINT16)analog_pwr_vtg();
    buf[len++] = (UINT8)(vol >> 8);
    buf[len++] = (UINT8)vol;
    return len;
}

static UINT8 hpm_status_get_param_backup_voltage(UINT8 *buf)
{
    UINT8 len = 0;
    UINT16 vol = (UINT16)analog_bat_vtg();
    buf[len++] = (UINT8)(vol >> 8);
    buf[len++] = (UINT8)vol;
    return len;
}

static UINT8 hpm_status_get_param_mcu_reset_count(UINT8 *buf)
{
    UINT8 len = 0;
    TBOX_CFG_ID cfg_id;
    UINT32 count = 0xFFFFFFFF;
    TBOX_CFG_ID_GET(RESETCOUNT, cfg_id);
    tbox_cfg_read(cfg_id, &count);

    buf[len++] = (UINT8)(count >> 24);
    buf[len++] = (UINT8)(count >> 16);
    buf[len++] = (UINT8)(count >> 8);
    buf[len++] = (UINT8)count;
    return len;
}

static UINT8 hpm_status_get_param_mpu_reset_count(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 count = 0U;
    buf[len++] = (UINT8)(count >> 24);
    buf[len++] = (UINT8)(count >> 16);
    buf[len++] = (UINT8)(count >> 8);
    buf[len++] = (UINT8)count;
    return len;
}

static UINT8 hpm_status_get_param_can1_baud(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 baud = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
    tbox_cfg_read(cfg_id, &baud);

    buf[len++] = (UINT8)(baud >> 8);
    buf[len++] = (UINT8)baud;
    return len;
}

static UINT8 hpm_status_get_param_can2_baud(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 baud = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
    tbox_cfg_read(cfg_id, &baud);

    buf[len++] = (UINT8)(baud >> 8);
    buf[len++] = (UINT8)baud;
    return len;
}

static UINT8 hpm_status_get_param_can3_baud(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 baud = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
    tbox_cfg_read(cfg_id, &baud);

    buf[len++] = (UINT8)(baud >> 8);
    buf[len++] = (UINT8)baud;
    return len;
}

static UINT8 hpm_status_get_param_can1_auto(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 baud = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(CAN1AUTO, cfg_id);
    tbox_cfg_read(cfg_id, &baud);

    buf[len++] = (UINT8)(baud >> 8);
    buf[len++] = (UINT8)baud;
    return len;
}

static UINT8 hpm_status_get_param_can2_auto(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 baud = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(CAN2AUTO, cfg_id);
    tbox_cfg_read(cfg_id, &baud);

    buf[len++] = (UINT8)(baud >> 8);
    buf[len++] = (UINT8)baud;
    return len;
}

static UINT8 hpm_status_get_param_can3_auto(UINT8 *buf)
{
    UINT8 len = 0;
    UINT32 baud = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(CAN3AUTO, cfg_id);
    tbox_cfg_read(cfg_id, &baud);

    buf[len++] = (UINT8)(baud >> 8);
    buf[len++] = (UINT8)baud;
    return len;
}

static UINT8 hpm_status_get_can_state(UINT8 instance)
{
    UINT8 status = 0xFF;
    UINT8 can_status = can_if_state_get(instance);
    switch (can_status)
    {
    case CAN_INSTANCE_IDLE:
        status = 0;
        break;
    case CAN_INSTANCE_BUSY:
        status = 1;
        break;
    case CAN_INSTANCE_OFF:
        status = 2;
        break;
    case CAN_INSTANCE_ERROR:
        status = 3;
        break;
    default:
        break;
    }
    return status;
}

static UINT8 hpm_status_get_param_can1_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = hpm_status_get_can_state(0);
    return len;
}

static UINT8 hpm_status_get_param_can2_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = hpm_status_get_can_state(1);
    return len;
}

static UINT8 hpm_status_get_param_can3_status(UINT8 *buf)
{
    UINT8 len = 0;
    buf[len++] = hpm_status_get_can_state(2);
    return len;
}

static UINT8 hpm_status_get_param_vin(UINT8 *buf)
{
    UINT8 vin[18] = {0};
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(VIN, cfg_id);
    tbox_cfg_read(cfg_id, vin);
    memcpy(buf, vin, 17);
    return 17;
}

static UINT8 hpm_status_get_param_dbc_name(UINT8 *buf)
{
    UNUSED(buf);
    return 0;
}

static UINT8 hpm_status_get_param_deviceid(UINT8 *buf)
{
    UINT8 deviceid[TBOX_CFG_DEVICEID_LEN] = {0};
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(DEVICEID, cfg_id);
    tbox_cfg_read(cfg_id, deviceid);
    snprintf((char *)buf, sizeof(deviceid), "%s", (char *)deviceid);
    return strlen((char *)deviceid);
}

static UINT8 hpm_status_get_param_tracecode(UINT8 *buf)
{
    UINT8 tracecode[TBOX_CFG_TRACECODE_LEN] = {0};
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(TRACECODE, cfg_id);
    tbox_cfg_read(cfg_id, tracecode);
    snprintf((char *)buf, sizeof(tracecode), "%s", (char *)tracecode);
    return strlen((char *)tracecode);
}

static UINT8 hpm_status_get_param_mpu_version(UINT8 *buf)
{
    const char *version = version_get(VERSION_TYPE_BOOT);
    snprintf((char *)buf, 64, "%s", version);
    return strlen(version);
}

static UINT8 hpm_status_get_param_mcu_version(UINT8 *buf)
{
    const char *version = version_get(VERSION_TYPE_APP);
    snprintf((char *)buf, 64, "%s", version);
    return strlen(version);
}

static UINT8 hpm_status_get_param_firmware_version(UINT8 *buf)
{
    UINT8 version[64] = {0};
    if_4g_get_fwversion(version, sizeof(version));
    snprintf((char*)buf, 64, "%s", version);
    return strlen((char *)version);
}

static UINT8 hpm_status_get_param_obd_diag(UINT8 *buf)
{
    UNUSED(buf);
    return 0;
}

static UINT8 hpm_status_get_param_base_station_id(UINT8 *buf)
{
    UNUSED(buf);
    return 0;
}

static UINT8 hpm_status_get_param_pub_apn(UINT8 *buf)
{
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(PUBAPN, cfg_id);
    char apn[TBOX_CFG_APN_LEN] = {0};
    tbox_cfg_read(cfg_id, apn);
    snprintf((char *)buf, TBOX_CFG_APN_LEN, "%s", apn);
    return strlen((char *)apn);
}

static UINT8 hpm_status_get_param_pri_apn(UINT8 *buf)
{
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(PRIAPN, cfg_id);
    char apn[TBOX_CFG_APN_LEN] = {0};
    tbox_cfg_read(cfg_id, apn);
    snprintf((char *)buf, TBOX_CFG_APN_LEN, "%s", apn);
    return strlen((char *)apn);
}

static UINT8 hpm_status_get_param_ota_apn(UINT8 *buf)
{
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(OTAAPN, cfg_id);
    char apn[TBOX_CFG_APN_LEN] = {0};
    tbox_cfg_read(cfg_id, apn);
    snprintf((char *)buf, TBOX_CFG_APN_LEN, "%s", apn);
    return strlen((char *)apn);
}

static UINT8 hpm_status_get_param_sms_center(UINT8 *buf)
{
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(SMSCENTER, cfg_id);
    char sms_center[TBOX_CFG_SMSCENTER_LEN] = {0};
    tbox_cfg_read(cfg_id, sms_center);
    snprintf((char *)buf, TBOX_CFG_SMSCENTER_LEN, "%s", sms_center);
    return strlen((char *)sms_center);
}

typedef UINT8 (*hpm_status_param_get_func_t)(UINT8 *buf);

static hpm_status_param_get_func_t hpm_status_param_get_funcs[HPM_STATUS_PARAM_MAX] = {
    hpm_status_get_param_position_status,
    hpm_status_get_param_longitude,
    hpm_status_get_param_latitude,
    hpm_status_get_param_satellites,
    hpm_status_get_param_antenna_status,
    hpm_status_get_param_gnss_adc_low,
    hpm_status_get_param_gnss_adc_high,
    hpm_status_get_param_signal_strength,
    hpm_status_get_param_iccid,
    hpm_status_get_param_imei,
    hpm_status_get_param_sim_number,
    hpm_status_get_param_flash_status,
    hpm_status_get_param_six_axis_status,
    hpm_status_get_param_bluetooth_status,
    hpm_status_get_param_acc_status,
    hpm_status_get_param_link_status,
    hpm_status_get_param_record_status,
    hpm_status_get_param_main_voltage,
    hpm_status_get_param_backup_voltage,
    hpm_status_get_param_mcu_reset_count,
    hpm_status_get_param_mpu_reset_count,
    hpm_status_get_param_can1_baud,
    hpm_status_get_param_can2_baud,
    hpm_status_get_param_can3_baud,
    hpm_status_get_param_can1_auto,
    hpm_status_get_param_can2_auto,
    hpm_status_get_param_can3_auto,
    hpm_status_get_param_can1_status,
    hpm_status_get_param_can2_status,
    hpm_status_get_param_can3_status,
    hpm_status_get_param_vin,
    hpm_status_get_param_dbc_name,
    hpm_status_get_param_deviceid,
    hpm_status_get_param_tracecode,
    hpm_status_get_param_mpu_version,
    hpm_status_get_param_mcu_version,
    hpm_status_get_param_firmware_version,
    hpm_status_get_param_obd_diag,
    hpm_status_get_param_base_station_id,
    hpm_status_get_param_pub_apn,
    hpm_status_get_param_pri_apn,
    hpm_status_get_param_ota_apn,
    hpm_status_get_param_sms_center,
};

static INT32 hpm_status_param_report(UINT8 *buf, INT32 remain_size)
{
    UINT16 len = 0;
    UINT8 data[HPM_STATUS_PARAM_SIZE] = {0};
    UINT8 *len_ptr = &buf[len];
    buf[len++] = 0;
    buf[len++] = 0;

    UINT8 count = 0;
    UINT8 *ptr_count = &buf[len];
    buf[len++] = 0;

    remain_size -= len;

    for (UINT8 i = 0; i < HPM_STATUS_PARAM_MAX; i++)
    {
        memset(data, 0, sizeof(data));
        UINT8 data_len = hpm_status_param_get_funcs[i](data);

        remain_size -= (2 + data_len);
        if (remain_size <= 0)
        {
            MODULE_LOG_E(HPM, "status data overflow");
            return -1;
        }

        if (0 == data_len)
        {
            buf[len++] = i;
            buf[len++] = 0;
        }
        else
        {
            buf[len++] = i;
            buf[len++] = data_len;
            memcpy(buf + len, data, data_len);
            len += data_len;
        }
        count++;
    }

    *ptr_count = count;
    len_ptr[0] = (UINT8)((len - 2) >> 8) & 0xFF;
    len_ptr[1] = (UINT8)(len - 2);
    return (INT32)len;
}

static INT32 hpm_status_get_param(UINT8 *buf, INT32 remain_size)
{
    INT32 len = 0;
    buf[len++] = HPM_STATUS_TBOX;
    DEV_TIME time = {0};
    time_if_get(&time);
    buf[len++] = (UINT8)time.year;
    buf[len++] = (UINT8)time.month;
    buf[len++] = (UINT8)time.day;
    buf[len++] = (UINT8)time.hour;
    buf[len++] = (UINT8)time.min;
    buf[len++] = (UINT8)time.sec;
    remain_size -= len;

    INT32 ret = hpm_status_param_report(buf + len, remain_size);
    if (ret <= 0)
    {
        MODULE_LOG_E(HPM, "hpm_status_param_report failed");
        return -1;
    }
    len += ret;

    return len;
}

static INT32 hpm_status_report_pack(UINT8 *buf)
{
    UINT8 count = 0;
    UINT16 len = 0;
    len += hpm_sesion_get_data_seq(buf + len);
    UINT8 *ptr_count = &buf[len];
    buf[len++] = 0;

    INT32 remin_size = HPM_STATUS_PACK_BUFF_LEN - len;
    INT32 ret = hpm_status_get_param(buf + len, remin_size);
    if (ret <= 0)
    {
        MODULE_LOG_E(HPM, "hpm_status_get_param failed");
        return -1;
    }

    count++;
    len += ret;
    *ptr_count = count;

    return len;
}

static INT32 hpm_status_report(VOID)
{
    UINT8 *buf = mempool_alloc(HPM_STATUS_PACK_BUFF_LEN);
    if (NULL_PTR == buf)
    {
        MODULE_LOG_E(HPM, "malloc status buf failed");
        return -1;
    }

    INT32 len = hpm_status_report_pack(buf);
    if (len <= 0)
    {
        MODULE_LOG_E(HPM, "status report pack failed");
        mempool_free(buf);
        return -1;
    }
    else
    {
        MODULE_LOG_DUMP(HPM, "status report", buf, len);
        // hpm_data_save_to_realtm_list(HPM_CMD_TBOX_STATUS, buf, len);
    }

    mempool_free(buf);
    return 0;
}

static VOID hpm_status_set_event(hpm_status_ev_e event)
{
    switch (event)
    {
    case HPM_STATUS_EV_ACCOFF:
        MODULE_LOG_I(HPM, "status report acc off");
        hpm_status_report();
        break;
    case HPM_STATUS_EV_ACCON:
        MODULE_LOG_I(HPM, "status report acc on");
        hpm_status_report();
        break;
    case HPM_STATUS_EV_WAKEUP:
        MODULE_LOG_I(HPM, "status report wakeup");
        hpm_status_report();
        break;
    default:
        break;
    }
}

static VOID hpm_status_set_state(hpm_status_state_e state)
{
    hpm_status_state = state;
}

static hpm_status_state_e hpm_status_get_state(VOID)
{
    return hpm_status_state;
}

static VOID hpm_status_handle_init(VOID)
{
    if (0 == hpm_status_ticks)
    {
        hpm_status_ticks = time_if_get_systick_ms();
    }

    if (TRUE == tbox_pm_io_acc_is_active())
    {
        hpm_status_set_state(HPM_STATUS_STATE_ACCON);
    }
}

static VOID hpm_status_handle_accon(VOID)
{
    if (FALSE == tbox_pm_io_acc_is_active())
    {
        hpm_status_set_state(HPM_STATUS_STATE_ACCOFF);
        return;
    }

    if (time_if_get_systick_ms() - hpm_status_ticks >= HPM_STATUS_WAIT_TIMEOUT)
    {
        hpm_status_ticks = time_if_get_systick_ms();
        hpm_status_set_event(HPM_STATUS_EV_ACCON);
        hpm_status_set_state(HPM_STATUS_STATE_IDLE);
    }
}

static VOID hpm_status_handle_accoff(VOID)
{
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        hpm_status_set_event(HPM_STATUS_EV_ACCON);
        hpm_status_set_state(HPM_STATUS_STATE_IDLE);
        return;
    }

    if (time_if_get_systick_ms() - hpm_status_ticks >= HPM_STATUS_WAIT_TIMEOUT)
    {
        hpm_status_ticks = time_if_get_systick_ms();
        hpm_status_set_event(HPM_STATUS_EV_ACCOFF);
        hpm_status_set_state(HPM_STATUS_STATE_INIT);
    }
}

static VOID hpm_status_handle_idle(VOID)
{
    if (FALSE == tbox_pm_io_acc_is_active())
    {
        hpm_status_set_event(HPM_STATUS_EV_ACCOFF);
        hpm_status_set_state(HPM_STATUS_STATE_ACCOFF);
    }
}

static VOID hpm_status_handle_wakeup(VOID)
{
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        hpm_status_set_state(HPM_STATUS_STATE_ACCON);
        return;
    }

    if (time_if_get_systick_ms() - hpm_status_ticks >= HPM_STATUS_WAIT_TIMEOUT)
    {
        hpm_status_ticks = time_if_get_systick_ms();
        hpm_status_set_state(HPM_STATUS_STATE_INIT);
        hpm_status_set_event(HPM_STATUS_EV_WAKEUP);
    }
}

INT32 hpm_status_init(UINT8 seq)
{
    if (seq == MODULE_INIT_SEQ_MODULE)
    {
        hpm_status_state = HPM_STATUS_STATE_INIT;
        hpm_status_ticks = 0;
    }
    return 0;
}

VOID hpm_status_wakeup(VOID)
{
    hpm_status_set_state(HPM_STATUS_STATE_WAKEUP);
    hpm_status_ticks = time_if_get_systick_ms();
}

VOID hpm_status_sleep(VOID)
{
    hpm_status_set_state(HPM_STATUS_STATE_INIT);
    hpm_status_ticks = time_if_get_systick_ms();
}

VOID hpm_status_process(VOID)
{
    hpm_status_state_e state = hpm_status_get_state();
    switch (state)
    {
    case HPM_STATUS_STATE_INIT:
        hpm_status_handle_init();
        break;
    case HPM_STATUS_STATE_ACCON:
        hpm_status_handle_accon();
        break;
    case HPM_STATUS_STATE_ACCOFF:
        hpm_status_handle_accoff();
        break;
    case HPM_STATUS_STATE_IDLE:
        hpm_status_handle_idle();
        break;
    case HPM_STATUS_STATE_WAKEUP:
        hpm_status_handle_wakeup();
        break;
    default:
        break;
    }
}
