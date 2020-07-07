/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     board.h
* @brief        Pin definitions
* @details
* @author   Chuanguo Xue
* @date     2015-4-7
* @version  v0.1
* *********************************************************************************************************
*/

#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************
*                 Audio Module Config
*******************************************************/
#define FEATURE_SUPPORT_AUDIO_DOWN_STREAMING 0  /* set 1 to enable audio down streaming feature */

#if FEATURE_SUPPORT_AUDIO_DOWN_STREAMING

#define FEATURE_SUPPORT_ADS_BY_HIDS 1  /* set 1 to support audio down streaming by HIDS */

#define AUDIO_CODEC_EN_PIN     H_1

/* I2C communication pin */
#define AUDIO_CODEC_SCL_Pin    P1_1//P1_7
#define AUDIO_CODEC_SDA_Pin    P1_0//P1_6

#define AUDIO_UART_TX_PIN      P3_0
#define AUDIO_UART_RX_PIN      P3_1

#define AUDIO_CODEC_I2S_BCLK_PIN        P3_0//P3_6
#define AUDIO_CODEC_I2S_LR_PIN          P3_1//P3_5
#define AUDIO_CODEC_I2S_DATA_PIN        P3_2//P3_4

#define AUDIO_CODEC_SUPPORT_CALC_TIMING 0
#if AUDIO_CODEC_SUPPORT_CALC_TIMING
#define AUDIO_CODEC_CALC_PIN     P1_5
#endif

#define AUDIO_TRANS_OUTPUT_I2S  0
#define AUDIO_TRANS_OUTPUT_UART 1
#define AUDIO_TRANS_OUTPUT_SEL AUDIO_TRANS_OUTPUT_I2S

#define AUDIO_CODEC_CHIPSET_ALC5616 0
#define AUDIO_CODEC_CHIPSET_ALC5628 1
#define AUDIO_CODEC_CHIPSET_SEL AUDIO_CODEC_CHIPSET_ALC5616

#define AUDIO_SUPPORT_HEADPHONE_DETECT 1
#if AUDIO_SUPPORT_HEADPHONE_DETECT
#define AUDIO_HD_DETECT_PIN   P2_5
#define AUDIO_HD_Int_Pin_IRQ  GPIO21_IRQn
#define audio_hd_intr_handler  GPIO21_Handler
#endif

#endif
/*******************************************************
*              LED Module Config
*******************************************************/
#define LED_EN    1

#if LED_EN

#define  LED_NUM_MAX   0x01
#define  LED_INDEX(n)   (n<<8)

/*uint16_t, first byte led index, last byte led pin*/
#define  LED_1         (LED_INDEX(0) | P0_6)

/* voltage level to trigger LED On action */
#define LED_ON_LEVEL_HIGH 0
#define LED_ON_LEVEL_LOW  1

#define LED_ON_LEVEL_TRIG LED_ON_LEVEL_HIGH

#endif
/*******************************************************
*                 VS1053b Config
*******************************************************/
#define  VS1053B_EN        1

#if VS1053B_EN
#define SPI0_SCK_PIN                    P2_3
#define SPI0_MOSI_PIN                   P2_6
#define SPI0_MISO_PIN                   P2_5

#define VS_DQ_PIN                       P2_2
#define VS_RST_PIN                      P2_7
#define VS_XCS_PIN                      P3_2
#define VS_XDCS_PIN                     P2_4
#endif
/*******************************************************
*                 Voice Module Config
*******************************************************/
#define RCU_VOICE_EN   1

#if RCU_VOICE_EN

/* mic type*/
#define AMIC_TYPE  0
#define DMIC_TYPE  1
/*voice type config*/
#define VOICE_MIC_TYPE    AMIC_TYPE
/* voice pin config */
#if (VOICE_MIC_TYPE == AMIC_TYPE)
#define  AMIC_PIN1            P2_6  /*MIC_N*/
#define  AMIC_PIN2            P2_7  /*MIC_P*/
#define  AMIC_MIC_BIAS        H_0   /*MICBIAS*/
#elif (VOICE_MIC_TYPE == DMIC_TYPE)
/* Pin define of DMIC peripheral */
#define DMIC_MSBC_CLK              P1_6
#define DMIC_MSBC_DAT              P1_7
#endif

/* voice flow type, default IFLYTEK_VOICE_FLOW */
#define IFLYTEK_VOICE_FLOW 0
#define HIDS_GOOGLE_VOICE_FLOW 1
#define ATV_GOOGLE_VOICE_FLOW 2
#define VOICE_FLOW_SEL IFLYTEK_VOICE_FLOW

/* voice encode type */
#define SW_MSBC_ENC      1  /* software msbc encode */
#define SW_SBC_ENC       2  /* software sbc encode */
#define SW_IMA_ADPCM_ENC 3  /* software IMA/DVI adpcm encode */
#define SW_OPT_ADPCM_ENC 4  /* software optimized adpcm encode */

/* codec sample rate type */
#define CODEC_SAMPLE_RATE_8K 1  /* 8KHz codec sample rate */
#define CODEC_SAMPLE_RATE_16K 2  /* 16KHz codec sample rate */

/* voice encode config */
#if ((VOICE_FLOW_SEL == IFLYTEK_VOICE_FLOW) || (VOICE_FLOW_SEL == HIDS_GOOGLE_VOICE_FLOW))
#define VOICE_ENC_TYPE SW_SBC_ENC
#define CODEC_SAMPLE_RATE_SEL CODEC_SAMPLE_RATE_16K

#elif (VOICE_FLOW_SEL == ATV_GOOGLE_VOICE_FLOW)
#define VOICE_ENC_TYPE SW_SBC_ENC  /* Use Simplified ATV Voice flow */
#define CODEC_SAMPLE_RATE_SEL CODEC_SAMPLE_RATE_16K

#endif

#endif

/*******************************************************
*                 RCU Keyscan Config
*******************************************************/
#define KEYSCAN_EN     1

/* keypad row and column */
#define KEYPAD_ROW_SIZE       5
#define KEYPAD_COLUMN_SIZE    4
#define ROW0                  P0_0
#define ROW1                  P0_1
#define ROW2                  P0_2
#define ROW3                  P1_1
#define ROW4                  P1_0

#define COLUMN0               P4_0
#define COLUMN1               P4_1
#define COLUMN2               P4_2
#define COLUMN3               P3_3

#define DATA_UART_TX_PIN    P3_0
#define DATA_UART_RX_PIN    P3_1

/* if use user define dlps enter/dlps exit callback function */
#define USE_USER_DEFINE_DLPS_EXIT_CB      0
#define USE_USER_DEFINE_DLPS_ENTER_CB     0

/* if use any peripherals below, #define it 1 */
#define USE_I2C0_DLPS        0
#define USE_I2C1_DLPS        0
#define USE_TIM_DLPS         0
#define USE_QDECODER_DLPS    0
#define USE_IR_DLPS          0
#define USE_RTC_DLPS         0
#define USE_UART_DLPS        0
#define USE_ADC_DLPS         0
#define USE_SPI0_DLPS        0
#define USE_SPI1_DLPS        0
#define USE_SPI2W_DLPS       0
#define USE_KEYSCAN_DLPS     0
#define USE_DMIC_DLPS        0
#define USE_GPIO_DLPS        0
#define USE_PWM0_DLPS        0
#define USE_PWM1_DLPS        0
#define USE_PWM2_DLPS        0
#define USE_PWM3_DLPS        0


/* do not modify USE_IO_DRIVER_DLPS macro */
#define USE_IO_DRIVER_DLPS   (USE_I2C0_DLPS | USE_I2C1_DLPS | USE_TIM_DLPS | USE_QDECODER_DLPS\
                              | USE_IR_DLPS | USE_RTC_DLPS | USE_UART_DLPS | USE_SPI0_DLPS\
                              | USE_SPI1_DLPS | USE_SPI2W_DLPS | USE_KEYSCAN_DLPS | USE_DMIC_DLPS\
                              | USE_GPIO_DLPS | USE_USER_DEFINE_DLPS_EXIT_CB\
                              | USE_RTC_DLPS | USE_PWM0_DLPS | USE_PWM1_DLPS | USE_PWM2_DLPS\
                              | USE_PWM3_DLPS | USE_USER_DEFINE_DLPS_ENTER_CB)

#ifdef __cplusplus
}
#endif

#endif

