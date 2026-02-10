/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_Mailbox_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Mailbox_Reg_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Dspt_lp.h"
#include "eHSM_Exclusive_Area.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "test_int_config.h"
#include "AC784xx_Hsm_Reg.h"

#include "Device_Register.h"
#include "Core_Hal.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define HSMMBX_IRQ_PRIO                         PRIORITY_4

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/
static ehsm_uint32_t mbox_handle_resp(ehsm_uint8_t *data_ptr, ehsm_uint32_t size, mailbox_channel_e channel_type);
/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static ehsm_uint8_t recv_data[128];

/* Parameters of mailbox channel */
mailbox_channel_st mbox_channel[] =
{
    {
        .type = MAILBOX_CHANNE_GENERAL_SERVICE,
        .surpport_asyn = TRUE,
        .s2h_start_addr = S2H_SRV_GENERAL_WORD_INDEX,
        .s2h_word_size = S2H_SRV_GENERAL_WORD_SIZE,
        .s2h_note_bit = S2H_SRV_GENERAL_NOTE_BIT,
        .h2s_start_addr = H2S_SRV_GENERAL_WORD_INDEX,
        .h2s_word_size = H2S_SRV_GENERAL_WORD_SIZE,
        .h2s_note_bit = H2S_SRV_GENERAL_NOTE_BIT,
        .cmd_inprogres = NULL
    },
    {
        .type = MAILBOX_CHANNE_CMD_CANCLE,
        .surpport_asyn = FALSE,
        .s2h_start_addr = S2H_SRV_CMD_CANCLE_WORD_INDEX,
        .s2h_word_size = S2H_SRV_CMD_CANCLE_WORD_SIZE,
        .s2h_note_bit = S2H_SRV_CMD_CANCLE_NOTE_BIT,
        .h2s_start_addr = H2S_SRV_CMD_CANCLE_WORD_INDEX,
        .h2s_word_size = H2S_SRV_CMD_CANCLE_WORD_SIZE,
        .h2s_note_bit = H2S_SRV_CMD_CANCLE_NOTE_BIT,
        .cmd_inprogres = NULL
    },
    {
        .type = MAILBOX_CHANNE_JTAG,
        .surpport_asyn = FALSE,
        .s2h_start_addr = S2H_SRV_CMD_JTAG_WORD_INDEX,
        .s2h_word_size = S2H_SRV_JTAG_WORD_SIZE,
        .s2h_note_bit = S2H_SRV_CMD_JTAG_NOTE_BIT,
        .h2s_start_addr = H2S_SRV_JTAG_WORD_INDEX,
        .h2s_word_size = H2S_SRV_JTAG_WORD_SIZE,
        .h2s_note_bit = H2S_SRV_CMD_JTAG_NOTE_BIT,
        .cmd_inprogres = NULL
    },
    {
        .type = MAILBOX_CHANNE_MGR_SERVICE,
        .surpport_asyn = FALSE,
        .s2h_start_addr = S2H_SRV_MGR_WORD_INDEX,
        .s2h_word_size = S2H_SRV_MGR_WORD_SIZE,
        .s2h_note_bit = S2H_SRV_MGR_NOTE_BIT,
        .h2s_start_addr = H2S_SRV_MGR_WORD_INDEX,
        .h2s_word_size = H2S_SRV_MGR_WORD_SIZE,
        .h2s_note_bit = H2S_SRV_MGR_NOTE_BIT,
        .cmd_inprogres = NULL
    },
};

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t __attribute__((weak))ehsm_tick_form_ms(ehsm_uint32_t ms)
{
    return 0xFFFFFFFF;
}

ehsm_uint32_t __attribute__((weak))ehsm_get_tick(void)
{
    return 0;
}

static void uint32_copy(ehsm_uint32_t *dst, ehsm_uint32_t *src, ehsm_uint32_t wordLen)
{
    ehsm_uint32_t i;

    if(dst != src)
    {
        for(i=0; i<wordLen; i++)
        {
            dst[i] = src[i];
        }
    }
    else
    {;}
}

static ehsm_bool_t mbox_waite_timeout(ehsm_uint32_t timout_tick)
{
    ehsm_bool_t ret = FALSE;

    ehsm_uint32_t tick_now = ehsm_get_tick();
    if (tick_now > timout_tick)
    {
        ret = TRUE;
    }
    else
    {;}
    return ret;
}

static void mbox_read_data(void)
{
    ehsm_uint32_t *data_ptr = (ehsm_uint32_t *)(&rSOCMBOX_RSP_D0);
    ehsm_uint32_t i = 0;

    for (i = MAILBOX_CHANNE_GENERAL_SERVICE; i < MAILBOX_CHANNE_MAX; i ++)
    {
        /* Do not handle data of jtag channel */
        if (i == MAILBOX_CHANNE_JTAG)
        {
            continue;
        }
        else
        {;}
        if (MB_H2S_NOTE & mbox_channel[i].h2s_note_bit)
        {
            uint32_copy((ehsm_uint32_t *)recv_data, &data_ptr[mbox_channel[i].h2s_start_addr], mbox_channel[i].h2s_word_size);
            mbox_handle_resp(recv_data, mbox_channel[i].h2s_word_size, mbox_channel[i].type);
            *((volatile ehsm_uint32_t *)(&MB_H2S_NOTE)) = mbox_channel[i].h2s_note_bit;
        }
        else
        {;}
    }
    MB_H2S_SOC_INT = 0xFFFFFFFF;
}

static void mark_cmd_done(ehsm_cmd_req_st *cmd, ehsm_uint32_t error_code)
{
    if (error_code == EHSM_ERR_MAILBOX_SUCCESS)
    {
        cmd->error_code = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        cmd->error_code = error_code;
    }
    cmd->cmd_state = EHSM_CMD_REQ_STATE_DONE;
}

static ehsm_uint32_t mbox_handle_resp(ehsm_uint8_t *data_ptr, ehsm_uint32_t size, mailbox_channel_e channel_type)
{
    ehsm_cmd_req_st *cmd = NULL;
    ehsm_uint8_t cmd_ptr[CMD_TAG_BYTE_SIZE];
    ehsm_uint32_t error_code = 0;
    ehsm_mbox_cancel_channel_rps_st *cancel_rps;
    mailbox_channel_st *channel = NULL;

    if (channel_type <= MAILBOX_CHANNE_MAX)
    {
        channel = &mbox_channel[channel_type];
    }
    else
    {;}

    if (channel_type == MAILBOX_CHANNE_GENERAL_SERVICE)
    {
        (void)System_Memcpy(cmd_ptr, &data_ptr[RESPONSE_TAG_INDEX], CMD_TAG_BYTE_SIZE);
        error_code = *(ehsm_uint32_t *)&data_ptr[0];

        (void)System_Memcpy((ehsm_uint8_t *)&cmd, cmd_ptr, sizeof(ehsm_cmd_req_st *));
        (void)System_Memcpy(cmd->rps_data, &data_ptr[4], MAX_RESPONSE_DATA_SIZE);
        mark_cmd_done(cmd, error_code);
    }
    else if (channel_type == MAILBOX_CHANNE_CMD_CANCLE)
    {
        cancel_rps = (ehsm_mbox_cancel_channel_rps_st *)data_ptr;
        error_code = *(ehsm_uint32_t *)&cancel_rps->ret_code[0];
        (void)System_Memcpy(cmd_ptr, cancel_rps->cmd_tag, CMD_TAG_BYTE_SIZE);
        (void)System_Memcpy((ehsm_uint8_t *)&cmd, cmd_ptr, sizeof(ehsm_cmd_req_st *));
        mark_cmd_done(cmd, error_code);
        if (channel->cmd_inprogres != NULL)
        {
            mark_cmd_done(channel->cmd_inprogres, error_code);
        }
        else
        {;}
    }
    else if (channel_type == MAILBOX_CHANNE_MGR_SERVICE)
    {
        error_code = *(ehsm_uint32_t *)&data_ptr[0];
        if (channel->cmd_inprogres != NULL)
        {
            mark_cmd_done(channel->cmd_inprogres, error_code);
        }
        else
        {;}
    }
    else
    {;}

    return 0;
}

static ehsm_uint32_t waite_cmd_done(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t timout_tick = ehsm_tick_form_ms(cmd->timeout);
    ehsm_uint32_t tick_now= 0;

    while (cmd->cmd_state != EHSM_CMD_REQ_STATE_DONE)
    {
#ifdef CONFIG_EHSM_UNIT_TEST_SHE_BUSY
        (void)osal_thread_yield();
#endif
        /* ATC ADD - 2025.0113 for sync hsm process status */
        mbox_read_data();

        tick_now = ehsm_get_tick();
        if (tick_now > timout_tick)
        {
            cmd->error_code = EHSM_ERR_TRNG_TIMEOUT_ERROR;
            break;
        }

    }
    return cmd->error_code;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void MAILBOX_Handler(void);
void MAILBOX_Handler(void)
{
    MB_H2S_SOC_INT_EN = 0;

    if(MB_H2S_SOC_INT)
    {
        mbox_read_data();
    }
    else if(MB_S2H_SOC_INT)
    {
        MB_S2H_SOC_INT = 0xFFFFFFFF;
    }
    MB_H2S_SOC_INT_EN = 0xFFFFFFFF;
}

ehsm_int32_t ehsm_mbox_init(void)
{
    ehsm_int32_t ret = EHSM_ERR_SW_SUCCESS;

    MB_H2S_SOC_INT_EN = 0;

#ifndef CONFIG_EHSM_UNIT_TEST_MAILBOX_POLLING
    MB_S2H_SOC_INT_EN = 0xFFFFFFFF;
#else
    MB_S2H_SOC_INT_EN = 0;
#endif

    return ret;
}

void ehsm_mbox_polling()
{
    mbox_read_data();
}

ehsm_int32_t ehsm_mbox_send_cmd(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t i = 0;
    ehsm_uint32_t timout_tick = ehsm_tick_form_ms(CONFIG_EHSM_ARCH_V_MAILBOX_TIMEOUT);
    ehsm_int32_t ret = EHSM_ERR_SW_SUCCESS;
    ehsm_uint32_t data_word_size;
    ehsm_uint32_t *cmd_data;
    ehsm_uint8_t cmd_tag[HOST_ADDRESS_SIZE];

    /* ATC ADD - 2025.0606 for switch access hsm path */
    /*close jtag path, open hsm mailbox path*/
    HOST2HSM_ACCESS_PATH |= (1U << 1U);

    cmd_data = (ehsm_uint32_t *)cmd->cmd_data;
    data_word_size = cmd->cmd_size >> 2;
    if ((cmd != NULL) && (cmd->channel < MAILBOX_CHANNE_MAX))
    {
        mailbox_channel_st *channel = &mbox_channel[cmd->channel];
        /*When mailbox channel is not surpport asynchronous cmd, and eHSM just processing cmd in the channel, return EHSM_ERR_EHSM_BUSY*/
        Exclusive_area_enter();
        if ((channel->surpport_asyn == FALSE) && (channel->cmd_inprogres != NULL))
        {
            ret = EHSM_ERR_EHSM_BUSY;
            Exclusive_area_exit();
        }
        else
        {
            channel->cmd_inprogres = cmd;
            Exclusive_area_exit();
            if (channel->type == MAILBOX_CHANNE_GENERAL_SERVICE)
            {
                ehsm_set_address_pointer(cmd_tag, (ehsm_uint8_t *)cmd);
            }
            else
            {;}

            while(MB_S2H_NOTE & channel->s2h_note_bit)
            {
                if (TRUE == mbox_waite_timeout(timout_tick))
                {
                    ret = EHSM_ERR_TRNG_TIMEOUT_ERROR;
                    break;
                }
                else
                {;}
            }

            if (ret != EHSM_ERR_TRNG_TIMEOUT_ERROR)
            {
                cmd->cmd_state = EHSM_CMD_REQ_STATE_PROCESSING;
                ehsm_uint32_t *ptr = (ehsm_uint32_t *)(&(rSOCMBOX_CMD_D0)) + channel->s2h_start_addr;
                for (i = 0; i < data_word_size; i ++)
                {
                    ptr[i] = cmd_data[i];
                }

                if (channel->type == MAILBOX_CHANNE_GENERAL_SERVICE)
                {
                    ehsm_uint32_t *tag_ptr = &ptr[CMD_TAG_WORD_INDEX];
                    uint32_copy(tag_ptr, (ehsm_uint32_t *)cmd_tag, CMD_TAG_WORD_SIZE);
                }
                else
                {;}

                MB_S2H_NOTE |= channel->s2h_note_bit;

                if (cmd->req_type == EHSM_CMD_REQ_TYPE_SYNC)
                {
                    ret = waite_cmd_done(cmd);
                }
                else
                {;}
            }
            else
            {;}

            channel->cmd_inprogres = NULL;
        }
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    /*close hsm mailbox path, open jtag path*/
    HOST2HSM_ACCESS_PATH &= ~(1U << 1U);

    return ret;
}

ehsm_uint32_t ehsm_is_cmd_addr_null(const ehsm_uint8_t cmd_addr[HOST_ADDRESS_SIZE])
{
    ehsm_uint32_t is_null;

    if (*(ehsm_uint32_t *)cmd_addr == 0)
    {
        is_null = 1;
    }
    else
    {
        is_null = 0;
    }

    return is_null;
}
