#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdio.h>
#include "board.h"

#define SIM_UART_NAME	"uart3"


struct rx_msg
{
	rt_device_t dev;
	rt_size_t size;
};


static rt_device_t _serial;
static struct rt_messagequeue rx_mq;

static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;


static rt_err_t sim_input(rt_device_t dev, rt_size_t size)
{
	rt_err_t result;
	struct rx_msg msg;
	msg.dev = dev;
	msg.size = size;

	result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
	if(result == -RT_EFULL)
	{
		rt_kprintf("msg queue full\n");
	}

	
	return result;
}


static void sim_read_thread(void *arg)
{
	struct rx_msg msg;
	rt_err_t result;
	rt_uint32_t rx_length;
	static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

	while(1)
	{
		rt_memset(&msg, 0, sizeof(msg));

		result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
		if(result == RT_EOK)
		{
			rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
			rx_buffer[rx_length] = '\0';

			rt_kprintf("%s", rx_buffer);

		}

	}


}

void SIM_Init(void)
{
	rt_thread_t tid;
	static char msg_pool[512];

	_serial = rt_device_find(SIM_UART_NAME);

	if(!_serial){
		rt_kprintf("can't find device:[%s]\n", SIM_UART_NAME);

		return;
	}


	rt_mq_init(&rx_mq, "rx_mq", msg_pool, sizeof(struct rx_msg), sizeof(msg_pool), RT_IPC_FLAG_FIFO);

	rt_device_open(_serial, RT_DEVICE_FLAG_DMA_RX|RT_DEVICE_FLAG_STREAM);

	rt_device_control(_serial, RT_DEVICE_CTRL_CONFIG, &config);

	rt_device_set_rx_indicate(_serial, sim_input);

	tid = rt_thread_create("sim_read", sim_read_thread, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX -1, 20);
	if(tid != RT_NULL)
	{
		rt_thread_startup(tid);
	}
	else
	{
		rt_kprintf("create sim_read thread fail\n");
	}
	
}



static void atcmd(int argc, char **argv)
{
	char buf[64] = {0};

	if(argc < 2){
		rt_kprintf("too few argv in atcmd\n");
		return;
	}


	rt_kprintf("atcmd:%s\n", argv[1]);
	
	sprintf(buf, "%s\r\n", argv[1]);

	rt_device_write(_serial, 0, buf, strlen(buf));
	
}

MSH_CMD_EXPORT(atcmd, at cmd transfor);

#include <at.h>

#define DBG_TAG  "app.SIM"
#define DBG_LVL  DBG_INFO
#include <rtdbg.h>


#define SIM868_CT_PIN	GET_PIN(B,1)

#define SIM868_CT_ON do{			\
	rt_pin_write(SIM868_CT_PIN, PIN_LOW);	\
}while(0)

#define SIM868_CT_OFF do{			\
	rt_pin_write(SIM868_CT_PIN, PIN_HIGH);	\
}while(0)



static struct at_urc urc_table[] = {
	{""},

};

static at_client_t sim868;


void SIM868_Init(void)
{
	int ret = 0;

	if(at_client_init(SIM_UART_NAME, 256) !=0){
		LOG_E("at init fail");
		return;
	}

	sim868 = at_client_get(SIM_UART_NAME);
	if(!sim868){
		LOG_E("at get client fail");
		return;
	}
		
	ret = at_client_obj_wait_connect(sim868, 700);
	if(ret == 0){
		LOG_I("sim868 connect ok");
		return;
	}

	if(ret == -RT_ETIMEOUT){
		LOG_E("at connect fail, try power up");

		SIM868_CT_ON;
	
		rt_thread_mdelay(600);

		SIM868_CT_OFF;

		ret = at_client_obj_wait_connect(sim868, 3000);
		if(ret < 0){
			LOG_E("sim868 can't connect");
			return;
		}

		LOG_I("at connect ok after power on");
		return;	
	}

	LOG_E("at connect fail:[%d]", ret);

}

void SIM868_GSM_Init(void)
{
	int ret;
	int n,stat;
	at_response_t resp;

	ret = at_client_obj_wait_connect(sim868, 2000);
	if(ret < 0){
		LOG_E("sim868 can't connect");
		return;
	}

	at_obj_exec_cmd(sim868, RT_NULL, "ATE0\r\n");
	
	resp = at_create_resp(128, 0, rt_tick_from_millisecond(500));

	ret = at_obj_exec_cmd(sim868, resp, "AT+CPIN?\r\n");
	if(ret < 0){
		LOG_E("sim868 sim card can't be detected");
		return;
	}
	
	if(!at_resp_get_line_by_kw(resp, "+CPIN: READY")){
		LOG_E("SIM card is not ready");
		return;
	}

	ret = at_obj_exec_cmd(sim868, resp, "AT+CREG?\r\n");
	
	if(at_resp_parse_line_args_by_kw(resp, "+CREG:", "+CREG:%[^ ]%d,%d", &n, &stat) <= 0)
	{
		LOG_E("SIM GSM network unkown");
	}
	else{
		if(stat != 1 && stat != 5){
			LOG_E("SIM GSM network state problem:[%d]-[%d]", n, stat);
		}
		else{
			LOG_I("SIM GSM OK");
		}
	}

	ret = at_obj_exec_cmd(sim868, resp, "AT+CGREG?\r\n");
	
	if(at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG:%[^ ]%d,%d", &n, &stat) <= 0)
	{
		LOG_E("SIM GPRS network unkown");
	}
	else{
		if(stat != 1 && stat != 5){
			LOG_E("SIM GPRS network state problem:[%d]-[%d]", n, stat);
		}
		else{
			LOG_I("SIM GPRS OK");
		}
	}

	//close TCP connect to discard any protential provious tcp session
	at_obj_exec_cmd(sim868, RT_NULL, "AT+CIPCLOSE=1\r\n");

	
	ret = at_client_obj_wait_wanted_resp(sim868, "AT+CIPSHUT\r\n", "SHUT OK", 1000);
	if(ret < 0){
		LOG_E("SIM CIPSHUT fail");
	}
	
	//set GPRS Mobile Station Class
	ret = at_client_obj_wait_wanted_resp(sim868, "AT+CGCLASS=\"B\"\r\n", "OK", 1000);
	//define PDP context
	ret = at_client_obj_wait_wanted_resp(sim868, "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n", "OK", 1000);
	//attach GPRS
	ret = at_client_obj_wait_wanted_resp(sim868, "AT+CGATT=1\r\n", "OK", 1000);
	//set GPRS connection Mode
	ret = at_client_obj_wait_wanted_resp(sim868, "AT+CIPCSGP=1,\"CMNET\"\r\n", "OK", 1000);


}
///////////////////////////////////////////////////////////////////////////
#include <at_device_sim800c.h>

#include <arpa/inet.h>
#include <netdev.h>

#define SERVER_HOST   "52.15.134.205"
#define SERVER_PORT 	80


int Sim_server_login(int argc, char **argv)
{
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	struct netdev *netdev = RT_NULL;
	int sockfd = -1;

	netdev = netdev_get_by_name("sim868");
	if(netdev == RT_NULL)
	{
		rt_kprintf("can't get netdev:sim868\n");
		return -RT_ERROR;
	}

	if((sockfd = socket(AF_AT, SOCK_STREAM, 0)) < 0 )
	{
		rt_kprintf("");
	
	}

	client_addr.sin_family = AF_AT;
	client_addr.sin_port = htons(8088);

	client_addr.sin_addr.s_addr = netdev->ip_addr.addr;
	rt_memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));

	if(bind(sockfd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0){
		rt_kprintf("socket bind failed\n");
		closesocket(sockfd);
		return -RT_ERROR;
	}

	rt_kprintf("socket bind network interface device(%s) success\n", netdev->name);

	server_addr.sin_family = AF_AT;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_HOST);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0){
		rt_kprintf("socket connect failed\n");
		closesocket(sockfd);
		return -RT_ERROR;
	}else{
		rt_kprintf("socket connect success\n");
	}

	closesocket(sockfd);
	
	return RT_EOK;
}

MSH_CMD_EXPORT(Sim_server_login, login server by sim gprs);



static struct at_device_sim800c _sim868 = 
{
	"sim0",
	SIM_UART_NAME,
	-1,
	-1,
	512,
};


static int sim868_device_register(void)
{
	struct at_device_sim800c *sim868 = &_sim868;

	return at_device_register(&(sim868->device),
					sim868->device_name,
					sim868->client_name,
					AT_DEVICE_CLASS_SIM800C,(void *)sim868);
}

INIT_APP_EXPORT(sim868_device_register);






















