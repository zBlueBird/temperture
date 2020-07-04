/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      simple_ble_peripheral_application.h
* @brief     simple_ble_peripheral_application
* @details   simple_ble_peripheral_application
* @author    jane
* @date      2015-12-22
* @version   v0.1
* *********************************************************************************************************
*/

#ifndef _PERIPHERAL_APPLICATION__
#define _PERIPHERAL_APPLICATION__

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <board.h>
#include <app_msg.h>
#include <gap_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include <test_mode.h>
#if FEATURE_SUPPORT_PRIVACY
#include "privacy_mgnt.h"
#endif

/*============================================================================*
 *                         Macros
 *============================================================================*/
/* @note:
*   keyscan debounce time should be smaller than RCU_CONNECT_INTERVAL*1.25ms, in case
*   keyscan can not be used due to keycan initialization when system enter dlps frequently,
*   you can change keyscan debunce time in function keyscan_init_driver() and keyscan_enter_dlps_config().
*/
#define RCU_CONNECT_INTERVAL               0x10
#define RCU_CONNECT_LATENCY                119
#define RCU_SUPERVISION_TIMEOUT            14400

#define MAX_DIRECT_ADV_RETRY_CNT 3  /* max retry count for direct adv */
#define MAX_ONOFF_LATENCY_SEND_RETRY_CNT 3  /* max retry count for call le_disable_slave_latency */
#define MAX_ONOFF_LATENCY_FAILED_RETRY_CNT 5  /* max retry count for le_disable_slave_latency failure case */
#define MAX_UPDATE_CONN_PARAMS_RETRY_CNT 3  /* max retry count for connection parameters update */
#define MAX_PAIR_FAILED_RETRY_CNT 3  /* max retry count for pair failed scenario */

#define NEW_SET_LATENCY_FUNC  /* compile error workaround, todo: refactor rcu_dfu_service */

typedef enum
{
    ADV_IDLE = 0,
    ADV_DIRECT_HDC,
    ADV_UNDIRECT_RECONNECT,
    ADV_UNDIRECT_PAIRING,
    ADV_UNDIRECT_PROMPT,
    ADV_UNDIRECT_POWER,
} T_ADV_TYPE;

typedef enum
{
    STOP_ADV_REASON_IDLE = 0,
    STOP_ADV_REASON_PAIRING,
    STOP_ADV_REASON_TIMEOUT,
    STOP_ADV_REASON_POWERKEY,
    STOP_ADV_REASON_IRLEARN,
    STOP_ADV_REASON_LOWPOWER,
    STOP_ADV_REASON_UART_CMD,
} T_STOP_ADV_REASON;

typedef enum
{
    DISCONN_REASON_IDLE = 0,
    DISCONN_REASON_PAIRING,
    DISCONN_REASON_TIMEOUT,
    DISCONN_REASON_OTA,
    DISCONN_REASON_PAIR_FAILED,
    DISCONN_REASON_LOW_POWER,
    DISCONN_REASON_UART_CMD,
    DISCONN_REASON_SILENT_OTA,
} T_DISCONN_REASON;

typedef enum
{
    RCU_STATUS_IDLE = 0,  /* RCU idle status */
    RCU_STATUS_ADVERTISING,  /* RCU adversting status */
    RCU_STATUS_STOP_ADVERTISING,  /* temporary status of stop adversting */
    RCU_STATUS_CONNECTED,  /* connect status but not start paring */
    RCU_STATUS_PAIRED,  /* rcu paired success status */
    RCU_STATUS_DISCONNECTING,  /* temporary status of disconnecting */
    RCU_STATUS_LOW_POWER,     /*rcu low power mode*/
} T_RCU_STATUS;

/**
 * @brief  APP latency state enum definition.
 */
typedef enum
{
    LANTENCY_ON = 0,
    LANTENCY_OFF,
} T_LATENCY_STATE;

typedef enum
{
    LATENCY_SYS_UPDATE_BIT    = 0x00,
    LATENCY_PAIR_PROC_BIT     = 0x01,
    LATENCY_VOICE_PROC_BIT    = 0x02,
    LATENCY_DFU_PROC_BIT      = 0x03,
    LATENCY_MAX_BIT_NUM       = (4),
} T_APP_LATENCY_MASK;

/**
 * @brief  APP latency status struct definition.
 */
typedef struct
{
    bool is_updating;
    bool is_pending;
    uint8_t retry_cnt;
    T_APP_LATENCY_MASK pending_module;
    T_LATENCY_STATE cur_status;
    T_LATENCY_STATE pending_status;
    uint16_t latency_bits_map;
} T_latency_status;

/**
 * @brief  APP battery status status definition.
 */
typedef struct
{
    uint16_t voltage_value;
    uint16_t level_value;
} T_BATTERY_STATUS;



/**
 * @brief  APP global data struct definition.
 */
typedef struct
{
    bool is_link_key_existed;  /* to indicate whether link key is existed or not */
    bool is_keyboard_notify_en;  /* to indicate whether keyboard notification is enabled or not */
    bool is_voice_notify_en;  /* to indicate whether voice notification is enabled or not */
    bool is_mm_keyboard_notify_en;  /* to indicate whether multimediea keyboard notification is enabled or not */
    bool is_adc_efuse_existed;  /* to indicate whether adc efuse data is right or not */
    uint8_t direct_adv_cnt;  /* to indicate the count of high duty adv */
    uint8_t updt_conn_params_retry_cnt;  /* to indicate the retry count for connection parameter update */
    uint8_t mtu_size;  /* to indicate the current mtu size */
    uint8_t pair_failed_retry_cnt;  /* to indicate the retry cnt for pair failed scenario */
    T_SERVER_ID bas_srv_id;  /* to indicate the service id for BAS */
    T_SERVER_ID dis_srv_id;  /* to indicate the service id for DIS */
    T_SERVER_ID hid_srv_id;  /* to indicate the service id for HID */
    T_SERVER_ID vendor_srv_id;  /* to indicate the service id for vendor service */
    T_SERVER_ID ota_srv_id;  /* to indicate the service id for OTA service */
    T_SERVER_ID dfu_srv_id;  /* to indicate the service id for DFU service */
    T_SERVER_ID atvv_srv_id;  /* to indicate the service id for ATVV service */
    T_SERVER_ID voice_srv_id;  /* to indicate the service id for voice service */
    T_RCU_STATUS rcu_status;  /* to indicate the current status of rcu state machine */
    T_GAP_DEV_STATE gap_dev_state;  /* to indicate the current GAP device state */
    T_GAP_CONN_STATE gap_conn_state;  /* to indicate the current GAP connection state */
    T_ADV_TYPE adv_type;  /* to indicate the current advertising type */
    T_STOP_ADV_REASON stop_adv_reason;  /* to indicate the reason of stop advertising */
    T_DISCONN_REASON disconn_reason;  /* to indicate the reason of disconnec  */
    T_BATTERY_STATUS battery_status;  /* to indicate the status of battery information */
    T_latency_status latency_status;  /* to indicate rcu app latency status */
    T_TEST_MODE test_mode;  /* to indicate current test mode */
    uint16_t conn_interval;  /* to indicate rcu connect interval */
    uint16_t conn_latency;   /* to indicate rcu connect latency */
    uint16_t conn_supervision_timeout; /* to indicate rcu supervision time out */
} T_APP_GLOBAL_DATA;

extern T_APP_GLOBAL_DATA app_global_data;
#if FEATURE_SUPPORT_PRIVACY
extern T_PRIVACY_STATE app_privacy_state;
extern T_PRIVACY_ADDR_RESOLUTION_STATE app_privacy_resolution_state;
#endif

void         app_handle_io_msg(T_IO_MSG io_driver_msg_recv);
T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data);
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);
void app_set_latency_status(T_APP_LATENCY_MASK index_bit, T_LATENCY_STATE new_state);
void app_init_global_data(void);
#if FEATURE_SUPPORT_PRIVACY
void app_privacy_callback(T_PRIVACY_CB_TYPE type, T_PRIVACY_CB_DATA cb_data);
#endif

#ifdef __cplusplus
}
#endif

#endif

