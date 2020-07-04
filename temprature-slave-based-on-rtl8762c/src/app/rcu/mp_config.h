/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     mp_config.h
* @brief    .
* @details
* @author   ranhui_xia
* @date     2015-11-15
* @version  v1.0
*********************************************************************************************************
*/

#ifndef _MP_CONFIG_H
#define _MP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define MP_TEST_LINE_1 1
#define MP_TEST_LINE_2 2
#define MP_TEST_LINE_3 3
#define MP_TEST_LINE_4 4
#define MP_TEST_LINE_5 5

#define MP_FLASH_PARAMS_OFFSET                0
#define MP_FLASH_PARAMS_LOCAL_BD_ADDR_OFFSET         (MP_FLASH_PARAMS_OFFSET)

void mp_set_local_bt_addr(uint8_t index);
void mp_set_auto_pair_info(uint8_t index);
bool mp_pair_info_remove(void);




#ifdef __cplusplus
}
#endif

#endif /* _PROFILE_INIT_H */
