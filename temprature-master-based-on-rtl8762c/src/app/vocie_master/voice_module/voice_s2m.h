#ifndef _VOICE_H_
#define _VOICE_H_

#include <stdint.h>
#include <stdbool.h>

#define  VOICE_NODE_BUFF_LEN    0x28//40bytes

typedef enum
{
    VOICE_KEY_HANDLE_INDEX = 1,
    VOICE_CMD_HANDLE_INDEX = 2,
    VOICE_DATA_HANDLE_INDEX = 3,
    TEMP_DATA_HANDLE_INDEX = 4,
} T_VOICE_Handle_Index;

typedef enum
{
    VOICE_DIRECT_MASTER_TO_SLAVE = 0,
    VOICE_DIRECT_SLAVE_TO_MASTER = 1,
} T_VOICE_Direct;

typedef enum
{
    VOICE_TYPE_KEY_EVENT = 0,
    VOICE_TYPE_CMD_EVENT = 1,
    VOICE_TYPE_DATA_EVENT = 2,
} T_VOICE_Event_Type;

#pragma pack(1)
typedef struct node
{
    uint8_t event_node_index;
    uint8_t event_node_num;//the number of nodes in the list
    uint16_t event_handle;//the handle of node
    uint8_t event_type;
    uint8_t event_direct;
    uint8_t event_value_length;
    uint8_t p_event_value_buf[VOICE_NODE_BUFF_LEN];
    struct node *pNext;
} voice_event_node;

typedef struct
{
    uint8_t start_offset;
    uint8_t end_offset;
} voice_event_data_offset;
#pragma pack()

extern bool voice_filter_device_mac(uint8_t *des_mac);
extern uint16_t voice_get_handle(T_VOICE_Handle_Index index);
extern bool voice_filter_mac_config(uint8_t *buf);
extern bool voice_node_config(uint8_t *pbuf);

#endif
