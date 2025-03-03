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
#include "rpi-lasershocw/ABE_ADCDACPI.h"
#include "Points.h"
#include "IldaReader.h"
#include "motor_drvr_lib.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

/* -------------------------------------------------------------------------- */
/*                                 Data Types                                 */
/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */
void onInterrupt(int);

int SetupPin(
  int pin,  // BCM Pin Number
  int mode  // Modes: INPUT, OUTPUT, PWM_OUTPUT, PWM_MS_OUTPUT, PWM_BAL_OUTPUT, GPIO_CLOCK, PM_OFF
) {
  // Setup BCM pin with desired mode
  int pin_setup_result = pinMode(pin, mode);

  // wiringPi's pinMode function has detected and invalid parameter error
  if (pin_setup_result == -1) {
    fprintf(stderr, "Error: wiringPiSetupPinType failed for BCM GPIO %d with mode %d. \n",pin,mode);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {

    // Validate arguments.
    if (argc < 3) {
    cout << "ERROR: Arguments missing." << endl;
    cout << "Required: [pointDelay] [fileName]" << endl;
      return 1;
  }
    int result;

    // Read arguments.
    int pointDelay = atoi(argv[1]);
    string fileName = argv[2];
    double frameDuration = 0.042; // ~24fps (1/24=0.04167..).

    // Setup hardware communication stuff.
    wiringPiSetupPinType(WPI_PIN_BCM);  // setup wiringPi library functions to use BCM-Numbering
    ADCDACPi dac_x(dac_spi_mode,dac_spi_bus_speed,adc_ref_voltage,dac_gain,dac_voltage);                     // setup DAC information using ABElectronics UK ADC-DAC Pi Library
    ADCDACPi dac_y(dac_spi_mode,dac_spi_bus_speed,adc_ref_voltage,dac_gain,dac_voltage);
    //                                    | BCM     | Wiringpi  | Physical Pin Number |
    // Control laser diode on BCM pin 23  | GPIO23  | 4         | 16                  |
    SetupPin(23,OUTPUT);
    // Open both DAC SPI interfaces.
    if (dac_x.open_dac(dac_device_x) != 1) return(1);
    if (dac_y.open_dac(dac_device_y) != 1) return(1);
    // Set the DAC gain to 1 which will give a voltage range of 0 to 2.024V.
    dac_x.set_dac_gain(1);
    dac_y.set_dac_gain(1);

    // Setup ILDA reader.
    Points points;
    IldaReader ildaReader;
    if (ildaReader.readFile(fileName)) {
        printf("Provided file is a valid ILDA file.\n");
        ildaReader.getNextFrame(&points);
        printf("Points loaded in first frame: %d\n", points.size);
    } else {
        printf("Error opening ILDA file.\n");
        return(1);
    }

    // Subscribe program to exit/interrupt signal.
    signal(SIGINT, onInterrupt);

    // Start the scanner loop with the current time.
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    while(true) {

        // Exit if no points found.
        if (points.size == 0) break;

        // Move galvos to x,y position. (4096 is to invert horizontally)
        dac_x.set_dac_raw(4096-points.store[points.index*3],0); // dac_x connected to chip select 0 on /dev/spidev0.0
        dac_y.set_dac_raw(points.store[(points.index*3)+1],2);  // dac_y connected to chip select 2 on /dev/spidev0.1 
        
        // Turn on/off laser diode.
        if (points.store[(points.index*3)+2] == 1) digitalWrite(0, HIGH); 
        else digitalWrite(0, LOW); 

        // Maybe wait a while there.
        if (pointDelay > 0) usleep(pointDelay);

        // In case there's no more points in the current frame check if it's time to load next frame.
        if (!points.next()) {
            std::chrono::duration<double> elapsedSeconds = std::chrono::system_clock::now() - start;
            if(elapsedSeconds.count() > frameDuration) {
                start = std::chrono::system_clock::now();
                digitalWrite(0, LOW);
                ildaReader.getNextFrame(&points);
            }
        }
    }

    // Cleanup and exit.
    ildaReader.closeFile();
    dac_x.close_dac();
    dac_y.close_dac();
    return (0);
}

// Function that is called when program needs to be terminated. 
void onInterrupt(int) {
    printf("Turn off laser diode.\n");
    digitalWrite(0, LOW);
    printf("Program was interrupted.\n");
    exit(1); 
}