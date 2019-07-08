/*
 * AT24Cxx.c
 *
 *  Created on: 20 трав. 2018 р.
 *      Author: Andriy
 */
#include "AT24Cxx.h"

// Перетворюємо адресу з 7 біт на на 8 біт
static unsigned eeprom_address = EEPROM_ADDRESS << 1;
// Маску обчислюємо по заданому розміру сторінки.
static unsigned inpage_addr_mask = EEPROM_PAGESIZE - 1;

static HAL_StatusTypeDef AT24Cxx_WriteReadEEPROM(unsigned address, const void* src, unsigned len, bool write);
static unsigned size_to_page_end(unsigned addr);

HAL_StatusTypeDef AT24Cxx_IsConnected(void)
{
	return HAL_I2C_IsDeviceReady(&EEPROM_I2C, eeprom_address, 1, EEPROM_TIMEOUT);
}

HAL_StatusTypeDef AT24Cxx_ReadEEPROM(unsigned address, const void* src, unsigned len)
{
	return AT24Cxx_WriteReadEEPROM(address, src, len, false);
}

HAL_StatusTypeDef AT24Cxx_WriteEEPROM(unsigned address, const void* src, unsigned len)
{
	return AT24Cxx_WriteReadEEPROM(address, src, len, true);
}

static HAL_StatusTypeDef AT24Cxx_WriteReadEEPROM(unsigned address, const void* src, unsigned len, bool write)
{
	uint8_t *pdata = (uint8_t*) src;

	HAL_StatusTypeDef result = HAL_OK;

    // Перша порція не може бути більша, щоб не перетнула розмір сторінки
    unsigned max_portion = size_to_page_end(address);
    unsigned portion;

    while (len != 0 && result == HAL_OK)
    {
        portion = len;              // Все, що залишилося -- у одну порцію

        if (portion > max_portion)
        {
        	portion = max_portion;  // Порція завелика, зменшуємо
        }

        // Працюємо із порцією, яку можна проковтнути
        if(write)
		{
			result = HAL_I2C_Mem_Write(&EEPROM_I2C,
									eeprom_address,
									address,
									I2C_MEMADD_SIZE_16BIT,
									pdata,
									portion,
									EEPROM_TIMEOUT);
		}
        else
        {
        	result = HAL_I2C_Mem_Read(&EEPROM_I2C,
        	        				eeprom_address,
        							address,
        							I2C_MEMADD_SIZE_16BIT,
        							pdata,
        							portion,
        							EEPROM_TIMEOUT);
        }

        // І реєструємо, що вже це зробили.
        len     -= portion;
        address += portion;
        pdata   += portion;

        // наступне, якщо взагалі буде, буде від початку сторінки
        max_portion = EEPROM_PAGESIZE;

        if(write)
        {
        	HAL_Delay(EEPROM_WRITE);
        }
        else
        {
        	HAL_Delay(EEPROM_WRITE / 2);
        }
    }

    return result;
}

// обчислення відстані до кінця сторінки
static unsigned size_to_page_end(unsigned addr)
{
    return (~addr & inpage_addr_mask) + 1;
}
