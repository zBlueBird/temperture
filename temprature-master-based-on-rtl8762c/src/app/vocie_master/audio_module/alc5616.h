/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      alc5616.h
* @brief
* @details
* @author    elliot chen
* @date      2018-04-13
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __ALC5616_H
#define __ALC5616_H

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

#if (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616)

/** @defgroup ALC5616_DAC_Channel DAC Channel
  * @{
  */

#define Left_Headphone                          ((uint16_t)(1 << 8))
#define Right_Headphone                         ((uint16_t)(0))

#define IS_HEADPHONE_CHANNEL(CHANNEL) (((CHANNEL) == Left_Headphone) || ((CHANNEL) == Right_Headphone))
/**
  * @}
  */

#define ALC5616_DAC_VOL_DEFAULT_VALUE   0x8F
#define ALC5616_DAC_VOL_MIN_VALUE       0x00
#define ALC5616_DAC_VOL_MAX_VALUE       0xAF
#define ALC5616_DAC_VOL_STEP            0x08

/* User configure -------------------------------------------------------------*/

/* I2C communication pin */
#define ALC5616_SCL_Pin         AUDIO_CODEC_SCL_Pin
#define ALC5616_SDA_Pin         AUDIO_CODEC_SDA_Pin
#define ALC_5616_I2C_CLK        I2C0_CLK
#define ALC_5616_I2C_SDA        I2C0_DAT

/* I2C define section */
#define ALC5616_I2C             I2C0
#define ALC5616_I2C_Speed       100000
#define ALC5616_Address         0x1B

/* Defines -------------------------------------------------------------------*/

void ALC5616_I2C_Pinmux_Pad_Config(void);
uint16_t ALC5616_ReadRegister(I2C_TypeDef *I2Cx, uint8_t reg_address);
uint8_t ALC5616_Reset(void);
void ALC5616_Init(void);
void ALC5616_DeInit(void);
void ALC5616_DACVolumeConfig(uint16_t Channel, uint16_t data);

#endif

#ifdef __cplusplus
}
#endif

#endif /*__ALC5616_H*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

