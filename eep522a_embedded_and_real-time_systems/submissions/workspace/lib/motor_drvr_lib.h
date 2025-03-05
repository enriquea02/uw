/*
 * MODULE: 
 *
 * DESCRIPTION: 
 * 
 */

#ifndef _MOTOR_DRVR_H_
#define _MOTOR_DRVR_H_

// Motor Driver Configuration Parameters 
#define adc_device_x        = "/dev/spidev0.0";
#define dac_device_x        = "/dev/spidev0.0";
#define adc_device_y        = "/dev/spidev0.1";
#define dac_device_y        = "/dev/spidev0.1";
#define dac_spi_mode        = "SPI_MODE_0";
#define dac_spi_bus_speed   = 2500000;
#define adc_ref_voltage     = 3.3;
#define dac_gain            = 1;
#define dac_voltage         = 2.042;

/* -------------------------------------------------------------------------- */
/*                                   IMPORTS                                  */
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

/* -------------------------------------------------------------------------- */
/*                                 Data Types                                 */
/* -------------------------------------------------------------------------- */
struct Mcp4921DacTestVector {
    int*  data;
    int   size;
};

typedef struct Mcp4921DacTestVector DacTestVector;

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */
extern DacTestVector global_dac_test_vector; // Declaration

/* -------------------------------------------------------------------------- */
/*                                  CONSTANTS                                 */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
/**
 * @brief Function generates MCP4921 DAC test vectors. 
 * @brief Value starts at 0 and goes to 4095, then back to 0.
 * @brief Each step is 32 until 4095 is reached, vice versa when returning to zero
 * @return A DacTestVector containing the test data and size.
*/
DacTestVector GenerateMcp4921DacTestVector();

/**
* @brief Drive DAC test vectors to the MCP4921 DAC
* @return 0 on success, exits the program on failure.
*/
int Mcp4921DacTest();

#endif // MOTOR_DRVR_LIB_H