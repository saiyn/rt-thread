#ifndef __FONT__H__
#define __FONT__H__

#include <rtthread.h>


#define FONT_INFO_ADDR	(1024*1024*12)


typedef struct
{
	unsigned char fontMagic;
	unsigned int ugbkaddr;
	unsigned int ugbksize;
	unsigned int f12addr;
	unsigned int gbk12size;
	unsigned int f16addr;
	unsigned int gbk16size;
	unsigned int f24addr;
	unsigned int gbk24size;

}__attribute__((packed)) Font_info_t;



int Font_init(void);



void Font_show_str(rt_uint16_t x, rt_uint16_t y, rt_uint16_t w, rt_uint16_t h, rt_uint8_t *str, rt_uint8_t size, rt_uint8_t mode);



#endif
