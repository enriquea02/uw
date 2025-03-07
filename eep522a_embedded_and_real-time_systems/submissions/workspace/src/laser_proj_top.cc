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
#include "../lib/laser_proj_top.h"

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

    int     spi_speed   = dac_spi_bus_speed;
    double  ref_voltage = adc_ref_voltage  ;
    int     gain        = dac_gain         ;
    double  voltage     = dac_voltage      ;

    // Setup hardware communication stuff.
    wiringPiSetupPinType(WPI_PIN_BCM);  // setup wiringPi library functions to use BCM-Numbering
    ADCDACPi dac_x(dac_spi_mode,&spi_speed,&ref_voltage,&gain,&voltage);                     // setup DAC information using ABElectronics UK ADC-DAC Pi Library
    ADCDACPi dac_y(dac_spi_mode,&spi_speed,&ref_voltage,&gain,&voltage);
    //                                    | BCM     | Wiringpi  | Physical Pin Number |
    // Control laser diode on BCM pin 23  | GPIO23  | 4         | 16                  |
    pinMode(23,OUTPUT);
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
        dac_x.set_dac_raw(4096-points.store[points.index*3]); // dac_x connected to chip select 0 on /dev/spidev0.0
        dac_y.set_dac_raw(points.store[(points.index*3)+1]);  // dac_y connected to chip select 2 on /dev/spidev1.0 
        
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