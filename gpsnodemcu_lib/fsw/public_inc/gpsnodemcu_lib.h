/*!
 * \file
 *
 * \brief This file contains functions for nodemcu as gps driver.
 *
 * @{
 */
// ----------------------------------------------------------------------------

#ifndef GPSNODEMCU_H
#define GPSNODEMCU_H

#include "cfe.h"

// -------------------------------------------------------------- PUBLIC MACROS 
/**
 * \defgroup macros Macros
 * \{
 */

/**
 * \defgroup i2c speed. BAUDRATE
 * \{
 */
#define BAUDRATE             10000

/**
 * \defgroup i2c_registers NodeMCU as GPS I2C register
 * \{
 */
#define NODEMCU_SLAVE_ADDRESS                   0x08  //Device address 

#define NODEMCU_WHOAMI                          0x10  //Identification register. Shall return 0x08

#define NODEMCU_TIME_REG_B0                     0x11  //Register to access time byte0
#define NODEMCU_TIME_REG_B1                     0x12  //Register to access time byte1
#define NODEMCU_TIME_REG_B2                     0x13  //Register to access time byte2
#define NODEMCU_TIME_REG_B3                     0x14  //Register to access time byte3
#define NODEMCU_TIME_REG_B4                     0x15  //Register to access time byte4
#define NODEMCU_TIME_REG_B5                     0x16  //Register to access time byte5
#define NODEMCU_TIME_REG_B6                     0x17  //Register to access time byte6
#define NODEMCU_TIME_REG_B7                     0x18  //Register to access time byte7

#define NODEMCU_XPOS_REG_B0                     0x21  //Register to access xpos byte0
#define NODEMCU_XPOS_REG_B1                     0x22  //Register to access xpos byte1
#define NODEMCU_XPOS_REG_B2                     0x23  //Register to access xpos byte2
#define NODEMCU_XPOS_REG_B3                     0x24  //Register to access xpos byte3
#define NODEMCU_XPOS_REG_B4                     0x25  //Register to access xpos byte4
#define NODEMCU_XPOS_REG_B5                     0x26  //Register to access xpos byte5
#define NODEMCU_XPOS_REG_B6                     0x27  //Register to access xpos byte6
#define NODEMCU_XPOS_REG_B7                     0x28  //Register to access xpos byte7

#define NODEMCU_YPOS_REG_B0                     0x31  //Register to access ypos byte0
#define NODEMCU_YPOS_REG_B1                     0x32  //Register to access ypos byte1
#define NODEMCU_YPOS_REG_B2                     0x33  //Register to access ypos byte2
#define NODEMCU_YPOS_REG_B3                     0x34  //Register to access ypos byte3
#define NODEMCU_YPOS_REG_B4                     0x35  //Register to access ypos byte4
#define NODEMCU_YPOS_REG_B5                     0x36  //Register to access ypos byte5
#define NODEMCU_YPOS_REG_B6                     0x37  //Register to access ypos byte6
#define NODEMCU_YPOS_REG_B7                     0x38  //Register to access ypos byte7

#define NODEMCU_ZPOS_REG_B0                     0x41  //Register to access zpos byte0
#define NODEMCU_ZPOS_REG_B1                     0x42  //Register to access zpos byte1
#define NODEMCU_ZPOS_REG_B2                     0x43  //Register to access zpos byte2
#define NODEMCU_ZPOS_REG_B3                     0x44  //Register to access zpos byte3
#define NODEMCU_ZPOS_REG_B4                     0x45  //Register to access zpos byte4
#define NODEMCU_ZPOS_REG_B5                     0x46  //Register to access zpos byte5
#define NODEMCU_ZPOS_REG_B6                     0x47  //Register to access zpos byte6
#define NODEMCU_ZPOS_REG_B7                     0x48  //Register to access zpos byte7



/** \} */


/** \} */ // End group macro 
// ----------------------------------------------- PUBLIC FUNCTION DECLARATIONS

/**
 * \defgroup public_function Public function
 * \{
 */
 
#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief Config Object Initialization function.
 *
 * @param cfg  Click configuration structure.
 *
 * @description This function initializes click configuration structure to init state.
 * @note All used pins will be set to unconnected state.
 */
void nodemcu_readregister(uint8_t slaveaddress, char registertoread, char *rxBuffer, ssize_t length);

/**
 * @brief Config Object Initialization function.
 *
 * @param cfg  Click configuration structure.
 *
 * @description This function initializes click configuration structure to init state.
 * @note All used pins will be set to unconnected state.
 */
double nodemcu_gettime(void);

double nodemcu_getxpos(void);

double nodemcu_getypos(void);

double nodemcu_getzpos(void);

/**
 * @brief Initialize library on cFS
 *
 * @param void            
 *
 * @returns               Int32 Success or failure
 *
 * @description Function initialize library on cFS.
 */
int32 GPSNODEMCU_LIB_Init(void);

#ifdef __cplusplus
}
#endif
#endif  // _MPU9DOF_H_

/** \} */ // End public_function group
/// \}    // End click Driver group  
/*! @} */
// ------------------------------------------------------------------------- END
