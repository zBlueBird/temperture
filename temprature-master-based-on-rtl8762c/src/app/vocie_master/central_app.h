/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      central_app.h
   * @brief     This file handles BLE central application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _CENTRAL_APP_H_
#define _CENTRAL_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <profile_client.h>
#include <app_msg.h>

/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_CLIENT_ID   gaps_client_id;        /**< Simple ble service client id*/
extern T_CLIENT_ID   voice_client_id;  /**< gap service client id*/
extern T_CLIENT_ID   bas_client_id;         /**< battery service client id*/

typedef enum
{
    DEVICE_STATUS_IDLE = 0,  /* RCU idle status */
    DEVICE_STATUS_SCAN_STARTING,  /* RCU adversting status */
    DEVICE_STATUS_SCAN,  /* temporary status of stop adversting */
    DEVICE_STATUS_SCAN_STOPING,
    DEVICE_STATUS_SCAN_STOPED,
    DEVICE_STATUS_CONNECTED,  /* connect status but not start paring */
    DEVICE_STATUS_PAIRED,  /* rcu paired success status */
    DEVICE_STATUS_SERVICE_DISCOV_DONE,
    DEVICE_STATUS_DISCONNECTING,  /* temporary status of disconnecting */
    DEVICE_STATUS_LOW_POWER,     /*rcu low power mode*/
} T_DEVICE_STATUS;

/**
 * @brief  APP global data struct definition.
 */
typedef struct
{
    bool is_link_key_existed;  /* to indicate whether link key is existed or not */
    bool is_keyboard_notify_en;  /* to indicate whether keyboard notification is enabled or not */
    bool is_voice_notify_en;  /* to indicate whether voice notification is enabled or not */
    bool is_mm_keyboard_notify_en;  /* to indicate whether multimediea keyboard notification is enabled or not */
    bool is_scan_device_hit;  /* to indicate whether adc efuse data is right or not */
    uint8_t direct_adv_cnt;  /* to indicate the count of high duty adv */
    uint8_t updt_conn_params_retry_cnt;  /* to indicate the retry count for connection parameter update */
    uint8_t mtu_size;  /* to indicate the current mtu size */
    uint8_t pair_failed_retry_cnt;  /* to indicate the retry cnt for pair failed scenario */
    T_DEVICE_STATUS device_ble_status;
    //T_AUDIO_FREQ cur_audio_sample_freq;  /* to indicate current audio sample frequency */
    uint16_t conn_interval;  /* to indicate rcu connect interval */
    uint16_t conn_latency;   /* to indicate rcu connect latency */
    uint16_t conn_supervision_timeout; /* to indicate rcu supervision time out */
} T_APP_GLOBAL_DATA;

extern T_APP_GLOBAL_DATA app_global_data;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);

#ifdef __cplusplus
}
#endif

#endif

