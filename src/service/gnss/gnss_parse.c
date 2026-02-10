#include "tbox_common.h"
#include "tbox_module.h"
#include "time_if.h"
#include "tbox_log.h"
#include "gnss_if.h"

#include "gnss_nmeap.h"

#define SPEED_KN_TO_KM(speed) (1.852 * (speed))
#define IS_DATA_VALID(ch)     (ch == 'A')
#define GNSS_PARSE_ENABLE_GGA (1)
#define GNSS_PARSE_ENABLE_RMC (1)
#define GNSS_PARSE_ENABLE_GSA (0)
#define GNSS_PARSE_ENABLE_GSV (0)
#define GNSS_PARSE_BUFF_SIZE  (256)


typedef struct gnss_info
{
    UINT32 time;
    UINT32 date;
    UINT8  fix_sta;
    UINT8  satellites;
    UINT8  is_east;
    UINT8  is_north;
    DOUBLE longitude;
    DOUBLE latitude;
    DOUBLE direction;
    DOUBLE speed;
    DOUBLE altitude;
	DOUBLE hdop;
} gnss_info_t;

static nmeap_context_t gnss_parse_nmea_context;
static gnss_info_t     gnss_parse_info;
static BOOL            gnss_parse_nmea_log;
static UINT8           gnss_parse_buff[GNSS_PARSE_BUFF_SIZE];
static DEV_TIME        gnss_parse_time;

static INT32 gnss_parse_get_timezone(VOID)
{
	int32_t time_zone = 8;
	
	#if 0
	if(0 == dev_cfg_get(CFG_ITEM_TIME_ZONE,(uint8_t *)&time_zone))
	{
		time_zone = 8;
	}
	#endif

    return time_zone;
}

static VOID double_to_string(DOUBLE v, INT8 *s, UINT32 size)
{
    INT8     b[100] = {0};
    INT8     c;
    UINT64 	 t;
    UINT64   i, j;

    if (v < 0 && --size > 0)
        *s++ = '-';
    t = (UINT64)(fabs(v) * 1E6 + 0.5);

    for (i = 0; t; i++)
    {
        if (6 == i)
            b[i++] = '.';

        b[i] = t % 10 + '0';
        t    = t / 10;
    }

    for (j = 0; j < i / 2; j++)
    {
        c            = b[j];
        b[j]         = b[i - j - 1];
        b[i - j - 1] = c;
    }

    if (0 == i)
        b[i++] = '0';
    b[i] = '\0';

    strncpy((char *)s, (char *)b, size);
}


static VOID gnss_parse_time_format(UINT32 time_ddmmss, UINT32 date_ddmmyy, DEV_TIME *time)
{

    time->year  = date_ddmmyy % 100;
    date_ddmmyy = date_ddmmyy / 100;

    time->month = date_ddmmyy % 100;
    date_ddmmyy = date_ddmmyy / 100;

    time->day = date_ddmmyy % 100;

    time->sec   = time_ddmmss % 100;
    time_ddmmss = time_ddmmss / 100;

    time->min   = time_ddmmss % 100;
    time_ddmmss = time_ddmmss / 100;

    time->hour = time_ddmmss % 100;
}


static VOID gnss_parse_utc_to_local(INT32 timezone, DEV_TIME *utc, DEV_TIME *local)
{
    struct tm  time_in;
    struct tm *time_out;
    time_t     time_sec;

    time_in.tm_year  = utc->year + 100;
    time_in.tm_mon   = utc->month - 1;
    time_in.tm_mday  = utc->day;
    time_in.tm_hour  = utc->hour;
    time_in.tm_min   = utc->min;
    time_in.tm_sec   = utc->sec;
    time_in.tm_isdst = 0;

    time_sec = mktime(&time_in);
    time_sec += timezone * 3600;
    time_out = localtime(&time_sec);
    if (NULL == time_out)
    {
        return;
    }

    local->year  = time_out->tm_year - 100;
    local->month = time_out->tm_mon + 1;
    local->day   = time_out->tm_mday;
    local->hour  = time_out->tm_hour;
    local->min   = time_out->tm_min;
    local->sec   = time_out->tm_sec;
    // local->week = time_out->tm_wday;
}

static VOID gnss_parse_auto_sysc_time(DEV_TIME *local_time)
{
    BOOL need_sync = false;

    // clang-format off
    need_sync = (gnss_parse_time.year != local_time->year) || \
                (gnss_parse_time.month != local_time->month)   || \
                (gnss_parse_time.day != local_time->day);
    // clang-format on

    if (need_sync)
    {
        //time_if_set_with_source(*local_time, TIME_SYNC_SOURCE_GNSS);
        memcpy(&gnss_parse_time, local_time, sizeof(DEV_TIME));
        MODULE_LOG_I(GNSS, "GNSS time sync to local time");
    }
}

static VOID gnss_parse_update_rmc(nmeap_rmc_t *nmea, gnss_info_t *gif)
{
    gif->time      = nmea->time;
    gif->date      = nmea->date;
    gif->fix_sta   = GNSS_POS_STATE_FIX;
    gif->is_east   = nmea->is_east;
    gif->is_north  = nmea->is_north;
    gif->longitude = nmea->longitude;
    gif->latitude  = nmea->latitude;
    gif->direction = nmea->course;
    gif->speed     = SPEED_KN_TO_KM(nmea->speed);
}

static VOID gnss_parse_update_gga(nmeap_gga_t *nmea, gnss_info_t *gif)
{
    gif->satellites = nmea->satellites;
    gif->altitude   = nmea->altitude;
    gif->hdop   	= nmea->hdop;
}


#if GNSS_PARSE_ENABLE_RMC
static VOID gnss_parse_callout_gprmc(nmeap_context_t *context, VOID *data, VOID *user_data)
{
    nmeap_rmc_t *rmc = (nmeap_rmc_t *)data;
    DEV_TIME     utc_time, local_time;
    INT32      timezone;

    (void)context;
    (void)user_data;

    if (IS_DATA_VALID(rmc->warn))
    {
        timezone = gnss_parse_get_timezone();
        gnss_parse_time_format(rmc->time, rmc->date, &utc_time);
        gnss_parse_utc_to_local(timezone, &utc_time, &local_time);
#if 0	//时间模块完成后放开
        if (time_if_check_is_valid(local_time))
        {
            gnss_parse_auto_sysc_time(&local_time);
            gnss_parse_update_rmc(rmc, &gnss_parse_info);
        }
        else
        {
            tbox_log_print("\r\nrmc time is invalid!");
        }
#else
		gnss_parse_auto_sysc_time(&local_time);
		gnss_parse_update_rmc(rmc, &gnss_parse_info);
#endif
    }
    else
    {
        gnss_parse_info.fix_sta = GNSS_POS_STATE_UNFIX;
    }
    //gnss_wdg_set_error_flag(GNSS_FLAG_NORMAL);
}
#endif

#if GNSS_PARSE_ENABLE_GGA
static VOID gnss_parse_callout_gpgga(nmeap_context_t *context, VOID *data, VOID *user_data)
{
    nmeap_gga_t *gga = (nmeap_gga_t *)data;

    (VOID)context;
    (VOID)user_data;
    gnss_parse_update_gga(gga, &gnss_parse_info);
    //gnss_wdg_set_error_flag(GNSS_FLAG_NORMAL);
}
#endif

#if GNSS_PARSE_ENABLE_GSV
static VOID gnss_parse_callout_gpgsv(nmeap_context_t *context, VOID *data, VOID *user_data)
{
    tbox_log_print("gsv callout");
}
#endif

#if GNSS_PARSE_ENABLE_GSA
static VOID gnss_parse_callout_gpgsa(nmeap_context_t *context, VOID *data, VOID *user_data)
{
    tbox_log_print("gsa callout");
}
#endif

static VOID gnss_parse_init_nmea(VOID)
{

    nmeap_init(&gnss_parse_nmea_context, NULL);

#if GNSS_PARSE_ENABLE_GGA
    static nmeap_gga_t gnss_gga;

    nmeap_addParser(&gnss_parse_nmea_context, "GGA", nmeap_gpgga, gnss_parse_callout_gpgga, &gnss_gga);

#endif

#if GNSS_PARSE_ENABLE_RMC
    static nmeap_rmc_t gnss_rmc;

    nmeap_addParser(&gnss_parse_nmea_context, "RMC", nmeap_gprmc, gnss_parse_callout_gprmc, &gnss_rmc);

#endif

#if GNSS_PARSE_ENABLE_GSV
    static nmeap_gsv_t gnss_gsv;

    nmeap_addParser(&gnss_parse_nmea_context, "GSV", nmeap_gpgsv, gnss_parse_callout_gpgsv, &gnss_gsv);
#endif

#if GNSS_PARSE_ENABLE_GSA
    static nmeap_gsa_t gnss_gsa;
    nmeap_addParser(&gnss_parse_nmea_context, "GSA", nmeap_gpgsa, gnss_parse_callout_gpgsa, &gnss_gsa);
#endif
}


INT32 gnss_parse_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			gnss_parse_nmea_log = FALSE;
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			gnss_parse_init_nmea();
            break;
            
        default:
            break;
    }
	return 0;
}

INT32 gnss_parse_data(UINT8 *data, UINT16 len)
{
    int32_t id     = 0;
    int32_t remain = len;

    id = nmeap_parseBuffer(&gnss_parse_nmea_context, (const char *)data, (int *)&remain);

    if (id < 0)
    {
        remain = 0;
    }

    return remain;
}

VOID gnss_parse_periodic(VOID)
{
    static INT32 rem = 0;
    UINT32       len = GNSS_PARSE_BUFF_SIZE - rem - 1;
	INT32		 ret = 0;

	ret = drv_eio_gnss_rx(&gnss_parse_buff[rem], len);
	if(ret < 0)
	{
		return;
	}

    if (gnss_parse_nmea_log)
    {
        gnss_parse_buff[rem + len] = '\0';
        tbox_log_print("%s", &gnss_parse_buff[rem]);
    }

    len = len + rem;
    rem = gnss_parse_data(gnss_parse_buff, len);
    if (rem > 0 && rem < GNSS_PARSE_BUFF_SIZE)
    {
        memmove(&gnss_parse_buff[0], &gnss_parse_buff[len - rem], rem);
    }
}

INT32 gnss_get_position(GNSS_POSITION_DATA *pos)
{
    if (GNSS_POS_STATE_FIX == gnss_parse_info.fix_sta)
    {
        pos->longitude = fabs(gnss_parse_info.longitude);
        pos->latitude  = fabs(gnss_parse_info.latitude);
        pos->is_east   = gnss_parse_info.is_east;
        pos->is_north  = gnss_parse_info.is_north;
        return 0;
    }
    return -1;
}

INT32 gnss_get_time(DEV_TIME *time)
{
    if (GNSS_POS_STATE_FIX == gnss_parse_info.fix_sta)
    {
        memcpy((void *)time, (void *)&gnss_parse_time, sizeof(DEV_TIME));
        return 0;
    }
    return -1;
}

GNSS_POS_STATE gnss_get_fix_state(VOID)
{
    if (GNSS_POS_STATE_FIX == gnss_parse_info.fix_sta)
        return GNSS_POS_STATE_FIX;

    return GNSS_POS_STATE_UNFIX;
}

INT32 gnss_get_satellites(VOID)
{
    return gnss_parse_info.satellites;
}

DOUBLE gnss_get_altitude(VOID)
{
    return gnss_parse_info.altitude;
}

DOUBLE gnss_get_hdop(VOID)
{
    return gnss_parse_info.hdop;
}

DOUBLE gnss_get_speed(VOID)
{
    return gnss_parse_info.speed;
}

DOUBLE gnss_get_direction(VOID)
{
    return gnss_parse_info.direction;
}

VOID gnss_parse_show_info(VOID)
{
    DEV_TIME time;
    INT8     fix[12];
    INT8     lon[20];
    INT8     lat[20];
    INT8     dir[20];
    INT8     spd[20];
    INT8     alt[20];
    INT8     hdp[20];

    memcpy(&time, &gnss_parse_time, sizeof(time));
    double_to_string(gnss_parse_info.longitude, lon, sizeof(lon));
    double_to_string(gnss_parse_info.latitude, lat, sizeof(lat));
    double_to_string(gnss_parse_info.direction, dir, sizeof(dir));
    double_to_string(gnss_parse_info.speed, spd, sizeof(spd));
    double_to_string(gnss_parse_info.altitude, alt, sizeof(alt));
    double_to_string(gnss_parse_info.hdop, hdp, sizeof(hdp));

    if (GNSS_POS_STATE_FIX == gnss_parse_info.fix_sta)
    {
        strncpy((char *)fix, "Located", sizeof(fix));
    }
	else
	{
		strncpy((char *)fix, "Not located", sizeof(fix));
	}
	
    tbox_log_print("\r\n-------------------------------------\r\n");
    tbox_log_print("Time      : %04u-%02u-%02u %02u:%02u:%02u\r\n", time.year + 2000, time.month, time.day, time.hour, time.min, time.sec);
    tbox_log_print("Gnss      : %s\r\n", fix);
    tbox_log_print("Satelites : %u\r\n", gnss_parse_info.satellites);
    tbox_log_print("Longitude : %10s    %c\r\n", lon, gnss_parse_info.is_east ? 'E' : 'W');
    tbox_log_print("Latitude  : %10s    %c\r\n", lat, gnss_parse_info.is_north ? 'N' : 'S');
    tbox_log_print("Direction : %10s    deg\r\n", dir);
    tbox_log_print("Speed     : %10s    km/h\r\n", spd);
    tbox_log_print("Altitude  : %10s    m\r\n", alt);
    tbox_log_print("Hdop      : %10s    m\r\n", hdp);
    tbox_log_print("-------------------------------------\r\n");
}

VOID gnss_parse_show_nmea(BOOL enable)
{
    gnss_parse_nmea_log = enable;
}

VOID gnss_parse_info_reset(VOID)
{
    gnss_parse_info.fix_sta = GNSS_POS_STATE_UNFIX;
    tbox_log_print("gps fault set fix staus unfixed.\r\n");
}


