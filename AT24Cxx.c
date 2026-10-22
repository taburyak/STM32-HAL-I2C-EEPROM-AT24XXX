/*
 * AT24Cxx.c
 */

#include "AT24Cxx.h"

static const uint16_t eeprom_device_addr = (EEPROM_ADDRESS << 1);
static const uint16_t inpage_addr_mask = (EEPROM_PAGESIZE - 1);

static uint16_t size_to_page_end(uint16_t addr)
{
    return EEPROM_PAGESIZE - (addr & inpage_addr_mask);
}

HAL_StatusTypeDef AT24Cxx_IsConnected(void)
{
    return HAL_I2C_IsDeviceReady(&EEPROM_I2C, eeprom_device_addr, 2, EEPROM_TIMEOUT);
}

/**
 * @brief Опитування готовності чипа після запису (Acknowledge Polling)
 *        Замінює блокуючий HAL_Delay на неблокуючу перевірку з поступкою кванта FreeRTOS.
 */
HAL_StatusTypeDef AT24Cxx_WaitStandbyState(void)
{
    uint32_t start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) < EEPROM_WRITE_MAX_MS)
    {
        if (HAL_I2C_IsDeviceReady(&EEPROM_I2C, eeprom_device_addr, 1, 5) == HAL_OK)
        {
            return HAL_OK;
        }

        // Якщо планувальник FreeRTOS активний, віддаємо квант часу іншим таскам
        if (osKernelGetState() == osKernelRunning)
        {
            osDelay(1);
        }
    }

    return HAL_TIMEOUT;
}

HAL_StatusTypeDef AT24Cxx_ReadEEPROM(uint16_t address, void* dest, uint16_t len)
{
    if (dest == NULL || len == 0 || (address + len) > 256)
    {
        return HAL_ERROR;
    }

    // Переконуємось, що попередня операція запису вже завершена
    HAL_StatusTypeDef status = AT24Cxx_WaitStandbyState();
    if (status != HAL_OK)
    {
        return status;
    }

    // Для AT24C02 використовується 8-бітний розмір адреси комірки
    return HAL_I2C_Mem_Read(&EEPROM_I2C,
                            eeprom_device_addr,
                            address,
                            I2C_MEMADD_SIZE_8BIT,
                            (uint8_t*)dest,
                            len,
                            EEPROM_TIMEOUT);
}

HAL_StatusTypeDef AT24Cxx_WriteEEPROM(uint16_t address, const void* src, uint16_t len)
{
    if (src == NULL || len == 0 || (address + len) > 256)
    {
        return HAL_ERROR;
    }

    const uint8_t *pdata = (const uint8_t*)src;
    HAL_StatusTypeDef result = HAL_OK;

    while (len > 0)
    {
        // Перевіряємо скільки байт можна записати до межі поточної 8-байтової сторінки
        uint16_t max_portion = size_to_page_end(address);
        uint16_t chunk = (len < max_portion) ? len : max_portion;

        // Чекаємо завершення попередніх операцій
        result = AT24Cxx_WaitStandbyState();
        if (result != HAL_OK)
        {
            return result;
        }

        // Запис порції даних на сторінку (8-бітна адресація)
        result = HAL_I2C_Mem_Write(&EEPROM_I2C,
                                   eeprom_device_addr,
                                   address,
                                   I2C_MEMADD_SIZE_8BIT,
                                   (uint8_t*)pdata,
                                   chunk,
                                   EEPROM_TIMEOUT);

        if (result != HAL_OK)
        {
            return result;
        }

        len     -= chunk;
        address += chunk;
        pdata   += chunk;
    }

    // Очікуємо готовності після завершення останнього циклу запису
    return AT24Cxx_WaitStandbyState();
}