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
 * \defgroup i2c_address MPU9150A I2C address
 * \{
 */
#define NODEMCU_SLAVE_ADDRESS                   0x08  //Device address 
#define NODEMCU_WHOAMI                          0x10  //Identification register. Shall return 0x08
#define NODEMCU_TIME_REG                        0x11  //Register to access time float

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
void nodemcu_readregister(uint8_t slaveaddress, uint8_t registertoread, uint8_t *rxBuffer, ssize_t length);

/**
 * @brief Config Object Initialization function.
 *
 * @param cfg  Click configuration structure.
 *
 * @description This function initializes click configuration structure to init state.
 * @note All used pins will be set to unconnected state.
 */
float nodemcu_gettime(void);


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
