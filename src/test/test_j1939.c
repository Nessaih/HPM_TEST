#include <stdint.h>
#include <string.h>
#include "tbox_log.h"
#include "j1939_if.h"


struct j1939_dm1_led
{
    uint8_t led_protect : 2;
    uint8_t led_warn    : 2;
    uint8_t led_stop    : 2;
    uint8_t led_fault   : 2;
};

static void test_j1939_dm1_cb(uint8_t *msg, uint16_t len, uint8_t sa, uint32_t pgn)
{
    // trace_dumphex(test_task_handle, "DM1:", msg, len);

    uint32_t cnt;
    uint32_t spn;
    uint8_t  fmi;
    uint8_t  oc;
    uint8_t  cm;

    UNUSED(pgn);
    struct j1939_dm1_led led;

    if (len < 2)
    {
        return;
    }
    memcpy(&led, msg, 1);
    cnt = (len - 2) / 4;
    msg += 2;

    LOG_PRINT("\nDM1 info\nLED fault:%u stop:%u warn:%u protect:%u\r\n", led.led_fault, led.led_stop, led.led_warn, led.led_protect);
    for (uint32_t i = 0; i < cnt; i++)
    {

        spn = msg[i * 4] + (msg[i * 4 + 1] << 8) + ((msg[i * 4 + 2] & 0xE0) << 3);
        fmi = msg[i * 4 + 2] & 0x1F;
        oc  = msg[i * 4 + 3] & 0x7F;
        cm  = (msg[i * 4 + 3] & 0x80) >> 7;

        LOG_PRINT(" %2u SPN:%8u FMI:%2u Transform:%1u OccurTimes:%3u\r\n", (unsigned int)i, (unsigned int)spn, (unsigned int)fmi, (unsigned int)cm,
                  (unsigned int)oc);
    }
}

static void test_j1939_vin_cb(uint8_t *msg, uint16_t len, uint8_t sa, uint32_t pgn)
{
    uint8_t vin_buf[18] = {0};

    UNUSED(pgn);

    strncpy((char *)vin_buf, (char *)msg, sizeof(vin_buf) - 1);
    LOG_PRINT("VIN:%s\r\n", vin_buf);
}

void test_j1939_init(void)
{
    j1939_subscribe(0x00, J1939_PGN_DM1, test_j1939_dm1_cb);
    j1939_subscribe(0x00, J1939_PGN_VIN, test_j1939_vin_cb);
    // dm_shell_reg("testj1939pgn", "j1939 req/add/show  pgn test", test_j1939_pgn_shell);
}

void test_j1939_loop(void)
{
}
