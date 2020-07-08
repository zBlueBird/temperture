/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      data_uart_test.c
* @brief    This file provides data uart test.
* @details
* @author  yuyin_zhang
* @date     2020-01-13
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "board.h"
#include <data_uart_test.h>
#include <version.h>
#include <patch_header_check.h>
#include <gap.h>
#include <string.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "voice_s2m.h"
#include "rtl876x_uart.h"
#include "gap_bond_le.h"
#include "os_sched.h"
#include "user_cmd.h"
#include "user_cmd_parse.h"

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
//static bool uart_test_is_dlps_allowed = true;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
void voice_scan_start(void *p_data);
void voice_scan_stop(void *p_data);
void voice_show_device(void *p_data);
void voice_connect_device(void *p_data);
void voice_pair_device(void *p_data);
void voice_cccd_write(void *p_data);
void voice_mac_config(void *p_data);
void voice_list_config(void *p_data);
void uart_test_get_cmd_func(UART_PacketTypeDef *p_data);

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
/**<  Array of all used test function informations */
const T_UART_TEST_PROTOCOL uart_test_func_map[UART_TEST_SUPPORT_NUM] =
{
    /* Opcode, Parameter Length, Function */
    {VOICE_CMD_SCAN_DEVICE, 0, voice_scan_start},
    {VOICE_CMD_STOP_SCAN,   0, voice_scan_stop},
    {VOICE_CMD_SHOW_DEVICE, 0, voice_show_device},
    {VOICE_CMD_CONN_DEVICE, 0, voice_connect_device},
    {VOICE_CMD_AUTH_DEVICE, 0, voice_pair_device},
    {VOICE_CMD_CCCD_WRITE, 2, voice_cccd_write},
    {VOICE_CMD_MAC_CONFIG, 6, voice_mac_config},
    {SET_VOICE_CONFIG_CMD, 32, NULL},
    {GET_VOICE_CONFIG_CMD, 0, NULL},
    {ENTER_DLPS_TEST_MODE_CMD, 0, NULL},
    {START_STOP_ADV_CMD, 1, NULL},
    {START_IR_TEST_MODE_CMD, 0, NULL},
    {ENTER_HCI_TEST_MODE_CMD, 0, NULL},
    {DISABLE_TEST_MODE_FLG_CMD, 0, NULL},
    {ENABLE_TEST_MODE_FLG_CMD, 0, NULL},
    {ERASE_PAIR_INFO_CMD, 0, NULL},
    {CHANGE_BAUDRATE_CMD, 1, NULL},
    {DIRECT_K_RF_FREQ_CMD, 2, NULL},
    {GET_GLODEN_INFO_CMD, 0, NULL},
    {GET_DUT_INFO_CMD, 32, NULL},
    {VERIFY_DUT_INFO_CMD, 0, NULL},
    {AUTO_K_RF_FREQ_CMD, 18, NULL},
    {FIND_DEVICE_TYPE_CMD, 0, NULL},
    {REBOOT_DEVICE_CMD, 0, NULL},
    {UPDATE_MAC_ADDR_CMD, 0, NULL},
    {ENTER_SINGLE_TONE_MODE_CMD, 1, NULL},
    {READ_HARDWARE_VERSION_CMD, 0, NULL},
    {TERMINATE_CONNECT_CMD, 0, NULL},
    {MANUAL_K_RF_FREQ_CMD, 1, NULL},
    {ENTER_HCI_DOWNLOAD_MODE_CMD, 0, NULL},
    {SET_CODEC_EQ_CONFIG_CMD, 0, NULL},
    {GET_CODEC_EQ_CONFIG_CMD, 0, NULL},

    //Add more command here,Please store in order according to opcode!
};

void voice_scan_start(void *p_data)
{

    T_USER_CMD_PARSED_VALUE p_parse_value;
    p_parse_value.param_count = 1;
    p_parse_value.dw_param[0] = 0;
    cmd_scan(&p_parse_value);

    /* Response data */
    UARTCmd_Response(VOICE_CMD_SCAN_DEVICE, UART_TEST_SUCCESS, NULL, 0);
}

void voice_scan_stop(void *p_data)
{

    cmd_stopscan(NULL);

    /* Response data */
    UARTCmd_Response(VOICE_CMD_STOP_SCAN, UART_TEST_SUCCESS, NULL, 0);
}

void voice_show_device(void *p_data)
{

    cmd_showdev(NULL);

    /* Response data */
    UARTCmd_Response(VOICE_CMD_SHOW_DEVICE, UART_TEST_SUCCESS, NULL, 0);
}

void voice_connect_device(void *p_data)
{
    T_USER_CMD_PARSED_VALUE p_parse_value;
    p_parse_value.dw_param[0] = 0;
    cmd_condev(&p_parse_value);

    /* Response data */
    UARTCmd_Response(VOICE_CMD_CONN_DEVICE, UART_TEST_SUCCESS, NULL, 0);
}

void voice_pair_device(void *p_data)
{
    T_USER_CMD_PARSED_VALUE p_parse_value;
    p_parse_value.dw_param[0] = 0x00;
    cmd_sauth(&p_parse_value);

    /* Response data */
    UARTCmd_Response(VOICE_CMD_AUTH_DEVICE, UART_TEST_SUCCESS, NULL, 0);
}

void voice_cccd_write(void *p_data)
{
    T_USER_CMD_PARSE_RESULT ret = RESULT_SUCESS;
    T_USER_CMD_PARSED_VALUE p_parse_value;
    p_parse_value.dw_param[0] = 0x00;
    p_parse_value.dw_param[1] = *(uint32_t *)((uint8_t *)p_data + 3);
    p_parse_value.dw_param[2] = *(uint32_t *)((uint8_t *)p_data + 4);
    ret = cmd_voicecccd(&p_parse_value);

    /* Response data */
    if (ret == RESULT_SUCESS)
    {
        UARTCmd_Response(VOICE_CMD_CCCD_WRITE, UART_TEST_SUCCESS, NULL, 0);
    }
    else
    {
        UARTCmd_Response(VOICE_CMD_CCCD_WRITE, UART_TEST_ERROR, NULL, 0);
    }
}

void voice_mac_config(void *p_data)
{
    if (true == voice_filter_mac_config(p_data))
    {
        UARTCmd_Response(VOICE_CMD_MAC_CONFIG, UART_TEST_SUCCESS, NULL, 0);
    }
    else
    {
        UARTCmd_Response(VOICE_CMD_MAC_CONFIG, UART_TEST_ERROR, NULL, 0);
    }
}

/**
 * @brief handle UART message
 * @param io_driver_msg_recv - recieved io message
 * @return none
 * @retval void
 */
void uart_test_handle_uart_msg(T_IO_MSG io_driver_msg_recv)
{
    UART_PacketTypeDef *pUartTestPacket = (UART_PacketTypeDef *)(io_driver_msg_recv.u.buf);

//    APP_PRINT_INFO1("[Temp] uart_test_handle_uart_msg:  value %b",
//                                    TRACE_BINARY(4, (uint8_t*)io_driver_msg_recv.u.buf));
    if (Packet_Decode(pUartTestPacket))
    {
        ;//uart_test_get_cmd_func(pUartTestPacket);
    }
}
/**
  * @brief  Get the spefied uart command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_get_cmd_func(UART_PacketTypeDef *p_data)
{
    uint16_t opcode = (p_data->Buf[2] << 8) + p_data->Buf[1];

    UART_DBG_BUFFER(MODULE_UART, LEVEL_INFO, "[uart_test_get_cmd_func] opcode is 0x%04X", 1, opcode);

    /* uart test command */
    if ((opcode >= UART_FN_BEGIN) && (opcode < UART_FN_END))
    {
        if (uart_test_func_map[opcode & UART_FN_MASK].fncb != NULL)
        {
            uart_test_func_map[opcode & UART_FN_MASK].fncb(p_data);
        }
        else
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "No uart test cmd function!", 0);
        }
    }
    else
    {
        //Other command
    }
}
/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

