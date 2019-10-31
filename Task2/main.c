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

const int imgRow = 480;
const int imgCol = 640;

int **Get_Image()
{
    int **matrix;
    matrix = malloc(sizeof(int *) * imgRow);

    for (int i = 0; i < (imgRow+imgCol); i++)
    {
        matrix[i] = malloc(sizeof(int *) * imgCol);
    }

    int count = 0;
    for (int i = 0; i < imgRow; i++)
    {
        for (int j = 0; j < imgCol; j++)
        {
            uint8_t *img_buff_ptr = cam_get_last_image_ptr();
            matrix[i][j] = img_buff_ptr[count];
            count++;
        }
    }
    return matrix;
}

// turn toward the most red object. 
void Face_Object()
{
    int turnSpeed = 200;
    int **image = Get_Image();

    int row = 0;
    int col = 0;
    int dis_min = 1000000;
    int leftLimit, rightLimit;
    int r,g,b;
    for (int i = 0; i <= imgRow; i++)
    {
        for (int j = 0; j <= imgCol; j++)
        {
            r = (int)image[i][j] & 0xF8;
            g = (int)(image[i][j] & 0x07) << 5;
            //| (img[1] & 0xE0) >> 3;
            b = (int)(image[i][j] & 0x1F) << 3;
            int dis = (r - 255) * (r - 255) + g * g + b * b;

            if (dis < dis_min)
            {
                dis_min = dis;
                row = i;
                col = j;
            }
        }
    }

    leftLimit = imgCol * .4;
    rightLimit = imgCol * .6;
    // turn left toward the object
    if (col < leftLimit)
    {
        left_motor_set_speed(turnSpeed * -1);
        right_motor_set_speed(turnSpeed);
    }
    // turn right toward the object
    else if (col > rightLimit)
    {
        left_motor_set_speed(turnSpeed);
        right_motor_set_speed(turnSpeed * -1);
    }
    else
    {
        // do nothing it stable
    }
}
// looks at the distance of an object and follow it at a distance. 
void Follow_Object()
{
    int driveSpeed = 500;
    if (VL53L0X_get_dist_mm() >= 46 && VL53L0X_get_dist_mm() <= 54)
    {
        left_motor_set_speed(driveSpeed * -1);
        right_motor_set_speed(driveSpeed * -1);
    }
    else if (VL53L0X_get_dist_mm() > 54)
    {
        left_motor_set_speed(driveSpeed);
        right_motor_set_speed(driveSpeed);
    }
    else if(VL53L0X_get_dist_mm() < 46)
    {
        left_motor_set_speed(0);
        right_motor_set_speed(0);
    }
}

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();
    VL53L0X_start();
//    cam_start();
//    cam_config(FORMAT_COLOR, SIZE_VGA);
//    dcmi_start();
//    spi_comm_start();
//    dcmi_prepare();
//    dcmi_set_capture_mode(CAPTURE_CONTINUOUS);
//    dcmi_capture_start();

    motors_init();

    /* Infinite loop. */
    while (1)
    {
        // Read camera.


//        Face_Object();
//        chThdSleepMilliseconds(500);
        Follow_Object();
        chThdSleepMilliseconds(500);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
    uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

    void __stack_chk_fail(void)
    {
        chSysHalt("Stack smashing detected");
    }

