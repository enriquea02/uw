/*
 * MODULE: laser_proj_top.h
 *
 * DESCRIPTION: Header file for laser_proj_top.cc.
 * */

#ifndef LASER_PROJ_TOP_H
#define LASER_PROJ_TOP_H

/* -------------------------------------------------------------------------- */
/* IMPORTS                                  */
/* -------------------------------------------------------------------------- */

/* --------------------------- standard libraries --------------------------- */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <wiringPi.h>

/* ----------------------------- local libraries ---------------------------- */
#include "../src/rpi-lasershow/ABE_ADCDACPi.h"
#include "../src/rpi-lasershow/Points.h"
#include "../src/rpi-lasershow/IldaReader.h"
#include "./motor_drvr_lib.h"

/* -------------------------------------------------------------------------- */
/* Data Types                                 */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Globals                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Routines                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Function to handle interrupt signals.
 * @param The signal number.
 */
void onInterrupt(int signal);

#endif // LASER_PROJ_TOP_H