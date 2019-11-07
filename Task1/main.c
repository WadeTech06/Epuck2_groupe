#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>

#include "motors.h"
#include "sensors/proximity.h"

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

//bluetooth
#include "epuck1x/uart/e_uart_char.h"
#include "stdio.h"
#include "serial_comm.h"

void Set_Speed(int speed) {
	if (speed >= -1000 && speed <= 1000) {
		left_motor_set_speed(speed);
		right_motor_set_speed(speed);
	}
}

void Turn_Puck(int sensor, int speed) {
	switch (sensor) {
	// if sensors are on the right side turn left
	case 0:
	case 1:
	case 2:
		left_motor_set_speed(speed * -1);
		right_motor_set_speed(speed);
		chThdSleepMilliseconds(500);
		break;

		// if sensors are on the left side turn right
	case 5:
	case 6:
	case 7:
		left_motor_set_speed(speed);
		right_motor_set_speed(speed * -1);
		chThdSleepMilliseconds(500);
		break;
	default:
		left_motor_set_speed(speed);
		right_motor_set_speed(speed * -1);
		break;
	}
}

void Fail_Safe(int speed) {
	left_motor_set_speed(speed * -1);
	right_motor_set_speed(speed);

	do {

		chThdSleepMilliseconds(500);
	} while (get_prox(0) != 0 && get_prox(7) != 0);

}
//TODO:
// gradually increase Initial speed when close to a wall
// handling narrow rooms
// handle dead ends

int main(void) {

	halInit();
	chSysInit();
	mpu_init();
	motors_init();
	messagebus_init(&bus, &bus_lock, &bus_condvar);
	proximity_start();
	calibrate_ir();

	//bluetooth
	serial_start();

	int forwardSpeed = 500;
	int turningSpeed = 350;
	//TODO set threshold
	int sensorThreshold = 75;
	int failsafecount = 0;

	Set_Speed(forwardSpeed);

	char str[100];
	int str_length;

	/* Infinite loop. */
	while (1) {

		// checks for walls; if a wall is near turn opposite direction
		for (int i = 0; i <= 7; i++) {

		// add selector to control debugging logs
			str_length = sprintf(str, "fail count: %d\n",failsafecount );
			e_send_uart1_char(str, str_length);

			if (get_prox(i) >= sensorThreshold) {
				//Set_Speed(0);
				//chThdSleepMilliseconds(500);
				Turn_Puck(i, turningSpeed);
				failsafecount++;
			} else {
				if (left_motor_get_desired_speed() != forwardSpeed
						&& right_motor_get_desired_speed() != forwardSpeed) {
					Set_Speed(forwardSpeed);
					failsafecount = 0;
				}
			}
		}

		//waits 1 second
		chThdSleepMilliseconds(500);
		if (failsafecount == 6) {
			Fail_Safe(turningSpeed);
			failsafecount = 0;
			Set_Speed(forwardSpeed);
		}
	}
}

#define STACK_CHK_GUARD 0xe2dee396
	uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

	void __stack_chk_fail(void) {
		chSysHalt("Stack smashing detected");
	}
