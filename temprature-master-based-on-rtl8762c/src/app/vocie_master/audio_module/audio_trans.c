/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     audio_trans.c
* @brief    This file provides audio transmission layer module driver.
* @details
* @author
* @date     2018-03-20
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "audio_trans.h"
#include "audio_bi_buffer.h"
#include "string.h"
#include "app_msg.h"
#include "audio_handle.h"
#include "app_task.h"
#include "app_section.h"

#if FEATURE_SUPPORT_AUDIO_DOWN_STREAMING

/*Define --------------------------------------------------------------------*/
#define AutoTrans_BLOCK_SIZE            (AUDIO_BI_BUFFER_MAX_SIZE)

/* LLI structure for I2S audio stream control */
GDMA_LLIDef AudioTrans_LLIStruct[2];

/* Global variable defines ------------------------------------------------------*/

/**
  * @brief  Configure pin for I2S communication.
  * @param  None.
  * @retval None
  */
void Board_AudioTrans_Init(void)
{
    Pad_Config(AUDIO_CODEC_I2S_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_CODEC_I2S_LR_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(AUDIO_CODEC_I2S_DATA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pinmux_Config(AUDIO_CODEC_I2S_BCLK_PIN, I2S_USR_BCLK);
    Pinmux_Config(AUDIO_CODEC_I2S_LR_PIN, I2S_USR_LRC);
    Pinmux_Config(AUDIO_CODEC_I2S_DATA_PIN, I2S_USR_DACDAT);
    I2S_MCLKOutputSelectCmd(I2S_USR);
}

/**
  * @brief  Initialize I2S in audio transmission layer.
  * @param   No parameter.
  * @return  void
  */
void Driver_AudioTrans_Init(T_AUDIO_FREQ sampling_freq)
{
    RCC_PeriphClockCmd(APB_I2S_USR, APB_I2S_USR_CLK, DISABLE);
    RCC_PeriphClockCmd(APB_I2S_USR, APB_I2S_USR_CLK, ENABLE);

    if (I2S_USR == I2S0)
    {
        I2S0_WithExtCodecCmd(ENABLE);
    }

    I2S_InitTypeDef I2S_InitStruct;
    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_ClockSource      = I2S_CLK_40M;

    /* set I2S BClock */
    switch (sampling_freq)
    {
    case AUDIO_FREQU16000:
        I2S_InitStruct.I2S_BClockMi         = 6250;
        I2S_InitStruct.I2S_BClockNi         = 160;
        break;

    case AUDIO_FREQU32000:
        I2S_InitStruct.I2S_BClockMi         = 6250;
        I2S_InitStruct.I2S_BClockNi         = 320;
        break;

    case AUDIO_FREQU44100:
        I2S_InitStruct.I2S_BClockMi         = 6250;
        I2S_InitStruct.I2S_BClockNi         = 441;
        break;

    case AUDIO_FREQU48000:
        I2S_InitStruct.I2S_BClockMi         = 6250;
        I2S_InitStruct.I2S_BClockNi         = 480;
        break;

    default:
        /* set to 44.1K */
        I2S_InitStruct.I2S_BClockMi         = 6250;
        I2S_InitStruct.I2S_BClockNi         = 441;
        break;
    }

    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_ChannelType      = I2S_Channel_Mono;//I2S_Channel_stereo;
    I2S_InitStruct.I2S_DataWidth        = I2S_Width_16Bits;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_InitStruct.I2S_DMACmd           = I2S_DMA_ENABLE;
    I2S_InitStruct.I2S_MCLKOutput       = I2S_MCLK_256fs;
    I2S_InitStruct.I2S_TxWaterlevel     = 32 - 16;
    I2S_InitStruct.I2S_DMACmd           = I2S_DMA_ENABLE;
    I2S_Init(I2S_USR, &I2S_InitStruct);

    I2S_Cmd(I2S_USR, I2S_MODE_TX, ENABLE);
}

/**
  * @brief  Configure I2S data rate.
  * @param   data_rate_Hz: data rate
  * @param   pI2S_InitStruct: pointer to a I2S_InitTypeDef structure that
  *   contains the configuration information for the specified I2S peripheral.
  * @return  void
  */
void AudioTrans_ConfigDataRate(T_AUDIO_FREQ sampling_freq)
{
    switch (sampling_freq)
    {
    case AUDIO_FREQU16000:
        {
            I2S_UpdateBClk(I2S_USR, 0x271, 0x10);
            break;
        }
    case AUDIO_FREQU32000:
        {
            I2S_UpdateBClk(I2S_USR, 0x271, 0x20);
            break;
        }
    case AUDIO_FREQU44100:
        {
            /* BCLK: 2.8224MHz, LR: 44.1K */
            I2S_UpdateBClk(I2S_USR, 6250, 441);
            break;
        }
    case AUDIO_FREQU48000:
        {
            I2S_UpdateBClk(I2S_USR, 0x271, 0x30);
            break;
        }
    default:
        {
            /* BCLK: 2.8224MHz, LR: 44.1K */
            I2S_UpdateBClk(I2S_USR, 6250, 441);
            break;
        }
    }
}

/**
  * @brief  Initialize LLI structure for audio stream output control.
  * @param   GDMA_InitStruct: pointer to a GDMA_InitTypeDef structure that
  *         contains the configuration information for the specified DMA Channel.
  * @param   GDMA_LLIStruct: pointer to a GDMA_LLIDef structure that
  *         contains the configuration information for Multi block mode.
  * @return  void
  */
DATA_RAM_FUNCTION
static void AudioTrans_LLIStruct_Init(GDMA_InitTypeDef *pGDMA_InitStruct,
                                      GDMA_LLIDef *pGDMA_LLIStruct)
{
    uint8_t i = 0;

    for (i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            pGDMA_LLIStruct->SAR = audio_bi_buffer_get_buf0_addr();
            pGDMA_LLIStruct->LLP = (uint32_t)(pGDMA_LLIStruct + 1);
        }
        else
        {
            pGDMA_LLIStruct->SAR = audio_bi_buffer_get_buf1_addr();
            pGDMA_LLIStruct->LLP = (uint32_t)(pGDMA_LLIStruct - 1);
        }

        pGDMA_LLIStruct->DAR = pGDMA_InitStruct->GDMA_DestinationAddr;//(uint32_t)(&(I2S_USR->TX_DR));
        pGDMA_LLIStruct->CTL_LOW = BIT(0)
                                   | (pGDMA_InitStruct->GDMA_DestinationDataSize << 1)
                                   | (pGDMA_InitStruct->GDMA_SourceDataSize << 4)
                                   | (pGDMA_InitStruct->GDMA_DestinationInc << 7)
                                   | (pGDMA_InitStruct->GDMA_SourceInc << 9)
                                   | (pGDMA_InitStruct->GDMA_DestinationMsize << 11)
                                   | (pGDMA_InitStruct->GDMA_SourceMsize << 14)
                                   | (pGDMA_InitStruct->GDMA_DIR << 20)
                                   | (pGDMA_InitStruct->GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
        //pGDMA_LLIStruct->CTL_HIGH = BI_BUFFER_MAX_SIZE/4;
        pGDMA_LLIStruct++;
    }
}

/**
  * @brief  Configure LLI structure for audio stream output control.
  * @param   GDMA_block_index: GDMA_LLIDef structure index.
  * @param   block_len: the specify block transmission size.
  * @param   is_last_block: the last block or not.
  * @return  void
  */
DATA_RAM_FUNCTION
void AudioTrans_ConfigLLIStruct(uint8_t GDMA_block_index, uint32_t block_len, bool is_last_block)
{
    GDMA_LLIDef *pGDMA_LLIStruct;
    pGDMA_LLIStruct = GDMA_block_index ? &AudioTrans_LLIStruct[1] : &AudioTrans_LLIStruct[0];

    pGDMA_LLIStruct->CTL_HIGH = block_len;

    if (is_last_block)
    {
        pGDMA_LLIStruct->CTL_LOW &= ~(LLP_SELECTED_BIT);
        pGDMA_LLIStruct->LLP = 0;
    }
}

static void AudioTrans_Init_GDMA_MemToI2S()
{
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_Cmd(AudioTrans_GDMA_Channel_NUM, DISABLE);
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = AudioTrans_GDMA_Channel_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_SourceAddr          = 0;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(I2S_USR->TX_DR));
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_I2S_USR_TX;
    GDMA_InitStruct.GDMA_Multi_Block_Mode    = LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_En      = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Struct  = (uint32_t)AudioTrans_LLIStruct;
    GDMA_Init(AudioTrans_GDMA_Channel, &GDMA_InitStruct);
    AudioTrans_LLIStruct_Init(&GDMA_InitStruct, AudioTrans_LLIStruct);
    GDMA_INTConfig(AudioTrans_GDMA_Channel_NUM, GDMA_INT_Block, ENABLE);
}

#ifdef USE_MEM_TO_UART
/**
  * @brief  Initialize UART peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_AudioTrans_Init_UART(void)
{
    APP_PRINT_INFO0("[Driver_AudioTrans_Init_UART] init");

    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, DISABLE);
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
    /* Initialize UART */
    UART_InitTypeDef uartInitStruct;
    UART_StructInit(&uartInitStruct);
    uartInitStruct.rxTriggerLevel = UART_RX_FIFO_TRIGGER_LEVEL_14BYTE;
    uartInitStruct.dmaEn = UART_DMA_ENABLE;
    uartInitStruct.TxDmaEn = UART_DMA_ENABLE;
//    /* Baudrate = 9600 */
//    uartInitStruct.div = 271;
//    uartInitStruct.ovsr = 10;
//    uartInitStruct.ovsr_adj = 0x24A;
//    /* 3M */
//    uartInitStruct.div = 1;
//    uartInitStruct.ovsr = 8;
//    uartInitStruct.ovsr_adj = 0x492;
    /* set baudrate to 0.5M */
    uartInitStruct.div = 8;
    uartInitStruct.ovsr = 5;
    uartInitStruct.ovsr_adj = 0;

//    /* set baudrate to 2M */
//    uartInitStruct.div = 2;
//    uartInitStruct.ovsr = 5;
//    uartInitStruct.ovsr_adj = 0;
//    uartInitStruct.parity = UART_PARITY_NO_PARTY;

    UART_Init(UART, &uartInitStruct);
}

/**
  * @brief  Initialize GDMA for audio transmission.
  * @param  addr: ram address.
  * @param  len: transmission size.
  * @return  void
  */
static void AudioTrans_Init_GDMA_MemToUART(uint32_t addr, uint32_t len)
{
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = AudioTrans_GDMA_Channel_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = len;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_SourceAddr          = addr;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_UART0_TX;
    GDMA_InitStruct.GDMA_Multi_Block_Mode    = LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_En      = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Struct  = (uint32_t)AudioTrans_LLIStruct;
    GDMA_Init(AudioTrans_GDMA_Channel, &GDMA_InitStruct);
    AudioTrans_LLIStruct_Init(&GDMA_InitStruct, AudioTrans_LLIStruct);
    GDMA_INTConfig(AudioTrans_GDMA_Channel_NUM, GDMA_INT_Block, ENABLE);
}
#endif



/**
  * @brief  Initialize audio transmission layer.
 * @param   pAudioTrans_InitStruct: pointer to a AudioTrans_InitTypeDef structure that
  *         contains the configuration information for audio transmission.
  * @return  void
  */
DATA_RAM_FUNCTION
void AudioTrans_GDMA_Ctrl(AudioTrans_InitTypeDef *pAudioTrans_InitStruct)
{

    switch (pAudioTrans_InitStruct->AudioTrans_Type)
    {
    case AUDIO_TRANS_TYPE_MEM_TO_I2S:
        {
            AudioTrans_Init_GDMA_MemToI2S();
            GDMA_Cmd(AudioTrans_GDMA_Channel_NUM, ENABLE);
            break;
        }
#ifdef USE_MEM_TO_UART
    case AUDIO_TRANS_TYPE_MEM_TO_UART:
        {
            Driver_AudioTrans_Init_UART();
            AudioTrans_Init_GDMA_MemToUART(pAudioTrans_InitStruct->mem_addr,
                                           pAudioTrans_InitStruct->audio_size);
            GDMA_Cmd(AudioTrans_GDMA_Channel_NUM, ENABLE);
            break;
        }
#endif
    default:
        {
            break;
        }
    }

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = AudioTrans_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Enables or disables the Audio transmission.
  * @param   pAudioTrans_InitStruct: pointer to a AudioTrans_InitTypeDef structure that
  *         contains the configuration information for audio transmission.
  * @param  NewState: new state of the audio transmission.
  *   This parameter can be: ENABLE or DISABLE.
  * @return  void
  */
DATA_RAM_FUNCTION
void AudioTrans_Cmd(AudioTrans_InitTypeDef *pAudioTrans_InitStruct, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        AudioTrans_GDMA_Ctrl(pAudioTrans_InitStruct);
    }
    else
    {
        if (GDMA_GetChannelStatus(AudioTrans_GDMA_Channel_NUM))
        {
            GDMA_SuspendCmd(AudioTrans_GDMA_Channel, ENABLE);

            uint32_t time_out = 0x1f;
            while ((RESET == GDMA_GetSuspendChannelStatus(AudioTrans_GDMA_Channel)) && time_out)
            {
                time_out--;
            }

            time_out = 0x0f;
            while ((RESET == GDMA_GetSuspendCmdStatus(AudioTrans_GDMA_Channel)) && time_out)
            {
                time_out--;
            }

            GDMA_Cmd(AudioTrans_GDMA_Channel_NUM, DISABLE);
            GDMA_SuspendCmd(AudioTrans_GDMA_Channel, DISABLE);
        }
    }
}

DATA_RAM_FUNCTION
void AudioTrans_GDMA_Handler(void)
{
    uint8_t block_index = audio_bi_buffer_get_read_index();
    uint8_t next_block_index = ((block_index == 0) ? 1 : 0);

    GDMA_ClearINTPendingBit(AudioTrans_GDMA_Channel_NUM, GDMA_INT_Block);

//  AUDIO_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[AudioTrans_GDMA_Handler] block_index is %d", 1, block_index);

    audio_bi_buffer_set_read_index(next_block_index);

    /* send messge to app */
    T_IO_MSG audio_io_msg;
    audio_io_msg.type = IO_MSG_TYPE_AUDIO;
    audio_io_msg.subtype = IO_MSG_AUDIO_PROCESS_DONE;
    audio_io_msg.u.param   = block_index;
    if (false == app_send_msg_to_apptask(&audio_io_msg))
    {
        APP_PRINT_WARN0("[AudioTrans_GDMA_Handler] Send IO_MSG_AUDIO_PROCESS_DONE failed!");
    }
}

#endif

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

