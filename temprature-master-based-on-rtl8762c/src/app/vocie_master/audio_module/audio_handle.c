/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      audio_handle.c
* @brief    Provider audio handle APIs
* @details
* @author  chenjie_jin
* @date     2018-05-15
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "audio_handle.h"
#include "board.h"
#include "audio_loop_buffer.h"
#include <audio_bi_buffer.h>
#include <audio_trans.h>
#include <rtl876x_gpio.h>
//#include <rcu_application.h>
#include <os_mem.h>
#include "os_timer.h"
//#include "swtimer.h"
//#include "rhqc.h"
#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)
#include "alc5616.h"
#elif (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5628)
#include "alc5628.h"
#endif
#include "app_task.h"
#include "platform_utils.h"
#include "app_section.h"
#include "audio_hd_detection.h"
#include "central_app.h"
#include "sbc.h"

//#include "key_handle.h"

#if FEATURE_SUPPORT_AUDIO_DOWN_STREAMING

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static uint8_t audio_current_volume = ALC5616_DAC_VOL_DEFAULT_VALUE;

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
T_AUDIO_GLOBAL_DATA audio_global_data;
T_ADS_GLOBAL_DATA ads_global_data;
TimerHandle_t codec_power_on_delay_timer;
bool is_audio_allowed_to_enter_dlps = true;

/*============================================================================*
 *                              Extern Functions
 *============================================================================*/

/*============================================================================*
 *                              Loacal Functions
 *============================================================================*/
#if AUDIO_CODEC_SUPPORT_CALC_TIMING
void audio_handle_decode_time_calc_init(void)
{
    Pad_Config(AUDIO_CODEC_CALC_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(AUDIO_CODEC_CALC_PIN, DWGPIO);
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = GPIO_GetPin(AUDIO_CODEC_CALC_PIN);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_ResetBits(GPIO_GetPin(AUDIO_CODEC_CALC_PIN));
}
#endif

/**
    * @brief    Function to handle write HIDS_ADS_CHAR_TX
    * @param    p_data      Pointer to data
    * @param    len         Length of data
    * @return   T_APP_RESULT, which indicates the function call is successful or not
    * @retval   APP_RESULT_SUCCESS  Function run successfully
    * @retval   others              Function run failed, and return number indicates the reason
    */
DATA_RAM_FUNCTION
T_APP_RESULT app_handle_hids_ads_write_char_tx(uint8_t *p_data, uint32_t len)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    APP_PRINT_INFO1("[app_handle_hids_ads_write_char_tx] write len %d", len);

    if (audio_global_data.is_working == true)
    {
        audio_global_data.total_recv_bytes += len;
        loop_buffer_in_queue(p_data, len);
        audio_handle_drop_useless_buff_data();

        if (audio_global_data.is_driver_initialized == false)
        {
            if (audio_global_data.audio_frame_index == 0)
            {
                if (audio_handle_loop_buffer_data(0) == true)
                {
#if (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_I2S)
                    AudioTrans_ConfigLLIStruct(0, audio_global_data.bytes_per_frame_after_docode *
                                               audio_global_data.decode_frame_cnt_for_once / 4, false);
#elif (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_UART)
                    AudioTrans_ConfigLLIStruct(0, audio_global_data.bytes_per_frame_after_docode *
                                               audio_global_data.decode_frame_cnt_for_once, false);
#endif
                }
            }

            if (audio_global_data.audio_frame_index == 1)
            {
                if (audio_handle_loop_buffer_data(1) == true)
                {
#if (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_I2S)
                    AudioTrans_ConfigLLIStruct(1, audio_global_data.bytes_per_frame_after_docode *
                                               audio_global_data.decode_frame_cnt_for_once / 4, false);
#elif (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_UART)
                    AudioTrans_ConfigLLIStruct(1, audio_global_data.bytes_per_frame_after_docode *
                                               audio_global_data.decode_frame_cnt_for_once, false);
#endif
                    audio_handle_start_gdma();
                }
            }

        }
    }

    return app_result;
}

/**
  * @brief  loop queue drop useless data.
  */
DATA_RAM_FUNCTION
void audio_handle_drop_useless_buff_data(void)
{
    uint32_t drop_bytes = 0;

    while (loop_buffer.in_index != loop_buffer.out_index)
    {
        /* check first byte */
        uint8_t byte_data = loop_buffer.buf[loop_buffer.out_index];
        if (byte_data == SBC_SYNCBYTE)
        {
            break;
        }
        else
        {
            loop_buffer.out_index = (loop_buffer.out_index + 1) % LOOP_BUFFER_MAX_SIZE;
            drop_bytes++;
            continue;
        }
    }

    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_drop_useless_buff_data] drop %d bytes", 1,
                     drop_bytes);
}

/**
  * @brief  audio handle enable power.
  */
void audio_handle_enable_power(void)
{
    is_audio_allowed_to_enter_dlps = false;
    /* enable codec LDO power */
    Pad_Config(AUDIO_CODEC_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    /* add 50ms delay to wait power stable */
    os_timer_start(&codec_power_on_delay_timer);
}

/**
  * @brief  audio handle enable power.
  */
void audio_handle_disable_power(void)
{
    os_timer_stop(&codec_power_on_delay_timer);

    /* disable codec LDO power */
    Pad_Config(AUDIO_CODEC_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(AUDIO_CODEC_SCL_Pin, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_CODEC_SDA_Pin, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_CODEC_I2S_BCLK_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_CODEC_I2S_LR_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_CODEC_I2S_DATA_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    is_audio_allowed_to_enter_dlps = true;
}

/*============================================================================*
*                              Global Functions
*============================================================================*/
void audio_handle_start_gdma(void)
{
    if (app_global_data.cur_audio_sample_freq != audio_global_data.sampling_freq)
    {
        AudioTrans_ConfigDataRate(audio_global_data.sampling_freq);
        app_global_data.cur_audio_sample_freq = audio_global_data.sampling_freq;
    }

    AudioTrans_InitTypeDef AudioTrans_InitStruct;
    AudioTrans_InitStruct.AudioTrans_Type = audio_global_data.audio_trans_type;
    AudioTrans_InitStruct.mem_addr = audio_bi_buffer_get_buf0_addr();
#if (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_I2S)
    AudioTrans_InitStruct.audio_size = audio_global_data.bytes_per_frame_after_docode / 4;
#elif (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_UART)
    AudioTrans_InitStruct.audio_size = audio_global_data.bytes_per_frame_after_docode;
#endif

    AudioTrans_Cmd(&AudioTrans_InitStruct, ENABLE);

    audio_global_data.is_driver_initialized = true;
}

/**
  * @brief  audio handle start.
  */
void audio_handle_start(void)
{
    if (audio_global_data.is_working == true)
    {
        return;
    }
    APP_PRINT_INFO0("[audio_handle_start] start audio");
    /* initial audio related source */
    loop_buffer_init();
    audio_bi_buffer_init();
    memset(&audio_global_data, 0, sizeof(audio_global_data));
    audio_global_data.is_working = true;
    audio_global_data.total_recv_bytes = 0;
#if (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_I2S)
    audio_global_data.audio_trans_type = AUDIO_TRANS_TYPE_MEM_TO_I2S;
#elif (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_UART)
    audio_global_data.audio_trans_type = AUDIO_TRANS_TYPE_MEM_TO_UART;
#endif

    audio_global_data.decode_frame_cnt_for_once = 10;
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_start] Init Decoder", 0);
    sbc_init_decoder();

    //rhqc_init_decoder();

#if AUDIO_CODEC_SUPPORT_CALC_TIMING
    audio_handle_decode_time_calc_init();
#endif

    audio_handle_enable_power();
}

/**
  * @brief  Deinitializes audio decoder.
  */
void audio_handle_stop(void)
{
    APP_PRINT_INFO0("[audio_handle_stop] stop audio");
    AudioTrans_ConfigLLIStruct(0, 0, true);
    /* Stop audio stream play */
    AudioTrans_InitTypeDef AudioTrans_InitStruct;
    AudioTrans_Cmd(&AudioTrans_InitStruct, DISABLE);

    loop_buffer_init();
    audio_bi_buffer_init();
    memset(&audio_global_data, 0, sizeof(audio_global_data));

    audio_handle_disable_power();
}

/**
  * @brief  Increase auido volume.
  */
bool audio_handle_volume_up(void)
{
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_volume_up] volume up", 0);
#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)
    if (audio_global_data.is_working)
    {
        audio_current_volume += ALC5616_DAC_VOL_STEP;
        if (audio_current_volume > ALC5616_DAC_VOL_MAX_VALUE)
        {
            audio_current_volume = ALC5616_DAC_VOL_MAX_VALUE;
        }

        ALC5616_DACVolumeConfig(Left_Headphone, audio_current_volume);
        ALC5616_DACVolumeConfig(Right_Headphone, audio_current_volume);

        return true;
    }
    else
    {
        return false;
    }
#endif
}

/**
  * @brief  Decrease auido volume.
  */
bool audio_handle_volume_down(void)
{
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_volume_down] volume down", 0);
#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)
    if (audio_global_data.is_working)
    {
        if (audio_current_volume >= ALC5616_DAC_VOL_MIN_VALUE + ALC5616_DAC_VOL_STEP)
        {
            audio_current_volume -= ALC5616_DAC_VOL_STEP;
        }

        ALC5616_DACVolumeConfig(Left_Headphone, audio_current_volume);
        ALC5616_DACVolumeConfig(Right_Headphone, audio_current_volume);

        return true;
    }
    else
    {
        return false;
    }
#endif
}

/**
  * @brief  Mute auido volume.
  */
bool audio_handle_volume_mute(void)
{
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_volume_mute] volume mute", 0);
#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)
    if (audio_global_data.is_working)
    {
        ALC5616_DACVolumeConfig(Left_Headphone, ALC5616_DAC_VOL_MIN_VALUE);
        ALC5616_DACVolumeConfig(Right_Headphone, ALC5616_DAC_VOL_MIN_VALUE);

        return true;
    }
    else
    {
        return false;
    }
#endif
}

/**
  * @brief  Unmute auido volume.
  */
bool audio_handle_volume_unmute(void)
{
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_volume_unmute] volume unmute", 0);
#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)
    if (audio_global_data.is_working)
    {
        ALC5616_DACVolumeConfig(Left_Headphone, audio_current_volume);
        ALC5616_DACVolumeConfig(Right_Headphone, audio_current_volume);

        return true;
    }
    else
    {
        return false;
    }
#endif
}


/**
  * @brief  Check aac frame is complete or not.
  */
DATA_RAM_FUNCTION
bool audio_handle_check_frame_complete(uint32_t *p_frame_len)
{
    bool result = false;
    uint32_t buffer_data_len = 0;

    *p_frame_len = 0;

    audio_handle_drop_useless_buff_data();
    buffer_data_len = loop_buffer_get_data_len();
    if (buffer_data_len >= 3)
    {
        /* check frame length */
        if ((audio_global_data.bytes_per_frame_before_docode == 0)
            || (audio_global_data.bytes_per_frame_after_docode == 0))
        {
            /* previous frame length is NOT valid */
            uint8_t data_buff[3] = {0};
            if (loop_buffer_get_queue_data(loop_buffer.out_index, 3, data_buff))
            {
                T_SBC_PARAMS sbc_params;
                if (sbc_get_params(data_buff, 3, &sbc_params) == SBC_SUCCESS)
                {
                    audio_global_data.bytes_per_frame_before_docode = 36;//rhqc_calc_encoded_frame_size(&sbc_params);
                    audio_global_data.sampling_freq = (T_AUDIO_FREQ)sbc_params.samplingFrequency;

                    if (buffer_data_len >= audio_global_data.bytes_per_frame_before_docode *
                        audio_global_data.decode_frame_cnt_for_once)
                    {
                        *p_frame_len = audio_global_data.bytes_per_frame_before_docode *
                                       audio_global_data.decode_frame_cnt_for_once;
                        result = true;
                    }
                    else
                    {
                        result = false;
                    }
                }
                else
                {
                    result = false;
                }
            }
            else
            {
                AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                                 "[audio_handle_check_frame_complete] loop_buffer_get_queue_data failed", 0);
            }
        }
        else if (buffer_data_len >= audio_global_data.bytes_per_frame_before_docode *
                 audio_global_data.decode_frame_cnt_for_once)
        {
            *p_frame_len = audio_global_data.bytes_per_frame_before_docode *
                           audio_global_data.decode_frame_cnt_for_once;
            result = true;
        }
        else
        {
            result = false;
        }
    }

    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                     "[audio_handle_check_frame_complete] result = %d, frame_len = %d, buffer_data_len = %d",
                     3, result, *p_frame_len, buffer_data_len);
    if (result == false)
    {
#if AUDIO_CODEC_SUPPORT_CALC_TIMING
        GPIO_SetBits(GPIO_GetPin(AUDIO_CODEC_CALC_PIN));
        GPIO_ResetBits(GPIO_GetPin(AUDIO_CODEC_CALC_PIN));
#endif
    }
    return result;
}

DATA_RAM_FUNCTION
void audio_handle_msg(T_IO_MSG *p_io_msg)
{
    if (p_io_msg->subtype == IO_MSG_AUDIO_PROCESS_DONE)
    {
        uint8_t block_index = (uint8_t)(p_io_msg->u.param);
        AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_msg] IO_MSG_AUDIO_PROCESS_DONE %d", 1,
                         block_index);

        audio_handle_loop_buffer_data(block_index);
#if FEATURE_SUPPORT_CODEC_TEST
        if (audio_global_data.is_codec_testing)
        {
            audio_handle_prepare_test_audio_buffer();
        }
#endif
    }
    else if (p_io_msg->subtype == IO_MSG_AUDIO_TIMEOUT)
    {

    }
#if AUDIO_SUPPORT_HEADPHONE_DETECT
    else if (p_io_msg->subtype == IO_MSG_AUDIO_HD_DETACHED)
    {
//        if (audio_global_data.is_working == true)
//        {
//            audio_handle_stop();
//        }
//        I2S_DeInit(I2S_USR);
//        ALC5616_DeInit();
//        audio_handle_disable_power();

    }
    else if (p_io_msg->subtype == IO_MSG_AUDIO_HD_ATTACHED)
    {
        audio_handle_enable_power();
    }
#endif
}


DATA_RAM_FUNCTION
bool audio_handle_loop_buffer_data(uint8_t free_index)
{
    bool result = false;
    uint32_t frame_len = 0;
    int32_t dec_res = 0;
    uint32_t loop_index = 0;

    uint8_t *p_output_buff = NULL;

    if (free_index == 0)
    {
        p_output_buff = (void *)audio_bi_buffer_get_buf0_addr();
    }
    else
    {
        p_output_buff = (void *)audio_bi_buffer_get_buf1_addr();
    }

    /* check if audio frame is complete */
    if (audio_handle_check_frame_complete(&frame_len))
    {

        if (frame_len > LOOP_BUFFER_MAX_SIZE - 1)
        {
            APP_PRINT_WARN1("[audio_handle_loop_buffer_data] frame_len(%d) is too large, just empty buffer to recover",
                            frame_len);
            /* empty queue */
            uint32_t cur_data_bytes = loop_buffer_get_data_len();
            loop_buffer.total_out_index += cur_data_bytes;
            loop_buffer.out_index = loop_buffer.in_index;

            result = false;
        }
        else
        {
            uint8_t *p_frame_data = os_mem_alloc(RAM_TYPE_DATA_ON, frame_len);

            if (loop_buffer_out_queue(p_frame_data, frame_len))
            {

                /* start decode frame */
                for (loop_index = 0; loop_index < audio_global_data.decode_frame_cnt_for_once; loop_index++)
                {
                    /* start to decode frame */
                    dec_res = 0;
                    int32_t tmp_output_size = LOOP_BUFFER_MAX_SIZE;
                    uint8_t *tmp_input_buff = p_frame_data + audio_global_data.bytes_per_frame_before_docode *
                                              loop_index;
                    uint8_t *tmp_output_buff = p_output_buff + audio_global_data.bytes_per_frame_after_docode *
                                               loop_index;

                    dec_res = sbc_decode(tmp_input_buff, audio_global_data.bytes_per_frame_before_docode,
                                         tmp_output_buff, &tmp_output_size);

                    if (dec_res == audio_global_data.bytes_per_frame_before_docode)
                    {
                        if (audio_global_data.bytes_per_frame_after_docode == 0)
                        {
                            /* first time decode successfully */
                            audio_global_data.bytes_per_frame_after_docode = tmp_output_size;
                            result = true;
                        }
                        else
                        {
                            if (audio_global_data.bytes_per_frame_after_docode == tmp_output_size)
                            {
                                result = true;
                            }
                            else
                            {
                                result = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        result = false;
                        break;
                    }

                }
            }
            else
            {
                result = false;
            }

            os_mem_free(p_frame_data);
        }
    }

    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                     "[audio_handle_loop_buffer_data] result = %d, free_index = %d, dec_res = %d, loop_index = %d",
                     4, result, free_index, dec_res, loop_index);


    if (result == false)
    {
        /* set buffer to all 0 */
        memset(p_output_buff, 0, AUDIO_BI_BUFFER_MAX_SIZE);
#if AUDIO_CODEC_SUPPORT_CALC_TIMING
        GPIO_SetBits(GPIO_GetPin(AUDIO_CODEC_CALC_PIN));
        GPIO_ResetBits(GPIO_GetPin(AUDIO_CODEC_CALC_PIN));
#endif
    }
    else
    {
        audio_global_data.audio_frame_index++;
    }

    return result;
}

/**
 * @brief    codec_power_on_delay_timer_cb is used to initialize codec after timeout
 * @param    p_timer - point of timer
 * @return   none
 * @retval   void
 * Caution   do NOT excute time consumption functions in timer callback
 */
DATA_RAM_FUNCTION
static void codec_power_on_delay_timer_cb(TimerHandle_t p_timer)
{
    APP_PRINT_INFO0("[codec_power_on_delay_timer_cb] timeout");

#if (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_I2S)
    Board_AudioTrans_Init();
    Driver_AudioTrans_Init(AUDIO_FREQU16000);
    app_global_data.cur_audio_sample_freq = AUDIO_FREQU16000;
#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_enable_power] init ALC5616", 0);
    ALC5616_I2C_Pinmux_Pad_Config();
    ALC5616_Init();
#elif (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5628)
    AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[audio_handle_enable_power] init ALC5628", 0);
    ALC5628_I2C_Pinmux_Pad_Config();
    ALC5628_Init();
#endif
    audio_handle_volume_unmute();  /* use unmute to reset volume config */

#elif (AUDIO_TRANS_OUTPUT_SEL == AUDIO_TRANS_OUTPUT_UART)
    Pad_Config(AUDIO_UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(AUDIO_UART_TX_PIN, UART0_TX);
    Pinmux_Config(AUDIO_UART_RX_PIN, UART0_RX);

#endif

}

/**
 * @brief    audio handler init timer
 */
void audio_handle_init_timer(void)
{
    APP_PRINT_INFO0("[audio_handle_init_timer] init timer");
    /* codec_power_on_delay_timer is used to initialize codec after timeout */
    if (false == os_timer_create(&codec_power_on_delay_timer,
                                 "codec_power_on_delay_timer",  1, \
                                 AUDIO_CODEC_POWER_ON_DELAY_TIMEOUT, false, codec_power_on_delay_timer_cb))
    {
        APP_PRINT_ERROR0("[audio_handle_init_timer] codec_power_on_delay_timer creat failed!");
    }
}

#if FEATURE_SUPPORT_CODEC_TEST
void audio_handle_prepare_test_audio_buffer(void)
{
    /* get free buffer */
    if (audio_global_data.current_audio_test_file_index + CODEC_TEST_AUDIO_BUFFER_SIZE >
        CODEC_TEST_AUDIO_FILE_MAX_LEN)
    {
        APP_PRINT_INFO0("[audio_handle_prepare_test_audio_buffer] stop audio");
        /* test audio file end */
        audio_handle_stop();
    }
    else if ((loop_buffer_get_free_len() > CODEC_TEST_AUDIO_BUFFER_SIZE)
             && (loop_buffer_get_free_len() > LOOP_BUFFER_NOTIFY_THRESHOLD))
    {
        APP_PRINT_INFO1("[audio_handle_prepare_test_audio_buffer] prepare test buffer 0x%X",
                        audio_global_data.current_audio_test_file_index);

        uint32_t buffer_addr = CODEC_TEST_AUDIO_FILE_BASE_ADDR +
                               audio_global_data.current_audio_test_file_index;

        uint8_t *audio_codec_test_buffer = os_mem_alloc(RAM_TYPE_DATA_ON, CODEC_TEST_AUDIO_BUFFER_SIZE);
        memcpy(audio_codec_test_buffer, (void *)buffer_addr, CODEC_TEST_AUDIO_BUFFER_SIZE);
        audio_global_data.current_audio_test_file_index += CODEC_TEST_AUDIO_BUFFER_SIZE;

        app_handle_hids_ads_write_char_tx(audio_codec_test_buffer, CODEC_TEST_AUDIO_BUFFER_SIZE);

        os_mem_free(audio_codec_test_buffer);
    }
}
#endif

#endif  // #if FEATURE_SUPPORT_AUDIO_DOWN_STREAMING

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

