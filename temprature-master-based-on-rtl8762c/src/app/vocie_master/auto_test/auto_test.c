
#include "os_timer.h"
#include <trace.h>
#include "board.h"
#include "central_app.h"
#include "auto_test.h"
#include "app_msg.h"
#include "os_timer.h"

#include "uart_transport.h"

#if AUTO_TEST_MODE


/*============================================================================*
 *                              Global Variables
 *============================================================================*/
volatile uint8_t g_auto_test_state = 0;
typedef void *TimerHandle_t;
TimerHandle_t auto_test_timer;

volatile uint8_t g_auto_esp8266_state = 0;
TimerHandle_t auto_esp8266_timer;
/*============================================================================*
 *                              External Variables
 *============================================================================*/
extern T_APP_GLOBAL_DATA app_global_data;
extern UartLoopQueue_TypeDef   UartLoopQueue;
extern const stg_AT_Cmd AtCMD[];
/*============================================================================*
 *                              Local Functions
 *============================================================================*/
void anto_test_timer_callback(TimerHandle_t p_timer);
void anto_esp8266_timer_callback(TimerHandle_t p_timer);
extern bool app_send_msg_to_apptask(T_IO_MSG *p_msg);

/**
 * @brief auto test init.
 */
void auto_test_init(void)
{
    /* no_act_disconn_timer is used to disconnect after timeout if there is on action under connection */
    if (false == os_timer_create(&auto_test_timer, "auto_test_timer",  1, \
                                 AUTO_TEST_TIMER_TICK, false, anto_test_timer_callback))
    {
        APP_PRINT_INFO0("[Auto] init auto_test_timer failed!");
    }

    //os_timer_restart(&auto_test_timer, AUTO_TEST_TIMER_TICK);

    if (false == os_timer_create(&auto_esp8266_timer, "auto_esp8266_timer",  1, \
                                 AUTO_TEST_TIMER_TICK, false, anto_esp8266_timer_callback))
    {
        APP_PRINT_INFO0("[Auto] init auto_esp8266_timer failed!");
    }
    os_timer_restart(&auto_esp8266_timer, AUTO_TEST_TIMER_TICK);
}

#if (AUTO_TEST_MODE == 1)
#include "user_cmd_parse.h"

extern T_USER_CMD_PARSE_RESULT cmd_scan(T_USER_CMD_PARSED_VALUE *p_parse_value);
extern T_USER_CMD_PARSE_RESULT cmd_stopscan(T_USER_CMD_PARSED_VALUE *p_parse_value);
extern T_USER_CMD_PARSE_RESULT cmd_condev(T_USER_CMD_PARSED_VALUE *p_parse_value);
extern T_USER_CMD_PARSE_RESULT cmd_sauth(T_USER_CMD_PARSED_VALUE *p_parse_value);
extern T_USER_CMD_PARSE_RESULT cmd_voicecccd(T_USER_CMD_PARSED_VALUE *p_parse_value);
extern T_USER_CMD_PARSE_RESULT cmd_voicehdl(T_USER_CMD_PARSED_VALUE *p_parse_value);

/**
* auto_esp8266_timer callback
*
*/
void anto_esp8266_timer_callback(TimerHandle_t p_timer)
{
    uint32_t period = AUTO_TEST_TIMER_TICK;


    if (LoopQueueIsEmpty(&UartLoopQueue) == false)
    {
        //check state
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_GOT_IP_INDEX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] got esp8266 ip!");
            g_auto_esp8266_state = 6;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else if (0 == LoopQueueFindString(&UartLoopQueue,
                                          (const char *)AtCMD[ESP_WIFI_DISCONNECT_INDEX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 disconnect!");
            g_auto_esp8266_state = 0;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else if (0 == LoopQueueFindString(&UartLoopQueue,
                                          (const char *)AtCMD[ESP_WIFI_STATUS_DISCONNECT_INDEX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 status disconnect!");
            g_auto_esp8266_state = 0;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else if (0 == LoopQueueFindString(&UartLoopQueue,
                                          (const char *)AtCMD[ESP_WIFI_STATUS_CONNECT_INDEX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 status connect!");
            //keep status
            //g_auto_esp8266_state = 1;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto]  --->!");
        }
    }

    if (g_auto_esp8266_state == 0)
    {
        LoopQueueClear(&UartLoopQueue);
        ESP8266_Cmd_Send((uint8_t *)AtCMD[ESP_GET_MODE].send_cmd);
        g_auto_esp8266_state = 1;
        period = AUTO_TEST_TIMER_TICK / 4;
        APP_PRINT_INFO0("[Auto]  esp8266 status check>!");
    }
    else if (g_auto_esp8266_state == 1)//check mode
    {
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_GET_MODE].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 status ok!");
            //keep status
            g_auto_esp8266_state = 2;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto] esp8266 status not ok, wait!");
            //keep status
            g_auto_esp8266_state = 2;
            period = AUTO_TEST_TIMER_TICK / 4;

            //LoopQueueClear(&UartLoopQueue);
        }
    }
    else if (g_auto_esp8266_state == 2)//scan ap
    {
        LoopQueueClear(&UartLoopQueue);
        ESP8266_Cmd_Send((uint8_t *)AtCMD[ESP_SCAN_WIFI].send_cmd);
        g_auto_esp8266_state = 3;
        period = AUTO_TEST_TIMER_TICK / 4;
        APP_PRINT_INFO0("[Auto]  esp8266 scan wifi cmd >!");
    }
    else if (g_auto_esp8266_state == 3)
    {
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_SCAN_WIFI].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 scan wifi ok!");
            //keep status
            g_auto_esp8266_state = 4;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto] esp8266 scan status not ok, wait!");
            //keep status
            g_auto_esp8266_state = 3;
            period = AUTO_TEST_TIMER_TICK / 4;

            //LoopQueueClear(&UartLoopQueue);
        }
    }
    else if (g_auto_esp8266_state == 4)//connect ap
    {
        LoopQueueClear(&UartLoopQueue);
        ESP8266_Cmd_Send((uint8_t *)AtCMD[ESP_CONNECT_AP].send_cmd);
        g_auto_esp8266_state = 5;
        period = AUTO_TEST_TIMER_TICK / 4;
        APP_PRINT_INFO0("[Auto]  esp8266 connect ap cmd >!");
    }
    else if (g_auto_esp8266_state == 5)//connect ap
    {
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_CONNECT_AP].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 connect ap ok!");
            //keep status
            g_auto_esp8266_state = 6;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto] esp8266 connect ap not ok, wait!");
            //keep status
            g_auto_esp8266_state = 4;
            period = AUTO_TEST_TIMER_TICK / 4;

            //LoopQueueClear(&UartLoopQueue);
        }
    }
    /*esp set link mux*/
    else if (g_auto_esp8266_state == 6)
    {
        LoopQueueClear(&UartLoopQueue);
        ESP8266_Cmd_Send((uint8_t *)AtCMD[ESP_SET_LINK_MUX].send_cmd);
        g_auto_esp8266_state = 7;
        period = AUTO_TEST_TIMER_TICK / 4;
        APP_PRINT_INFO0("[Auto]  esp8266 send set link mux cmd >!");
    }
    else if (g_auto_esp8266_state == 7)
    {
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_SET_LINK_MUX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 set link mux ok!");
            //keep status
            g_auto_esp8266_state = 8;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto] esp8266 set link mux not ok, wait!");
            //keep status
            g_auto_esp8266_state = 6;
            period = AUTO_TEST_TIMER_TICK / 4;

            //LoopQueueClear(&UartLoopQueue);
        }
    }
    /*config server*/
    else if (g_auto_esp8266_state == 8)
    {
        LoopQueueClear(&UartLoopQueue);
        ESP8266_Cmd_Send((uint8_t *)AtCMD[ESP_CONFIG_SERER_INDEX].send_cmd);
        g_auto_esp8266_state = 9;
        period = AUTO_TEST_TIMER_TICK / 4;
        APP_PRINT_INFO0("[Auto]  esp8266 send config server cmd >!");
    }
    else if (g_auto_esp8266_state == 9)
    {
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_CONFIG_SERER_INDEX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 config server ok!");
            //keep status
            g_auto_esp8266_state = 10;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto] esp8266 config server not ok, wait!");
            //keep status
            g_auto_esp8266_state = 8;
            period = AUTO_TEST_TIMER_TICK / 4;

            //LoopQueueClear(&UartLoopQueue);
        }
    }

    /*got ip address*/
    else if (g_auto_esp8266_state == 10)
    {
        LoopQueueClear(&UartLoopQueue);
        ESP8266_Cmd_Send((uint8_t *)AtCMD[ESP_CHECK_IP_ADDR_INDEX].send_cmd);
        g_auto_esp8266_state = 11;
        period = AUTO_TEST_TIMER_TICK / 4;
        APP_PRINT_INFO0("[Auto]  esp8266 send get ip address cmd >!");
    }
    else if (g_auto_esp8266_state == 11)
    {
        if (0 == LoopQueueFindString(&UartLoopQueue, (const char *)AtCMD[ESP_CHECK_IP_ADDR_INDEX].ret_str))
        {
            APP_PRINT_INFO0("[Auto] esp8266 get ip address ok!");
            //keep status
            g_auto_esp8266_state = 12;
            period = AUTO_TEST_TIMER_TICK / 4;

            LoopQueueClear(&UartLoopQueue);
        }
        else
        {
            APP_PRINT_INFO0("[Auto] esp8266 get ip address not ok, wait!");
            //keep status
            g_auto_esp8266_state = 10;
            period = AUTO_TEST_TIMER_TICK / 4;

            //LoopQueueClear(&UartLoopQueue);
        }
    }
    else
    {
        APP_PRINT_INFO0("[Auto] esp8266 no action, clear queue, wait!");
        LoopQueueClear(&UartLoopQueue);
    }

    APP_PRINT_INFO1("[Auto] anto_esp8266_timer_callback, g_auto_esp8266_state = %d!",
                    g_auto_esp8266_state);
    /* restart auto timer */
    if (true == os_timer_restart(&auto_esp8266_timer, period))
    {
        //APP_PRINT_INFO0("[Auto] auto_esp8266_timer restart success!");
    }
    else
    {
        APP_PRINT_INFO0("[Auto] auto_esp8266_timer restart failed!");
    }
}
/**
 *  auto test callback for pair and unpair.
 *
  * 0 idle state
    * 1 send comb keys
    * 2 send key release
    * 3 pair success
    * 4 send key V+ and V-
    * >4 wait for disconnect
 */
void anto_test_timer_callback(TimerHandle_t p_timer)
{
    uint32_t period = 0;
    APP_PRINT_INFO1("[Auto] app_global_data.device_ble_status = %d!",
                    app_global_data.device_ble_status);
    if (g_auto_test_state == 0)//start scan
    {
        if ((app_global_data.device_ble_status == DEVICE_STATUS_IDLE)
            || (app_global_data.device_ble_status == DEVICE_STATUS_SCAN_STOPED))
        {
            T_USER_CMD_PARSED_VALUE p_parse_value;
            p_parse_value.param_count = 1;
            p_parse_value.dw_param[0] = 0;
            if (RESULT_SUCESS == cmd_scan(&p_parse_value))
            {
                app_global_data.is_scan_device_hit = false;
                g_auto_test_state = 1;
                app_global_data.device_ble_status = DEVICE_STATUS_SCAN_STARTING;
            }
            else
            {
                g_auto_test_state = 0;
            }
        }
        else
        {
            g_auto_test_state = 0;
        }

        APP_PRINT_INFO1("[Auto] scan device start, g_auto_test_state = %d!", g_auto_test_state);
        period = (uint32_t)AUTO_TEST_TIMER_TICK;

    }
    else if (g_auto_test_state == 1)//stop scan
    {
        if (app_global_data.device_ble_status == DEVICE_STATUS_SCAN_STARTING)
        {
            g_auto_test_state = 1;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK / 4);
        }
        else if (app_global_data.device_ble_status == DEVICE_STATUS_SCAN)
        {
            if (app_global_data.is_scan_device_hit == true)
            {
                //stop scan
                if (RESULT_SUCESS == cmd_stopscan(0))
                {
                    g_auto_test_state = 2;
                    period = (uint32_t)(AUTO_TEST_TIMER_TICK / 16);
                    app_global_data.device_ble_status = DEVICE_STATUS_SCAN_STOPING;
                }
            }
            else
            {
                g_auto_test_state = 1;
                period = (uint32_t)(AUTO_TEST_TIMER_TICK / 4);
            }
        }
        else
        {
            g_auto_test_state = 0;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK / 4);
        }

        APP_PRINT_INFO1("[Auto] stop scan device, g_auto_test_state = %d!", g_auto_test_state);
    }
    else if (g_auto_test_state == 2)//connect
    {
        //connect device
        if (app_global_data.device_ble_status == DEVICE_STATUS_SCAN_STOPING)
        {
            g_auto_test_state = 2;
        }
        else if (app_global_data.device_ble_status == DEVICE_STATUS_SCAN_STOPED)
        {
            if (app_global_data.is_scan_device_hit == true)
            {
                T_USER_CMD_PARSED_VALUE p_parse_value;
                p_parse_value.dw_param[0] = 0;

                if (RESULT_SUCESS == cmd_condev(&p_parse_value))
                {
                    APP_PRINT_INFO0("[Auto] scan hit and connect device success , auto state = 4!");
                    g_auto_test_state = 4;
                }
                else
                {
                    APP_PRINT_INFO0("[Auto] scan hit and connect device failed , auto state = 0!");
                    g_auto_test_state = 0;
                }
            }
            else
            {
                APP_PRINT_INFO0("[Auto] scan not hit no need connect auto state = 0!");
                g_auto_test_state = 0;
            }
        }
        else
        {
            APP_PRINT_INFO0("[Auto] 2, unexpected state return auto state = 0!");
            g_auto_test_state = 0;
        }
        period = (uint32_t)(AUTO_TEST_TIMER_TICK / 4);
    }
    else if (g_auto_test_state == 4)
    {
        //pair time out
        APP_PRINT_INFO1("[Auto] 4, app_global_data.device_ble_status = %d!",
                        app_global_data.device_ble_status);
        if ((app_global_data.device_ble_status == DEVICE_STATUS_CONNECTED)
            || (app_global_data.device_ble_status == DEVICE_STATUS_PAIRED))
        {
            T_USER_CMD_PARSED_VALUE p_parse_value;
            p_parse_value.dw_param[0] = 0x00;

            cmd_voicehdl(&p_parse_value);

            g_auto_test_state = 5;
            APP_PRINT_INFO0("[Auto] cccd enable!");
        }
        else if (app_global_data.device_ble_status == DEVICE_STATUS_SCAN_STOPED)
        {
            APP_PRINT_INFO0("[Auto] 4, wait for stopped state exit!");
            g_auto_test_state = 4;
        }
        else
        {
            APP_PRINT_INFO0("[Auto] 4, state error return state 0!");
            g_auto_test_state = 0;
        }

        period = (uint32_t)(AUTO_TEST_TIMER_TICK / 8);
    }
    else if (g_auto_test_state == 5)
    {
        if (app_global_data.device_ble_status == DEVICE_STATUS_SERVICE_DISCOV_DONE)
        {
            g_auto_test_state = 6;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK);
        }
        else if ((app_global_data.device_ble_status == DEVICE_STATUS_PAIRED)
                 || (app_global_data.device_ble_status == DEVICE_STATUS_CONNECTED))
        {
            g_auto_test_state = 5;
            APP_PRINT_INFO0("[Auto] 5, wait temp service discovery process done!");
            period = (uint32_t)(AUTO_TEST_TIMER_TICK / 8);
        }
        else
        {
            g_auto_test_state = 0;
            APP_PRINT_INFO0("[Auto] 5, state error return state 0!");
            period = (uint32_t)(AUTO_TEST_TIMER_TICK / 8);
        }
    }
    else if (g_auto_test_state == 6)
    {
        static bool flag = 0;
        if (app_global_data.device_ble_status == DEVICE_STATUS_SERVICE_DISCOV_DONE)
        {
            g_auto_test_state = 6;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK / 4);

            if (false == flag)
            {
                flag = 1;

                T_USER_CMD_PARSED_VALUE p_parse_value;
                p_parse_value.dw_param[0] = 0x00;
                p_parse_value.dw_param[1] = 3;
                p_parse_value.dw_param[2] = 1;
                cmd_voicecccd(&p_parse_value);

                APP_PRINT_INFO0("[Auto] 6, temp cccd enable!");
            }
            else
            {
                g_auto_test_state = 6;
                APP_PRINT_INFO0("[Auto] 6, state check !");
            }

        }
        else
        {
            APP_PRINT_INFO0("[Auto] 6, state change , switch to state 0!");
            g_auto_test_state = 0;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK / 4);
        }
        APP_PRINT_INFO1("[Auto] g_auto_test_state = %d!", g_auto_test_state);
    }
    /* restart auto timer */
    if (true == os_timer_restart(&auto_test_timer, period))
    {
        APP_PRINT_INFO0("[Auto] timer restart success!");
    }
    else
    {
        APP_PRINT_INFO0("[Auto] timer restart failed!");
    }
}
#endif
#endif
