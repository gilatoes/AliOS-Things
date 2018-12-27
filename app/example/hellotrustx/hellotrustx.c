/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <stdio.h>
#include <hal/soc/soc.h>
#include <aos/errno.h>
#include <k_api.h>

#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_register.h>

#include <vfs_gpio.h>

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

//When VFS interface is enabled, a unified user interface for various files (including device files and system files) is enabled.
//Otherwise, the application will access directly to the HAL layer
#define ENABLE_VFS_INTERFACE    0
#define ENABLE_GPIO_TEST        0
#define ENABLE_TIMER_TEST       1

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

	printf(">optiga_init()\r\n");

	do
	{
		if (pal_os_event_init() == PAL_STATUS_FAILURE)
		{
			return OPTIGA_COMMS_BUSY;
		}

		//Invoke optiga_comms_open to initialize the IFX I2C Protocol and security chip
		optiga_comms_status = OPTIGA_COMMS_BUSY;
		optiga_comms.upper_layer_handler = optiga_comms_event_handler;
		status = optiga_comms_open(&optiga_comms);
		if(E_COMMS_SUCCESS != status)
		{
			printf("Failure: optiga_comms_open(): 0x%04X\n\r", status);
			break;
		}

		//Wait until IFX I2C initialization is complete
		while(optiga_comms_status == OPTIGA_COMMS_BUSY)
		{
			pal_os_timer_delay_in_milliseconds(10);
		}

		if((OPTIGA_COMMS_SUCCESS != status) || (optiga_comms_status == OPTIGA_COMMS_ERROR))
		{
			printf("Failure: optiga_comms_status(): 0x%04X\n\r", status);
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

	printf("<optiga_init()\r\n");

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
	printf("\n\nImage: %s %s\n", __DATE__, __TIME__);

//Simple GPIO test routines
#if (ENABLE_GPIO_TEST == 1)
#if (ENABLE_VFS_INTERFACE == 1)	
	char* gpio_path = "/dev/gpio/";
	printf("Register GPIO driver\r\n"); //Default GPIO is high
    ret = aos_register_driver(gpio_path, &gpio_ops, &trustx_reset);
	if(ret!=0){
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
	if(ret!=0){
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
#endif	
#endif
#endif

//Simple timer test routines
#if (ENABLE_TIMER_TEST == 1)

	uint32_t time_in_ms = 0;
	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("Start time: %d ms\r\n", time_in_ms);
	time_in_ms=0;

	pal_os_timer_delay_in_milliseconds(30);

	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("Running time 1: %d ms\r\n", time_in_ms);
	time_in_ms=0;

	pal_os_timer_delay_in_milliseconds(35);
	
	time_in_ms = pal_os_timer_get_time_in_milliseconds();
	printf("Running time 2: %d ms\r\n", time_in_ms);
	time_in_ms=0;

#endif
    //aos_post_delayed_action(5000, app_delayed_action, NULL);
    //aos_loop_run();

    return 0;
}