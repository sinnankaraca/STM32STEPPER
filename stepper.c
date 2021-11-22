/*******************************************************************************
 * File Name          : stepperPart1.c
 * Description        : Controlling stepper with terminal
 *			 Torque is decided with potentiometer
 *
 *
 *
 * Author:              Group 1
 *			 Sinan KARACA
 *			 Mohammed Al Bunde
 * Date:                12.10.2021
 ******************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "common.h"

TIM_HandleTypeDef tim11;

// FUNCTION      : gpioinit1()
// DESCRIPTION   : The function enables the GPIO Pins of port A
// PARAMETERS    : The function accepts an int value
// RETURNS       : None
ParserReturnVal_t gpioinit1(int mode) {
	/* Turn on clocks to I/O */

	__GPIOA_CLK_ENABLE();

	/* Configure GPIO pins */
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = (GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9
		| GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
		| GPIO_PIN_14);
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_TIM_Base_Start(&tim11);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);     //pwm
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);     //Reset

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);	 //ps
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 1);     //MD1

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 1);    //Md2

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 0);    //AT1

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, 0);    //AT2

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, 1);    //FR

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);    // OE

	return CmdReturnOk;
}

ADD_CMD("gpioinit1", gpioinit1, "              Initialize the GPIO Pins for StepMotor");

//Memory Array for user input data
//1. data is direction, 2. data is speed in rpm, 3. data is steps
int data[100][3] = { { 1, 100, 10000 }, { 1, 150, 10000 }, { 1, 50, 5000 } };

// Initial value for memory array
// It will get incremented inside the functions
uint32_t arrayIndex = 3;

// FUNCTION      : addArray
// DESCRIPTION   : 
// PARAMETERS    : 
// RETURNS       : 
ParserReturnVal_t addArray(int mode) {

	uint32_t userDir;
	uint32_t userSpeed;
	uint32_t userSteps;
	uint32_t delayVal;

	fetch_uint32_arg(&userDir);
	fetch_uint32_arg(&userSpeed);
	fetch_uint32_arg(&userSteps);

	delayVal = 1600 * userSpeed / 60;

	if (delayVal > 15000 || delayVal < 0) {

		printf("Please write a valid frequency!! Between 1 - 15k Hz\n ");

		return CmdReturnBadParameter1;
	}

	if (userDir == 1) {
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 1);    //FR clockwise
	}

	else if (userDir == 0) {

		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);    //FR clockwise

	}
	else {

		return CmdReturnBadParameter1;

	}

	// User input to store inside the array
	data[arrayIndex][0] = userDir;
	data[arrayIndex][1] = userSpeed;
	data[arrayIndex][2] = userSteps;

	arrayIndex = arrayIndex + 1;

	return CmdReturnOk;
}

ADD_CMD("addelem", addArray, "<direction 0-1><speed 0-450><steps>  Add element to the queue");

uint32_t delayCalc, tempCount;
uint32_t delayCalcTemp = 1000;
uint32_t dir;
uint32_t stepTemp;
uint32_t pwmDivider = 0;
uint32_t pwmDividerFast = 1;
uint32_t pwmDividerSlow = 33;
uint32_t stepNumberTimesTwo, arrayIncrementx;
uint32_t stepperPartTwoCount = 0;
uint32_t arrayIncrementx = 0;
uint32_t stepCount = 0;
uint32_t stepperMode;
uint32_t pwmDivider2Temp = 0;
uint32_t rotationDirTemp;

// FUNCTION      : stepspeed()
// DESCRIPTION   : The function activates the stepper motor to operate
//                 in clockwise/Anticlockwise direction and also in 2 different modes:
//		           Half-step and Full-Step
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
/*Adding Delay*/
ParserReturnVal_t stepspeed(int mode) {
	uint32_t stepNumber;
	uint32_t delayVal;
	uint32_t speed;

	stepCount = 0;
	delayCalcTemp = 1;
	tempCount = 0;
	pwmDivider = 50;

	fetch_uint32_arg(&stepperMode);

	// There are 2 modes for stepper part 1 and stepper part 2 labs
	if (stepperMode < 1 || stepperMode > 2) {

		printf("Please choose mode between 1-2\n ");

		return CmdReturnBadParameter1;

	}

	fetch_uint32_arg(&dir);
	fetch_uint32_arg(&speed);

	//Calculation for speed(rpm) to pwm
	delayVal = 1600 * speed / 60;
	delayCalc = 1000000 / delayVal / 2;

	fetch_uint32_arg(&stepNumber);
	stepNumberTimesTwo = stepNumber * 2;

	//For only stepper mode
	//Change the turning direction according to the user input

	if (stepperMode == 1) {
		//Check for if speeds reach the maximum value
		if (delayVal > 15000 || delayVal < 0) {

			printf("Please write a valid frequency!! Between 1 - 15k Hz\n ");

			return CmdReturnBadParameter1;
		}

		//Change the direction according to the user input
		if (dir == 0) {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, 0);    //FR counterclockwise

		}

		else if (dir == 1) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 1);    //FR clockwise
		}

		else if (dir == 2) {
			HAL_TIM_Base_Stop_IT(&tim11);
		}

	}

	if (mode != CMD_INTERACTIVE) {
		return CmdReturnBadParameter1;
	}

	// Initialise the timer 
	__HAL_RCC_TIM11_CLK_ENABLE();
	tim11.Instance = TIM11;
	tim11.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
	tim11.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim11.Init.Period = delayCalc - 1;
	;
	tim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim11.Init.RepetitionCounter = 0;

	HAL_TIM_Base_Init(&tim11);

	// Timer interrupt parameters
	HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 10, 0U);
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	// Run the timer in interrupt note
	HAL_TIM_Base_Start_IT(&tim11);

	return CmdReturnOk;

}
ADD_CMD("stepspeed", stepspeed, "       Makes stepper step at a speed set by the delay value, continuously step<steps><delay>");

void stepperPartOne(void) {

	uint32_t delayTemp;

	// Algorithm to reach max speed
	// Take the step counts of rising time to use it for descreasing the speed later

	if ((stepTemp < stepNumberTimesTwo - stepCount)) {

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

		stepCount = stepCount + 1;

		delayTemp = 2000 - delayCalc;

		delayTemp = delayTemp / 32;

		if (pwmDivider2Temp < pwmDividerFast * 10) {

			pwmDivider2Temp = pwmDivider2Temp + 1;
			stepTemp = stepCount;

		}
		else if (delayCalc + delayTemp * (pwmDividerFast + 1) >= 2000) {

			__HAL_TIM_SET_AUTORELOAD(&tim11, delayCalc);

		}
		else {

			pwmDividerFast = pwmDividerFast + 1;
			pwmDivider2Temp = 0;
			stepTemp = stepCount;
			__HAL_TIM_SET_AUTORELOAD(&tim11, 2000 - delayTemp * pwmDividerFast);

		}

		HAL_TIM_Base_Start_IT(&tim11);

		// Check if there is plenty of steps to decrease the speed to 0
	}
	else if (stepCount < stepNumberTimesTwo) {

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

		stepCount = stepCount + 1;

		delayTemp = 1800 - delayCalc;

		delayTemp = delayTemp / 32;

		if (pwmDivider2Temp < pwmDividerSlow * 10) {

			pwmDivider2Temp = pwmDivider2Temp + 1;

			HAL_TIM_Base_Start_IT(&tim11);

		}
		else {
			pwmDividerSlow = pwmDividerSlow - 1;
			pwmDivider2Temp = 0;
			__HAL_TIM_SET_AUTORELOAD(&tim11,
				delayCalc + delayTemp * (33 - pwmDividerSlow));

			HAL_TIM_Base_Start_IT(&tim11);

		}

		//If movement is done, stop the timer and give the initials values to variables         
	}
	else {

		stepCount = 0;
		stepTemp = 0;
		pwmDivider2Temp = 0;
		pwmDividerFast = 0;
		pwmDividerSlow = 33;
		HAL_TIM_Base_Stop_IT(&tim11);

	}

}

// FUNCTION      : TIM1_TRG_COM_TIM11_IRQHandler()
// DESCRIPTION   : The interrupt function to handle interrupt
// PARAMETERS    : void
// RETURNS       : void

void TIM1_TRG_COM_TIM11_IRQHandler(void) {

	HAL_TIM_IRQHandler(&tim11);

}

// FUNCTION      : stepperPartTwo()
// DESCRIPTION   : It handles speed the smoothly for changing speed.
// PARAMETERS    : void
// RETURNS       : void

void stepperPartTwo(void) {

	uint32_t delayTemp;
	uint32_t delayCalcTemp, delayCalcTemp2;

	//For initial accelaretion 0 to demanded speed
	//0 to demanded speed divided  in 32 steps
	if (arrayIncrementx == 0 && stepperPartTwoCount == 0) {

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

		delayTemp = 1600 * data[arrayIncrementx][1] / 60;

		delayCalcTemp = 1000000 / delayTemp / 2;

		delayTemp = 1800 - delayCalcTemp;

		delayTemp = delayTemp / 32;

		if (pwmDivider2Temp < pwmDividerFast * 50) {

			pwmDivider2Temp = pwmDivider2Temp + 1;

		}
		else if (delayCalcTemp + delayTemp * (pwmDividerFast + 1) >= 1800) {

			pwmDivider2Temp = 0;

			pwmDividerFast = 1;

			stepperPartTwoCount = 1;

		}
		else {
			pwmDivider2Temp = 0;
			pwmDividerFast = pwmDividerFast + 1;

		}

		__HAL_TIM_SET_AUTORELOAD(&tim11, 1800 - delayTemp * pwmDividerFast);

		HAL_TIM_Base_Start_IT(&tim11);

		//current state to next state divided  in 32 steps
		//If the demanded speed reached, keep it in constant level

	}
	else if (stepperPartTwoCount < 2 * data[arrayIncrementx][2]) {

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

		delayTemp = 1600 * data[arrayIncrementx][1] / 60;

		delayCalcTemp = 1000000 / delayTemp / 2;

		__HAL_TIM_SET_AUTORELOAD(&tim11, delayCalcTemp);

		stepperPartTwoCount = stepperPartTwoCount + 1;

		HAL_TIM_Base_Start_IT(&tim11);

		//Check if the current state is greater or lower than next state
	}
	else {

		//If the current state speed is greater than next state
		//Decrease the speed in 32 steps
		if (data[arrayIncrementx][1] > data[arrayIncrementx + 1][1]) {

			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

			delayTemp = 1600 * data[arrayIncrementx][1] / 60;

			delayCalcTemp = 1000000 / delayTemp / 2;

			delayTemp = 1600 * data[arrayIncrementx + 1][1] / 60;

			delayCalcTemp2 = 1000000 / delayTemp / 2;

			delayTemp = delayCalcTemp2 - delayCalcTemp;

			delayTemp = delayTemp / 32;

			if (pwmDivider2Temp < pwmDividerSlow * 100) {

				pwmDivider2Temp = pwmDivider2Temp + 1;

			}
			else if (delayCalcTemp2 - delayTemp * (38 - pwmDividerSlow)
				<= delayCalcTemp) {

				pwmDivider2Temp = 0;

				pwmDividerSlow = 33;

				stepperPartTwoCount = 1;

				arrayIncrementx = arrayIncrementx + 1;

				rotationDirTemp = 1;

			}
			else {
				pwmDivider2Temp = 0;
				pwmDividerSlow = pwmDividerSlow - 1;

			}

			__HAL_TIM_SET_AUTORELOAD(&tim11,
				delayCalcTemp + delayTemp * (33 - pwmDividerSlow));

			HAL_TIM_Base_Start_IT(&tim11);

			//If the current state speed is lower than next state
			//Decrease the speed in 32 steps
		}
		else if (data[arrayIncrementx][1] < data[arrayIncrementx + 1][1]) {

			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

			delayTemp = 1600 * data[arrayIncrementx][1] / 60;

			delayCalcTemp = 1000000 / delayTemp / 2;

			delayTemp = 1600 * data[arrayIncrementx + 1][1] / 60;

			delayCalcTemp2 = 1000000 / delayTemp / 2;

			delayTemp = delayCalcTemp - delayCalcTemp2;

			delayTemp = delayTemp / 32;

			if (pwmDivider2Temp < pwmDividerFast * 100) {

				pwmDivider2Temp = pwmDivider2Temp + 1;
				s
			}
			else if (delayCalcTemp2 + delayTemp * (pwmDividerFast + 1)
				>= delayCalcTemp) {

				pwmDivider2Temp = 0;

				pwmDividerFast = 1;

				stepperPartTwoCount = 1;

				arrayIncrementx = arrayIncrementx + 1;

				rotationDirTemp = 1;

			}
			else {
				pwmDivider2Temp = 0;
				pwmDividerFast = pwmDividerFast + 1;

			}

			__HAL_TIM_SET_AUTORELOAD(&tim11,
				delayCalcTemp - delayTemp * pwmDividerFast);

			HAL_TIM_Base_Start_IT(&tim11);

		}

	}

}

//Check if the selected mode in interrupt handler
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {

	if (htim == &tim11) {
		if (stepperMode == 1) {
			stepperPartOne();
		}
		else if (stepperMode == 2) {
			stepperPartTwo();
		}

	}
}

