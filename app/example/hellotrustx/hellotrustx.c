/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <stdio.h>
#include <hal/soc/soc.h>
#include <aos/errno.h>
#include <k_api.h>

#include <hal/soc/i2c.h>

#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <vfs_gpio.h>
#include <vfs_i2c.h>

//#include "driver/gpio.h"
//#include "driver/i2c.h"

//#include <gpio.h>
//#include <../../../platform/mcu/esp32/bsp/include/driver/include/driver/gpio.h>
#include <../../../platform/mcu/esp32/bsp/include/soc/esp32/include/soc/gpio_sig_map.h>


#include "trustx/optiga/include/optiga/Version.h"

#include "pal_gpio.h"

#include "trustx/optiga/include/optiga/pal/pal.h"
#include "trustx/optiga/include/optiga/comms/optiga_comms.h"
#include "trustx/optiga/include/optiga/optiga_util.h"
#include "trustx/optiga/include/optiga/optiga_crypt.h"
#include "trustx/optiga/include/optiga/pal/pal_os_timer.h"
#include "trustx/optiga/include/optiga/pal/pal_os_event.h"
#include "trustx/optiga/include/optiga/ifx_i2c/ifx_i2c.h"

//Independent AliOS test routines
#define ENABLE_GPIO_TEST              0
//When VFS interface is enabled, a unified user interface for various files (including device files and system files) is enabled.
//Otherwise, the application will access directly to the HAL layer
#define ENABLE_VFS_INTERFACE          0
#define ENABLE_TIMER_TEST             0
#define ENABLE_EVENT_TIMER_TEST       0
#define ENABLE_CREATE_NEW_TASK_TEST   0
#define ENABLE_QUEUE_TEST             0
#define ENABLE_I2C_TEST               0
#define ENABLE_TRUSTX_DRIVER          1

void timer_test(void);
void gpio_test(void);
void event_timer_test(void);
void create_new_task_test(void);
void queue_test(void);
void i2c_test(void);

//Timer
aos_timer_t event_timer;

//GPIO 18 is used to control Trust X reset pin
#define GPIO_IO_TRUSTX_RST    U1RTS_OUT_IDX
gpio_dev_t trustx_reset =
{
    .port = GPIO_IO_TRUSTX_RST, /* gpio port config */
    .config = OUTPUT_PUSH_PULL      /* set as output mode */
};

pal_gpio_t* trustx_reset_context;

static int32_t optiga_init(void); 
optiga_comms_t optiga_comms = {(void*)&ifx_i2c_context_0, NULL, NULL, 0};
static host_lib_status_t optiga_comms_status;

static void gpio_init(void);

void gpio_init(void)
{
    hal_gpio_init(&trustx_reset); 
	//Make sure GPIO is high after init
	hal_gpio_output_high(&trustx_reset);
    return;
}

/**
 * OPTIGA™ Trust X comms handler
 */
static void optiga_comms_event_handler(void* upper_layer_ctx, host_lib_status_t event)
{
    optiga_comms_status = event;
}

/**
 * OPTIGA™ Trust X initialization
 */
static int32_t optiga_init(void)
{
	int32_t status = (int32_t) OPTIGA_LIB_ERROR;

	//printf(">optiga_init()\r\n");

	do
	{
		if (pal_os_event_init() == PAL_STATUS_FAILURE)
		{
			return OPTIGA_COMMS_BUSY;
		}
		
		pal_os_timer_delay_in_milliseconds(5);
		//printf("pal_os_event_init completed\r\n");

		//Invoke optiga_comms_open to initialize the IFX I2C Protocol and security chip
		optiga_comms_status = OPTIGA_COMMS_BUSY;
		optiga_comms.upper_layer_handler = optiga_comms_event_handler;
		status = optiga_comms_open(&optiga_comms);
		if(E_COMMS_SUCCESS != status)
		{
			printf("Failure: optiga_comms_open(): 0x%04X\n\r", status);
			break;
		}

		pal_os_timer_delay_in_milliseconds(10);

		//Wait until IFX I2C initialization is complete
		while(optiga_comms_status == OPTIGA_COMMS_BUSY)
		{
			pal_os_timer_delay_in_milliseconds(20);
		}		

		if((OPTIGA_COMMS_SUCCESS != status) || (optiga_comms_status == OPTIGA_COMMS_ERROR))
		{
			printf("Failure: Unable to initialize IFX I2C: 0x%04X\n\r", status);
			break;
		}

		status = optiga_util_open_application(&optiga_comms);
		if(OPTIGA_LIB_SUCCESS != status)
		{
			printf("Failure: CmdLib_OpenApplication(): 0x%04X\n\r", status);
			break;
		}

		status = OPTIGA_LIB_SUCCESS;
	} while(0);

	//printf("<optiga_init()\r\n");

	return status;
}


static void app_delayed_action(void *arg)
{
    printf(">app_delayed_action()\r\n");
    LOG("hellotrustx %s:%d %s\r\n", __func__, __LINE__, aos_task_name());
    aos_post_delayed_action(5000, app_delayed_action, NULL);
}

int application_start(int argc, char *argv[])
{       
	int ret = -1;

    LOG(">application started()");
    printf("Trust X Library:%s\r\n", VERSION_HOST_LIBRARY);
	printf("*************************************\n");
	printf("Compiled time: %s %s\n", __DATE__, __TIME__);
	printf("*************************************\n");

	printf("Task name: %s\r\n", aos_task_name());

#if (ENABLE_TIMER_TEST == 1)
	timer_test();
#endif	
#if (ENABLE_GPIO_TEST == 1)	
	gpio_test();
#endif	
#if (ENABLE_EVENT_TIMER_TEST == 1)	
	event_timer_test();
#endif
#if (ENABLE_NEW_TASK_TEST == 1)	
	create_new_task_test();
#endif

#if (ENABLE_QUEUE_TEST == 1)	
	queue_test();
#endif

#if (ENABLE_I2C_TEST == 1)
	i2c_test();
#endif	

#if (ENABLE_TRUSTX_DRIVER ==1)
	printf("Start Trust X driver\r\n");
	gpio_init();
	optiga_init();	

	uint8_t UID[27];
	uint16_t  UID_length = 27;
	printf("Trust X UID:\r\n");
	optiga_util_read_data(eCOPROCESSOR_UID, 0, UID, &UID_length);
	for(int x=0; x<27;)
	{
		if(x!=24){
			printf("%.2x %.2x %.2x %.2x\n", UID[x], UID[x+1], UID[x+2], UID[x+3]);
		}
		else{
			printf("%.2x %.2x %.2x\n", UID[x], UID[x+1], UID[x+2]);

		}
		x+=4;
	}

#endif

    //aos_post_delayed_action(5000, app_delayed_action, NULL);
    //aos_loop_run();

    return 0;
}

#define TEST_CONFIG_SYNC_TIMES                  (100000)
#define TEST_CONFIG_QUEUE_BUF_SIZE              (32)
static char         queue_buf[TEST_CONFIG_QUEUE_BUF_SIZE];
static aos_queue_t  g_queue1;
static char         queue1_buf[TEST_CONFIG_QUEUE_BUF_SIZE];
static aos_queue_t  g_queue2;
static char         queue2_buf[TEST_CONFIG_QUEUE_BUF_SIZE];
static aos_queue_t  g_queue3;
static char         queue3_buf[TEST_CONFIG_QUEUE_BUF_SIZE];
/* task: g_queue1 -> g_queue2 */
static void task8(void *arg)
{
    int msg = -1;
    unsigned int size = 0;

    while(1) {
        aos_queue_recv(&g_queue1, -1, &msg, &size);
        aos_queue_send(&g_queue2, &msg, size);
        if(msg == TEST_CONFIG_SYNC_TIMES) {
            break;
        }
    }
    //cut_printf("%s exit!\r\n", aos_task_name());
    aos_task_exit(0);
}

/* task: g_queue2 -> g_queue3 */
static void task9(void *arg)
{
    int msg = -1;
    unsigned int size = 0;

    while(1) {
        aos_queue_recv(&g_queue2, -1, &msg, &size);
        aos_queue_send(&g_queue3, &msg, size);
        if(msg == TEST_CONFIG_SYNC_TIMES) {
            break;
        }
    }
    //cut_printf("%s exit!\r\n", aos_task_name());
    aos_task_exit(0);
}
 
void queue_test(void)
{
	 int          ret = -1;
    unsigned int stack_size = 512;
    unsigned int msg_send = 0;
    unsigned int msg_recv = 0;
    unsigned int size_send = sizeof(msg_send);
    unsigned int size_recv = 0;
    int i = 0;

	printf(">queue_test()\r\n");

    ret = aos_queue_new(&g_queue1, queue1_buf, TEST_CONFIG_QUEUE_BUF_SIZE, TEST_CONFIG_QUEUE_BUF_SIZE);
    ret = aos_queue_new(&g_queue2, queue2_buf, TEST_CONFIG_QUEUE_BUF_SIZE, TEST_CONFIG_QUEUE_BUF_SIZE);
    ret = aos_queue_new(&g_queue3, queue3_buf, TEST_CONFIG_QUEUE_BUF_SIZE, TEST_CONFIG_QUEUE_BUF_SIZE);

    ret = aos_task_new("task8", task8, NULL, stack_size);
    ret = aos_task_new("task9", task9, NULL, stack_size);

    for(i=1; i<=TEST_CONFIG_SYNC_TIMES; i++) {
        msg_send = i;
        ret = aos_queue_send(&g_queue1, &msg_send, size_send);
        ret = aos_queue_recv(&g_queue3, -1, &msg_recv, &size_recv);
        //ASSERT_EQ(ret, 0);
        //ASSERT_EQ(msg_send, msg_recv);
        //ASSERT_EQ(size_send, size_recv);
        if(i%(TEST_CONFIG_SYNC_TIMES/10) == 0) {
			printf("%d/%d\r\n", i, TEST_CONFIG_SYNC_TIMES);
        }
    }
    //ASSERT_EQ(msg_recv, TEST_CONFIG_SYNC_TIMES);

	printf("queue_test completed\r\n");

    aos_queue_free(&g_queue1);
    aos_queue_free(&g_queue2);
    aos_queue_free(&g_queue3);
    aos_task_exit(0);
}

static aos_sem_t    g_sem_taskexit_sync; 
static int task1_counter = 0;

static void task1(void *arg)
{
    int i=0;
	for(i=0;i<10;i++){
		task1_counter++;
	}
    aos_sem_signal(&g_sem_taskexit_sync);
    aos_task_exit(0);
}

void create_new_task_test(void)
{
	unsigned int stack_size = 1024;
    int ret = -1;

	printf(">create_new_task_test()\r\n");

    aos_sem_new(&g_sem_taskexit_sync, 0);

    ret = aos_task_new("task1", task1, NULL, stack_size);

    aos_sem_wait(&g_sem_taskexit_sync, -1);
    printf("task1 exit!\r\n");

	printf("newtask completed. event_counter = %d\r\n", task1_counter );	

    aos_sem_free(&g_sem_taskexit_sync);
    aos_task_exit(0);
}

static aos_sem_t    event_complete_flag;
static int event_counter = 0;

static void event_handler(void *arg, void *arg2)
{
	if(event_counter==10){
		aos_sem_signal(&event_complete_flag);	
	}else{
		event_counter++;
	}
}

void event_timer_test()
{
	int ret = -1;
	int repeat=1;

	printf(">event_timer_test()\r\n");
	ret = aos_sem_new(&event_complete_flag, 0);
	
	//动态创建软件定时器
    ret = aos_timer_new(&event_timer, event_handler, NULL, 100, repeat);
	
	aos_sem_wait(&event_complete_flag, -1);
    aos_sem_free(&event_complete_flag);

	aos_timer_stop(&event_timer);
    aos_timer_free(&event_timer);

	printf("Event completed. event_counter = %d\r\n", event_counter);	
}
 
void timer_test(void)
{
	uint32_t time_in_ms = 0;
	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("App Start time: %d ms\r\n", time_in_ms);
	
	pal_os_timer_delay_in_milliseconds(20);

	time_in_ms=0;
	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("Run time 1: %d ms\r\n", time_in_ms);
	
	pal_os_timer_delay_in_milliseconds(50); 
	
	time_in_ms=0;
	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("Run time 2: %d ms\r\n", time_in_ms);

	pal_os_timer_delay_in_milliseconds(100);

	time_in_ms=0;
	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("Run time 3: %d ms\r\n", time_in_ms);
}

void gpio_test(void)
{
#if (ENABLE_VFS_INTERFACE == 1)	
	char* gpio_path = "/dev/gpio/";
	printf("Register GPIO driver\r\n"); //Default GPIO is high
    ret = aos_register_driver(gpio_path, &gpio_ops, &trustx_reset);
	if(ret!=VFS_SUCCESS){
		printf("failed to register gpio device driver, ret=%d\r\n", ret);
		while(1){};
	}

	printf("Open GPIO driver\r\n");
	int fd_gpio = aos_open(gpio_path,0); 
	if(fd_gpio < AOS_CONFIG_VFS_FD_OFFSET){
		printf("failed to open gpio device driver, ret=%d\r\n", fd_gpio);
		while(1){};
	}

#if 1
    //Toggle API - On ESP32 takes 10.5ms
	printf("Toggle GPIO\r\n");
	ret = aos_ioctl(fd_gpio, IOCTL_GPIO_OUTPUT_TOGGLE, 0);
	if(ret!=0){
		printf("failed to configure output toggle, ret=%d\r\n", ret);
	}
#else
	//Manual toggle - On ESP32 takes 9.36ms
	ret = aos_ioctl(fd_gpio, IOCTL_GPIO_OUTPUT_LOW, 0);
	if(ret!=0){
		printf("failed to configure output low, ret=%d\r\n", ret);
	}
	
	ret = aos_ioctl(fd_gpio, IOCTL_GPIO_OUTPUT_HIGHT, 0);
	if(ret!=0){
		printf("failed to configure output high, ret=%d\r\n", ret);
	}	
#endif

	printf("Close GPIO driver\r\n");
	ret = aos_close(fd_gpio);
	if(ret!=VFS_SUCCESS){
		printf("failed to close gpio driver, ret=%d\r\n", ret);
	}

#else
	//Direct access to the GPIO HAL layer
	gpio_init();

#if 0
	//Toggle GPIO (On ESP32, takes 9.31ms)
	hal_gpio_output_toggle(&trustx_reset);
#else	
    //Manual toggle GPIO (On ESP32, takes 9.35ms)
	//Set GPIO low
	hal_gpio_output_low(&trustx_reset);
	//Set GPIO high
    hal_gpio_output_high(&trustx_reset);	
	hal_gpio_finalize(&trustx_reset);
#endif	
#endif

}

i2c_dev_t i2c_dev_test =
{
    .port = 0,
    .config.address_width = 7,
    .config.freq = 100000,
    .config.mode = I2C_MODE_MASTER,
    .config.dev_addr = (0x30<<1)
};

void i2c_test(void)
{
	char* i2c_path = "/dev/i2c/";
    int fd_i2c = 0;    
    int i = 0;
    int ret = -1;
	int res = 0;
	uint8_t write_buf[10] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA};
	uint8_t read_buf[10] = {0};

	printf(">i2c_test()\r\n");

	 /* i2c_ops is a structure in vfs_i2c.c */
    ret = aos_register_driver(i2c_path, &i2c_ops, &i2c_dev_test);
	if(ret!=VFS_SUCCESS){
		printf("failed to register i2c driver, ret=%d\r\n", ret);
	}
    
	printf("Open i2c:\r\n");
    fd_i2c = aos_open(i2c_path,0);
	if(ret!=VFS_SUCCESS){
		printf("failed to open i2c driver, ret=%d\r\n", ret);
	}
	
/*
	printf("Write i2c:\r\n");
    ret = aos_write(fd_i2c, write_buf, sizeof(write_buf));
	if(ret<0){
		printf("failed to write i2c driver, ret=%d\r\n", ret);
	}else{
		printf("failed to write i2c device, ret=%d\r\n", ret);
	}
*/  
#if 0	
	printf("Read i2c:\r\n");
    ret = aos_read(fd_i2c, read_buf, sizeof(read_buf));
	if(ret < 0){
		printf("failed to read i2c device, ret=%d\r\n", ret);
	}else{
		printf("read i2c device, ret=%d\r\n", ret);
	}

    for (i = 0; i < 10; i++) {
        if(read_buf[i] != i) {
            res = -1;
        }
    }
#endif

	printf("Close i2c:\r\n");
	ret = aos_close(fd_i2c);
	if(ret!=VFS_SUCCESS){
		printf("failed to close i2c driver, ret=%d\r\n", ret);
	}
	printf("<i2c_test()\r\n");
    
}