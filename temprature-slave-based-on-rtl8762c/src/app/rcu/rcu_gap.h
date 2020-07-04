/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      profile_init.h
* @brief
* @details
* @author    Chuanguo Xue
* @date      2015-3-27
* @version   v0.1
* *********************************************************************************************************
*/


#ifndef _RCU_GAP_H
#define _RCU_GAP_H

#include <stdint.h>
#include "rcu_application.h"

#ifdef __cplusplus
extern "C" {
#endif

void rcu_le_gap_init(void);
bool rcu_start_adv(T_ADV_TYPE adv_type);
bool rcu_stop_adv(T_STOP_ADV_REASON stop_adv_reason);
bool rcu_terminate_connection(T_DISCONN_REASON disconn_reason);
void rcu_update_conn_params(uint16_t interval, uint16_t latency, uint16_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* _PROFILE_INIT_H */
