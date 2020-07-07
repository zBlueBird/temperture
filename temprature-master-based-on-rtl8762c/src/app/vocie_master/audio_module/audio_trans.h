/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      audio_trans.h
* @brief    This file provides audio transmission layer module driver
* @details
* @author    elliot chen
* @date      2018-3-20
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __AUDIO_TRANS_H
#define __AUDIO_TRANS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "rtl876x.h"
#include "rtl876x_bitfields.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_i2s.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"

/* Defines -------------------------------------------------------------------*/

//#define USE_MEM_TO_UART
#ifdef USE_MEM_TO_UART
#include "rtl876x_uart.h"

#define AUDIO_I2S_GDMA_MSIZE_BYTES 4
#define AUDIO_UART_GDMA_MSIZE_BYTES 1

#endif

/* I2S defines section configuration */
#define I2S_USR_BCLK                    BCLK_SPORT1
#define I2S_USR_LRC                     LRC_SPORT1
#define I2S_USR_DACDAT                  DACDAT_SPORT1
#define APB_I2S_USR                     APBPeriph_I2S1
#define APB_I2S_USR_CLK                 APBPeriph_I2S1_CLOCK
#define I2S_USR                         I2S1
#define GDMA_Handshake_I2S_USR_TX       GDMA_Handshake_SPORT1_TX

/* GDMA define section */

#define AudioTrans_GDMA_Channel         GDMA_Channel2
#define AudioTrans_GDMA_Channel_NUM     2
#define AudioTrans_GDMA_Channel_IRQn    GDMA0_Channel2_IRQn
#define AudioTrans_GDMA_Handler         GDMA0_Channel2_Handler

/** @defgroup pfnGDMAIntrHandlerCB_t   pfnGDMAIntrHandlerCB_t
  * @brief Function pointer type used by GDMA interrupt handle to general Callback, to send events to application.
  * @{
  */

typedef void (*pfnGDMAIntrHandlerCB_t)(void *pParam);

/**
  * @}
  */


typedef enum
{
    AUDIO_FREQU16000 = 0,
    AUDIO_FREQU32000,
    AUDIO_FREQU44100,
    AUDIO_FREQU48000,
} T_AUDIO_FREQ;

/** @defgroup AudioTrans_TransType
  * @brief
  * @{
  */
typedef enum
{
    AUDIO_TRANS_TYPE_MEM_TO_I2S,
#ifdef USE_MEM_TO_UART
    AUDIO_TRANS_TYPE_MEM_TO_UART,
#endif
} AudioTrans_TypeDef;

/**
  * @}
  */

/** @defgroup AudioTrans_StreamType
  * @brief
  * @{
  */

typedef enum
{
    Audio_BT_Stream,
    Audio_Flash_Stream
} AudioTrans_StreamSrc;

/**
  * @}
  */

/** @defgroup AudioTrans_InitTypeDef
  * @brief
  * @{
  */

typedef struct
{
    AudioTrans_TypeDef AudioTrans_Type;
    uint32_t mem_addr;
    uint32_t audio_size;
} AudioTrans_InitTypeDef;


void Board_AudioTrans_Init(void);
void Driver_AudioTrans_Init(T_AUDIO_FREQ sampling_freq);
void AudioTrans_GDMA_Init(AudioTrans_InitTypeDef *pAudioTrans_InitStruct);
void AudioTrans_Cmd(AudioTrans_InitTypeDef *pAudioTrans_InitStruct, FunctionalState NewState);
void AudioTrans_RegisterGDMAIntrHandlerCB(pfnGDMAIntrHandlerCB_t pFunc);
void AudioTrans_ConfigLLIStruct(uint8_t GDMA_block_index, uint32_t block_len, bool is_last_block);
void AudioTrans_ConfigDataRate(T_AUDIO_FREQ sampling_freq);
#ifdef USE_MEM_TO_UART
void Driver_AudioTrans_Init_UART(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__AUDIO_TRANS_H*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

