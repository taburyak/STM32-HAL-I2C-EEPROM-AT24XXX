/*
 * AT24Cxx.c
 * Universal driver for AT24C series EEPROM (I2C)
 */

#include "AT24Cxx.h"

#if (EEPROM_USE_FREERTOS == 1)
#include "cmsis_os2.h"
#endif

extern I2C_HandleTypeDef EEPROM_I2C;

/**
 * @brief Get the actual I2C device address based on the memory address.
 *        For AT24C04-C16, parts of the memory address are sent in the I2C device address.
 */
static uint16_t get_device_addr(uint32_t mem_addr)
{
    uint16_t dev_addr = (EEPROM_ADDRESS << 1);

#if (EEPROM_ADDR_16BIT == 0)
    /* For chips <= 16Kbit, the page bits (8, 9, 10) are part of the device address */
    dev_addr |= ((mem_addr >> 8) & 0x07) << 1;
#endif

    return dev_addr;
}

HAL_StatusTypeDef AT24Cxx_IsConnected(void)
{
    return HAL_I2C_IsDeviceReady(&EEPROM_I2C, (EEPROM_ADDRESS << 1), 3, EEPROM_TIMEOUT);
}

HAL_StatusTypeDef AT24Cxx_WaitReady(void)
{
    uint32_t start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) < EEPROM_WRITE_CYCLE_MS)
    {
        /* We check the base address; if the chip is busy, it won't ACK */
        if (HAL_I2C_IsDeviceReady(&EEPROM_I2C, (EEPROM_ADDRESS << 1), 1, 5) == HAL_OK)
        {
            return HAL_OK;
        }

#if (EEPROM_USE_FREERTOS == 1)
        if (osKernelGetState() == osKernelRunning)
        {
            osDelay(1);
        }
#endif
    }

    return HAL_TIMEOUT;
}

HAL_StatusTypeDef AT24Cxx_Read(uint32_t address, void* dest, uint32_t len)
{
    if (dest == NULL || len == 0 || (address + len) > EEPROM_MAX_ADDR)
    {
        return HAL_ERROR;
    }

    if (AT24Cxx_WaitReady() != HAL_OK)
    {
        return HAL_TIMEOUT;
    }

    uint16_t dev_addr = get_device_addr(address);
    uint16_t mem_addr_size = (EEPROM_ADDR_16BIT) ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT;

    return HAL_I2C_Mem_Read(&EEPROM_I2C,
                            dev_addr,
                            (uint16_t)address,
                            mem_addr_size,
                            (uint8_t*)dest,
                            len,
                            EEPROM_TIMEOUT);
}

HAL_StatusTypeDef AT24Cxx_Write(uint32_t address, const void* src, uint32_t len)
{
    if (src == NULL || len == 0 || (address + len) > EEPROM_MAX_ADDR)
    {
        return HAL_ERROR;
    }

    const uint8_t *pdata = (const uint8_t*)src;
    uint16_t mem_addr_size = (EEPROM_ADDR_16BIT) ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT;

    while (len > 0)
    {
        /* Calculate how many bytes can be written until the end of the current page */
        uint32_t bytes_to_page_end = EEPROM_PAGESIZE - (address % EEPROM_PAGESIZE);
        uint32_t chunk = (len < bytes_to_page_end) ? len : bytes_to_page_end;

        if (AT24Cxx_WaitReady() != HAL_OK)
        {
            return HAL_TIMEOUT;
        }

        if (HAL_I2C_Mem_Write(&EEPROM_I2C,
                               get_device_addr(address),
                               (uint16_t)address,
                               mem_addr_size,
                               (uint8_t*)pdata,
                               (uint16_t)chunk,
                               EEPROM_TIMEOUT) != HAL_OK)
        {
            return HAL_ERROR;
        }

        len     -= chunk;
        address += chunk;
        pdata   += chunk;
    }

    return AT24Cxx_WaitReady();
}
