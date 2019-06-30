#include "w25Q.h"
#include "font.h"

static Font_info_t _ftinfo;


int Font_init(void)
{
	int t;

	for(t = 0; t < 10; t++)
	{
		w25q_read((unsigned char *)&_ftinfo, FONT_INFO_ADDR, sizeof(_ftinfo));

		if(_ftinfo.fontMagic == 0xaa)
		{
			rt_kprintf("find font well\n");
			return 0;
		}
			

		rt_hw_us_delay(20*1000);
	}

	rt_kprintf("no font found:%d\n", _ftinfo.fontMagic);

	return -1;
}
