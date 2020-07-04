/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     profile_init.c
* @brief    .
* @details
* @author   Ken_mei
* @date     2015-03-27
* @version  v0.2
**********************************************************************************************************/
#include "rtl876x.h"
#include "gap.h"
#include "gap_adv.h"
#include "rcu_gap.h"
#include "board.h"
#include "gap_storage_le.h"
#include "gap_conn_le.h"
#include "string.h"
#include <trace.h>
#include "mem_config.h"
#include "rcu_application.h"
#include <gap_bond_le.h>
#include "os_timer.h"
#include "swtimer.h"
#if FEATURE_SUPPORT_PRIVACY
#include "privacy_mgnt.h"
#endif

/*============================================================================*
 *                         Macros
 *============================================================================*/
/* What is the advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN 320 /* 20ms */
#define DEFAULT_ADVERTISING_INTERVAL_MAX 320 /* 30ms */

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
/* GAP - SCAN RSP data (max size = 31 bytes) */
static uint8_t app_scan_rsp_data[] =
{
    0x03,  /* length */
    0x19,  /* type="Appearance" */
    0x80, 0x01,  /* Remote Control */
};

/* Pairing advertisement data (max size = 31 bytes, though this is
 best kept short to conserve power while advertisting) */
static uint8_t app_pairing_adv_data[] =
{
    0x02,  /* length */
    0x01,  /* type="Flags" */
    0x05,  /* LE Limited Discoverable Mode, BR/EDR not supported */

    0x03,  /* length */
    0x03,  /* type="More 16-bit UUIDs available" */
    0x12, 0x18,  /* HID Service */

    0x03,  /* length */
    0x19,  /* type="Appearance" */
    0x80, 0x01,  /* Remote Control */

    C_DEVICE_NAME_LEN,  /* length */
    0x09,  /* type="Complete local name" */
    C_DEVICE_NAME,

    0x05,  /* length */
    0xFF,  /* vendor information */
    0x5D, 0x00, 0x04, 0x00,  /* vendor info */
};

/* Prompt advertisement data (max size = 31 bytes, though this is
 best kept short to conserve power while advertisting) */
static uint8_t app_prompt_adv_data[] =
{
    0x02,  /* length */
    0x01,  /* type="Flags" */
    0x04,  /* No Discoverable Mode, BR/EDR not supported */

    0x03,  /* length */
    0x03,  /* type="More 16-bit UUIDs available" */
    0x12, 0x18,  /* HID Service */

    0x03,  /* length */
    0x19,  /* type="Appearance" */
    0x80, 0x01,  /* Remote Control */

    C_DEVICE_NAME_LEN,  /* length */
    0x09,  /* type="Complete local name" */
    C_DEVICE_NAME,

    0x05,  /* length */
    0xFF,  /* vendor information */
    0x5D, 0x00, 0x04, 0x00,  /* vendor info */
};

/* GAP - Advertisement data (max size = 31 bytes, though this is
 best kept short to conserve power while advertisting) */
static uint8_t app_power_adv_data[] =
{
    0x02,  /* length */
    0x01,  /* type="Flags" */
    0x04,  /* BR/EDR not supported */

    0x03,  /* length     */
    0x03,  /* type="More 16-bit UUIDs available" */
    0x12, 0x18,    /* HID Service */

    0x03,  /* length     */
    0x19,  /* type="Appearance" */
    0x80, 0x01,  /* Remote Control */

    0x0D,  /* length     */
    0xFF,  /* vendor information */
    0x5D, 0x00,
    0x03, 0x00,
    0x01,
    0x01,  /* index */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* master BD address */
};

/******************************************************************
 * @fn          Initial gap parameters
 * @brief      Initialize peripheral and gap bond manager related parameters
 *
 * @return     void
 */
void rcu_le_gap_init(void)
{
    uint8_t  device_name[GAP_DEVICE_NAME_LEN] = {C_DEVICE_NAME};
    uint16_t appearance = GAP_GATT_APPEARANCE_GENERIC_REMOTE_CONTROL;

    //advertising parameters
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_int_max = DEFAULT_ADVERTISING_INTERVAL_MIN;

    //GAP Bond Manager parameters
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t  auth_sec_req_enalbe = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  slave_init_mtu_req = true;
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);

    //Register gap callback
    le_register_app_cb(app_gap_callback);

#if FEATURE_SUPPORT_PRIVACY
    privacy_init(app_privacy_callback, true);
#endif

    //Set device name and device appearance
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);

    //Set advertising parameters
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(app_pairing_adv_data), app_pairing_adv_data);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(app_scan_rsp_data), app_scan_rsp_data);

    // Setup the GAP Bond Manager
    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enalbe), &auth_sec_req_enalbe);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);

}

/**
 * @brief start advertising
 * @param unsigned char flg - indicate to send which type of adv
 * @return true or false
 * @retval void
 */
bool rcu_start_adv(T_ADV_TYPE adv_type)
{
    bool result = false;
    uint8_t adv_event_type;
    uint8_t adv_channel_map;
    uint8_t adv_filter_policy;
    uint16_t adv_interval_min;
    uint16_t adv_interval_max;
    T_LE_KEY_ENTRY *p_le_key_entry = le_get_high_priority_bond();

    /* check current status, only allow to start adv if RCU_STATUS_IDLE */
    if (app_global_data.rcu_status != RCU_STATUS_IDLE)
    {
        APP_PRINT_WARN1("rcu_status %d is not allowed to start adv again!", app_global_data.rcu_status);
        return false;
    }

    switch (adv_type)
    {
    case ADV_DIRECT_HDC:
        {
            adv_event_type = GAP_ADTYPE_ADV_HDC_DIRECT_IND;
            adv_filter_policy = GAP_ADV_FILTER_ANY;
            adv_channel_map = GAP_ADVCHAN_ALL;

            le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_event_type), &adv_event_type);
            le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(p_le_key_entry->remote_bd.remote_bd_type),
                             &(p_le_key_entry->remote_bd.remote_bd_type));
            le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(p_le_key_entry->remote_bd.addr),
                             p_le_key_entry->remote_bd.addr);
            le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
            le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_channel_map), &adv_channel_map);

            if (GAP_CAUSE_SUCCESS == le_adv_start())
            {
                APP_PRINT_INFO0("rcu start ADV_DIRECT_HDC success!");
                app_global_data.rcu_status = RCU_STATUS_ADVERTISING;
                app_global_data.adv_type = ADV_DIRECT_HDC;
                result = true;
            }
            else
            {
                APP_PRINT_WARN0("rcu start ADV_DIRECT_HDC fail!");
            }
        }
        break;

    case ADV_UNDIRECT_RECONNECT:
        {
#if FEATURE_SUPPORT_PRIVACY
            if (app_privacy_resolution_state == PRIVACY_ADDR_RESOLUTION_DISABLED)
            {

                privacy_set_addr_resolution(true);
            }
#endif
            adv_event_type = GAP_ADTYPE_ADV_IND;
            adv_channel_map = GAP_ADVCHAN_ALL;
            adv_filter_policy = GAP_ADV_FILTER_WHITE_LIST_ALL;
            adv_interval_min = 0x20;  /* 20ms */
            adv_interval_max = 0x20;  /* 20ms */

            le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_event_type), &adv_event_type);
            le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_channel_map), &adv_channel_map);
            le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
            le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_interval_min), &adv_interval_min);
            le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_interval_max), &adv_interval_max);
            le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(app_pairing_adv_data), app_pairing_adv_data);
            le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(app_scan_rsp_data), app_scan_rsp_data);

            if (GAP_CAUSE_SUCCESS == le_adv_start())
            {
                APP_PRINT_INFO0("rcu start ADV_UNDIRECT_RECONNECT success!");
                app_global_data.rcu_status = RCU_STATUS_ADVERTISING;
                app_global_data.adv_type = ADV_UNDIRECT_RECONNECT;
                os_timer_restart(&adv_timer, ADV_UNDIRECT_RECONNECT_TIMEOUT);
                result = true;
            }
            else
            {
                APP_PRINT_WARN0("rcu start ADV_UNDIRECT_RECONNECT fail!");
            }
        }
        break;

    case ADV_UNDIRECT_POWER:
        {
            PROFILE_PRINT_INFO0("ADV_UNDIRECT_POWER for power on master!");
#if FEATURE_SUPPORT_PRIVACY
            if (app_privacy_resolution_state == PRIVACY_ADDR_RESOLUTION_DISABLED)
            {

                privacy_set_addr_resolution(true);
            }
#endif
            adv_event_type = GAP_ADTYPE_ADV_IND;
            adv_channel_map = GAP_ADVCHAN_ALL;
            adv_filter_policy = GAP_ADV_FILTER_WHITE_LIST_ALL;
            adv_interval_min = 0x20;  /* 20ms */
            adv_interval_max = 0x30;  /* 30ms */
            for (int i = 0; i < 6; i++)
            {
                app_power_adv_data[sizeof(app_power_adv_data) - 6 + i] = p_le_key_entry->remote_bd.addr[i];
            }

            le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_event_type), &adv_event_type);
            le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_channel_map), &adv_channel_map);
            le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
            le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_interval_min), &adv_interval_min);
            le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_interval_max), &adv_interval_max);
            le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(app_power_adv_data), app_power_adv_data);
            le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(app_scan_rsp_data), app_scan_rsp_data);

            if (GAP_CAUSE_SUCCESS == le_adv_start())
            {
                APP_PRINT_INFO0("rcu start ADV_UNDIRECT_POWER success!");
                app_global_data.rcu_status = RCU_STATUS_ADVERTISING;
                app_global_data.adv_type = ADV_UNDIRECT_POWER;
                os_timer_restart(&adv_timer, ADV_UNDIRECT_POWER_TIMEOUT);
                result = true;
            }
            else
            {
                APP_PRINT_WARN0("rcu start ADV_UNDIRECT_POWER fail!");
            }
        }
        break;

    case ADV_UNDIRECT_PAIRING:
        {
            PROFILE_PRINT_INFO0("ADV_UNDIRECT_PAIRING for pairing!");
#if FEATURE_SUPPORT_PRIVACY
            if (app_privacy_resolution_state == PRIVACY_ADDR_RESOLUTION_ENABLED)
            {
                privacy_set_addr_resolution(false);
            }
#endif
            adv_event_type = GAP_ADTYPE_ADV_IND;
            adv_channel_map = GAP_ADVCHAN_ALL;
            adv_filter_policy = GAP_ADV_FILTER_ANY;
            adv_interval_min = 0x640;  /* 1000ms */
            adv_interval_max = 0x640;  /* 1000ms */

            le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_event_type), &adv_event_type);
            le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_channel_map), &adv_channel_map);
            le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
            le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_interval_min), &adv_interval_min);
            le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_interval_max), &adv_interval_max);
            le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(app_pairing_adv_data), app_pairing_adv_data);
            le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(app_scan_rsp_data), app_scan_rsp_data);

            if (GAP_CAUSE_SUCCESS == le_adv_start())
            {
                APP_PRINT_INFO0("rcu start ADV_UNDIRECT_PAIRING success!");
                app_global_data.rcu_status = RCU_STATUS_ADVERTISING;
                app_global_data.adv_type = ADV_UNDIRECT_PAIRING;
                os_timer_restart(&adv_timer, ADV_UNDIRECT_PAIRING_TIMEOUT);
                result = true;
            }
            else
            {
                APP_PRINT_WARN0("rcu start ADV_UNDIRECT_PAIRING fail!");
            }
        }
        break;

    case ADV_UNDIRECT_PROMPT:
        PROFILE_PRINT_INFO0("ADV_UNDIRECT_PROMPT for prompt!");
#if FEATURE_SUPPORT_PRIVACY
        if (app_privacy_resolution_state == PRIVACY_ADDR_RESOLUTION_DISABLED)
        {

            privacy_set_addr_resolution(true);
        }
#endif
        adv_event_type = GAP_ADTYPE_ADV_NONCONN_IND;
        adv_channel_map = GAP_ADVCHAN_ALL;
        adv_filter_policy = GAP_ADV_FILTER_WHITE_LIST_ALL;
        adv_interval_min = 0x20;
        adv_interval_max = 0x30;

        le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_event_type), &adv_event_type);
        le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_channel_map), &adv_channel_map);
        le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
        le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_interval_min), &adv_interval_min);
        le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_interval_max), &adv_interval_max);
        le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(app_prompt_adv_data), app_prompt_adv_data);
        le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA,  sizeof(app_scan_rsp_data), app_scan_rsp_data);

        if (GAP_CAUSE_SUCCESS == le_adv_start())
        {
            APP_PRINT_INFO0("rcu start ADV_UNDIRECT_PROMPT success!");
            app_global_data.rcu_status = RCU_STATUS_ADVERTISING;
            app_global_data.adv_type = ADV_UNDIRECT_PROMPT;
            os_timer_restart(&adv_timer, ADV_UNDIRECT_PROMPT_TIMEOUT);
            result = true;
        }
        else
        {
            APP_PRINT_WARN0("rcu start ADV_UNDIRECT_PROMPT fail!");
        }
        break;

    default:
        /* invalid adv type */
        APP_PRINT_WARN1("[rcu_start_adv] unknown adv type: %d", adv_type);
        break;
    }

    return result;
}

/**
 * @brief stop advertising
 * @param stop_adv_reason - indicate to the reason to stop adv
 * @return true or false
 * @retval void
 */
bool rcu_stop_adv(T_STOP_ADV_REASON stop_adv_reason)
{
    bool result = false;
    app_global_data.stop_adv_reason = stop_adv_reason;

    if (app_global_data.rcu_status == RCU_STATUS_ADVERTISING)
    {
        if (le_adv_stop() == GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_INFO0("[rcu_stop_adv] stop adv command sent successfully");
            app_global_data.rcu_status = RCU_STATUS_STOP_ADVERTISING;
            result = true;
        }
        else
        {
            APP_PRINT_WARN0("[rcu_stop_adv] stop adv command sent failed");
        }
    }
    else
    {
        APP_PRINT_WARN1("[rcu_stop_adv] Invalid RCU status: %d", app_global_data.rcu_status);
    }

    return result;
}

/**
 * @brief terminate connection
 */
bool rcu_terminate_connection(T_DISCONN_REASON disconn_reason)
{
    bool result = false;
    app_global_data.disconn_reason = disconn_reason;

    if ((app_global_data.rcu_status == RCU_STATUS_CONNECTED)
        || (app_global_data.rcu_status == RCU_STATUS_PAIRED))
    {
        if (le_disconnect(0) == GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_INFO0("[rcu_terminate_connection] terminate command sent successfully");
            result = true;
        }
        else
        {
            APP_PRINT_WARN0("[rcu_terminate_connection] terminate command sent failed");
        }
    }
    else
    {
        APP_PRINT_WARN1("[rcu_terminate_connection] Invalid RCU status: %d", app_global_data.rcu_status);
    }

    return result;

}



/**
 * @brief update peripheral connection parameters
 * @param interval - connection interval; latency - peripheral connection latency
 * @return none
 */
void rcu_update_conn_params(uint16_t interval, uint16_t latency, uint16_t timeout)
{
    app_global_data.updt_conn_params_retry_cnt = 0;  /* reset retry cnt */

    if (GAP_CAUSE_SUCCESS != le_update_conn_param(0, interval, interval, latency, timeout / 10,
                                                  interval * 2 - 2,
                                                  interval * 2 - 2))
    {
        if (app_global_data.updt_conn_params_retry_cnt < MAX_UPDATE_CONN_PARAMS_RETRY_CNT)
        {
            app_global_data.updt_conn_params_retry_cnt++;
            APP_PRINT_WARN0("[rcu_update_conn_params] send HCI command retry");
            os_timer_restart(&update_conn_params_timer, UPDATE_CONN_PARAMS_TIMEOUT);
        }
        else
        {
            APP_PRINT_WARN0("[rcu_update_conn_params] send HCI command failed");
        }
    }
}
