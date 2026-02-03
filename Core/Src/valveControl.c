/******************************************************************************
 * @file    valveControl.c
 * @brief   Water brake stepper control logic
 *
 * Author: Liam Homburger
 * Project: Senior Design Team - Guido Dynamics (F25-66)
 * GitHub: https://github.com/Liamq12/GuidoDynamicsFirmware
 *
 * Created on: 2/2/2026
 * Last Modified: 2/2/2026
 *****************************************************************************/

#include "stdio.h"
#include "string.h"

#include <main.h>

#include "valveControl.h"

int pulseDiff;
int pulsesToGo;

extern struct valveData valveData;

void valveControlLoop(void){
	if(valveData.position != valveData.targetPosition){
		valveData.pulseDiff = valveData.targetPosition - valveData.position;
		valveData.position = valveData.targetPosition;
		int direction = 1;
		if(valveData.polarity >= 1){
			direction = 1;
		}else{
			direction = 0;
		}
		generatePulses(valveData.pulseDiff, direction);
	}
	valveData.intFlag = 0;
}

void generatePulses(int pulses, int direction){
	HAL_GPIO_WritePin(DIO2_GPIO_Port, DIO2_Pin, direction);
	HAL_GPIO_WritePin(DIO3_GPIO_Port, DIO3_Pin, 0); // start low
	pulsesToGo = 2*pulses; // Two times the number of pulses, due to toggle logic
}
