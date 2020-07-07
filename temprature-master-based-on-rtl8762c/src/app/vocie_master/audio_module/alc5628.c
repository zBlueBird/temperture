/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     alc5628.c
* @brief    This file provides driver of ALC5628.
* @details
* @author  elliot chen
* @date     2015-10-12
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "alc5628.h"

#if (FEATURE_SUPPORT_AUDIO_DOWN_STREAMING && (AUDIO_CODEC_CHIPSET_SEL == AUDIO_CODEC_CHIPSET_ALC5628))
/* Internal defines ------------------------------------------------------------*/

/* ALC5628 register defines */
#define RT5628_RESET                        0X00            //RESET CODEC TO DEFAULT
#define RT5628_SPK_OUT_VOL                  0X02            //SPEAKER OUT VOLUME
#define RT5628_HP_OUT_VOL                   0X04            //HEADPHONE OUTPUT VOLUME
#define RT5628_AUXIN_VOL                    0X08            //AUXIN VOLUME
#define RT5628_LINE_IN_VOL                  0X0A            //LINE IN VOLUME
#define RT5628_STEREO_DAC_VOL               0X0C            //STEREO DAC VOLUME
#define RT5628_SOFT_VOL_CTRL_TIME           0X16            //SOFT DELAY VOLUME CONTROL TIME
#define RT5628_OUTPUT_MIXER_CTRL            0X1C            //OUTPUT MIXER CONTROL
#define RT5628_AUDIO_DATA_CTRL              0X34            //STEREO AUDIO DATA CONTROL
#define RT5628_DAC_CLK_CTRL                 0X38            //STEREO DAC CLOCK CONTROL
#define RT5628_PWR_MANAG_ADD1               0X3A            //POWER MANAGMENT ADDITION 1
#define RT5628_PWR_MANAG_ADD2               0X3C            //POWER MANAGMENT ADDITION 2
#define RT5628_PWR_MANAG_ADD3               0X3E            //POWER MANAGMENT ADDITION 3
#define RT5628_GEN_CTRL                     0X40            //GENERAL PURPOSE CONTROL
#define RT5628_GLOBAL_CLK_CTRL              0X42            //GLOBAL CLOCK CONTROL
#define RT5628_PLL_CTRL                     0X44            //PLL CONTROL
#define RT5628_GPIO_PIN_CONFIG              0X48            //GPIO PIN CONFIG   
#define RT5628_GPIO_OUTPUT_PIN_CTRL         0X4A            //GPIO CONTROL
#define RT5628_JACK_DET_CTRL                0X5A            //JACK DETECT CONTROL 
#define RT5628_MISC1_CTRL                   0X5C            //MISC1 CONTROL
#define RT5628_MISC2_CTRL                   0X5E            //MISC2 CONTROL
#define RT5628_AVC_CTRL                     0X68            //AVC CONTROL
#define RT5628_HID_CTRL_INDEX               0X6A            //PRIVATE REGISTER ADDRESS
#define RT5628_HID_CTRL_DATA                0X6C            //PRIVATE REGISTER DATA
#define RT5628_VENDOR_ID1                   0x7C            //VENDOR ID1
#define RT5628_VENDOR_ID2                   0x7E            //VENDOR ID2

//global definition
#define RT_L_MUTE                           (0x1<<15)       //MUTE LEFT CONTROL BIT
#define RT_R_MUTE                           (0x1<<7)        //MUTE RIGHT CONTROL BIT
#define RT_M_HP_MIXER                       (0x1<<15)       //Mute source to HP Mixer
#define RT_M_SPK_MIXER                      (0x1<<14)       //Mute source to Speaker Mixer
#define RT_M_MONO_MIXER                     (0x1<<13)       //Mute source to Mono Mixer

#define ALL_FIELD                           0xffff

//Headphone Output Volume
#define HP_VOLUME_MASK                      (0x1f)

//Stereo DAC Digital Volume(0x0C)
#define DAC_VOLUME_MASK                     (0x3f)

//Output Mixer Control(0x1C)
#define SPKOUT_N_SOUR_MASK                  (0x3<<14)
#define SPKOUT_N_SOUR_MUTE                  (0x3<<14)
#define SPKOUT_N_SOUR_LN                    (0x2<<14)
#define SPKOUT_N_SOUR_RP                    (0x1<<14)
#define SPKOUT_N_SOUR_RN                    (0x0<<14)
#define SPKOUT_INPUT_SEL_MASK               (0x3<<10)
#define SPKOUT_INPUT_SEL_SPKMIXER           (0x2<<10)
#define SPKOUT_INPUT_SEL_HPMIXER            (0x1<<10)
#define SPKOUT_INPUT_SEL_VMID               (0x0<<10)
#define HPL_INPUT_SEL_HPLMIXER              (0x1<<9)
#define HPR_INPUT_SEL_HPRMIXER              (0x1<<8)
#define AUXOUT_L_INPUT_SEL_MASK             (0x1<<7)
#define AUXOUT_L_INPUT_SEL_HPLMIXER         (0x1<<7)
#define AUXOUT_L_INPUT_SEL_SPKMIXER         (0x0<<7)
#define AUXOUT_R_INPUT_SEL_MASK             (0x1<<6)
#define AUXOUT_R_INPUT_SEL_HPLMIXER         (0x1<<6)
#define AUXOUT_R_INPUT_SEL_SPKMIXER         (0x0<<6)
#define SPK_VOL_DIFF_NEG_SIG_ENA            (0x2<<2)
#define DAC_DIRECT_TO_HP                    (0x1<<1)
#define DAC_DIRECT_TO_AUXOUT                (0x1)


//Audio Interface(0x34)
#define SDP_MASTER_MODE                     (0x0<<15)
#define SDP_SLAVE_MODE                      (0x1<<15)
#define MAIN_I2S_BCLK_POL_CTRL              (0x1<<7)        //0:Normal 1:Invert
#define DAC_DATA_L_R_SWAP                   (0x1<<4)        //0:DAC data appear at left phase of LRCK
//1:DAC data appear at right phase of LRCK
//Data Length Slection
#define I2S_DL_MASK                         (0x3<<2)        //main i2s Data Length mask 
#define I2S_DL_16                           (0x0<<2)        //16 bits
#define I2S_DL_20                           (0x1<<2)        //20 bits
#define I2S_DL_24                           (0x2<<2)        //24 bits


//PCM Data Format Selection
#define I2S_DF_MASK                         (0x3)           //IIS Data Format Mask
#if 0
#define I2S_DF_I2S                          (0x0)           //I2S FORMAT 
#define I2S_DF_LEFT                         (0x1)           //LEFT JUSTIFIED FORMAT
#define I2S_DF_PCM_A                        (0x2)           //PCM MODE A
#define I2S_DF_PCM_B                        (0x3)           //PCM MODE B
#endif

//Stereo AD/DA Clock Control(0x38h)
#define I2S_PRE_DIV_MASK                    (uint16_t)(~(0x7<<13))
#define I2S_PRE_DIV_1                       (0x0<<13)           //DIV 1
#define I2S_PRE_DIV_2                       (0x1<<13)           //DIV 2
#define I2S_PRE_DIV_4                       (0x2<<13)           //DIV 4
#define I2S_PRE_DIV_8                       (0x3<<13)           //DIV 8
#define I2S_PRE_DIV_16                      (0x4<<13)           //DIV 16
#define I2S_PRE_DIV_32                      (0x5<<13)           //DIV 32

#define I2S_BCLK_SEL_64FS                   (0x0<<12)           //32 BITS(64FS)
#define I2S_BCLK_SEL_32FS                   (0x1<<12)           //16 BITS(32FS)

#define DAC_FILTER_CLK_SEL_256FS            (0<<2)          //256FS
#define DAC_FILTER_CLK_SEL_384FS            (1<<2)          //384FS

//Power managment addition 1 (0x3A),0:Disable,1:Enable
#define PWR_MAIN_I2S_EN                     (0x1<<15)
#define PWR_ZC_DET_PD_EN                    (0x1<<14)
#define PWR_SOFTGEN_EN                      (0x1<<8)
#define PWR_HP_AMP                          (0x1<<5)
#define PWR_HP_ENH_AMP                      (0x1<<4)

//Power managment addition 2(0x3C),0:Disable,1:Enable
#define PWR_CLASS_D                         (0x1<<14)
#define PWR_VREF                            (0x1<<13)
#define PWR_PLL                             (0x1<<12)
#define PWR_THERMAL_SD                      (0x1<<11)
#define PWR_DAC_REF_CIR                     (0x1<<10)
#define PWR_L_DAC_CLK                       (0x1<<9)
#define PWR_R_DAC_CLK                       (0x1<<8)
#define PWR_L_DAC_L_D_S                     (0x1<<7)
#define PWR_R_DAC_R_D_S                     (0x1<<6)
#define PWR_L_HP_MIXER                      (0x1<<5)
#define PWR_R_HP_MIXER                      (0x1<<4)
#define PWR_SPK_MIXER                       (0x1<<3)


//Power managment addition 3(0x3E),0:Disable,1:Enable
#define PWR_MAIN_BIAS                       (0x1<<15)
#define PWR_SPK_OUT                         (0x1<<12)
#define PWR_HP_L_OUT_VOL_AMP                (0x1<<10)
#define PWR_HP_R_OUT_VOL_AMP                (0x1<<9)
#define PWR_LINEIN_L_VOL                    (0x1<<7)
#define PWR_LINEIN_R_VOL                    (0x1<<6)
#define PWR_AUXIN_L_VOL                     (0x1<<5)
#define PWR_AUXIN_R_VOL                     (0x1<<4)

//Additional Control Register(0x40)
#define SPK_D_AMP_CTRL_MASK                 (0x7<<9)
#define SPK_D_AMP_CTRL_RATIO_225            (0x0<<9)        //2.25 Vdd
#define SPK_D_AMP_CTRL_RATIO_200            (0x1<<9)        //2.00 Vdd
#define SPK_D_AMP_CTRL_RATIO_175            (0x2<<9)        //1.75 Vdd
#define SPK_D_AMP_CTRL_RATIO_150            (0x3<<9)        //1.50 Vdd
#define SPK_D_AMP_CTRL_RATIO_125            (0x4<<9)        //1.25 Vdd  
#define SPK_D_AMP_CTRL_RATIO_100            (0x5<<9)        //1.00 Vdd

#define STEREO_DAC_H_PASS_EN                (0x1<<8)        //enable HIGH PASS FILTER FOR DAC

//Global Clock Control Register(0x42)
#define SYSCLK_SOUR_SEL_MASK                (0x1<<15)
#define SYSCLK_SOUR_SEL_MCLK                (~(0x1<<15))    //system Clock source from MCLK
#define SYSCLK_SOUR_SEL_PLL                 (0x1<<15)       //system Clock source from PLL
#define PLLCLK_SOUR_SEL_MCLK                (~(0x1<<14))    //PLL clock source from MCLK
#define PLLCLK_SOUR_SEL_BITCLK              (0x1<<14)       //PLL clock source from BITCLK

#define PLLCLK_DIV_RATIO_MASK               (0x3<<1)
#define PLLCLK_DIV_RATIO_DIV1               (0x0<<1)        //DIV 1
#define PLLCLK_DIV_RATIO_DIV2               (0x1<<1)        //DIV 2
#define PLLCLK_DIV_RATIO_DIV4               (0x2<<1)        //DIV 4
#define PLLCLK_DIV_RATIO_DIV8               (0x3<<1)        //DIV 8

#define PLLCLK_PRE_DIV1                     (0x0)           //DIV 1
#define PLLCLK_PRE_DIV2                     (0x1)           //DIV 2

//PLL Control(0x44)

#define PLL_CTRL_M_VAL(m)                   ((m)&0xf)
#define PLL_CTRL_K_VAL(k)                   (((k)&0x7)<<4)
#define PLL_CTRL_N_VAL(n)                   (((n)&0xff)<<8)


//GPIO CONTROL(0x4A)
#define GPIO_PIN_SEL_MASK                   (0x3<<14)
#define GPIO_PIN_SEL_LOG_OUT                (0x0<<14)
#define GPIO_PIN_SEL_IRQ                    (0x1<<14)
#define GPIO_PIN_SEL_PLLOUT                 (0x3<<14)

#define GPIO_PIN_CON_MASK                   (0x1<<3)
#define GPIO_PIN_CON_OUTPUT                 (0x0<<3)
#define GPIO_PIN_CON_INPUT                  (0x1<<3)

#define GPIO_PIN_OUTPUT_SET_MASK            (0x1<<2)
#define GPIO_PIN_OUTPUT_SET_LOW             (0x0<<2)
#define GPIO_PIN_OUTPUT_SET_HIGH            (0x1<<2)

#define GPIO_PIN_POLARITY_INV               (0x1<<1)

//JACK DETECT CONTROL(0x5A)
#define JACK_DET_SEL_MASK                   (0x3<<14)
#define JACK_DET_SEL_OFF                    (0x0<<14)       //Jack Detect Select none
#define JACK_DET_SEL_GPIO                   (0x1<<14)       //Jack Detect Select GPIO
#define JACK_DET_SEL_JD1                    (0x2<<14)       //Jack Detect Select JD1,LineIn Left disable
#define JACK_DET_SEL_JD2                    (0x3<<14)       //Jack Detect Select JD2,LineIn Right Disable

#define JACK_DET_TRI_VREF                   (0x1<<13)
#define JACK_DET_POL_TRI_VREF               (0x1<<12)
#define JACK_DET_TRI_HP                     (0x1<<11)
#define JACK_DET_POL_TRI_HP                 (0x1<<10)
#define JACK_DET_TRI_SPK                    (0x1<<9)
#define JACK_DET_POL_TRI_SPK                (0x1<<8)
#define JACK_DET_POL                        (0x1<<3)


//MISC1 CONTROL(0x5C)
#define SPK_L_ZC_CTRL_EN                    (0x1<<15)
#define SPK_L_SV_CTRL_EN                    (0x1<<14)
#define SPK_R_ZC_CTRL_EN                    (0x1<<13)
#define SPK_R_SV_CTRL_EN                    (0x1<<12)
#define HP_L_ZC_CTRL_EN                     (0x1<<11)
#define HP_L_SV_CTRL_EN                     (0x1<<10)
#define HP_R_ZC_CTRL_EN                     (0x1<<9)
#define HP_R_SV_CTRL_EN                     (0x1<<8)
#define AUXOUT_L_ZC_CTRL_EN                 (0x1<<7)
#define AUXOUT_L_SV_CTRL_EN                 (0x1<<6)
#define AUXOUT_R_ZC_CTRL_EN                 (0x1<<5)
#define AUXOUT_R_SV_CTRL_EN                 (0x1<<4)
#define DAC_ZC_CTRL_EN                      (0x1<<3)
#define DAC_SV_CTRL_EN                      (0x1<<2)


////MISC2 CONTROL(0x5E)
#define FAST_VREG_EN                        (uint16_t)(~(0x1<<15))
#define THERMAL_SHUTDOWN_EN                 (0x1<<14)
#define HP_DEPOP_MODE2_EN                   (0x1<<9)
#define HP_DEPOP_MODE1_EN                   (0x1<<8)
#define HP_L_M_UM_DEPOP_EN                  (0x1<<7)
#define HP_R_M_UM_DEPOP_EN                  (0x1<<6)
#define M_UM_DEPOP_EN                       (0x1<<5)
#define M_DAC_L_INPUT                       (0x1<<3)
#define M_DAC_R_INPUT                       (0x1<<2)

//AVC Control(0x68)
#define AVC_ENABLE                          (0x1<<15)
#define AVC_TARTGET_SEL_MASK                (0x1<<14)
#define AVC_TARTGET_SEL_R                   (0x1<<14)
#define AVC_TARTGET_SEL_L                   (0x0<<14)


//WaveOut channel for realtek codec
enum
{
    RT_WAVOUT_SPK               = (0x1 << 0),
    RT_WAVOUT_SPK_R             = (0x1 << 1),
    RT_WAVOUT_SPK_L             = (0x1 << 2),
    RT_WAVOUT_HP                = (0x1 << 3),
    RT_WAVOUT_HP_R              = (0x1 << 4),
    RT_WAVOUT_HP_L              = (0x1 << 5),
    RT_WAVOUT_MONO              = (0x1 << 6),
    RT_WAVOUT_AUXOUT            = (0x1 << 7),
    RT_WAVOUT_AUXOUT_R          = (0x1 << 8),
    RT_WAVOUT_AUXOUT_L          = (0x1 << 9),
    RT_WAVOUT_LINEOUT           = (0x1 << 10),
    RT_WAVOUT_LINEOUT_R         = (0x1 << 11),
    RT_WAVOUT_LINEOUT_L         = (0x1 << 12),
    RT_WAVOUT_DAC               = (0x1 << 13),
    RT_WAVOUT_ALL_ON            = (0x1 << 14),
};

//WaveIn channel for realtek codec
enum
{
    RT_WAVIN_R_MONO_MIXER       = (0x1 << 0),
    RT_WAVIN_R_SPK_MIXER        = (0x1 << 1),
    RT_WAVIN_R_HP_MIXER         = (0x1 << 2),
    RT_WAVIN_R_PHONE            = (0x1 << 3),
    RT_WAVIN_R_AUXIN            = (0x1 << 3),
    RT_WAVIN_R_LINE_IN          = (0x1 << 4),
    RT_WAVIN_R_MIC2             = (0x1 << 5),
    RT_WAVIN_R_MIC1             = (0x1 << 6),

    RT_WAVIN_L_MONO_MIXER       = (0x1 << 8),
    RT_WAVIN_L_SPK_MIXER        = (0x1 << 9),
    RT_WAVIN_L_HP_MIXER         = (0x1 << 10),
    RT_WAVIN_L_PHONE            = (0x1 << 11),
    RT_WAVIN_L_AUXIN            = (0x1 << 11),
    RT_WAVIN_L_LINE_IN          = (0x1 << 12),
    RT_WAVIN_L_MIC2             = (0x1 << 13),
    RT_WAVIN_L_MIC1             = (0x1 << 14),
};

enum
{
    POWER_STATE_D0 = 0,
    POWER_STATE_D1,
    POWER_STATE_D1_PLAYBACK,
    POWER_STATE_D1_RECORD,
    POWER_STATE_D2,
    POWER_STATE_D2_PLAYBACK,
    POWER_STATE_D2_RECORD,
    POWER_STATE_D3,
    POWER_STATE_D4

};

/**
  * @brief  test fuction of the I2C simulation module.
  * @param  None.
  * @retval None
  */
void ALC5628_I2C_Pinmux_Pad_Config(void)
{
    /* PWM Pinmux and PAD config */
    Pad_Config(ALC5628_SCL_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(ALC5628_SDA_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(ALC5628_SCL_Pin, ALC5628_SCL_PINMUX);
    Pinmux_Config(ALC5628_SDA_Pin, ALC5628_SDA_PINMUX);
}

/**
  * @brief  Internal function which set specific bits of register in ALC5628.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5628 register address.
  * @param  data: half word to be transmitted.
  * @retval None
  */
static void ALC5628_SetBits(I2C_TypeDef *I2Cx, uint8_t reg_address, uint16_t data)
{
    uint8_t reg_data[2] = {0, 0};

    /* Read value of target register */
    while (I2C_GetFlagState(I2Cx, I2C_FLAG_TFE) == RESET);
    I2C_RepeatRead(I2Cx, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    I2Cx->IC_DATA_CMD = reg_address;

    /* Send data */
    I2Cx->IC_DATA_CMD = (data >> 8) | reg_data[0];
    I2Cx->IC_DATA_CMD = (data & 0xff) | (1 << 9) | reg_data[1];
}

/**
  * @brief Internal function which reset specific bits of register in ALC5628.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5628 register address.
  * @param  data: half word to be transmitted.
  * @retval None
  */
static void ALC5628_ResetBits(I2C_TypeDef *I2Cx, uint8_t reg_address, uint16_t data)
{
    uint8_t reg_data[2] = {0, 0};

    /* Read value of target register */
    while (I2C_GetFlagState(I2Cx, I2C_FLAG_TFE) == RESET);
    I2C_RepeatRead(I2Cx, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    I2Cx->IC_DATA_CMD = reg_address;

    /* Send data */
    I2Cx->IC_DATA_CMD = (data >> 8) & reg_data[0];
    I2Cx->IC_DATA_CMD = (data & reg_data[1]) | (1 << 9);
}

/**
  * @brief  Internal function which Send data to ALC5628 directly.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5628 register address.
  * @param  data: half word to be transmitted.
  * @retval None
  */
static void ALC5628_Write(I2C_TypeDef *I2Cx, uint8_t reg_address, uint16_t data)
{
    /* Send register address of ALC5628 */
    while (I2C_GetFlagState(I2Cx, I2C_FLAG_TFE) == RESET);
    I2Cx->IC_DATA_CMD = reg_address;

    /* Send data */
    I2Cx->IC_DATA_CMD = (data >> 8) & 0xff;
    I2Cx->IC_DATA_CMD = (data & 0xff) | (1 << 9);
}

/**
  * @brief  Read register of ALC5628 and print log.
  * @param  I2Cx: where x can be 0 or 1 to select the I2C peripheral.
  * @param  reg_address: ALC5628 register address.
  * @retval value of register.
  */
uint16_t ALC5628_ReadRegister(I2C_TypeDef *I2Cx, uint8_t reg_address)
{
    uint8_t reg_data[2] = {0, 0};
    uint16_t reg_value = 0;

    while (I2C_GetFlagState(I2Cx, I2C_FLAG_TFE) == RESET);
    I2C_RepeatRead(I2Cx, &reg_address, 1, &reg_data[0], 2);

    reg_value = reg_data[0] << 8 | reg_data[1];
    //DBG_DIRECT("Register:0x%x Value = 0x%x!", reg_address, reg_value);

    return reg_value;
}

/**
  * @brief  Reset the ALC5628.
  * @param None.
  * @retval None
  */
uint8_t ALC5628_Reset()
{
    uint8_t reg_address = RT5628_RESET;
    uint8_t reg_data[2] = {0, 0};

    /* Read value of target register */
    while (I2C_GetFlagState(ALC5628_I2C, I2C_FLAG_TFE) == RESET);
    I2C_RepeatRead(ALC5628_I2C, &reg_address, 1, &reg_data[0], 2);

    return true;
}

/**
  * @brief  Initializes the ALC5628.
  * @param None.
  * @retval None
  */
void ALC5628_Init(void)
{
    I2C_InitTypeDef  I2C_InitStructure;

    /*-------------------Enable Bee I2C clock------------------*/
    if (ALC5628_I2C == I2C0)
    {
        RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);
    }
    else
    {
        if (ALC5628_I2C == I2C1)
        {
            RCC_PeriphClockCmd(APBPeriph_I2C1, APBPeriph_I2C1_CLOCK, ENABLE);
        }
    }

    /*-------------------I2C init ----------------------------*/
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed = ALC5628_I2C_Speed;
    I2C_InitStructure.I2C_DeviveMode = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = ALC5628_Address;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_Init(ALC5628_I2C, &I2C_InitStructure);
    I2C_Cmd(ALC5628_I2C, ENABLE);

    /*-------------------ALC5628 init section------------------*/

    /*-------------------Power block enable-------------------*/

    /* reg_0x3c: enable pow_vref*/
    ALC5628_Write(ALC5628_I2C, RT5628_PWR_MANAG_ADD2, PWR_VREF);
    /* reg_0x3e: Enable pow_main_bias*/
    ALC5628_Write(ALC5628_I2C, RT5628_PWR_MANAG_ADD3, PWR_MAIN_BIAS);
    /* reg_0x5e: Enable fast Vref */
    ALC5628_ResetBits(ALC5628_I2C, RT5628_MISC2_CTRL, FAST_VREG_EN);

    /* ------------------system clock config ------------------*/

    /* reg_0x44: set pll parameter: MCLK is 10M */
    ALC5628_Write(ALC5628_I2C, RT5628_PLL_CTRL,
                  0x4b2f);//0x4b2f, 0x0702, 0x986f//44.1KHz, //0x2229//5MHz,  0x397e and 0x1029 //10M   0x1ee0//2.8224M

    /* reg_0x42: clock source MUX control (PLL not XTI) , form BCLK */
    ALC5628_Write(ALC5628_I2C, RT5628_GLOBAL_CLK_CTRL, SYSCLK_SOUR_SEL_PLL | PLLCLK_SOUR_SEL_BITCLK);

    /* reg_0x42: PLL source select MCLK (not BIT_CLK)*/
    //ALC5628_ResetBits(ALC5628_I2C, RT5628_GLOBAL_CLK_CTRL, PLLCLK_SOUR_SEL_MCLK);
    //ALC5628_SetBits(ALC5628_I2C, RT5628_GLOBAL_CLK_CTRL, PLLCLK_SOUR_SEL_BITCLK);

    /* reg_0x3c: Power on PLL , PLL is used to generate I2S required clock for Audio DAC by input an unrelated reference clock */
    ALC5628_SetBits(ALC5628_I2C, RT5628_PWR_MANAG_ADD2, PWR_PLL);

    //delay 10m sec

    /*------------------Audio DAC configuration----------------*/

    /*------------------audio data control---------------------*/
    /* reg_0x34: slave mode, I2S mode */
    ALC5628_Write(ALC5628_I2C, RT5628_AUDIO_DATA_CTRL, SDP_SLAVE_MODE | I2S_DF_I2S | I2S_DL_16);
    /* reg_0x38: prescaler config, BLCK/FS= 64 */
    //ALC5628_ResetBits(ALC5628_I2C, RT5628_DAC_CLK_CTRL, I2S_PRE_DIV_MASK);
    ALC5628_Write(ALC5628_I2C, RT5628_DAC_CLK_CTRL,
                  I2S_PRE_DIV_1 | I2S_BCLK_SEL_64FS | DAC_FILTER_CLK_SEL_256FS);

    /* reg_0x1c: headphone  mixer */
    ALC5628_Write(ALC5628_I2C, RT5628_OUTPUT_MIXER_CTRL, SPKOUT_N_SOUR_MASK | DAC_DIRECT_TO_HP);

    /* reg_0x3e: enable headphone left and right volume */
    ALC5628_SetBits(ALC5628_I2C, RT5628_PWR_MANAG_ADD3, PWR_HP_R_OUT_VOL_AMP | PWR_HP_L_OUT_VOL_AMP);

    /*-------------------adjust volume----------------------*/

    /* reg_0x0c: Stereo DAC Volume to 0 db */
    ALC5628_Write(ALC5628_I2C, RT5628_STEREO_DAC_VOL, (3 << 14) | (3 << 6) | (0x10 << 8) | 0x10);

    /* reg_0x04: headphone output Volume */
    ALC5628_Write(ALC5628_I2C, RT5628_HP_OUT_VOL, 0x00);

    /* reg_0x3a: enable I2S interface */
    ALC5628_Write(ALC5628_I2C, RT5628_PWR_MANAG_ADD1, PWR_MAIN_I2S_EN | PWR_HP_ENH_AMP | PWR_HP_AMP);

    /* reg_0x3c: enable left and right DAC , I2S clcok is output */
    ALC5628_SetBits(ALC5628_I2C, RT5628_PWR_MANAG_ADD2,
                    PWR_DAC_REF_CIR | PWR_R_DAC_CLK | PWR_L_DAC_CLK);

#if 0
    ALC5628_Write(ALC5628_I2C, 0x3a, 0x8030);
    ALC5628_Write(ALC5628_I2C, 0x3c, 0x37F0);
    ALC5628_Write(ALC5628_I2C, 0x3e, 0x8600);
    ALC5628_Write(ALC5628_I2C, 0x0c, 0x5050);
    ALC5628_Write(ALC5628_I2C, 0x1c, 0xC300);
    ALC5628_Write(ALC5628_I2C, 0x04, 0x0000);
#endif

#if 0
    /* Dump registers */
    DBG_DIRECT("0x3a = 0x%x", ALC5628_ReadRegister(ALC5628_I2C, 0x3a));
    DBG_DIRECT("0x3c = 0x%x", ALC5628_ReadRegister(ALC5628_I2C, 0x3c));
    DBG_DIRECT("0x3e = 0x%x", ALC5628_ReadRegister(ALC5628_I2C, 0x3e));
    DBG_DIRECT("0x0c = 0x%x", ALC5628_ReadRegister(ALC5628_I2C, 0x0c));
    DBG_DIRECT("0x04 = 0x%x", ALC5628_ReadRegister(ALC5628_I2C, 0x04));
    DBG_DIRECT("0x42 = 0x%x", ALC5628_ReadRegister(ALC5628_I2C, 0x42));
#endif
}

/**
  * @brief  Configure mixer parameter.
  * @param data: data format.
    which can be I2S_DF_I2S, I2S_DF_LEFT, I2S_DF_PCM_A and I2S_DF_PCM_B.
  * @retval function state.
  */
uint8_t ALC5628_HeadphoneMixerConfig(uint16_t data)
{
    uint8_t reg_address = RT5628_OUTPUT_MIXER_CTRL;
    uint8_t reg_data[2] = {0, 0};


    /* Read value of target register */
    I2C_RepeatRead(ALC5628_I2C, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    ALC5628_I2C->IC_DATA_CMD = reg_address;

    /* Send data */
    ALC5628_I2C->IC_DATA_CMD = data | (reg_data[0] & (~(HP_VOLUME_MASK)));
    ALC5628_I2C->IC_DATA_CMD = (reg_data[1]) | (1 << 9);

    return true;
}

/**
  * @brief  Configure stereo PCM data format.
  * @param data: data format.
    which can be I2S_DF_I2S, I2S_DF_LEFT, I2S_DF_PCM_A and I2S_DF_PCM_B.
  * @retval function state.
  */
uint8_t ALC5628_DataFormatConfig(uint16_t data)
{
    uint8_t reg_address = RT5628_AUDIO_DATA_CTRL;
    uint8_t reg_data[2] = {0, 0};

    /* Check parameter */
    assert_param(IS_DATA_FORMAT(data));

    /* Read value of target register */
    I2C_RepeatRead(ALC5628_I2C, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    ALC5628_I2C->IC_DATA_CMD = reg_address;

    /* Send data */
    ALC5628_I2C->IC_DATA_CMD = reg_data[0];
    ALC5628_I2C->IC_DATA_CMD = data | (reg_data[1] & (~(I2S_DF_MASK))) | BIT(9);

    return true;
}

/**
  * @brief  Configure head phone volume .
  * @param Headphone_Channel: select head phone channel, which can be HP_Channel_Left or HP_Channel_Right.
  * @param data: volume which can be 0(0 dB attenuation) to 0x1f(46.5 dB attenuation).
  * @retval function state.
  */
uint8_t ALC5628_HeadphoneVolumeConfig(uint8_t Headphone_Channel, uint16_t data)
{
    uint8_t reg_address = RT5628_HP_OUT_VOL;
    uint8_t reg_data[2] = {0, 0};

    /* Check parameter */
    assert_param(IS_HP_CHANNEL(Headphone_Channel));

    /* Read value of target register */
    I2C_RepeatRead(ALC5628_I2C, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    ALC5628_I2C->IC_DATA_CMD = reg_address;

    if (Headphone_Channel == DAC_Channel_Left)
    {
        /* Send data */
        ALC5628_I2C->IC_DATA_CMD = data | (reg_data[0] & (~(HP_VOLUME_MASK)));
        ALC5628_I2C->IC_DATA_CMD = (reg_data[1]) | (1 << 9);
    }
    else
    {
        if (Headphone_Channel == DAC_Channel_Right)
        {
            /* Send data */
            ALC5628_I2C->IC_DATA_CMD = reg_data[0];
            ALC5628_I2C->IC_DATA_CMD = data | (reg_data[1] & (~(HP_VOLUME_MASK))) | (1 << 9);
        }
    }

    return true;
}

#if 1
/**
  * @brief  Config the volume of DAC in ALC5628.
  * @param DAC_channel: select DAC channel, which can be DAC_Channel_Left or DAC_Channel_Right.
  * @param data: volume data of DAC, which can be 0x00 to 0x3f.
    0x00 is +12dB gain, 0x10 is 0dB attenuation.0x3f is 35.25dB attenuation.
  * @retval
  */
uint8_t ALC5628_DACVolumeConfig(uint8_t DAC_Channel, uint16_t data)
{
    uint8_t reg_address = 0x0c;
    uint8_t reg_data[2] = {0, 0};

    /* Check parameter */
    assert_param(IS_DAC_CHANNEL(DAC_Channel));

    /* Read value of target register */
    I2C_RepeatRead(ALC5628_I2C, &reg_address, 1, &reg_data[0], 2);

    /* Send register address of ALC5628 */
    ALC5628_I2C->IC_DATA_CMD = reg_address;

    if (DAC_Channel == DAC_Channel_Left)
    {
        /* Send data */
        ALC5628_I2C->IC_DATA_CMD = data | (reg_data[0] & (~(DAC_VOLUME_MASK)));
        ALC5628_I2C->IC_DATA_CMD = (reg_data[1]) | (1 << 9);
    }
    else
    {
        if (DAC_Channel == DAC_Channel_Right)
        {
            /* Send data */
            ALC5628_I2C->IC_DATA_CMD = reg_data[0];
            ALC5628_I2C->IC_DATA_CMD = data | (reg_data[1] & (~(DAC_VOLUME_MASK))) | (1 << 9);
        }
    }

    return true;
}

#else
/**
  * @brief  Config the volume of DAC in ALC5628.
  * @param DAC_channel: select DAC channel:.
  * @param data: volume data of DAC, which can be 0x00 to 0x3f.
    0x00 is +12dB gain, 0x10 is 0dB attenuation.0x3f is 35.25dB attenuation.
  * @retval
  */
uint8_t ALC5628_DACVolumeConfig(uint8_t DAC_Channel, uint16_t data)
{
    uint8_t reg_address = 0x0c;
    uint16_t reg_data = 0;

    /* Check parameter */
    assert_param(IS_DAC_CHANNEL(DAC_Channel));

    /*----------------Read value of target register---------------*/

    /* Send register address of ALC5628 */
    ALC5628_I2C->IC_DATA_CMD = reg_address;

    /* Send first read command */
    ALC5628_I2C->IC_DATA_CMD = BIT(8);

    /* Send second read command  and stop signal */
    ALC5628_I2C->IC_DATA_CMD = BIT(8) | BIT(9);

    /* wait for Rx FIFO not empty flag */
    while ((ALC5628_I2C->IC_STATUS & (1 << 3)) == 0);
    /* read high byte of register */
    reg_data = (uint8_t)ALC5628_I2C->IC_DATA_CMD;
    reg_data = reg_data << 8;

    /* wait for Rx FIFO not empty flag */
    while ((ALC5628_I2C->IC_STATUS & (1 << 3)) == 0);
    /* read low byte of register */
    reg_data = reg_data | (uint8_t)ALC5628_I2C->IC_DATA_CMD;

    /*---------------write value of target register---------------*/

    /* Send register address of ALC5628 */
    ALC5628_I2C->IC_DATA_CMD = reg_address;

#if 1
    /* Send data */
    reg_data = (data << DAC_Channel) | (reg_data & (~(DAC_VOLUME_MASK << DAC_Channel)));

    ALC5628_I2C->IC_DATA_CMD = reg_data >> 8;
    ALC5628_I2C->IC_DATA_CMD = (reg_data & 0xff) | (1 << 9);
#else
    if (DAC_Channel == DAC_Channel_Left)
    {
        /* Send data */
        ALC5628_I2C->IC_DATA_CMD = data | (reg_data[0] & (~(DAC_VOLUME_MASK)));
        ALC5628_I2C->IC_DATA_CMD = (reg_data[1]) | (1 << 9);
    }
    else
    {
        if (DAC_Channel == DAC_Channel_Right)
        {
            /* Send data */
            ALC5628_I2C->IC_DATA_CMD = reg_data[0];
            ALC5628_I2C->IC_DATA_CMD = data | (reg_data[1] & (~(DAC_VOLUME_MASK))) | (1 << 9);
        }
    }
#endif
}
#endif
#endif
/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/

