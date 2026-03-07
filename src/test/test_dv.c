#include "tbox_cfg_if.h"
#include "tbox_log.h"
#include "tbox_pm_io.h"
#include "4g_if.h"
#include "analog_if.h"
#include "can_if.h"
#include "gnss_if.h"
#include "se_if.h"
#include "time_if.h"
#include "api_rtos.h"
#include "macros.h"
#include "test_dv.h"

#define DV_DIAG_DES_STR_MAX_LEN (128)
#define DV_DIAG_LOG_INFO_MIGC   (0x11223344)

typedef INT8 (*DV_DIAG_PROC)(CHAR *des, INT32 deslen);

typedef enum
{
    DV_DIAG_OK = 0x00,
    DV_DIAG_NG = 0x01,
} DV_DIAG_STA_E;

typedef struct
{
    CHAR         *name;
    UINT8         intv;
    CHAR          des[DV_DIAG_DES_STR_MAX_LEN];
    DV_DIAG_STA_E sta;
    DV_DIAG_PROC  proc;
} DV_DIAG_INFO_T;

typedef struct
{
    BOOL  flag;
    INT32 len;
    UINT8 data[DV_DIAG_DES_STR_MAX_LEN];
} DV_DIAG_485_T;

typedef struct
{
    INT32  migc;
    BOOL   flag;
    UINT32 intv;
} DV_DIAG_LOG_T;

static VOID dv_diag_status_dump(VOID);

static DV_DIAG_LOG_T dv_diag_log_info;
static DV_DIAG_485_T dv_diag_485_info;

static UNION8 dv_diag_io_sta;

static TaskHandle_t  xTaskHandle  = NULL;
static TimerHandle_t xTimerHandle = NULL;

static INT8 dv_diag_4g_iccid(CHAR *des, INT32 deslen)
{
    UINT8 iccid[IF_4G_MAX_ICCID_LEN] = {0};
    UINT8 len                        = sizeof(iccid);

    if (FALSE == if_4g_get_iccid(iccid, &len))
    {
        return -1;
    }

    if (0 == strlen((CHAR *)iccid))
    {
        return -1;
    }

    snprintf(des, deslen, "+4gIccid:%s", iccid);

    return 0;
}

static INT8 dv_diag_4g_signal(CHAR *des, INT32 deslen)
{
    UINT8 ret;
    ret = if_4g_get_signal_signalstrength();
    if (ret >= 99)
    {
        return -1;
    }

    snprintf(des, deslen, "+4gSignal:%d", ret);
    return 0;
}

static INT8 dv_diag_4g_nat(CHAR *des, INT32 deslen)
{
    INT32       state;
    const CHAR *status_str[] = {"disconnected", "disconnecting", "connecting", "connected"};

    state = if_4g_get_call_state(IF_4G_PUBLIC_APN);
    snprintf(des, deslen, "+4gNat:%s", status_str[state]);
    if (IF_4G_STATE_CONNECTED == state)
    {
        return 0;
    }

    return -1;
}

static INT8 dv_diag_4g_ant(CHAR *des, INT32 deslen)
{
    INT32       state;
    const CHAR *status_str[] = {"normal", "open", "short", "unknow"};

    state = analog_lte_ant_status();
    snprintf(des, deslen, "+4gAnt:%s", status_str[state]);
    if (ANT_NORMAL == state)
    {
        return 0;
    }

    return -1;
}

static INT8 dv_diag_4g_com(CHAR *des, INT32 deslen)
{
    UINT8       state        = 0;
    const CHAR *status_str[] = {"hpm disconnected", "hpm connected"};

    if (IF_4G_STATE_CONNECTED == if_4g_get_socket_conn_state(IF_4G_HPM_CONN_ID))
    {
        state = 1;
    }

    snprintf(des, deslen, "+4gCom:%s", status_str[state]);

    if (FALSE == state)
    {
        return -1;
    }

    return 0;
}

static INT8 dv_diag_4g_tmp(CHAR *des, INT32 deslen)
{
    UINT8 temp = 0;
    temp       = if_4g_get_temperature();

    snprintf(des, deslen, "+4gTemp:%d", temp);
    return 0;
}

static INT8 dv_diag_gns_fix(CHAR *des, INT32 deslen)
{
    INT32       state;
    const CHAR *status_str[] = {"UNFIX", "FIX"};

    state = gnss_get_fix_state();
    snprintf(des, deslen, "+GnsSta:%s", status_str[state]);
    if (GNSS_POS_STATE_FIX == state)
    {
        return 0;
    }

    return -1;
}

static INT8 dv_diag_gns_ant(CHAR *des, INT32 deslen)
{
    INT32       state;
    const CHAR *status_str[] = {"normal", "open", "short", "unknow"};

    state = analog_gps_ant_status();
    snprintf(des, deslen, "+GnsAnt:%s", status_str[state]);
    if (ANT_NORMAL == state)
    {
        return 0;
    }

    return -1;
}

static INT8 __attribute__((unused)) dv_diag_ble_sta(CHAR *des, INT32 deslen)
{
    return -1;
#if 0
	CHAR *ver =	bt_ble_get_version();
	if(NULL == ver)
	{
		return -1;
	}

	snprintf(des, deslen, "+bleVer: %s", ver);
    return 0;
#endif
}

static INT8 __attribute__((unused)) dv_diag_ble_com(CHAR *des, INT32 deslen)
{
    return -1;
#if 0
	const CHAR *status_str[] = {"idle", "advertising", "wait connect", "connected", "disconnected"};
	BT_BLE_SVR_CONNECT_STA ble_sta = BT_BLE_SVR_CONNECT_IDLE;
	ble_sta = bt_ble_get_sta();

	snprintf(des, deslen, "+BleSta:%s", status_str[ble_sta]);

	if(BT_BLE_SVR_CONNECT_IDLE != ble_sta)
	{
		dv_diag_bt_test();
		return 0;
	}
	
    return -1;
#endif
}

static INT8 dv_diag_adc_light(CHAR *des, INT32 deslen)
{
    INT32 lht = 0;
    lht       = analog_light();

    snprintf(des, deslen, "+light:%d", lht);
    return 0;
}

static INT8 dv_diag_norflash_wr(CHAR *des, INT32 deslen)
{
#define EXFLASH_ADDR_DV_TEST 0x00000000

    UINT8 ret           = 0;
    UINT8 test_data[10] = {0x12, 0x34, 0x67, 0xAC, 0x71, 0xC7, 0x89, 0xDA, 0x65, 0x46};
    UINT8 rcv_data[10]  = {0};

    ret = drv_flash_nor_erase(EXFLASH_ADDR_DV_TEST, 1);
    if (0 != ret)
    {
        tbox_log_print("dv erase data failed, ret: %d\r\n", ret);
        return -1;
    }

    ret = drv_flash_nor_write(EXFLASH_ADDR_DV_TEST, test_data, sizeof(test_data));
    if (0 != ret)
    {
        tbox_log_print("dv write data failed, ret: %d\r\n", ret);
        return -1;
    }

    ret = drv_flash_nor_read(EXFLASH_ADDR_DV_TEST, rcv_data, sizeof(rcv_data));
    if (0 != ret)
    {
        tbox_log_print("dv read data failed, ret: %d\r\n", ret);
        return -1;
    }

    if (0 != memcmp(test_data, rcv_data, sizeof(rcv_data)))
    {
        tbox_log_print("dv cmp data failed\r\n");
        return -1;
    }

    snprintf(des, deslen, "NorFlash w/r:%d", ret);
    return 0;
}

static INT8 dv_diag_nandflash_wr(CHAR *des, INT32 deslen)
{
#if 0
#define NAND_ERASE_SIZE 0x20000U
#define NAND_WRBUF_SIZE 512U

    static uint32_t erase_addr  = 0;
    static uint32_t wr_addr     = 0;
    static uint32_t wr_len      = NAND_WRBUF_SIZE;
    static uint32_t test_count  = 0;
    static uint32_t error_count = 0;
    static bool     need_erase  = true;
    static bool     init_buf    = false;
    static uint8_t  wbuf[NAND_WRBUF_SIZE];
    static uint8_t  rbuf[NAND_WRBUF_SIZE];
    int32_t         status = 0;

    if (need_erase)
    {
        status = drv_flash_nand_erase(erase_addr, 1);
        if (status != 0)
        {
            snprintf(des, deslen, "EC/TC:%d/%d  EA:%08X  WRA:%08X EERR:%d", error_count, test_count, erase_addr, wr_addr, status);
            tbox_log_print("%s", des);
            error_count++;
            test_count++;
            return -1;
        }
        else
        {
            need_erase = false;
        }
    }

    if (init_buf == false)
    {
        for (size_t i = 0; i < NAND_WRBUF_SIZE; i++)
        {
            wbuf[i] = (uint8_t)i;
        }
        init_buf = true;
    }

    status = drv_flash_nand_write(wr_addr, wbuf, wr_len);
    if (status != 0)
    {
        snprintf(des, deslen, "EC/TC:%d/%d  EA:%08X  WRA:%08X WERR:%d", error_count, test_count, erase_addr, wr_addr, status);
        error_count++;
        goto next_nand_test;
    }

    status = drv_flash_nand_read(wr_addr, rbuf, wr_len);
    if (status != 0)
    {
        snprintf(des, deslen, "EC/TC:%d/%d  EA:%08X  WRA:%08X RERR:%d", error_count, test_count, erase_addr, wr_addr, status);
        error_count++;
        goto next_nand_test;
    }

    status = 0;
    for (size_t i = 0; i < NAND_WRBUF_SIZE; i++)
    {
        if (wbuf[i] != rbuf[i])
        {
            status = -1;
            tbox_log_print("OFFSET:%d W:%02X R:%02X\n", i, wbuf[i], rbuf[i]);
        }
    }

    if (0 != status)
    {
        snprintf(des, deslen, "EC/TC:%d/%d  EA:%08X  WRA:%08X CERR:%d", error_count, test_count, erase_addr, wr_addr, status);
        tbox_log_print("%s\n", des);
        error_count++;
        goto next_nand_test;
    }

    test_count++;
    snprintf(des, deslen, "EC/TC:%d/%d  EA:%08X  WRA:%08X SUCCESS", error_count, test_count, erase_addr, wr_addr);

next_nand_test:
    wr_addr += wr_len;
    if (wr_addr >= erase_addr + NAND_ERASE_SIZE)
    {
        need_erase = true;
    }
    return status;
#else
    return -1;
#endif
}

static INT8 __attribute__((unused)) dv_diag_se_spi(CHAR *des, INT32 deslen)
{
    return -1;
#if 0
	INT32  ret;
	UINT8  data_buf[SE_PUBKEY_MAX_LEN] = {0};	
	UINT16 data_len = SE_PUBKEY_MAX_LEN;
	UINT16 i;
	CHAR   hex_buf[SE_PUBKEY_MAX_LEN * 2 + 1] = {0};
	
	ret = se_get_pubkey(data_buf, &data_len);
	if(0 != ret)
	{
		return -1;
	}

	for(i = 0; i < data_len; i++)
	{
		snprintf(&hex_buf[i * 2], 3, "%02X", data_buf[i]);
	}
	
	snprintf(des, deslen, "PubKey:%s", hex_buf);
    return 0;
#endif
}

static INT8 dv_diag_rs485(CHAR *des, INT32 deslen)
{
    if (FALSE == dv_diag_485_info.flag)
    {
        snprintf(des, deslen, "+rs485:[flag:%d][len:%d]", dv_diag_485_info.flag, dv_diag_485_info.len);
        return -1;
    }

    snprintf(des, deslen, "+rs485:[flag: %d][len: %d][data: %s]", dv_diag_485_info.flag, dv_diag_485_info.len, dv_diag_485_info.data);
    return 0;
}

static INT8 dv_diag_can1_sta(CHAR *des, INT32 deslen)
{
    UINT8       sta          = 0;
    const CHAR *status_str[] = {"unused", "idle", "busy", "error", "off"};
    sta                      = can_if_state_get(0);
    snprintf(des, deslen, "+Can1Sta:%s", status_str[sta]);
    if (CAN_INSTANCE_IDLE == sta || CAN_INSTANCE_BUSY == sta)
    {
        return 0;
    }
    return -1;
}

static INT8 dv_diag_can2_sta(CHAR *des, INT32 deslen)
{
    UINT8       sta          = 0;
    const CHAR *status_str[] = {"unused", "idle", "busy", "error", "off"};
    sta                      = can_if_state_get(1);
    snprintf(des, deslen, "+Can2Sta:%s", status_str[sta]);
    if (CAN_INSTANCE_IDLE == sta || CAN_INSTANCE_BUSY == sta)
    {
        return 0;
    }
    return -1;
}

static INT8 dv_diag_can1_com(CHAR *des, INT32 deslen)
{
    INT32 count = -1;
    count       = can_if_get_recv_count(0);

    if (count <= 0)
    {
        return -1;
    }

    snprintf(des, deslen, "+Can1:%d", count);
    return 0;
}

static INT8 dv_diag_can2_com(CHAR *des, INT32 deslen)
{
    INT32 count = -1;
    count       = can_if_get_recv_count(1);

    if (count <= 0)
    {
        return -1;
    }

    snprintf(des, deslen, "+Can2:%d", count);
    return 0;
}

static INT8 dv_diag_io_state(CHAR *des, INT32 deslen)
{
    /*
     * B0: DOH[PC05, 94U]
     * B1: ACC[PE12, 19U]
     * SET: DOH->↑ -- GET: ACC->↓
     * 		DOH->↓ -- GET: ACC->↑
     *
     * B2: DOL[PD11, 35U]
     * B3: DIL[PB04, 28U]
     * SET: DOL->↓ -- GET: DIL->↓
     *		DOL->↑ -- GET: DIL->↑
     *
     * B4: control io level
     */

    dv_diag_io_sta.bit.B0 = tbox_pm_io_acc_is_active();
    dv_diag_io_sta.bit.B1 = tbox_pm_io_doh_is_active();
    dv_diag_io_sta.bit.B2 = tbox_pm_io_dol_is_active();
    dv_diag_io_sta.bit.B3 = tbox_pm_io_is_removed();

    dv_diag_io_sta.bit.B4 ^= 1;

    if (1 == dv_diag_io_sta.bit.B0)
    {
        if (0 != dv_diag_io_sta.bit.B1)
        {
            snprintf(des, deslen, "+ioSta:[ERR][DOH:%d, ACC:%d]", dv_diag_io_sta.bit.B0, dv_diag_io_sta.bit.B1);
            return -1;
        }
    }
    else
    {
        if (1 != dv_diag_io_sta.bit.B1)
        {
            snprintf(des, deslen, "+ioSta:[ERR][DOH:%d, ACC:%d]", dv_diag_io_sta.bit.B0, dv_diag_io_sta.bit.B1);
            return -1;
        }
    }

    if (1 == dv_diag_io_sta.bit.B2)
    {
        if (1 != dv_diag_io_sta.bit.B3)
        {
            snprintf(des, deslen, "+ioSta:[ERR][DOL:%d, DIL:%d]", dv_diag_io_sta.bit.B2, dv_diag_io_sta.bit.B3);
            return -1;
        }
    }
    else
    {
        if (0 != dv_diag_io_sta.bit.B3)
        {
            snprintf(des, deslen, "+ioSta:[ERR][DOL:%d, DIL:%d]", dv_diag_io_sta.bit.B2, dv_diag_io_sta.bit.B3);
            return -1;
        }
    }

    snprintf(des, deslen, "+ioSta:[OK][0x%x]", dv_diag_io_sta.byte);
    return 0;
}

static INT8 __attribute__((unused)) dv_diag_imu_sta(CHAR *des, INT32 deslen)
{
    return -1;
#if 0
	uint8_t id = 0;

	if(0 != imu_read_id(&id))
	{
		return -1;
	}
	snprintf(des, deslen, "+IMU ID:0x%x", id);
    return 0;
#endif
}

static INT8 dv_diag_rtc_time(CHAR *des, INT32 deslen)
{
    INT32    ret = 0;
    DEV_TIME time;
    ret = time_if_rtc_get(&time);
    if (0 != ret)
    {
        return -1;
    }

    snprintf(des, deslen, "+rtcTime:%d-%d-%d %d:%d:%d", time.year, time.month, time.day, time.hour, time.min, time.sec);
    return 0;
}

static INT8 dv_diag_main_vol(CHAR *des, INT32 deslen)
{
    INT32 vol = 0;
    vol       = analog_pwr_vtg();

    snprintf(des, deslen, "+mainVol:%d", vol);
    return 0;
}

static INT8 dv_diag_bat_vol(CHAR *des, INT32 deslen)
{
    INT32 vol = 0;
    vol       = analog_bat_vtg();

    snprintf(des, deslen, "+batVol:%d", vol);
    return 0;
}

static INT8 dv_diag_bat_tmp(CHAR *des, INT32 deslen)
{
    INT32 tmp = 0;
    tmp       = analog_bat_tmp();

    snprintf(des, deslen, "+batTmp:%d", tmp);
    return 0;
}

static INT8 dv_diag_mcu_reset(CHAR *des, INT32 deslen)
{
    INT32       ret   = 0;
    INT32       reset = 0;
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(RESETCOUNT, cfg_id);

    ret = tbox_cfg_read(cfg_id, &reset);
    if (0 != ret)
    {
        return -1;
    }

    snprintf(des, deslen, "+resetCnt:%d", reset);
    return 0;
}

static DV_DIAG_INFO_T dv_diag_table[] = {
    {"4gIccid",  2,  "", DV_DIAG_OK, dv_diag_4g_iccid    },
    {"4gSig",    2,  "", DV_DIAG_OK, dv_diag_4g_signal   },
    {"4gNat",    2,  "", DV_DIAG_OK, dv_diag_4g_nat      },
    {"4gAnt",    2,  "", DV_DIAG_OK, dv_diag_4g_ant      },
    {"4gCom",    10, "", DV_DIAG_OK, dv_diag_4g_com      },
    {"4gTemp",   10, "", DV_DIAG_OK, dv_diag_4g_tmp      },
    {"GnsFix",   2,  "", DV_DIAG_OK, dv_diag_gns_fix     },
    {"GnsAnt",   2,  "", DV_DIAG_OK, dv_diag_gns_ant     },
    //	{"BleSta",	2,	"", DV_DIAG_OK, dv_diag_ble_sta		},
    //	{"BleCom",	2,	"", DV_DIAG_OK, dv_diag_ble_com		},
    {"AdcLit",   2,  "", DV_DIAG_OK, dv_diag_adc_light   },
    {"NorFlash", 2,  "", DV_DIAG_OK, dv_diag_norflash_wr },
    {"NanFlash", 2,  "", DV_DIAG_OK, dv_diag_nandflash_wr},
    //	{"SeSpi",	2,	"", DV_DIAG_OK, dv_diag_se_spi		},
    {"Rs485",    2,  "", DV_DIAG_OK, dv_diag_rs485       },
    {"Can1Sta",  2,  "", DV_DIAG_OK, dv_diag_can1_sta    },
    {"Can2Sta",  2,  "", DV_DIAG_OK, dv_diag_can2_sta    },
    {"Can1Com",  2,  "", DV_DIAG_OK, dv_diag_can1_com    },
    {"Can2Com",  2,  "", DV_DIAG_OK, dv_diag_can2_com    },
    {"IOChk",    2,  "", DV_DIAG_OK, dv_diag_io_state    },
    //	{"IMUChk",	2,	"", DV_DIAG_OK, dv_diag_imu_sta		},
    {"RtcTime",  2,  "", DV_DIAG_OK, dv_diag_rtc_time    },
    {"MainVol",  10, "", DV_DIAG_OK, dv_diag_main_vol    },
    {"BatVol",   10, "", DV_DIAG_OK, dv_diag_bat_vol     },
    {"BatTmp",   10, "", DV_DIAG_OK, dv_diag_bat_tmp     },
    {"McuRst",   10, "", DV_DIAG_OK, dv_diag_mcu_reset   },
};

VOID dv_diag_timeout(VOID)
{
    INT8          ret   = 0;
    UINT8         index = 0;
    static UINT16 count = 0;

    for (index = 0; index < ARRAY_SIZE(dv_diag_table); index++)
    {
        if (0 == dv_diag_table[index].intv)
        {
            continue;
        }

        if ((0 == (count % dv_diag_table[index].intv)) && (0 != count))
        {
            if (NULL != dv_diag_table[index].proc)
            {
                ret = dv_diag_table[index].proc(dv_diag_table[index].des, DV_DIAG_DES_STR_MAX_LEN);
                if (0 != ret)
                {
                    dv_diag_table[index].sta = DV_DIAG_NG;
                }
                else
                {
                    dv_diag_table[index].sta = DV_DIAG_OK;
                }
            }
        }
    }

    if ((0 != dv_diag_log_info.flag) && (0 != count))
    {
        if (0 == (count % dv_diag_log_info.intv))
        {
            dv_diag_status_dump();
        }
    }

    drv_pin_set_level(PIN_ENABLE_DOH, dv_diag_io_sta.bit.B4);
    drv_pin_set_level(PIN_ENABLE_DOL, dv_diag_io_sta.bit.B4);

    count++;
}

static VOID dv_diag_status_dump(VOID)
{
    UINT8       index        = 0;
    const CHAR *status_str[] = {"OK", "NG"};

    tbox_log_print("\r\n=== DV Diagnostic Status ===\r\n");
    tbox_log_print("Log Info: flag=%d, interval=%d s\r\n", dv_diag_log_info.flag, dv_diag_log_info.intv);

    // 打印表格标题
    tbox_log_print("+----------+--------+-------------+--------------------------------------------------------+\r\n");
    tbox_log_print("| %-8s | %-6s | %-11s | %-55s|\r\n", "Name", "Status", "DiagIntv(s)", "Description");
    tbox_log_print("+----------+--------+-------------+--------------------------------------------------------+\r\n");

    // 打印表格内容
    for (index = 0; index < ARRAY_SIZE(dv_diag_table); index++)
    {
        // 处理描述字段，如果为空则显示"-"
        const CHAR *description = dv_diag_table[index].des;
        if (description[0] == '\0')
        {
            description = "-";
        }

        // 截断过长的描述，确保表格对齐
        CHAR truncated_des[56] = {0};
        strncpy(truncated_des, description, 55);

        tbox_log_print("| %-8s | %-6s | %-11d | %-55s|\r\n", dv_diag_table[index].name, status_str[dv_diag_table[index].sta],
                       dv_diag_table[index].intv, truncated_des);
    }

    // 打印表格底部
    tbox_log_print("+----------+--------+-------------+--------------------------------------------------------+\r\n");
    tbox_log_print("\r\n");
}

static VOID dv_diag_write_log_info(VOID)
{
    INT32 ret             = 0;
    dv_diag_log_info.migc = DV_DIAG_LOG_INFO_MIGC;
    ret                   = tbox_cfg_setkv(TEST_DV_LOG_INFO_MANE, &dv_diag_log_info, sizeof(dv_diag_log_info));
    if (0 != ret)
    {
        tbox_log_print("dv diag log info write failed, ret: %d\r\n", ret);
    }

    return;
}

static VOID dv_diag_read_log_info(VOID)
{
    INT32 ret = 0;
    ret       = tbox_cfg_getkv(TEST_DV_LOG_INFO_MANE, &dv_diag_log_info, sizeof(dv_diag_log_info));
    if (0 != ret)
    {
        tbox_log_print("dv diag log info read failed, ret: %d\r\n", ret);
        dv_diag_write_log_info();
        return;
    }

    if (DV_DIAG_LOG_INFO_MIGC != dv_diag_log_info.migc)
    {
        dv_diag_write_log_info();
    }

    return;
}

static VOID dv_diag_485_cb(VOID)
{
    INT32 ret     = 0;
    CHAR *newline = NULL;

    memset(dv_diag_485_info.data, 0, DV_DIAG_DES_STR_MAX_LEN);
    ret = drv_uart_485_rx(dv_diag_485_info.data, DV_DIAG_DES_STR_MAX_LEN);
    if (ret < 0)
    {
        dv_diag_485_info.flag = FALSE;
        return;
    }

    dv_diag_485_info.flag = TRUE;
    dv_diag_485_info.len  = ret;

    newline = strstr((CHAR *)dv_diag_485_info.data, "\r\n");
    if (newline != NULL)
    {
        *newline             = '\0';
        dv_diag_485_info.len = strlen((CHAR *)dv_diag_485_info.data);
    }
}

static VOID dv_diag_shell_tips(VOID)
{
    tbox_log_print("DV Shell Commands:\r\n");
    tbox_log_print("  -h             		- Show this help\r\n");
    tbox_log_print("  once             		- Dump diag info once\r\n");
    tbox_log_print("  perd [Flag] [Cycle]    - Dump diag info perd, [0,1] [0, 255]\r\n");
    tbox_log_print("Usage: dvcmd <command> [parameters]\r\n");
    return;
}

static BaseType_t dv_shell_diag(char *buf, size_t bufsz, const char *cmd)
{
    const char *param1_ptr, *param2_ptr, *param3_ptr;
    BaseType_t  param1_len, param2_len, param3_len;
    INT32       tmp = 0;

    param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
    if (NULL == param1_ptr)
    {
        goto DV_SHELL_ERR;
    }

    if (0 == strncmp(param1_ptr, "-h", param1_len))
    {
        dv_diag_shell_tips();
        return pdFALSE;
    }
    else if (0 == strncmp(param1_ptr, "once", param1_len))
    {
        dv_diag_status_dump();
        return pdFALSE;
    }
    else if (0 == strncmp(param1_ptr, "perd", param1_len))
    {
        param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
        if (NULL == param2_ptr)
        {
            goto DV_SHELL_ERR;
        }

        if (1 != param2_len)
        {
            goto DV_SHELL_ERR;
        }

        tmp = atoi(param2_ptr);
        if (0 != tmp && 1 != tmp)
        {
            goto DV_SHELL_ERR;
        }

        dv_diag_log_info.flag = tmp;

        param3_ptr = FreeRTOS_CLIGetParameter(cmd, 3, &param3_len);
        if (NULL == param3_ptr)
        {
            goto DV_SHELL_ERR;
        }

        tmp                   = atoi(param3_ptr);
        dv_diag_log_info.intv = tmp;
        tbox_log_print("dv dump perd info, flag: %d, perd: %d \r\n", dv_diag_log_info.flag, dv_diag_log_info.intv);
        dv_diag_write_log_info();
    }
    else
    {
        dv_diag_shell_tips();
    }

    return pdFALSE;
DV_SHELL_ERR:
    dv_diag_shell_tips();
    return pdFALSE;
}

void dv_test_task(void *param)
{
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (!tbox_pm_io_acc_is_active())
        {
            continue;
        }

        dv_diag_timeout();
    }
}

static void dv_timer_callback(TimerHandle_t xTimer)
{
    xTaskNotifyGive(xTaskHandle);
}

static const CLI_Command_Definition_t xDvCmd = {"dvcmd", "dv cmd diag api", dv_shell_diag, -1};

void test_dv_init(void *param)
{

    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate((TaskFunction_t)dv_test_task,  /* 任务函数 */
                          (const char *)"dv_test",       /* 任务名称 */
                          (configSTACK_DEPTH_TYPE)256,   /* 任务堆栈大小 */
                          (void *)NULL,                  /* 传递给任务函数的参数 */
                          (UBaseType_t)10,               /* 任务优先级 */
                          (TaskHandle_t *)&xTaskHandle); /* 任务句柄 */

    configASSERT(pdPASS == xReturn);

    xTimerHandle = xTimerCreate("dv_timer",          /* 定时器名称 */
                                pdMS_TO_TICKS(1000), /* 周期（ms） */
                                pdTRUE,              /* 自动重载（周期定时器） */
                                (void *)0,           /* 定时器ID（可自定义） */
                                dv_timer_callback);  /* 回调函数 */

    configASSERT(NULL != xTimerHandle);

    vTaskDelay(pdMS_TO_TICKS(5000)); /*确保任务已启动后再启动定时器*/
    xReturn = xTimerStart(xTimerHandle, 0);

    configASSERT(pdPASS == xReturn);

    dv_diag_io_sta.byte = 0;
    memset(&dv_diag_log_info, 0, sizeof(dv_diag_log_info));
    dv_diag_read_log_info();

    memset(&dv_diag_485_info, 0, sizeof(dv_diag_485_info));
    drv_uart_485_register(dv_diag_485_cb);
    FreeRTOS_CLIRegisterCommand(&xDvCmd);
}
