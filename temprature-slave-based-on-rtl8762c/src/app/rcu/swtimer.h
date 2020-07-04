/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      swtimer.h
* @brief     header file of software timer implementation
* @details
* @author    KEN_MEI
* @date      2017-02-08
* @version   v0.1
* *********************************************************************************************************
*/

#ifndef _SWTIMER__
#define _SWTIMER__

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <board.h>

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define ADV_UNDIRECT_RECONNECT_TIMEOUT 3000  /* 3s */
#define ADV_UNDIRECT_POWER_TIMEOUT 12000  /* 12s */
#define ADV_UNDIRECT_PAIRING_TIMEOUT 20000  /* 20s */
#define ADV_UNDIRECT_PROMPT_TIMEOUT 5000  /* 5s */
#define PAIRING_EXCEPTION_TIMEOUT 10000  /* 10s */
#define NO_ACTION_DISCON_TIMEOUT 300000  /* 300s */
#define UPDATE_CONN_PARAMS_TIMEOUT 5000  /* 5s */
#define DFU_PROCESS_WAIT_TIMEOUT   5000  /* 5s */
#define PAIR_FAIL_DISCONN_TIMEOUT  2000  /* 2s */

/*============================================================================*
 *                         Types
 *============================================================================*/
typedef void *TimerHandle_t;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern TimerHandle_t adv_timer;
#if FEATURE_SUPPORT_NO_ACTION_DISCONN
extern TimerHandle_t no_act_disconn_timer;
#endif
extern TimerHandle_t update_conn_params_timer;
extern TimerHandle_t next_state_time_out;
/*============================================================================*
 *                         Functions
 *============================================================================*/
void sw_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif /*_SWTIMER__*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/
