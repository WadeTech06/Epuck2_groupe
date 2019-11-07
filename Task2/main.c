#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>
#include "camera/camera.h"
#include "camera/dcmi_camera.h"
#include "spi_comm.h"
#include "sensors/VL53L0X/VL53L0X.h"
#include "motors.h"
#include "sensors/proximity.h"

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

//bluetooth
#include "epuck1x/uart/e_uart_char.h"
#include "stdio.h"
#include "serial_comm.h"

const int imgRow = 480;
const int imgCol = 640;

int **Get_Image() {
	int **matrix;
	matrix = malloc(sizeof(int *) * imgRow);

	for (int i = 0; i < (imgRow + imgCol); i++) {
		matrix[i] = malloc(sizeof(int *) * imgCol);
	}
	uint8_t *img_buff_ptr = cam_get_last_image_ptr();
	int count = 0;
	for (int i = 0; i < imgRow; i++) {
		for (int j = 0; j < imgCol; j++) {

			matrix[i][j] = img_buff_ptr[count];
			count++;
		}
	}
	return matrix;
}

// turn toward the most red object. 
void Face_Object() {
	int turnSpeed = 200;
	int **image = Get_Image();

	int row = 0;
	int col = 0;
	int dis_min = 1000000;
	int leftLimit, rightLimit;
	int r, g, b;
	for (int i = 0; i <= imgRow; i++) {
		for (int j = 0; j <= imgCol; j++) {
			r = (int) image[i][j] & 0xF8;
			g = (int) (image[i][j] & 0x07) << 5;
			//| (img[1] & 0xE0) >> 3;
			b = (int) (image[i][j] & 0x1F) << 3;
			int dis = (r - 255) * (r - 255) + g * g + b * b;

			if (dis < dis_min) {
				dis_min = dis;
				row = i;
				col = j;
			}
		}
	}

	leftLimit = imgCol * .4;
	rightLimit = imgCol * .6;
	// turn left toward the object
	if (col < leftLimit) {
		left_motor_set_speed(turnSpeed * -1);
		right_motor_set_speed(turnSpeed);
	}
	// turn right toward the object
	else if (col > rightLimit) {
		left_motor_set_speed(turnSpeed);
		right_motor_set_speed(turnSpeed * -1);
	} else {
		// do nothing it stable
	}
}
// looks at the distance of an object and follow it at a distance. 
void Follow_Object() {
	int driveSpeed = 500;
	if (VL53L0X_get_dist_mm() >= 50 && VL53L0X_get_dist_mm() <= 100) {
		left_motor_set_speed(0);
		right_motor_set_speed(0);
	} else if (VL53L0X_get_dist_mm() > 100) {
		left_motor_set_speed(driveSpeed);
		right_motor_set_speed(driveSpeed);
	} else if (VL53L0X_get_dist_mm() < 50) {
		left_motor_set_speed(driveSpeed * -1);
		right_motor_set_speed(driveSpeed * -1);
	}
}
void Turn_Puck(int sensor, int speed) {
	switch (sensor) {
	// if sensors are on the right side turn right
//	case 0:
	case 1:
		left_motor_set_speed(75);
		right_motor_set_speed(75 * -1);
		chThdSleepMilliseconds(500);
		break;
	case 2:
	case 3:
		left_motor_set_speed(speed);
		right_motor_set_speed(speed * -1);
		chThdSleepMilliseconds(500);
		break;

		// if sensors are on the left side turn left
//
	case 6:
		left_motor_set_speed(-75);
		right_motor_set_speed(75);
		chThdSleepMilliseconds(500);
		break;
	case 4:
	case 5:
//	case 7:
		left_motor_set_speed(speed * -1);
		right_motor_set_speed(speed);
		chThdSleepMilliseconds(500);
		break;
	default:
//		left_motor_set_speed(speed);
//		right_motor_set_speed(speed * -1);
		break;
	}
}

int main(void) {
	halInit();
	chSysInit();
	mpu_init();
	/** Inits the Inter Process Communication bus. */
	messagebus_init(&bus, &bus_lock, &bus_condvar);
	proximity_start();
	calibrate_ir();
	motors_init();
	serial_start();
	VL53L0X_start();

//	dcmi_start();
//	spi_comm_start();
//	cam_start();
//
//	cam_config(FORMAT_COLOR, SIZE_VGA);
//	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
//	dcmi_prepare();
//	chThdSleepMilliseconds(1000);
	int turningSpeed = 350;
	int sensorThreshold = 80;
	char str[100];
	int str_length;
	/* Infinite loop. */
	while (1) {
		// Read camera.
//		spi_comm_suspend();
//		dcmi_capture_start();
//		wait_image_ready();
//		spi_comm_resume();
//		Face_Object();
//		chThdSleepMilliseconds(500);

//TODO: fix turning radius, create acceleration
		for (int i = 0; i <= 7; i++) {

//			// add selector to control debugging logs
			str_length = sprintf(str, "Prox Sensor %d: %d\n", i, get_prox(i));
			e_send_uart1_char(str, str_length);

			if (i == 0 || i == 6) {
				if (get_prox(i) >= 500) {
					Turn_Puck(i, turningSpeed);
				}
			} else if (get_prox(i) >= sensorThreshold) {
				Turn_Puck(i, turningSpeed);
			}

		}
		chThdSleepMilliseconds(300);
		str_length = sprintf(str, "Distance Sensor: %d\n",
				VL53L0X_get_dist_mm());
		e_send_uart1_char(str, str_length);
		Follow_Object();
		chThdSleepMilliseconds(500);
	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
	chSysHalt("Stack smashing detected");
}

