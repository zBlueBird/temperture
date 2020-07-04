/*********************************************************************
File    : i2c.h
Purpose :
**********************************************************************/
#ifndef __MPU_I2C_H__
#define __MPU_I2C_H__
/****************************** Includes *****************************/
#include "rtl876x.h"

/****************************** Defines *******************************/
#define SENSORS_I2C               I2C2


#define   I2C0_SCL                            P0_0
#define   I2C0_SDA                            P0_1

#define   SLAVE_ADDR                 (0x40 )    //7Î»Æ÷¼þµØÖ·

#define I2C_SPEED                 400000
#define I2C_OWN_ADDRESS           0x00

#define I2C_Config() I2cMaster_Init();


void Set_I2C_Retry(unsigned short ml_sec);
unsigned short Get_I2C_Retry(void);

/*==================================================================
*                      I2C External API
==================================================================*/

void delay_us(unsigned int n);
void delay_ms(unsigned int n);
void get_ms(unsigned long *time);
void I2cMaster_Init(void);

int Sensors_I2C_ReadRegister(unsigned char Address, unsigned char RegisterAddr,
                             unsigned short RegisterLen, unsigned char *RegisterValue);
int Sensors_I2C_WriteRegister(unsigned char Address, unsigned char RegisterAddr,
                              unsigned short RegisterLen, const unsigned char *RegisterValue);
unsigned long RS_Sensors_I2C_WriteRegister(unsigned char Address, unsigned char RegisterAddr,
                                           unsigned short RegisterLen, const unsigned char *RegisterValue);
unsigned long RS_Sensors_I2C_ReadRegister(unsigned char Address, unsigned char RegisterAddr,
                                          unsigned short RegisterLen, unsigned char *RegisterValue);

#endif // __MPU_I2C_H__


