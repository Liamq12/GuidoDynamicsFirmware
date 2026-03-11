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
#include "math.h"

#include <main.h>

#include "valveControl.h"
#include "udpClientRAW.h"

#define CLAMP(x, min, max)  ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

int pulseDiff;
float pulsesToGo;
int isZeroing;

extern struct valveData valveData;
extern struct PID_Data PID_Data;
extern struct dataPacket dataPacketNow;

void valveControlLoop(void){
	if(valveData.position != valveData.targetPosition){
		valveData.pulseDiff = valveData.targetPosition - valveData.position;
		valveData.position = valveData.targetPosition;
		int direction = 1;
		//HAL_NVIC_EnableIRQ(TIM4_IRQn);
		if(valveData.polarity >= 1 && valveData.pulseDiff >= 1){
			direction = 1;
			generatePulses(abs(valveData.pulseDiff), direction); // Put this in if statement to skip if not valid
		}else if(valveData.polarity >= 1 && valveData.pulseDiff <= 0){
			direction = 0;
			generatePulses(abs(valveData.pulseDiff), direction); // Put this in if statement to skip if not valid
		}else if(valveData.polarity <= 0 && valveData.pulseDiff <= 0){
			direction = 1;
			generatePulses(abs(valveData.pulseDiff), direction); // Put this in if statement to skip if not valid
		}else if(valveData.polarity <= 0 && valveData.pulseDiff >= 1){
			direction = 0;
			generatePulses(abs(valveData.pulseDiff), direction);
		}
	}
	valveData.intFlag = 0;
}

void generatePulses(int pulses, int direction){
	HAL_GPIO_WritePin(DIO2_GPIO_Port, DIO2_Pin, direction);
	HAL_GPIO_WritePin(DIO3_GPIO_Port, DIO3_Pin, 0); // start low
	pulsesToGo = 2*pulses; // Two times the number of pulses, due to toggle logic
}

//void zeroValve(){
//	isZeroing = 1;
//}

void ramp(){

}

void PID_OP_PT(){
	// PID_Data.RPM_Target = ((PID_Data.RPM_Count/100) * PID_Data.RPM_Ramp_Rate) + PID_Data.RPM_Idle;
	float error = PID_Data.RPM_Target - dataPacketNow.RPM;

	PID_Data.accum += error * (-PID_Data.KP*PID_Data.KI);
	PID_Data.accum = CLAMP(PID_Data.accum, -PID_Data.accumMax, PID_Data.accumMax);

	float sumOfROC = 0.0f;
	for(uint8_t i = 0; i < (sizeof(PID_Data.rpms)/sizeof(PID_Data.rpms[0]))-1; i++){
		sumOfROC += PID_Data.rpms[i] - PID_Data.rpms[i+1];
	}
	sumOfROC = sumOfROC/((sizeof(PID_Data.rpms)/sizeof(PID_Data.rpms[0]))-1);
	PID_Data.avgRPMROC = sumOfROC;

	float correction = (error*(-1)*PID_Data.KP) + (PID_Data.accum) - (PID_Data.avgRPMROC*(PID_Data.KD * PID_Data.KP));
	if(correction <= 0.0){
		correction = 0.0f;
	}else if(correction >= 1.0){
		correction = 1.0f;
	}

	//if(dataPacketNow.RPM >= 500){
//	correction /= 100;
//	correction = CLAMP(correction, 0.0f, 1.0f);
//	correction *= (float) (600/dataPacketNow.RPM);
	//}
	valveData.targetPosition = ((powf(10,correction)-1)/9)*100;
//	valveData.targetPosition = valveData.position - correction; // Try adding factor that scales down the correction based on RPM
}
