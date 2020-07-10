/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      uart_transport.h
* @brief
* @details
* @author    chenjie jin
* @date      2018-04-08
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __UART_TRANSPORT_H
#define __UART_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdint.h>
#include <stdbool.h>
#include "trace.h"

/* Defines ------------------------------------------------------------------*/

/* Enable print log or not */
#define UART_PRINT_LOG

/* Configure loop queue parameters */
#define UART_LOOP_QUEUE_MAX_SIZE             (512 * 2)
#define UART_QUEUE_CAPABILITY                (UART_LOOP_QUEUE_MAX_SIZE-1)
/* Configure UART packet buffer length */
#define CMD_SIZE                        50

#define  ESP_GET_MODE                             0x00
#define  ESP_SCAN_WIFI                            0x01
#define  ESP_CONNECT_AP                           0x02
#define  ESP_SET_LINK_MUX                         0x03
#define  ESP_CONFIG_SERER_INDEX                   0x04
#define  ESP_CHECK_IP_ADDR_INDEX                  0x05
#define  ESP_GOT_IP_INDEX                         0x09
#define  ESP_WIFI_CONNECTED_INDEX                 0x0A
#define  ESP_WIFI_DISCONNECT_INDEX                0x0B
#define  ESP_WIFI_STATUS_CONNECT_INDEX            0x0C
#define  ESP_WIFI_STATUS_DISCONNECT_INDEX         0x0D

#ifdef UART_PRINT_LOG
#define UART_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) DBG_BUFFER_##LEVEL(TYPE_BEE2, SUBTYPE_FORMAT, MODULE, fmt, para_num, ##__VA_ARGS__)
#else
#define UART_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) ((void)0)
#endif


/* Loop queue data struct */
typedef struct
{
    volatile uint16_t   ReadIndex;          /* index of read queue */
    volatile uint16_t   WriteIndex;         /* index of write queue */
    volatile bool       OverFlow;           /* loop queue overflow or not */
    uint8_t buf[UART_LOOP_QUEUE_MAX_SIZE];       /* Buffer for loop queue */
} UartLoopQueue_TypeDef;

/* Packet decode status */
typedef enum
{
    WaitHeader0,
    WaitHeader1,
    WaitHeader2,
    WaitHeader,

    WaitCMD,
    WaitParams,
    WaitCRC,
} WaitState;

typedef struct
{
    uint8_t send_cmd[50];
    uint8_t ret_str[50];
} stg_AT_Cmd;

/* UART packet data structure */
typedef struct _UART_PacketTypeDef
{
    uint8_t     Buf[CMD_SIZE];  /* command buffer */
    uint16_t    BufIndex;       /* index of buffer */
    uint16_t    PayloadLen;     /* length of decoder payload */
    uint16_t    CRCLen;         /* index of CRC */
    WaitState   Status;         /* status of decoding */

} UART_PacketTypeDef;

void DataUARTInit(uint8_t buadrate_opt);
void UartTransport_Init(void);
bool Packet_Decode(UART_PacketTypeDef *pPacket);
void UARTCmd_Response(uint16_t opcode, uint8_t status, uint8_t *pPayload, uint32_t payload_len);

bool LoopQueueIsEmpty(UartLoopQueue_TypeDef *pQueueStruct);
bool LoopQueueIsFull(UartLoopQueue_TypeDef *pQueueStruct);
uint8_t LoopQueueFindString(UartLoopQueue_TypeDef *pQueueStruct, const char *sub);
uint8_t LoopQueueClear(UartLoopQueue_TypeDef *pQueueStruct);
uint8_t ESP8266_Cmd_Send(uint8_t *pCmdBuf);
bool LoopQueueOut(UartLoopQueue_TypeDef *pQueueStruct, uint8_t *pBuf, uint16_t size);
void LoopQueuePrint(UartLoopQueue_TypeDef *pQueueStruct);

#ifdef __cplusplus
}
#endif

#endif /*__UART_TRANSPORT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

