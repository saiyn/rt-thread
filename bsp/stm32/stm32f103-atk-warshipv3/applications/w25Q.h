#ifndef __W25Q_H__
#define __W25Q_H__

#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"


#define W25Q_READ_DATA	(0x03)



void w25q_init(void);


void w25q_read_id(void);


void w25q_read(unsigned char *pbuf, unsigned int addr, unsigned short size);








#endif
