#ifndef _VS1053_HEAD_
#define _VS1053_HEAD_
#include "rtl876x.h"

#define  VS1053B_SPIx   SPI0


//RIFF块
typedef __packed struct
{
    uint32_t ChunkID;            //chunk id;这里固定为"RIFF",即0X46464952
    uint32_t ChunkSize ;         //集合大小;文件总大小-8
    uint32_t Format;             //格式;WAVE,即0X45564157
} ChunkRIFF ;
//fmt块
typedef __packed struct
{
    uint32_t ChunkID;            //chunk id;这里固定为"fmt ",即0X20746D66
    uint32_t ChunkSize ;         //子集合大小(不包括ID和Size);这里为:20.
    uint16_t AudioFormat;        //音频格式;0X10,表示线性PCM;0X11表示IMA ADPCM
    uint16_t NumOfChannels;      //通道数量;1,表示单声道;2,表示双声道;
    uint32_t SampleRate;         //采样率;0X1F40,表示8Khz
    uint32_t ByteRate;           //字节速率;
    uint16_t BlockAlign;         //块对齐(字节);
    uint16_t BitsPerSample;      //单个采样数据大小;4位ADPCM,设置为4
//  uint16_t ByteExtraData;      //附加的数据字节;2个; 线性PCM,没有这个参数
//  uint16_t ExtraData;          //附加的数据,单个采样数据块大小;0X1F9:505字节  线性PCM,没有这个参数
} ChunkFMT;
//fact块
typedef __packed struct
{
    uint32_t ChunkID;            //chunk id;这里固定为"fact",即0X74636166;
    uint32_t ChunkSize ;         //子集合大小(不包括ID和Size);这里为:4.
    uint32_t NumOfSamples;       //采样的数量;
} ChunkFACT;
//data块
typedef __packed struct
{
    uint32_t ChunkID;            //chunk id;这里固定为"data",即0X61746164
    uint32_t ChunkSize ;         //子集合大小(不包括ID和Size);文件大小-60.
} ChunkDATA;

//wav头
typedef __packed struct
{
    ChunkRIFF riff; //riff块
    ChunkFMT fmt;   //fmt块
    //ChunkFACT fact;   //fact块 线性PCM,没有这个结构体
    ChunkDATA data; //data块
} __WaveHeader;

__packed typedef struct
{
    uint8_t mvol;        //主音量,范围:0~254
    uint8_t bflimit;     //低音频率限定,范围:2~15(单位:10Hz)
    uint8_t bass;        //低音,范围:0~15.0表示关闭.(单位:1dB)
    uint8_t tflimit;     //高音频率限定,范围:1~15(单位:Khz)
    uint8_t treble;      //高音,范围:0~15(单位:1.5dB)(原本范围是:-8~7,通过函数修改了);
    uint8_t effect;      //空间效果设置.0,关闭;1,最小;2,中等;3,最大.
    uint8_t speakersw;   //板载喇叭开关,0,关闭;1,打开
    uint8_t saveflag;    //保存标志,0X0A,保存过了;其他,还从未保存
} _vs10xx_obj;
#define VS_WRITE_COMMAND    0x02
#define VS_READ_COMMAND     0x03
//VS10XX寄存器定义
#define SPI_MODE            0x00
#define SPI_STATUS          0x01
#define SPI_BASS            0x02
#define SPI_CLOCKF          0x03
#define SPI_DECODE_TIME     0x04
#define SPI_AUDATA          0x05
#define SPI_WRAM            0x06
#define SPI_WRAMADDR        0x07
#define SPI_HDAT0           0x08
#define SPI_HDAT1           0x09

#define SPI_AIADDR          0x0a
#define SPI_VOL             0x0b
#define SPI_AICTRL0         0x0c
#define SPI_AICTRL1         0x0d
#define SPI_AICTRL2         0x0e
#define SPI_AICTRL3         0x0f
#define SM_DIFF             0x01
#define SM_JUMP             0x02
#define SM_RESET            0x04
#define SM_OUTOFWAV         0x08
#define SM_PDOWN            0x10
#define SM_TESTS            0x20
#define SM_STREAM           0x40
#define SM_PLUSV            0x80
#define SM_DACT             0x100
#define SM_SDIORD           0x200
#define SM_SDISHARE         0x400
#define SM_SDINEW           0x800
#define SM_ADPCM            0x1000
#define SM_ADPCM_HP         0x2000

#define I2S_CONFIG          0XC040
#define GPIO_DDR            0XC017
#define GPIO_IDATA          0XC018
#define GPIO_ODATA          0XC019


#define BIT_POOL_SIZE   28//14  /* BIT_POOL_SIZE is support to adjust */
#define VOICE_PCM_FRAME_CNT              2
#define VOICE_PCM_FRAME_SIZE             256
#define VOICE_FRAME_SIZE_AFTER_ENC       (2 * BIT_POOL_SIZE + 8)
#define VOICE_REPORT_FRAME_SIZE          (VOICE_FRAME_SIZE_AFTER_ENC * VOICE_PCM_FRAME_CNT)
#define VOICE_GDMA_FRAME_SIZE            (VOICE_PCM_FRAME_SIZE * VOICE_PCM_FRAME_CNT)
typedef struct
{
    uint8_t buf0[VOICE_GDMA_FRAME_SIZE];
    uint8_t buf1[VOICE_GDMA_FRAME_SIZE];
} T_GDMA_BUF_TYPE_DEF;

typedef struct
{
    bool is_allowed_to_enter_dlps;  /* to indicate whether to allow to enter dlps or not */
    bool is_voice_driver_working;  /* indicate whether voice driver is working or not */
    uint8_t current_bibuff_index;  /* indicate which buffer the voice using now */
    //T_GDMA_BUF_TYPE_DEF voice_buf_index;
} T_VOICE_DRIVER_GLOBAL_DATA;


/*buffer data num*/
#define  BUFFER_LEN      2048
#define  VOICE_DATA_BUFFER_LENGTH      (BUFFER_LEN + 1)

#define  KEY_BUFFER_LOG_EN 0
typedef struct
{
    uint16_t   head;
    uint16_t   tail;
    uint8_t   buf[VOICE_DATA_BUFFER_LENGTH];
} buffer_key_stg;

extern void vs1053b_gpio_init(void);
extern void vs1053b_driver_spi_init(void);
extern uint8_t recoder_play(void);
extern void vs1053b_read_buffer(uint16_t num);
extern void VS_Sine_Test(void);
extern void vs1053b_recoder_start(uint8_t recagc);
extern uint16_t vs1053b_recoder_check_buffer_length(void);

extern bool key_queue_in(uint8_t key_index);
extern bool key_queue_out(uint8_t *poBuf, uint16_t length);
extern bool key_queue_clear(void);
extern bool key_queue_print(uint8_t idx);
extern bool key_queue_check_level(uint16_t level_value);

#endif
