#include "board.h"
#include "vs1053b.h"
#include "rtl876x_spi.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_gpio.h"
#include "rtl876x_uart.h"
#include "trace.h"
#include "platform_utils.h"
#include <os_mem.h>

#if VS1053B_EN
/*============================================================================*
 *                         Macros
 *============================================================================*/
#define  delay_ms(n)  platform_delay_ms(n)
#define  delay_us(n)  platform_delay_us(n)

#define VS_DQ                      GPIO_ReadInputDataBit(GPIO_GetPin(VS_DQ_PIN))
#define VS_RST_RESET()             GPIO_WriteBit(GPIO_GetPin(VS_RST_PIN), (BitAction)(0))
#define VS_RST_SET()               GPIO_WriteBit(GPIO_GetPin(VS_RST_PIN), (BitAction)(1))

#define VS_XCS_RESET()             GPIO_WriteBit(GPIO_GetPin(VS_XCS_PIN), (BitAction)(0))
#define VS_XCS_SET()               GPIO_WriteBit(GPIO_GetPin(VS_XCS_PIN), (BitAction)(1))

#define VS_XDCS_RESET()            GPIO_WriteBit(GPIO_GetPin(VS_XDCS_PIN), (BitAction)(0))
#define VS_XDCS_SET()              GPIO_WriteBit(GPIO_GetPin(VS_XDCS_PIN), (BitAction)(1))

/*============================================================================*
 *                         Local Variables
 *============================================================================*/

T_VOICE_DRIVER_GLOBAL_DATA voice_driver_global_data = {0};
//VS10XX defalt parameters
_vs10xx_obj vsset =
{
    220,    //volume:220
    6,      //低音上线 60Hz
    15,     //低音提升 15dB
    10,     //高音下限 10Khz
    15,     //高音提升 10.5dB
    0,      //空间效果
    1,      //板载喇叭默认打开.
};

//VS1053 WAV recorder have bugs, need load this plugin
const uint16_t wav_plugin[40] = /* Compressed plugin */
{
    0x0007, 0x0001, 0x8010, 0x0006, 0x001c, 0x3e12, 0xb817, 0x3e14, /* 0 */
    0xf812, 0x3e01, 0xb811, 0x0007, 0x9717, 0x0020, 0xffd2, 0x0030, /* 8 */
    0x11d1, 0x3111, 0x8024, 0x3704, 0xc024, 0x3b81, 0x8024, 0x3101, /* 10 */
    0x8024, 0x3b81, 0x8024, 0x3f04, 0xc024, 0x2808, 0x4800, 0x36f1, /* 18 */
    0x9811, 0x0007, 0x0001, 0x8028, 0x0006, 0x0002, 0x2a00, 0x040e,
};

buffer_key_stg g_key_ready_data = {0};
/*============================================================================*
 *                         External Functions
 *============================================================================*/

extern void data_uart_send(uint8_t *pbuf, uint16_t length);
/*============================================================================*
*                         Local Functions
*============================================================================*/
uint16_t  VS_RD_Reg(uint8_t address);             //读寄存器
uint16_t  VS_WRAM_Read(uint16_t addr);            //读RAM
void VS_WRAM_Write(uint16_t addr, uint16_t val);  //写RAM
void VS_WR_Data(uint8_t data);               //写数据
void VS_WR_Cmd(uint8_t address, uint16_t data);   //写命令
uint8_t   VS_HD_Reset(void);                 //硬复位
void VS_Soft_Reset(void);               //软复位
uint16_t VS_Ram_Test(void);                  //RAM测试
void VS_Sine_Test(void);                //正弦测试

uint8_t   VS_SPI_ReadWriteByte(uint8_t data);
void VS_SPI_SpeedLow(void);
void VS_SPI_SpeedHigh(void);
void VS_Init(void);                     //初始化VS10XX
void VS_Set_Speed(uint8_t t);                //设置播放速度
uint16_t  VS_Get_HeadInfo(void);             //得到比特率
uint32_t VS_Get_ByteRate(void);              //得到字节速率
uint16_t VS_Get_EndFillByte(void);           //得到填充字节
uint8_t   VS_Send_MusicData(uint8_t *buf);        //向VS10XX发送32字节
void VS_Restart_Play(void);             //重新开始下一首歌播放
void VS_Reset_DecodeTime(void);         //重设解码时间
uint16_t  VS_Get_DecodeTime(void);           //得到解码时间

void VS_Load_Patch(uint16_t *patch, uint16_t len); //加载用户patch
uint8_t   VS_Get_Spec(uint16_t *p);               //得到分析数据
void VS_Set_Bands(uint16_t *buf, uint8_t bands);  //设置中心频率
void VS_Set_Vol(uint8_t volx);               //设置主音量
void VS_Set_Bass(uint8_t bfreq, uint8_t bass, uint8_t tfreq, uint8_t treble); //设置高低音
void VS_Set_Effect(uint8_t eft);             //设置音效
void VS_SPK_Set(uint8_t sw);                 //板载喇叭输出开关控制
void VS_Set_All(void);


/*******************************************************
*                 SPI Drivers
*******************************************************/
void spi0_pinmux_config(void)
{
    Pinmux_Deinit(SPI0_SCK_PIN);
    Pinmux_Deinit(SPI0_MOSI_PIN);
    Pinmux_Deinit(SPI0_MISO_PIN);
    //Pinmux_Deinit(SPI0_CS_PIN);
    Pinmux_Config(SPI0_SCK_PIN, SPI0_CLK_MASTER);
    Pinmux_Config(SPI0_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI0_MISO_PIN, SPI0_MI_MASTER);
}
/**
* @brief spi0 pins pad config
*
*/
void spi0_init_pad_config(void)
{
    Pad_Config(SPI0_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    //Pad_Config(SPI0_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
}
/**
* @brief vs1053b module config
* @param none
*/
void vs1053b_driver_spi_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    spi0_pinmux_config();
    spi0_init_pad_config();

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 16;
    /* SPI_Direction_EEPROM mode read data lenth. */
    SPI_InitStruct.SPI_RxThresholdLevel  = 1;/* Flash id lenth = 3*/
    SPI_InitStruct.SPI_NDF               = 0;/* Flash id lenth = 3*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_Init(VS1053B_SPIx, &SPI_InitStruct);

    SPI_Cmd(VS1053B_SPIx, ENABLE);
}

void vs1053b_gpio_init(void)
{
    /*0. pad config*/
    Pad_Config(VS_DQ_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(VS_RST_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(VS_XCS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(VS_XDCS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    /*1. pinmux config*/
    Pinmux_Config(VS_DQ_PIN, DWGPIO);
    Pinmux_Config(VS_RST_PIN,  DWGPIO);
    Pinmux_Config(VS_XCS_PIN,  DWGPIO);
    Pinmux_Config(VS_XDCS_PIN, DWGPIO);

    /*2. clock*/
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    /*3. gpio init*/
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(VS_DQ_PIN);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(VS_RST_PIN);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(VS_XCS_PIN);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(VS_XDCS_PIN);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);

}



uint8_t vs1053b_spi_write_read(uint8_t addr)
{
    uint8_t write_value;
    uint8_t len = 0;
    uint8_t retry = 0;
    uint8_t ret_value = 0;

    //Pad_Config(SPI0_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON,  PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    write_value = (addr & 0xff);
    SPI_SendBuffer(VS1053B_SPIx, &write_value, 1);
    while (SPI_GetFlagState(VS1053B_SPIx, SPI_FLAG_BUSY))
    {
        retry++;
        if (retry > 200) { return 0; }
    }

    len = SPI_GetRxFIFOLen(VS1053B_SPIx);
    while (len--)
    {
        ret_value = SPI_ReceiveData(VS1053B_SPIx);
    }

    return ret_value;
}

void vs1053b_SetSpeed(uint8_t SpeedSet)
{
    SPI_Change_CLK(VS1053B_SPIx, SpeedSet);
}



/*******************************************************
*                 Vs1053b module config
*******************************************************/
uint8_t VS_SPI_ReadWriteByte(uint8_t data)
{
    return vs1053b_spi_write_read(data);
}

void VS_SPI_SpeedLow(void)
{
    vs1053b_SetSpeed(SPI_BaudRatePrescaler_32);//设置到低速模式
}

void VS_SPI_SpeedHigh(void)
{
    vs1053b_SetSpeed(SPI_BaudRatePrescaler_8);//设置到高速模式
}

/*
* software reset vs1053b
*/
void VS_Soft_Reset(void)
{
    uint8_t retry = 0;
    while (VS_DQ == 0);                 //wait software reset end
    VS_SPI_ReadWriteByte(0Xff);         //wait for transfer
    retry = 0;
    while (VS_RD_Reg(SPI_MODE) != 0x0800) // software reset, new mode
    {
        VS_WR_Cmd(SPI_MODE, 0x0804);    //sw reset, new mode
        delay_ms(2);//wait at least 1.35ms
        if (retry++ > 100) { break; }
    }
    while (VS_DQ == 0); //wait sw reset end
    retry = 0;
    while (VS_RD_Reg(SPI_CLOCKF) != 0X9800) //set vs1053 clock, 3 frequency doubling, 1.5xADD
    {
        VS_WR_Cmd(SPI_CLOCKF, 0X9800);  //set vs1053 clock, 3 frequency doubling, 1.5xADD
        if (retry++ > 100) { break; }
    }
    delay_ms(20);
}
uint8_t VS_HD_Reset(void)
{
    uint8_t retry = 0;
    VS_RST_RESET();
    delay_ms(200);
    VS_XDCS_SET(); //cancel data transfer
    VS_XCS_SET(); //cancel data transfer
    VS_RST_SET();
    while (VS_DQ == 0 && retry < 200) //wait DREQ high
    {
        retry++;
        delay_us(50);
    };
    delay_ms(200);
    if (retry >= 200) { return 1; }
    else { return 0; }
}

//正弦测试
void VS_Sine_Test(void)
{
    VS_HD_Reset();
    VS_WR_Cmd(0x0b, 0X2020);  //设置音量
    VS_WR_Cmd(SPI_MODE, 0x0820); //进入VS10XX的测试模式
    while (VS_DQ == 0);  //等待DREQ为高
    //printf("mode sin:%x\n",VS_RD_Reg(SPI_MODE));
    //向VS10XX发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
    //其中n = 0x24, 设定VS10XX所产生的正弦波的频率值，具体计算方法见VS10XX的datasheet
    VS_SPI_SpeedLow();//低速
    VS_XDCS_RESET(); //选中数据传输
    VS_SPI_ReadWriteByte(0x53);
    VS_SPI_ReadWriteByte(0xef);
    VS_SPI_ReadWriteByte(0x6e);
    VS_SPI_ReadWriteByte(0x24);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    delay_ms(400);
    VS_XDCS_SET();
    //退出正弦测试
    VS_XDCS_RESET(); //选中数据传输
    VS_SPI_ReadWriteByte(0x45);
    VS_SPI_ReadWriteByte(0x78);
    VS_SPI_ReadWriteByte(0x69);
    VS_SPI_ReadWriteByte(0x74);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    delay_ms(200);
    VS_XDCS_SET();

    //再次进入正弦测试并设置n值为0x44，即将正弦波的频率设置为另外的值
    VS_XDCS_RESET(); //选中数据传输
    VS_SPI_ReadWriteByte(0x53);
    VS_SPI_ReadWriteByte(0xef);
    VS_SPI_ReadWriteByte(0x6e);
    VS_SPI_ReadWriteByte(0x44);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    delay_ms(200);
    VS_XDCS_SET();
    //退出正弦测试
    VS_XDCS_RESET(); //选中数据传输
    VS_SPI_ReadWriteByte(0x45);
    VS_SPI_ReadWriteByte(0x78);
    VS_SPI_ReadWriteByte(0x69);
    VS_SPI_ReadWriteByte(0x74);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    VS_SPI_ReadWriteByte(0x00);
    delay_ms(200);
    VS_XDCS_SET();


}

//向VS10XX写命令
//address:命令地址
//data:命令数据
void VS_WR_Cmd(uint8_t address, uint16_t data)
{
    //DBG_DIRECT("003");
    while (VS_DQ == 0); //等待空闲
    //DBG_DIRECT("004");
    VS_SPI_SpeedLow();//低速
    VS_XDCS_SET();
    VS_XCS_RESET();
    VS_SPI_ReadWriteByte(VS_WRITE_COMMAND);//发送VS10XX的写命令
    VS_SPI_ReadWriteByte(address);  //地址
    VS_SPI_ReadWriteByte(data >> 8); //发送高八位
    VS_SPI_ReadWriteByte(data);     //第八位
    VS_XCS_SET();
    VS_SPI_SpeedHigh();             //高速
}
//向VS10XX写数据
//data:要写入的数据
void VS_WR_Data(uint8_t data)
{
    VS_SPI_SpeedHigh();//高速,对VS1003B,最大值不能超过36.864/4Mhz，这里设置为9M
    VS_XDCS_RESET();
    VS_SPI_ReadWriteByte(data);
    VS_XDCS_SET();
}
//读VS10XX的寄存器
//address：寄存器地址
//返回值：读到的值
//注意不要用倍速读取,会出错
uint16_t VS_RD_Reg(uint8_t address)
{
    uint16_t temp = 0;
    while (VS_DQ == 0); //非等待空闲状态
    VS_SPI_SpeedLow();//低速
    VS_XDCS_SET();
    VS_XCS_RESET();
    VS_SPI_ReadWriteByte(VS_READ_COMMAND);  //发送VS10XX的读命令
    VS_SPI_ReadWriteByte(address);          //地址
    temp = VS_SPI_ReadWriteByte(0xff);      //读取高字节
    temp = temp << 8;
    temp += VS_SPI_ReadWriteByte(0xff);     //读取低字节
    VS_XCS_SET();
    VS_SPI_SpeedHigh();//高速
    return temp;
}
//读取VS10xx的RAM
//addr：RAM地址
//返回值：读到的值
uint16_t VS_WRAM_Read(uint16_t addr)
{
    uint16_t res;
    VS_WR_Cmd(SPI_WRAMADDR, addr);
    res = VS_RD_Reg(SPI_WRAM);
    return res;
}
//写VS10xx的RAM
//addr：RAM地址
//val:要写入的值
void VS_WRAM_Write(uint16_t addr, uint16_t val)
{
    VS_WR_Cmd(SPI_WRAMADDR, addr);  //写RAM地址
    while (VS_DQ == 0);             //等待空闲
    VS_WR_Cmd(SPI_WRAM, val);       //写RAM值
}
//设置播放速度（仅VS1053有效）
//t:0,1,正常速度;2,2倍速度;3,3倍速度;4,4倍速;以此类推
void VS_Set_Speed(uint8_t t)
{
    VS_WRAM_Write(0X1E04, t);       //写入播放速度
}
//FOR WAV HEAD0 :0X7761 HEAD1:0X7665
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//比特率预定值,阶层III
const uint16_t bitrate[2][16] =
{
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
    {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}
};
//返回Kbps的大小
//返回值：得到的码率
uint16_t VS_Get_HeadInfo(void)
{
    unsigned int HEAD0;
    unsigned int HEAD1;
    HEAD0 = VS_RD_Reg(SPI_HDAT0);
    HEAD1 = VS_RD_Reg(SPI_HDAT1);
    //printf("(H0,H1):%x,%x\n",HEAD0,HEAD1);
    switch (HEAD1)
    {
    case 0x7665://WAV格式
    case 0X4D54://MIDI格式
    case 0X4154://AAC_ADTS
    case 0X4144://AAC_ADIF
    case 0X4D34://AAC_MP4/M4A
    case 0X4F67://OGG
    case 0X574D://WMA格式
    case 0X664C://FLAC格式
        {
            ////printf("HEAD0:%d\n",HEAD0);
            HEAD1 = HEAD0 * 2 / 25; //相当于*8/100
            if ((HEAD1 % 10) > 5) { return HEAD1 / 10 + 1; } //对小数点第一位四舍五入
            else { return HEAD1 / 10; }
        }
    default://MP3格式,仅做了阶层III的表
        {
            HEAD1 >>= 3;
            HEAD1 = HEAD1 & 0x03;
            if (HEAD1 == 3) { HEAD1 = 1; }
            else { HEAD1 = 0; }
            return bitrate[HEAD1][HEAD0 >> 12];
        }
    }
}
//得到平均字节数
//返回值：平均字节数速度
uint32_t VS_Get_ByteRate(void)
{
    return VS_WRAM_Read(0X1E05);//平均位速
}
//得到需要填充的数字
//返回值:需要填充的数字
uint16_t VS_Get_EndFillByte(void)
{
    return VS_WRAM_Read(0X1E06);//填充字节
}
//发送一次音频数据
//固定为32字节
//返回值:0,发送成功
//       1,VS10xx不缺数据,本次数据未成功发送
uint8_t VS_Send_MusicData(uint8_t *buf)
{
    uint8_t n;
    if (VS_DQ != 0) //送数据给VS10XX
    {
        VS_XDCS_RESET();
        for (n = 0; n < 32; n++)
        {
            VS_SPI_ReadWriteByte(buf[n]);
        }
        VS_XDCS_SET();
    }
    else { return 1; }
    return 0;//成功发送了
}
//切歌
//通过此函数切歌，不会出现切换“噪声”
void VS_Restart_Play(void)
{
    uint16_t temp;
    uint16_t i;
    uint8_t n;
    uint8_t vsbuf[32];
    for (n = 0; n < 32; n++) { vsbuf[n] = 0; } //清零
    temp = VS_RD_Reg(SPI_MODE); //读取SPI_MODE的内容
    temp |= 1 << 3;             //设置SM_CANCEL位
    temp |= 1 << 2;             //设置SM_LAYER12位,允许播放MP1,MP2
    VS_WR_Cmd(SPI_MODE, temp);  //设置取消当前解码指令
    for (i = 0; i < 2048;)      //发送2048个0,期间读取SM_CANCEL位.如果为0,则表示已经取消了当前解码
    {
        if (VS_Send_MusicData(vsbuf) == 0) //每发送32个字节后检测一次
        {
            i += 32;                    //发送了32个字节
            temp = VS_RD_Reg(SPI_MODE); //读取SPI_MODE的内容
            if ((temp & (1 << 3)) == 0) { break; } //成功取消了
        }
    }
    if (i < 2048) //SM_CANCEL正常
    {
        temp = VS_Get_EndFillByte() & 0xff; //读取填充字节
        for (n = 0; n < 32; n++) { vsbuf[n] = temp; } //填充字节放入数组
        for (i = 0; i < 2052;)
        {
            if (VS_Send_MusicData(vsbuf) == 0) { i += 32; } //填充
        }
    }
    else { VS_Soft_Reset(); }       //SM_CANCEL不成功,坏情况,需要软复位
    temp = VS_RD_Reg(SPI_HDAT0);
    temp += VS_RD_Reg(SPI_HDAT1);
    if (temp)                   //软复位,还是没有成功取消,放杀手锏,硬复位
    {
        VS_HD_Reset();          //硬复位
        VS_Soft_Reset();        //软复位
    }
}
//重设解码时间
void VS_Reset_DecodeTime(void)
{
    VS_WR_Cmd(SPI_DECODE_TIME, 0x0000);
    VS_WR_Cmd(SPI_DECODE_TIME, 0x0000); //操作两次
}
//得到mp3的播放时间n sec
//返回值：解码时长
uint16_t VS_Get_DecodeTime(void)
{
    uint16_t dt = 0;
    dt = VS_RD_Reg(SPI_DECODE_TIME);
    return dt;
}
//vs10xx装载patch.
//patch：patch首地址
//len：patch长度
void VS_Load_Patch(uint16_t *patch, uint16_t len)
{
    uint16_t i;
    uint16_t addr, n, val;
    for (i = 0; i < len;)
    {
        addr = patch[i++];
        n    = patch[i++];
        if (n & 0x8000U) //RLE run, replicate n samples
        {
            n  &= 0x7FFF;
            val = patch[i++];
            while (n--) { VS_WR_Cmd(addr, val); }
        }
        else  //copy run, copy n sample
        {
            while (n--)
            {
                val = patch[i++];
                VS_WR_Cmd(addr, val);
            }
        }
    }
}
//设定VS10XX播放的音量和高低音
//volx:音量大小(0~254)
void VS_Set_Vol(uint8_t volx)
{
    uint16_t volt = 0;           //暂存音量值
    volt = 254 - volx;      //取反一下,得到最大值,表示最大的表示
    volt <<= 8;
    volt += 254 - volx;     //得到音量设置后大小
    VS_WR_Cmd(SPI_VOL, volt); //设音量
}
//设定高低音控制
//bfreq:低频上限频率    2~15(单位:10Hz)
//bass:低频增益         0~15(单位:1dB)
//tfreq:高频下限频率    1~15(单位:Khz)
//treble:高频增益       0~15(单位:1.5dB,小于9的时候为负数)
void VS_Set_Bass(uint8_t bfreq, uint8_t bass, uint8_t tfreq, uint8_t treble)
{
    uint16_t bass_set = 0; //暂存音调寄存器值
    signed char temp = 0;
    if (treble == 0) { temp = 0; }      //变换
    else if (treble > 8) { temp = treble - 8; }
    else { temp = treble - 9; }
    bass_set = temp & 0X0F;         //高音设定
    bass_set <<= 4;
    bass_set += tfreq & 0xf;        //高音下限频率
    bass_set <<= 4;
    bass_set += bass & 0xf;         //低音设定
    bass_set <<= 4;
    bass_set += bfreq & 0xf;        //低音上限
    VS_WR_Cmd(SPI_BASS, bass_set);  //BASS
}
//设定音效
//eft:0,关闭;1,最小;2,中等;3,最大.
void VS_Set_Effect(uint8_t eft)
{
    uint16_t temp;
    temp = VS_RD_Reg(SPI_MODE); //读取SPI_MODE的内容
    if (eft & 0X01) { temp |= 1 << 4; } //设定LO
    else { temp &= ~(1 << 5); }     //取消LO
    if (eft & 0X02) { temp |= 1 << 7; } //设定HO
    else { temp &= ~(1 << 7); }     //取消HO
    VS_WR_Cmd(SPI_MODE, temp);  //设定模式
}
//板载喇叭开/关设置函数.
//战舰V3板载了HT6872功放,通过VS1053的GPIO4(36脚),控制其工作/关闭.
//GPIO4=1,HT6872正常工作.
//GPIO4=0,HT6872关闭(默认)
//sw:0,关闭;1,开启.
void VS_SPK_Set(uint8_t sw)
{
    VS_WRAM_Write(GPIO_DDR, 1 << 4); //VS1053的GPIO4设置成输出
    VS_WRAM_Write(GPIO_ODATA, sw << 4); //控制VS1053的GPIO4输出值(0/1)
}
///////////////////////////////////////////////////////////////////////////////
//设置音量,音效等.
void VS_Set_All(void)
{
    VS_Set_Vol(vsset.mvol);         //设置音量
    VS_Set_Bass(vsset.bflimit, vsset.bass, vsset.tflimit, vsset.treble);
    VS_Set_Effect(vsset.effect);    //设置空间效果
    VS_SPK_Set(vsset.speakersw);    //控制板载喇叭状态
}

//inactive PCM record mode
void recoder_enter_rec_mode(uint16_t agc)
{
    VS_WR_Cmd(SPI_BASS, 0x0000);
    VS_WR_Cmd(SPI_AICTRL0, 16000);
    VS_WR_Cmd(SPI_AICTRL1, agc);
    VS_WR_Cmd(SPI_AICTRL2, 0);
    VS_WR_Cmd(SPI_AICTRL3, 6);
    VS_WR_Cmd(SPI_CLOCKF, 0X2000);
    //VS_WR_Cmd(SPI_MODE, 0x1804);
    VS_WR_Cmd(SPI_MODE, 0x5804);
    delay_ms(20);
    VS_Load_Patch((uint16_t *)wav_plugin, 40);// load patch
}
//init wav header
void recoder_wav_init(__WaveHeader *wavhead) //初始化WAV头
{
    wavhead->riff.ChunkID = 0X46464952; //"RIFF"
    wavhead->riff.ChunkSize = 0;
    wavhead->riff.Format = 0X45564157;  //"WAVE"
    wavhead->fmt.ChunkID = 0X20746D66;  //"fmt "
    wavhead->fmt.ChunkSize = 16;        //16 bytes size
    wavhead->fmt.AudioFormat = 0X01;    //0X01,PCM;   0X01, IMA ADPCM
    wavhead->fmt.NumOfChannels = 1;     //mono
    wavhead->fmt.SampleRate = 8000;     //8Khz
    wavhead->fmt.ByteRate = wavhead->fmt.SampleRate * 2; //16bits
    wavhead->fmt.BlockAlign = 2;        //block size, 2bytes per block
    wavhead->fmt.BitsPerSample = 16;    //16 bits pcm
    wavhead->data.ChunkID = 0X61746164; //"data"
    wavhead->data.ChunkSize = 0;        //data size
}

void vs1053b_recoder_start(uint8_t recagc)
{
    recoder_enter_rec_mode(1024 * recagc);
}

uint16_t vs1053b_recoder_check_buffer_length()
{
    return (uint16_t)VS_RD_Reg(SPI_HDAT1);
}

void vs1053b_read_buffer(uint16_t num)
{
    uint16_t data = 0;

    for (uint16_t index = 0; index < num; index ++)
    {
        data = VS_RD_Reg(SPI_HDAT0);

        //data_uart_send(&data, sizeof(data));

        key_queue_in(data & 0xff);

        key_queue_in(((data & 0xff00) >> 8) & 0xff);
    }


}

uint8_t recoder_play(void)
{
    uint8_t rval = 0;
    //uint8_t recbuf[512] = {0};                     //数据内存
    uint16_t w;
    uint8_t recagc = 4;                  //默认增益为4

    if (rval == 0)                              //内存申请OK
    {
        //DBG_DIRECT("001");
        recoder_enter_rec_mode(1024 * recagc);
        //while (VS_RD_Reg(SPI_HDAT1));       //等到buf 较为空闲再开始
        //DBG_DIRECT("002");
        while (1)
        {
            w = VS_RD_Reg(SPI_HDAT1);
            if (w >= 256)
            {
                //while (idx < 512) //一次读取512字节
                {
                    w = VS_RD_Reg(SPI_HDAT0);

                    data_uart_send((uint8_t *)&w, sizeof(w));
                }
                //res = f_write(f_rec, recbuf, 512, &bw); //写入文件
            }
        }
    }
    return rval;
}

void data_uart_send(uint8_t *pbuf, uint16_t length)
{
    uint16_t blk_cnt, remainder;
    uint8_t *p_buf = pbuf;

    blk_cnt = (length) / UART_TX_FIFO_SIZE;
    remainder = (length) % UART_TX_FIFO_SIZE;
    /* send voice data through uart */
    for (int i = 0; i < blk_cnt; i++)
    {
        /* 1. max send 16 bytes(Uart tx and rx fifo size is 16) */
        UART_SendData(UART, p_buf, 16);
        /* wait tx fifo empty */
        while (UART_GetFlagState(UART, UART_FLAG_THR_EMPTY) != SET);
        p_buf += 16;
    }

    /* send left bytes */
    UART_SendData(UART, p_buf, remainder);
    /* wait tx fifo empty */
    while (UART_GetFlagState(UART, UART_FLAG_THR_EMPTY) != SET);
}

/*=============================================================
*                buffer voice data queue
*==============================================================*/
/**
* @brief   check key queue full
* @param   none
* @return  bool true full, false not full
*/
bool is_key_queue_full(void)
{
    if (((g_key_ready_data.head + 1) % VOICE_DATA_BUFFER_LENGTH) == g_key_ready_data.tail)
    {
#if 1//KEY_BUFFER_LOG_EN
        APP_PRINT_INFO0("[is_key_queue_full] is full");
#endif
        return true;
    }
    else
    {
#if KEY_BUFFER_LOG_EN
        //APP_PRINT_INFO0("[is_key_queue_full] not full");
#endif
        return false;
    }
}
/**
* @brief   check key queue empty
* @param   none
* @return  bool true empty, false not empty
*/
bool is_key_queue_empty(void)
{
    if (g_key_ready_data.head == g_key_ready_data.tail)
    {
#if 1//KEY_BUFFER_LOG_EN
        APP_PRINT_INFO0("[is_key_queue_empty] is empty");
#endif
        return true;
    }
    else
    {
#if KEY_BUFFER_LOG_EN
        //APP_PRINT_INFO0("[is_key_queue_empty] not empty");
#endif
        return false;
    }
}

/**
* @brief   buffer key in queue
* @param   uint8_t key_index value in queue
* @return  bool true inqueue success, else false
*/
bool key_queue_in(uint8_t key_index)
{
    if (is_key_queue_full())
    {
#if KEY_BUFFER_LOG_EN
        //full queue
        APP_PRINT_INFO0("[key_queue_in] drop the oldest data, full");
#endif
        g_key_ready_data.tail = (g_key_ready_data.tail + 1) % VOICE_DATA_BUFFER_LENGTH;;
    }

    if (is_key_queue_full())
    {
#if KEY_BUFFER_LOG_EN
        APP_PRINT_INFO0("[key_queue_in] queue in failed, full");
#endif
        return false;
    }


    g_key_ready_data.buf[g_key_ready_data.head] = key_index;
    g_key_ready_data.head = (g_key_ready_data.head + 1) % VOICE_DATA_BUFFER_LENGTH;
#if KEY_BUFFER_LOG_EN
    APP_PRINT_INFO1("[key_queue_in] queue in success, key_index = %d", key_index);
    APP_PRINT_INFO2("[key_queue_in] queue, g_key_ready_data.head = %d, g_key_ready_data.tail = %d",
                    g_key_ready_data.head, g_key_ready_data.tail);
#endif

    return true;
}

/**
* @brief   buffer key out queue
* @param   none
* @return  uint8_t value out queue
*/
bool key_queue_out(uint8_t *poBuf, uint16_t length)
{
    if (poBuf == NULL)
    {
        return false;
    }

    if (length == 0)
    {
        return false;
    }

    //uint8_t key_index = VK_NC;
    if (is_key_queue_empty())
    {
#if KEY_BUFFER_LOG_EN
        //empty
        APP_PRINT_INFO0("[key_queue_out] queue out failed, empty.");
#endif
        return false;
    }
    else
    {
#if KEY_BUFFER_LOG_EN
        APP_PRINT_INFO2("[key_queue_out]before, head = %d, tail = %d",
                        g_key_ready_data.head, g_key_ready_data.tail);
#endif

        for (uint16_t index = 0; index < (length); index ++)
        {
            *(poBuf + index) = g_key_ready_data.buf[(g_key_ready_data.tail)];
            g_key_ready_data.tail = (g_key_ready_data.tail + 1) % VOICE_DATA_BUFFER_LENGTH;
        }
#if KEY_BUFFER_LOG_EN
        APP_PRINT_INFO2("[key_queue_out] after, head = %d, tail = %d",
                        g_key_ready_data.head, g_key_ready_data.tail);
#endif
    }

    return true;
}
/**
* @brief   clear key buffer queue
* @param   none
* @return  none
* @note    noly clear when triggle reconnect adv
*/
bool key_queue_clear(void)
{
#if KEY_BUFFER_LOG_EN
    APP_PRINT_INFO0("[key_queue_clear] key queue clear.");
#endif
    g_key_ready_data.head = g_key_ready_data.tail;

    memset((void *)&g_key_ready_data, 0, sizeof(g_key_ready_data));
    return true;
}

bool key_queue_print(uint8_t idx)
{
#if KEY_BUFFER_LOG_EN
    APP_PRINT_INFO3("[key_queue_print (%d)] queue, head = %d, tail = %d",
                    idx, g_key_ready_data.head, g_key_ready_data.tail);
#endif

    for (uint8_t index = g_key_ready_data.tail; index != (g_key_ready_data.head);)
    {
        APP_PRINT_INFO2("[key_queue_print] g_key_ready_data[%d] = %d", index, g_key_ready_data.buf[index]);
        index = (index + 1) % VOICE_DATA_BUFFER_LENGTH;
    }

    return true;
}

bool key_queue_check_level(uint16_t level_value)
{
    if (is_key_queue_empty())
    {
//            APP_PRINT_INFO2("[key_queue_out0] false key_queue_check_level, head = %d, tail = %d",
//                                      g_key_ready_data.head, g_key_ready_data.tail);
        return false;
    }


    if (g_key_ready_data.tail < g_key_ready_data.head)
    {
        if (g_key_ready_data.tail + level_value < g_key_ready_data.head)
        {
//                      APP_PRINT_INFO2("[key_queue_out2] true key_queue_check_level, head = %d, tail = %d",
//                                      g_key_ready_data.head, g_key_ready_data.tail);
            return true;
        }
        else
        {
//                     APP_PRINT_INFO2("[key_queue_out1] false key_queue_check_level, head = %d, tail = %d",
//                                      g_key_ready_data.head, g_key_ready_data.tail);
            return false;
        }
    }
    else
    {
        if ((g_key_ready_data.tail + level_value) < (g_key_ready_data.head + VOICE_DATA_BUFFER_LENGTH))
        {
//                    APP_PRINT_INFO2("[key_queue_out2] true key_queue_check_level, head = %d, tail = %d",
//                                      g_key_ready_data.head, g_key_ready_data.tail);
            return true;
        }
        else
        {
//                    APP_PRINT_INFO2("[key_queue_out2] false key_queue_check_level, head = %d, tail = %d",
//                                      g_key_ready_data.head, g_key_ready_data.tail);
            return false;
        }
    }
}

#endif
