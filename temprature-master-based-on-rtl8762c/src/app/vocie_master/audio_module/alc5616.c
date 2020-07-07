/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     alc5616.c
* @brief    This file provides driver of ALC5616.
* @details
* @author  elliot chen
* @date     2018-04-13
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "alc5616.h"
#include "platform_utils.h"

#if (FEATURE_SUPPORT_AUDIO_DOWN_STREAMING && (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5616))
/* Internal defines ------------------------------------------------------------*/
static void ALC5616_Init_Seq(void);

/**
  * @brief  test fuction of the I2C simulation module.
  * @param  None.
  * @retval None
  */
void ALC5616_I2C_Pinmux_Pad_Config(void)
{
    /* PWM Pinmux and PAD config */
    Pad_Config(ALC5616_SCL_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(ALC5616_SDA_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(ALC5616_SCL_Pin, ALC_5616_I2C_CLK);
    Pinmux_Config(ALC5616_SDA_Pin, ALC_5616_I2C_SDA);
}

#if 0
/**
  * @brief  Internal function which set specific bits of register in ALC5628.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5616 register address.
  * @param  data: half word to be transmitted.
  * @retval None
  */
static void ALC5616_SetBits(I2C_TypeDef *I2Cx, uint8_t reg_address, uint16_t data)
{
    uint8_t reg_data[2] = {0, 0};

    /* Read value of target register */
    I2C_RepeatRead(I2Cx, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5616 */
    I2Cx->IC_DATA_CMD = reg_address;

    /* Send data */
    I2Cx->IC_DATA_CMD = (data >> 8) | reg_data[0];
    I2Cx->IC_DATA_CMD = (data & 0xff) | (1 << 9) | reg_data[1];
}

/**
  * @brief Internal function which reset specific bits of register in ALC5628.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5616 register address.
  * @param  data: half word to be transmitted.
  * @retval None
  */
static void ALC5616_ResetBits(I2C_TypeDef *I2Cx, uint8_t reg_address, uint16_t data)
{
    uint8_t reg_data[2] = {0, 0};

    /* Read value of target register */
    I2C_RepeatRead(I2Cx, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5616 */
    I2Cx->IC_DATA_CMD = reg_address;

    /* Send data */
    I2Cx->IC_DATA_CMD = (data >> 8) & reg_data[0];
    I2Cx->IC_DATA_CMD = (data & reg_data[1]) | (1 << 9);
}
#endif

/**
  * @brief  Internal function which Send data to ALC5616 directly.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5616 register address.
  * @param  data: half word to be transmitted.
  * @retval None
  */
static void ALC5616_Write(I2C_TypeDef *I2Cx, uint8_t reg_address, uint16_t data)
{
    /* Check I2C communication is busy or not */
    while (I2C_GetFlagState(ALC5616_I2C, I2C_FLAG_TFE) == RESET);
    while (I2C_GetFlagState(ALC5616_I2C, I2C_FLAG_ACTIVITY) == SET);

    /* Send register address of ALC5616 */
    I2Cx->IC_DATA_CMD = reg_address;

    /* Send data */
    I2Cx->IC_DATA_CMD = (data >> 8) & 0xff;
    I2Cx->IC_DATA_CMD = (data & 0xff) | (1 << 9);
}

/**
  * @brief  Read register of ALC5616 and print log.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5616 register address.
  * @retval value of register.
  */
uint16_t ALC5616_ReadRegister(I2C_TypeDef *I2Cx, uint8_t reg_address)
{
    uint8_t reg_data[2] = {0, 0};
    uint16_t reg_value = 0;

    I2C_RepeatRead(I2Cx, &reg_address, 1, &reg_data[0], 2);

    reg_value = (reg_data[0] << 8) | reg_data[1];
    return reg_value;
}

/**
  * @brief  Reset the ALC5616.
  * @param None.
  * @retval None
  */
uint8_t ALC5616_Reset(void)
{
    uint8_t reg_address = 0x00;
    uint16_t reg_data = 0x21;

    /* Reset ALC5616 */
    ALC5616_Write(ALC5616_I2C, reg_address, reg_data);

    return true;
}

/**
  * @brief  Check chid id of the ALC5616.
  * @param None.
  * @retval None
  */
uint8_t ALC5616_Check_Chip_Id(void)
{
    /* Check chid id of ALC5616 */
    uint8_t max_check_count = 5;
    uint8_t current_check_count = 0;
    uint16_t chipid_value = 0;//0x10EC

    for (current_check_count = 0; current_check_count < max_check_count; current_check_count++)
    {
        chipid_value = ALC5616_ReadRegister(ALC5616_I2C, 0xFE);
        if (chipid_value == 0x10EC)
        {
            APP_PRINT_INFO1("[Check Chip Id] success, read_value = %x", chipid_value);
            return true;
        }
        else
        {
            APP_PRINT_INFO1("[Check Chip Id] fail, read_value = %x", chipid_value);
        }
    }
    return false;
}

/**
  * @brief  Check init info of the ALC5616.
  * @param None.
  * @retval None
  */
uint8_t ALC5616_Check_Init_Info(void)
{
    /* Check init info of ALC5616 */
    uint8_t max_check_count = 5;
    uint8_t current_check_count = 0;
    uint16_t init_info_value[7] = {0};//0x10EC

    for (current_check_count = 0; current_check_count < max_check_count; current_check_count++)
    {
        //MX-61=9800    MX-62=0800      MX-63=E8FE      MX-64=0A00      MX-65=C000      MX-66=0C00      MX-02=0808
        init_info_value[0] = ALC5616_ReadRegister(ALC5616_I2C, 0x61);
        init_info_value[1] = ALC5616_ReadRegister(ALC5616_I2C, 0x62);
        init_info_value[2] = ALC5616_ReadRegister(ALC5616_I2C, 0x63);
        init_info_value[3] = ALC5616_ReadRegister(ALC5616_I2C, 0x64);
        init_info_value[4] = ALC5616_ReadRegister(ALC5616_I2C, 0x65);
        init_info_value[5] = ALC5616_ReadRegister(ALC5616_I2C, 0x66);
        init_info_value[6] = ALC5616_ReadRegister(ALC5616_I2C, 0x02);
        if (
            (init_info_value[0] == 0x9800) &&
            (init_info_value[1] == 0x0800) &&
            (init_info_value[2] == 0xE8FE) &&
            (init_info_value[3] == 0x0A00) &&
            (init_info_value[4] == 0xC000) &&
            (init_info_value[5] == 0x0C00) &&
            (init_info_value[6] == 0x0808)
        )
        {
            APP_PRINT_INFO0("[ALC5616_Check_Init_Info] success,");
            return true;
        }
        else
        {
            APP_PRINT_INFO7("[ALC5616_Check_Init_Info] fail, read_value = %x,%x,%x,%x,%x,%x,%x,%x ",
                            init_info_value[0], init_info_value[1], init_info_value[3], init_info_value[3], init_info_value[4],
                            init_info_value[5], init_info_value[6]);
            platform_delay_ms(10);
            ALC5616_Init_Seq();
        }
    }
    return false;
}

/**
  * @brief  init seq of the ALC5616.
  * @param None.
  * @retval None
  */
void ALC5616_Init_Seq(void)
{
    uint16_t default_vol = ((uint16_t)ALC5616_DAC_VOL_DEFAULT_VALUE << 8) +
                           ALC5616_DAC_VOL_DEFAULT_VALUE;

    ALC5616_Write(ALC5616_I2C, 0x00, 0x0000);
    ALC5616_Write(ALC5616_I2C, 0xFA, 0x0091);
    ALC5616_Write(ALC5616_I2C, 0x61, 0x9800);
    ALC5616_Write(ALC5616_I2C, 0x62, 0x0800);
    ALC5616_Write(ALC5616_I2C, 0x63, 0xA8F6);
    ALC5616_Write(ALC5616_I2C, 0x63, 0xE8FE);
    ALC5616_Write(ALC5616_I2C, 0x65, 0xC000);
    ALC5616_Write(ALC5616_I2C, 0x66, 0x0C00);
    ALC5616_Write(ALC5616_I2C, 0x80, 0x5000);
    ALC5616_Write(ALC5616_I2C, 0x81, 0x0F06);
    ALC5616_Write(ALC5616_I2C, 0x82, 0x0800);
    ALC5616_Write(ALC5616_I2C, 0x8F, 0x3100);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x0009);
    ALC5616_Write(ALC5616_I2C, 0x6A, 0x0077);
    ALC5616_Write(ALC5616_I2C, 0x6C, 0x9F00);
    ALC5616_Write(ALC5616_I2C, 0x6A, 0x003D);
    ALC5616_Write(ALC5616_I2C, 0x6C, 0x3E00);
    ALC5616_Write(ALC5616_I2C, 0x2A, 0x1212);
    ALC5616_Write(ALC5616_I2C, 0x45, 0x4000);
    ALC5616_Write(ALC5616_I2C, 0x73, 0x0000);
    ALC5616_Write(ALC5616_I2C, 0x64, 0x0A00);
    ALC5616_Write(ALC5616_I2C, 0x8F, 0x3100);
    ALC5616_Write(ALC5616_I2C, 0x4F, 0x0278);
    ALC5616_Write(ALC5616_I2C, 0x52, 0x0278);
    ALC5616_Write(ALC5616_I2C, 0x91, 0x0F00);
    ALC5616_Write(ALC5616_I2C, 0x90, 0x0737);
    ALC5616_Write(ALC5616_I2C, 0x19, default_vol);
    ALC5616_Write(ALC5616_I2C, 0x27, 0x3802);
    ALC5616_Write(ALC5616_I2C, 0x3C, 0x006B);
    ALC5616_Write(ALC5616_I2C, 0x3E, 0x006B);
    ALC5616_Write(ALC5616_I2C, 0xD9, 0xBCF5);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x0009);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x001D);
    ALC5616_Write(ALC5616_I2C, 0x6A, 0x0037);
    ALC5616_Write(ALC5616_I2C, 0x6C, 0xFC00);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x801D);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x805D);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x801D);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x831D);
    ALC5616_Write(ALC5616_I2C, 0x02, 0x0808);
    //delay 100);
    platform_delay_ms(100);
    ALC5616_Write(ALC5616_I2C, 0x8E, 0x8019);
}

/**
  * @brief  deinit seq of the ALC5616.
  * @param None.
  * @retval None
  */
void ALC5616_Deinit_Seq(void)
{
    ALC5616_Write(ALC5616_I2C, 0x24, 0x0120);
    ALC5616_Write(ALC5616_I2C, 0xD6, 0x0400);
    ALC5616_Write(ALC5616_I2C, 0x90, 0x0636);
    ALC5616_Write(ALC5616_I2C, 0x37, 0xFC00);
    ALC5616_Write(ALC5616_I2C, 0X8E, 0x801D);
    ALC5616_Write(ALC5616_I2C, 0X8E, 0x803D);
    ALC5616_Write(ALC5616_I2C, 0X8E, 0x801D);
    ALC5616_Write(ALC5616_I2C, 0x02, 0x8888);
    //DELAY 100
    platform_delay_ms(100);
    ALC5616_Write(ALC5616_I2C, 0X8E, 0x8019);
    ALC5616_Write(ALC5616_I2C, 0x02, 0xC8C8);
    ALC5616_Write(ALC5616_I2C, 0x00, 0x0000);
}

/**
  * @brief  Initializes the ALC5616.
  * @param None.
  * @retval None
  */
void ALC5616_Init(void)
{
    I2C_InitTypeDef  I2C_InitStructure;
    /* Enable Bee I2C clock */
    if (ALC5616_I2C == I2C0)
    {
        RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);
    }
    else
    {
        if (ALC5616_I2C == I2C1)
        {
            RCC_PeriphClockCmd(APBPeriph_I2C1, APBPeriph_I2C1_CLOCK, ENABLE);
        }
    }

    /* Initialize I2C */
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed = ALC5616_I2C_Speed;
    I2C_InitStructure.I2C_DeviveMode = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = ALC5616_Address;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_Init(ALC5616_I2C, &I2C_InitStructure);
    I2C_Cmd(ALC5616_I2C, ENABLE);

    if (ALC5616_Check_Chip_Id() == true)
    {
        ALC5616_Init_Seq();

        if (ALC5616_Check_Init_Info() == true)
        {
            APP_PRINT_INFO0("[ALC5616_Init] , Success !");
        }
    }
    else
    {
        APP_PRINT_INFO0("[ALC5616_Init] , Fail !");
    }
}

/**
  * @brief  DeInitializes the ALC5616.
  * @param None.
  * @retval None
  */
void ALC5616_DeInit(void)
{
    /* Disable Bee I2C clock */
    ALC5616_Deinit_Seq();

}

/**
  * @brief  Configure head phone volume .
  * @param channel: select head phone channel, which can be Left_Headphone or Right_Headphone.
  * @param data: volume which can be 0(-65.625 dB) to 0xAF(0 dB).
  * @retval function state.
  */
void ALC5616_DACVolumeConfig(uint16_t channel, uint16_t data)
{
    uint8_t reg_address = 0x19;
    uint8_t reg_data[2] = {0, 0};

    /* Check parameter */
    assert_param(IS_HEADPHONE_CHANNEL(channel));

    if (data > ALC5616_DAC_VOL_MAX_VALUE)
    {
        return;
    }

    /* Read value of target register */
    I2C_RepeatRead(ALC5616_I2C, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    ALC5616_I2C->IC_DATA_CMD = reg_address;

    if (channel == Left_Headphone)
    {
        /* Send data */
        ALC5616_I2C->IC_DATA_CMD = data | (reg_data[0] & (~(0xff)));
        ALC5616_I2C->IC_DATA_CMD = (reg_data[1]) | (1 << 9);
    }
    else
    {
        if (channel == Right_Headphone)
        {
            /* Send data */
            ALC5616_I2C->IC_DATA_CMD = reg_data[0];
            ALC5616_I2C->IC_DATA_CMD = data | (reg_data[1] & (~(0xff))) | (1 << 9);
        }
    }
}

#endif

/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/

