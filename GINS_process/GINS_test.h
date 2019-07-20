#ifndef _GINS_TEST_H
#define _GINS_TESTH
#include <string.h>
#include "GINS_process.h"
#include "GINS_process_lib.h"
int GINS_string_decode(char *buff, GINS_raw_t *pRaw);
int Str2Array(const char *str, const char *sep, double *val);
#define MAXVAL		50 // max value for spilting
#define MAXLEN		1024
#define IMU_BW_HZ                  40
/*--------------------------------IMU WALK-----------------------------*/
/*TUpdate(,1)*/
#define GYRO_WALK (0.3*0.05)
#define VEL_WALK (5000)

/*--------------------------------IMU STD-----------------------------*/
#define GYRO_STD_X (0.011*sqrt(IMU_BW_HZ))
#define ACC_STD_X (0.000150*sqrt(IMU_BW_HZ))

#define GYRO_STD_Y GYRO_STD_X
#define GYRO_STD_Z GYRO_STD_X

#define ACC_STD_Y ACC_STD_X
#define ACC_STD_Z ACC_STD_X
#define TEST_RAW_FILE_PATH "D://"
#define OUT_RAW_FILE_PATH "D://"

#endif