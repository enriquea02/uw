/*
================================================
ABElectronics UK ADC-DAC Pi
Version 1.0 Created 23/06/2017
================================================

*/

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "ABE_ADCDACPi.h"

using namespace ABElectronics_CPP_Libraries;



ADCDACPi::ADCDACPi(
  const int    spi_bus_speed   ,
  const double adc_ref_voltage ,
  const int    dac_gain        ,
  const double dac_voltage
){
  mode 	        = SPI_MODE_0;        // SPI_MODE_0
  speed         = spi_bus_speed;   // SPI bus speed

  adctx[0] 		  = 0x01; 		        // transmit buffer for the ADC
  adctx[1] 		  = 0x80;
  adctx[2] 		  = 0x00;
  
  adcrefvoltage = adc_ref_voltage; // reference voltage for the ADC chip.
  dacgain 		  = dac_gain;        // gain setting for the DAC chip.
  dacvoltage    = dac_voltage;     // maximum voltage for the DAC output
}

int ADCDACPi::open_adc(const char* adc_device_path) {
  /**
  * Open the ADC SPI bus channel
  * This needs to be called before using the DAC
  */

  // Open SPI device
  if ((adc = open(adc_device_path, O_RDWR)) < 0)
    return (0);

  if (ioctl(adc, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1)
    return (0);
  // Set SPI mode
  if (ioctl(adc, SPI_IOC_WR_MODE, &mode) == -1)
    return (0);

  return (1);
}

void ADCDACPi::close_adc() {
  /**
  * Close the ADC SPI bus channel
  */
  close(adc);
}

int ADCDACPi::open_dac(const char* dac_device_path) {
  /**
  * Open the DAC SPI bus channel
  * This needs to be called before using the DAC
  */

  // Open SPI device
  if ((dac = open(dac_device_path, O_RDWR)) < 0)
    return (0);

  if (ioctl(dac, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1)
    return (0);
  // Set SPI mode
  if (ioctl(dac, SPI_IOC_WR_MODE, &mode) == -1)
    return (0);

  return (1);
}

void ADCDACPi::close_dac() {
  /**
  * Close the DAC SPI bus channel
  */
  close(dac);
}

double ADCDACPi::read_adc_voltage(int channel, int mode) {
  /**
  * Read the voltage from the ADC
  * @param channel - 1 or 2
  * @param mode - 0 = Single Ended or 1 = Differential
  * When in differential mode setting channel to 1 will make IN1 = IN+ and IN2 = IN-
  * When in differential mode setting channel to 2 will make IN1 = IN- and IN2 = IN+
  * @returns between 0V and the reference voltage
  */
  
  int rawval = read_adc_raw(channel, mode);
  return ((adcrefvoltage / 4096) * (double) rawval);
}



int ADCDACPi::read_adc_raw(int channel, int mode) {
  /**
  * Read the raw value from the ADC
  * @param channel -  1 to 8
  * @param mode -  0 = Single Ended or 1 = Differential
  * When in differential mode setting channel to 1 will make IN1 = IN+ and IN2 = IN-
  * When in differential mode setting channel to 2 will make IN1 = IN- and IN2 = IN+
  * @returns 12 bit value between 0 and 4096
  */
  
  if (channel == 1) {
    if (mode == 0) {
      adctx[1] = 0x80;
    } else if (mode == 1) {
      adctx[1] = 0x00;
    } else {
      return (0);
    }
  } else if (channel == 2) {
    if (mode == 0) {
      adctx[1] = 0xC0;
    } else if (mode == 1) {
      adctx[1] = 0x40;
    } else {
      return (0);
    }
  } else {
    return (0);
  }

  struct spi_ioc_transfer spi;
  memset(&spi,0,sizeof(spi));

  spi.tx_buf = (unsigned long)adctx;	
  spi.rx_buf = (unsigned long)adcrx;
  spi.len = 3;
  spi.speed_hz = 1800000; // 1.8MHz Clock Speed
  spi.delay_usecs = 0;		
  spi.bits_per_word = 8;


  ioctl(adc, SPI_IOC_MESSAGE(1), &spi);

  return (((adcrx[1] & 0x0F) << 8) + (adcrx[2]));

}

void ADCDACPi::set_adc_refvoltage(double ref) {
  /**
  * Set the reference voltage for the adc
  * @param ref - Set this value to be the same as the voltage measured on the 3.3V GPIO pin
  */
  adcrefvoltage = ref;
}

void ADCDACPi::set_dac_voltage(double voltage, int channel) {
  /**
  * Set the DAC voltage
  * @param voltage - between 0 and 2.048 when gain is set to 1,  0 and 3.3 when gain is set to 2
  * @param channel - 1 or 2
  */
  if (channel < 1 && channel > 2) {
    throw std::out_of_range("set_dac_voltage channel out of range: 1 or 2");
  } 		

  if ((voltage >= 0.0) && (voltage < dacvoltage)) {
    uint16_t rawval = ((voltage / 2.048) * 4096) / dacgain;
    set_dac_raw(rawval);
  } else {
    throw std::out_of_range("set_dac_voltage voltage out of range");
  }
}

void ADCDACPi::set_dac_raw(uint16_t raw) {
  /**
  * Set the raw value from the selected channel on the DAC
  * @param raw - between 0 and 4095
  * @param channel - 1 or 2
  */
  
  // MCP4921 Datasheet - 16-bit DAC write command
  // Upper Bit  | Lower Bit | Description
  // 15         | 15        | Don't Care
  // 14         | 14        | Don't Care (LDAC pulled down)
  // 13         | 13        | 1, set gain=1
  // 12         | 12        | 1, keep device active
  // 11         | 0         | 12-bit Data
  dactx[1] = (raw & 0xff);
  dactx[0] = (((raw >> 8) & 0xff) | 0x1 << 5 | 1 << 4);

  if (dacgain == 2) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsequence-point"
        dactx[0] = (dactx[0] &= ~(1 << 5));
    #pragma GCC diagnostic pop
    }

  struct spi_ioc_transfer tr;
    memset(&tr,0,sizeof(tr));

  tr.tx_buf = (unsigned long)&dactx;
  tr.rx_buf = (unsigned long)NULL;
  tr.len = 2;
  tr.speed_hz = 20000000; // 20MHz clock speed
  tr.delay_usecs = 0;		
  tr.bits_per_word = 8;
  tr.cs_change = 0;

  // Write data
  ioctl(dac, SPI_IOC_MESSAGE(1), &tr);
  return;
    
}



void ADCDACPi::set_dac_gain(int gain) {
  /**
  * Set the DAC gain
  * @param gain - 1 or 2 - The output voltage will be between 0 and 2.048V when gain is set to 1,  0 and 3.3V when gain is set to 2	
  */
  if (gain == 1) {
    dacgain = 1;
    dacvoltage = 2.048;
  }
  if (gain == 2) {
    dacgain = 2;
    dacvoltage = 3.3;
  }

}
