/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-08     obito0   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "w25Q.h"


#include "lcd.h"

/* defined the LED0 pin: PB5 */
#define LED0_PIN    GET_PIN(B, 5)
/* defined the LED0 pin: PE5 */
#define LED1_PIN    GET_PIN(E, 5)


static rt_device_t lcd;

typedef struct {
	unsigned short x0,y0,x1,y1,color;
}lcd_arg_t;


extern int Font_init(void);
extern int Sdcard_mount(void);

extern void SIM_Init();


int main(void)
{
    int count = 1;
    rt_err_t res = RT_EOK;
	
   

    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);


     SIM_Init();

    
    // Sdcard_mount();

     w25q_init();


     Font_init();

     //LCD_init();


     //LCD_show_ui();
	
    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(5000);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_thread_mdelay(5000);
    }

    return RT_EOK;
}
