
#ifndef _LED_H_
#define _LED_H_

#include "stdint.h"
#include "board.h"

#define  LED_PERIOD 50  /*50ms*/

#define  LED_DEBUG  0

/*led types*/
typedef   uint32_t      LED_TYPE   ;
#define   LED_TYPE_IDLE                      0
#define   LED_TYPE_BLINK_LOW_POWER          (1)
#define   LED_TYPE_BLINK_DATA_REV           (2)
#define   LED_TYPE_BLINK_IR_LEARN_WAITING   (3)
#define   LED_TYPE_BLINK_IR_LEARN_MODE      (4)
#define   LED_TYPE_BLINK_PAIR_SCAN          (5)
#define   LED_TYPE_BLINK_PAIR_SUCCESS       (6)
#define   LED_TYPE_ON                       (7)
#define   LED_TYPE_MAX                       8

/*led bit map 32bits, High bits(low priority) ---  Low bits(high priority) */
#define LED_BIT_MAP_LOW_POWER           (0x00000001)
#define LED_BIT_MAP_DATA_REV            (0x00000003)
#define LED_BIT_MAP_IR_LEARN_WAITING    (0xffffffff)
#define LED_BIT_MAP_IR_LEARN_MODE       (0x00FF00FF)
#define LED_BIT_MAP_PAIR_SCAN           (0x00050005)
#define LED_BIT_MAP_PAIR_SUCCESS        (0x03030303)
#define LED_BIT_MAP_ON                  (0xffffffff)

/*struct support for led blink count*/
typedef struct
{
    uint8_t led_cnt;
    uint8_t cur_tick_cnt;
} led_cnt_stg;

/*support for each led data*/
typedef struct
{
    uint8_t led_index;
    uint8_t led_pin;
    led_cnt_stg led_cnt_arr[LED_TYPE_MAX];
    uint32_t led_map;
} led_data_stg;

/*led return code*/
typedef enum
{
    LED_SUCCESS                      = 0,
    LED_ERR_TYPE                     = 1,
    LED_ERR_INDEX                    = 2,
    LED_ERR_CODE_MAX
} LED_RET_CAUSE;

extern void led_init_timer(void);
extern LED_RET_CAUSE led_blink_start(uint16_t index, LED_TYPE type, uint8_t cnt);
extern LED_RET_CAUSE led_blink_exit(uint16_t index, LED_TYPE type);

#if LED_EN
#define LED_ON(index)                   led_blink_start(index, LED_TYPE_ON, 0)
#define LED_OFF(index)                  led_blink_exit(index, LED_TYPE_ON)
#define LED_BLINK(index, type, n)       led_blink_start(index, type, n)
#define LED_BLINK_EXIT(index, type)     led_blink_exit(index, type)
#else
#define LED_ON(index)
#define LED_OFF(index)
#define LED_BLINK(index, type, n)
#define LED_BLINK_EXIT(index, type)
#endif

#endif
