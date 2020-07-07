
#include "os_timer.h"
#include <trace.h>
#include "board.h"
#include "central_app.h"
#include "auto_test.h"
#include "keyscan_driver.h"
#include "app_msg.h"
#include "os_timer.h"
#include "key_handle.h"
#include "vs1053b.h"

#if AUTO_TEST_MODE


/*============================================================================*
 *                              Global Variables
 *============================================================================*/
volatile uint8_t g_auto_test_state = 0;
T_KEYSCAN_FIFIO_DATA key_data;
TimerHandle_t auto_test_timer;

/*============================================================================*
 *                              External Variables
 *============================================================================*/
extern T_APP_GLOBAL_DATA app_global_data;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
void anto_test_timer_callback(TimerHandle_t p_timer);
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
        APP_PRINT_INFO0("[sw_timer_init] init no_act_disconn_timer failed");
    }

    os_timer_restart(&auto_test_timer, AUTO_TEST_TIMER_TICK);
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
//            T_USER_CMD_PARSED_VALUE p_parse_value;
//            p_parse_value.dw_param[0] = 0x00;
//            p_parse_value.dw_param[1] = 3;
//            p_parse_value.dw_param[2] = 1;
//            cmd_voicecccd(&p_parse_value);
            g_auto_test_state = 6;

            //APP_PRINT_INFO0("[Auto] 5, temp cccd enable!");
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
#elif (AUTO_TEST_MODE == 3)
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
    /*1. send key triggle adv*/
    if (g_auto_test_state == 0)
    {
        T_IO_MSG bee_io_msg;

        if (app_global_data.is_link_key_existed)
        {
            key_data.len = 1;
            key_data.key[0].column = 0;
            key_data.key[0].row = 1;

            APP_PRINT_INFO1("[Auto][Direct] link key exist, send single keys for reconnect, \
				                    g_auto_test_state = %d!", g_auto_test_state);

            period = (uint32_t)AUTO_TEST_TIMER_TICK / 4;
        }
        else
        {
            key_data.len = 2;
            key_data.key[0].column = 0;
            key_data.key[0].row = 1;

            key_data.key[1].column = 2;
            key_data.key[1].row = 1;

            APP_PRINT_INFO1("[Auto][Direct] send comb keys for repairing, \
				                    g_auto_test_state = %d!", g_auto_test_state);

            period = (uint32_t)AUTO_TEST_TIMER_TICK;
        }

        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_RX_PKT;
        bee_io_msg.u.buf   = (void *)(&key_data);
        if (false == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto][Direct] send comb keys failed!");
        }

        /*switch to next state*/
        g_auto_test_state = 1;
    }
    /*2. send key release triggle*/
    else if (g_auto_test_state == 1)
    {
        T_IO_MSG bee_io_msg;
        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto][Direct] send key release msg!");
        }

        APP_PRINT_INFO1("[Auto][Direct] key release msg, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
        g_auto_test_state = 2;
    }
    /*3. check rcu if has been paired*/
    else if ((g_auto_test_state >= 2) && (g_auto_test_state <= 4))
    {
        //pair time out
        if (app_global_data.rcu_status != RCU_STATUS_PAIRED)
        {
            if (g_auto_test_state < 4)
            {
                g_auto_test_state ++;
            }
            else//retry 3 times, timeout
            {
                g_auto_test_state = 0;
            }
        }
        else//pair success
        {
            g_auto_test_state = 5;
        }

        APP_PRINT_INFO1("[Auto] wait for pair, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if (g_auto_test_state == 5)
    {
        key_handle_notfiy_key_data(VK_VOLUME_UP);
        key_handle_notfiy_key_data(VK_NC);
        key_handle_notfiy_key_data(VK_VOLUME_DOWN);
        key_handle_notfiy_key_data(VK_NC);

        APP_PRINT_INFO1("[Auto] send V+ and V- to host, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        g_auto_test_state = 6;
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if (g_auto_test_state >= 6)
    {
        if (app_global_data.rcu_status != RCU_STATUS_PAIRED)
        {
            g_auto_test_state = 0;//received the disconnect from host
        }
        else
        {
            g_auto_test_state ++;
        }
        APP_PRINT_INFO1("[Auto] wait for disconnect from host, \
				                    g_auto_test_state = %d!", g_auto_test_state);
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }

    /* restart auto timer */
    os_timer_restart(&auto_test_timer, period);
}

#elif (AUTO_TEST_MODE == 2)
/**
 * auto test voice
 */
void anto_test_timer_callback(TimerHandle_t p_timer)
{
    uint32_t period = 0;
    if (g_auto_test_state == 0)//send comb keys
    {
        T_IO_MSG bee_io_msg;

        key_data.len = 2;
        key_data.key[0].column = 2;
        key_data.key[0].row = 3;

        key_data.key[1].column = 2;
        key_data.key[1].row = 1;

        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_RX_PKT;
        bee_io_msg.u.buf   = (void *)(&key_data);
        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto Voice] send comb keys!");
        }

        APP_PRINT_INFO1("[Auto Voice] send comb keys for pairing, g_auto_test_state = %d!",
                        g_auto_test_state);

        period = (uint32_t)AUTO_TEST_TIMER_TICK;
        g_auto_test_state = 1;
    }
    else if (g_auto_test_state == 1)//send key release msg
    {
        T_IO_MSG bee_io_msg;
        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto Voice] send key release!");
        }

        if (app_global_data.rcu_status == RCU_STATUS_IDLE)
        {
            //start adv failed
            g_auto_test_state = 0;
            period = (uint32_t)(10);
        }
        else
        {
            g_auto_test_state = 2;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK * 3 + 500);
        }

        APP_PRINT_INFO1("[Auto Voice] key release msg, g_auto_test_state = %d!", g_auto_test_state);
    }
    else if (g_auto_test_state == 2)//wait for paired
    {
        if (app_global_data.rcu_status == RCU_STATUS_CONNECTED)
        {
            g_auto_test_state = 2;
            period = (uint32_t)(100);
            APP_PRINT_INFO1("[Auto Voice] waiting for pair, g_auto_test_state = %d!", g_auto_test_state);
        }
        else if (app_global_data.rcu_status == RCU_STATUS_PAIRED)
        {
            g_auto_test_state = 3;
            period = (uint32_t)(AUTO_TEST_TIMER_TICK);
            APP_PRINT_INFO1("[Auto Voice] check pair success, g_auto_test_state = %d!", g_auto_test_state);
        }
        else
        {
            g_auto_test_state = 0;
            period = (uint32_t)(10);
            APP_PRINT_INFO1("[Auto Voice] check pair failed, g_auto_test_state = %d!", g_auto_test_state);
        }
    }
    else if (g_auto_test_state == 3)//send voice key
    {
        T_IO_MSG bee_io_msg;

        key_data.len = 1;
        key_data.key[0].column = 0;
        key_data.key[0].row = 3;

        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_RX_PKT;
        bee_io_msg.u.buf   = (void *)(&key_data);
        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto Voice] send voice key!");
        }

        g_auto_test_state = 4;
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);

        APP_PRINT_INFO1("[Auto Voice] send voice key to host, g_auto_test_state = %d!", g_auto_test_state);
    }
    else if (g_auto_test_state == 4)//send voice key release
    {
        T_IO_MSG bee_io_msg;
        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto Voice] send voice key release!");
        }

        if (app_global_data.rcu_status == RCU_STATUS_PAIRED)
        {
            period = (uint32_t)(AUTO_TEST_TIMER_TICK);
            g_auto_test_state = 5;
        }
        else
        {
            g_auto_test_state = 0;
            period = (uint32_t)(10);
        }

        APP_PRINT_INFO1("[Auto Voice] voice key release msg, g_auto_test_state = %d!", g_auto_test_state);
    }
    else if (g_auto_test_state >= 5)
    {
        if (app_global_data.rcu_status != RCU_STATUS_PAIRED)
        {
            g_auto_test_state = 0;//received the disconnect from host
            period = (uint32_t)(10);
        }
        else
        {
            g_auto_test_state = 3;// send voice data again
            period = (uint32_t)(AUTO_TEST_TIMER_TICK);
        }
        APP_PRINT_INFO1("[Auto Voice] wait for disconnect from host, g_auto_test_state = %d!",
                        g_auto_test_state);
    }

    APP_PRINT_INFO1("[Auto Voice] rcu state = %d!",
                    app_global_data.rcu_status);
    /* restart auto timer */
    if (true == os_timer_restart(&auto_test_timer, period))
    {
        APP_PRINT_INFO1("[Auto Voice] timer start success, rcu state = %d!", app_global_data.rcu_status);
    }
    else
    {
        APP_PRINT_INFO0("[Auto Voice] timer start faile!!!");
    }
}

uint32_t auto_test_check_voice_data_file(void)
{
    uint8_t *p = (uint8_t *) AUTO_VOICE_ADDR;
    uint32_t len = 0;
    if ((*p == 0x62) && (*(p + 1) == 0x87))
    {
        len = *(p + 2) | (*(p + 3) << 8) | (*(p + 4) << 16) | (*(p + 5) << 24);
    }
    else
    {
        len = 0;
    }
    return len;
}

#elif (AUTO_TEST_MODE == 4)
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
    if (g_auto_test_state == 0)
    {
        T_IO_MSG bee_io_msg;

        key_data.len = 2;
        key_data.key[0].column = 0;
        key_data.key[0].row = 1;

        key_data.key[1].column = 2;
        key_data.key[1].row = 1;

        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_RX_PKT;
        bee_io_msg.u.buf   = (void *)(&key_data);
        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto] send comb keys!");
        }

        APP_PRINT_INFO1("[Auto] send comb keys for pairing, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        period = (uint32_t)AUTO_TEST_TIMER_TICK;
        g_auto_test_state = 1;
    }
    else if (g_auto_test_state == 1)
    {
        T_IO_MSG bee_io_msg;
        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto] send comb keys release!");
        }

        APP_PRINT_INFO1("[Auto] key release msg, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        period = (uint32_t)(AUTO_TEST_TIMER_TICK * 3);
        g_auto_test_state = 2;
    }
    else if (g_auto_test_state == 2)
    {
        //pair time out
        if (app_global_data.rcu_status != RCU_STATUS_PAIRED)
        {
            g_auto_test_state = 0;
        }
        else
        {
            g_auto_test_state = 3;
        }

        APP_PRINT_INFO1("[Auto] wait for pair, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if (g_auto_test_state == 3)
    {
        key_handle_notfiy_key_data(VK_VOLUME_UP);
        key_handle_notfiy_key_data(VK_NC);
        key_handle_notfiy_key_data(VK_VOLUME_DOWN);
        key_handle_notfiy_key_data(VK_NC);

        APP_PRINT_INFO1("[Auto] send V+ and V- to host, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        g_auto_test_state = 4;
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if ((g_auto_test_state >= 4) && (g_auto_test_state < 7))/*power off TV*/
    {
        if (app_global_data.rcu_status == RCU_STATUS_PAIRED)
        {
            key_handle_notfiy_key_data(VK_POWER);
            key_handle_notfiy_key_data(VK_NC);

            g_auto_test_state = 7;
        }
        else
        {
            APP_PRINT_INFO0("[Auto] send power off TV failed, retry ");
            g_auto_test_state ++;
        }

        APP_PRINT_INFO1("[Auto] power off TV, \
				                    g_auto_test_state = %d!", g_auto_test_state);
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if (g_auto_test_state == 7)/*wait for the disconnect from host*/
    {
        if (app_global_data.rcu_status != RCU_STATUS_PAIRED)
        {
            g_auto_test_state = 8;
        }
        else
        {
            g_auto_test_state = 7;//forever
        }

        APP_PRINT_INFO1("[Auto] wait for disconnect from host, \
				                    g_auto_test_state = %d!", g_auto_test_state);
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if (g_auto_test_state == 8)/*power key press*/
    {
        if (app_global_data.rcu_status != RCU_STATUS_PAIRED)
        {
            T_IO_MSG bee_io_msg;

            key_data.len = 1;
            key_data.key[0].column = 0;
            key_data.key[0].row = 0;

            bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
            bee_io_msg.subtype = IO_MSG_KEYSCAN_RX_PKT;
            bee_io_msg.u.buf   = (void *)(&key_data);
            if (true == app_send_msg_to_apptask(&bee_io_msg))
            {
                APP_PRINT_ERROR0("[Auto] send power key!");
            }

            APP_PRINT_INFO1("[Auto] send power key for power on, \
																g_auto_test_state = %d!", g_auto_test_state);

            period = (uint32_t)AUTO_TEST_TIMER_TICK;
            g_auto_test_state = 9;
        }
        else
        {
            g_auto_test_state = 7;//forever
        }

        APP_PRINT_INFO1("[Auto] wait for disconnect from host, \
				                    g_auto_test_state = %d!", g_auto_test_state);
        period = (uint32_t)(AUTO_TEST_TIMER_TICK);
    }
    else if (g_auto_test_state == 9)/*power key release*/
    {
        T_IO_MSG bee_io_msg;
        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

        if (true == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[Auto] send power key release!");
        }

        APP_PRINT_INFO1("[Auto] key release msg, \
				                    g_auto_test_state = %d!", g_auto_test_state);

        period = (uint32_t)(AUTO_TEST_TIMER_TICK * 3);
        g_auto_test_state = 0;
    }


    /* restart auto timer */
    os_timer_restart(&auto_test_timer, period);
}
#endif
#endif
