/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <stdio.h>
#include <hal/soc/soc.h>
//#include <hal/soc/gpio.h>
//#include "driver/gpio.h"
//#include <../../../platform/mcu/esp32/bsp/include/driver/include/driver/gpio.h>

#include "trustx/optiga/include/optiga/Version.h"

#include "pal_gpio.h"

#include "trustx/optiga/include/optiga/pal/pal.h"
#include "trustx/optiga/include/optiga/comms/optiga_comms.h"
#include "trustx/optiga/include/optiga/optiga_util.h"
#include "trustx/optiga/include/optiga/optiga_crypt.h"
#include "trustx/optiga/include/optiga/pal/pal_os_timer.h"
#include "trustx/optiga/include/optiga/pal/pal_os_event.h"
#include "trustx/optiga/include/optiga/ifx_i2c/ifx_i2c.h"

//GPIO 18 is used to control Trust X reset pin
#define GPIO_OUTPUT_IO_OTX_RST    18

gpio_dev_t trustx_reset;

pal_gpio_t* trustx_reset_context;

static int32_t optiga_init(void);
optiga_comms_t optiga_comms = {(void*)&ifx_i2c_context_0, NULL, NULL, 0};
static host_lib_status_t optiga_comms_status;

void init_gpio()
{
    /* gpio port config */
    trustx_reset.port = GPIO_OUTPUT_IO_OTX_RST;
    /* set as output mode */
    trustx_reset.config = OUTPUT_PUSH_PULL;
    /* configure GPIO with the given settings */
    hal_gpio_init(&trustx_reset);
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
			pal_os_timer_delay_in_milliseconds(5);
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

    hal_gpio_output_low(&trustx_reset);
    hal_gpio_output_high(&trustx_reset);
}

int application_start(int argc, char *argv[])
{        
    LOG(">application started()");
    printf("Trust X Library:%s\r\n", VERSION_HOST_LIBRARY);

    optiga_init();

    
   
    aos_post_delayed_action(5000, app_delayed_action, NULL);
    aos_loop_run();

    return 0;
}