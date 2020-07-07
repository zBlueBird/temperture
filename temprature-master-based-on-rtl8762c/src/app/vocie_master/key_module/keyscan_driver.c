/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This is the entry of user code which the main function resides in.
* @details
* @author   ranhui
* @date     2015-03-29
* @version  v0.2
*********************************************************************************************************
*/

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <board.h>
#include <string.h>
#include <trace.h>
#include <os_msg.h>
#include <app_msg.h>
#include <os_timer.h>
#include <keyscan_driver.h>
#include <rtl876x_rcc.h>
#include <rtl876x_keyscan.h>
#include <app_section.h>
#include <rtl876x_pinmux.h>
#include <rtl876x_nvic.h>
#include <app_task.h>

#if KEYSCAN_EN
/*============================================================================*
 *                         Macros
 *============================================================================*/
#define keyscan_interrupt_handler Keyscan_Handler

/*============================================================================*
 *                              Local Variables
 *============================================================================*/

/*============================================================================*
*                              Global Variables
*============================================================================*/
T_KEYSCAN_GLOBAL_DATA keyscan_global_data;
TimerHandle_t keyscan_timer;

/*============================================================================*
*                              External Functions
*============================================================================*/

/*============================================================================*
*                              Local Functions
*============================================================================*/
/******************************************************************
* @brief    keyscan row pad config
*/
void keyscan_row_pad_config(PAD_Mode AON_PAD_Mode,
                            PAD_PWR_Mode AON_PAD_PwrOn,
                            PAD_Pull_Mode AON_PAD_Pull,
                            PAD_OUTPUT_ENABLE_Mode AON_PAD_E,
                            PAD_OUTPUT_VAL AON_PAD_O)
{
#ifdef ROW0
    Pad_Config(ROW0, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW1
    Pad_Config(ROW1, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW2
    Pad_Config(ROW2, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW3
    Pad_Config(ROW3, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW4
    Pad_Config(ROW4, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW5
    Pad_Config(ROW5, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW6
    Pad_Config(ROW6, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef ROW7
    Pad_Config(ROW7, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
}

/******************************************************************
 * @brief    keyscan column pad config
 */
void keyscan_column_pad_config(PAD_Mode AON_PAD_Mode,
                               PAD_PWR_Mode AON_PAD_PwrOn,
                               PAD_Pull_Mode AON_PAD_Pull,
                               PAD_OUTPUT_ENABLE_Mode AON_PAD_E,
                               PAD_OUTPUT_VAL AON_PAD_O)
{
#ifdef COLUMN0
    Pad_Config(COLUMN0, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN1
    Pad_Config(COLUMN1, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN2
    Pad_Config(COLUMN2, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN3
    Pad_Config(COLUMN3, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN4
    Pad_Config(COLUMN4, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN5
    Pad_Config(COLUMN5, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN6
    Pad_Config(COLUMN6, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
#ifdef COLUMN7
    Pad_Config(COLUMN7, AON_PAD_Mode, AON_PAD_PwrOn, AON_PAD_Pull, AON_PAD_E, AON_PAD_O);
#endif
}

/******************************************************************
 * @brief    keyscan enable wakeup config function
 */
void keyscan_enable_wakeup_config(void)
{
    /* @note: no key is pressed, use PAD wake up function with debounce,
    but pad debunce time should be smaller than ble connect interval */
    System_WakeUpDebounceTime(0x08);
#ifdef ROW0
    System_WakeUpPinEnable(ROW0, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW1
    System_WakeUpPinEnable(ROW1, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW2
    System_WakeUpPinEnable(ROW2, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW3
    System_WakeUpPinEnable(ROW3, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW4
    System_WakeUpPinEnable(ROW4, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW5
    System_WakeUpPinEnable(ROW5, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW6
    System_WakeUpPinEnable(ROW6, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
#ifdef ROW7
    System_WakeUpPinEnable(ROW7, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#endif
}

/******************************************************************
 * @brief    keyscan disable wakeup config function
 */
void keyscan_disable_wakeup_config(void)
{
#ifdef ROW0
    System_WakeUpPinDisable(ROW0);
#endif
#ifdef ROW1
    System_WakeUpPinDisable(ROW1);
#endif
#ifdef ROW2
    System_WakeUpPinDisable(ROW2);
#endif
#ifdef ROW3
    System_WakeUpPinDisable(ROW3);
#endif
#ifdef ROW4
    System_WakeUpPinDisable(ROW4);
#endif
#ifdef ROW5
    System_WakeUpPinDisable(ROW5);
#endif
#ifdef ROW6
    System_WakeUpPinDisable(ROW6);
#endif
#ifdef ROW7
    System_WakeUpPinDisable(ROW7);
#endif
}

/*============================================================================*
*                              Global Functions
*============================================================================*/
/**
* @fn  keyscan_init_data
* @brief  Initialize Keyscan driver data
*/
void keyscan_init_data(void)
{
    KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[keyscan_init_data] init data", 0);
    memset(&keyscan_global_data, 0, sizeof(keyscan_global_data));
    keyscan_global_data.is_allowed_to_enter_dlps = true;
    keyscan_global_data.is_all_key_released = true;
}

/******************************************************************
 * @fn       keyscan_init_driver
 * @brief    keyscan module initial
 *
 * @param    uint32_t is_debounce
 * @return   void
 * @note     when system in dlsp mode, keyscan debunce time should be smaller
 *           than wake up interval time (like ble interval), which can be modified
 *           through KeyScan_InitStruct.debouncecnt.
 */
DATA_RAM_FUNCTION
void keyscan_init_driver(uint32_t is_debounce)
{
    RCC_PeriphClockCmd(APBPeriph_KEYSCAN, APBPeriph_KEYSCAN_CLOCK, DISABLE);
    /* turn on keyscan clock */
    RCC_PeriphClockCmd(APBPeriph_KEYSCAN, APBPeriph_KEYSCAN_CLOCK, ENABLE);
    keyscan_global_data.is_allowed_to_repeat_report = false;
    KEYSCAN_InitTypeDef  KeyScan_InitStruct;
    KeyScan_StructInit(&KeyScan_InitStruct);
    KeyScan_InitStruct.colSize         = KEYPAD_COLUMN_SIZE;
    KeyScan_InitStruct.rowSize         = KEYPAD_ROW_SIZE;
    KeyScan_InitStruct.scanmode        = KeyScan_Manual_Scan_Mode;

    KeyScan_InitStruct.clockdiv         = 0x26;  /* 128kHz = 5MHz/(clockdiv+1) */
    KeyScan_InitStruct.delayclk         = 0x0f;  /* 8kHz = 5MHz/(clockdiv+1)/(delayclk+1) */

    KeyScan_InitStruct.debounceEn       = is_debounce;
    KeyScan_InitStruct.scantimerEn      = KeyScan_ScanInterval_Disable;//KeyScan_ScanInterval_Enable;
    KeyScan_InitStruct.detecttimerEn    =
        KeyScan_Release_Detect_Disable;

    KeyScan_InitStruct.scanInterval     = 0x190;  /* 50ms = scanInterval/8kHz */
    KeyScan_InitStruct.debouncecnt      = 0x40;   //8ms = debouncecnt/8kHz
    KeyScan_InitStruct.releasecnt       = 0x01;   //0.125ms = releasecnt/8kHz

    KeyScan_Init(KEYSCAN, &KeyScan_InitStruct);
    KeyScan_INTConfig(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);
    KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);
    KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, DISABLE);  /* Mask keyscan interrupt */
    KeyScan_Cmd(KEYSCAN, ENABLE);
}

/******************************************************************
 * @brief    keyscan nvic config
 */
void keyscan_nvic_config(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = KeyScan_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}

/******************************************************************
 * @brief    keyscan pinmux config
 */
void keyscan_pinmux_config(void)
{
#ifdef ROW0
    Pinmux_Config(ROW0, KEY_ROW_0);
#endif
#ifdef ROW1
    Pinmux_Config(ROW1, KEY_ROW_1);
#endif
#ifdef ROW2
    Pinmux_Config(ROW2, KEY_ROW_2);
#endif
#ifdef ROW3
    Pinmux_Config(ROW3, KEY_ROW_3);
#endif
#ifdef ROW4
    Pinmux_Config(ROW4, KEY_ROW_4);
#endif
#ifdef ROW5
    Pinmux_Config(ROW5, KEY_ROW_5);
#endif
#ifdef ROW6
    Pinmux_Config(ROW6, KEY_ROW_6);
#endif
#ifdef ROW7
    Pinmux_Config(ROW7, KEY_ROW_7);
#endif

#ifdef COLUMN0
    Pinmux_Config(COLUMN0, KEY_COL_0);
#endif
#ifdef COLUMN1
    Pinmux_Config(COLUMN1, KEY_COL_1);
#endif
#ifdef COLUMN2
    Pinmux_Config(COLUMN2, KEY_COL_2);
#endif
#ifdef COLUMN3
    Pinmux_Config(COLUMN3, KEY_COL_3);
#endif
#ifdef COLUMN4
    Pinmux_Config(COLUMN4, KEY_COL_4);
#endif
#ifdef COLUMN5
    Pinmux_Config(COLUMN5, KEY_COL_5);
#endif
#ifdef COLUMN6
    Pinmux_Config(COLUMN6, KEY_COL_6);
#endif
#ifdef COLUMN7
    Pinmux_Config(COLUMN7, KEY_COL_7);
#endif
}

/******************************************************************
 * @brief    keyscan init pad config
 */
void keyscan_init_pad_config(void)
{
    keyscan_row_pad_config(PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    keyscan_column_pad_config(PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                              PAD_OUT_LOW);
}

/******************************************************************
 * @brief    keyscan enter DLPS config
 */
void keyscan_enter_dlps_config(void)
{
    KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[keyscan_enter_dlps_config] enter DLPS pad config", 0);
    keyscan_column_pad_config(PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    if (keyscan_global_data.is_all_key_released == true)
    {
        keyscan_enable_wakeup_config();
        keyscan_row_pad_config(PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    }
    else
    {
        /* any key is pressed, disable key row pins, just wait keyscan sw timer to wake */
        keyscan_disable_wakeup_config();
        keyscan_row_pad_config(PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
    }
}

/******************************************************************
 * @brief    keyscan exit DLPS config
 */
void keyscan_exit_dlps_config(void)
{
    keyscan_row_pad_config(PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    keyscan_column_pad_config(PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                              PAD_OUT_LOW);

    if (true == keyscan_global_data.is_all_key_released)
    {
        keyscan_init_driver(KeyScan_Debounce_Enable);
    }
}

/******************************************************************
 * @brief    keyscan interrupt handler
 *
 * If use Manual Scan Mode, it shall clear KEYSCAN_INT_SCAN_END to start next keyscan,
 * otherwise, keyscan will not start.
 */
DATA_RAM_FUNCTION
void keyscan_interrupt_handler(void)
{
    KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[keyscan_interrupt_handler] interrupt handler", 0);

    T_IO_MSG bee_io_msg;

    if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_INT_FLAG_SCAN_END) == SET)
    {
        keyscan_global_data.is_allowed_to_enter_dlps = true;
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);  /* Mask keyscan interrupt */

        keyscan_global_data.cur_fifo_data.len = KeyScan_GetFifoDataNum(KEYSCAN);
        if (keyscan_global_data.cur_fifo_data.len != 0)
        {
            /* read keyscan fifo data */
            KeyScan_Read(KEYSCAN, (uint16_t *) & (keyscan_global_data.cur_fifo_data.key[0]),
                         keyscan_global_data.cur_fifo_data.len);
            keyscan_global_data.is_key_pressed = true;
            keyscan_global_data.is_all_key_released = false;

            /* start sw timer to check press status */
            if (!os_timer_restart(&keyscan_timer, KEYSCAN_SW_INTERVAL))
            {
                APP_PRINT_ERROR0("[keyscan_interrupt_handler] restart keyscan_timer failed!");
                /* set flag to default status and reinit keyscan module with debounce enabled */
                keyscan_init_data();
                keyscan_init_driver(KeyScan_Debounce_Enable);
                return;
            }

            if (false == keyscan_global_data.is_allowed_to_repeat_report)
            {
                if (!memcmp(&keyscan_global_data.cur_fifo_data, &keyscan_global_data.pre_fifo_data,
                            sizeof(T_KEYSCAN_FIFIO_DATA)))
                {
                    /* some keyscan FIFO data, just return */
                    return;
                }
                else
                {
                    /* updata previous keyscan FIFO data */
                    memcpy(&keyscan_global_data.pre_fifo_data, &keyscan_global_data.cur_fifo_data,
                           sizeof(T_KEYSCAN_FIFIO_DATA));
                }
            }

            bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
            bee_io_msg.subtype = IO_MSG_KEYSCAN_RX_PKT;
            bee_io_msg.u.buf   = (void *)(&keyscan_global_data.pre_fifo_data);
            if (false == app_send_msg_to_apptask(&bee_io_msg))
            {
                APP_PRINT_ERROR0("[keyscan_interrupt_handler] send IO_MSG_KEYSCAN_RX_PKT message failed!");
                /* set flag to default status and reinit keyscan module with debounce enabled */
                keyscan_init_data();
                os_timer_stop(&keyscan_timer);
                keyscan_init_driver(KeyScan_Debounce_Enable);
                return;
            }
        }
        else
        {
            if (false == keyscan_global_data.is_all_key_released)
            {
                /* keyscan release event detected */
                KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                                   "[keyscan_interrupt_handler] keyscan release event detected", 0);
                T_IO_MSG bee_io_msg;
                bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
                bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

                if (false == app_send_msg_to_apptask(&bee_io_msg))
                {
                    APP_PRINT_ERROR0("[keyscan_interrupt_handler] Send IO_MSG_TYPE_KEYSCAN message failed!");
                }

                keyscan_init_data();
                keyscan_init_driver(KeyScan_Debounce_Enable);
            }
            else
            {
                /*if system active, keyscan no debounce can arrive here*/
                KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                                   "[keyscan_interrupt_handler] if system active, keyscan no debounce can arrive here", 0);
                keyscan_init_data();
                keyscan_init_driver(KeyScan_Debounce_Enable);
                return;
            }
        }
    }
    else
    {
        /* if not KEYSCAN_INT_FLAG_SCAN_END interrupt */
        KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                           "[keyscan_interrupt_handler] not KEYSCAN_INT_FLAG_SCAN_END interrupt", 0);
        keyscan_init_data();
        keyscan_init_driver(KeyScan_Debounce_Enable);
        return;
    }
}

/******************************************************************
 * @brief    keyscan init timer
 */
void keyscan_init_timer(void)
{
    KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[keyscan_init_timer] init timer", 0);
    /*keyscan_timer is used for keyscan dlps*/
    if (false == os_timer_create(&keyscan_timer, "keyscan_timer",  1, \
                                 KEYSCAN_SW_INTERVAL, false, keyscan_timer_callback))
    {
        APP_PRINT_ERROR0("[keyscan_init_timer] timer creat failed!");
    }
}

/******************************************************************
 * @brief    keyscan timer callback
 */
DATA_RAM_FUNCTION
void keyscan_timer_callback(TimerHandle_t pxTimer)
{
    if (keyscan_global_data.is_key_pressed == true)
    {
        KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[keyscan_timer_callback] start release timer", 0);
        keyscan_global_data.is_key_pressed = false;
        keyscan_global_data.is_allowed_to_enter_dlps = false;
        keyscan_init_driver(KeyScan_Debounce_Disable);
        /* start timer to check key status */
        os_timer_restart(&pxTimer, KEYSCAN_SW_RELEASE_TIMEOUT);
    }
    else
    {
        /* keyscan release event detected */
        KEYSCAN_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                           "[keyscan_timer_callback] keyscan release event detected", 0);
        T_IO_MSG bee_io_msg;
        bee_io_msg.type = IO_MSG_TYPE_KEYSCAN;
        bee_io_msg.subtype = IO_MSG_KEYSCAN_ALLKEYRELEASE;

        if (false == app_send_msg_to_apptask(&bee_io_msg))
        {
            APP_PRINT_ERROR0("[keyscan_timer_callback] Send IO_MSG_TYPE_KEYSCAN message failed!");
        }

        keyscan_init_data();
        keyscan_init_driver(KeyScan_Debounce_Enable);
    }
}
#endif
