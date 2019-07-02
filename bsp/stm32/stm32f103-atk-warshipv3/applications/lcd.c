#include <rtthread.h>
#include <rtdevice.h>

#include "lcd.h"
#include "font.h"


#define LCD_DRAW_RECTANGLE	(0x15)
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



void LCD_draw_rectangle(rt_uint16_t x0, rt_uint16_t y0, rt_uint16_t x1, rt_uint16_t y1, rt_uint16_t color,int bord)
{

	lcd_arg_t rect = {x0, y0, x1, y1, color};

	rt_device_control(_lcd, LCD_DRAW_RECTANGLE, &rect);

}

#define MENU_BAR_H		(80)
#define LEFT_RIGHT_ALIAN	(50)
#define CENTER_ALIAN		(50)
#define MAIN_BOX_H		(150)
#define MAIN_BOX_W		(200)
#define MAIN_BOX_CMNT_H		(50)



void LCD_show_ui(void)
{
	int i;
	rt_uint16_t x0,x1,y0,y1;

	for(i = 0; i < 3; i++)
	{
		x0 = (LEFT_RIGHT_ALIAN + i*MAIN_BOX_W + i*CENTER_ALIAN);
		y0 = MENU_BAR_H;

		LCD_draw_rectangle(x0, y0, x0+MAIN_BOX_W, y0+MAIN_BOX_H, 0x0000, 0);


    		Font_show_str(x0 + (MAIN_BOX_W / 2) - 10, y0 + MAIN_BOX_H + 10, 200, 24, "ÓÍÎÂ",24, 0);  

		LCD_draw_rectangle(x0, y0 + MAIN_BOX_CMNT_H + MAIN_BOX_H, x0 + MAIN_BOX_W, y0 + MAIN_BOX_H*2 + MAIN_BOX_CMNT_H, 0xf5f5, 0);

    		Font_show_str(x0 + (MAIN_BOX_W / 2) - 10, y0 + MAIN_BOX_CMNT_H + MAIN_BOX_H*2 + 1 , 200, 24, "×ªËÙ",24, 0);  


	}


	
}



















