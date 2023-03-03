#pragma once

#define SIZE 1000				//The number of accelerometer readings to hold

int x_accelerometer[SIZE];      //Data from the accelerometer's x-axis
int y_accelerometer[SIZE];      //Data from the accelerometer's y-axis
int z_accelerometer[SIZE];      //Data from the accelerometer's z-axis
int accel_reading = 0;          //Location within the accelerometer data arrays
bool interrupted = false;       //Interrupt flag