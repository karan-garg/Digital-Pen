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
#define DELAY 110
// Define needed for pin_map.h
#define PART_TM4C123GH6PM
#include <time.h>
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
#include <stdlib.h>

#include "delay.h"
#include "FillPat.h"
#include "I2CEEPROM.h"
#include "LaunchPad.h"
#include "OrbitBoosterPackDefs.h"
#include "OrbitOled.h"
#include "OrbitOledChar.h"
#include "OrbitOledGrph.h"


char	chSwtCur;
char	chSwtPrev;
bool	fClearOled;
long 			lBtn1;
long 			lBtn2;


uint32_t	ulAIN0;
long 			lBtn1;
long 			lBtn2;
char			szAIN[6] = {
  0            };
char			cMSB = 0x00;
char			cMIDB = 0x00;
char			cLSB = 0x00;

char szAnalog[] = {
  'A', 'n', 'a', 'l', 'o', 'g', ':', ' ', '\0'            };
char szDemo1[]	= {
  'O', 'r', 'b', 'i', 't', ' ', 'D', 'e', 'm', 'o', '!', '\0'            };
char szDemo2[]	= {
  'B', 'y', ' ', 'D', 'i', 'g', 'i', 'l', 'e', 'n', 't', '\0'            };

char line[1000][2] = {0};
int lineSize = 0;

float xc = 3;
float yc = 28;
#define DEMO_0		0
#define DEMO_1		2
#define DEMO_2		1
#define DEMO_3		3

int penDown = 1;
int freehand = 0;
float velX = 0;
float velY = 0;
int undo = 0;

char checkSwitches();
void drawArrow(float velX, float velY, double angle);
double getAngle();

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint8_t ui8PinData=2;

void Accel_int(); 			// Function prototype to initialize the Accelerometer

int Accel_read();	// Function prototype to read the Accelerometer

//#define INT_DIGITS 19		/* enough for 64 bit integer */
//
//char *itoa(i)
//     int i;
//{
//  /* Room for INT_DIGITS digits, - and '\0' */
//  static char buf[INT_DIGITS + 2];
//  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
//  if (i >= 0) {
//    do {
//      *--p = '0' + (i % 10);
//      i /= 10;
//    } while (i != 0);
//    return p;
//  }
//  else {			/* i < 0 */
//    do {
//      *--p = '0' - (i % 10);
//      i /= 10;
//    } while (i != 0);
//    *--p = '-';
//  }
//  return p;
//}


void main(void) {

	line[0][0] = xc;
	line[0][1] = yc;

	lineSize++;

	  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA );
	  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOB );
	  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOC );
	  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOD );
	  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOE );
	  SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOF );
	  /*
	   * Pad Configure.. Setting as per the Button Pullups on
	   * the Launch pad (active low).. changing to pulldowns for Orbit
	   */
	  GPIOPadConfigSet(SWTPort, SWT1 | SWT2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

	  GPIOPadConfigSet(BTN1Port, BTN1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
	  GPIOPadConfigSet(BTN2Port, BTN2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

	  GPIOPadConfigSet(LED1Port, LED1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	  GPIOPadConfigSet(LED2Port, LED2, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	  GPIOPadConfigSet(LED3Port, LED3, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	  GPIOPadConfigSet(LED4Port, LED4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);

	  /*
	   * Initialize Switches as Input
	   */
	  GPIOPinTypeGPIOInput(SWTPort, SWT1 | SWT2);

	  /*
	   * Initialize Buttons as Input
	   */
	  GPIOPinTypeGPIOInput(BTN1Port, BTN1);
	  GPIOPinTypeGPIOInput(BTN2Port, BTN2);

	  /*
	   * Initialize LEDs as Output
	   */
	  GPIOPinTypeGPIOOutput(LED1Port, LED1);
	  GPIOPinTypeGPIOOutput(LED2Port, LED2);
	  GPIOPinTypeGPIOOutput(LED3Port, LED3);
	  GPIOPinTypeGPIOOutput(LED4Port, LED4);

	  /*
	   * Enable ADC Periph
	   */
	  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	  GPIOPinTypeADC(AINPort, AIN);

	  /*
	   * Enable ADC with this Sequence
	   * 1. ADCSequenceConfigure()
	   * 2. ADCSequenceStepConfigure()
	   * 3. ADCSequenceEnable()
	   * 4. ADCProcessorTrigger();
	   * 5. Wait for sample sequence ADCIntStatus();
	   * 6. Read From ADC
	   */
	  ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	  ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
	  ADCSequenceEnable(ADC0_BASE, 0);

	  /*
	   * Initialize the OLED
	   */
	  OrbitOledInit();

	  /*
	   * Reset flags
	   */
	  chSwtCur = 0;
	  chSwtPrev = 0;
	  fClearOled = true;







	int accelX = 0, accelY = 0;
//	double velX = 0, velY = 0, posX = 0, posY = 0;

	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  //setup clock

	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);		// Enable I2C hardware
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);	// Enable Pin hardware

	GPIOPinConfigure(GPIO_PB3_I2C0SDA);				// Configure GPIO pin for I2C Data line
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);				// Configure GPIO Pin for I2C clock line

	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);  // Set Pin Type

	//setup the I2C
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);		// SDA MUST BE STD
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);		// SCL MUST BE OPEN DRAIN
	I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false); 							// The False sets the controller to 100kHz communication

	Accel_int();		// Function to initialize the Accelerometer
//	static int aX[50] = {0};
//	static int vX[50] = {0};
////	static int pX[30] = {0};
//	static int aY[50] = {0};
//	static int vY[50] = {0};
//	static int pY[30] = {0};

//	static int pointX[30] = {0};
//	int endX = 0, endY = 0;
	int i = 0;
//	int xIndex = 0, yIndex = 0;
	const int X_LOWER_THRESH = 20;
	const int X_UPPER_THRESH = 200;
	const int Y_LOWER_THRESH = 10;
	const int Y_UPPER_THRESH = 200;

//	int xStop = -5, yStop = -5;
//	int turtleSpeedX = 0, turtleSpeedY = 0;
	double angle = 0;
	double axisAngle = 0, velX = 0, velY = 0;
//	int yStopped = 0, xStopped = 0;
//	int xisMoving = 0, yisMoving = 0;

	lBtn1 = GPIOPinRead(BTN1Port, BTN1);

	if ((checkSwitches() == DEMO_3 || checkSwitches() == DEMO_2 ) &&  !(lBtn1 == BTN1) ){
		line[0][0] = 0;
			line[0][1] = 0;

			lineSize--;
	}

	if(checkSwitches() == DEMO_3 || checkSwitches() == DEMO_1)freehand = 1;
			else freehand = 0;
	while(1){
		penDown = 1;
		// fprintf(stderr,"%s\n",itoa((int)getAngle()));

		//fprintf(stderr, "\n%s\n", itoa((int)checkSwitches()));
		  /*
		   * Initiate ADC Conversion and update the OLED
		   */
		  ADCProcessorTrigger(ADC0_BASE, 0);

		  while(!ADCIntStatus(ADC0_BASE, 0, false));

		  ADCSequenceDataGet(ADC0_BASE, 0, &ulAIN0);

//		  /*
//		   * Process data
//		   */
//		  cMSB = (0xF00 & ulAIN0) >> 8;
//		  cMIDB = (0x0F0 & ulAIN0) >> 4;
//		  cLSB = (0x00F & ulAIN0);
//
//
//		  szAIN[0] = '0';
//		  szAIN[1] = 'x';
//		  szAIN[2] = (cMSB > 9) ? 'A' + (cMSB - 10) : '0' + cMSB;
//		  szAIN[3] = (cMIDB > 9) ? 'A' + (cMIDB - 10) : '0' + cMIDB;
//		  szAIN[4] = (cLSB > 9) ? 'A' + (cLSB - 10) : '0' + cLSB;
//		  szAIN[5] = '\0';
//
//		  /*
//		   * Update the Reading
//		   */
////		  OrbitOledSetCursor(8, 4);
//		  OrbitOledPutString(szAIN);





//		  lBtn2 = GPIOPinRead(BTN2Port, BTN2);
//
		 //			  OrbitOledClear();
//			      OrbitOledMoveTo(0,0);
//			      OrbitOledSetCursor(0,0);
//		    GPIOPinWrite(LED1Port, LED1, LED1);
//		    GPIOPinWrite(LED2Port, LED2, LED2);
//		  }
//		  else {
//		    GPIOPinWrite(LED1Port, LED1, LOW);
//		    GPIOPinWrite(LED2Port, LED2, LOW);
//		  }
//		  if(lBtn2 == BTN2) {
//		    GPIOPinWrite(LED3Port, LED3, LED3);
//		    GPIOPinWrite(LED4Port, LED4, LED4);
//		  }
//		  else {
//		    GPIOPinWrite(LED3Port, LED3, LOW);
//		    GPIOPinWrite(LED4Port, LED4, LOW);
//		  }

//
//
//
//
//
//
		accelX = Accel_read(0);
		accelY = Accel_read(1);
//
		if (fabs(accelX) > X_LOWER_THRESH && fabs(accelX) < X_UPPER_THRESH) {

			if(accelX < 0) velX = 1;
			else velX = -1;

			//aX[xIndex] = accelX;

//			if (xIndex > 0){
//				if((accelX < 0 != oax < 0) && xisMoving){
//					xStop = xIndex;
//					xStopped = 1;
////					accel = aX[xIndex-1];
////					pointX[pointIndex] = pointX[pointIndex-1] + (i-time)*(int)((double)accel/fabs(accel));
////					pointIndex++;
////					time = i;
////
////					turtleSpeedX = 0;
//				}
//				else if (xIndex > (xStop + 5)){
//					turtleSpeedX = oax;
//					xisMoving = 1;
//				}
//			}
//			xIndex++;
//			oax = accelX;
//
		}else velX = 0;

//
		if (fabs(accelY) > Y_LOWER_THRESH && fabs(accelY) < Y_UPPER_THRESH) {
					//aY[yIndex] = accelY;
					if (accelY < 0) velY = -1;
					else velY = 1;
//					if (yIndex > 0){
//						if(((accelY < 0) != (oay < 0)) && yisMoving){
//							yStop = yIndex;
//							yStopped = 1;
//
//			//			turtleSpeedY = 0;
//
//						}
//						else if (yIndex > (yStop + 5)){
//							turtleSpeedY = oay;
//							yisMoving = 1;
//						}
//				}
//					yIndex++;
//					oay = accelY;

		}else velY = 0;

		if(checkSwitches() == DEMO_3 || checkSwitches() == DEMO_1){
			GPIOPinWrite(LED1Port, LED1, LOW);
			GPIOPinWrite(LED2Port, LED2, LOW);
			freehand = 1;
		}
		else {
			freehand = 0;
			GPIOPinWrite(LED1Port, LED1, LED1);
			GPIOPinWrite(LED2Port, LED2, LED2);
		}

		angle = getAngle();
		axisAngle = angle;

		if (!freehand){
			if(axisAngle > 3.14) axisAngle -= 3.14;
	//		if (fabs(axisAngle) < 0.06 || fabs(axisAngle - 3.14) < 0.06) axisAngle = 0;
	//		if (fabs(axisAngle - 1.57) < 0.06) axisAngle = 1.57079632679;

			if (axisAngle <= 0.785){
				velX = -velY*fabs(sin(axisAngle));
				velY = velY*fabs(cos(axisAngle));
			}else if (axisAngle > 0.785 && axisAngle <= 1.57){
				velX = velX*fabs(sin(axisAngle));
				velY = -velX*fabs(cos(axisAngle));
			}else if(axisAngle > 1.57 && axisAngle <= 2.355){
				velX = velX*fabs(sin(axisAngle));
				velY = velX*fabs(cos(axisAngle));
			}

			else if(axisAngle > 2.355){
				velX = velY*fabs(sin(axisAngle));
				velY = velY*fabs(cos(axisAngle));

			}
		}

//		if (yIndex >= yStop && yStopped){
//			yStopped = 0;
//			velY = 0;
//			yisMoving = 0;
//		}
//		if (xIndex >= xStop && xStopped){
//			xStopped = 0;
//			velX = 0;
//			xisMoving = 0;
//		}

		lBtn1 = GPIOPinRead(BTN1Port, BTN1);
		lBtn2 = GPIOPinRead(BTN2Port, BTN2);
		if (lBtn2 == BTN2){
			undo = 1;
		}else{
			undo = 0;
		}

		if ((checkSwitches() == DEMO_3 || checkSwitches() == DEMO_2 ) &&  !(lBtn1 == BTN1) ){
//			 GPIOPinWrite(LED1Port, LED1, LOW);
//			GPIOPinWrite(LED2Port, LED2, LOW);
			GPIOPinWrite(LED3Port, LED3, LOW);
			    GPIOPinWrite(LED4Port, LED4, LOW);
					    penDown = 0;
		}else{
//			 GPIOPinWrite(LED1Port, LED1, LED1);
//					    GPIOPinWrite(LED2Port, LED2, LED2);
					    GPIOPinWrite(LED3Port, LED3, LED3);
					    		    GPIOPinWrite(LED4Port, LED4, LED4);
		}
		drawArrow(velX, velY, angle);
//
//
//
//

//		if (i != 0) {
//			aX0 = aX[i-1];
//			aY0 = aY[i-1];
//		}

//		aX[i] = accelX;
//		aY[i] = accelY;



//		if (fabs(accelX) > 20 && fabs(accelX) < 220 && fabs(accelX) < 200) velX += accelX * 0.1;
//		if (fabs(accelY) > 20 && fabs(accelY) < 220 && fabs(accelY) < 200) velY += accelY * 0.1;
//
//		if (fabs(accelX)/accelX == -(fabs(aX0)/aX0) && fabs(accelX-aX0)  > 30 && fabs(accelX) > 20 && fabs(aX0) > 20 && fabs(accelX-aX0) < 200)	endX = 1;
//		if (fabs(accelY)/accelY == -(fabs(aY0)/aY0) && fabs(accelY-aY0)  > 30 && fabs(accelY) > 20 && fabs(aY0) > 20 && fabs(accelY-aY0) < 200)	endY = 1;
//		if ((endX%6)){
//			velX = 0;
//			endX++;
//		}
//
//		if ((endY%6)){
//				velY = 0;
//				endY++;
//			}
//
//		posX += velX * 0.1;
//		posY += velY * 0.1;
//
//    	pX[i] = (int) posX;
//		vX[i] = (int) velX;
//
//		pY[i] = (int) posY;
//		vY[i] = (int) velY;
//

		i++;

	}

//	for(i = 0; i < 50; i++){
//		fprintf(stderr, "Xacc:\t%s", itoa(aX[i]));
//		fprintf(stderr, "\tXvel:\t%s", itoa(vX[i]));
//		fprintf(stderr, "\tXpos:\t%s", itoa(pX[i]));
//		fprintf(stderr, "\tYacc:\t%s", itoa(aY[i]));
//		fprintf(stderr, "\tYvel:\t%s\n", itoa(vY[i]));
//		fprintf(stderr, "\tXPOINT:\t%s", itoa(pointX[i]));
//		fprintf(stderr, "\tYpos:\t%s\n", itoa(pY[i]));
//	}
//	FILE *ptr_file;
//			int j;
//
//			ptr_file =fopen("output.txt", "w");
//
//			for (j=0; j<30; j++){
//				fprintf(ptr_file,"%s\n", itoa(pX[j]));
//				fprintf(ptr_file,"%s\n", itoa(pY[j]));
//			}
//			fclose(ptr_file);
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
	int accel = 0;
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

	SysCtlDelay(10*DELAY);

	accel = value;

	return accel;						// return value as int
}

char checkSwitches() {

  long 	lSwt1;
  long 	lSwt2;

  chSwtPrev = chSwtCur;

  lSwt1 = GPIOPinRead(SWT1Port, SWT1);
  lSwt2 = GPIOPinRead(SWT2Port, SWT2);

  chSwtCur = (lSwt1 | lSwt2) >> 6;


  return chSwtCur;

}

void drawArrow(float velX, float velY, double angle){
	int i,j;
	int ni,nj;
	char arrow[7][7] = {
			0,0,1,1,1,0,0,
			0,1,0,1,0,1,0,
			0,0,0,1,0,0,0,
			0,0,0,1,0,0,0,
			0,0,0,1,0,0,0,
			0,1,0,1,0,1,0,
			0,0,1,1,1,0,0
	};

	char rotArrow[7][7] = {0};
	for (i = 0; i < 7; i++){
		for(j = 0; j < 7; j++){
			if(arrow[i][j] == 1){
				nj = (int) ((j-3)*cos(angle) - (i-3)*sin(angle)) + 3;
				ni = (int) ((j-3)*sin(angle) + (i-3)*cos(angle)) + 3;
				rotArrow[ni][nj] = 1;
			}
		}
	}



//	for(i=0;i<7;i++){
//				for(j=0;j<7;j++)
//					if(rotArrow[i][j]==1){
//						OrbitOledMoveTo(j-3+(int)xc,i-3+(int)yc);
//						OrbitOledFillRect(j-3+(int)xc,i-3+(int)yc);
//					}
//			}
//	OrbitOledMoveTo(xc,yc);
//	OrbitOledLineTo(xc+velX,yc+velY);
	xc+=velX;
	yc+=velY;

	if (penDown){

	line[lineSize][0] = xc;
	line[lineSize][1] = yc;
	lineSize++;
	}
	OrbitOledClear();

	if (undo && lineSize > 0){
		line[lineSize-1][0] = 0;
		line[lineSize-1][1] = 0;
		lineSize-=1;

	}
	for(i = 0; i<lineSize; i++){
		OrbitOledMoveTo(line[i][0], line[i][1]);
		OrbitOledDrawPixel();
	}


//	int count = 0;
//	for (i = 0; i < 7; i++){
//				if (rotArrow[0][i] == 1){
//					count++;
//
//
//			}
//
//		}
//	fprintf(stderr, "%s\n", itoa(count));
	//fprintf(stderr, "%s\t%s\n", itoa(xc), itoa(velX));
		//OrbitOledClear();
		for(i=0;i<7;i++){
			for(j=0;j<7;j++)
				if(rotArrow[i][j]==1){
					OrbitOledMoveTo(j-3+(int)xc,i-3+(int)yc);
					OrbitOledDrawPixel();
				}
		}

		OrbitOledUpdate();

}

double getAngle(){
	  cMSB = (0xF00 & ulAIN0) >> 8;
	  cMIDB = (0x0F0 & ulAIN0) >> 4;
	  cLSB = (0x00F & ulAIN0);


	  szAIN[0] = '0';
	  szAIN[1] = 'x';
	  szAIN[2] = (cMSB > 9) ? 'A' + (cMSB - 10) : '0' + cMSB;
	  szAIN[3] = (cMIDB > 9) ? 'A' + (cMIDB - 10) : '0' + cMIDB;
	  szAIN[4] = (cLSB > 9) ? 'A' + (cLSB - 10) : '0' + cLSB;
	  szAIN[5] = '\0';
	  int potVal=0;
//	  int i;
//	  for(i=2;i<5;i++){
//		  if(szAIN[i] <= '9'){
//			  potval+=(szAIN[i]-'0')*pow(10,4-i);
//		  }
//	  }
	  potVal = (int)strtol(szAIN,NULL,0);
	  if(potVal>3960)potVal=3960;

	  return (double)  (potVal/11 * (3.14/180));

}
