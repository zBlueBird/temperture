/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      audio_hd_detection.h
* @brief    header file of audio headphone detection module.
* @details
* @author    Chenjie Jin
* @date      2018-10-28
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __AUDIO_HD_DECT_H
#define __AUDIO_HD_DECT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "trace.h"
#include "board.h"
#include "app_section.h"
#include "rtl876x.h"

/* Defines ------------------------------------------------------------------*/

/**
 * @brief Enable print log or not
 */
#if AUDIO_SUPPORT_HEADPHONE_DETECT

#define PRINT_HP_DETC_BUFFER_LOG
#ifdef PRINT_HP_DETC_BUFFER_LOG
#define HP_DETC_BUFFER(MODULE, LEVEL, pFormat, para_num,...) DBG_BUFFER_##LEVEL(MODULE, pFormat, para_num, ##__VA_ARGS__)
#else
#define HP_DETC_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

/*============================================================================*
 *                         Types
 *============================================================================*/
/**
 * @brief  Audio headphone status enum.
 */
typedef enum _AUDIO_HD_STATUS
{
    AUDIO_HD_STATUS_DETACHED = 0,
    AUDIO_HD_STATUS_ATTACHED = 1,
} AUDIO_HD_STATUS;

extern AUDIO_HD_STATUS audio_hd_cur_state;

__STATIC_INLINE void audio_hd_set_status_to_attached(void)
{
    audio_hd_cur_state = AUDIO_HD_STATUS_ATTACHED;
}

__STATIC_INLINE void audio_hd_set_status_to_detached(void)
{
    audio_hd_cur_state = AUDIO_HD_STATUS_DETACHED;
}

__STATIC_INLINE AUDIO_HD_STATUS audio_hd_get_status(void)
{
    return audio_hd_cur_state;
}

extern void audio_hd_driver_init(void);
extern void audio_hd_nvic_config(void);
extern void audio_hd_enter_dlps_config(void);
extern void audio_hd_exit_dlps_config(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /*__AUDIO_HD_DECT_H */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

