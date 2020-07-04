/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rcu_application.c
* @brief     rcu application implementation
* @details   rcu application implementation
* @author    ken_mei
* @date      2017-07-03
* @version   v0.1
* *********************************************************************************************************
*/
#include <trace.h>
#include <string.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include "rcu_gap.h"
#include <profile_server.h>
#include <gap_msg.h>
#include "board.h"
#include <app_msg.h>
#include <rcu_application.h>
#include "hids_rmc.h"
#include "bas.h"
#include "dis.h"
#include "os_timer.h"
#include "swtimer.h"
#include "gap_conn_le.h"
#include "vendor_service.h"
#include "rcu_ota_service.h"
#include "rtl876x_wdg.h"
#include "dfu_api.h"
#include "hids_rmc.h"
//#include "rcu_dfu_service.h"
#include "patch_header_check.h"

#include "gatt_builtin_services.h"
#include "battery_driver.h"
/*============================================================================*
 *                              Local Variables
 *============================================================================*/

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
T_APP_GLOBAL_DATA app_global_data;
#if FEATURE_SUPPORT_PRIVACY
T_PRIVACY_STATE app_privacy_state = PRIVACY_STATE_INIT;
T_PRIVACY_ADDR_RESOLUTION_STATE app_privacy_resolution_state = PRIVACY_ADDR_RESOLUTION_DISABLED;
#endif

/*============================================================================*
 *                              External Variables
 *============================================================================*/

/*============================================================================*
 *                              External Functions
 *============================================================================*/
extern void periph_handle_gap_msg(T_IO_MSG  *p_gap_msg);
extern void app_nvic_config(void);
extern bool BTIF_VendorGetResponse(uint8_t *pData, uint8_t len);

/*============================================================================*
 *                              Loacal Functions
 *============================================================================*/
/**
* @brief handler stop adverting reason
* @param   T_STOP_ADV_REASON reason.
* @return  void
*/
static void app_stop_adv_reason_handler(T_STOP_ADV_REASON reason)
{
    switch (reason)
    {
    case STOP_ADV_REASON_POWERKEY:
        APP_PRINT_INFO0("STOP_ADV_REASON_POWERKEY!");
        rcu_start_adv(ADV_UNDIRECT_POWER);
        break;
    case STOP_ADV_REASON_PAIRING:
        APP_PRINT_INFO0("STOP_ADV_REASON_PAIRING!");
        rcu_start_adv(ADV_UNDIRECT_PAIRING);
        break;
    case STOP_ADV_REASON_TIMEOUT:
        APP_PRINT_INFO0("STOP_ADV_REASON_TIMEOUT!");
        break;
    case STOP_ADV_REASON_LOWPOWER:
        APP_PRINT_INFO0("STOP_ADV_REASON_LOWPOWER!");
        app_global_data.rcu_status = RCU_STATUS_LOW_POWER;
        break;
    case STOP_ADV_REASON_UART_CMD:
        APP_PRINT_INFO0("STOP_ADV_REASON_UART_CMD!");
        break;
    case STOP_ADV_REASON_IDLE:
        APP_PRINT_INFO0("STOP_ADV_REASON_IDLE!");
        if (app_global_data.adv_type == ADV_DIRECT_HDC)
        {
            APP_PRINT_INFO1("[STOP_ADV_REASON_IDLE] retry cnt is %d", app_global_data.direct_adv_cnt);
            if (app_global_data.direct_adv_cnt < MAX_DIRECT_ADV_RETRY_CNT)
            {
                rcu_start_adv(ADV_DIRECT_HDC);
                app_global_data.direct_adv_cnt++;
            }
            else
            {
                app_global_data.direct_adv_cnt = 0;  /* reset direct_adv_cnt to 0 */

            }
        }
        break;
    default:
        APP_PRINT_WARN1("Unknown stop_adv_reason: %d", app_global_data.stop_adv_reason);
        break;
    }
}

/**
* @brief handler disconnect reason
* @param   T_DISCONN_REASON reason.
* @return  void
*/
static void app_disconn_reason_handler(T_DISCONN_REASON reason)
{
    switch (reason)
    {
    case DISCONN_REASON_PAIRING:
        APP_PRINT_INFO0("[GAP_CONN_STATE_DISCONNECTED] DISCONN_REASON_PAIRING");
        rcu_start_adv(ADV_UNDIRECT_PAIRING);
        break;
    case DISCONN_REASON_TIMEOUT:
        APP_PRINT_INFO0("[GAP_CONN_STATE_DISCONNECTED] DISCONN_REASON_TIMEOUT");
        break;
    case DISCONN_REASON_OTA:
        APP_PRINT_INFO0("[GAP_CONN_STATE_DISCONNECTED] DISCONN_REASON_OTA");
        /* switch to OTA and reboot */
        dfu_switch_to_ota_mode();
        WDG_SystemReset(RESET_ALL_EXCEPT_AON, DFU_SWITCH_TO_OTA);
        break;
    case DISCONN_REASON_PAIR_FAILED:
        APP_PRINT_INFO1("[GAP_CONN_STATE_DISCONNECTED] DISCONN_REASON_PAIR_FAILED, retry cnt is %d",
                        app_global_data.pair_failed_retry_cnt);
        if (app_global_data.pair_failed_retry_cnt < MAX_PAIR_FAILED_RETRY_CNT)
        {
            app_global_data.pair_failed_retry_cnt++;
            rcu_start_adv(ADV_UNDIRECT_PAIRING);
        }
        break;
    case DISCONN_REASON_LOW_POWER:
        APP_PRINT_INFO0("[GAP_CONN_STATE_DISCONNECTED] DISCONN_REASON_LOW_POWER");
        app_global_data.rcu_status = RCU_STATUS_LOW_POWER;
        break;
    case DISCONN_REASON_UART_CMD:
        APP_PRINT_INFO0("[GAP_CONN_STATE_DISCONNECTED] DISCONN_REASON_UART_CMD");
        break;
#if SUPPORT_SILENT_OTA
    case DISCONN_REASON_SILENT_OTA:
        WDG_SystemReset(RESET_ALL_EXCEPT_AON, DFU_SWITCH_TO_OTA);
        break;
#endif
    default:
        APP_PRINT_WARN1("[GAP_CONN_STATE_DISCONNECTED] unknown reason: %d", app_global_data.disconn_reason);
        break;
    }
}

/******************************************************************
 * @fn          app_general_srv_cb
 * @brief      General service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_general_srv_cb(T_SERVER_APP_CB_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    switch (p_data->eventId)
    {
    case PROFILE_EVT_SRV_REG_COMPLETE:
        APP_PRINT_INFO1("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
                        p_data->event_data.service_reg_result);
        break;

    case PROFILE_EVT_SEND_DATA_COMPLETE:
        APP_PRINT_INFO5("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits = %d",
                        p_data->event_data.send_data_result.conn_id,
                        p_data->event_data.send_data_result.cause,
                        p_data->event_data.send_data_result.service_id,
                        p_data->event_data.send_data_result.attrib_idx,
                        p_data->event_data.send_data_result.credits);
        if (p_data->event_data.send_data_result.cause == GAP_SUCCESS)
        {
            APP_PRINT_INFO0("PROFILE_EVT_SEND_DATA_COMPLETE success");

        }
        else
        {
            APP_PRINT_ERROR0("PROFILE_EVT_SEND_DATA_COMPLETE failed");
        }
        break;

    default:
        break;
    }

    return cb_result;
}

/******************************************************************
 * @fn          app_bas_srv_cb
 * @brief      BAS service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_bas_srv_cb(T_BAS_CALLBACK_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    switch (p_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
        {
            if (p_data->msg_data.notification_indification_index == BAS_NOTIFY_BATTERY_LEVEL_ENABLE)
            {
                APP_PRINT_INFO0("[app_bas_srv_cb] Battery level notification enable");
            }
            else if (p_data->msg_data.notification_indification_index ==
                     BAS_NOTIFY_BATTERY_LEVEL_DISABLE)
            {
                APP_PRINT_INFO0("[app_bas_srv_cb] Battery level notification disable");
            }
        }
        break;
    case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
        {
            /* get battery value and level*/
            rcu_get_bat_value(&app_global_data.battery_status.level_value,
                              &app_global_data.battery_status.voltage_value);

            APP_PRINT_INFO2("RCU_Battery_Level_Update voltage is %d, level is %d",
                            app_global_data.battery_status.voltage_value,
                            app_global_data.battery_status.level_value);

            uint8_t bat_level_temp = app_global_data.battery_status.level_value;
            bas_set_parameter(BAS_PARAM_BATTERY_LEVEL, 1,
                              (uint8_t *)&bat_level_temp);
        }
        break;
    default:
        break;
    }

    return cb_result;
}

/******************************************************************
 * @fn          app_dis_srv_cb
 * @brief      DIS service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_dis_srv_cb(T_DIS_CALLBACK_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    switch (p_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
        {
            if (p_data->msg_data.read_value_index == DIS_READ_MANU_NAME_INDEX)
            {
                const uint8_t DISManufacturerName[] = "Realtek BT";
                dis_set_parameter(DIS_PARAM_MANUFACTURER_NAME,
                                  sizeof(DISManufacturerName) - 1,
                                  (void *)DISManufacturerName);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_MODEL_NUM_INDEX)
            {
                const uint8_t DISModelNumber[] = "Model Nbr 0.9";
                dis_set_parameter(DIS_PARAM_MODEL_NUMBER,
                                  sizeof(DISModelNumber) - 1,
                                  (void *)DISModelNumber);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_SERIAL_NUM_INDEX)
            {
                const uint8_t DISSerialNumber[] = "RTKBeeSerialNum";
                dis_set_parameter(DIS_PARAM_SERIAL_NUMBER,
                                  sizeof(DISSerialNumber) - 1,
                                  (void *)DISSerialNumber);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_HARDWARE_REV_INDEX)
            {
                const uint8_t DISHardwareRev[] = "RTKBeeHardwareRev";
                dis_set_parameter(DIS_PARAM_HARDWARE_REVISION,
                                  sizeof(DISHardwareRev) - 1,
                                  (void *)DISHardwareRev);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_FIRMWARE_REV_INDEX)
            {
                const uint8_t DISFirmwareRev[] = "RTKBeeFirmwareRev";
                dis_set_parameter(DIS_PARAM_FIRMWARE_REVISION,
                                  sizeof(DISFirmwareRev) - 1,
                                  (void *)DISFirmwareRev);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_SOFTWARE_REV_INDEX)
            {
                const uint8_t DISSoftwareRev[] = "RTKBeeSoftwareRev";
                dis_set_parameter(DIS_PARAM_SOFTWARE_REVISION,
                                  sizeof(DISSoftwareRev) - 1,
                                  (void *)DISSoftwareRev);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_SYSTEM_ID_INDEX)
            {
                const uint8_t DISSystemID[DIS_SYSTEM_ID_LENGTH] = {0, 1, 2, 0, 0, 3, 4, 5};
                dis_set_parameter(DIS_PARAM_SYSTEM_ID,
                                  sizeof(DISSystemID),
                                  (void *)DISSystemID);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_IEEE_CERT_STR_INDEX)
            {
                const uint8_t DISIEEEDataList[] = "RTKBeeIEEEDatalist";
                dis_set_parameter(DIS_PARAM_IEEE_DATA_LIST,
                                  sizeof(DISIEEEDataList) - 1,
                                  (void *)DISIEEEDataList);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_PNP_ID_INDEX)
            {
                /* TODO: need to conform this info with Androind team */
                uint16_t version = 0x03; //VERSION_BUILD;

                uint8_t DISPnpID[DIS_PNP_ID_LENGTH] = {0x01, 0x5D, 0x00, 0x01, 0x00, (uint8_t)version, (uint8_t)(version >> 8)}; //VID_005D&PID_0001?

                dis_set_parameter(DIS_PARAM_PNP_ID,
                                  sizeof(DISPnpID),
                                  DISPnpID);
            }
        }
        break;
    default:
        break;
    }

    return cb_result;
}

/******************************************************************
 * @fn          app_hid_srv_cb
 * @brief      HID service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_hid_srv_cb(T_HID_CALLBACK_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    APP_PRINT_INFO1("[app_hid_srv_cb] msg_type %d", p_data->msg_type);
    switch (p_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
        {
            switch (p_data->msg_data.notification_indification_index)
            {
            case HID_NOTIFY_INDICATE_KB_ENABLE:
                APP_PRINT_INFO0("[app_hid_srv_cb] HID_NOTIFY_INDICATE_KB_ENABLE");
                app_global_data.is_keyboard_notify_en = true;
                break;
            case HID_NOTIFY_INDICATE_KB_DISABLE:
                APP_PRINT_INFO0("[app_hid_srv_cb] HID_NOTIFY_INDICATE_KB_DISABLE");
                app_global_data.is_keyboard_notify_en = false;
                break;
            default:
                break;
            }
        }
        break;

    case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
        {
            APP_PRINT_INFO0("SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE\n");
        }
        break;
    default:
        break;
    }

    return cb_result;
}

/******************************************************************
 * @fn          app_vendor_srv_cb
 * @brief      Vendor service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_vendor_srv_cb(T_VENDOR_CALLBACK_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    APP_PRINT_INFO1("[app_vendor_srv_cb] msg_type %d", p_data->msg_type);
    switch (p_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
        {
            if (p_data->msg_data.notification_indification_index == VENDOR_NOTIFY_ENABLE)
            {
                APP_PRINT_INFO0("[app_vendor_srv_cb] VENDOR_NOTIFY_ENABLE");
//                server_send_data(0, app_global_data.vendor_srv_id,
//                                 BLE_SERVICE_CHAR_VENDOR_HANDSHAKE_INDEX,
//                                 (uint8_t *)vendor_svc_handshake_values,
//                                 16,
//                                 GATT_PDU_TYPE_NOTIFICATION);
            }
            else if (p_data->msg_data.notification_indification_index == VENDOR_NOTIFY_DISABLE)
            {
                APP_PRINT_INFO0("[app_vendor_srv_cb] VENDOR_NOTIFY_DISABLE");
            }
        }
        break;
    case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
        {
            if (p_data->msg_data.write_msg.write_type == VENDOR_WRITE_HANDSHAKE)
            {
                APP_PRINT_INFO0("[app_vendor_srv_cb] SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE");
                memcpy(vendor_svc_handshake_values, p_data->msg_data.write_msg.write_parameter.report_data.report,
                       16);
                if (!BTIF_VendorGetResponse(vendor_svc_handshake_values, 16))
                {
                    APP_PRINT_WARN0("[app_vendor_srv_cb] blueAPI_HandShakeGetResponse failed!");
                    cb_result = APP_RESULT_APP_ERR;
                }
            }

            else
            {
                cb_result = APP_RESULT_APP_ERR;
            }
        }
        break;
    default:
        break;
    }

    return cb_result;
}

/******************************************************************
 * @fn          app_ota_srv_cb
 * @brief      OTA service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_ota_srv_cb(TOTA_CALLBACK_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    switch (p_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
        if (OTA_WRITE_CHAR_VAL == p_data->msg_data.write.opcode &&
            OTA_VALUE_ENTER == p_data->msg_data.write.u.value)
        {
            /* unlock flash */
#if 0
            uint8_t prev_bp_lv = 0;
            if (1 == flash_sw_protect_unlock_by_addr((0x00800000), &prev_bp_lv))
            {
                APP_PRINT_INFO1("[Flash Set] Flash unlock address = 0x00800000, prev_bp_lv = %d", prev_bp_lv);
            }
            else
            {
                APP_PRINT_INFO0("[Flash Set] Flash unlock address = 0x00800000, unlock error");
            }
#endif
            /* TODO: add OTA battery check */
            APP_PRINT_INFO0("Preparing switch into OTA mode\n");
            /*prepare to enter OTA mode, before switch action, we should disconnect first.*/
            rcu_terminate_connection(DISCONN_REASON_OTA);
        }
        break;

    default:
        break;
    }

    return cb_result;
}



/**
* @brief   handle bond modify message.
* @param   T_LE_BOND_MODIFY_TYPE type, T_LE_KEY_ENTRY *p_entry
* @return  void
*/
void app_handle_bond_modify_msg(T_LE_BOND_MODIFY_TYPE type, T_LE_KEY_ENTRY *p_entry)
{
    APP_PRINT_INFO1("app_handle_bond_modify_msg  GAP_MSG_LE_BOND_MODIFY_INFO:type=0x%x",
                    type);

    switch (type)
    {
    case LE_BOND_DELETE:
        {
            le_modify_white_list(GAP_WHITE_LIST_OP_REMOVE, p_entry->remote_bd.addr,
                                 (T_GAP_REMOTE_ADDR_TYPE)(p_entry->remote_bd.remote_bd_type));
        }
        break;
    case LE_BOND_ADD:
        {
            le_modify_white_list(GAP_WHITE_LIST_OP_ADD, p_entry->remote_bd.addr,
                                 (T_GAP_REMOTE_ADDR_TYPE)(p_entry->remote_bd.remote_bd_type));
        }
        break;
    case LE_BOND_CLEAR:
        {
            le_modify_white_list(GAP_WHITE_LIST_OP_CLEAR, p_entry->remote_bd.addr,
                                 (T_GAP_REMOTE_ADDR_TYPE)(p_entry->remote_bd.remote_bd_type));
        }
        break;
    default:
        break;
    }
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
/**
* @brief set or get rcu app latency status.
* @param   T_LATENCY_STATUS new_status.
* @return  void
*/
void app_set_latency_status(T_APP_LATENCY_MASK index_bit, T_LATENCY_STATE new_state)
{
    uint8_t retry_cnt = 0;
    bool latency_upate_value = true;

    APP_PRINT_INFO2("[app_set_latency_status] module index = %d, new_state is %d", index_bit,
                    new_state);

    /*0. check new state valid or not */
    if ((new_state != LANTENCY_ON) && (new_state != LANTENCY_OFF))
    {
        APP_PRINT_WARN0("[app_set_latency_status] new_state is invalid");
        return;
    }

    /*1. check latency state is updating */
    if (app_global_data.latency_status.is_updating == true)
    {
        APP_PRINT_INFO0("[app_set_latency_status] latency_status is pending!");
        /* update pending latency status */
        app_global_data.latency_status.is_pending = true;
        app_global_data.latency_status.pending_status = new_state;
        return ;
    }

    /*2. update latency data value */
    if (new_state == LANTENCY_OFF)
    {
        app_global_data.latency_status.latency_bits_map |= (1 << index_bit);
    }
    else
    {
        app_global_data.latency_status.latency_bits_map &= ~(1 << index_bit);
    }

    /*3. check whether need update according to latency value*/
    if (0 == app_global_data.latency_status.latency_bits_map)
    {
        /* need enable latency*/
        if (LANTENCY_ON == app_global_data.latency_status.cur_status)
        {
            APP_PRINT_INFO0("[app_set_latency_status] latency_status is already enabled!");
            return;
        }
        latency_upate_value = false;
    }
    else
    {
        /* need disable latency */
        if (LANTENCY_OFF == app_global_data.latency_status.cur_status)
        {
            APP_PRINT_INFO0("[app_set_latency_status] latency_status is already disabled!");
            return;
        }
        latency_upate_value = true;
    }

    /*4. process latency update*/
    while (retry_cnt < MAX_ONOFF_LATENCY_SEND_RETRY_CNT)
    {
        if (le_disable_slave_latency(0, latency_upate_value) == GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_INFO1("[app_set_latency_status] call le_disable_slave_latency successfully, latency bit map: %#x",
                            app_global_data.latency_status.latency_bits_map);
            app_global_data.latency_status.is_pending = false;
            app_global_data.latency_status.is_updating = true;
            app_global_data.latency_status.cur_status = new_state;
            app_global_data.latency_status.pending_module = index_bit;
            return;
        }
        retry_cnt++;
    }

    /*5. check update success or not*/
    if (retry_cnt >= MAX_ONOFF_LATENCY_SEND_RETRY_CNT)
    {
        /* all retry failed */
        APP_PRINT_WARN0("[app_set_latency_status] all retry failed");
        /* TBD: how to handle this scenario */
    }
}

/******************************************************************
 * @fn  app_init_global_data
 * @brief  Initialize global APP data
 * @param  void
 * @return  void
 */
void app_init_global_data(void)
{
    memset(&app_global_data, 0, sizeof(app_global_data));
    app_global_data.mtu_size = 23;
    app_global_data.is_adc_efuse_existed = false;
    app_global_data.is_adc_efuse_existed = ADC_CalibrationInit();
    if (false == app_global_data.is_adc_efuse_existed)
    {
        APP_PRINT_WARN0("Read ADC efuse data error!");
    }

    //uint32_t addr = flash_get_ota_bank_addr_by_sign(SIGNATURE_APP);
    //APP_PRINT_INFO1("Free App Bank Addr  = 0x%X", addr);
    //addr = flash_get_ota_bank_addr_by_sign(SIGNATURE_PATCH_FLASH);
    //APP_PRINT_INFO1("Free Patch Bank Addr  = 0x%X", addr);
}

/******************************************************************
 * @fn          app_handle_io_msg
 * @brief      All the application events are pre-handled in this function.
 *                All the IO MSGs are sent to this function, Then the event handling function
 *                shall be called according to the MSG type.
 *
 * @param    io_driver_msg_recv  - bee io msg data
 * @return     void
 */
void app_handle_io_msg(T_IO_MSG io_driver_msg_recv)
{
    uint16_t msg_type = io_driver_msg_recv.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        {
            periph_handle_gap_msg(&io_driver_msg_recv);
        }
        break;

#if BAT_EN
    case IO_MSG_TYPE_BAT_LPC:
        {
            battery_msg_handle(msg_type);
        }
        break;
#endif
    case IO_MSG_TYPE_RESET_WDG_TIMER:
        {
            APP_PRINT_INFO0("[WDG] Watch Dog Rset Timer");
            WDG_Restart();
        }
        break;
    default:
        break;
    }
}

/******************************************************************
 * @fn          peripheral_HandleBtDevStateChangeEvt
 * @brief      All the gaprole_States_t events are pre-handled in this function.
 *                Then the event handling function shall be called according to the newState.
 *
 * @param    newState  - new gap state
 * @return     void
 */
void periph_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO4("periph_handle_dev_state_evt: init state %d, adv state %d, conn state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state,
                    new_state.gap_conn_state, cause);

#if FEATURE_SUPPORT_PRIVACY
    if ((new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        && (new_state.gap_adv_state == GAP_ADV_STATE_IDLE)
        && (new_state.gap_conn_state == GAP_CONN_DEV_STATE_IDLE))
    {
        privacy_handle_resolv_list();
    }
#endif

    if (app_global_data.gap_dev_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            APP_PRINT_INFO0("GAP stack ready");

            /* set PPCP, otherwise may cause connection update failed case */
            gaps_set_peripheral_preferred_conn_param(RCU_CONNECT_INTERVAL, RCU_CONNECT_INTERVAL,
                                                     RCU_CONNECT_LATENCY, RCU_SUPERVISION_TIMEOUT / 10);

            app_global_data.rcu_status = RCU_STATUS_IDLE;

            T_LE_KEY_ENTRY *p_le_key_entry;
            p_le_key_entry = le_get_high_priority_bond();
            if ((p_le_key_entry != NULL) && (p_le_key_entry->is_used))
            {
                app_global_data.is_link_key_existed = true;
                /* attempt to reconnect with master */
#if FEATURE_SUPPORT_PRIVACY
                rcu_start_adv(ADV_UNDIRECT_RECONNECT);
#else
                /* add paired device in white list */
                le_modify_white_list(GAP_WHITE_LIST_OP_ADD, p_le_key_entry->remote_bd.addr,
                                     (T_GAP_REMOTE_ADDR_TYPE)(p_le_key_entry->remote_bd.remote_bd_type));
                app_global_data.direct_adv_cnt = MAX_DIRECT_ADV_RETRY_CNT;
                rcu_start_adv(ADV_DIRECT_HDC);
#endif
            }
            else
            {
                app_global_data.is_link_key_existed = false;
            }

            app_nvic_config();  /* enable peripheral NVIC after GAP stack ready */
        }
    }

    if (app_global_data.gap_dev_state.gap_adv_state != new_state.gap_adv_state)
    {
        if (new_state.gap_adv_state == GAP_ADV_STATE_IDLE)
        {
            /* sync rcu status to idle */
            app_global_data.rcu_status = RCU_STATUS_IDLE;
            if (app_global_data.adv_type != ADV_DIRECT_HDC)
            {
                os_timer_stop(&adv_timer);
            }

            if (new_state.gap_adv_sub_state == GAP_ADV_TO_IDLE_CAUSE_STOP)
            {
                APP_PRINT_INFO0("Advertisng is stopped by user stop or timeouot");

                /* check adv stop reason */
                app_stop_adv_reason_handler(app_global_data.stop_adv_reason);
                app_global_data.stop_adv_reason = STOP_ADV_REASON_IDLE;  /* reset stop_adv_reason */
            }
            else if (new_state.gap_adv_sub_state == GAP_ADV_TO_IDLE_CAUSE_CONN)
            {
                APP_PRINT_INFO0("Advertisng is stopped for link establishment");
            }
        }
        else if (new_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING)
        {
            APP_PRINT_INFO0("GAP adv start");
            app_global_data.rcu_status = RCU_STATUS_ADVERTISING;
        }
    }

    if (app_global_data.gap_dev_state.gap_conn_state != new_state.gap_conn_state)
    {
        APP_PRINT_INFO2("conn state: %d -> %d",
                        app_global_data.gap_dev_state.gap_conn_state,
                        new_state.gap_conn_state);
    }
    app_global_data.gap_dev_state = new_state;
}

/******************************************************************
 * @brief      Handle connection status change event.
 */
void periph_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    APP_PRINT_INFO3("periph_handle_conn_state_evt: conn_id = %d old_state = %d new_state = %d",
                    conn_id, app_global_data.gap_conn_state, new_state);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            app_global_data.rcu_status = RCU_STATUS_IDLE;
#if FEATURE_SUPPORT_NO_ACTION_DISCONN
            /* stop no_act_disconn_timer if disconnection detected */
            os_timer_stop(&no_act_disconn_timer);
#endif
            os_timer_stop(&next_state_time_out);  /* ensure pair_fail_disconn_timer is stopped */



            if (disc_cause == (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
            {
                APP_PRINT_INFO0("[GAP_CONN_STATE_DISCONNECTED] REMOTE_USER_TERMINATE");

                if (app_global_data.is_link_key_existed == false)
                {
                    if (app_global_data.pair_failed_retry_cnt < MAX_PAIR_FAILED_RETRY_CNT)
                    {
                        app_global_data.pair_failed_retry_cnt++;
                        rcu_start_adv(ADV_UNDIRECT_PAIRING);
                    }
                }
            }
            else if (disc_cause == (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE))
            {
                /* handle APP disconection reason */
                app_disconn_reason_handler(app_global_data.disconn_reason);
            }
            else
            {
                APP_PRINT_WARN1("[GAP_CONN_STATE_DISCONNECTED] Connection lost: cause 0x%x", disc_cause);
            }

            app_global_data.disconn_reason = DISCONN_REASON_IDLE;  /* reset disconn_reason */
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            app_global_data.rcu_status = RCU_STATUS_CONNECTED;
            //os_timer_restart(&next_state_time_out, PAIRING_EXCEPTION_TIMEOUT);

            /* reset latency_status to default after connect */
            memset(&app_global_data.latency_status, 0, sizeof(app_global_data.latency_status));
            /* turn off slave latency to speed up pairing process, have to turn on latency later */
            app_set_latency_status(LATENCY_SYS_UPDATE_BIT, LANTENCY_OFF);

            /* print remote device information */
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;
            uint8_t  remote_bd[6];
            uint8_t remote_bd_type;
            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            le_get_conn_addr(conn_id, remote_bd, &remote_bd_type);
            APP_PRINT_INFO5("GAP_CONN_STATE_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
                            TRACE_BDADDR(remote_bd), remote_bd_type,
                            conn_interval, conn_latency, conn_supervision_timeout);
        }
        break;

    default:
        break;
    }
    app_global_data.gap_conn_state = new_state;
}

/******************************************************************
 * @fn          peripheral_HandleBtGapAuthenStateChangeEvt
 * @brief      All the bonding state change  events are pre-handled in this function.
 *                Then the event handling function shall be called according to the newState.
 *
 * @param    newState  - new bonding state
 * @return     void
 */
void periph_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO1("periph_handle_authen_state_evt:conn_id %d", conn_id);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("GAP_AUTHEN_STATE_STARTED");
            os_timer_restart(&next_state_time_out, PAIRING_EXCEPTION_TIMEOUT);
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_CAUSE_SUCCESS)
            {
                APP_PRINT_INFO0("[GAP_AUTHEN_STATE_COMPLETE] pairing success");
                app_global_data.is_link_key_existed = true;
                app_global_data.rcu_status = RCU_STATUS_PAIRED;
                app_global_data.pair_failed_retry_cnt = 0;

                os_timer_stop(&next_state_time_out);
#if FEATURE_SUPPORT_NO_ACTION_DISCONN
                os_timer_restart(&no_act_disconn_timer, NO_ACTION_DISCON_TIMEOUT);
#endif
                //LED_BLINK(LED_1, LED_TYPE_BLINK_PAIR_SUCCESS, 3);

                {
                    /* start timer to update connection parameter, postpone with timer to accelate GATT process */
                    os_timer_restart(&update_conn_params_timer, UPDATE_CONN_PARAMS_TIMEOUT);
                }
            }
            else
            {
                APP_PRINT_INFO0("[GAP_AUTHEN_STATE_COMPLETE] pair failed");
#if (FEATURE_SUPPORT_REMOVE_LINK_KEY_BEFORE_PAIRING && FEATURE_SUPPORT_PRIVACY)
                if (app_global_data.is_link_key_existed)
                {
                    app_global_data.is_link_key_existed = false;
                    le_bond_clear_all_keys();

                }
#endif
                os_timer_restart(&next_state_time_out, PAIR_FAIL_DISCONN_TIMEOUT);
            }
        }
        break;

    default:
        {
            APP_PRINT_INFO1("GAP_MSG_LE_AUTHEN_STATE_CHANGE:(unknown newstate: %d)", new_state);
        }
        break;
    }
}

/******************************************************************
 * @fn          peripheral_HandleBtGapConnParaChangeEvt
 * @brief      All the connection parameter update change  events are pre-handled in this function.
 *                Then the event handling function shall be called according to the status.
 *
 * @param    status  - connection parameter result, 0 - success, otherwise fail.
 * @return     void
 */
void periph_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            /* reset updt_conn_params_retry_cnt */
            app_global_data.updt_conn_params_retry_cnt = 0;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &app_global_data.conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &app_global_data.conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &app_global_data.conn_supervision_timeout, conn_id);

            APP_PRINT_INFO3("GAP_MSG_LE_CONN_PARAM_UPDATE update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            app_global_data.conn_interval, app_global_data.conn_latency,
                            app_global_data.conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR1("GAP_MSG_LE_CONN_PARAM_UPDATE failed: cause 0x%x", cause);
            if (app_global_data.updt_conn_params_retry_cnt < MAX_UPDATE_CONN_PARAMS_RETRY_CNT)
            {
                app_global_data.updt_conn_params_retry_cnt++;
                APP_PRINT_WARN0("[rcu_update_conn_params] GAP_CONN_PARAM_UPDATE_STATUS_FAIL retry");
                os_timer_restart(&update_conn_params_timer, UPDATE_CONN_PARAMS_TIMEOUT);
            }
            else
            {
                APP_PRINT_WARN0("[rcu_update_conn_params] GAP_CONN_PARAM_UPDATE_STATUS_FAIL failed");
            }
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("GAP_CONN_PARAM_UPDATE_STATUS_PENDING");
        }
        break;

    default:
        break;
    }
}

/******************************************************************
 * @fn          peripheral_HandleBtGapMessage
 * @brief      All the bt gap msg  events are pre-handled in this function.
 *                Then the event handling function shall be called according to the subType
 *                of BEE_IO_MSG.
 *
 * @param    pBeeIoMsg  - pointer to bee io msg
 * @return     void
 */
void periph_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t conn_id;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));

    APP_PRINT_TRACE1("periph_handle_gap_msg subType = %d", p_gap_msg->subtype);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:
        {
            periph_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                        gap_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:
        {
            periph_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                         (T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
                                         gap_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_AUTHEN_STATE_CHANGE:
        {
            periph_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
                                           gap_msg.msg_data.gap_authen_state.new_state,
                                           gap_msg.msg_data.gap_authen_state.status);
        }
        break;

    case GAP_MSG_LE_BOND_JUST_WORK:
        {
            conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
            le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_DISPLAY:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_DISPLAY:passkey %d", display_value);
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_USER_CONFIRMATION: passkey %d", display_value);
            le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
            le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_OOB_INPUT:
        {
            uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_OOB_INPUT");
            le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
            le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:
        {
            periph_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
                                         gap_msg.msg_data.gap_conn_param_update.status,
                                         gap_msg.msg_data.gap_conn_param_update.cause);
        }
        break;
    case GAP_MSG_LE_CONN_MTU_INFO:
        {
            uint8_t mtu_size = 0;
            le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu_size,
                              gap_msg.msg_data.gap_conn_mtu_info.conn_id);

            if (mtu_size >= 23)
            {
                /* update mtu size */
                app_global_data.mtu_size = mtu_size;
                APP_PRINT_INFO1("[periph_handle_gap_msg] update mtu size to %d", mtu_size);
            }
        }
        break;

    default:
        APP_PRINT_ERROR1("periph_handle_gap_msg: unknown subtype %d", p_gap_msg->subtype);
        break;
    }
}

T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;

    switch (cb_type)
    {
    case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:
        APP_PRINT_INFO3("GAP_MSG_LE_DATA_LEN_CHANGE_INFO: conn_id %d, tx octets 0x%x, max_tx_time 0x%x",
                        p_data->p_le_data_len_change_info->conn_id,
                        p_data->p_le_data_len_change_info->max_tx_octets,
                        p_data->p_le_data_len_change_info->max_tx_time);
        break;

    case GAP_MSG_LE_BOND_MODIFY_INFO:
        APP_PRINT_INFO1("GAP_MSG_LE_BOND_MODIFY_INFO: type 0x%x",
                        p_data->p_le_bond_modify_info->type);
#if FEATURE_SUPPORT_PRIVACY
        privacy_handle_bond_modify_msg(p_data->p_le_bond_modify_info->type,
                                       p_data->p_le_bond_modify_info->p_entry, true);
#else
        app_handle_bond_modify_msg(p_data->p_le_bond_modify_info->type,
                                   p_data->p_le_bond_modify_info->p_entry);
#endif
        break;

    case GAP_MSG_LE_MODIFY_WHITE_LIST:
        APP_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation %d, cause 0x%x",
                        p_data->p_le_modify_white_list_rsp->operation,
                        p_data->p_le_modify_white_list_rsp->cause);
        break;

    case GAP_MSG_LE_DISABLE_SLAVE_LATENCY:
        APP_PRINT_INFO1("GAP_MSG_LE_DISABLE_SLAVE_LATENCY: cause 0x%x",
                        p_data->p_le_disable_slave_latency_rsp->cause);
        /* restore is_updating to false */
        app_global_data.latency_status.is_updating = false;
        if (p_data->p_le_disable_slave_latency_rsp->cause == GAP_CAUSE_SUCCESS)
        {
            app_global_data.latency_status.retry_cnt = 0;
            /* check if has any pending status */
            if (app_global_data.latency_status.is_pending == true)
            {
                APP_PRINT_INFO0("[app_gap_callback] handle onoff latency pending status");
                app_set_latency_status(app_global_data.latency_status.pending_module,
                                       app_global_data.latency_status.pending_status);
            }
        }
        else
        {
            if (app_global_data.latency_status.retry_cnt < MAX_ONOFF_LATENCY_FAILED_RETRY_CNT)
            {
                APP_PRINT_WARN0("[app_gap_callback] retry onoff latency");
                app_global_data.latency_status.retry_cnt++;
                app_set_latency_status(app_global_data.latency_status.pending_module,
                                       app_global_data.latency_status.cur_status);
            }
            else
            {
                APP_PRINT_ERROR0("[app_gap_callback] OnOff latency failed!");
                /* TBD: how to handle this scenario */
            }
        }
        break;

    default:
        APP_PRINT_INFO1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}

#if FEATURE_SUPPORT_PRIVACY
void app_privacy_callback(T_PRIVACY_CB_TYPE type, T_PRIVACY_CB_DATA cb_data)
{
    APP_PRINT_INFO1("app_privacy_callback: type %d", type);
    switch (type)
    {
    case PRIVACY_STATE_MSGTYPE:
        app_privacy_state = cb_data.privacy_state;
        break;

    case PRIVACY_RESOLUTION_STATUS_MSGTYPE:
        app_privacy_resolution_state = cb_data.resolution_state;
        break;

    default:
        break;
    }
}
#endif

/******************************************************************
 * @fn          app_profile_callback
 * @brief      All the bt profile callbacks are handled in this function.
 *                Then the event handling function shall be called according to the serviceID
 *                of BEE_IO_MSG.
 *
 * @param    serviceID  -  service id of profile
 * @param    pData  - pointer to callback data
 * @return     void
 */
T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    if (service_id == SERVICE_PROFILE_GENERAL_ID)
    {
        app_general_srv_cb((T_SERVER_APP_CB_DATA *)p_data);
    }
    else if (service_id == app_global_data.bas_srv_id)
    {
        app_result = app_bas_srv_cb((T_BAS_CALLBACK_DATA *)p_data);
    }
    else if (service_id == app_global_data.dis_srv_id)
    {
        app_result = app_dis_srv_cb((T_DIS_CALLBACK_DATA *)p_data);
    }
    else if (service_id == app_global_data.hid_srv_id)
    {
        app_result = app_hid_srv_cb((T_HID_CALLBACK_DATA *)p_data);
    }
    else if (service_id == app_global_data.vendor_srv_id)
    {
        app_result = app_vendor_srv_cb((T_VENDOR_CALLBACK_DATA *)p_data);
    }
    else if (service_id == app_global_data.ota_srv_id)
    {
        app_result = app_ota_srv_cb((TOTA_CALLBACK_DATA *)p_data);
    }

    return app_result;
}
