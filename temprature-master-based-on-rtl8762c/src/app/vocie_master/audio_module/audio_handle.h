/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      audio_handle.h
* @brief    header file of audio handle APIs.
* @details
* @author    yuyin_zhang
* @date      2018-04-15
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __AUDIO_HANDLE_H
#define __AUDIO_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "trace.h"
#include "app_msg.h"
#include "audio_trans.h"
#include "gap.h"

/* Defines ------------------------------------------------------------------*/
typedef void *TimerHandle_t;
/**
 * @brief Enable print log or not

 */

//#define PRINT_AUDIO_HANDLE_LOG
#ifdef PRINT_AUDIO_HANDLE_LOG
#define AUDIO_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) DBG_BUFFER_##LEVEL(TYPE_BEE2, SUBTYPE_FORMAT, MODULE, fmt, para_num, ##__VA_ARGS__)
#else
#define AUDIO_DBG_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

#define ADS_CHAR_RX_LEN 20
#define ADS_CHAR_TX_LEN 255
#define ADS_CHAR_CTL_LEN 20

#define ADS_CTL_OPCODE_GET_CAPS_LEN 5
#define ADS_RX_OPCODE_GET_CAPS_RESP_LEN 7
#define ADS_CTL_OPCODE_AUDIO_START_LEN 3
#define ADS_RX_OPCODE_AUDIO_START_ERROR_LEN 3
#define ADS_RX_OPCODE_AUDIO_END_LEN 1
#define ADS_RX_OPCODE_AUDIO_PAUSE_LEN 1
#define ADS_RX_OPCODE_AUDIO_PLAY_LEN 1
#define ADS_CTL_OPCODE_AUDIO_STOP_LEN 1
#define ADS_RX_OPCODE_BUFFER_READY_LEN 9

#define ADS_VERSION_MAJOR 0x00
#define ADS_VERSION_MINOR 0x01

#define ADS_CODEC_MASK_RHQC         0x0008

#define ADS_CODECS_SUPPORT (ADS_CODEC_MASK_RHQC)

#define AUDIO_NOTIFY_BUFF_READY_TIMEOUT 1000
#define AUDIO_CODEC_POWER_ON_DELAY_TIMEOUT 500

#define FEATURE_SUPPORT_CODEC_TEST 1  /* set 1 to support audio codec test */

#if FEATURE_SUPPORT_CODEC_TEST
#define CODEC_TEST_AUDIO_FILE_BASE_ADDR 0x00829000
#define CODEC_TEST_AUDIO_FILE_MAX_LEN 0x00012750

#define CODEC_TEST_AUDIO_BUFFER_SIZE 1024
#endif

typedef enum
{
    AUDIO_INPUT_BUFF_STATE_IDLE = 0,
    AUDIO_INPUT_BUFF_STATE_READY,
} T_AUDIO_INPUT_BUFF_STATUS;

typedef enum
{
    ADS_RX_OPCODE_GET_CAPS_RESP = 0x01,
    ADS_RX_OPCODE_AUDIO_START_ERROR = 0x02,
    ADS_RX_OPCODE_AUDIO_END = 0x03,
    ADS_RX_OPCODE_BUFFER_READY = 0x04,
    ADS_RX_OPCODE_PLAY_PREV_ITEM = 0x05,
    ADS_RX_OPCODE_PLAY_NEXT_ITEM = 0x06,
    ADS_RX_OPCODE_AUDIO_PAUSE = 0x07,
    ADS_RX_OPCODE_AUDIO_PLAY = 0x08,
} ADS_CHAR_RX_OPCODE;

typedef enum
{
    ADS_CTL_OPCODE_GET_CAPS = 0x0A,
    ADS_CTL_OPCODE_AUDIO_START = 0x0B,
    ADS_CTL_OPCODE_AUDIO_STOP = 0x0C,
    ADS_CTL_OPCODE_AUDIO_VOL_UP = 0x0D,
    ADS_CTL_OPCODE_AUDIO_VOL_DOWN = 0x0E,
    ADS_CTL_OPCODE_AUDIO_MUTE = 0x0F,
    ADS_CTL_OPCODE_AUDIO_UNMUTE = 0x10,
} ADS_CHAR_CTL_CMD_OPCODE;

typedef enum
{
    ADS_INVALID_CODEC = 0x0F01,
} ADS_ERROR_CODE;

/** @} End of ADS_Exported_Types */

typedef struct _T_ADS_GLOBAL_DATA
{
    uint8_t master_version_major;
    uint8_t master_version_minor;
    uint16_t codec_used;
    uint8_t char_rx_data_buff[ADS_CHAR_RX_LEN];
    uint8_t char_ctl_data_buff[ADS_CHAR_CTL_LEN];
    uint8_t char_tx_data_buff[ADS_CHAR_TX_LEN];
} T_ADS_GLOBAL_DATA;

/**
 * @brief  AAC decoder global data struct definition.
 */
typedef struct
{
    bool is_working;  /* to indicate whether AAC decoder mudule is working or not */
    bool is_driver_initialized;
    bool is_buffer_ready_notify_en;
    bool is_codec_testing;
    uint32_t audio_frame_index;
    uint32_t total_recv_bytes;
    uint32_t except_recv_bytes;
    uint32_t bytes_per_frame_before_docode;
    uint32_t bytes_per_frame_after_docode;
    uint32_t decode_frame_cnt_for_once;
    uint32_t current_audio_test_file_index;
    T_AUDIO_FREQ sampling_freq;
    AudioTrans_TypeDef audio_trans_type;
} T_AUDIO_GLOBAL_DATA;

extern T_ADS_GLOBAL_DATA ads_global_data;
extern T_AUDIO_GLOBAL_DATA audio_global_data;
extern TimerHandle_t codec_power_on_delay_timer;
extern bool is_audio_allowed_to_enter_dlps;

void audio_handle_start(void);
void audio_handle_stop(void);
bool audio_handle_check_frame_complete(uint32_t *p_frame_len);
void audio_handle_msg(T_IO_MSG *p_io_msg);
bool audio_handle_loop_buffer_data(uint8_t free_index);
void audio_handle_init_timer(void);
void audio_handle_drop_useless_buff_data(void);
bool audio_handle_volume_up(void);
bool audio_handle_volume_down(void);
bool audio_handle_volume_mute(void);
bool audio_handle_volume_unmute(void);
void audio_handle_disable_power(void);
void audio_handle_enable_power(void);
void audio_handle_start_gdma(void);
void audio_handle_prepare_test_audio_buffer(void);
T_APP_RESULT app_handle_hids_ads_write_char_tx(uint8_t *p_data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /*__AUDIO_HANDLE_H */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

