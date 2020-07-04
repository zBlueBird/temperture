#include "SI7021.h"
#include "trace.h"
#include "rtl876x_i2c.h"
#include "si7021_i2c_driver.h"

//结构体定义
_si7021_value si7021;//数据缓存区结构体
_si7021_filter si7021_filter;//平均值滤波器结构体

//变量定义，滤波后的最终结果，可使用串口打印
float TEMP_buf, Humi_buf;

#if 1
//函数名称：single_write_Si7021
//函数功能：单字节写入传感器
//参数描述：
////返 回 值：
//void single_write_Si7021(uint8_t REG_address)
//{
//  //IIC_Start();
//
//  IIC_Send_Byte((SLAVE_ADDR<<1)|0);
//  IIC_Wait_Ack();
//
//  IIC_Send_Byte(REG_address);
//  IIC_Wait_Ack();
//
//  IIC_Stop();
//}

//函数名称：Multiple_read_Si7021
//函数功能：多字节读取传感器
//参数描述：
//返 回 值：
void Multiple_read_Si7021(uint8_t REG_address, uint16_t *value)
{
    uint8_t Si7021_BUF[2] = {0};

    RS_Sensors_I2C_ReadRegister(0x40, REG_address, sizeof(Si7021_BUF), Si7021_BUF);

//  Si7021_BUF[0] = 0x67;
//  Si7021_BUF[1] = 0xFC;

    *value = (((uint16_t)(Si7021_BUF[0] << 8) & 0xff00) | (uint8_t)Si7021_BUF[1]);

}
#endif

//函数名称：measure_si7021
//函数功能：NO HOLD MASTER模式下读取温湿度
//参数描述：无
//返 回 值：无
void measure_Si7021(float *pTemp, float *pHumi)
{
    //缓存变量定义
    uint16_t TEMP, HUMI;
    uint8_t curI;

    //读取温度
    Multiple_read_Si7021(TEMP_NOHOLD_MASTER, &TEMP);
    si7021.temp = (((((float)TEMP) * 175.72f) / 65536.0f) -
                   46.85f); //将原始温度数据计算为实际温度数据并传递给缓存区，单位 ℃
//  TEMP_buf=(((((float)TEMP)*175.72f)/65536.0f) - 46.85f);

    Multiple_read_Si7021(HUMI_NOHOLD_MASTER, &HUMI);
    si7021.humi = (((((float)HUMI) * 125.0f) / 65535.0f) -
                   6.0f); //将原始湿度数据计算为实际湿度数据并传递给缓存区，单位 %RH
//  Humi_buf=(((((float)HUMI)*125.0f)/65535.0f) - 6.0f);

    //以下为平均值滤波代码，循环储存10次的数据，调用一次measure_Si7021()就存一次
    if (MEAN_NUM > si7021_filter.curI) //当MEAN_NUM==10时，完成10次读取
    {
        si7021_filter.tBufs[si7021_filter.curI] = si7021.temp;
        si7021_filter.hBufs[si7021_filter.curI] = si7021.humi;

        si7021_filter.curI++;
    }
    else
    {
        si7021_filter.curI = 0;

        si7021_filter.tBufs[si7021_filter.curI] = si7021.temp;
        si7021_filter.hBufs[si7021_filter.curI] = si7021.humi;

        si7021_filter.curI++;
    }

    if (MEAN_NUM <= si7021_filter.curI)
    {
        si7021_filter.thAmount = MEAN_NUM;
    }

    //判断是否初次循环
    if (0 == si7021_filter.thAmount)
    {
        //计算采集第10次数据之前的平均值
        for (curI = 0; curI < si7021_filter.curI; curI++)
        {
            si7021.temp += si7021_filter.tBufs[curI];
            si7021.humi += si7021_filter.hBufs[curI];
        }

        si7021.temp = si7021.temp / si7021_filter.curI;
        si7021.humi = si7021.humi / si7021_filter.curI;

        *pTemp = si7021.temp;
        *pHumi = si7021.humi;
    }
    else if (MEAN_NUM == si7021_filter.thAmount)
    {
        //计算采集第10次数据之后的平均值
        for (curI = 0; curI < si7021_filter.thAmount; curI++)
        {
            si7021.temp += si7021_filter.tBufs[curI];
            si7021.humi += si7021_filter.hBufs[curI];
        }

        si7021.temp = si7021.temp / si7021_filter.thAmount;
        si7021.humi = si7021.humi / si7021_filter.thAmount;

        *pTemp = si7021.temp;
        *pHumi = si7021.humi;
    }
}

//#include "swtimer.h"
//#include "os_timer.h"
//#define AUTO_TEST_TIMER_TICK 2000
//TimerHandle_t auto_test_timer;
//void anto_test_timer_callback(TimerHandle_t p_timer);
///**
// * @brief auto test init.
// */
//void auto_test_init(void)
//{
//    /* no_act_disconn_timer is used to disconnect after timeout if there is on action under connection */
//    if (false == os_timer_create(&auto_test_timer, "auto_test_timer",  1, \
//                                 AUTO_TEST_TIMER_TICK, false, anto_test_timer_callback))
//    {
//        APP_PRINT_INFO0("[sw_timer_init] init no_act_disconn_timer failed");
//    }
//    else
//    {
//        APP_PRINT_INFO0("[sw_timer_init] init success");
//    }

//    os_timer_restart(&auto_test_timer, AUTO_TEST_TIMER_TICK);
//}

//void anto_test_timer_callback(TimerHandle_t p_timer)
//{
//    os_timer_restart(&auto_test_timer, AUTO_TEST_TIMER_TICK);

//    measure_Si7021();

//    APP_PRINT_INFO2("TEMP_buf = %d, Humi_buf = %f", (uint16_t)(TEMP_buf * 10),
//                    (uint16_t)(Humi_buf * 10));
//}
