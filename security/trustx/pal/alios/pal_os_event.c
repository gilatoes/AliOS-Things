/**
* MIT License
*
* Copyright (c) 2018 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*
*
* \file
*
* \brief This file implements the platform abstraction layer APIs for os event/scheduler.
*
* \ingroup  grPAL
* @{
*/

/**********************************************************************************************************************
 * HEADER FILES
 *********************************************************************************************************************/
#include "pal.h"
#include "pal_os_event.h"
#include "pal.h"
#include "stdio.h"

#include <aos/kernel.h>
#include <aos/aos.h>

#include <hal/soc/timer.h>

#include <k_api.h> //rhino API

/**********************************************************************************************************************
 * MACROS
 *********************************************************************************************************************/
#define MAX_CALLBACKS	1

/*********************************************************************************************************************
 * LOCAL DATA
 *********************************************************************************************************************/
/// @cond hidden 

typedef struct callbacks {
	/// Callback function when timer elapses
	volatile register_callback clb;
	/// Pointer to store upper layer callback context (For example: Ifx i2c context)
	void * clb_ctx;
}pal_os_event_clbs_t;

static pal_os_event_clbs_t clbs[MAX_CALLBACKS]; 

/**
*  Timer callback handler. 
*
*  This get called from the TIMER elapse event.<br>
*  Once the timer expires, the registered callback funtion gets called from the timer event handler, if
*  the call back is not NULL.<br>
*
*\param[in] args Callback argument
*
*/

/// @endcond

//Timer
aos_timer_t callback_timer;

#define TEST_CONFIG_QUEUE_BUF_SIZE              (32)
static aos_queue_t  callback_queue;
static char         queue_buf[TEST_CONFIG_QUEUE_BUF_SIZE];

//Elapsed time handler
static void time_elapsed_handler(void *arg, void *arg2)
{
	int ret =0;
	//printf(">time_elapsed_handler()\r\n");
	pal_os_event_clbs_t clb_params;

	CPSR_ALLOC();

	RHINO_CRITICAL_ENTER(); //No printing in Critical Section

	clb_params.clb = clbs[0].clb;
	clb_params.clb_ctx = clbs[0].clb_ctx;

	if((clb_params.clb==NULL) || (clb_params.clb_ctx==NULL) )
	{
		printf("clb_params is null\r\n");	
		return;
	}

	ret = aos_queue_is_valid(&callback_queue);
	if(ret == 0)
	{
		printf("Invalid queue ret=%d\r\n", ret);
	}else{
		ret = aos_queue_send(&callback_queue, (void *) &clb_params, sizeof(pal_os_event_clbs_t));
		if(ret!=0)
		{
			//printf("failed to send queue: taskname: %s ret=%d\r\n", aos_task_name(), ret);
		}
		aos_timer_free(&callback_timer);
	}

	RHINO_CRITICAL_EXIT();

	//printf("<time_elapsed_handler()\r\n");
}

//Task which receive message from a queue and perform callback after timer is up
static void recv_queue_and_callback_task(void *arg)
{
	pal_os_event_clbs_t clb_params;
	register_callback func = NULL;
	void * func_args = NULL;
	int msg = -1;
	int ret = -1;
	unsigned int size = 0;

	//printf("Start task name: %s\r\n", aos_task_name());

	while(1){
		
		//从queue内收取数据，如没有数据则阻塞当前任务等待
		ret = aos_queue_is_valid(&callback_queue);
		if(ret == 0)
		{
			printf("Invalid queue ret=%d\r\n", ret);
		}
		//printf("?");
		ret = aos_queue_recv(&callback_queue, AOS_WAIT_FOREVER, &clb_params, &size);
		if(ret != 0 )
		{			
			printf("task name: %s error:%d\r\n", aos_task_name(), ret);
			//printf("aos_queue receive error ret=%d size of msg q=%d\r\n", ret, size);				
		}else{			
			//Process the callbacks		
			if (clb_params.clb != NULL){
					//printf("@@\r\n");
					func = clb_params.clb;
					func_args = clb_params.clb_ctx; 
					func((void*)func_args);
			}
		}

		//Clear registered callbacks
		func = NULL;
		clb_params.clb = NULL;		
	}
}
/** 
* Platform specific event init function. 
* <br>
*
* <b>API Details:</b> 
*         This function initialise all required event related variables.<br>
*
*
*/
pal_status_t pal_os_event_init(void)
{
	unsigned int stack_size = 8*(1024);
    int ret = -1;

	int single_shot=0; 

	//printf(">pal_os_event_init()\r\n");
	//printf("Start task name: %s\r\n", aos_task_name());

	//printf("create new callback queue\r\n");
	ret = aos_queue_new(&callback_queue, queue_buf, TEST_CONFIG_QUEUE_BUF_SIZE, TEST_CONFIG_QUEUE_BUF_SIZE);	
	if(ret>0)
	{
		printf("failed to create new queue\r\n");
	}

	//动态创建一个任务，任务句柄不返回，创建完后自动运行； 采用默认优先级AOS_DEFAULT_APP_PRI（32） 受宏RHINO_CONFIG_KOBJ_DYN_ALLOC开关控制
	//printf("create new task\r\n");
    ret = aos_task_new("recv_queue_and_callback_task", recv_queue_and_callback_task, NULL, stack_size);
	if(ret>0)
	{
		printf("failed to create new task\r\n");
	}

	//动态创建软件定时器
#if 0	
	printf("create timer\r\n");
	ret = aos_timer_new(&callback_timer, time_elapsed_handler, NULL, 100, 0);
	
	if(ret != 0)
	{
		printf("Error: Unable to create a new timer error: %d\r\n", ret);
	}
#endif
    //printf("recv_queue_and_callback_task exit!\r\n");

	//printf("<pal_os_event_init()\r\n\n");
	return PAL_STATUS_SUCCESS;
}
/**
* Platform specific event call back registration function to trigger once when timer expires.
* <br>
*
* <b>API Details:</b>
*         This function registers the callback function supplied by the caller.<br>
*         It triggers a timer with the supplied time interval in microseconds.<br>
*         Once the timer expires, the registered callback function gets called.<br>
* 
* \param[in] callback              Callback function pointer
* \param[in] callback_args         Callback arguments
* \param[in] time_us               time in micro seconds to trigger the call back
*
*/
void pal_os_event_register_callback_oneshot(register_callback callback, 
                                            void* callback_args, 
                                            uint32_t time_us)
{
	int ret;
	//printf(">pal_os_event_register_callback_oneshot() time=%d \r\n", time_us);
	//printf("Start task name: %s\r\n", aos_task_name());

	if(time_us < 1000)
		time_us = 1000;

#if 1
	CPSR_ALLOC();

	RHINO_CRITICAL_ENTER();
	//printf("Start timer \r\n");
	ret = aos_timer_new(&callback_timer, time_elapsed_handler, NULL, time_us/1000, 0);	
	if(ret != 0)
	{
		printf("Error: failed to create timer ret=%d \r\n", ret);
	}
	else{
		//printf("Register the callback to global memory \r\n");
		//Register the callback function and arguments
		clbs[0].clb = callback;
    	clbs[0].clb_ctx = callback_args;
	}
	RHINO_CRITICAL_EXIT();
#endif
	//printf("<pal_os_event_register_callback_oneshot()\r\n");

}

/**
* Platform specific task delay function.
* <br>
*
* <b>API Details:</b>
*         This function produces delay only for the. <br>
*         If you don't need this functionality just leave it empty.<br>
*         n this case a compiler should optimize this function out.<br>
* 
* \param[in] time_ms               time in milli seconds to delay
*
*/
void pal_os_event_delayms(uint32_t time_ms)
{
	aos_msleep(time_ms);
}

/**
* @}
*/

