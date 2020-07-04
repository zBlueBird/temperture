/*******************************************************************************
File    : mpu_i2c.c
Purpose : I2c 3 to communicate with the sensors
Author  :
********************************** Includes ***********************************/
#include <stdio.h>
#include "rtl876x.h"
#include "si7021_i2c_driver.h"
#include "rtl876x_rcc.h"
#include "rtl876x_i2c.h"
#include "rtl876x_pinmux.h"
#include "board.h"
#include "platform_utils.h"
#include "app_section.h"
#include <string.h>
#include "si7021.h"
#include "trace.h"
/********************************* Defines ************************************/

#define mdelay(n) delay_ms(n)


#define I2Cx_FLAG_TIMEOUT             ((uint32_t) 900) //0x1100
#define I2Cx_LONG_TIMEOUT             ((uint32_t) (300 * I2Cx_FLAG_TIMEOUT)) //was300


#define WAIT_FOR_FLAG(flag, value, timeout, errorcode)  I2CTimeout = timeout;\
    while(I2C_GetFlagStatus(SENSORS_I2C, flag) != value) {\
        if((I2CTimeout--) == 0) return I2Cx_TIMEOUT_UserCallback(errorcode); \
    }\

#define CLEAR_ADDR_BIT      I2C_ReadRegister(SENSORS_I2C, I2C_Register_SR1);\
    I2C_ReadRegister(SENSORS_I2C, I2C_Register_SR2);\

/********************************* Globals ************************************/

/********************************* Prototypes *********************************/
unsigned long RS_Sensors_I2C_WriteRegister(unsigned char Address, unsigned char RegisterAddr,
                                           unsigned short RegisterLen, const unsigned char *RegisterValue);
unsigned long RS_Sensors_I2C_ReadRegister(unsigned char Address, unsigned char RegisterAddr,
                                          unsigned short RegisterLen, unsigned char *RegisterValue);
/*******************************  Function ************************************/

#if 1//RCU_MOUSE_EN

void delay_us(uint32_t n)
{
    platform_delay_us(n);
}

void delay_ms(uint32_t n)
{
    platform_delay_ms(n);
}

void get_ms(unsigned long *time)
{
    *time = 0;
}

void I2cMaster_Init(void)
{
    Pinmux_Deinit(I2C0_SCL);
    Pinmux_Deinit(I2C0_SDA);

    RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);

    Pad_Config(I2C0_SCL, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(I2C0_SCL, I2C0_CLK);
    Pinmux_Config(I2C0_SDA, I2C0_DAT);

    Pad_PullConfigValue(I2C0_SCL, PAD_STRONG_PULL);
    Pad_PullConfigValue(I2C0_SDA, PAD_STRONG_PULL);

    /************I2C configuration************************/
    I2C_InitTypeDef  I2C_InitStructure;

    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed = 40000;
    I2C_InitStructure.I2C_DeviveMode = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = SLAVE_ADDR;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;

    I2C_Init(I2C0, &I2C_InitStructure);
    I2C_Cmd(I2C0, DISABLE);
    I2C_Cmd(I2C0, ENABLE);

    I2C_SetSlaveAddress(I2C0, SLAVE_ADDR);

}
#if 0
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
static uint32_t I2Cx_TIMEOUT_UserCallback(char value)
{
    return 1;
}
#endif

int Sensors_I2C_WriteRegister(unsigned char slave_addr,
                              unsigned char reg_addr,
                              unsigned short len,
                              const unsigned char *data_ptr)
{
    char retries = 0;
    int ret = 0;
    unsigned short retry_in_mlsec = Get_I2C_Retry();

tryWriteAgain:
    ret = 0;
    ret = RS_Sensors_I2C_WriteRegister(slave_addr, reg_addr, len, data_ptr);

    if (ret && retry_in_mlsec)
    {
        if (retries++ > 4)
        {
            return ret;
        }

        mdelay(retry_in_mlsec);
        goto tryWriteAgain;
    }
    return ret;
}

int Sensors_I2C_ReadRegister(unsigned char slave_addr,
                             unsigned char reg_addr,
                             unsigned short len,
                             unsigned char *data_ptr)
{
    char retries = 0;
    int ret = 0;
    unsigned short retry_in_mlsec = Get_I2C_Retry();

tryReadAgain:
    ret = 0;
    ret = RS_Sensors_I2C_ReadRegister(slave_addr, reg_addr, len, data_ptr);

    if (ret && retry_in_mlsec)
    {
        if (retries++ > 4)
        {
            return ret;
        }

        mdelay(retry_in_mlsec);
        goto tryReadAgain;
    }
    return ret;
}


/**
  * @brief  Writes a Byte to a given register to the sensors through the
            control interface (I2C)
  * @param  RegisterAddr: The address (location) of the register to be written.
  * @param  RegisterValue: the Byte value to be written into destination register.
  * @retval 0 if correct communication, else wrong communication
  */
DATA_RAM_FUNCTION unsigned long RS_Sensors_I2C_WriteRegister(unsigned char Address,
                                                             unsigned char RegisterAddr,
                                                             unsigned short RegisterLen,
                                                             const unsigned char *RegisterValue)
{
    uint8_t data_sent[17];
    data_sent[0] = RegisterAddr;
    memcpy(&data_sent[1], RegisterValue, RegisterLen);

    while (I2C_GetFlagState(I2C0, I2C_FLAG_TFE) == RESET \
           || I2C_GetFlagState(I2C0, I2C_FLAG_RFNE) == SET \
           || I2C_GetFlagState(I2C0, I2C_FLAG_MST_ACTIVITY) == SET) {};

    if (I2C_Success != I2C_MasterWrite(I2C0, data_sent, RegisterLen + 1))
    {
        return 1;
    }


    return 0;
}

DATA_RAM_FUNCTION unsigned long RS_Sensors_I2C_ReadRegister(unsigned char Address,
                                                            unsigned char RegisterAddr,
                                                            unsigned short RegisterLen,
                                                            unsigned char *RegisterValue)
{
    I2C_Status status = I2C_Success;
    while (I2C_GetFlagState(I2C0, I2C_FLAG_TFE) == RESET \
           || I2C_GetFlagState(I2C0, I2C_FLAG_RFNE) == SET \
           || I2C_GetFlagState(I2C0, I2C_FLAG_MST_ACTIVITY) == SET) {};
    //Pad_Config(ICM_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    status = I2C_RepeatRead(I2C0, &RegisterAddr, 1, RegisterValue, RegisterLen);
    //Pad_Config(ICM_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    if (status == I2C_Success)
    {
        return 0;
    }
    return 1;

}

static unsigned short RETRY_IN_MLSEC  = 55;

void Set_I2C_Retry(unsigned short ml_sec)
{
    RETRY_IN_MLSEC = ml_sec;
}

unsigned short Get_I2C_Retry(void)
{
    return RETRY_IN_MLSEC;
}


#endif//IR_FUN

