/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     key_handle.c
* @brief    This is the entry of user code which the key handle module resides in.
* @details
* @author   chenjie jin
* @date     2018-05-03
* @version  v1.1
*********************************************************************************************************
*/

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <string.h>
#include "board.h"
#include "key_handle.h"
#include <trace.h>
#include "profile_server.h"
#include "central_app.h"
//#include "rcu_application.h"
#include "os_timer.h"
//#include "swtimer.h"
#include "voice_m2s.h"
//#include "voice_driver.h"
//#include "rcu_gap.h"
#include "gap_conn_le.h"
#include "gap_storage_le.h"
#include "rtl876x_wdg.h"
#include "rtl876x_keyscan.h"
#include "gap_bond_le.h"
#include "app_section.h"
#if IR_FUN
#include "ir_app_config.h"
#endif
#include "led_driver.h"
#if FEATURE_SUPPORT_MP_TEST_MODE
#include "mp_test.h"
#endif
#include "audio_handle.h"

#if KEYSCAN_EN

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
/* Key Mapping Table Definiton */
DATA_RAM_FUNCTION static const T_KEY_INDEX_DEF
KEY_MAPPING_TABLE[KEYPAD_ROW_SIZE][KEYPAD_COLUMN_SIZE] =
{
    {VK_POWER,      VK_HOME,       VK_LEFT,         VK_EXIT},
    {VK_NC,         VK_UP,         VK_ENTER,        VK_DOWN},
    {VK_RIGHT,      VK_MENU,       VK_NC,           VK_NC},
    {VK_VOICE,      VK_NC,         VK_VOLUME_UP,    VK_VOLUME_DOWN},
    {VK_PAGE_UP,    VK_PAGE_DOWN,  VK_NC,           VK_NC},
};

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
/* BLE HID code table definition */
DATA_RAM_FUNCTION const uint16_t BLE_KEY_CODE_TABLE[BLE_KEY_CODE_TABLE_SIZE] = {0x00,  /* VK_NC */
                                                                                0x66,  /* VK_POWER */
                                                                                0x4B,  /* VK_PAGE_UP */
                                                                                0x4E,  /* VK_PAGE_DOWN */
                                                                                0x76,  /* VK_MENU */
                                                                                0x4A,  /* VK_HOME */
                                                                                0x3E,  /* VK_VOICE */
                                                                                0x28,  /* VK_ENTER */
                                                                                0x29,  /* VK_EXIT */
                                                                                0x50,  /* VK_LEFT */
                                                                                0x4F,  /* VK_RIGHT */
                                                                                0x52,  /* VK_UP */
                                                                                0x51,  /* VK_DOWN */
                                                                                0x00,  /* VK_MOUSE_EN */
                                                                                0x7F,  /* VK_VOLUME_MUTE */
                                                                                0x80,  /* VK_VOLUME_UP */
                                                                                0x81,  /* VK_VOLUME_DOWN */
                                                                                0x3F,  /* VK_VOICE_STOP */
                                                                                0x42,  /* VK_HP_ATTACHED */
                                                                                0x43,  /* VK_HP_DETACHED */
#if FEATURE_SUPPORT_MULTIMEDIA_KEYBOARD
                                                                                0x0101,  /* MM_ScanNext */
                                                                                0x0102,  /* MM_ScanPrevious */
                                                                                0x0104,  /* MM_Stop */
                                                                                0x0108,  /* MM_Play_Pause */
                                                                                0x0110,  /* MM_Mute */
                                                                                0x0120,  /* MM_BassBoost */
                                                                                0x0140,  /* MM_Loudness */
                                                                                0x0180,  /* MM_VolumeIncrement */
                                                                                0x0201,  /* MM_VolumeDecrement */
                                                                                0x0202,  /* MM_BassIncrement */
                                                                                0x0204,  /* MM_BassDecrement */
                                                                                0x0208,  /* MM_TrebleIncrement */
                                                                                0x0210,  /* MM_TrebleDecrement */
                                                                                0x0220,  /* MM_AL_ConsumerControl */
                                                                                0x0240,  /* MM_AL_EmailReader */
                                                                                0x0280,  /* MM_AL_Calculator */
                                                                                0x0301,  /* MM_AL_LocalMachineBrowser */
                                                                                0x0302,  /* MM_AC_Search */
                                                                                0x0304,  /* MM_AC_Home */
                                                                                0x0308,  /* MM_AC_Back */
                                                                                0x0310,  /* MM_AC_Forward */
                                                                                0x0320,  /* MM_AC_Stop */
                                                                                0x0340,  /* MM_AC_Refresh */
                                                                                0x0380,  /* MM_AC_Bookmarks */
#endif
                                                                               };

T_KEY_HANDLE_GLOBAL_DATA key_handle_global_data;  /* Value to indicate the reconnection key data */

/*============================================================================*
 *                              Local Functions
 *============================================================================*/


/**
 * @brief handle one key pressed scenario
 * @param key_index - pressed key index
 * @return none
 * @retval void
 */
static void key_handle_one_key_scenario(T_KEY_INDEX_DEF key_index)
{
    if (app_global_data.device_ble_status == DEVICE_STATUS_PAIRED)
    {
        if (key_index == VK_VOICE)
        {
#if RCU_VOICE_EN
            voice_handle_mic_key_pressed();
#endif
        }
        else
        {
            key_handle_write_key_data(key_index);
        }
    }

    key_handle_global_data.last_pressed_key_index = key_index;
}


/*============================================================================*
 *                              Global Functions
 *============================================================================*/


/**
* @brief   handle key pressed event
* @param   p_keyscan_fifo_data - point of keyscan FIFO data
* @return  void
*/
void key_handle_pressed_event(T_KEYSCAN_FIFIO_DATA *p_keyscan_fifo_data)
{
    APP_PRINT_INFO1("[key_handle_pressed_event] keyscan FIFO length is %d", p_keyscan_fifo_data->len);

    for (uint8_t index = 0; index < (p_keyscan_fifo_data->len); index++)
    {
        APP_PRINT_INFO4("[key_handle_pressed_event] keyscan data[%d]: row - %d, column - %d, value - %d", \
                        index, p_keyscan_fifo_data->key[index].row, p_keyscan_fifo_data->key[index].column,
                        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[index].row][p_keyscan_fifo_data->key[index].column]);
    }

    T_KEY_INDEX_DEF key_index_1 =
        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[0].row][p_keyscan_fifo_data->key[0].column];
    T_KEY_INDEX_DEF key_index_2 =
        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[1].row][p_keyscan_fifo_data->key[1].column];

    LED_ON(LED_1);

    if (p_keyscan_fifo_data->len == 0)
    {
        APP_PRINT_WARN0("[key_handle_pressed_event] FIFO length is 0!");
    }
    else if (p_keyscan_fifo_data->len == 1)
    {
        key_handle_one_key_scenario(key_index_1);
    }
    else if (p_keyscan_fifo_data->len == 2)
    {
        //key_handle_two_keys_scenario(key_index_1, key_index_2);
        key_index_2 = (T_KEY_INDEX_DEF) key_index_2;
    }
    else
    {
        /* more than two keys are pressed, just ignore this scenario.
           If need to use three or more keys as combined keys, need
           to caution ghost keys!
        */
    }
}

/**
* @brief   handle key release event
* @param   none
* @return  void
*/
void key_handle_release_event(void)
{
    APP_PRINT_WARN2("[key_handle_release_event] key release, app_global_data.device_ble_status = %d, key_handle_global_data.last_pressed_key_index = %d!",
                    app_global_data.device_ble_status, key_handle_global_data.last_pressed_key_index);

    LED_OFF(LED_1);

    if (app_global_data.device_ble_status == DEVICE_STATUS_PAIRED)
    {
        if (key_handle_global_data.last_pressed_key_index == VK_VOICE)
        {
#if RCU_VOICE_EN
            voice_handle_mic_key_released();
#endif
        }
        else
        {
            key_handle_write_key_data(VK_NC);
        }
    }
}

/**
* @brief   notify key data
* @param   key_index - index of key
* @return  true or false
*/
bool key_handle_write_key_data(T_KEY_INDEX_DEF key_index)
{
    bool result = false;
    uint16_t key_data = BLE_KEY_CODE_TABLE[key_index];
    uint16_t pre_key_data = BLE_KEY_CODE_TABLE[key_handle_global_data.last_pressed_key_index];

    APP_PRINT_INFO3("[key_handle_write_key_data] key_index %d, key_code %d, pre_key_data %d",
                    key_index, key_data, pre_key_data);

    if (((key_data != VK_NC) && (key_data < BLE_MM_START_KEY_CODE))
        || ((key_data == VK_NC) && (pre_key_data < BLE_MM_START_KEY_CODE)))
    {
        /* normal key code */
        if (true)//app_global_data.is_keyboard_notify_en)
        {
            uint8_t notfiy_key_data[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
            notfiy_key_data[2] = (uint8_t) key_data;

            if (client_attr_write(0, voice_client_id, GATT_WRITE_TYPE_CMD, VOICE_CTRL_CHAR_HANDLE,
                                  8, notfiy_key_data) == GAP_CAUSE_SUCCESS)
            {
                APP_PRINT_WARN0("[key_handle_write_key_data] client_attr_write success!");
            }
            else
            {
                APP_PRINT_WARN0("[key_handle_write_key_data] client_attr_write failed!");
            }
        }
        else
        {
            APP_PRINT_WARN0("[key_handle_write_key_data] is_keyboard_notify_en is disbaled!");
            result = false;
        }
    }


    return result;
}
#endif
