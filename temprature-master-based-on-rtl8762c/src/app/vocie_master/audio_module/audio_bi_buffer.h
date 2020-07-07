/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      audio_bi_buffer.h
* @brief    header file of audio double buffer data struct.
* @details
* @author    Chenjie Jin
* @date      2018-05-16
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __AUDIO_BI_BUFFER_H
#define __AUDIO_BI_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/

/**
 * @brief Enable print log or not
 */

//#define PRINT_BI_BUFFER_LOG
#ifdef PRINT_BI_BUFFER_LOG
#define BI_BUFFER(MODULE, LEVEL, pFormat, para_num,...) DBG_BUFFER_##LEVEL(MODULE, pFormat, para_num, ##__VA_ARGS__)
#else
#define BI_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

/**
 * @brief Configure double buffer parameters
 */

#define AUDIO_BI_BUFFER_MAX_SIZE              2560

/**
  * @brief  double buffer status
  */

typedef enum
{
    AUDIO_BI_BUF_STATE_IDLE = 0,
    AUDIO_BI_BUF_STATE_PENDING,
    AUDIO_BI_BUF_STATE_PROCESSING,
} T_AUDIO_BI_BUF_STATUS;

/**
 * @brief Loop queue data struct
 */

typedef struct
{
    uint8_t curr_read_buf_index;
    uint8_t buf0[AUDIO_BI_BUFFER_MAX_SIZE];
    uint8_t buf1[AUDIO_BI_BUFFER_MAX_SIZE];
    volatile T_AUDIO_BI_BUF_STATUS buf0_stat;
    volatile T_AUDIO_BI_BUF_STATUS buf1_stat;
    uint16_t buf0_len;
    uint16_t buf1_len;
} T_AUDIO_BI_BUFF_DEF;

void audio_bi_buffer_init(void);
uint32_t audio_bi_buffer_get_buf0_addr(void);
void audio_bi_buffer_set_buf0_status(T_AUDIO_BI_BUF_STATUS status);
T_AUDIO_BI_BUF_STATUS audio_bi_buffer_get_buf0_status(void);
uint32_t audio_bi_buffer_get_buf1_addr(void);
void audio_bi_buffer_set_buf1_status(T_AUDIO_BI_BUF_STATUS status);
T_AUDIO_BI_BUF_STATUS audio_bi_buffer_get_buf1_status(void);
void audio_bi_buffer_set_read_index(uint8_t index);
uint8_t audio_bi_buffer_get_read_index(void);

#ifdef __cplusplus
}
#endif

#endif /*__AUDIO_BI_BUFFER_H */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

