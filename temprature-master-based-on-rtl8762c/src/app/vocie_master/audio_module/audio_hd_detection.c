/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      audio_hd_detection.c
* @brief    audio headphone detection module.
* @details
* @author  Chenejie_Jin
* @date     2018-10-28
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "audio_hd_detection.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "app_msg.h"
#include "app_task.h"
#include "os_timer.h"

#if AUDIO_SUPPORT_HEADPHONE_DETECT

#define AUDIO_HP_DETTACHED_LEVEL 0  /* low level */
#define AUDIO_HP_ATTACHED_LEVEL  1  /* high level */

#define AUDIO_HP_DETECT_TIMEOUT 100

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
AUDIO_HD_STATUS audio_hd_cur_state = AUDIO_HD_STATUS_DETACHED;
static void *audio_hd_debounce_timer = NULL;

/*============================================================================*
*                              Local Functions
*============================================================================*/
static void audio_hd_config_interrupt(GPIOIT_PolarityType polarity)
{
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);

    Gpio_Struct.GPIO_Pin = GPIO_GetPin(AUDIO_HD_DETECT_PIN);
    Gpio_Struct.GPIO_ITCmd = ENABLE;
    Gpio_Struct.GPIO_ITTrigger = GPIO_INT_Trigger_LEVEL;
    Gpio_Struct.GPIO_ITPolarity = polarity;
    GPIO_Init(&Gpio_Struct);

    GPIO_MaskINTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), ENABLE);
}

static void audio_hd_debounce_timeout_cb(void *p_timer)
{
    T_IO_MSG bee_io_msg;
    uint8_t input_data = GPIO_ReadInputDataBit(GPIO_GetPin(AUDIO_HD_DETECT_PIN));

    APP_PRINT_INFO1("[audio_hd_intr_handler] Input Data is %d", input_data);
    if (input_data == AUDIO_HP_DETTACHED_LEVEL)
    {
        GPIO->INTPOLARITY |= GPIO_GetPin(AUDIO_HD_DETECT_PIN);
        System_WakeUpPinEnable(AUDIO_HD_DETECT_PIN, PAD_WAKEUP_POL_HIGH, PAD_WK_DEBOUNCE_DISABLE);

        if (audio_hd_get_status() == AUDIO_HD_STATUS_ATTACHED)
        {
            bee_io_msg.type = IO_MSG_TYPE_AUDIO;
            bee_io_msg.subtype = IO_MSG_AUDIO_HD_DETACHED;
            if (false == app_send_msg_to_apptask(&bee_io_msg))
            {
                APP_PRINT_WARN0("[audio_hd_intr_handler] Send IO_MSG_AUDIO_HD_DETACHED failed!");
            }
            else
            {
                audio_hd_set_status_to_detached();
            }
        }
    }
    else
    {
        GPIO->INTPOLARITY &= ~GPIO_GetPin(AUDIO_HD_DETECT_PIN);
        System_WakeUpPinEnable(AUDIO_HD_DETECT_PIN, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_DISABLE);

        if (audio_hd_get_status() == AUDIO_HD_STATUS_DETACHED)
        {
            bee_io_msg.type = IO_MSG_TYPE_AUDIO;
            bee_io_msg.subtype = IO_MSG_AUDIO_HD_ATTACHED;

            if (false == app_send_msg_to_apptask(&bee_io_msg))
            {
                APP_PRINT_WARN0("[audio_hd_intr_handler] Send IO_MSG_AUDIO_HD_ATTACHED failed!");
            }
            else
            {
                audio_hd_set_status_to_attached();
            }
        }
    }

    GPIO_INTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), ENABLE);
    GPIO_MaskINTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), DISABLE);
}

/*============================================================================*
*                              Global Functions
*============================================================================*/
/******************************************************************
* @brief    audio headphone driver initial
*/
void audio_hd_driver_init(void)
{
    /* init pad and pinmux */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    Pinmux_Config(AUDIO_HD_DETECT_PIN, DWGPIO);
    Pad_Config(AUDIO_HD_DETECT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    /* check current status */
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);
    Gpio_Struct.GPIO_Pin = GPIO_GetPin(AUDIO_HD_DETECT_PIN);
    Gpio_Struct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(&Gpio_Struct);

    if (GPIO_ReadInputDataBit(GPIO_GetPin(AUDIO_HD_DETECT_PIN)) == AUDIO_HP_DETTACHED_LEVEL)
    {
        audio_hd_set_status_to_detached();
        audio_hd_config_interrupt(GPIO_INT_POLARITY_ACTIVE_HIGH);
    }
    else
    {
        audio_hd_set_status_to_attached();
        audio_hd_config_interrupt(GPIO_INT_POLARITY_ACTIVE_LOW);
    }

    /* init sw debounce timer */
    os_timer_create(&audio_hd_debounce_timer, "audio headphone debounce timer",
                    1, AUDIO_HP_DETECT_TIMEOUT, false, audio_hd_debounce_timeout_cb);
}

/******************************************************************
* @brief    audio headphone init nvic
*/
void audio_hd_nvic_config(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = AUDIO_HD_Int_Pin_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
* @brief GPIO interrupt handler function.
* @param   No parameter.
* @return  void
*/
void audio_hd_intr_handler(void)
{
    GPIO_INTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), DISABLE);
    GPIO_MaskINTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), ENABLE);
    GPIO_ClearINTPendingBit(GPIO_GetPin(AUDIO_HD_DETECT_PIN));

    System_WakeUpPinDisable(AUDIO_HD_DETECT_PIN);

    if (os_timer_start(&audio_hd_debounce_timer) == false)
    {
        GPIO_INTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), ENABLE);
        GPIO_MaskINTConfig(GPIO_GetPin(AUDIO_HD_DETECT_PIN), DISABLE);
    }
}

/******************************************************************
 * @brief    audio header enter DLPS config
 */
void audio_hd_enter_dlps_config(void)
{
    Pad_Config(AUDIO_HD_DETECT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
}

/******************************************************************
 * @brief    audio header exit DLPS config
 */
void audio_hd_exit_dlps_config(void)
{
    Pad_Config(AUDIO_HD_DETECT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
}

#endif

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

