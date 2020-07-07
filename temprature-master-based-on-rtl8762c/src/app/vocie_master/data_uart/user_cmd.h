/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     user_cmd.h
* @brief    Define user command.
* @details
* @author   jane
* @date     2016-02-18
* @version  v0.1
*********************************************************************************************************
*/
#ifndef _USER_CMD_H_
#define _USER_CMD_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

#include <data_uart.h>
#include <user_cmd_parse.h>

extern const T_USER_CMD_TABLE_ENTRY user_cmd_table[];
extern T_USER_CMD_IF    user_cmd_if;

T_USER_CMD_PARSE_RESULT cmd_scan(T_USER_CMD_PARSED_VALUE *p_parse_value);
T_USER_CMD_PARSE_RESULT cmd_stopscan(T_USER_CMD_PARSED_VALUE *p_parse_value);
T_USER_CMD_PARSE_RESULT cmd_showdev(T_USER_CMD_PARSED_VALUE *p_parse_value);
T_USER_CMD_PARSE_RESULT cmd_condev(T_USER_CMD_PARSED_VALUE *p_parse_value);
T_USER_CMD_PARSE_RESULT cmd_sauth(T_USER_CMD_PARSED_VALUE *p_parse_value);
T_USER_CMD_PARSE_RESULT cmd_voicecccd(T_USER_CMD_PARSED_VALUE *p_parse_value);

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif

