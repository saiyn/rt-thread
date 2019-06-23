

#define RT_DISP_CMD_DARW_POINT 0x13

#define DIR_HORIZON 	(1)
#define DIR_VERTICAL 	(0)

typedef struct
{
	unsigned short lcd_reg;
	unsigned short lcd_ram;
}lcd_typedef;


#define LCD_BASE	((unsigned int)(0x6c000000 | 0x000007ef))
#define LCD		((lcd_typedef *)LCD_BASE)


#define LCD_WRITE_REG(r, v) do{	\
	LCD->lcd_reg = r;	\
	LCD->lcd_ram = v;	\
}while(0)


#define LCD_WRITE_PREPARE do{	\
	LCD->lcd_reg = _lcd.cmd_wram;	\
}while(0)




typedef struct
{
	unsigned short w,h;
	unsigned short id;
	unsigned char dir;
	unsigned char cmd_wram;
	unsigned char cmd_setx;
	unsigned char cmd_sety;

}lcd_dev_t;




typedef struct{
	unsigned short x0,y0,x1,y1,color;
	
}disp_arg_t;


static rt_device _display;
static lcd_dev_t _lcd;



static void lcd_set_cursor(unsigned short x, unsigned short y)
{
	if(_lcd.dir == DIR_HORIZON)
	{
		x = _lcd.w - 1 - x;
	}
	
	LCD_WRITE_REG(_lcd.cmd_setx, x);
	LCD_WRITE_REG(_lcd.cmd_sety, y);

}



static void _dis_draw_point(disp_arg_t *arg)
{
	lcd_set_cursor(arg->x0, arg->y0);
	LCD_WRITE_PREPARE;
	LCD->LCD_RAM = POINT_COLOR;
}


static rt_err_t _disp_control(rt_device_t dev, int cmd, void *args)
{
	switch(cmd)
	{
		case RT_DISP_CMD_DRAW_POINT:
			_dis_draw_point((disp_arg_t *)args);
		break;

		default:
			rt_printk("NO support yet - %s:(%d)\r\n", __FUNCTION__, cmd);
		break;
	}

	return 0;
}


static rt_err_t _disp_init(struct rt_device *dev)
{
	

}


#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops = 
{



};




int rt_hw_display_init(void)
{
	_display.type = RT_Device_Class_Miscellaneous;
	_display.rx_indicate = RT_NULL;
	_display.tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
	_display.ops = &disp_ops;
#endif

	_display.user_data = NULL;


	rt_device_register(&_display, "display", RT_DEVICE_FLAG_RDWR);


	return 0;
}





