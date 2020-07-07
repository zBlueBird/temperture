/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      audio_loop_buffer.c
* @brief    source code of loop buffer data struct.
* @details
* @author  chenjie_jin
* @date     2018-05-16
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "audio_loop_buffer.h"
#include "board.h"
#include "mem_config.h"
#include "app_section.h"
#include "board.h"
#include <rtl876x_gpio.h>
#if FEATURE_SUPPORT_AUDIO_DOWN_STREAMING

/* Globals Values -------------------------------------------------------------------*/
RAM_BUFFERON_BSS_SECTION T_LOOP_BUFFER_TYPE_DEF    loop_buffer __attribute__((used));

/* Local Functions -------------------------------------------------------------------*/

/* Global Functions -------------------------------------------------------------------*/
/**
  * @brief  get loop buffer free bytes length.
  */
DATA_RAM_FUNCTION
uint32_t loop_buffer_get_free_len(void)
{
    uint32_t free_bytes = 0;

    if (loop_buffer.in_index < loop_buffer.out_index)
    {
        free_bytes = loop_buffer.out_index - loop_buffer.in_index - 1;
    }
    else
    {
        free_bytes = LOOP_BUFFER_MAX_SIZE + loop_buffer.out_index - loop_buffer.in_index - 1;
    }

    return free_bytes;
}

/**
  * @brief  get loop buffer data length.
  */
DATA_RAM_FUNCTION
uint32_t loop_buffer_get_data_len(void)
{
    uint32_t len = 0;

    if (loop_buffer.in_index < loop_buffer.out_index)
    {
        len = LOOP_BUFFER_MAX_SIZE + loop_buffer.in_index - loop_buffer.out_index;
    }
    else
    {
        len = loop_buffer.in_index - loop_buffer.out_index;
    }

    return len;
}

/**
  * @brief  Initializes loop buffer to their default reset values.
  */
DATA_RAM_FUNCTION
void loop_buffer_init(void)
{
    memset(&loop_buffer, 0, sizeof(T_LOOP_BUFFER_TYPE_DEF));
}

/**
  * @brief  Loop buffer in queue handler.
  */
DATA_RAM_FUNCTION
bool loop_buffer_in_queue(uint8_t *p_data, uint32_t len)
{
    if (loop_buffer_get_free_len() >= len)
    {
        uint32_t bytes_to_end = LOOP_BUFFER_MAX_SIZE - loop_buffer.in_index;

        LOOP_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                        "[loop_buffer_in_queue] total_in_index = %d, in_index = %d, total_out_index = %d, out_index = %d, bytes_to_end = %d, len = %d",
                        6, loop_buffer.total_in_index, loop_buffer.in_index, loop_buffer.total_out_index,
                        loop_buffer.out_index, bytes_to_end, len);

        if (bytes_to_end == 0)
        {
            memcpy(loop_buffer.buf, p_data, len);
            loop_buffer.in_index = len;
        }
        else if (bytes_to_end >= len)
        {
            memcpy((loop_buffer.buf + loop_buffer.in_index), p_data, len);
            loop_buffer.in_index += len;
        }
        else
        {
            /* copy first segment */
            memcpy((loop_buffer.buf + loop_buffer.in_index), p_data, bytes_to_end);
            /* copy second segment */
            memcpy(loop_buffer.buf, (p_data + bytes_to_end), len - bytes_to_end);
            loop_buffer.in_index = len - bytes_to_end;
        }

        loop_buffer.in_index = loop_buffer.in_index % LOOP_BUFFER_MAX_SIZE;
        loop_buffer.total_in_index += len;
    }
    else
    {
        LOOP_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                        "[loop_buffer_in_queue] Remaining size is not enough, in queue failed!", 0);

        loop_buffer.out_index = loop_buffer.in_index;
        return false;
    }

    return true;
}

/**
  * @brief  Loop buffer out queue handler.
  */
DATA_RAM_FUNCTION
bool loop_buffer_out_queue(uint8_t *p_data, uint32_t len)
{
    uint32_t cur_data_bytes = loop_buffer_get_data_len();
    uint32_t bytes_to_end = LOOP_BUFFER_MAX_SIZE - loop_buffer.out_index;

    LOOP_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                    "[loop_buffer_out_queue] total_in_index = %d, in_index = %d, total_out_index = %d, out_index = %d, bytes_to_end = %d, len = %d, cur_data_bytes = %d",
                    7, loop_buffer.total_in_index, loop_buffer.in_index, loop_buffer.total_out_index,
                    loop_buffer.out_index, bytes_to_end, len, cur_data_bytes);

    if (cur_data_bytes >= len)
    {
        if (bytes_to_end == 0)
        {
            memcpy(p_data, loop_buffer.buf, len);
            loop_buffer.out_index = len;
        }
        else if (bytes_to_end >= len)
        {
            memcpy(p_data, (loop_buffer.buf + loop_buffer.out_index), len);
            loop_buffer.out_index += len;
        }
        else
        {
            /* first segment */
            memcpy(p_data, (loop_buffer.buf + loop_buffer.out_index), bytes_to_end);
            /* second segment */
            memcpy((p_data + bytes_to_end), loop_buffer.buf, (len - bytes_to_end));
            loop_buffer.out_index = len - bytes_to_end;
        }

        loop_buffer.out_index = loop_buffer.out_index % LOOP_BUFFER_MAX_SIZE;
        loop_buffer.total_out_index += len;
    }
    else
    {
        LOOP_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                        "[loop_buffer_out_queue] cur_data_bytes is not enough, out queue failed!", 0);
        return false;
    }

    return true;
}

/**
  * @brief  get loop queue buffer data.
  */
DATA_RAM_FUNCTION
bool loop_buffer_get_queue_data(uint32_t start_index, uint32_t len, uint8_t *p_data)
{
    uint32_t cur_data_bytes = loop_buffer_get_data_len();
    uint32_t bytes_to_end = LOOP_BUFFER_MAX_SIZE - loop_buffer.out_index;

    LOOP_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                    "[loop_buffer_get_queue_data] total_in_index = %d, in_index = %d, total_out_index = %d, out_index = %d, bytes_to_end = %d, len = %d, cur_data_bytes = %d",
                    7, loop_buffer.total_in_index, loop_buffer.in_index, loop_buffer.total_out_index,
                    loop_buffer.out_index, bytes_to_end, len, cur_data_bytes);

    if ((cur_data_bytes >= len) && (len > 0))
    {
        if (bytes_to_end == 0)
        {
            memcpy(p_data, loop_buffer.buf, len);
        }
        else if (bytes_to_end >= len)
        {
            memcpy(p_data, (loop_buffer.buf + loop_buffer.out_index), len);
        }
        else
        {
            /* first segment */
            memcpy(p_data, (loop_buffer.buf + loop_buffer.out_index), bytes_to_end);
            /* second segment */
            memcpy((p_data + bytes_to_end), loop_buffer.buf, (len - bytes_to_end));
        }
    }
    else
    {
        LOOP_DBG_BUFFER(MODULE_APP, LEVEL_WARN,
                        "[loop_buffer_get_queue_data] cur_data_bytes is not enough or len is invalid!", 0);
        return false;
    }

    return true;
}

#endif

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

