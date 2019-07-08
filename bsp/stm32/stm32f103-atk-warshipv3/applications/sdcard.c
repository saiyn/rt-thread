#include <rtthread.h>


#include <dfs_elm.h>
#include <dfs_fs.h>
#include <dfs_posix.h>


static void sd_mount(void *arg)
{

	int retry = 5;

	for(;;)
	{
		rt_thread_mdelay(500);
		
		if(rt_device_find("sd0") != RT_NULL)
		{
			if(dfs_mount("sd0", "/", "elm", 0, 0) == RT_EOK)
			{
				rt_kprintf("sd card successfully mount to '/'\n");
				break;
			}
			else
			{
				if(retry--)
					rt_kprintf("sd card mount failed\n");

				else
					break;

			}

		}


	}
}



int Sdcard_mount(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("sd_mount", sd_mount, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX - 2, 20);

	if(tid != RT_NULL)
	{
		rt_thread_startup(tid);
	}
	else
	{

		rt_kprintf("create sd_mount thread fail\n");
	}

	return RT_EOK;
}
