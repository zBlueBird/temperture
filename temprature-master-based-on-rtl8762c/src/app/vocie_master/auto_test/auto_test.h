
#ifndef _AUTO_TEST_H_
#define _AUTO_TEST_H_

#define  AUTO_TEST_TIMER_TICK     4000 //4s

/*
 * 1 scan and connect
 * 2 auto voice
 * 3 direct adv pair and unpair
 * 4 power on and off
 */
#define  AUTO_TEST_MODE       1

#if (AUTO_TEST_MODE == 2)
#define AUTO_VOICE_ADDR    (0x1829000)
#endif

extern void auto_test_init(void);
extern unsigned int auto_test_check_voice_data_file(void);

#endif
