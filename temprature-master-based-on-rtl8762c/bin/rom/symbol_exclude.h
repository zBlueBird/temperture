#ifndef _SYMBOL_EXCLUDE_H_
#define _SYMBOL_EXCLUDE_H_

/* GDMA */
void GDMA_DeInit(void);
void GDMA_Init(GDMA_ChannelTypeDef *GDMA_Channelx, GDMA_InitTypeDef *GDMA_InitStruct);
void GDMA_StructInit(GDMA_InitTypeDef *GDMA_InitStruct);
void GDMA_Cmd(uint8_t GDMA_ChannelNum, FunctionalState NewState);
void GDMA_INTConfig(uint8_t GDMA_ChannelNum, uint32_t GDMA_IT, FunctionalState NewState);
void GDMA_ClearINTPendingBit(uint8_t GDMA_ChannelNum, uint32_t GDMA_IT);

/* UART */
void UART_Init(UART_TypeDef *UARTx, UART_InitTypeDef *UART_InitStruct);
void UART_DeInit(UART_TypeDef *UARTx);
void UART_StructInit(UART_InitTypeDef *UART_InitStruct);
void UART_ReceiveData(UART_TypeDef *UARTx, uint8_t *outBuf, uint16_t count);
void UART_SendData(UART_TypeDef *UARTx, const uint8_t *inBuf, uint16_t count);
void UART_INTConfig(UART_TypeDef *UARTx, uint32_t UART_IT, FunctionalState newState);
FlagStatus UART_GetFlagState(UART_TypeDef *UARTx, uint32_t UART_FLAG);
void UART_LoopBackCmd(UART_TypeDef *UARTx, FunctionalState NewState);

/* GPIO */
void GPIO_DeInit(void);
void GPIO_Init(GPIO_InitTypeDef *GPIO_InitStruct);
void GPIO_StructInit(GPIO_InitTypeDef *GPIO_InitStruct);
void GPIO_INTConfig(uint32_t GPIO_Pin, FunctionalState NewState);
void GPIO_ClearINTPendingBit(uint32_t GPIO_Pin);
void GPIO_MaskINTConfig(uint32_t GPIO_Pin, FunctionalState NewState);
uint32_t GPIO_GetPin(uint8_t Pin_num);
uint8_t GPIO_GetNum(uint8_t Pin_num);
void GPIO_DBClkCmd(FunctionalState NewState);

/* RCC */
void RCC_PeriphClockCmd(uint32_t APBPeriph, uint32_t APBPeriph_Clock, FunctionalState NewState);
void RCC_SPIClkDivConfig(SPI_TypeDef *SPIx, uint16_t ClockDiv);
void RCC_I2CClkDivConfig(I2C_TypeDef *I2Cx, uint16_t ClockDiv);
void RCC_UARTClkDivConfig(UART_TypeDef *UARTx, uint16_t ClockDiv);
void RCC_PeriClockConfig(uint32_t APBPeriph_Clock, FunctionalState NewState);
void RCC_PeriFunctionConfig(uint32_t APBPeriph, FunctionalState NewState);

/* I2C */
void I2C_DeInit(I2C_TypeDef *I2Cx);
void I2C_Init(I2C_TypeDef *I2Cx, I2C_InitTypeDef *I2C_InitStruct);
void I2C_StructInit(I2C_InitTypeDef *I2C_InitStruct);
void I2C_Cmd(I2C_TypeDef *I2Cx, FunctionalState NewState);
void I2C_MasterWrite(I2C_TypeDef *I2Cx, uint8_t *pBuf, uint16_t len);
void I2C_MasterRead(I2C_TypeDef *I2Cx, uint8_t *pBuf, uint16_t len);
uint16_t I2C_RepeatRead(I2C_TypeDef *I2Cx, uint8_t *pWriteBuf, uint16_t Writelen,
                        uint8_t *pReadBuf,
                        uint16_t Readlen);
void I2C_INTConfig(I2C_TypeDef *I2Cx, uint16_t I2C_INT, FunctionalState NewState);
void I2C_ClearINTPendingBit(I2C_TypeDef *I2Cx, uint16_t I2C_IT);

/* KeyScan */
void KeyScan_Init(KEYSCAN_TypeDef *KeyScan, KEYSCAN_InitTypeDef *KeyScan_InitStruct);
void KeyScan_DeInit(KEYSCAN_TypeDef *KeyScan);
void KeyScan_StructInit(KEYSCAN_InitTypeDef *KeyScan_InitStruct);
void KeyScan_INTConfig(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_IT, FunctionalState newState);
void KeyScan_INTMask(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_IT, FunctionalState newState);
void KeyScan_Read(KEYSCAN_TypeDef *KeyScan, uint16_t *outBuf, uint16_t count);
void KeyScan_Cmd(KEYSCAN_TypeDef *KeyScan, FunctionalState NewState);
void KeyScan_FilterDataConfig(KEYSCAN_TypeDef *KeyScan, uint16_t data, FunctionalState NewState);

/* Timer */
void TIM_DeInit();
void TIM_TimeBaseInit(TIM_TypeDef *TIMx, TIM_TimeBaseInitTypeDef *TIM_TimeBaseInitStruct);
void TIM_StructInit(TIM_TimeBaseInitTypeDef *TIM_TimeBaseInitStruct);
void TIM_Cmd(TIM_TypeDef *TIMx, FunctionalState NewState);
void TIM_ChangePeriod(TIM_TypeDef *TIMx, uint32_t period);
void TIM_INTConfig(TIM_TypeDef *TIMx, FunctionalState NewState);
void TIM_PWMChangeFreqAndDuty(TIM_TypeDef *TIMx, uint32_t high_count, uint32_t low_count);

/* WDG */
void WDG_SystemReset(WDG_MODE wdg_mode);

/* LPC */
void LPC_Init(LPC_InitTypeDef *LPC_InitStruct);
void LPC_StructInit(LPC_InitTypeDef *LPC_InitStruct);
void LPC_Cmd(FunctionalState NewState);
void LPC_CounterCmd(FunctionalState NewState);
void LPC_CounterReset(void);
void LPC_WriteComparator(uint32_t data);
uint16_t LPC_ReadComparator(void);
uint16_t LPC_ReadCounter(void);
void LPC_INTConfig(uint32_t LPC_INT, FunctionalState NewState);
void LPC_ClearINTPendingBit(uint32_t LPC_INT);
ITStatus LPC_GetINTStatus(uint32_t LPC_INT);

/*GAP*/
bool le_check_privacy_bond(T_LE_KEY_ENTRY *p_entry);
T_GAP_CAUSE le_bond_just_work_confirm(uint8_t conn_id, T_GAP_CFM_CAUSE cause);
T_GAP_CAUSE le_gen_rand_addr(T_GAP_RAND_ADDR_TYPE rand_addr_type, uint8_t *random_bd);
#endif
