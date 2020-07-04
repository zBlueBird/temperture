/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This is the entry of user code which the main function resides in.
* @details
* @author   chenjie_jin
* @date     2018-04-12
* @version  v0.3
*********************************************************************************************************
*/
#include <stdlib.h>
#include <board.h>
#include <os_sched.h>
#include <string.h>
#include <trace.h>
#include <gap.h>
#include <gap_bond_le.h>
#include <profile_server.h>
#include <rcu_application.h>
#include <dlps.h>
#include <rtl876x_io_dlps.h>
#include <app_task.h>
#include <swtimer.h>
#include <os_timer.h>
#include <bas.h>
#include <dis.h>
#include <hids_rmc.h>
#include <vendor_service.h>
#include <rcu_ota_service.h>
#include <rcu_gap.h>
#include <app_section.h>
#if SUPPORT_SILENT_OTA
#include <rcu_dfu_service.h>
#endif

#include <mem_config.h>

#include <rtl876x_pinmux.h>


#include "battery_driver.h"

#include "si7021_i2c_driver.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/
#define app_system_on_interrupt_handler System_Handler

/*============================================================================*
 *                              Global Variables
 *============================================================================*/

/*============================================================================*
*                              Global Functions
*============================================================================*/
/**
* @brief  global_data_init() contains the initialization of global data.
*/
void global_data_init(void)
{
    app_init_global_data();

}

/******************************************************************
 * @brief  app_pinmux_config() contains the initialization of app pinmux config.
 */
void app_pinmux_config(void)
{

}

/******************************************************************
 * @brief  app_pad_config() contains the initialization of app pad config.
 */
void app_pad_config(void)
{

}

/******************************************************************
 * @brief  app_nvic_config() contains the initialization of app NVIC config.
 */
void app_nvic_config(void)
{
    rcu_bat_nvic_config();
}

/**
* @brief    Board_Init() contains the initialization of pinmux settings and pad settings.
*
*               All the pinmux settings and pad settings shall be initiated in this function.
*               But if legacy driver is used, the initialization of pinmux setting and pad setting
*               should be peformed with the IO initializing.
*
* @return  void
*/
void board_init(void)
{
    app_pinmux_config();
    app_pad_config();
}

/**
* @brief    driver_init() contains the initialization of peripherals.
*
*               Both new architecture driver and legacy driver initialization method can be used.
*
* @return  void
*/
void driver_init(void)
{
    /*rcu battery module init*/
    rcu_bat_init();


    I2cMaster_Init();
}

#if DLPS_EN
/**
 * @brief this function will be called before enter DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
void app_enter_dlps_config(void) DATA_RAM_FUNCTION;
void app_enter_dlps_config(void)
{
    Pad_PullConfigValue(I2C0_SCL, PAD_WEAK_PULL);
    Pad_PullConfigValue(I2C0_SDA, PAD_WEAK_PULL);
    Pad_Config(I2C0_SCL, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
}

/**
 * @brief this function will be called after exit DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
void app_exit_dlps_config(void) DATA_RAM_FUNCTION;
void app_exit_dlps_config(void)
{
    Pad_PullConfigValue(I2C0_SCL, PAD_STRONG_PULL);
    Pad_PullConfigValue(I2C0_SDA, PAD_STRONG_PULL);
    Pad_Config(I2C0_SCL, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
}

/**
 * @brief app_dlps_check_cb() contains the setting about app dlps callback.
*/
bool app_dlps_check_cb(void) DATA_RAM_FUNCTION;
bool app_dlps_check_cb(void)
{
    return (

#if BAT_EN
               bat_allow_enter_dlps()
#endif
           );
}
#endif

/**
* @brief  pwr_mgr_init() contains the setting about power mode.
*/
void pwr_mgr_init(void)
{
#if DLPS_EN
    if (false == dlps_check_cb_reg(app_dlps_check_cb))
    {
        APP_PRINT_ERROR0("Error: dlps_check_cb_reg(app_dlps_check_cb) failed!");
    }
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
#else
    lps_mode_set(LPM_ACTIVE_MODE);
#endif
}

/******************************************************************
 * @fn          Initial profile
 * @brief      Add simple profile service and register callbacks
 *
 * @return     void
 */
void app_le_profile_init(void)
{
    server_init(5);
    app_global_data.bas_srv_id = bas_add_service((void *)app_profile_callback);
    app_global_data.dis_srv_id = dis_add_service((void *)app_profile_callback);
    app_global_data.vendor_srv_id = vendor_svc_add_service((void *)app_profile_callback);
    app_global_data.ota_srv_id = ota_add_service((void *)app_profile_callback);

#if SUPPORT_SILENT_OTA
    app_global_data.dfu_srv_id = dfu_add_service((void *)app_profile_callback);
#endif
    server_register_app_cb(app_profile_callback);
}

/**
* @brief  app_system_on_interrupt_handler() contains the handler for System_On interrupt.
*/
void app_system_on_interrupt_handler(void) DATA_RAM_FUNCTION;
void app_system_on_interrupt_handler(void)
{
    APP_PRINT_INFO0("[app_system_on_interrupt_handler] sytem on interrupt");

}

/**
* @brief  task_init() contains the initialization of all the tasks.
*
*           There are four tasks are initiated.
*           Lowerstack task and upperstack task are used by bluetooth stack.
*           Application task is task which user application code resides in.
*           Emergency task is reserved.
*
* @return  void
*/
void task_init(void)
{
    app_task_init();
}

/**
* @brief  app_normal_power_on_seq() contains the app normal power on sequence.
*/
void app_normal_power_on_seq(void)
{
    board_init();
    driver_init();
    le_gap_init(1);
    rcu_le_gap_init();
    app_le_profile_init();
    pwr_mgr_init();
    sw_timer_init();
    task_init();
}

int main(void)
{
    extern uint32_t random_seed_value;
    srand(random_seed_value);

    global_data_init();

    /* check test mode */
    app_global_data.test_mode = get_test_mode();
    reset_test_mode();

    APP_PRINT_INFO1("Test Mode is %d", app_global_data.test_mode);

    switch (app_global_data.test_mode)
    {
    case NOT_TEST_MODE:
        {
            app_normal_power_on_seq();
        }
        break;
    default:
        break;
    }
    os_sched_start();
    return 0;
}
