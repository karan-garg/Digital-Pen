/*******************************************************

Project : Orbit Lab 8 ATE (ACCEL)
Version : 1.0
Date    : 2/20/2013
Author  : Brian Zufelt / Craig Kief
Company : COSMIAC/UNM
Comments:
This source provides an introduction to the sensing capabilities
for embedded systems. The student will read accelerometer data
and toggle the proper LED to provide tilt measurement


******************************************************
Chip type               : ARM TM4C123GH6PM
Program type            : Firmware
Core Clock frequency	: 80.000000 MHz
*******************************************************/

#define ACCEL_W 0x3A			// Addresses for the accelerometer
#define ACCEL_R 0x3B
#define ACCEL_ADDR 0x1D
#define DELAY 150
// Define needed for pin_map.h
#define PART_TM4C123GH6PM

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint8_t ui8PinData=2;

void Accel_int(); 			// Function prototype to initialize the Accelerometer

signed int Accel_read();	// Function prototype to read the Accelerometer

#define INT_DIGITS 19		/* enough for 64 bit integer */

char *itoa(i)
     int i;
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}


void main(void) {


	signed short int LED_value = 1;

	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  //setup clock

	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);		// Enable I2C hardware
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);	// Enable Pin hardware

	GPIOPinConfigure(GPIO_PB3_I2C0SDA);				// Configure GPIO pin for I2C Data line
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);				// Configure GPIO Pin for I2C clock line

	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);  // Set Pin Type

	// Enable Peripheral ports for output
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);  					//PORTC
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7); 	// LED 1 LED 2

	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5); 			// LED 4

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);  					//PORT D
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_6); 			// LED 3

	//setup the I2C
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);		// SDA MUST BE STD
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);		// SCL MUST BE OPEN DRAIN
	I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false); 							// The False sets the controller to 100kHz communication

	Accel_int();		// Function to initialize the Accelerometer

		SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	while(1){


			// Fill in this section to read data from the Accelerometer and move the LEDs according to the X axis

		LED_value = LED_value + Accel_read(1);

		if(LED_value <= 1){
			// Cycle through the LEDs on the Orbit board
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x40); // LED 1 on LED 2 Off
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00); 			// LED 3 off, Note different PORT
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x00); 			// LED 4 off
			LED_value = 1;	// reset value to maintain range
		}

		else if(LED_value == 2){
			// Cycle through the LEDs on the Orbit board
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x80); // LED 1 off LED 2 on
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00); // LED 3 off, Note different PORT
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x00); // LED 4 on
		}
		else if(LED_value == 3){
			// Cycle through the LEDs on the Orbit board
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x00); // LED 1 off LED 2 off
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x40); // LED 3 on, Note different PORT
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x00); // LED 4 0ff
		}
		else if(LED_value >= 4){
			// Cycle through the LEDs on the Orbit board
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x00); // LED 1 off LED 2 Off
			GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00); // LED 3 off, Note different PORT
			GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x20); // LED 4 on
			LED_value = 4;	// reset value to maintain range
		}

	}
}

void Accel_int(){		// Function to initialize the Accelerometer

	I2CMasterSlaveAddrSet(I2C0_BASE, ACCEL_ADDR, false);  // false means transmit

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);	// Send Start condition

	I2CMasterDataPut(I2C0_BASE, 0x2D); 								// Writing to the Accel control reg
	SysCtlDelay(10*DELAY);														// Delay for first transmission
	I2CMasterDataPut(I2C0_BASE, 0x08);								// Send Value to control Register

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);	// Send Stop condition

	while(I2CMasterBusBusy(I2C0_BASE)){};							// Wait for I2C controller to finish operations

}

signed int Accel_read(int axis) {	//x--> 0, y--> 1, z--> 2// Function to read the Accelerometer
	uint8_t byte1 = 0x32;
	uint8_t byte2 = 0x33;
	if (axis == 0){
		byte1 = 0x32;
		byte2 = 0x33;
	}
	else if(axis == 1){
		byte1 = 0x34;
		byte2 = 0x35;
	}
	else if(axis == 2){
		byte1 = 0x36;
		byte2 = 0x37;
	}

	signed int data;
	signed short value = 0;			// value of x

	unsigned char MSB;
	unsigned char LSB;


	I2CMasterSlaveAddrSet(I2C0_BASE, ACCEL_ADDR, false);  				// false means transmit

	I2CMasterDataPut(I2C0_BASE, byte1);
	SysCtlDelay(DELAY);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);				//Request LSB of X Axis
	SysCtlDelay(1000*DELAY);														// Delay for first transmission

	I2CMasterSlaveAddrSet(I2C0_BASE, ACCEL_ADDR, true);  				// false means transmit

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);			//Request LSB of X Axis
	SysCtlDelay(10*DELAY);

	LSB = I2CMasterDataGet(I2C0_BASE);
	SysCtlDelay(10*DELAY);

	I2CMasterSlaveAddrSet(I2C0_BASE, ACCEL_ADDR, false);  				// false means transmit
	I2CMasterDataPut(I2C0_BASE, byte2);

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);				//Request LSB of X Axis
	SysCtlDelay(1000*DELAY);														// Delay for first transmission

	I2CMasterSlaveAddrSet(I2C0_BASE, ACCEL_ADDR, true);  				// false means transmit

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);			//Request LSB of X Axis
	SysCtlDelay(10*DELAY);

	MSB = I2CMasterDataGet(I2C0_BASE);

	value = (MSB << 8 | LSB);
	fprintf(stderr, "%s%s%s", "X:\t", itoa(value), "\n");

	if(value < -250 ){					// testing axis for value
		data = -1;

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8PinData);
							SysCtlDelay(1000*DELAY);
							GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
							SysCtlDelay(1000*DELAY);
							if(ui8PinData==8) {ui8PinData=2;} else {ui8PinData=ui8PinData*2;}
	}
	else if (value > 250){
		data = 1;

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8PinData);
							SysCtlDelay(1000*DELAY);
							GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
							SysCtlDelay(1000*DELAY);
							if(ui8PinData==8) {ui8PinData=2;} else {ui8PinData=ui8PinData*2;}
	}

	else{
		data = 0;
	}

	SysCtlDelay(10*DELAY);

	return data;						// return value
}
