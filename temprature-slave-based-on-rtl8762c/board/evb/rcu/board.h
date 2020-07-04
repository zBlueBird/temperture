/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      board.h
* @brief     header file of Keypad demo.
* @details
* @author    chenjie_jin
* @date      2018-5-6
* @version   v1.2
* *********************************************************************************************************
*/


#ifndef _BOARD_H_
#define _BOARD_H_

#include "otp_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define H_DEMO_RCU            0  /* H Demo RCU */
#define R_DEMO_RCU            1  /* R Demo RCU */

#define RCU_HD_PLATFORM_SEL   H_DEMO_RCU  /* RCU Hardware Platform selection */

/*******************************************************
*                 GAP Parameter Config
*******************************************************/
#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define C_DEVICE_NAME 'B','e','e','2',' ','R','C','U'
#define C_DEVICE_NAME_LEN (8+1)  /* sizeof(C_DEVICE_NAME) + 1 */

#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define C_DEVICE_NAME 'M','e','d','i','a','Q',' ','R','C','U'
#define C_DEVICE_NAME_LEN (10+1)  /* sizeof(C_DEVICE_NAME) + 1 */

#endif

/*******************************************************
*                 OTA Config
*******************************************************/
#define SUPPORT_SILENT_OTA        1

#define OTA_BUF_CHECK_EN          1  /*1 enable, 0 disable, */
#define DFU_TEMP_BUFFER_SIZE      (2048) /*dfu max buffer size*/

/* user data signature and address */
#define SIGNATURE_APP_DEFINE      0xFFFE
#define APP_DEFINE_DATA_ADDR      0x900000
#define APP_DEFINE_DATA_SIZE      0x19000  //100K

/*******************************************************
*                 RCU Feature Config
*******************************************************/
#define FEATURE_SUPPORT_MULTIMEDIA_KEYBOARD 0  /* set 1 to enable multimedia keyboard */

#define FEATURE_SUPPORT_UNDIRECT_PROMPT_ADV 0  /* set 1 to enable UNDIRECT_PROMPT_ADV */

#define FEATURE_SUPPORT_NO_ACTION_DISCONN 0  /* set 1 to enable NO_ACTION_DISCONN after timeout */

#define FEATURE_SUPPORT_ANY_KEY_TRIG_PAIRING_ADV 0  /* set 1 to enable any key pressed event to trigger paring adv */

#define FEATURE_SUPPORT_POWER_ON_ADV 0  /* set 1 to enable power on adv */

#define FEATURE_SUPPORT_REMOVE_LINK_KEY_BEFORE_PAIRING 0 /* set 1 to enable remove link key before paring */

#define FEATURE_SUPPORT_MP_TEST_MODE 0  /* set 1 to enable MP test */

#define FEATURE_SUPPORT_PRIVACY 0  /* set 1 to enable privacy feature */

#define FEATURE_SUPPORT_HIDS_CHAR_AUTHEN_REQ 0  /* set 1 to enable HIDS char authentication request */

/*******************************************************
*                 FTL Address Config
*******************************************************/
#define MP_TEST_FTL_PARAMS_BASE_ADDR 0
#define MP_TEST_FTL_PARAMS_MAX_LEN 12

#define IR_MODULE_FTL_PARAMS_BASE_ADDR 100
#define IR_MODULE_FTL_PARAMS_MAX_LEN 640

/*******************************************************
*                 MP Test Config
*******************************************************/

#if FEATURE_SUPPORT_MP_TEST_MODE

#define MP_TEST_FTL_PARAMS_TEST_MODE_FLG_OFFSET (MP_TEST_FTL_PARAMS_BASE_ADDR)
#define MP_TEST_FTL_PARAMS_TEST_MODE_FLG_LEN 4

#define MP_TEST_FTL_PARAMS_LOCAL_BD_ADDR_OFFSET (MP_TEST_FTL_PARAMS_TEST_MODE_FLG_OFFSET + MP_TEST_FTL_PARAMS_TEST_MODE_FLG_LEN)
#define MP_TEST_FTL_PARAMS_LOCAL_BD_ADDR_LEN 8

#define MP_TEST_MODE_SUPPORT_HCI_UART_TEST      1  /* set 1 to support HCI Uart Test Mode */
#define MP_TEST_MODE_SUPPORT_DATA_UART_TEST     1  /* set 1 to support Data Uart Test Mode */
#define MP_TEST_MODE_SUPPORT_SINGLE_TONE_TEST   1  /* set 1 to support SingleTone Test Mode */
#define MP_TEST_MODE_SUPPORT_FAST_PAIR_TEST     1  /* set 1 to support Fast Pair Test */
#define MP_TEST_MODE_SUPPORT_AUTO_K_RF          0  /* set 1 to support Auto K RF */
#define MP_TEST_MODE_SUPPORT_DATA_UART_DOWNLOAD 0  /* set 1 to support Data UART download */

#define MP_TEST_MODE_TRIG_BY_GPIO               0x0001  /* GPIO signal while power on to trigger MP test mode */
#define MP_TEST_MODE_TRIG_BY_COMBINE_KEYS       0x0002  /* Combine keys to trigger MP test mode */

#define MP_TEST_MODE_TRIG_SEL                   (MP_TEST_MODE_TRIG_BY_GPIO | MP_TEST_MODE_TRIG_BY_COMBINE_KEYS)

#if (MP_TEST_MODE_TRIG_SEL & MP_TEST_MODE_TRIG_BY_GPIO)

#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define MP_TEST_TRIG_PIN_1      P3_2
#define MP_TEST_TRIG_PIN_2      P3_3

#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define MP_TEST_TRIG_PIN_1      P2_2
#define MP_TEST_TRIG_PIN_2      P2_3
#endif

#endif

#if MP_TEST_MODE_SUPPORT_DATA_UART_TEST
#define MP_TEST_UART_TX_PIN     P3_0
#define MP_TEST_UART_RX_PIN     P3_1
#endif

#endif

/*******************************************************
*                 Voice Module Config
*******************************************************/
#define RCU_VOICE_EN   0

#if RCU_VOICE_EN

/* mic type*/
#define AMIC_TYPE  0
#define DMIC_TYPE  1
/*voice type config*/
#define VOICE_MIC_TYPE    AMIC_TYPE

/* amic input type */
#define AMIC_INPUT_TYPE_DIFF    0
#define AMIC_INPUT_TYPE_SINGLE  1
#define AMIC_INPUT_TYPE_SEL     AMIC_INPUT_TYPE_DIFF

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
#define IFLYTEK_VOICE_FLOW      0
#define HIDS_GOOGLE_VOICE_FLOW  1
#define ATV_GOOGLE_VOICE_FLOW   2
#define RTK_GATT_VOICE_FLOW     3
#define VOICE_FLOW_SEL          IFLYTEK_VOICE_FLOW

/* voice encode type */
#define SW_MSBC_ENC      1  /* software msbc encode */
#define SW_SBC_ENC       2  /* software sbc encode */
#define SW_IMA_ADPCM_ENC 3  /* software IMA/DVI adpcm encode */

/* codec sample rate type */
#define CODEC_SAMPLE_RATE_8K 1  /* 8KHz codec sample rate */
#define CODEC_SAMPLE_RATE_16K 2  /* 16KHz codec sample rate */

/* voice encode config */
#if ((VOICE_FLOW_SEL == IFLYTEK_VOICE_FLOW) || (VOICE_FLOW_SEL == HIDS_GOOGLE_VOICE_FLOW))
#define VOICE_ENC_TYPE SW_MSBC_ENC
#define CODEC_SAMPLE_RATE_SEL CODEC_SAMPLE_RATE_16K

#elif (VOICE_FLOW_SEL == ATV_GOOGLE_VOICE_FLOW)
#define VOICE_ENC_TYPE SW_IMA_ADPCM_ENC  /* ATV_GOOGLE_VOICE_FLOW must use SW_IMA_ADPCM_ENC */
#define CODEC_SAMPLE_RATE_SEL CODEC_SAMPLE_RATE_8K

#elif (VOICE_FLOW_SEL == RTK_GATT_VOICE_FLOW)
#define VOICE_ENC_TYPE SW_SBC_ENC
#define CODEC_SAMPLE_RATE_SEL CODEC_SAMPLE_RATE_16K
#endif

#define SUPPORT_CODEC_EQ_CONFIG_FEATURE 0  /* set 1 to support EQ config */

#endif


/*******************************************************
*                 RCU Keyscan Config
*******************************************************/
/* keypad row and column */
#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define KEYPAD_ROW_SIZE       5
#define KEYPAD_COLUMN_SIZE    4

#define ROW0                  P4_3
#define ROW1                  P4_2
#define ROW2                  P4_1
#define ROW3                  P4_0
#define ROW4                  P0_6

#define COLUMN0               P3_0
#define COLUMN1               P3_1
#define COLUMN2               P3_2
#define COLUMN3               P3_3

#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define KEYPAD_ROW_SIZE       3
#define KEYPAD_COLUMN_SIZE    4

#define ROW0                  P3_2
#define ROW1                  P3_3
#define ROW2                  P3_4

#define COLUMN0               P4_0
#define COLUMN1               P4_1
#define COLUMN2               P4_2
#define COLUMN3               P4_3

#endif

/*******************************************************
*                 IR Module Config
*******************************************************/
#define IR_FUN           0

#if IR_FUN

/*IR send config*/
#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define IR_SEND_PIN           P2_3
#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define IR_SEND_PIN           P2_4
#endif

#define IR_SEND_WAVE_MAX_SIZE   70

/*IR repeat code period config */
#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define IR_REPEAT_CODE_PERIOD 108  /* 108 ms, unit ms */
#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define IR_REPEAT_CODE_PERIOD 110  /* 110 ms, unit ms */
#endif

/*IR learn module config*/
#define IR_LEARN_MODE    0

#if IR_LEARN_MODE

#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define IR_LEARN_PIN          P2_2
#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define IR_LEARN_PIN          P2_5
#endif

/* IR_LEARN_TRIG_MODE need to be configured according to hardware */
#define IR_LEARN_TRIG_RISING_EDGE 0
#define IR_LEARN_TRIG_FALL_EDGE   1
#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define IR_LEARN_TRIG_MODE          IR_LEARN_TRIG_FALL_EDGE
#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define IR_LEARN_TRIG_MODE          IR_LEARN_TRIG_FALL_EDGE
#endif

/*max key number can be storged*/
#define  IR_LEARN_MAX_KEY_NUM        0x06
#define  IR_WAVE_CONFIG_BASE_ADDR    IR_MODULE_FTL_PARAMS_BASE_ADDR
#define  IR_WAVE_DATA_BASE_ADDR      (IR_WAVE_CONFIG_BASE_ADDR + sizeof(uint32_t)/*config CRC value*/ + \
                                      sizeof(IR_LearnStorgeInfo)*IR_LEARN_MAX_KEY_NUM)
#define  IR_LEARN_WAVE_MAX_SIZE      IR_SEND_WAVE_MAX_SIZE

#endif
#endif

/*******************************************************
*              LED Module Config
*******************************************************/
#define LED_EN    0

#if LED_EN

#if (RCU_HD_PLATFORM_SEL == R_DEMO_RCU)
#define  LED_NUM_MAX   0x01
#define  LED_INDEX(n)   (n<<8)
/*uint16_t, first byte led index, last byte led pin*/
#define  LED_1         (LED_INDEX(0) | P2_4)

#elif (RCU_HD_PLATFORM_SEL == H_DEMO_RCU)
#define  LED_NUM_MAX   0x01
#define  LED_INDEX(n)   (n<<8)
/*uint16_t, first byte led index, last byte led pin*/
#define  LED_1         (LED_INDEX(0) | P2_2)

#endif

/* voltage level to trigger LED On action */
#define LED_ON_LEVEL_HIGH 0
#define LED_ON_LEVEL_LOW  1

#define LED_ON_LEVEL_TRIG LED_ON_LEVEL_HIGH

#endif

/*******************************************************
*              Battery Module Config
*******************************************************/
#define BAT_EN    1

#if BAT_EN

#define BAT_LPC_EN    0 /* 1 enable lpc, 0 disable */
#if BAT_LPC_EN
#define BAT_LPC_CONFIG             LPC_2000_mV /*lpc detect bat threshold value*/
#endif

#define BAT_LOW_POWER_THRESHOLD    2000 /*2.0v threshold for rcu system*/
#define BAT_ADC_DETECT_THRESHOLD   (BAT_LOW_POWER_THRESHOLD + 200)
#define BAT_LOW_POWER_INDICATE     1 /*1 led indicate enable, 0 disable */

#endif

/*******************************************************
*                 DLPS Module Config
*******************************************************/
#define DLPS_EN        1

/** @defgroup IO Driver Config
  * @note user must config it firstly!! Do not change macro names!!
  * @{
  */
/* if use user define dlps enter/dlps exit callback function */
#define USE_USER_DEFINE_DLPS_EXIT_CB      1
#define USE_USER_DEFINE_DLPS_ENTER_CB     1

/* if use any peripherals below, #define it 1 */
#define USE_I2C0_DLPS        1
#define USE_I2C1_DLPS        0
#if (ROM_WATCH_DOG_ENABLE == 1)
#define USE_TIM_DLPS         1 //must be 1 if enable watch dog
#else
#define USE_TIM_DLPS         0
#endif
#define USE_QDECODER_DLPS    0
#define USE_IR_DLPS          1
#define USE_RTC_DLPS         0
#define USE_UART_DLPS        0
#define USE_ADC_DLPS         1
#define USE_SPI0_DLPS        0
#define USE_SPI1_DLPS        0
#define USE_SPI2W_DLPS       0
#define USE_KEYSCAN_DLPS     0
#define USE_DMIC_DLPS        0
#define USE_GPIO_DLPS        1
#define USE_PWM0_DLPS        0
#define USE_PWM1_DLPS        0
#define USE_PWM2_DLPS        0
#define USE_PWM3_DLPS        0
#define USE_CODEC_DLPS       0
#define USE_I2S0_DLPS        0
#define USE_I2S1_DLPS        0


/* do not modify USE_IO_DRIVER_DLPS macro */
#define USE_IO_DRIVER_DLPS   (USE_I2C0_DLPS | USE_I2C1_DLPS | USE_TIM_DLPS | USE_QDECODER_DLPS\
                              | USE_IR_DLPS | USE_RTC_DLPS | USE_UART_DLPS | USE_SPI0_DLPS\
                              | USE_SPI1_DLPS | USE_SPI2W_DLPS | USE_KEYSCAN_DLPS | USE_DMIC_DLPS\
                              | USE_GPIO_DLPS | USE_USER_DEFINE_DLPS_EXIT_CB\
                              | USE_RTC_DLPS | USE_PWM0_DLPS | USE_PWM1_DLPS | USE_PWM2_DLPS\
                              | USE_PWM3_DLPS | USE_CODEC_DLPS | USE_I2S0_DLPS | USE_USER_DEFINE_DLPS_ENTER_CB)


#ifdef __cplusplus
}
#endif

#endif  /* _BOARD_H_ */

