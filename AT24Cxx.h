/*
 * AT24Cxx.h
 *
 *  Created on: 20 трав. 2018 р.
 *      Author: Andriy
 */

#ifndef AT24CXX_H_
#define AT24CXX_H_

#include "stm32f1xx_hal.h"
#include "stdbool.h"

#define	EEPROM_I2C			hi2c1
#define EEPROM_ADDRESS		0x50
#define EEPROM_PAGESIZE		128
#define EEPROM_WRITE		10					//time to wait in ms
#define EEPROM_TIMEOUT		5 * EEPROM_WRITE	//timeout while writing

extern I2C_HandleTypeDef EEPROM_I2C;

HAL_StatusTypeDef AT24Cxx_IsConnected(void);
HAL_StatusTypeDef AT24Cxx_ReadEEPROM(unsigned address, const void* src, unsigned len);
HAL_StatusTypeDef AT24Cxx_WriteEEPROM(unsigned address, const void* src, unsigned len);

#endif /* AT24CXX_H_ */
