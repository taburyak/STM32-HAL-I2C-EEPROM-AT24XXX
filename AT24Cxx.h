/*
 * AT24Cxx.h
 * Universal driver for AT24C series EEPROM (I2C)
 * Supports AT24C01 through AT24C512
 */

#ifndef AT24CXX_H_
#define AT24CXX_H_

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

/* --- User Configuration --- */

#define EEPROM_I2C              hi2c1
#define EEPROM_ADDRESS          0x50    /* Base 7-bit I2C address */
#define EEPROM_USE_FREERTOS     1       /* 1: Use osDelay, 0: Use HAL_Delay */

/* Select your chip model */
#define AT24C02                         /* Options: AT24C01, AT24C02, AT24C04, AT24C08, 
                                           AT24C16, AT24C32, AT24C64, AT24C128, 
                                           AT24C256, AT24C512 */

/* --- Chip Parameter Auto-Configuration --- */

#if defined(AT24C01) || defined(AT24C02)
    #define EEPROM_PAGESIZE     8
    #define EEPROM_MAX_ADDR     (defined(AT24C01) ? 128 : 256)
    #define EEPROM_ADDR_16BIT   0
#elif defined(AT24C04) || defined(AT24C08) || defined(AT24C16)
    #define EEPROM_PAGESIZE     16
    #define EEPROM_MAX_ADDR     (defined(AT24C04) ? 512 : (defined(AT24C08) ? 1024 : 2048))
    #define EEPROM_ADDR_16BIT   0
#elif defined(AT24C32) || defined(AT24C64)
    #define EEPROM_PAGESIZE     32
    #define EEPROM_MAX_ADDR     (defined(AT24C32) ? 4096 : 8192)
    #define EEPROM_ADDR_16BIT   1
#elif defined(AT24C128) || defined(AT24C256)
    #define EEPROM_PAGESIZE     64
    #define EEPROM_MAX_ADDR     (defined(AT24C128) ? 16384 : 32768)
    #define EEPROM_ADDR_16BIT   1
#elif defined(AT24C512)
    #define EEPROM_PAGESIZE     128
    #define EEPROM_MAX_ADDR     65536
    #define EEPROM_ADDR_16BIT   1
#endif

#define EEPROM_TIMEOUT          100     /* I2C timeout in ms */
#define EEPROM_WRITE_CYCLE_MS   10      /* Max internal write cycle time */

/* --- Function Prototypes --- */

/**
 * @brief Check if the EEPROM device is ready on the I2C bus
 * @return HAL_OK if found, error code otherwise
 */
HAL_StatusTypeDef AT24Cxx_IsConnected(void);

/**
 * @brief Wait for the EEPROM to finish its internal write cycle
 * @return HAL_OK if ready, HAL_TIMEOUT if failed
 */
HAL_StatusTypeDef AT24Cxx_WaitReady(void);

/**
 * @brief Read data from EEPROM
 * @param address Internal memory address to start reading from
 * @param dest Pointer to the destination buffer
 * @param len Number of bytes to read
 * @return HAL status
 */
HAL_StatusTypeDef AT24Cxx_Read(uint32_t address, void* dest, uint32_t len);

/**
 * @brief Write data to EEPROM (handles page boundaries automatically)
 * @param address Internal memory address to start writing to
 * @param src Pointer to the data to be written
 * @param len Number of bytes to write
 * @return HAL status
 */
HAL_StatusTypeDef AT24Cxx_Write(uint32_t address, const void* src, uint32_t len);

#endif /* AT24CXX_H_ */

