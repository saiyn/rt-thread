#include "w25Q.h"

static struct rt_spi_device *w25q;


void w25q_init(void)
{
	struct rt_spi_configuration cfg;


	w25q = (struct rt_spi_device *)rt_device_find("spi20");


	cfg.data_width = 8;
	cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
	cfg.max_hz = 20 * 1000 * 1000;
	rt_spi_configure(w25q, &cfg);

	w25q_read_id();
}


void w25q_read_id(void)
{
	struct rt_spi_message msg1, msg2;
	rt_uint8_t cmd_read_id = 0x90;
	rt_uint8_t id[5] = {0};

	msg1.send_buf = &cmd_read_id;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;


	msg2.send_buf = RT_NULL;
	msg2.recv_buf = id;
	msg2.length = 5;
	msg2.cs_take = 0;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;
	
	rt_spi_transfer_message(w25q, &msg1);

	rt_kprintf("w25q id is 0x%x%x\n", id[3], id[4]);
}

void w25q_read(unsigned char *pbuf, unsigned int addr, unsigned short size)
{
	struct rt_spi_message msg1, msg2;
	rt_uint8_t cmd_and_addr[4] = {W25Q_READ_DATA, (rt_uint8_t)(addr >> 16), (rt_uint8_t)(addr >> 8), (rt_uint8_t)addr};
	
	msg1.send_buf = &cmd_and_addr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 4;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = RT_NULL;
	msg2.recv_buf = pbuf;
	msg2.length = size;
	msg2.cs_take = 0;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;

	rt_spi_transfer_message(w25q, &msg1);
}



