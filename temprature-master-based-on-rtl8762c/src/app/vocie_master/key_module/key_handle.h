/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2018 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#ifndef _KEY_HANDLE_H_
#define _KEY_HANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <keyscan_driver.h>

/*============================================================================*
 *                              Macro Definitions
 *============================================================================*/
/* define the bit mask of combine keys */
#define INVALID_COMBINE_KEYS_BIT_MASK               0x0000
#define PAIRING_COMBINE_KEYS_BIT_MASK               0x0001
#define IR_LEARNING_COMBINE_KEYS_BIT_MASK           0x0002
#define HCI_UART_TEST_COMBINE_KEYS_BIT_MASK         0x0004
#define DATA_UART_TEST_COMBINE_KEYS_BIT_MASK        0x0008
#define SINGLE_TONE_TEST_COMBINE_KEYS_BIT_MASK      0x0010
#define FAST_PAIR_1_COMBINE_KEYS_BIT_MASK           0x0020
#define FAST_PAIR_2_COMBINE_KEYS_BIT_MASK           0x0040
#define FAST_PAIR_3_COMBINE_KEYS_BIT_MASK           0x0080
#define FAST_PAIR_4_COMBINE_KEYS_BIT_MASK           0x0100
#define FAST_PAIR_5_COMBINE_KEYS_BIT_MASK           0x0200
#define CODEC_TEST_COMBINE_KEYS_BIT_MASK            0x0400

#define COMBINE_KEYS_DETECT_TIMEOUT 2000  /* 2 sec */
#define NOTIFY_KEY_DATA_TIMEOUT 300  /* 300 ms */

/*============================================================================*
 *                         Types
 *============================================================================*/
typedef enum _RECON_KEY_STATUS
{
    RECON_KEY_IDLE = 0,
    RECON_KEY_PRESS = 1,
    RECON_KEY_REALSE_IMM = 2,
    RECON_KEY_SEND_REALSE_BY_TIM = 3,
    RECON_KEY_SEND_REALSE_NORMAL = 4,
} RECON_KEY_STATUS;

typedef enum
{
    VK_NC            = 0x00,
    VK_POWER         = 0x01,
    VK_PAGE_UP       = 0x02,
    VK_PAGE_DOWN     = 0x03,
    VK_MENU          = 0x04,
    VK_HOME          = 0x05,
    VK_VOICE         = 0x06,
    VK_ENTER         = 0x07,
    VK_EXIT          = 0x08,
    VK_LEFT          = 0x09,
    VK_RIGHT         = 0x0A,
    VK_UP            = 0x0B,
    VK_DOWN          = 0x0C,
    VK_MOUSE_EN      = 0x0D,
    VK_VOLUME_MUTE   = 0x0E,
    VK_VOLUME_UP     = 0x0F,
    VK_VOLUME_DOWN   = 0x10,
    VK_VOICE_STOP    = 0x11,
    VK_HP_ATTACHED   = 0x12,
    VK_HP_DETACHED   = 0x13,

#if FEATURE_SUPPORT_MULTIMEDIA_KEYBOARD
    MM_ScanNext                = 0x14,
    MM_ScanPrevious,
    MM_Stop,
    MM_Play_Pause,
    MM_Mute,
    MM_BassBoost,
    MM_Loudness,
    MM_VolumeIncrement,
    MM_VolumeDecrement,
    MM_BassIncrement,
    MM_BassDecrement,
    MM_TrebleIncrement,
    MM_TrebleDecrement,
    MM_AL_ConsumerControl,
    MM_AL_EmailReader,
    MM_AL_Calculator,
    MM_AL_LocalMachineBrowser,
    MM_AC_Search,
    MM_AC_Home,
    MM_AC_Back,
    MM_AC_Forward,
    MM_AC_Stop,
    MM_AC_Refresh,
    MM_AC_Bookmarks,
#endif
    KEY_INDEX_ENUM_GUAID
} T_KEY_INDEX_DEF;

/* define the key code table size, the value should modify according to BLE_KEY_CODE_TABLE */
#define BLE_KEY_CODE_TABLE_SIZE KEY_INDEX_ENUM_GUAID

/* define the start key code for multimedia keys */
#define BLE_MM_START_KEY_CODE 0x0100

/* Key global parameters' struct */
typedef struct
{
    T_KEY_INDEX_DEF reconn_key_index;  /* to indicate the reconnection key index */
    T_KEY_INDEX_DEF last_pressed_key_index;  /* to indicate the last pressed key index */
    uint32_t combine_keys_status;  /* to indicate the status of combined keys */
} T_KEY_HANDLE_GLOBAL_DATA;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern const uint16_t BLE_KEY_CODE_TABLE[BLE_KEY_CODE_TABLE_SIZE];
extern T_KEY_HANDLE_GLOBAL_DATA key_handle_global_data;
extern TimerHandle_t combine_keys_detection_timer;
extern TimerHandle_t notify_key_data_after_reconn_timer;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void key_handle_init_data(void);
void key_handle_pressed_event(T_KEYSCAN_FIFIO_DATA *pKey_Data);
void key_handle_release_event(void);
bool key_handle_write_key_data(T_KEY_INDEX_DEF key_index);
void key_handle_init_timer(void);

#ifdef __cplusplus
}
#endif

#endif
