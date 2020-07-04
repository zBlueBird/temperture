/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     battery_driver.h
* @details
* @author   yuyin_zhang
* @date     2018-05-16
* @version  v1.0
*********************************************************************************************************
*/
#include "board.h"
#include "rtl876x_lpc.h"
#include "bee2_adc_lib.h"

/***********ADC config************/

#define ADC_CHANNEL_INDEX               1
#define ADC_SCHEDULE_INDEX              0

#define VBAT_DETECT_TIMEOUT             BAT_TIMER_TIMEOUT  /*time out for battery collection*/
#define VBAT_DETECT_KEY_CNT             10
/***********LPC config************/

#define  LPC_COMP_VALUE     LPC_2520_mV


typedef enum
{
    NORMAL_MODE = 0,
    LOW_POWER_MODE,
} BAT_MODE;

typedef struct
{
    bool       is_working;                 /* indicate if adc working*/
    uint8_t    vbat_mode;
    uint8_t    vbat_level;
    uint8_t    vbat_detect_by_cnt;         /* indicate the counts to trrigle bat detect*/
    uint16_t   vbat_value;
    uint16_t   vbat_low_power_threshold;
} vbat_stg;


/*============================================================================*
 *                       Interface Functions
 *============================================================================*/

void bat_module_init(void);
void battery_get_value(uint16_t *p_level, uint16_t *p_value);
bool is_vbat_working(void);
void battery_msg_handle(uint16_t msg_sub_type);
void bat_nvic_config(void);

#if BAT_EN
#define rcu_bat_init()                    bat_module_init()
#define rcu_get_bat_value(level, value)   battery_get_value(level,value)
#define bat_allow_enter_dlps()            is_vbat_working()
#define rcu_bat_handle(type)              battery_msg_handle(type)
#if BAT_LPC_EN
#define rcu_bat_nvic_config()             bat_nvic_config()
#else
#define rcu_bat_nvic_config()
#endif//#BAT_LPC_EN
#else
#define rcu_bat_init()
#define rcu_get_bat_value(level, value)
#define bat_allow_enter_dlps()
#define rcu_bat_handle(type)
#define rcu_bat_nvic_config()
#endif//#BAT_EN

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

