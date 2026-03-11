#include <string.h>
#include "FreeRTOS.h"
#include "tbox_log.h"
#include "task.h"
#include "vse.h"
#include "vse_type.h"
#include "vse_cfg.h"


static char log_buf[1024] = {0};

#define log_print(fmt, ...) snprintf(log_buf, sizeof(log_buf), fmt, ##__VA_ARGS__);tbox_log_raw_output(log_buf,strlen(log_buf))

#define TEST_FUNC_PERFORMANCE(func, times)                                                                                                           \
    do                                                                                                                                               \
    {                                                                                                                                                \
        TickType_t start_ticks, end_ticks;                                                                                                           \
        uint32_t   total_ticks   = 0;                                                                                                                \
        uint32_t   success_count = 0;                                                                                                                \
        uint32_t   fail_count    = 0;                                                                                                                \
        uint32_t   min_ticks     = UINT32_MAX;                                                                                                       \
        uint32_t   max_ticks     = 0;                                                                                                                \
        uint32_t   current_ticks;                                                                                                                    \
        int        ret;                                                                                                                              \
        log_print("========================================\n");                                                                                     \
        log_print("Function Test: %s\n", #func);                                                                                                     \
        log_print("Iterations: %lu\n", (uint32_t)(times));                                                                                           \
        log_print("----------------------------------------\n");                                                                                     \
        for (uint32_t i = 0; i < (times); i++)                                                                                                       \
        {                                                                                                                                            \
            start_ticks = xTaskGetTickCount();                                                                                                       \
            ret         = (func)();                                                                                                                  \
            end_ticks   = xTaskGetTickCount();                                                                                                       \
            vTaskDelay(pdMS_TO_TICKS(1));                                                                                                            \
            /* 处理计数器溢出 */                                                                                                              \
            if (end_ticks >= start_ticks)                                                                                                            \
            {                                                                                                                                        \
                current_ticks = (end_ticks - start_ticks);                                                                                           \
            }                                                                                                                                        \
            else                                                                                                                                     \
            {                                                                                                                                        \
                current_ticks = (portMAX_DELAY - start_ticks + end_ticks + 1);                                                                       \
            }                                                                                                                                        \
            total_ticks += current_ticks;                                                                                                            \
            /* 更新最长时间和最短时间 */                                                                                                  \
            if (current_ticks < min_ticks)                                                                                                           \
            {                                                                                                                                        \
                min_ticks = current_ticks;                                                                                                           \
            }                                                                                                                                        \
            if (current_ticks > max_ticks)                                                                                                           \
            {                                                                                                                                        \
                max_ticks = current_ticks;                                                                                                           \
            }                                                                                                                                        \
            /* 统计成功失败次数 */                                                                                                           \
            if (ret == 0)                                                                                                                            \
            {                                                                                                                                        \
                success_count++;                                                                                                                     \
            }                                                                                                                                        \
            else                                                                                                                                     \
            {                                                                                                                                        \
                fail_count++;                                                                                                                        \
            }                                                                                                                                        \
        }                                                                                                                                            \
        log_print("----------------------------------------\n");                                                                                     \
        log_print("Performance Results:\n");                                                                                                         \
        log_print("  Total ticks: %lu\n", total_ticks);                                                                                              \
        log_print("  Average ticks per execution: %.2f\n", (double)total_ticks / (times));                                                            \
        log_print("  Min ticks: %lu (%.2f ms)\n", min_ticks, (double)min_ticks *portTICK_PERIOD_MS);                                                  \
        log_print("  Max ticks: %lu (%.2f ms)\n", max_ticks, (double)max_ticks *portTICK_PERIOD_MS);                                                  \
        log_print("  Total time: %.2f ms\n", (double)total_ticks *portTICK_PERIOD_MS);                                                                \
        log_print("  Average time per execution: %.2f ms\n", (double)total_ticks *portTICK_PERIOD_MS / (times));                                      \
        log_print("\nSuccess/Failure Statistics:\n");                                                                                                \
        log_print("  Success: %lu\n", success_count);                                                                                                \
        log_print("  Fail: %lu\n", fail_count);                                                                                                      \
        log_print("  Success rate: %.2f%%\n", (double)success_count * 100.0f / (times));                                                              \
        log_print("========================================\n\n");                                                                                   \
    } while (0)

#define FUNC_MSG_PRINT(...)
// #define FUNC_MSG_PRINT(...)  log_print(__VA_ARGS__)

#define MSG_MAX_SIZE (1024)

typedef struct
{
    uint8_t flag;
    uint8_t sm2za[SM2_PREP_ZA_SIZE];
} vse_data_t;

static vse_data_t    VseData;
static const uint8_t VseUid[SM2_USER_ID_SIZE]     = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
static uint8_t       sm2_msg[MSG_MAX_SIZE]        = {0};
static uint8_t       sm2_sign[SM2_SIGNATURE_SIZE] = {0};

static void vse_init_data(void)
{
    VseData.flag = FALSE;
    memset(VseData.sm2za, 0x00, SM2_PREP_ZA_SIZE);
}

int32_t test_vse_init(void)
{
    int32_t ret = -1;

    vse_init_data();

    vse_gpio_reset();
    vse_deinit();
    vse_init();
    ret = vse_connect();
    if (0 == ret)
    {
        FUNC_MSG_PRINT("vse init success.\r\n");
    }
    else
    {
        FUNC_MSG_PRINT("vse init failed,ret:%d.\r\n");
    }

    return ret;
}

int32_t test_vse_version(void)
{
    uint8_t  ver[VSE_VERSION_SIZE] = {0};
    uint16_t len;
    int32_t  ret = 0;

    len = VSE_VERSION_SIZE;
    if (0 == vse_get_ver(VERSION_TYPE_SDK, ver, &len))
    {
        FUNC_MSG_PRINT("SDK version: %02x%02x%02x%02x\r\n", ver[0], ver[1], ver[2], ver[3]);
    }
    else
    {
        ret++;
    }

    len = VSE_VERSION_SIZE;
    if (0 == vse_get_ver(VERSION_TYPE_COS, ver, &len))
    {
        FUNC_MSG_PRINT("COS version: %02x%02x%02x%02x\r\n", ver[0], ver[1], ver[2], ver[3]);
    }
    else
    {
        ret++;
    }

    return ret;
}

int32_t test_vse_sm2key(void)
{
    int32_t  ret                   = -1;
    uint8_t  pub[SM2_PUB_KEY_SIZE] = {0};
    uint16_t len                   = SM2_PUB_KEY_SIZE;

    if (0 == vse_get_sm2key(pub, &len))
    {
        FUNC_MSG_PRINT("get sm2 pubilc key success.\r\n");
        ret = 0;
    }
    else
    {
        FUNC_MSG_PRINT("get sm2 pubilc key failed.\r\n");
        ret = -1;
    }
    return ret;
}

int32_t test_vse_crypt(void)
{

    sm2_cipher_t cipher;
    uint16_t     msg_len = SM2_CIPHER_C2_SIZE;
    uint16_t     i;

    for (i = 0; i < msg_len; ++i)
    {
        sm2_msg[i] = LSB(i);
    }

    if (vse_do_sm2enc(sm2_msg, msg_len, &cipher) != VSE_SUCCESS)
    {
        FUNC_MSG_PRINT("VSE SM2 encrypt failed, msg_len is %u.", msg_len);
        return -1;
    }
    else
    {
        FUNC_MSG_PRINT("VSE SM2 encrypt success.");
    }

    if (vse_do_sm2dec(&cipher, sm2_msg, &msg_len) != VSE_SUCCESS)
    {
        FUNC_MSG_PRINT("VSE SM2 decrypt failed.");
        return -1;
    }
    else
    {
        FUNC_MSG_PRINT("VSE SM2 decrypt success.");
    }

    return 0;
}

int32_t test_vse_sign(void)
{
    int32_t         ret   = VSE_ERROR;
    static uint32_t count = 0;
    uint16_t        i;
    uint16_t        msg_len               = 288;
    uint16_t        sign_len              = SM2_SIGNATURE_SIZE;
    uint8_t         pub[SM2_PUB_KEY_SIZE] = {0};
    uint16_t        len                   = SM2_PUB_KEY_SIZE;

    if (!VseData.flag)
    {
        len = SM2_PREP_ZA_SIZE;
        ret = vse_do_sm2prep((uint8_t *)&VseUid, SM2_USER_ID_SIZE, pub, SM2_PUB_KEY_SIZE, VseData.sm2za, &len);
        if (ret == VSE_SUCCESS)
        {
            VseData.flag = TRUE;
        }
    }

    if (VseData.flag)
    {
        for (i = SM2_PREP_ZA_SIZE; i < msg_len; ++i)
        {
            sm2_msg[i] = LSB(i);
        }
        memcpy(sm2_msg, VseData.sm2za, SM2_PREP_ZA_SIZE);

        ret = vse_do_sm2sign(sm2_msg, msg_len, sm2_sign, &sign_len);
        if (ret == VSE_SUCCESS)
        {
            // VSE_BUFFER("SM2 sign data is:", sm2_sign, sign_len);
            count++;
        }
        else
        {
            FUNC_MSG_PRINT("VSE sign continue %d times.", count);
            count = 0;
            goto exit;
        }

        ret = vse_do_sm2verify(sm2_msg, msg_len, sm2_sign, sign_len);
        if (ret == VSE_SUCCESS)
        {
            FUNC_MSG_PRINT("VSE SM2 verify success.");
        }
        else
        {
            FUNC_MSG_PRINT("VSE SM2 verify failed.");
            goto exit;
        }
    }
    else
    {
        FUNC_MSG_PRINT("VSE didn't calculate SM2 ZA.");
    }

exit:
    return ret;
}

void vse_test(void)
{
    TEST_FUNC_PERFORMANCE(test_vse_init, 1);
    TEST_FUNC_PERFORMANCE(test_vse_version, 1000);
    TEST_FUNC_PERFORMANCE(test_vse_sm2key, 100);
}