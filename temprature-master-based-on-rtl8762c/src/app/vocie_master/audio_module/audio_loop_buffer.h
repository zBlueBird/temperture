/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      audio_loop_buffer.h
* @brief    header file of loop buffer data struct.
* @details
* @author    chenjie jin
* @date      2018-05-16
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __AUDIO_LOOP_BUFFER_H
#define __AUDIO_LOOP_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "trace.h"
#include "board.h"

/* Defines ------------------------------------------------------------------*/

/**
 * @brief Enable print log or not
 */
#if FEATURE_SUPPORT_AUDIO_DOWN_STREAMING

#define PRINT_LOOP_BUFFER_LOG
#ifdef PRINT_LOOP_BUFFER_LOG
#define LOOP_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) DBG_BUFFER_##LEVEL(TYPE_BEE2, SUBTYPE_FORMAT, MODULE, fmt, para_num, ##__VA_ARGS__)
#else
#define LOOP_DBG_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

/**
 * @brief Configure double buffer parameters
 */

#define LOOP_BUFFER_MAX_SIZE              (1024 * 7)
#define LOOP_BUFFER_NOTIFY_THRESHOLD      (LOOP_BUFFER_MAX_SIZE / 5)

/**
 * @brief Loop queue data struct
 */

typedef struct
{
    uint32_t in_index;
    uint32_t out_index;
    uint32_t total_in_index;
    uint32_t total_out_index;
    uint8_t buf[LOOP_BUFFER_MAX_SIZE];
} T_LOOP_BUFFER_TYPE_DEF;

extern T_LOOP_BUFFER_TYPE_DEF loop_buffer;

void loop_buffer_init(void);
uint32_t loop_buffer_get_free_len(void);
uint32_t loop_buffer_get_data_len(void);
bool loop_buffer_in_queue(uint8_t *p_data, uint32_t len);
bool loop_buffer_out_queue(uint8_t *p_data, uint32_t len);
bool loop_buffer_get_queue_data(uint32_t start_index, uint32_t len, uint8_t *p_data);

#endif

#ifdef __cplusplus
}
#endif

#endif /*__AUDIO_LOOP_BUFFER_H */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

