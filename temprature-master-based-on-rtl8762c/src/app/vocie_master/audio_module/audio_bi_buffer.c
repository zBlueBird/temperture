/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      bibuffer.c
* @brief    double buffer data struct.
* @details
* @author  elliot_chen
* @date     2017-06-05
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "audio_bi_buffer.h"
#include "board.h"

#include "mem_config.h"
#if 1//SCENARIO_SWITCH_EN
#include "overlay_mgr.h"
#include "app_section.h"
#endif

/* Defines -------------------------------------------------------------------*/
#define APP_SECTION_AUDIO DATA_RAM_FUNCTION

/* Globals -------------------------------------------------------------------*/
RAM_BUFFERON_BSS_SECTION T_AUDIO_BI_BUFF_DEF    audio_bi_buffer __attribute__((used));
//APP_SECTION_AUDIO T_AUDIO_BI_BUFF_DEF   audio_bi_buffer;

/**
  * @brief  Initializes double buffer to their default reset values.
  * @param None.
  * @retval None
  */
APP_SECTION_AUDIO
void audio_bi_buffer_init(void)
{
    memset(&audio_bi_buffer, 0, sizeof(T_AUDIO_BI_BUFF_DEF));
    audio_bi_buffer_set_buf0_status(AUDIO_BI_BUF_STATE_IDLE);
    audio_bi_buffer_set_buf1_status(AUDIO_BI_BUF_STATE_IDLE);
}

/**
  * @brief  Get buffer address.
  * @param None.
  * @retval buffer address.
  */
APP_SECTION_AUDIO
uint32_t audio_bi_buffer_get_buf0_addr(void)
{
    return (uint32_t)audio_bi_buffer.buf0;
}

/**
  * @brief Set buffer status.
  * @param None.
  * @retval None.
  */
APP_SECTION_AUDIO
void audio_bi_buffer_set_buf0_status(T_AUDIO_BI_BUF_STATUS status)
{
    APP_PRINT_INFO1("[audio_bi_buffer_set_buf0_status] status is %d", status);
    audio_bi_buffer.buf0_stat = status;
}

/**
  * @brief  Get buffer status.
  * @param None.
  * @retval None.
  */
APP_SECTION_AUDIO
T_AUDIO_BI_BUF_STATUS audio_bi_buffer_get_buf0_status(void)
{
    return audio_bi_buffer.buf0_stat;
}

/**
  * @brief  Get buffer address.
  * @param None.
  * @retval buffer address.
  */
APP_SECTION_AUDIO
uint32_t audio_bi_buffer_get_buf1_addr(void)
{
    return (uint32_t)audio_bi_buffer.buf1;
}

/**
  * @brief Set buffer status.
  * @param None.
  * @retval None.
  */
APP_SECTION_AUDIO
void audio_bi_buffer_set_buf1_status(T_AUDIO_BI_BUF_STATUS status)
{
    APP_PRINT_INFO1("[audio_bi_buffer_set_buf1_status] status is %d", status);
    audio_bi_buffer.buf1_stat = status;
}

/**
  * @brief  Get buffer status.
  * @param None.
  * @retval None.
  */
APP_SECTION_AUDIO
T_AUDIO_BI_BUF_STATUS audio_bi_buffer_get_buf1_status(void)
{
    return audio_bi_buffer.buf1_stat;
}


/**
  * @brief Set current bibuffer index.
* @param curr_read_buffer_index: set current read buffer index.
  * @retval None.
  */
APP_SECTION_AUDIO
void audio_bi_buffer_set_read_index(uint8_t index)
{
    audio_bi_buffer.curr_read_buf_index = index;
}

/**
  * @brief  Get current bibuffer index.
  * @param None.
  * @retval bi buffer index.
  */
APP_SECTION_AUDIO
uint8_t audio_bi_buffer_get_read_index(void)
{
    return audio_bi_buffer.curr_read_buf_index;
}

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

