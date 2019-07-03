#include "w25Q.h"
#include "font.h"
#include "lcd.h"

#define POINT_COLOR	(0x0000)
#define BACK_COLOR	(0xffff)


static Font_info_t _ftinfo;


int Font_init(void)
{
	int t;

	for(t = 0; t < 10; t++)
	{
		w25q_read((unsigned char *)&_ftinfo, FONT_INFO_ADDR, sizeof(_ftinfo));

		if(_ftinfo.fontMagic == 0xaa)
		{
			rt_kprintf("find font well f12addr:0x%x\n", _ftinfo.f12addr);
			return 0;
		}
			

		rt_hw_us_delay(20*1000);
	}

	rt_kprintf("no font found:%d\n", _ftinfo.fontMagic);

	return -1;
}


static void get_HzMat(rt_uint8_t *code, rt_uint8_t *mat, rt_uint8_t size)
{
	rt_uint8_t qh, ql;
	rt_uint8_t i;
	unsigned long foffset;

	rt_uint8_t csize = (size / 8 + ((size % 8) ? 1:0)) * size;

	qh = *code;
	ql = *(++code);

	if(qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)
	{
		for(i = 0; i < csize; i++)
			*mat++ = 0x00;

		rt_kprintf("get hzmat error, qh:0x%x, ql:0x%x\n", qh, ql);

		return;
	}

	rt_kprintf("in hzmat, qh:0x%x, ql:0x%x\n", qh, ql);

	if(ql < 0x7f)
		ql -= 0x40;
	else
		ql -= 0x41;

	qh -= 0x81;

	foffset = ((unsigned long)(190 *qh) + ql) * csize;

	switch(size)
	{
		case 12:
			w25q_read(mat, foffset+_ftinfo.f12addr, csize);
		break;

		case 16:
			w25q_read(mat, foffset+_ftinfo.f16addr, csize);
		break;

		case 24:
			w25q_read(mat, foffset+_ftinfo.f24addr, csize);

		break;
	}


}


void Font_show_font(rt_uint16_t x, rt_uint16_t y, rt_uint8_t *font, rt_uint8_t size, rt_uint8_t mode)
{
	rt_uint8_t tmp, t, t1;
	rt_uint16_t y0 = y;
	rt_uint8_t dzk[72];

	rt_uint8_t csize = (size / 8 + ((size % 8) ? 1:0)) * size;

	if(size != 12 && size != 16 && size != 24)
	{
		rt_kprintf("unsupported font size:%d\n", size);
		return;
	}


	get_HzMat(font, dzk, size);

	for(t = 0; t < csize; t++)
	{
		tmp = dzk[t];

		for(t1 = 0; t1 < 8; t1++)
		{
			if(tmp & 0x80)
				LCD_fast_drawPoint(x, y, POINT_COLOR);
			else if(mode == 0)
				LCD_fast_drawPoint(x, y, BACK_COLOR);

			tmp <<= 1;
			y++;
			if((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}

		}
	
	}

}


void Font_show_str(rt_uint16_t x, rt_uint16_t y, rt_uint16_t w, rt_uint16_t h, rt_uint8_t *str, rt_uint8_t size, rt_uint8_t mode)
{
	rt_uint16_t x0 = x;
	rt_uint16_t y0 = y;
	int bHz = 0;

	while(*str != 0)
	{

		if(!bHz)
		{
			if(*str > 0x80) 
				bHz = 1;
			else
			{
				if(x > (x0 + w - size / 2))
				{
					y += size;
					x = x0;
				}

				if(y > (y0 + h - size))
					break;

				if(*str == 13)
				{
					y += size;
					x = x0;
					str++;
				}
				else
					LCD_show_char(x, y, *str, size, mode);


				str++;
				x += size / 2;
			}
		}
		else
		{
			bHz = 0;

			if(x > (x0 + w - size))
			{
				y += size;
				x = x0;
			}

			if(y > (y0 + h - size))
				break;

			Font_show_font(x,y,str,size,mode);
			str += 2;
			x += size;
		}


	}


}




