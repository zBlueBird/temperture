/**
*****************************************************************************************
*     Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     voice_client.h
  * @brief    Head file for using battery service client.
  * @details  Battery service client data structs and external functions declaration.
  * @author   jane
  * @date     2016-02-18
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _VOICE_CLIENT_H_
#define _VOICE_CLIENT_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Add Includes here */
#include <profile_client.h>
#include <stdint.h>
#include <stdbool.h>



/** @defgroup VOICE_CLIENT Battery Service Client
  * @brief VOICE client
  * @details
     Application shall register voice client when initialization through @ref voice_add_client function.

     Application can start discovery battery service through @ref voice_start_discovery function.

     Application can read battery level characteristic value through @ref voice_read_battery_level function.

     Application can config and read the notification flag through @ref voice_set_notify and @ref voice_read_notify function.

     Application shall handle callback function registered by voice_add_client.
  * \code{.c}
    T_APP_RESULT app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data)
    {
        T_APP_RESULT  result = APP_RESULT_SUCCESS;
        if (client_id == voice_client_id)
        {
            T_VOICE_CLIENT_CB_DATA *p_voice_cb_data = (T_VOICE_CLIENT_CB_DATA *)p_data;
            switch (p_voice_cb_data->cb_type)
            {
            case VOICE_CLIENT_CB_TYPE_DISC_STATE:
                switch (p_voice_cb_data->cb_content.disc_state)
                {
                case DISC_VOICE_DONE:
                ......
        }
    }
  * \endcode
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @addtogroup VOICE_CLIENT_Exported_Macros VOICE Client Exported Macros
  * @brief
  * @{
  */

/** @brief  Define links number. range: 0-4 */
#define VOICE_MAX_LINKS  4

/** End of VOICE_CLIENT_Exported_Macros
* @}
*/


/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup VOICE_CLIENT__Exported_Types VOICE Client Exported Types
  * @brief
  * @{
  */

/** @brief VOICE client handle type*/
typedef enum
{
    HDL_VOICE_SRV_START,           //!< start handle of battery service
    HDL_VOICE_SRV_END,             //!< end handle of battery service
    HDL_VOICE_BATTERY_LEVEL,       //!< battery level characteristic value handle
    HDL_VOICE_TYPE_DATA,           //!< battery level characteristic CCCD handle
    HDL_VOICE_TYPE_KEY,
    HDL_VOICE_TYPE_CMD,
    HDL_TEMP_TYPE_DATA,
    HDL_VOICE_CACHE_LEN            //!< handle cache length
} T_VOICE_HANDLE_TYPE;

/** @brief VOICE client discovery state*/
typedef enum
{
    DISC_VOICE_IDLE,
    DISC_VOICE_START,
    DISC_VOICE_DONE,
    DISC_VOICE_FAILED
} T_VOICE_DISC_STATE;

/** @brief VOICE client notification data struct*/
typedef struct
{
    uint8_t  dataType;
    uint16_t bufLength;
    uint8_t *pDataBuf;
} T_VOICE_NOTIFY_DATA;

/** @brief VOICE client write type*/
typedef enum
{
    VOICE_WRITE_NOTIFY_ENABLE,
    VOICE_WRITE_NOTIFY_DISABLE,
    VOICE_WRITE_DATA,
} T_VOICE_WRTIE_TYPE;

/** @brief VOICE client write result*/
typedef struct
{
    T_VOICE_WRTIE_TYPE type;
    uint16_t cause;
} T_VOICE_WRITE_RESULT;

/** @brief VOICE client read data */
typedef union
{
    uint8_t battery_level;
    bool notify;
} T_VOICE_READ_DATA;

/** @brief VOICE client read type*/
typedef enum
{
    VOICE_READ_NOTIFY,
    VOICE_READ_BATTERY_LEVEL,
} T_VOICE_READ_TYPE;

/** @brief VOICE client read result*/
typedef struct
{
    T_VOICE_READ_TYPE type;
    T_VOICE_READ_DATA data;
    uint16_t cause;
} T_VOICE_READ_RESULT;

/** @brief VOICE client callback type*/
typedef enum
{
    VOICE_CLIENT_CB_TYPE_DISC_STATE,          //!< Discovery procedure state, done or pending.
    VOICE_CLIENT_CB_TYPE_READ_RESULT,         //!< Read request's result data, responsed from server.
    VOICE_CLIENT_CB_TYPE_WRITE_RESULT,        //!< Write request result, success or fail.
    VOICE_CLIENT_CB_TYPE_NOTIF_IND_RESULT,    //!< Notification or indication data received from server.
    VOICE_CLIENT_CB_TYPE_INVALID              //!< Invalid callback type, no practical usage.
} T_VOICE_CLIENT_CB_TYPE;

/** @brief VOICE client callback content*/
typedef union
{
    T_VOICE_DISC_STATE disc_state;
    T_VOICE_READ_RESULT read_result;
    T_VOICE_NOTIFY_DATA notify_data;
    T_VOICE_WRITE_RESULT write_result;
} T_VOICE_CLIENT_CB_CONTENT;

/** @brief VOICE client callback data*/
typedef struct
{
    T_VOICE_CLIENT_CB_TYPE     cb_type;
    T_VOICE_CLIENT_CB_CONTENT  cb_content;
} T_VOICE_CLIENT_CB_DATA;
/** End of VOICE_CLIENT_Exported_Types
* @}
*/


/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup VOICE_CLIENT_Exported_Functions VOICE Client Exported Functions
  * @brief
  * @{
  */

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
T_CLIENT_ID voice_add_client(P_FUN_GENERAL_APP_CB app_cb, uint8_t link_num);

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
bool voice_start_discovery(uint8_t conn_id);

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
bool voice_set_notify(uint8_t conn_id, uint8_t handle_index, bool notify);

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
bool voice_read_notify(uint8_t conn_id);

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
bool voice_read_battery_level(uint8_t conn_id);

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
bool voice_get_hdl_cache(uint8_t conn_id, uint16_t *p_hdl_cache, uint8_t len);

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
bool voice_set_hdl_cache(uint8_t conn_id, uint16_t *p_hdl_cache, uint8_t len);

/** @} End of VOICE_CLIENT_Exported_Functions */

/** @} End of VOICE_CLIENT */

bool voice_ble_client_write_cmd(uint8_t conn_id, uint16_t length, uint8_t *p_value,
                                T_GATT_WRITE_TYPE type);


extern T_CLIENT_ID voice_client;

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif  /* _VOICE_CLIENT_H_ */
