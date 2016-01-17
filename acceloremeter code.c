void OrbitDemo3() {

  short	dataX;
  short dataY;
  short dataZ;
  
  char printVal[10];
  
  char 	chPwrCtlReg = 0x2D;
  char 	chX0Addr = 0x32;
  char  chY0Addr = 0x34;
  char  chZ0Addr = 0x36;
  
  char 	rgchReadAccl[] = {
    0, 0, 0            };
  char 	rgchWriteAccl[] = {
    0, 0            };
    
  char rgchReadAccl2[] = {
    0, 0, 0            };
    
    char rgchReadAccl3[] = {
    0, 0, 0            };

  int	xcoRocketCur = xcoRocketStart;
  int 	ycoRocketCur = ycoRocketStart;
  int 	xcoExhstCur = xcoExhstStart;
  int 	ycoExhstCur = ycoExhstStart;

  int		xDirThreshPos = 50;
  int		xDirThreshNeg = -50;

  bool fDir = true;

  /*
   * If applicable, reset OLED
   */
  if(fClearOled == true) {
    OrbitOledClear();
    OrbitOledMoveTo(0,0);
    OrbitOledSetCursor(0,0);
    fClearOled = false;

    /*
     * Enable I2C Peripheral
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    /*
     * Set I2C GPIO pins
     */
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    /*
     * Setup I2C
     */
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    /* Initialize the Accelerometer
     *
     */
    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    rgchWriteAccl[0] = chPwrCtlReg;
    rgchWriteAccl[1] = 1 << 3;		// sets Accl in measurement mode
    I2CGenTransmit(rgchWriteAccl, 1, WRITE, ACCLADDR);
  }
  
  /*
   * Loop and check for movement until switches
   * change
   */
  while(CheckSwitches() == DEMO_3) {

    /*
     * Read the X data register
     */
    rgchReadAccl[0] = chX0Addr;
    rgchReadAccl2[0] = chY0Addr;
    rgchReadAccl3[0] = chZ0Addr;
    
    I2CGenTransmit(rgchReadAccl, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl2, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl3, 2, READ, ACCLADDR);
    
    dataX = (rgchReadAccl[2] << 8) | rgchReadAccl[1];
    dataY = (rgchReadAccl2[2] << 8) | rgchReadAccl2[1];
    dataZ = (rgchReadAccl3[2] << 8) | rgchReadAccl2[1];
    
     sprintf(printVal, "%d, %d, %d", (int)dataX, (int)dataY, (int)dataZ);

     OrbitOledSetCursor(0,0);

     OrbitOledPutString(printVal);
     OrbitOledUpdate();
     delay(1000);
     printVal[10] = {0};
     OrbitOledClear();
    
  }
    
}
