#ifndef __LCD__H__
#define __LCD__H__


#define POINT_COLOR	(0x0000)
#define BACK_COLOR	(0xffff)

void LCD_init();

void LCD_fast_drawPoint(rt_uint16_t x, rt_uint16_t y, rt_uint16_t color);

void LCD_show_ui();


void LCD_show_char(rt_uint16_t x, rt_uint16_t y, rt_uint8_t c, rt_uint8_t size, rt_uint8_t mode);







#endif
