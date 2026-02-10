
#include "Adc_Hal.h"
#include "Ctu_Hal.h"
#include "api_rtos.h"
#include "drv_adc.h"
#include "drv_pin.h"
#include "drv_timer.h"
#include "drv_log.h"
#include "macros.h"

#define ADC0_CHANNEL_COUNT 3U
#define ADC1_CHANNEL_COUNT 3U

#define ADC_SAMPLE_COUNT   4U
#define ADC_SAMPLE_TIME    20U//ms

typedef struct
{
    uint8_t name : 6;
    uint8_t ins  : 2;
    uint8_t chn  : 8;
} adc_channel_t;

typedef struct
{
    uint8_t  index;
    uint16_t values[ADC_SAMPLE_COUNT];
} adc_data_t;

static adc_channel_t adc_list[DRV_ADC_COUNT] = {
    {DRV_ADC_LIGHT,  0, 0 }, //  LIGHTSensor
    {DRV_ADC_BAT_TMP,  0, 1 }, //  NTC
    {DRV_ADC_GNSS_2,   0, 13}, //  gps2 - ADC0_IN13 (PIN45)
    {DRV_ADC_PWR_VTG,  1, 2 }, // power adc
    {DRV_ADC_BAT_VTG,  1, 3 }, // VEB
    {DRV_ADC_MAIN_ANT, 1, 4 }, //  main ant
};

static const Ctu_MappingCfgType ctu_mapping_cfg[] = {
    {.Module = TRIG_SEL_ADC0_REGULAR0, .Source = TRIG_SOURCE_PCT0_TRIG},
    {.Module = TRIG_SEL_ADC1_REGULAR0, .Source = TRIG_SOURCE_PCT0_TRIG},
};

static drv_timer_t adc_timer;
static adc_data_t  adc_data[DRV_ADC_COUNT];

static void        drv_adc_callback(const Adc_InterruptInfoType *info)
{
    uint16_t value;
    uint8_t  adc_index;
    uint8_t  sample_index;

    if (0U != (info->Event & (uint32_t)ADC_EVENT_EOC))
    {
        value        = Adc_Hal_GetSeqResult(info->Instance, info->sequence);
        adc_index    = info->Instance * ADC0_CHANNEL_COUNT + (uint8_t)info->sequence;
        sample_index = adc_data[adc_index].index++ % ADC_SAMPLE_COUNT;

        adc_data[adc_index].values[sample_index] = value;
        //DRV_LOG_I(DRVADC, "ADC[%u] Channel[%d]  Value:%d\n", info->Instance, info->sequence, value);
    }
}

static int32_t drv_adc_init_ins(uint8_t ins, uint8_t len)
{
    Adc_InitConfigType      cb_cfg;
    Adc_ConverterConfigType convert;
    Adc_GroupConfigType     group;

    Adc_Hal_InitConverterStruct(&convert);
    Adc_Hal_InitGroupStruct(&group);

    cb_cfg.Interrupt.Callback   = drv_adc_callback;
    group.ScanModeEn            = TRUE;
    group.RegularTrigger        = ADC_TRIGG_SRC_HW;
    group.RegularSequenceLength = len;
    group.InjectSequenceLength  = 0;

    Adc_Hal_Init(ins, &cb_cfg);
    Adc_Hal_ConfigGroup(ins, &group);
    Adc_Hal_ConfigConverter(ins, &convert);

    return 0;
}

static int32_t drv_adc_init_chn(uint8_t ins, uint8_t chn, uint8_t seq)
{
    Adc_ChanConfigType channel;

    Adc_Hal_InitChanStruct(&channel);
    channel.Channel     = (Adc_InputChannelType)chn;
    channel.Spt         = ADC_SPT_CLK_5;
    channel.InterruptEn = TRUE;
    channel.SeqIndex    = (Adc_SequenceType)seq;
    Adc_Hal_ConfigChannel(ins, &channel);

    return 0;
}

static int32_t drv_adc_init_ctu(void)
{
    Ctu_CfgType ctu_cfg;
    ctu_cfg.EnLock      = TRUE;
    ctu_cfg.CfgCnt      = (uint8_t)ARRAY_SIZE(ctu_mapping_cfg);
    ctu_cfg.MappingCfgs = ctu_mapping_cfg;
    Ctu_Hal_Init(&ctu_cfg);

    return 0;
}

static int32_t drv_adc_deinit_ctu(void)
{
    Ctu_Hal_DeInit();
    return 0;
}

int32_t drv_adc_init(void)
{
    uint8_t i, offset, sequence;

    configASSERT(DRV_ADC_COUNT == (ADC0_CHANNEL_COUNT + ADC1_CHANNEL_COUNT));

    drv_adc_init_ins(0U, ADC0_CHANNEL_COUNT);
    drv_adc_init_ins(1U, ADC1_CHANNEL_COUNT);

    offset   = 0U;
    sequence = 0U;
    for (i = 0U; i < ADC0_CHANNEL_COUNT; i++)
    {
        drv_adc_init_chn(adc_list[i + offset].ins, adc_list[i + offset].chn, sequence);
        ++sequence;
    }

    offset   = ADC0_CHANNEL_COUNT;
    sequence = 0U;
    for (i = 0U; i < ADC1_CHANNEL_COUNT; i++)
    {
        drv_adc_init_chn(adc_list[i + offset].ins, adc_list[i + offset].chn, sequence);
        ++sequence;
    }

    drv_adc_init_ctu();
    drv_timer_control(&adc_timer, DRV_TIMER_CMD_SET_CFG, DRV_TIMER_INS_PCT, ADC_SAMPLE_TIME, DRV_TIMER_MODE_STARTUP);
    drv_timer_init(&adc_timer);
    return 0;
}

int32_t drv_adc_deinit(void)
{
    uint32_t ret = 0;

    Adc_Hal_Deinit(0);
    Adc_Hal_Deinit(1);

    ret |= (uint32_t)drv_adc_deinit_ctu();
    ret |= (uint32_t)drv_timer_deinit(&adc_timer);
    return (int32_t)ret;
}

int32_t drv_adc_sleep(void)
{
    return drv_adc_deinit();
}

int32_t drv_adc_wake(void)
{
    return drv_adc_init();
}

int32_t drv_adc_get(uint8_t adc_id)
{
    uint8_t i, count;
    int32_t sum = 0;

    if (adc_id >= DRV_ADC_COUNT)
    {
        return -1;
    }

    count = 0U;
    for (i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
        if (adc_data[adc_id].values[i] != 0U)
        {
            sum += (int32_t)adc_data[adc_id].values[i];
            ++count;
        }
    }
    if (0U != count)
    {
        sum = sum / (int32_t)count;
    }
    return sum;
}
