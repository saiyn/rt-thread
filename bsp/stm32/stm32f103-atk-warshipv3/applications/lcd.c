#include <rtthread.h>
#include <rtdevice.h>

#include "lcd.h"

#define LCD_DRAW_FAST_POINT (0x16)

typedef struct{
	rt_uint16_t x0,y0,x1,y1,color;
}lcd_arg_t;


static rt_device_t _lcd;



void LCD_init(void)
{
	_lcd = rt_device_find("display");
	if(!_lcd)
	{
		rt_kprintf("find display failed\n");
		return;
	}

	if(rt_device_init(_lcd) != RT_EOK)
	{
		rt_kprintf("init display failed\n");
		return;
	}


	if(rt_device_open(_lcd, 0) != RT_EOK)
	{
		rt_kprintf("open display failed\n");
		return;
	}

}

void LCD_fast_drawPoint(rt_uint16_t x, rt_uint16_t y, rt_uint16_t color)
{
	lcd_arg_t point = {x,y,x,y,color};


	rt_device_control(_lcd, LCD_DRAW_FAST_POINT, &point);
}



