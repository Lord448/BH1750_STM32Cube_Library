/**
 * @author Pedro Rojo (pedroeroca@outlook.com)
 * @note  This Source Code Form is subject to the terms of the Mozilla Public
  		  License, v. 2.0. If a copy of the MPL was not distributed with this
  		  file, You can obtain one at http://mozilla.org/MPL/2.0/.
   @brief  This program allows you to use the BH1750 sensor with some few code steps
          also, you can mix this library with other libraries writted by me.
 * @version 0.1.0b
 * @date 2023-06-02
 * @copyright Copyright (c) 2023
 */

#include "Rojo_BH1750.h"

/*STATIC ZONE*/

static uint8_t Buffer;

/*STATIC ZONE*/

/**
 * @brief This function automatizates the reset command 
 * 
 * @param Rojo_BH1750: Structture that handles the sensor
 * @return Rojo_Status 
 */
static Rojo_Status ResetCommand(Rojo_BH1750 *Rojo_BH1750)
{
	Buffer = Reset;
	if(HAL_I2C_Master_Transmit(Rojo_BH1750 -> I2C, Rojo_BH1750 -> Address, &Buffer, 1, 100) != HAL_OK)
		return Rojo_Error;
	else
		return Rojo_OK;
}

/**
 * @brief This function automatizates the power on command
 * 
 * @param Rojo_BH1750: Structture that handles the sensor
 * @return Rojo_Status 
 */
static Rojo_Status PowerOnCommand(Rojo_BH1750 *Rojo_BH1750)
{
	Buffer = PowerOn;
	if(HAL_I2C_Master_Transmit(Rojo_BH1750 -> I2C, Rojo_BH1750 -> Address, &Buffer, 1, 100) != HAL_OK)
		return Rojo_Error;
	else
		return Rojo_OK;
}

/**
 * @brief This function automatizates the power down command
 * 
 * @param Rojo_BH1750: Structture that handles the sensor
 * @return Rojo_Status 
 */
static Rojo_Status PowerDownCommand(Rojo_BH1750 *Rojo_BH1750)
{
	Buffer = PowerDown;
	if(HAL_I2C_Master_Transmit(Rojo_BH1750 -> I2C, Rojo_BH1750 -> Address, &Buffer, 1, 100) != HAL_OK)
		return Rojo_Error;
	else
		return Rojo_OK;
}

/**
 * @brief This function sends the set of opcodes by I2C to acquire a measure divided by two numbers of 8 bit
 * 		  and it's processed to unite them in a 16 bit variable
 * 
 * @param Rojo_BH1750: Structture that handles the sensor
 * @return uint16_t: Value of the meausure in 16 bit code (Not luxes)
 */
static uint16_t Measure_Subrutine(Rojo_BH1750 *Rojo_BH1750)
{
	uint8_t Data[2];
	switch(Rojo_BH1750 -> Resolution)
	{
		case High_Res:
			Buffer = Continuously_H_ResolutionMode2;
		break;
		case Medium_Res:
			Buffer = Continuously_H_ResolutionMode;
		break;
		case Low_Res:
			Buffer = Continuously_L_ResolutionMode;
		break;
		default:
			return 0;
		break;
	}
	Rojo_BH1750 -> Status = Busy;
	if(HAL_I2C_Master_Transmit(Rojo_BH1750 -> I2C, Rojo_BH1750 -> Address, &Buffer, 1, 100) != HAL_OK)
		return 0;
	HAL_Delay(120);
	if(HAL_I2C_Master_Receive(Rojo_BH1750 -> I2C, Rojo_BH1750 -> Address, Data, 2, 100) != HAL_OK)
		return 0;
	Rojo_BH1750 -> Status = Standby;
	return (uint16_t) (Data[0] << 8 | Data[1]);
}

/*END OF STATIC ZONE*/

Rojo_Status BH1750_Init(Rojo_BH1750 *Rojo_BH1750, I2C_HandleTypeDef *hi2c, uint8_t Address)
{

	Rojo_BH1750 -> I2C = hi2c;
	Rojo_BH1750 -> Address = Address;
	Rojo_BH1750 -> Resolution = Medium_Res;
	Rojo_BH1750 -> Status = Standby;
	Rojo_BH1750 -> Value = 0;
	if(PowerOnCommand(Rojo_BH1750) != Rojo_OK) //Waking the sensor logic
		return Rojo_Error;
	HAL_Delay(10);
	if(ResetCommand(Rojo_BH1750) != Rojo_OK) //Clearing all the register of the sensor
		return Rojo_Error;
	return Rojo_OK;
}

Rojo_Status BH1750_Read(Rojo_BH1750 *Rojo_BH1750, float *Measure)
{
	uint16_t RegisterValue;
	switch(Rojo_BH1750 -> Status)
	{
		case Busy:
			RegisterValue = Rojo_BH1750 -> Value;
			Rojo_BH1750 -> Status = Standby;
		break;
		case Standby:
			RegisterValue = Measure_Subrutine(Rojo_BH1750);
		break;
		case Sleep:
			if(PowerDownCommand(Rojo_BH1750) != Rojo_OK)
				return Rojo_Error;
			HAL_Delay(10);
			RegisterValue = Measure_Subrutine(Rojo_BH1750);
		break;
		default:
			return Rojo_Error;
		break;
	}
	*Measure = RegisterValue / 1.2;
	return Rojo_OK;
}

Rojo_Status BH1750_ReCalibrate(Rojo_BH1750 *Rojo_BH1750)
{
	switch(Rojo_BH1750 -> Status)
	{
		case Standby:
			if(ResetCommand(Rojo_BH1750) != Rojo_OK) //Just making the reset
				return Rojo_Error;
		break;
		case Sleep:
			if(PowerOnCommand(Rojo_BH1750) != Rojo_OK) //Waking up the sensor logic
				return Rojo_Error;
			if(ResetCommand(Rojo_BH1750) != Rojo_OK) //Making the reset
				return Rojo_Error;
		break;
		default:
			return Rojo_Error;
		break;
	}
	return Rojo_OK;
}

Rojo_Status BH1750_Command(Rojo_BH1750 *Rojo_BH1750, uint8_t Command)
{
	//Checking if the command matches with the possibles ones
	if(Command == PowerOn)
		Rojo_BH1750 -> Status = Standby;
	else if(Command == PowerDown)
		Rojo_BH1750 -> Status = Sleep;
	//Sending the command
	if(HAL_I2C_Master_Transmit(Rojo_BH1750 -> I2C, Rojo_BH1750 -> Address, &Command, 1, 100) != HAL_OK)
		return Rojo_Error;
	return Rojo_OK;
}

Rojo_Status BH1750_Sleep(Rojo_BH1750 *Rojo_BH1750)
{
	if(Rojo_BH1750 -> Status != Standby)
		return Rojo_Error;
	if(PowerDownCommand(Rojo_BH1750) != Rojo_OK)
		return Rojo_Error;
	Rojo_BH1750 -> Status = Sleep;
	return Rojo_OK;
}

Rojo_Status BH1750_SetResolution(Rojo_BH1750 *Rojo_BH1750, BH1750_Resolutions Resolution)
{
	switch(Resolution)
	{
		case High_Res:
			Rojo_BH1750 -> Resolution = Resolution;
		break;
		case Medium_Res:
			Rojo_BH1750 -> Resolution = Resolution;
		break;
		case Low_Res:
			Rojo_BH1750 -> Resolution = Resolution;
		break;
		default:
			return Rojo_Error;
		break;
	}
	return Rojo_OK;
}

#ifdef INC_ROJO_SENSORS_H_
/*Not Working*/
void Prepare_to_print(float Value, char Integer_str[], char Decimal_str[], uint16_t Number_of_decimals)
{
	uint32_t Integer_Part, Decimal_Part;
	uint32_t Multiplier = 1;
	if(Number_of_decimals > 10)
		Number_of_decimals = 10;
	Integer_Part = (uint32_t) Value;
	for(uint16_t i = 0; i > Number_of_decimals; i++)
	{
		Multiplier *= 10;
	}
	Decimal_Part = (Value - Integer_Part) * Multiplier;
	sprintf(Integer_str, "%d", (int)Integer_Part);
	sprintf(Decimal_str, "%d", (int)Decimal_Part);
}
#endif /*INC_ROJO_SENSORS_H_*/
