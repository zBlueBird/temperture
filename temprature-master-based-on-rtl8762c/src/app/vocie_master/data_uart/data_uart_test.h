/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rcu_uart_test.h
* @brief
* @details
* @author    chenjie jin
* @date      2018-04-08
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __RCU_UART_TEST_H
#define __RCU_UART_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "uart_transport.h"
#include "app_msg.h"

/* Defines ------------------------------------------------------------------ */

/** @brief  Number of function used in application.*/
/* You must modify this value according the function number used in your application. */

#define UART_FN_MASK                    ((uint16_t)0x00FF)

/* Protocol command defines */
#define UART_FN_BEGIN                   ((uint16_t)0x1100)

#define VOICE_CMD_SCAN_DEVICE               ((uint16_t)0x1100)
#define VOICE_CMD_STOP_SCAN                 ((uint16_t)0x1101)
#define VOICE_CMD_SHOW_DEVICE               ((uint16_t)0x1102)
#define VOICE_CMD_CONN_DEVICE               ((uint16_t)0x1103)
#define VOICE_CMD_AUTH_DEVICE               ((uint16_t)0x1104)
#define VOICE_CMD_CCCD_WRITE                ((uint16_t)0x1105)
#define VOICE_CMD_MAC_CONFIG                ((uint16_t)0x1106)
#define SET_VOICE_CONFIG_CMD                ((uint16_t)0x1107)
#define GET_VOICE_CONFIG_CMD                ((uint16_t)0x1108)
#define ENTER_DLPS_TEST_MODE_CMD            ((uint16_t)0x1109)
#define START_STOP_ADV_CMD                  ((uint16_t)0x110A)
#define START_IR_TEST_MODE_CMD              ((uint16_t)0x110B)
#define ENTER_HCI_TEST_MODE_CMD             ((uint16_t)0x110C)
#define DISABLE_TEST_MODE_FLG_CMD           ((uint16_t)0x110D)
#define ENABLE_TEST_MODE_FLG_CMD            ((uint16_t)0x110E)
#define ERASE_PAIR_INFO_CMD                 ((uint16_t)0x110F)
#define CHANGE_BAUDRATE_CMD                 ((uint16_t)0x1110)
#define DIRECT_K_RF_FREQ_CMD                ((uint16_t)0x1111)
#define GET_GLODEN_INFO_CMD                 ((uint16_t)0x1112)
#define GET_DUT_INFO_CMD                    ((uint16_t)0x1113)
#define VERIFY_DUT_INFO_CMD                 ((uint16_t)0x1114)
#define AUTO_K_RF_FREQ_CMD                  ((uint16_t)0x1115)
#define FIND_DEVICE_TYPE_CMD                ((uint16_t)0x1116)
#define REBOOT_DEVICE_CMD                   ((uint16_t)0x1117)
#define UPDATE_MAC_ADDR_CMD                 ((uint16_t)0x1118)
#define ENTER_SINGLE_TONE_MODE_CMD          ((uint16_t)0x1119)
#define READ_HARDWARE_VERSION_CMD           ((uint16_t)0x111A)
#define TERMINATE_CONNECT_CMD               ((uint16_t)0x111B)
#define MANUAL_K_RF_FREQ_CMD                ((uint16_t)0x111C)
#define ENTER_HCI_DOWNLOAD_MODE_CMD         ((uint16_t)0x111D)
#define SET_CODEC_EQ_CONFIG_CMD             ((uint16_t)0x111E)
#define GET_CODEC_EQ_CONFIG_CMD             ((uint16_t)0x111F)

#define UART_FN_END                     ((uint16_t)0x1120)

#define UART_TEST_SUPPORT_NUM         (UART_FN_END - UART_FN_BEGIN)

typedef void (*p_uart_test_func)(void *);

typedef struct
{
    uint16_t opcode;
    uint16_t param_len;
    p_uart_test_func fncb;
} T_UART_TEST_PROTOCOL;

typedef enum
{
    UART_TEST_SUCCESS = 0,
    UART_TEST_ERROR = 1
} T_UART_TEST_STATUS;

typedef enum
{
    CHANGE_BAUDRATE_OPTION_115200 = 0,
    CHANGE_BAUDRATE_OPTION_1M = 1,
    CHANGE_BAUDRATE_OPTION_2M = 2,
} CHANGE_BAUDRATE_OPTION;

typedef enum
{
    UART_TEST_MIC_TYPE_AMIC = 0,
    UART_TEST_MIC_TYPE_DMIC = 1,
} UART_TEST_MIC_TYPE;

typedef enum
{
    UART_TEST_MIC_INPUT_TYPE_DIFF = 0,
    UART_TEST_MIC_INPUT_TYPE_SINGLE = 1,
} UART_TEST_MIC_INPUT_TYPE;

typedef enum
{
    UART_TEST_MIC_BIAS_VOL_1_507 = 0,
    UART_TEST_MIC_BIAS_VOL_1_62 = 1,
    UART_TEST_MIC_BIAS_VOL_1_705 = 2,
    UART_TEST_MIC_BIAS_VOL_1_8 = 3,
    UART_TEST_MIC_BIAS_VOL_1_906 = 4,
    UART_TEST_MIC_BIAS_VOL_2_025 = 5,
    UART_TEST_MIC_BIAS_VOL_2_16 = 6,
    UART_TEST_MIC_BIAS_VOL_2_314 = 7,
    UART_TEST_MIC_BIAS_VOL_ENUM_GUARD,
} UART_TEST_MIC_BIAS_VOL;

typedef enum
{
    UART_TEST_AMIC_ANALOG_GAIN_0dB = 0,
    UART_TEST_AMIC_ANALOG_GAIN_20dB = 1,
    UART_TEST_AMIC_ANALOG_GAIN_30dB = 2,
    UART_TEST_AMIC_ANALOG_GAIN_40dB = 3,
    UART_TEST_AMIC_ANALOG_GAIN_ENUM_GUARD,
} UART_TEST_AMIC_ANALOG_GAIN;

typedef enum
{
    UART_TEST_VOICE_ENC_TYPE_RAW = 0,
    UART_TEST_VOICE_ENC_TYPE_MSBC = 1,
    UART_TEST_VOICE_ENC_TYPE_SBC = 2,
    UART_TEST_VOICE_ENC_TYPE_ADPCM = 3,
    UART_TEST_VOICE_ENC_TYPE_ENUM_GUARD,
} UART_TEST_VOICE_ENC_TYPE;

typedef enum
{
    UART_TEST_CODEC_SAMPLE_RATE_16K = 0,
    UART_TEST_CODEC_SAMPLE_RATE_8K = 1,
    UART_TEST_CODEC_SAMPLE_RATE_ENUM_GUARD,
} UART_TEST_CODEC_SAMPLE_RATE;

#if SUPPORT_CODEC_EQ_CONFIG_FEATURE
typedef enum
{
    UART_TEST_CODEC_EQ_STATUS_DISABLE = 0,
    UART_TEST_CODEC_EQ_STATUS_ENABLE = 1,
    UART_TEST_CODEC_EQ_STATUS_ENUM_GUARD,
} UART_TEST_CODEC_EQ_STATUS;
#endif

extern const T_UART_TEST_PROTOCOL uart_test_func_map[UART_TEST_SUPPORT_NUM];

void uart_test_init(void);
bool uart_test_check_dlps_allowed(void);
void uart_test_handle_uart_msg(T_IO_MSG io_driver_msg_recv);
void uart_test_handle_gdma_msg(T_IO_MSG io_driver_msg_recv);

#ifdef __cplusplus
}
#endif

#endif /*__RCU_UART_TEST_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

