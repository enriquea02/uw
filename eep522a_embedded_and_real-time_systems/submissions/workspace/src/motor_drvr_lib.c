/*
* MODULE: 
*
* DESCRIPTION: 
* 
*/
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
#include "rpi-lasershow/ABE_ADCDACPi.h"

using namespace ABElectronics_CPP_Libraries;

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
DacTestVector global_dac_test_vector;

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
void onInterrupt(int);

DacTestVector GenerateMcp4921DacTestVector(){
  int test_vector_size    = ((2*4096)/32) + 1
  int* dac_test_data    = (int*)malloc(vector_size * sizeof(int));

  if (dac_test_data == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      IntArray error = {NULL, 0}; // Indicate error
      return error; // Indicate an error
  }

    int current_value = 0;
    int index = 0;

    // Ascending part (0 to 4096)
    while (current_value <= 4096) {
        dac_test_data[index++] = current_value;
        current_value += 32;
    }

    // Correct the last overshoot if any.
    if(dac_test_data[index-1] > 4096){
        dac_test_data[index-1] = 4096;
    }

    current_value -= 32; //start the descending part from 4096.

    // Descending part (4096 to 0)
    while (current_value >= 0) {
        //avoid adding the 4096 again, as it already exists.
        if(current_value != 4096){
            dac_test_data[index++] = current_value;
        }
        current_value -= 32;
    }

    DacTestVector test_vector = {dac_test_data, vector_size}
    return test_vector;
}

// Test code to drive the MCP4921 DAC from 0-4096 (0V-(2*Vref))
int Mcp4921DacTest(){
  // initialize test vector
  DacTestVector dac_test_data = GenerateMcp4921DacTestVector();

  global_dac_test_vector = dac_test_data;

  // Verify test vector generated correctly
  if (dac_test_data.data == NULL) {
    printf("Error generating DAC vector.\n");
  } else {
      int* dac_test_vector = dac_test_data.data;
      int vector_size = dac_test_data.size;
  }

  // initialize DAC hardware variables
  ADCDACPi dac_x(dac_spi_mode,dac_spi_bus_speed,adc_ref_voltage,dac_gain,dac_voltage);
  ADCDACPi dac_y(dac_spi_mode,dac_spi_bus_speed,adc_ref_voltage,dac_gain,dac_voltage);
  // initialize DAC hardware
  if (dac_x.open_dac(dac_device_x) != 1) return(1);
  if (dac_y.open_dac(dac_device_y) != 1) return(1);
  // set the DAC gain to 1 which will give a voltage range of 0 to 2.024V.
  dac_x.set_dac_gain(1);
  dac_y.set_dac_gain(1);

  // subscribe program to exit/interrupt signal.
  signal(SIGINT, onInterrupt);

  int i = 0;

  while(true) {
      printf("DAC Input: %d ", dac_test_vector[i]);
      printf("\n");
      // cycle through DAC test vector which is steps of 32 from 0 to 4095 and back to 0

      i++;

      // reset DAC test loop
      if (i == vector_size) {
        i = 0;
      }

      // move galvos to x,y position. (4096 is to invert horizontally)
      dac_x.set_dac_raw(dac_test_vector[i]); // dac_x connected to chip select 0 on /dev/spidev0.0
      dac_y.set_dac_raw(dac_test_vector[i]);  // dac_y connected to chip select 2 on /dev/spidev0.1 

      usleep(100000);
  }

  return 0;
}

// function that is called when program needs to be terminated. 
void onInterrupt(int) {
  printf("Motor program was interrupted.\n");
    if (global_dac_test_vector.data != NULL) {
        free(global_dac_test_vector.data);
    }
    exit(1); 
}