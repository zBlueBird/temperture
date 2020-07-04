/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     mp_config.c
* @brief    .
* @details
* @author   ranhui_xia
* @date     2015-11-15
* @version  v1.0
*********************************************************************************************************
*/

#include "rtl876x.h"
#include "trace.h"
#include "gap.h"
#include "board.h"
#include "mp_config.h"
#if MP_TEST_SUPPORT
#include "test_mode.h"
#endif
#include "app_section.h"
#include "gap_storage_le.h"
#include "mem_types.h"
#include "ftl.h"
#include "os_mem.h"
#include "gap_bond_le.h"

#if MP_TEST_SUPPORT

// the test mode flag of user image
//[Lory] the flag will be written through MP Tool
const uint32_t test_mode_flag USER_FLASH_TEST_MODE_FLAG = 0xFFFFFFFF; //TEST_MODE_FLAG_DEFAULT;

void mp_set_local_bt_addr(uint8_t index)
{
    uint8_t mp_local_bd[8] = {0};

    switch (index)
    {
    case MP_TEST_LINE_1:
        {
            //00:e0:4c:23:88:01
            mp_local_bd[5] = 0x00;
            mp_local_bd[4] = 0xe0;
            mp_local_bd[3] = 0x4c;
            mp_local_bd[2] = 0x23;
            mp_local_bd[1] = 0x88;
            mp_local_bd[0] = 0x01;
        }
        break;

    case MP_TEST_LINE_2:
        {
            //00:e0:4c:23:88:02
            mp_local_bd[5] = 0x00;
            mp_local_bd[4] = 0xe0;
            mp_local_bd[3] = 0x4c;
            mp_local_bd[2] = 0x23;
            mp_local_bd[1] = 0x88;
            mp_local_bd[0] = 0x02;
        }
        break;
    case MP_TEST_LINE_3:
        {
            //00:e0:4c:23:88:03
            mp_local_bd[5] = 0x00;
            mp_local_bd[4] = 0xe0;
            mp_local_bd[3] = 0x4c;
            mp_local_bd[2] = 0x23;
            mp_local_bd[1] = 0x88;
            mp_local_bd[0] = 0x03;
        }
        break;
    case MP_TEST_LINE_4:
        {
            //00:e0:4c:23:88:04
            mp_local_bd[5] = 0x00;
            mp_local_bd[4] = 0xe0;
            mp_local_bd[3] = 0x4c;
            mp_local_bd[2] = 0x23;
            mp_local_bd[1] = 0x88;
            mp_local_bd[0] = 0x04;
        }
        break;
    case MP_TEST_LINE_5:
        {
            //00:e0:4c:23:88:05
            mp_local_bd[5] = 0x00;
            mp_local_bd[4] = 0xe0;
            mp_local_bd[3] = 0x4c;
            mp_local_bd[2] = 0x23;
            mp_local_bd[1] = 0x88;
            mp_local_bd[0] = 0x05;
        }
        break;
    default:
        break;
    }

    ftl_save(mp_local_bd, MP_FLASH_PARAMS_LOCAL_BD_ADDR_OFFSET, sizeof(mp_local_bd));


}


void mp_set_auto_pair_info(uint8_t index)
{
    uint8_t ccc_bits_count = 16;
    T_LE_CCCD *p_cccd_data;
    T_LE_KEY_TYPE key_type = LE_KEY_UNAUTHEN;

    T_GAP_REMOTE_ADDR_TYPE remote_addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t bd_addr[6] = {0};/*remote bd address*/
    uint8_t ltk_length = 16;/*need to get real num*/
    uint8_t local_ltk[16];
    switch (index)
    {
    case MP_TEST_LINE_1:
        {
            //00:e0:c4:23:99:87
            bd_addr[5] = 0x00;
            bd_addr[4] = 0xe0;
            bd_addr[3] = 0x4c;
            bd_addr[2] = 0x23;
            bd_addr[1] = 0x99;
            bd_addr[0] = 0x87;
            remote_addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;

            /*link key config*/
            key_type = LE_KEY_UNAUTHEN;
            ltk_length = 16;
            local_ltk[0]  = 0xcb;
            local_ltk[1]  = 0x84;
            local_ltk[2]  = 0xaa;
            local_ltk[3]  = 0x4d;
            local_ltk[4]  = 0x66;
            local_ltk[5]  = 0x42;
            local_ltk[6]  = 0xd5;
            local_ltk[7]  = 0xa2;
            local_ltk[8]  = 0x33;
            local_ltk[9]  = 0xa1;
            local_ltk[10] = 0x6a;
            local_ltk[11] = 0x51;
            local_ltk[12] = 0x9a;
            local_ltk[13] = 0x50;
            local_ltk[14] = 0xb5;
            local_ltk[15] = 0xac;
        }
        break;
    case MP_TEST_LINE_2:
        {
            //00:e0:c4:23:99:87
            bd_addr[5] = 0x00;
            bd_addr[4] = 0xe0;
            bd_addr[3] = 0x4c;
            bd_addr[2] = 0x23;
            bd_addr[1] = 0x99;
            bd_addr[0] = 0x87;
            remote_addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;

            /*link key config*/
            key_type = LE_KEY_UNAUTHEN;
            ltk_length = 16;
            local_ltk[0]  = 0xcb;
            local_ltk[1]  = 0x84;
            local_ltk[2]  = 0xaa;
            local_ltk[3]  = 0x4d;
            local_ltk[4]  = 0x66;
            local_ltk[5]  = 0x42;
            local_ltk[6]  = 0xd5;
            local_ltk[7]  = 0xa2;
            local_ltk[8]  = 0x33;
            local_ltk[9]  = 0xa1;
            local_ltk[10] = 0x6a;
            local_ltk[11] = 0x51;
            local_ltk[12] = 0x9a;
            local_ltk[13] = 0x50;
            local_ltk[14] = 0xb5;
            local_ltk[15] = 0xac;
        }
        break;
    case MP_TEST_LINE_3:
        {
            //00:e0:c4:23:99:87
            bd_addr[5] = 0x00;
            bd_addr[4] = 0xe0;
            bd_addr[3] = 0x4c;
            bd_addr[2] = 0x23;
            bd_addr[1] = 0x99;
            bd_addr[0] = 0x87;
            remote_addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;


            /*link key config*/
            key_type = LE_KEY_UNAUTHEN;
            ltk_length = 16;
            local_ltk[0]  = 0xcb;
            local_ltk[1]  = 0x84;
            local_ltk[2]  = 0xaa;
            local_ltk[3]  = 0x4d;
            local_ltk[4]  = 0x66;
            local_ltk[5]  = 0x42;
            local_ltk[6]  = 0xd5;
            local_ltk[7]  = 0xa2;
            local_ltk[8]  = 0x33;
            local_ltk[9]  = 0xa1;
            local_ltk[10] = 0x6a;
            local_ltk[11] = 0x51;
            local_ltk[12] = 0x9a;
            local_ltk[13] = 0x50;
            local_ltk[14] = 0xb5;
            local_ltk[15] = 0xac;
        }
        break;
    case MP_TEST_LINE_4:
        {
            //00:e0:c4:23:99:87
            bd_addr[5] = 0x00;
            bd_addr[4] = 0xe0;
            bd_addr[3] = 0x4c;
            bd_addr[2] = 0x23;
            bd_addr[1] = 0x99;
            bd_addr[0] = 0x87;
            remote_addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;

            /*link key config*/
            key_type = LE_KEY_UNAUTHEN;
            ltk_length = 16;
            local_ltk[0]  = 0xcb;
            local_ltk[1]  = 0x84;
            local_ltk[2]  = 0xaa;
            local_ltk[3]  = 0x4d;
            local_ltk[4]  = 0x66;
            local_ltk[5]  = 0x42;
            local_ltk[6]  = 0xd5;
            local_ltk[7]  = 0xa2;
            local_ltk[8]  = 0x33;
            local_ltk[9]  = 0xa1;
            local_ltk[10] = 0x6a;
            local_ltk[11] = 0x51;
            local_ltk[12] = 0x9a;
            local_ltk[13] = 0x50;
            local_ltk[14] = 0xb5;
            local_ltk[15] = 0xac;
        }
        break;
    case MP_TEST_LINE_5:
        {
            //00:e0:c4:23:99:87
            bd_addr[5] = 0x00;
            bd_addr[4] = 0xe0;
            bd_addr[3] = 0x4c;
            bd_addr[2] = 0x23;
            bd_addr[1] = 0x99;
            bd_addr[0] = 0x87;
            remote_addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;

            /*link key config*/
            key_type = LE_KEY_UNAUTHEN;
            ltk_length = 16;
            local_ltk[0]  = 0xcb;
            local_ltk[1]  = 0x84;
            local_ltk[2]  = 0xaa;
            local_ltk[3]  = 0x4d;
            local_ltk[4]  = 0x66;
            local_ltk[5]  = 0x42;
            local_ltk[6]  = 0xd5;
            local_ltk[7]  = 0xa2;
            local_ltk[8]  = 0x33;
            local_ltk[9]  = 0xa1;
            local_ltk[10] = 0x6a;
            local_ltk[11] = 0x51;
            local_ltk[12] = 0x9a;
            local_ltk[13] = 0x50;
            local_ltk[14] = 0xb5;
            local_ltk[15] = 0xac;
        }
        break;
    default:
        break;
    }
    p_cccd_data = os_mem_alloc(RAM_TYPE_DATA_ON, 4 + ccc_bits_count * 4);
    p_cccd_data->data_length = 0x14;
    p_cccd_data->data[0]  = 0x4F;
    p_cccd_data->data[1]  = 0x00;
    p_cccd_data->data[2]  = 0x01;
    p_cccd_data->data[3]  = 0x00;
    p_cccd_data->data[4]  = 0x33;
    p_cccd_data->data[5]  = 0x00;
    p_cccd_data->data[6]  = 0x01;
    p_cccd_data->data[7]  = 0x00;
    p_cccd_data->data[8]  = 0x3C;
    p_cccd_data->data[9]  = 0x00;
    p_cccd_data->data[10] = 0x01;
    p_cccd_data->data[11] = 0x00;
    p_cccd_data->data[12] = 0x43;
    p_cccd_data->data[13] = 0x00;
    p_cccd_data->data[14] = 0x01;
    p_cccd_data->data[15] = 0x00;
    p_cccd_data->data[16] = 0x47;
    p_cccd_data->data[17] = 0x00;
    p_cccd_data->data[18] = 0x01;
    p_cccd_data->data[19] = 0x00;

    /*set mp auto pair device info*/
    if (le_gen_bond_dev(bd_addr, remote_addr_type, GAP_LOCAL_ADDR_LE_PUBLIC,
                        ltk_length, local_ltk, key_type, p_cccd_data))
    {
        APP_PRINT_INFO0("[MP] Generate bond device success.");
    }
    else
    {
        APP_PRINT_INFO0("[MP] Err: Generate bond device failed!");
    }

    os_mem_free(p_cccd_data);
}

bool mp_pair_info_remove(void)
{
    bool bRet = true;

    le_bond_clear_all_keys();

    if (0 < le_get_bond_dev_num())
    {
        APP_PRINT_INFO0("[MP] Fail, pair information still exist.");
        bRet = false;
    }
    else
    {
        APP_PRINT_INFO0("[MP] Success, pair information removed.");
        TestModeFlagDisable();
        bRet = true;
    }

    return bRet;
}

#endif


