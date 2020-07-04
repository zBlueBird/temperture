#ifndef _SI7021_H
#define _SI7021_H

#include "si7021_i2c_driver.h"

//SI7021_SCL
//SI7021_SDA

//数据缓存区结构体
typedef struct
{
    float temp;
    float humi;
    uint8_t crc;
} _si7021_value;

//平均值滤波器结构体
typedef struct
{
    uint8_t curI;
    uint8_t thAmount;
    float tBufs[10];
    float hBufs[10];
} _si7021_filter;

//外部声明，滤波后的最终结果，可使用串口打印
extern float TEMP_buf, Humi_buf;

/************多少个数据参与平均值滤波************/
#define MEAN_NUM  10

/*******************传感器相关*******************/
//#define   SLAVE_ADDR      0x40    //7位器件地址

#define HUMI_HOLD_MASTER    0xE5
#define TEMP_HOLD_MASTER    0xE3

#define HUMI_NOHOLD_MASTER  0xF5
#define TEMP_NOHOLD_MASTER  0xF3

#define Si7021_RST          0xFE
#define Write_UserReg       0xE6
#define Read_UserReg        0xE7


void single_write_Si7021(uint8_t REG_address);
void Multiple_read_Si7021(uint8_t REG_address, uint16_t *value);
void measure_Si7021(float *pTemp, float *pHumi);

#endif
