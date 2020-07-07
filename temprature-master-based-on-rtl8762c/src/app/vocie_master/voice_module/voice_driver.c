#include <string.h>
#include "board.h"
#include <trace.h>
#include "voice_driver.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_codec.h"
#include "rtl876x_spi.h"
//#include "swtimer.h"
#include "os_timer.h"
#include "app_msg.h"
#include <app_task.h>
#include "audio_handle.h"
#include "app_section.h"

#if 0

/*============================================================================*
 *                              Local Variables
 *============================================================================*/


/*============================================================================*
 *                              External Variables
 *============================================================================*/

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
DATA_RAM_FUNCTION T_VOICE_DRIVER_GLOBAL_DATA voice_driver_global_data;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
static void voice_driver_init_pad_and_pinmux(void)
{

    Pinmux_Deinit(SPI0_SCK_PIN);
    Pinmux_Deinit(SPI0_MOSI_PIN);
    Pinmux_Deinit(SPI0_MISO_PIN);
    //Pinmux_Deinit(SPI0_CS_PIN);
    Pinmux_Config(SPI0_SCK_PIN, SPI0_CLK_MASTER);
    Pinmux_Config(SPI0_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI0_MISO_PIN, SPI0_MI_MASTER);

    Pad_Config(SPI0_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    //Pad_Config(SPI0_CS_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

}

/**
  * @brief  Initialize I2S peripheral.
  * @param   No parameter.
  * @return  void
  */
static void voice_driver_init_spi0(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GDMA_InitTypeDef GDMA_InitStruct;


    /*----------------SPI init---------------------------*/
    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler  = 8;
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_InitStructure.SPI_RxThresholdLevel  = 1;/* Flash id lenth = 3*/
    SPI_InitStructure.SPI_NDF               = 0;/* Flash id lenth = 3*/

    /*---------------------GDMA initial------------------------------*/
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_Channel_RX_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = VOICE_GDMA_FRAME_SIZE / 4;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_16;

    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)SPI0->DR;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)voice_driver_global_data.gdma_buffer.buf0;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPI0_RX;

    GDMA_Init(GDMA_Channel_RX, &GDMA_InitStruct);
    SPI_Init(SPI0, &SPI_InitStructure);

    SPI_Cmd(SPI0, ENABLE);

    /*-----------------GDMA IRQ-----------------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA0_Channel_RX_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    /* Enable transfer interrupt */
    GDMA_INTConfig(GDMA_Channel_RX_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_Cmd(GDMA_Channel_RX_NUM, ENABLE);
}


void voice_driver_gpio_init(void)
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
/**
  * @brief  Deinitialize GDMA peripheral.
  * @param   No parameter.
  * @return  void
  */
static void voice_driver_deinit_rx_gdma(void)
{
    GDMA_Cmd(GDMA_Channel_RX_NUM, DISABLE);
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
/**
* @brief  Initialize voice driver global data
*/
void voice_driver_init_data(void)
{
    APP_PRINT_INFO0("[voice_driver_init_data] init data");
    memset(&voice_driver_global_data, 0, sizeof(voice_driver_global_data));
    voice_driver_global_data.is_allowed_to_enter_dlps = true;
}

/**
* @brief  Initialize voice driver.
* @param   No parameter.
* @return  void
*/
void voice_driver_init(void)
{
    voice_driver_init_data();
    voice_driver_global_data.is_allowed_to_enter_dlps = false;
    voice_driver_global_data.is_voice_driver_working = true;

    voice_driver_init_pad_and_pinmux();
    voice_driver_init_spi0();
    voice_driver_gpio_init();
    //voice_driver_init_codec();
    //voice_driver_init_rx_gdma();
}

/**
* @brief  Deinitialize voice driver.
* @param   No parameter.
* @return  void
*/
void voice_driver_deinit(void)
{
    voice_driver_global_data.is_allowed_to_enter_dlps = true;
    voice_driver_global_data.is_voice_driver_working = false;

    {
        voice_driver_deinit_rx_gdma();
    }
}

/**
* @brief GDMA interrupt handler function.
* @param   No parameter.
* @return  void
*/
DATA_RAM_FUNCTION
void GDMA0_Channel_RX_Handler(void)
{
    GDMA_ClearINTPendingBit(GDMA_Channel_RX_NUM, GDMA_INT_Transfer);
    if (voice_driver_global_data.is_voice_driver_working == true)
    {
        T_IO_MSG gdma_msg;

        APP_PRINT_INFO0("[GDMA0_Channel_RX_Handler] GDMA interrupt!");
        gdma_msg.type = IO_MSG_TYPE_GDMA;
        gdma_msg.subtype = 0;
        if (voice_driver_global_data.current_bibuff_index == 0)
        {
            gdma_msg.u.buf = (void *)voice_driver_global_data.gdma_buffer.buf0;
        }
        else
        {
            gdma_msg.u.buf = (void *)voice_driver_global_data.gdma_buffer.buf1;
        }
        app_send_msg_to_apptask(&gdma_msg);
        GDMA_SetSourceAddress(GDMA_Channel_RX, (uint32_t)(&(I2S0->RX_DR)));
        if (voice_driver_global_data.current_bibuff_index == 0)
        {
            voice_driver_global_data.current_bibuff_index = 1;
            GDMA_SetDestinationAddress(GDMA_Channel_RX, (uint32_t)voice_driver_global_data.gdma_buffer.buf1);
        }
        else
        {
            voice_driver_global_data.current_bibuff_index = 0;
            GDMA_SetDestinationAddress(GDMA_Channel_RX, (uint32_t)voice_driver_global_data.gdma_buffer.buf0);
        }
        GDMA_SetBufferSize(GDMA_Channel_RX, VOICE_GDMA_FRAME_SIZE / 4);
        GDMA_Cmd(GDMA_Channel_RX_NUM, ENABLE);
    }
}

#endif


