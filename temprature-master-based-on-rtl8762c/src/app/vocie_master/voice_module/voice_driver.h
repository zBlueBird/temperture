/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     voice_driver.h
* @brief    This is the header file of user code which the voice driver resides in.
* @details
* @author   chenjie jin
* @date     2018-05-03
* @version  v1.1
*********************************************************************************************************
*/

#ifndef _VOICE_DRIVER_H_
#define _VOICE_DRIVER_H_

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "stdint.h"
#include "stdbool.h"
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                              Macros
 *============================================================================*/
/*voice config*/
#if (VOICE_ENC_TYPE == SW_MSBC_ENC)
#define VOICE_PCM_FRAME_SIZE 240
#define VOICE_PCM_FRAME_CNT 2
#define VOICE_GDMA_FRAME_SIZE (VOICE_PCM_FRAME_SIZE * VOICE_PCM_FRAME_CNT)
#define VOICE_FRAME_SIZE_AFTER_ENC 57
#define VOICE_REPORT_FRAME_SIZE ((VOICE_FRAME_SIZE_AFTER_ENC + 3) * VOICE_PCM_FRAME_CNT)  /* 3 bytes header per frame */

#elif (VOICE_ENC_TYPE == SW_SBC_ENC)
//#define BIT_POOL_SIZE   14  /* BIT_POOL_SIZE is support to adjust */
//#define VOICE_PCM_FRAME_CNT              5
//#define VOICE_PCM_FRAME_SIZE             256
//#define VOICE_FRAME_SIZE_AFTER_ENC       (2 * BIT_POOL_SIZE + 8)
//#define VOICE_REPORT_FRAME_SIZE          (VOICE_FRAME_SIZE_AFTER_ENC * VOICE_PCM_FRAME_CNT)
//#define VOICE_GDMA_FRAME_SIZE            (VOICE_PCM_FRAME_SIZE * VOICE_PCM_FRAME_CNT)

#elif (VOICE_ENC_TYPE == SW_IMA_ADPCM_ENC)
#define VOICE_PCM_FRAME_CNT              1
#define VOICE_PCM_FRAME_SIZE             512
#define VOICE_FRAME_SIZE_AFTER_ENC       128
#define VOICE_REPORT_FRAME_SIZE          (VOICE_FRAME_SIZE_AFTER_ENC * VOICE_PCM_FRAME_CNT + 6)
#define VOICE_GDMA_FRAME_SIZE            (VOICE_PCM_FRAME_SIZE * VOICE_PCM_FRAME_CNT)

#elif (VOICE_ENC_TYPE == SW_OPT_ADPCM_ENC)
#define VOICE_IMG_BITS 3
#define VOICE_PCM_FRAME_CNT   1
#define VOICE_PCM_FRAME_SIZE  320
#if (VOICE_IMG_BITS == 4)
#define VOICE_FRAME_SIZE_AFTER_ENC  80
#elif (VOICE_IMG_BITS == 3)
#define VOICE_FRAME_SIZE_AFTER_ENC  60
#endif
#define VOICE_REPORT_FRAME_SIZE     (VOICE_FRAME_SIZE_AFTER_ENC * VOICE_PCM_FRAME_CNT)
#define VOICE_GDMA_FRAME_SIZE       (VOICE_PCM_FRAME_SIZE * VOICE_PCM_FRAME_CNT)

#endif

/* GDMA RX defines */
#define GDMA_Channel_RX             GDMA_Channel3
#define GDMA_Channel_RX_NUM         3
#define GDMA0_Channel_RX_IRQn       GDMA0_Channel3_IRQn
#define GDMA0_Channel_RX_Handler    GDMA0_Channel3_Handler

/*============================================================================*
 *                         Types
 *============================================================================*/
/**
 * @brief Loop queue data struct
 */
//typedef struct
//{
//    uint8_t buf0[VOICE_GDMA_FRAME_SIZE];
//    uint8_t buf1[VOICE_GDMA_FRAME_SIZE];
//} T_GDMA_BUF_TYPE_DEF;

//typedef struct
//{
//    bool is_allowed_to_enter_dlps;  /* to indicate whether to allow to enter dlps or not */
//    bool is_voice_driver_working;  /* indicate whether voice driver is working or not */
//    uint8_t current_bibuff_index;  /* indicate which buffer the voice using now */
//    T_GDMA_BUF_TYPE_DEF gdma_buffer;  /* indicate the buffer used by voice gdma */
//} T_VOICE_DRIVER_GLOBAL_DATA;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
//extern T_VOICE_DRIVER_GLOBAL_DATA voice_driver_global_data;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void voice_driver_init_data(void);
void voice_driver_init(void);
void voice_driver_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
