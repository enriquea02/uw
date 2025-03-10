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
#include "../lib/motor_drvr_lib.h"

using namespace ABElectronics_CPP_Libraries;

/* -------------------------------------------------------------------------- */
/*                                 Data Types                                 */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */
DacTestVector global_dac_test_vector;
ADCDACPi* dac_x = nullptr;
ADCDACPi* dac_y = nullptr;

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
void onInterrupt(int);

DacTestVector GenerateMcp4921DacTestVector(){
  int test_vector_size    = ((2*4096)/32) + 1;
  int* dac_test_data    = (int*)malloc(test_vector_size * sizeof(int));

  if (dac_test_data == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      DacTestVector error = {NULL, 0}; // Indicate error
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
    if(dac_test_data[index-1] >= 4095){
        dac_test_data[index-1] = 4095;
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

    DacTestVector test_vector = {dac_test_data, test_vector_size};
    return test_vector;
}

// Test code to drive the MCP4921 DAC from 0-4096 (0V-(2*Vref))
int Mcp4921DacTest(){
  // initialize test vector
  DacTestVector dac_test_data = GenerateMcp4921DacTestVector();

  global_dac_test_vector = dac_test_data;

  int* dac_test_vector  = NULL;
  int vector_size       = 0;

  // Verify test vector generated correctly
  if (dac_test_data.data == NULL) {
    printf("Error generating DAC vector.\n");
  } else {
    dac_test_vector = dac_test_data.data;
    vector_size     = dac_test_data.size;
  }

  int     spi_speed   = dac_spi_bus_speed;
  double  ref_voltage = adc_ref_voltage  ;
  int     gain        = dac_gain         ;
  double  voltage     = dac_voltage      ;

  // initialize DAC hardware variables
  dac_x = new ADCDACPi(spi_speed,ref_voltage,gain,voltage);
  dac_y = new ADCDACPi(spi_speed,ref_voltage,gain,voltage);

  // initialize DAC hardware
  if (dac_x->open_dac(dac_device_x) != 1) onInterrupt;
  if (dac_y->open_dac(dac_device_y) != 1) onInterrupt;
  // set the DAC gain to 1 which will give a voltage range of 0 to 2.024V.
  dac_x->set_dac_gain(1);
  dac_y->set_dac_gain(1);

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
      dac_x->set_dac_raw(dac_test_vector[i]); // dac_x connected to chip select 0 on /dev/spidev0.0
      dac_y->set_dac_raw(dac_test_vector[i]);  // dac_y connected to chip select 2 on /dev/spidev1.0 

      usleep(200000);
  }

  return 0;
}

int main() {
  Mcp4921DacTest();
}

// function that is called when program needs to be terminated. 
void onInterrupt(int) {
  printf("Motor program was interrupted.\n");
    if (global_dac_test_vector.data != NULL) {
        free(global_dac_test_vector.data);
    }
    if (dac_x != nullptr) { // Check if dac_x was initialized
      dac_x->close_dac();
      delete dac_x; // Free allocated memory
  }
  if (dac_y != nullptr) { // Check if dac_y was initialized
      dac_y->close_dac();
      delete dac_y; // Free allocated memory
  }
    exit(1); 
}