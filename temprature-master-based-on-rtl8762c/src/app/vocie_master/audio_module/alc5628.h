/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      alc5628.h
* @brief
* @details
* @author    elliot chen
* @date      2015-10-12
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __ALC5628_H
#define __ALC5628_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "rtl876x.h"
#include "rtl876x_bitfields.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_i2c.h"
#include "rtl876x_nvic.h"
#include "trace.h"

#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5628)
/* User configure -------------------------------------------------------------*/

/* I2C communication pin */
#define ALC5628_SCL_Pin         AUDIO_CODEC_SCL_Pin
#define ALC5628_SDA_Pin         AUDIO_CODEC_SDA_Pin
#define ALC5628_SCL_PINMUX      I2C0_CLK
#define ALC5628_SDA_PINMUX      I2C0_DAT
/* I2C define section */

#define ALC5628_I2C             I2C0
#define ALC5628_I2C_Speed       100000UL
#define ALC5628_Address         0x18

/* Defines -------------------------------------------------------------------*/

/** @defgroup ALC5628_Data_format data format selection
  * @{
  */
#define I2S_DF_I2S              (0x0)           //I2S FORMAT 
#define I2S_DF_LEFT             (0x1)           //LEFT JUSTIFIED FORMAT
#define I2S_DF_PCM_A            (0x2)           //PCM MODE A
#define I2S_DF_PCM_B            (0x3)           //PCM MODE B

#define IS_DATA_FORMAT(FORMAT) (((FORMAT) == I2S_DF_I2S) || ((FORMAT) == I2S_DF_LEFT)||\
                                ((FORMAT) == I2S_DF_PCM_A) || ((FORMAT) == I2S_DF_PCM_B))
/**
  * @}
  */

/** @defgroup ALC5628_Headphone_Channel headphone Channel
  * @{
  */

#define HP_Channel_Left                         ((uint16_t)(1 << 3))
#define HP_Channel_Right                        ((uint16_t)(0))

#define IS_HP_CHANNEL(CHANNEL) (((CHANNEL) == HP_Channel_Left) || ((CHANNEL) == HP_Channel_Right))
/**
  * @}
  */


/** @defgroup ALC5628_DAC_Channel DAC Channel
  * @{
  */

#define DAC_Channel_Left                            ((uint16_t)(1 << 3))
#define DAC_Channel_Right                           ((uint16_t)(0))

#define IS_DAC_CHANNEL(CHANNEL) (((CHANNEL) == DAC_Channel_Left) || ((CHANNEL) == DAC_Channel_Right))
/**
  * @}
  */

void ALC5628_I2C_Pinmux_Pad_Config(void);
uint8_t ALC5628_Reset(void);
void ALC5628_Init(void);
uint8_t ALC5628_DataFormatConfig(uint16_t data);
uint8_t ALC5628_HeadphoneVolumeConfig(uint8_t Headphone_Channel, uint16_t data);
uint8_t ALC5628_DACVolumeConfig(uint8_t DAC_Channel, uint16_t data);
uint16_t ALC5628_ReadRegister(I2C_TypeDef *I2Cx, uint8_t reg_address);

#endif

#ifdef __cplusplus
}
#endif

#endif /*__ALC5628_H*/


/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/

