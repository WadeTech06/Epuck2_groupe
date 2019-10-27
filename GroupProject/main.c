#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>

#include "leds.h"
#include "spi_comm.h"
#include "motors.h"
#include "selector.h"

#include "sensors/proximity.h"
messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

void Toggle_Led(int sensor, int toggle)
{
	switch (sensor)
	{
	case 0:
		set_led(LED1, toggle);
		//TODOt test RGB sensors
		//
		break;
	case 1:
		toggle_rgb_led(LED2, RED_LED,100);
		break;
	case 2:
		set_led(LED3, toggle);
		break;
	case 3:
		toggle_rgb_led(LED4, RED_LED,100);
		break;
	case 4:
		set_led(LED5, toggle);
		break;
	case 5:
		toggle_rgb_led(LED6, RED_LED,100);
		break;
	case 6:
		set_led(LED7, toggle);
		break;
	case 7:
		toggle_rgb_led(LED8, RED_LED,100);
		break;
	default:
		break;
	}
}

int main(void)
{

	halInit();
	chSysInit();
	mpu_init();

	clear_leds();
	spi_comm_start();

	// Op Task 1
	//    /* Infinite loop. */
	//    while (1) {
	//    	//set_led(LED1, 1);
	//    	//set_led(LED3, 1);
	//    	//waits 1 second
	//        chThdSleepMilliseconds(1000);
	//        //set_led(LED1, 0);
	//        //set_led(LED3, 0);
	//        chThdSleepMilliseconds(1000);
	//    }

	// Op Task 2
	/* 	motors_init();
	set_body_led(2);
	int count = 0;
	int selector = 0;
	int leftSpeed = -900, rightSpeed = 900;

	left_motor_set_speed(leftSpeed);
	right_motor_set_speed(rightSpeed);
	while (1)
	{
		if (selector != get_selector())
		{
			selector = get_selector();
			count = 0
		}
		else if (count == selector && count != 0)
		{
			if (math.abs(left_motor_get_desired_speed()) == leftSpeed)
				leftSpeed * -1;
			else
				math.abs(leftSpeed);

			if (math.abs(right_motor_get_desired_speed()) == rightSpeed)
				rightSpeed * -1;
			else
				math.abs(rightSpeed)
		}
		else
		{
			count++;
		}

		chThdSleepMilliseconds(1000);
	} */

	// Op Task 3
	messagebus_init(&bus, &bus_lock, &bus_condvar);
	proximity_start();
	calibrate_ir();
	clear_leds();
	spi_comm_start();
	//TODO setting RGB values needed?
	// for(int i=0; i<=4; i++)
	// 	set_rgb_led(i,255,0,0);
	//TODO define threshold
	int threshold = 0;
	while (1)
	{
		for (int i = 0; i <= 7; i++)
		{
			if (get_calibrated_prox(i) >= threshold)
				Toggle_Led(i,1);
			else
			{
				Toggle_Led(i,0);
			}
		}
		chThdSleepMilliseconds(1000);
	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
	chSysHalt("Stack smashing detected");
}
