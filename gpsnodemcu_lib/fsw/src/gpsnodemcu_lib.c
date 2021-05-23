/*
 * MikroSDK - MikroE Software Development Kit
 * Copyright© 2020 MikroElektronika d.o.o.
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
 * OR OTHER DEALINGS IN THE SOFTWARE. 
 */

/*!
 * \file
 *
 */

#include "gpsnodemcu_lib.h"
#include "bcm2835_lib.h"

#include "cfe.h"

// ------------------------------------------------ PUBLIC FUNCTION DEFINITIONS
void nodemcu_readregister(uint8_t slaveaddress, char registertoread, char *rxBuffer, ssize_t length){
    
    // Set slave address in the i2c
    bcm2835_i2c_setSlaveAddress(slaveaddress);
    
    // Write the register and read the number of bytes expected
    bcm2835_i2c_write(&registertoread,1);
    bcm2835_i2c_read(rxBuffer,length);

}

float nodemcu_gettime(void){

    // Declare variables
    char rxBuffer[4] = {0};
    float result = 0;
    uint32_t temp;
    
    // Read register from the nodemcu
    nodemcu_readregister(NODEMCU_SLAVE_ADDRESS, NODEMCU_TIME_REG, rxBuffer, 4);

    temp = ((uint32_t) rxBuffer[0] << 24) | ((uint32_t) rxBuffer[1] << 16 ) | ((uint32_t) rxBuffer[2] << 8) | ((uint32_t) rxBuffer[3]);
       
    result = *(float *)&temp;

    return result;

}


// Function of initialization for the cFS
int32 GPSNODEMCU_LIB_Init(void)
{
    /*
     * Initialize the variables for the default configuration and read
     * the WHO AM I register. If the response is not the expected avoid 
     * implementation
     */
    
    // Definition of class and variables
    char            RxBuffer[1] = {0};
    
        
    // Read the WHO AM I register
    nodemcu_readregister (NODEMCU_SLAVE_ADDRESS, NODEMCU_WHOAMI, RxBuffer, 1);
    
    // Check for the expected value
    if (RxBuffer[0] != 0x08){
        OS_printf("GPSNODEMCU Lib not implemented. NodeMCU not found. Response: %c. ", RxBuffer[0]);
        return CFE_STATUS_NOT_IMPLEMENTED;
    }
    
    OS_printf("GPSNODEMCU Lib Initialized.\n");

    return CFE_SUCCESS;

}

// ------------------------------------------------------------------------- END

