#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"

#define RT_DISP_CMD_DRAW_POINT 0x13

#define DIR_HORIZON 	(1)
#define DIR_VERTICAL 	(0)


#define POINT_COLOR	(0x0000)
#define BACK_COLOR	(0xffff)

typedef struct
{
	volatile unsigned short lcd_reg;
	volatile unsigned short lcd_ram;
}lcd_typedef;


#define LCD_BASE	((unsigned int)(0x6c000000 | 0x000007ef))
#define LCD		((lcd_typedef *)LCD_BASE)


#define LCD_WRITE_REG(r) do{	\
	LCD->lcd_reg = r;	\
}while(0)


#define LCD_WRITE_PREPARE do{	\
	LCD->lcd_reg = _lcd.cmd_wram;	\
}while(0)

#define LCD_WRITE_DATA(d) do{	\
	LCD->lcd_ram = d;	\
}while(0)

#define LCD_READ_REG(r)  ({	\
	unsigned short d;	\
	LCD->lcd_reg = r;	\
	delay_us(5);		\
	d = LCD->lcd_ram;	\
	d;			\
})






typedef struct
{
	unsigned short w,h;
	unsigned short id;
	unsigned char dir;
	unsigned short cmd_wram;
	unsigned short cmd_setx;
	unsigned short cmd_sety;

}lcd_dev_t;




typedef struct{
	unsigned short x0,y0,x1,y1,color;
	
}disp_arg_t;


static SRAM_HandleTypeDef hsram;
static struct rt_device _display;
static lcd_dev_t _lcd;


void delay_us(rt_uint32_t us)
{
	while(us--){
		__NOP();
	}
}


void delay_ms(rt_uint32_t ms)
{
	while(ms--)
	{

		int i;
		for(i = 0; i < 1000; i++)
		{
			__NOP();
		}
	}

}



static void lcd_set_cursor(unsigned short x, unsigned short y)
{
	if(_lcd.dir == DIR_HORIZON)
	{
		x = _lcd.w - 1 - x;
		LCD_WRITE_REG(_lcd.cmd_setx);
		LCD_WRITE_DATA(0);
		LCD_WRITE_DATA(0);
		LCD_WRITE_DATA(x >> 8);
		LCD_WRITE_DATA(x & 0xff);
	}
	else
	{
		LCD_WRITE_REG(_lcd.cmd_setx);
		LCD_WRITE_DATA(x >> 8);
		LCD_WRITE_DATA(x & 0xff);
		LCD_WRITE_DATA((_lcd.w - 1) >> 8);
		LCD_WRITE_DATA((_lcd.w - 1) & 0xff);
	}
	
	LCD_WRITE_REG(_lcd.cmd_sety);
	
	LCD_WRITE_DATA(y >> 8);
	LCD_WRITE_DATA(y & 0XFF);
	LCD_WRITE_DATA((_lcd.h - 1) >> 8);
	LCD_WRITE_DATA((_lcd.h - 1) & 0xff);
}



static void _dis_draw_point(disp_arg_t *arg)
{
	lcd_set_cursor(arg->x0, arg->y0);
	LCD_WRITE_PREPARE;
	LCD->lcd_ram = POINT_COLOR;
}


static rt_err_t _disp_control(rt_device_t dev, int cmd, void *args)
{
	switch(cmd)
	{
		case RT_DISP_CMD_DRAW_POINT:
			_dis_draw_point((disp_arg_t *)args);
		break;

		default:
			rt_kprintf("NO support yet - %s:(%d)\r\n", __FUNCTION__, cmd);
		break;
	}

	return 0;
}


static rt_err_t _disp_init(struct rt_device *dev)
{

	FSMC_NORSRAM_TimingTypeDef wrTiming;
	FSMC_NORSRAM_TimingTypeDef wTiming;

	hsram.Instance = FSMC_NORSRAM_DEVICE;
	hsram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

	wrTiming.AddressSetupTime = 0x01; /* 2 HCLK = 1/36M = 27ns */
	wrTiming.AddressHoldTime = 0x00;
	wrTiming.DataSetupTime = 0x0f; //16 HCLK
	wrTiming.BusTurnAroundDuration = 0x00;
	wrTiming.CLKDivision = 0x00;
	wrTiming.DataLatency = 0x00;
	wrTiming.AccessMode = FSMC_ACCESS_MODE_A;

	wTiming.AddressSetupTime = 0x00; //1 HCLK
	wTiming.AddressHoldTime = 0x00;
	wTiming.DataSetupTime = 0x03; //4 HCLK
	wTiming.BusTurnAroundDuration = 0x00;
	wTiming.CLKDivision = 0x00;
	wTiming.DataLatency = 0x00;
	wTiming.AccessMode = FSMC_ACCESS_MODE_A;

	hsram.Init.NSBank = FSMC_NORSRAM_BANK1;
	hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
	hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
	hsram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
	hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
	hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
	hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
	hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
	hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
	hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

	if(HAL_SRAM_Init(&hsram, &wrTiming, &wTiming) != HAL_OK)
	{
		rt_kprintf("fsmc for lcd init failed");
		return -1;
	}

	return 0;
}

#define SSD_HOR_RES (800)
#define SSD_VER_RES (480)


#define SSD_HOR_PULSE_WIDTH	(1)
#define SSD_HOR_BACK_PORCH	(46)
#define SSD_HOR_FRONT_PORCH	(210)

#define SSD_VER_PULSE_WIDTH	(1)
#define SSD_VER_BACK_PORCH	(23)
#define SSD_VER_FRONT_PORCH	(22)

#define SSD_HT (SSD_HOR_RES + SSD_HOR_BACK_PORCH + SSD_HOR_FRONT_PORCH)
#define SSD_HPS (SSD_HOR_BACK_PORCH)
#define SSD_VT (SSD_VER_RES + SSD_VER_BACK_PORCH + SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)


#define D2U_L2R		(6)
#define DET_SCAN_DIR  D2U_L2R

#define LCD_LED_PIN	GET_PIN(B, 0)


#define LCD_LED_ON do{				\
	rt_pin_write(LCD_LED_PIN, PIN_LOW);	\
}while(0)


#define WHITE	(0xffff)


void LCD_Set_BackLight(unsigned char pwm)
{
	LCD_WRITE_REG(0xBE);
	LCD_WRITE_DATA(0x05);
	LCD_WRITE_DATA(pwm*2.55);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
}


void LCD_Scan_Dir(unsigned char dir)
{
	unsigned short regval = 0;
	unsigned short dirreg = 0;
	unsigned tmp;


	if(dir == D2U_L2R)
	{
		regval |= (0 << 7) | (0 << 6) | (1 << 5);
	}

	dirreg = 0x36;

	LCD_WRITE_REG(dirreg);
	LCD_WRITE_DATA(regval);

	LCD_WRITE_REG(_lcd.cmd_setx);
	LCD_WRITE_DATA(0);
	LCD_WRITE_DATA(0);
	LCD_WRITE_DATA((_lcd.w - 1) >> 8);
	LCD_WRITE_DATA((_lcd.w - 1)&0xff);
	LCD_WRITE_REG(_lcd.cmd_sety);
	LCD_WRITE_DATA(0);
	LCD_WRITE_DATA(0);
	LCD_WRITE_DATA((_lcd.h - 1) >> 8);
	LCD_WRITE_DATA((_lcd.h - 1)& 0xff);
}


void LCD_Set_Dir(unsigned char d)
{
	if(d == DIR_HORIZON)
	{
		_lcd.dir = d;
		_lcd.cmd_setx = 0x2a00;
		_lcd.cmd_sety = 0x2b00;
		_lcd.cmd_wram = 0x2c00;
		_lcd.w = 800;
		_lcd.h = 480;

	}

	LCD_Scan_Dir(DET_SCAN_DIR);
}


void LCD_Clear(unsigned short color)
{
	unsigned int i = 0;
	unsigned int totalpoint = _lcd.w * _lcd.h;

	lcd_set_cursor(0x00, 0x0000);

	LCD_WRITE_PREPARE;

	for(i = 0; i < totalpoint; i++)
	{
		LCD_WRITE_DATA(color);
	}


}



static rt_err_t _disp_open(struct rt_device *dev, rt_uint16_t oflag)
{
	_lcd.id = LCD_READ_REG(0x0000);

	rt_kprintf("get lcd id:0x%x\n", _lcd.id);

	LCD_WRITE_REG(0xE2);
	LCD_WRITE_DATA(0x1d);
	LCD_WRITE_DATA(0x02);
	LCD_WRITE_DATA(0x04);
	delay_us(100);

	LCD_WRITE_REG(0xe0);
	LCD_WRITE_DATA(0x01);
	delay_ms(10);

	LCD_WRITE_REG(0xe0);
	LCD_WRITE_DATA(0x03);
	delay_ms(12);

	LCD_WRITE_REG(0x01);
	delay_ms(10);

	LCD_WRITE_REG(0xe6);
	LCD_WRITE_DATA(0x2f);
	LCD_WRITE_DATA(0xff);
	LCD_WRITE_DATA(0xff);

	LCD_WRITE_REG(0xB0);
	LCD_WRITE_DATA(0x20);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_DATA((SSD_HOR_RES - 1)>>8);
	LCD_WRITE_DATA(SSD_HOR_RES - 1);
	LCD_WRITE_DATA((SSD_VER_RES - 1)>>8);
	LCD_WRITE_DATA(SSD_VER_RES-1);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_REG(0xB4);
	LCD_WRITE_DATA((SSD_HT - 1) >> 8);
	LCD_WRITE_DATA(SSD_HT - 1);
	LCD_WRITE_DATA(SSD_HPS >> 8);
	LCD_WRITE_DATA(SSD_HPS);
	LCD_WRITE_DATA(SSD_HOR_PULSE_WIDTH - 1);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);


	LCD_WRITE_REG(0xB6);
	LCD_WRITE_DATA((SSD_VT - 1) >> 8);
	LCD_WRITE_DATA(SSD_VT - 1);
	LCD_WRITE_DATA(SSD_VPS >> 8);
	LCD_WRITE_DATA(SSD_VPS);
	LCD_WRITE_DATA(SSD_VER_FRONT_PORCH - 1);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_REG(0xF0);
	LCD_WRITE_DATA(0x03);


	LCD_WRITE_REG(0x29);
	LCD_WRITE_REG(0xD0);
	LCD_WRITE_DATA(0x00);


	LCD_WRITE_REG(0xBE);
	LCD_WRITE_DATA(0x05);
	LCD_WRITE_DATA(0xFE);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_REG(0xB8);
	LCD_WRITE_DATA(0x03);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_REG(0xBA);
	LCD_WRITE_DATA(0x01);

	LCD_Set_BackLight(100);

	LCD_Set_Dir(DIR_HORIZON);

	LCD_LED_ON;

	LCD_Clear(WHITE);

	return 0;
}


#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops disp_ops = 
{
	_disp_init,
	_disp_open,
	RT_NULL,
	RT_NULL,
	RT_NULL,
	_disp_control
};
#endif



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





