/*
 * AT24Cxx.h
 */

#ifndef AT24CXX_H_
#define AT24CXX_H_

#include "main.h"
#include <stdbool.h>
#include "cmsis_os2.h"

#define EEPROM_I2C          hi2c1
#define EEPROM_ADDRESS      0x50

/* Для AT24C02 розмір сторінки дорівнює 8 байт (усього 256 байт = 32 сторінки) */
#define EEPROM_PAGESIZE     8
#define EEPROM_TIMEOUT      100     /* Таймаут операцій I2C в мс */
#define EEPROM_WRITE_MAX_MS 10      /* Максимальний час внутрішнього циклу запису */

extern I2C_HandleTypeDef EEPROM_I2C;

HAL_StatusTypeDef AT24Cxx_IsConnected(void);
HAL_StatusTypeDef AT24Cxx_WaitStandbyState(void);
HAL_StatusTypeDef AT24Cxx_ReadEEPROM(uint16_t address, void* dest, uint16_t len);
HAL_StatusTypeDef AT24Cxx_WriteEEPROM(uint16_t address, const void* src, uint16_t len);

#endif /* AT24CXX_H_ */