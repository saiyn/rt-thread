#include <rtthread.h>
#include <rtdevice.h>

#include "lcd.h"
//#include "font.h"


#define LCD_DRAW_RECTANGLE	(0x15)
#define LCD_DRAW_FAST_POINT (0x16)




extern void Font_show_str(rt_uint16_t x, rt_uint16_t y, rt_uint16_t w, rt_uint16_t h, rt_uint8_t *str, rt_uint8_t size, rt_uint8_t mode);


extern const unsigned char asc2_2412[95][36];	  


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



void LCD_show_char(rt_uint16_t x, rt_uint16_t y, rt_uint8_t c, rt_uint8_t size, rt_uint8_t mode)
{
	rt_uint8_t tmp, t, t1;
	rt_uint16_t y0 = y;
	rt_uint8_t csize = (size / 8 + ((size % 8) ? 1:0)) * (size / 2);

	c = c - ' ';

	for(t = 0; t < csize; t++)
	{
		if(size == 24)
		{
			tmp = asc2_2412[c][t];
		}
		else
		{

			rt_kprintf("invlaid size in lcd_show_char\n");
			return;
		}

		for(t1 = 0; t1 < 8; t1++)
		{
			if(tmp & 0x80)
				LCD_fast_drawPoint(x, y, POINT_COLOR);
			else if(mode == 0)
				LCD_fast_drawPoint(x, y, BACK_COLOR);

			tmp <<= 1;
			y++;

			if(y >= 480)
				return;

			if((y - y0) == size)
			{
				y = y0;
				x++;

				if(x >= 800)
					return;

				break;
			}
			
		}
	}

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
		LCD_draw_rectangle(x0+1, y0+1, x0+MAIN_BOX_W - 1, y0+MAIN_BOX_H - 1, 0x0000, 0);


    		Font_show_str(x0 + (MAIN_BOX_W / 2) - 20, y0 + MAIN_BOX_H + 10, 200, 24, "ÓÍÎÂ",24, 0);  


		Font_show_str(x0 + (MAIN_BOX_W / 2) - 25, y0 + MAIN_BOX_H / 2, 200, 24,"85.5", 24, 0);

		LCD_draw_rectangle(x0, y0 + MAIN_BOX_CMNT_H + MAIN_BOX_H, x0 + MAIN_BOX_W, y0 + MAIN_BOX_H*2 + MAIN_BOX_CMNT_H, 0xf5f5, 0);
		LCD_draw_rectangle(x0+1, y0 + MAIN_BOX_CMNT_H + MAIN_BOX_H + 1, x0 + MAIN_BOX_W - 1, y0 + MAIN_BOX_H*2 + MAIN_BOX_CMNT_H - 1, 0xf5f5, 0);

    		Font_show_str(x0 + (MAIN_BOX_W / 2) - 20, y0 + MAIN_BOX_CMNT_H + MAIN_BOX_H*2 + 1 , 200, 24, "×ªËÙ",24, 0);  
		
		Font_show_str(x0 + (MAIN_BOX_W / 2) - 50, y0 + MAIN_BOX_CMNT_H + MAIN_BOX_H + MAIN_BOX_H / 2, 200, 24,"240r/h", 24, 0);

	}


	
}



















