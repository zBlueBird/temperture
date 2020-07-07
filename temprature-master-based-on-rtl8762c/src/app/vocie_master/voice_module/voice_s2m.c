#include <trace.h>
#include <string.h>
#include "voice_s2m.h"
#include "os_mem.h"

/*============================================================================*
 *                              Micro
 *============================================================================*/
#define VOICE_KEY_HANDLE      0x2F//0x16
#define VOICE_CMD_HANDLE      0x3F//0x25
#define VOICE_DATA_HANDLE     0x33//0x21

#define TEMP_DATA_HANDLE      0x27

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
//static uint8_t voice_queue_buffer[VOICE_REPORT_FRAME_SIZE * VOICE_QUEUE_MAX_LENGTH] = {0};
uint8_t  filter_device_mac[6] = {0x78, 0x56, 0x34, 0x12, 0xE0, 0x99};
voice_event_data_offset g_voice_data_offset = {0};
uint16_t g_voice_key_handle = VOICE_KEY_HANDLE;
uint16_t g_voice_cmd_handle = VOICE_CMD_HANDLE;
uint16_t g_voice_data_handle = VOICE_DATA_HANDLE;
uint16_t g_temp_data_handle = TEMP_DATA_HANDLE;

uint8_t g_Voice_flow_stage = 0;

voice_event_node *p_Voice_List_Head = NULL;
voice_event_node *p_Voice_List_Tail = NULL;
voice_event_node *p_Voice_List_Current = NULL;
/*============================================================================*
 *                              Local Functions
 *============================================================================*/
void voice_add_node(voice_event_node *pNode);
void voice_node_print(voice_event_node *pNode);

/**
*@brief check filter device mac
*/
bool voice_filter_device_mac(uint8_t *des_mac)
{
    if (des_mac == NULL)
    {
        return false;
    }

    if (!memcmp(filter_device_mac, des_mac, 6))
    {
        return true;
    }

    return false;
}

uint16_t voice_get_handle(T_VOICE_Handle_Index index)
{
    uint16_t handle = NULL;
    switch (index)
    {
    case VOICE_KEY_HANDLE_INDEX:
        handle = g_voice_key_handle;
        break;
    case VOICE_CMD_HANDLE_INDEX:
        handle = g_voice_cmd_handle;
        break;
    case VOICE_DATA_HANDLE_INDEX:
        handle = g_voice_data_handle;
        break;
    case TEMP_DATA_HANDLE_INDEX:
        handle = g_temp_data_handle;
    default:
        break;
    }

    return handle;
}

bool voice_filter_mac_config(uint8_t *buf)
{
    uint8_t *p;

    if (buf == NULL)
    {
        return false;
    }

    p = buf;

    if (*p != 0x87)
    {
        return false;
    }
    p ++;
    if ((*p != 0x06) || (*(p + 1) != 0x11))
    {
        return false;
    }

    p += 2;

    memcpy(filter_device_mac, p, 6);

    PROFILE_PRINT_INFO1("[voice] voice_filter_mac_config, mac = %b.", TRACE_BINARY(6,
                                                                                   filter_device_mac));

    return true;
}


