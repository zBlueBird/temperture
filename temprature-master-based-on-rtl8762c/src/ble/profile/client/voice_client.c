/**
*****************************************************************************************
*     Copyright(c) 2020, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    voice_client.c
  * @brief
  * @details
  * @author  yuyin
  * @date    2020-01-10
  * @version v1.0
  ******************************************************************************
  */

/** Add Includes here **/
#include <string.h>
#include <voice_client.h>
#include <trace.h>
#include <os_mem.h>
#include "voice_s2m.h"

/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/

#define GATT_UUID_VOICE                           0xD1FF
#define GATT_UUID_CHAR_VOICE_LEVEL                0xA001

uint8_t GATT_UUID_VENDOR_SERVICE[16] = {0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0xFF, 0xD1, 0x00, 0x00};
uint8_t GATT_UUID_VENDOR_CHAR[16] = {0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0xFF, 0x01, 0xA0, 0x00};


/**
 * @brief  VOICE client Link control block definition.
 */
typedef struct
{
    T_VOICE_DISC_STATE disc_state;
    bool             write_notify_value;
    uint16_t         properties;
    uint16_t         hdl_cache[HDL_VOICE_CACHE_LEN];
} T_VOICE_LINK, *P_VOICE_LINK;

static P_VOICE_LINK voice_table;
static uint8_t voice_link_num;

/**<  VOICE client ID. */
T_CLIENT_ID voice_client = CLIENT_PROFILE_GENERAL_ID;
/**<  Callback used to send data to app from VOICE client layer. */
static P_FUN_GENERAL_APP_CB voice_client_cb = NULL;

/**
  * @brief  Used by application, to start the discovery procedure of battery service.
  * @param[in]  conn_id connection ID.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    static T_USER_CMD_PARSE_RESULT cmd_voicedis(T_USER_CMD_PARSED_VALUE *p_parse_value)
    {
        uint8_t conn_id = p_parse_value->dw_param[0];
        bool ret = voice_start_discovery(conn_id);
        ......
    }
  * \endcode
  */
bool voice_start_discovery(uint8_t conn_id)
{
    PROFILE_PRINT_INFO0("voice_start_discovery");
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_start_discovery: failed invalid conn_id %d", conn_id);
        return false;
    }
    /* First clear handle cache. */
    memset(&voice_table[conn_id], 0, sizeof(T_VOICE_LINK));
    voice_table[conn_id].disc_state = DISC_VOICE_START;
    if (client_by_uuid128_srv_discovery(conn_id, voice_client,
                                        GATT_UUID_VENDOR_SERVICE) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    return false;
}

/**
  * @brief  Used by application, to read battery level.
  * @param[in]  conn_id connection ID.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    static T_USER_CMD_PARSE_RESULT cmd_voiceread(T_USER_CMD_PARSED_VALUE *p_parse_value)
    {
        uint8_t conn_id = p_parse_value->dw_param[0];
        bool ret = false;
        ret = voice_read_battery_level(conn_id);
        ......
    }
  * \endcode
  */
bool voice_read_battery_level(uint8_t conn_id)
{
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_read_battery_level: failed invalid conn_id %d", conn_id);
        return false;
    }
    if (voice_table[conn_id].hdl_cache[HDL_VOICE_BATTERY_LEVEL])
    {
        uint16_t handle = voice_table[conn_id].hdl_cache[HDL_VOICE_BATTERY_LEVEL];
        if (client_attr_read(conn_id, voice_client, handle) == GAP_CAUSE_SUCCESS)
        {
            return true;
        }
    }
    PROFILE_PRINT_ERROR0("voice_read_battery_level: false handle = 0");
    return false;
}
/**
  * @brief  Used by application, to set the notification flag.
  * @param[in]  conn_id connection ID.
  * @param[in]  notify value to enable or disable notify.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    static T_USER_CMD_PARSE_RESULT cmd_voicecccd(T_USER_CMD_PARSED_VALUE *p_parse_value)
    {
        uint8_t conn_id = p_parse_value->dw_param[0];
        bool notify = p_parse_value->dw_param[1];
        bool ret;
        ret = voice_set_notify(conn_id, notify);
        ......
    }
  * \endcode
  */
bool voice_set_notify(uint8_t conn_id, uint8_t handle_index, bool notify)
{
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_set_notify: failed invalid conn_id %d", conn_id);
        return false;
    }

    if (handle_index == TEMP_DATA_HANDLE_INDEX)
    {
        T_GAP_CAUSE ret = GAP_CAUSE_SUCCESS;
        uint16_t handle = voice_get_handle(TEMP_DATA_HANDLE_INDEX) +
                          1;//voice_table[conn_id].hdl_cache[HDL_VOICE_TYPE_DATA];
        uint16_t length = sizeof(uint16_t);
        uint16_t cccd_bits = notify ? 1 : 0;
        ret = client_attr_write(conn_id, voice_client, GATT_WRITE_TYPE_REQ, handle,
                                length, (uint8_t *)&cccd_bits);
        PROFILE_PRINT_INFO1("voice_set_notify: ret = %d", ret);
        if (ret == GAP_CAUSE_SUCCESS)
        {
            voice_table[conn_id].write_notify_value = notify;
            PROFILE_PRINT_INFO1("voice_set_notify: correct handle = %d", handle);
            return true;
        }
        else
        {
            PROFILE_PRINT_ERROR1("voice_set_notify: incorrect handle = %d", handle);
        }
    }

    PROFILE_PRINT_ERROR1("voice_set_notify: false handle index = %d", handle_index);
    return false;
}

/**
  * @brief  Used by application, to read the notification flag.
  * @param[in]  conn_id connection ID.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    static T_USER_CMD_PARSE_RESULT cmd_voiceread(T_USER_CMD_PARSED_VALUE *p_parse_value)
    {
        uint8_t conn_id = p_parse_value->dw_param[0];
        bool ret = false;
        ret = voice_read_notify(conn_id);
        ......
    }
  * \endcode
  */
bool voice_read_notify(uint8_t conn_id)
{
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_read_notify: failed invalid conn_id %d", conn_id);
        return false;
    }
    if (voice_table[conn_id].hdl_cache[HDL_VOICE_TYPE_DATA])
    {
        uint16_t handle = voice_table[conn_id].hdl_cache[HDL_VOICE_TYPE_DATA];
        if (client_attr_read(conn_id, voice_client, handle) == GAP_CAUSE_SUCCESS)
        {
            return true;
        }
    }
    PROFILE_PRINT_ERROR0("voice_read_battery_level: false handle = 0");
    return false;
}
/**
  * @brief  Used by application, to write data of V2 write Characteristic.
  * @param[in]  conn_id connection ID.
  * @param[in]  length  write data length
  * @param[in]  p_value point the value to write
  * @param[in]  type    write type.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  */
bool voice_ble_client_write_cmd(uint8_t conn_id, uint16_t length, uint8_t *p_value,
                                T_GATT_WRITE_TYPE type)
{
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_ble_client_write_cmd: failed invalid conn_id %d", conn_id);
        return false;
    }

    //if (voice_table[conn_id].hdl_cache[HDL_SIMBLE_V2_WRITE])
    {
        uint16_t handle = (uint16_t)voice_get_handle(VOICE_DATA_HANDLE_INDEX);
        if (client_attr_write(conn_id, voice_client, type, handle,
                              length, p_value) == GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_WARN0("simp_ble_client_write_v2_char: Request fail! Please check!");
            return true;
        }
    }

    APP_PRINT_WARN0("simp_ble_client_write_v2_char: Request fail! Please check!");
    return false;
}
/**
  * @brief  Used by application, to get handle cache.
  * @param[in]  conn_id connection ID.
  * @param[in]  p_hdl_cache pointer of the handle cache table
  * @param[in]  len the length of handle cache table
  * @retval true success.
  * @retval false failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    static T_USER_CMD_PARSE_RESULT cmd_voicehdl(T_USER_CMD_PARSED_VALUE *p_parse_value)
    {
        uint8_t conn_id = p_parse_value->dw_param[0];
        uint16_t hdl_cache[HDL_VOICE_CACHE_LEN];
        bool ret = voice_get_hdl_cache(conn_id, hdl_cache,
                                     sizeof(uint16_t) * HDL_VOICE_CACHE_LEN);

        ......
    }
  * \endcode
  */
bool voice_get_hdl_cache(uint8_t conn_id, uint16_t *p_hdl_cache, uint8_t len)
{
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_get_hdl_cache: failed invalid conn_id %d", conn_id);
        return false;
    }
    if (voice_table[conn_id].disc_state != DISC_VOICE_DONE)
    {
        PROFILE_PRINT_ERROR1("voice_get_hdl_cache: failed invalid state %d",
                             voice_table[conn_id].disc_state);
        return false;
    }
    if (len != sizeof(uint16_t) * HDL_VOICE_CACHE_LEN)
    {
        PROFILE_PRINT_ERROR1("voice_get_hdl_cache: failed invalid len %d", len);
        return false;
    }
    memcpy(p_hdl_cache, voice_table[conn_id].hdl_cache, len);
    return true;
}

/**
  * @brief  Used by application, to set handle cache.
  * @param[in]  conn_id connection ID.
  * @param[in]  p_hdl_cache pointer of the handle cache table
  * @param[in]  len the length of handle cache table
  * @retval true success.
  * @retval false failed.
  *
  * <b>Example usage</b>
  * \code{.c}
    void app_discov_services(uint8_t conn_id, bool start)
    {
        ......
        if (app_srvs_table.srv_found_flags & APP_DISCOV_VOICE_FLAG)
        {
            voice_set_hdl_cache(conn_id, app_srvs_table.voice_hdl_cache, sizeof(uint16_t) * HDL_VOICE_CACHE_LEN);
        }
        ......
    }
  * \endcode
  */
bool voice_set_hdl_cache(uint8_t conn_id, uint16_t *p_hdl_cache, uint8_t len)
{
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_set_hdl_cache: failed invalid conn_id %d", conn_id);
        return false;
    }
    if (voice_table[conn_id].disc_state != DISC_VOICE_IDLE)
    {
        PROFILE_PRINT_ERROR1("voice_set_hdl_cache: failed invalid state %d",
                             voice_table[conn_id].disc_state);
        return false;
    }
    if (len != sizeof(uint16_t) * HDL_VOICE_CACHE_LEN)
    {
        PROFILE_PRINT_ERROR1("voice_set_hdl_cache: failed invalid len %d", len);
        return false;
    }
    memcpy(voice_table[conn_id].hdl_cache, p_hdl_cache, len);
    voice_table[conn_id].disc_state = DISC_VOICE_DONE;
    return true;
}

static bool voice_start_char_discovery(uint8_t conn_id)
{
    uint16_t start_handle;
    uint16_t end_handle;

    PROFILE_PRINT_INFO0("voice_start_char_discovery");
    start_handle = voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_START];
    end_handle = voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_END];
    if (client_all_char_discovery(conn_id, voice_client, start_handle,
                                  end_handle) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    return false;
}

static bool voice_start_char_descriptor_discovery(uint8_t conn_id)
{
    uint16_t start_handle;
    uint16_t end_handle;

    PROFILE_PRINT_INFO0("voice_start_char_descriptor_discovery");
    start_handle = voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_START];
    end_handle = voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_END];
    if (client_all_char_descriptor_discovery(conn_id, voice_client, start_handle,
                                             end_handle) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    return false;
}

static void voice_client_discover_state_cb(uint8_t conn_id,  T_DISCOVERY_STATE discovery_state)
{
    bool cb_flag = false;
    T_VOICE_CLIENT_CB_DATA cb_data;
    cb_data.cb_type = VOICE_CLIENT_CB_TYPE_DISC_STATE;

    PROFILE_PRINT_INFO1("voice_client_discover_state_cb: discovery_state = %d", discovery_state);
    if (voice_table[conn_id].disc_state == DISC_VOICE_START)
    {
        switch (discovery_state)
        {
        case DISC_STATE_SRV_DONE:
            /* Indicate that service handle found. Start discover characteristic. */
            if ((voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_START] != 0)
                || (voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_END] != 0))
            {
                if (voice_start_char_discovery(conn_id) == false)
                {
                    voice_table[conn_id].disc_state = DISC_VOICE_FAILED;
                    cb_flag = true;
                }
            }
            /* No VOICE handle found. Discover procedure complete. */
            else
            {
                voice_table[conn_id].disc_state = DISC_VOICE_FAILED;
                cb_flag = true;
            }
            break;
        case DISC_STATE_CHAR_DONE:
            if (voice_table[conn_id].properties & GATT_CHAR_PROP_NOTIFY)
            {
                //discovery cccd
                if (voice_start_char_descriptor_discovery(conn_id) == false)
                {
                    voice_table[conn_id].disc_state = DISC_VOICE_FAILED;
                    cb_flag = true;
                }
            }
            else
            {
                voice_table[conn_id].disc_state = DISC_VOICE_DONE;
                cb_flag = true;
            }
            break;

        case DISC_STATE_CHAR_DESCRIPTOR_DONE:
            voice_table[conn_id].disc_state = DISC_VOICE_DONE;
            cb_flag = true;
            break;

        case DISC_STATE_FAILED:
            voice_table[conn_id].disc_state = DISC_VOICE_FAILED;
            cb_flag = true;
            break;

        default:
            PROFILE_PRINT_ERROR0("Invalid Discovery State!");
            break;
        }
    }

    /* Send discover state to application if needed. */
    if (cb_flag && voice_client_cb)
    {
        cb_data.cb_content.disc_state = voice_table[conn_id].disc_state;
        (*voice_client_cb)(voice_client, conn_id, &cb_data);
    }
    return;
}


static void voice_client_discover_result_cb(uint8_t conn_id,  T_DISCOVERY_RESULT_TYPE result_type,
                                            T_DISCOVERY_RESULT_DATA result_data)
{
    PROFILE_PRINT_INFO1("voice_client_discover_result_cb: result_type = %d", result_type);
    if (voice_table[conn_id].disc_state == DISC_VOICE_START)
    {
        switch (result_type)
        {
        case DISC_RESULT_SRV_DATA:
            voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_START] =
                result_data.p_srv_disc_data->att_handle;
            voice_table[conn_id].hdl_cache[HDL_VOICE_SRV_END] =
                result_data.p_srv_disc_data->end_group_handle;
            break;

        case DISC_RESULT_CHAR_UUID16:
            {
                uint16_t handle;
                handle = result_data.p_char_uuid16_disc_data->value_handle;
                if (result_data.p_char_uuid16_disc_data->uuid16 == GATT_UUID_CHAR_VOICE_LEVEL)
                {
                    voice_table[conn_id].hdl_cache[HDL_VOICE_BATTERY_LEVEL] = handle;
                    voice_table[conn_id].properties = result_data.p_char_uuid16_disc_data->properties;
                }
            }
            break;

        case DISC_RESULT_CHAR_UUID128:
            {
                uint16_t handle;
                handle = result_data.p_char_uuid128_disc_data->value_handle;
                if (memcmp(result_data.p_char_uuid128_disc_data->uuid128, GATT_UUID_VENDOR_CHAR, 16) == 0)
                {
                    voice_table[conn_id].hdl_cache[HDL_TEMP_TYPE_DATA] = handle;
                    voice_table[conn_id].properties = result_data.p_char_uuid16_disc_data->properties;
                }
            }
            break;

        case DISC_RESULT_CHAR_DESC_UUID16:
            if (result_data.p_char_desc_uuid16_disc_data->uuid16 == GATT_UUID_CHAR_CLIENT_CONFIG)
            {
                voice_table[conn_id].hdl_cache[HDL_TEMP_TYPE_DATA] =
                    result_data.p_char_desc_uuid16_disc_data->handle;
                PROFILE_PRINT_ERROR0("DISC_RESULT_CHAR_DESC_UUID16 --> 0!");
            }
            else
            {
                PROFILE_PRINT_ERROR0("DISC_RESULT_CHAR_DESC_UUID16 --> 1!");
            }
            break;

        default:
            PROFILE_PRINT_ERROR0("Invalid Discovery Result Type!");
            break;
        }
    }

    return;
}

static void voice_client_write_result_cb(uint8_t conn_id, T_GATT_WRITE_TYPE type,
                                         uint16_t handle,
                                         uint16_t cause,
                                         uint8_t credits)
{
    T_VOICE_CLIENT_CB_DATA cb_data;
    cb_data.cb_type = VOICE_CLIENT_CB_TYPE_WRITE_RESULT;

    PROFILE_PRINT_INFO2("voice_client_write_result_cb: handle 0x%x, cause 0x%x", handle, cause);
    cb_data.cb_content.write_result.cause = cause;

//    if (handle == VOICE_DATA_HANDLE)
//    {
//        cb_data.cb_content.write_result.type = VOICE_WRITE_DATA;
//    }
//    else
//    {
//        return;
//    }

    if (voice_client_cb)
    {
        (*voice_client_cb)(voice_client, conn_id, &cb_data);
    }
    return;
}

static void voice_client_read_result_cb(uint8_t conn_id,  uint16_t cause,
                                        uint16_t handle, uint16_t value_size, uint8_t *p_value)
{
    T_VOICE_CLIENT_CB_DATA cb_data;
    uint16_t *hdl_cache;
    hdl_cache = voice_table[conn_id].hdl_cache;
    cb_data.cb_type = VOICE_CLIENT_CB_TYPE_READ_RESULT;

    PROFILE_PRINT_INFO2("voice_client_read_result_cb: handle 0x%x, cause 0x%x", handle, cause);
    cb_data.cb_content.read_result.cause = cause;

    if (handle == hdl_cache[HDL_TEMP_TYPE_DATA])
    {
        cb_data.cb_content.read_result.type = VOICE_READ_NOTIFY;
        if (cause == GAP_SUCCESS)
        {
            uint16_t ccc_bit;
            if (value_size != 2)
            {
                PROFILE_PRINT_ERROR1("voice_client_read_result_cb: invalid cccd len %d", value_size);
                return;
            }
            LE_ARRAY_TO_UINT16(ccc_bit, p_value);

            if (ccc_bit & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                cb_data.cb_content.read_result.data.notify = true;
            }
            else
            {
                cb_data.cb_content.read_result.data.notify = false;
            }
        }
    }
    else if (handle == hdl_cache[HDL_VOICE_BATTERY_LEVEL])
    {
        cb_data.cb_content.read_result.type = VOICE_READ_BATTERY_LEVEL;
        if (cause == GAP_SUCCESS)
        {
            if (value_size != 1)
            {
                PROFILE_PRINT_ERROR1("voice_client_read_result_cb: invalid battery value len %d", value_size);
                return;
            }
            cb_data.cb_content.read_result.data.battery_level = *p_value;
        }
    }
    else
    {
        return;
    }

    if (voice_client_cb)
    {
        (*voice_client_cb)(voice_client, conn_id, &cb_data);
    }
    return;
}

static T_APP_RESULT voice_client_notify_ind_cb(uint8_t conn_id, bool notify, uint16_t handle,
                                               uint16_t value_size, uint8_t *p_value)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_VOICE_CLIENT_CB_DATA cb_data;

    PROFILE_PRINT_INFO4("voice_client_notify_ind_cb: conn_id %x, notify 0x%x, handle 0x%x, value_size 0x%x",
                        conn_id, notify, handle, value_size);
    cb_data.cb_type = VOICE_CLIENT_CB_TYPE_NOTIF_IND_RESULT;

    if (handle == voice_get_handle(VOICE_KEY_HANDLE_INDEX))
    {
        PROFILE_PRINT_INFO1("[voice] voice_client_notify_ind_cb, key = %b.", TRACE_BINARY(value_size,
                            p_value));
        cb_data.cb_content.notify_data.dataType = HDL_VOICE_TYPE_KEY;
        cb_data.cb_content.notify_data.pDataBuf = p_value;
        cb_data.cb_content.notify_data.bufLength = value_size;
    }
    else if (handle == voice_get_handle(VOICE_DATA_HANDLE_INDEX))
    {
        PROFILE_PRINT_INFO0("[voice] voice_client_notify_ind_cb, voice data.");
        cb_data.cb_content.notify_data.dataType = HDL_VOICE_TYPE_DATA;
        cb_data.cb_content.notify_data.pDataBuf = p_value;
        cb_data.cb_content.notify_data.bufLength = value_size;
    }
    else if (handle == voice_get_handle(VOICE_CMD_HANDLE_INDEX))
    {
        PROFILE_PRINT_INFO0("[voice] voice_client_notify_ind_cb, voice cmd.");
        cb_data.cb_content.notify_data.dataType = HDL_VOICE_TYPE_CMD;
        cb_data.cb_content.notify_data.pDataBuf = p_value;
        cb_data.cb_content.notify_data.bufLength = value_size;
    }
    else if (handle == voice_get_handle(TEMP_DATA_HANDLE_INDEX))
    {
        PROFILE_PRINT_INFO0("[voice] voice_client_notify_ind_cb, temp data.");
        cb_data.cb_content.notify_data.dataType = HDL_TEMP_TYPE_DATA;
        cb_data.cb_content.notify_data.pDataBuf = p_value;
        cb_data.cb_content.notify_data.bufLength = value_size;
    }
    else
    {
        return APP_RESULT_SUCCESS;
    }

    if (voice_client_cb)
    {
        app_result = (*voice_client_cb)(voice_client, conn_id, &cb_data);
    }

    return app_result;
}

static void voice_client_disc_cb(uint8_t conn_id)
{
    PROFILE_PRINT_INFO0("voice_client_disc_cb.");
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_client_disc_cb: failed invalid conn_id %d", conn_id);
        return;
    }
    memset(&voice_table[conn_id], 0, sizeof(T_VOICE_LINK));
    return;
}

/**
  * @brief  Used by application, to read data from server by using handles.
  * @param[in]  conn_id connection ID.
  * @param[in]  read_type one of characteristic that has the readable property.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  */
bool voice_ble_client_read_by_handle(uint8_t conn_id, T_VOICE_READ_TYPE read_type)
{
    bool hdl_valid = false;
    uint16_t  handle;
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_ble_client_read_by_handle: failed invalid conn_id %d", conn_id);
        return false;
    }

//    switch (read_type)
//    {
//    case VOICE_READ_V1_READ:
//        if (voice_table[conn_id].hdl_cache[HDL_SIMBLE_V1_READ])
//        {
//            handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_V1_READ];
//            hdl_valid = true;
//        }
//        break;
//    case VOICE_READ_V3_NOTIFY_CCCD:
//        if (voice_table[conn_id].hdl_cache[HDL_SIMBLE_V3_NOTIFY_CCCD])
//        {
//            handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_V3_NOTIFY_CCCD];
//            hdl_valid = true;
//        }
//        break;
//    case VOICE_READ_V4_INDICATE_CCCD:
//        if (voice_table[conn_id].hdl_cache[HDL_SIMBLE_V4_INDICATE_CCCD])
//        {
//            handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_V4_INDICATE_CCCD];
//            hdl_valid = true;
//        }
//        break;

//    default:
//        return false;
//    }

    if (hdl_valid)
    {
        if (client_attr_read(conn_id, voice_client, handle) == GAP_CAUSE_SUCCESS)
        {
            return true;
        }
    }

    APP_PRINT_WARN0("voice_ble_client_read_by_handle: Request fail! Please check!");
    return false;
}

/**
  * @brief  Used by application, to read data from server by using UUIDs.
  * @param[in]  conn_id connection ID.
  * @param[in]  read_type one of characteristic that has the readable property.
  * @retval true send request to upper stack success.
  * @retval false send request to upper stack failed.
  */
bool voice_ble_client_read_by_uuid(uint8_t conn_id, T_VOICE_READ_TYPE read_type)
{
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t  uuid16;
    if (conn_id >= voice_link_num)
    {
        PROFILE_PRINT_ERROR1("voice_ble_client_read_by_uuid: failed invalid conn_id %d", conn_id);
        return false;
    }

//    switch (read_type)
//    {
//    case VOICE_READ_V1_READ:
//        start_handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_SRV_START];
//        end_handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_SRV_END];
//        uuid16 = GATT_UUID_CHAR_VOICELE_V1_READ;
//        break;
//    case VOICE_READ_V3_NOTIFY_CCCD:
//        start_handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_V3_NOTIFY];
//        end_handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_V4_INDICATE];
//        uuid16 = GATT_UUID_CHAR_CLIENT_CONFIG;
//        break;
//    case VOICE_READ_V4_INDICATE_CCCD:
//        start_handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_V4_INDICATE];
//        end_handle = voice_table[conn_id].hdl_cache[HDL_SIMBLE_SRV_END];
//        uuid16 = GATT_UUID_CHAR_CLIENT_CONFIG;
//        break;
//    default:
//        return false;
//    }

    if (client_attr_read_using_uuid(conn_id, voice_client, start_handle, end_handle,
                                    uuid16, NULL) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    return false;
}
/**
 * @brief VOICE Client Callbacks.
*/
const T_FUN_CLIENT_CBS voice_client_cbs =
{
    voice_client_discover_state_cb,   //!< Discovery State callback function pointer
    voice_client_discover_result_cb,  //!< Discovery result callback function pointer
    voice_client_read_result_cb,      //!< Read response callback function pointer
    voice_client_write_result_cb,     //!< Write result callback function pointer
    voice_client_notify_ind_cb,       //!< Notify Indicate callback function pointer
    voice_client_disc_cb              //!< Link disconnection callback function pointer
};

/**
  * @brief      Add voice client to application.
  * @param[in]  app_cb pointer of app callback function to handle specific client module data.
  * @param[in]  link_num initialize link num.
  * @return Client ID of the specific client module.
  * @retval 0xff failed.
  * @retval other success.
  *
  * <b>Example usage</b>
  * \code{.c}
    void app_le_profile_init(void)
    {
        client_init(1);
        voice_client_id = voice_add_client(app_client_callback, APP_MAX_LINKS);
    }
  * \endcode
  */
T_CLIENT_ID voice_add_client(P_FUN_GENERAL_APP_CB app_cb, uint8_t link_num)
{
    uint16_t size;
    if (link_num > VOICE_MAX_LINKS)
    {
        PROFILE_PRINT_ERROR1("voice_add_client: invalid link_num %d", link_num);
        return 0xff;
    }
    if (false == client_register_spec_client_cb(&voice_client, &voice_client_cbs))
    {
        voice_client = CLIENT_PROFILE_GENERAL_ID;
        PROFILE_PRINT_ERROR0("voice_add_client:register fail");
        return voice_client;
    }
    PROFILE_PRINT_INFO1("voice_add_client: client id %d", voice_client);

    /* register callback for profile to inform application that some events happened. */
    voice_client_cb = app_cb;
    voice_link_num = link_num;
    size = voice_link_num * sizeof(T_VOICE_LINK);
    voice_table = os_mem_zalloc(RAM_TYPE_DATA_ON, size);

    return voice_client;
}

