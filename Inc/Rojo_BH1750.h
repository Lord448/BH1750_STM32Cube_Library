/**
 * @author Pedro Rojo (pedroeroca@outlook.com) Lord448 @ github.com
 * @note  This Source Code Form is subject to the terms of the Mozilla Public
  		  License, v. 2.0. If a copy of the MPL was not distributed with this
  		  file, You can obtain one at http://mozilla.org/MPL/2.0/.
   @brief  This program allows you to use the BH1750 sensor with some few code steps
          also, you can mix this library with other libraries writted by me.
 * @version 0.1.0b
 * @date 2023-06-02
 * @copyright Copyright (c) 2023
 */

//Edited version
/*
 * Hacer funciones variadicas para poder
 * llevar un mejor control de las impresiones con el modulo de la division
 * there are some uncommented features for this new version*/

#ifndef INC_ROJO_BH1750_H_
#define INC_ROJO_BH1750_H_

#ifdef __cplusplus
extern C {
#endif

#ifndef ROJO_LIB_
#define ROJO_LIB_

//Checking the board are used

#ifdef __STM32F103xB_H //STM32F103C8T6 && STM32F103C6T6A
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_it.h"
#include "stm32f1xx_hal_conf.h"
#include "stm32f1xx_hal_def.h"
#endif

#ifdef __STM32F401xE_H //STM32F401CEU6
#include "stm32f401xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal_conf.h"
#endif

#ifdef __STM32F411xE_H //STM32F411CEU6
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal_conf.h"
#endif

/**
 * @brief It handles the status of the desired function or action, it also give some
 *        idea what could be the problem with the code
 */

typedef enum Status{
	Rojo_OK,
	Rojo_Variable_Overflow,
	Rojo_Invalid_Mode,
	Rojo_Stack_Overflow,
	Rojo_OverFrequency,
	Rojo_InexistentChannel,
	Rojo_Invalid_Action,
	Rojo_Overwrite,
	Rojo_Error
}Rojo_Status;

/**
 * @brief It gives the posibility to have false and true, making the code
 *        easier to understand.
 */
typedef enum bool
{
	false,
	true
}bool;

#endif /*ROJO_LIB*/

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

/* 												||-----------------||
 * 												||Measurement Modes||
 * 												||-----------------||
 *							   |----------------------------------------------------|
 *						       |  Measurement Mode  | Measurement Time | Resolution |
 *				      		   |--------------------|------------------|------------|
 *							   | H-Resolution Mode2 |    Typ. 120ms    |    0.5lx   |
 *							   |--------------------|------------------|------------|
 *							   | H-Resolution Mode  |    Typ. 120ms    |    1 lx    |
 *						       |--------------------|------------------|------------|
 *							   | L-Resolution Mode  |    Typ. 16ms     |    4lx     |
 *							   |----------------------------------------------------|
 */
//Commands
//*********************************************
#define PowerDown	 					(uint8_t)0b00000000 //No Active state
#define PowerOn	  						(uint8_t)0b00000001 //Waiting for measurement command
#define Reset 	  						(uint8_t)0b00000111 //Reset Data register value. Reset command is not acceptable in Power Down mode
#define Continuously_H_ResolutionMode	(uint8_t)0b00010000 //Start measurement at 1lx resolution. Measurement Time is typically 120ms
#define Continuously_H_ResolutionMode2	(uint8_t)0b00010001 //Start measurement at 0.5lx resolution. Measurement Time is typically 120ms
#define Continuously_L_ResolutionMode	(uint8_t)0b00010011 //Start measurement at 4lx resolution. Measurement Time is typically 16ms
//When used the sensor is automatically set to Power Down mode after measurement
#define OneTime_H_ResolutionMode		(uint8_t)0b00100000 //Start measurement at 1lx resolution. Measurement Time is typically 120ms
#define OneTime_H_ResolutionMode2		(uint8_t)0b00100001 //Start measurement at 0.5lx resolution. Measurement Time is typically 120ms
#define OneTime_L_ResolutionMode		(uint8_t)0b00100011 //Start measurement at 4lx resolution. Measurement Time is typically 16ms
//*********************************************
//Addresses
//*********************************************
#define Address_High 0xB8 //0x5C shifted 1 bit to the left because of the 7 bit address with "ADDR" pin in high
#define Address_Low 0x46 //0x23 shifted 1 bit to the left because of the 7 bit address with "ADDR" pin in low
//*********************************************
typedef enum BH1750_Status
{
	Standby, //The sensor is in PowerOn mode and it's waiting the instruction
	Busy, //The sensor is working
	Sleep //The sensor is PowerDown
}BH1750_Status;

typedef enum BH1750_Resolutions
{
	High_Res,
	Medium_Res,
	Low_Res
}BH1750_Resolutions;

typedef struct BH1750
{
	I2C_HandleTypeDef *I2C;
	uint8_t Address;
	BH1750_Resolutions Resolution;
	BH1750_Status Status;
	uint16_t Value;
}Rojo_BH1750;

/**
 * @brief It initializes the sensor structure and Power up the sensor in case it was
 * 		  power down and clear all the registers in the logic of the sensor
 * 
 * @param Rojo_BH1750: Structure that handles the sensor
 * @param hi2c: Structure declared by the HAL libraries, used for the I2C Commands
 * @param Address: Address configured of the sensor
 * @return Rojo_Status 
 */
Rojo_Status BH1750_Init(Rojo_BH1750 *Rojo_BH1750, I2C_HandleTypeDef *hi2c, uint8_t Address);

/**
 * @brief Takes the value of the measure register of the logic of the sensor and it process this
 * 		  value in a float value, so it can be used by the user
 * 
 * @param Rojo_BH1750: Strucuture that handles the sensor
 * @param Measure: Pointer to a float variable that will have the data in luxes
 * @return Rojo_Status 
 */
Rojo_Status BH1750_Read(Rojo_BH1750 *Rojo_BH1750, float *Measure);

/**
 * @brief It doesn't move the MTreg register or the sensitivity of the sensor
 * 		  It only reset all the registers of the sensor for a accurate and basic use
 * 
 * @see   BH1750VI datasheet for more information about changing the sensitivity of the sensor
 * @param Rojo_BH1750: Structure that handles the sensor 
 * @return Rojo_Status 
 */
Rojo_Status BH1750_ReCalibrate(Rojo_BH1750 *Rojo_BH1750);

/**
 * @brief It sends a command desired by the user 
 * 
 * @param Rojo_BH1750: Structure that handles the sensor
 * @param Command: Command that will be sended to the sensor
 * @return Rojo_Status 
 */
Rojo_Status BH1750_Command(Rojo_BH1750 *Rojo_BH1750, uint8_t Command);

/**
 * @brief It puts the sensor in a sleep mode for less current consumption
 * @note  Most of the functions ensure that the sensor is wake up before sending the commands
 * @param Rojo_BH1750: Structure that handles the sensor
 * @return Rojo_Status 
 */
Rojo_Status BH1750_Sleep(Rojo_BH1750 *Rojo_BH1750);

/**
 * @brief It set the resolution of the sensor by changing the mode of work of the sensor
 * 		  those modes are, Constinous mode at high resolution 1, Continous mode at high resolution2
 * 		  and Continuos mode at low resolution
 * @note  The one time mode of the sensor it's not used in the library
 * 		  because it powers down the sensor after getting the measure, so
 * 		  it is not important to use these modes since we have the sleep function
 * @see	  BH1750FVI datasheet for further information about one time modes
 * @param Rojo_BH1750: Structure that handles the sensor
 * @param Resolution: Objective resolution
 * @return Rojo_Status 
 */
Rojo_Status BH1750_SetResolution(Rojo_BH1750 *Rojo_BH1750, BH1750_Resolutions Resolution);

/*General Rojo sensors library functions*/

#ifndef INC_ROJO_SENSORS_H_
#define INC_ROJO_SENSORS_H_

/**
 * @brief This function prepare the "Integer_str" and the "Decimal_str" to print in any display
 * 		  taking a float variable (Obviously using the respective function in each type of display) 
 * 
 * @param value: Value that will appear on the two strings
 * @param Integer_str: String that will have the integer part of the number
 * @param Decimal_str: String that will have the decimal part of the number
 * @param Number_of_decimals: Number of decimals that will appear on the "Decimal_str"
 */
void Prepare_to_print(float value, char *Integer_str, char *Decimal_str, uint16_t Number_of_decimals);

#endif /*INC_ROJO_SENSORS_H_*/

#endif /* INC_ROJO_BH1750_H_ */
