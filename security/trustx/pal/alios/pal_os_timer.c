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
* \brief This file implements the platform abstraction layer APIs for timer.
*
* \ingroup  grPAL
* @{
*/

/**********************************************************************************************************************
 * HEADER FILES
 *********************************************************************************************************************/
#include "pal_os_timer.h"
#include "stdio.h"
#include <aos/kernel.h>
#include <aos/aos.h>

#include <k_api.h>

//#include "FreeRTOS.h"
//#include "task.h"
/**********************************************************************************************************************
 * MACROS
 *********************************************************************************************************************/
 
/// @cond hidden 
/*********************************************************************************************************************
 * LOCAL DATA
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL ROUTINES
 *********************************************************************************************************************/


/// @endcond
/**********************************************************************************************************************
 * API IMPLEMENTATION
 *********************************************************************************************************************/

/**
* Get the current time in milliseconds<br>
* 得到任务的当前tick值
*
* \retval  uint32_t time in milliseconds
*/
uint32_t pal_os_timer_get_time_in_milliseconds(void)
{
	return krhino_sys_tick_get();
}

/**
* Waits or delays until the given milliseconds time
* 任务睡眠多少ticks
*
* \param[in] milliseconds Delay value in milliseconds
*
*/
void pal_os_timer_delay_in_milliseconds(uint16_t milliseconds)
{

//AliOS supports 2 methods of timer configuration
#if 1
	aos_msleep(milliseconds);
#else
	if (milliseconds < 10)
		milliseconds = 10;
	
	//milliseconds to tick
	uint64_t dtick = milliseconds * RHINO_CONFIG_TICKS_PER_SECOND / 1000L;
	//krhino_task_sleep(dtick);
	krhino_task_sleep(1);
#endif

    //#TODO:
   // const TickType_t xDelay = pdMS_TO_TICKS(milliseconds);
//	vTaskDelay( xDelay );
}

/**
* @}
*/

