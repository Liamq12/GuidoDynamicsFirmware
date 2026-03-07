/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DIO3_Pin GPIO_PIN_0
#define DIO3_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_14
#define LED_GPIO_Port GPIOE
#define DIO2_Pin GPIO_PIN_12
#define DIO2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
struct valveData {
    int pulsesPerRev;
    double gearReduction;
    int upperBound;
    int isZero;
    int position;
    int targetPosition;
    int targetPositionSteps;
    int polarity;
    int intFlag;
    int pulseDiff;
    double positionInSteps;
    int zeroRoutine;
};

struct PID_Data {
	float RPM_Idle; // "Start" RPM
    float RPM_Target; // PID RPM target
    float RPM_End_Target; // RPM at the end
    float RPM_Ramp_Rate; // RPM increase per second
	int RPM_Count; // RPM cycle count
    int RPM_EN;
    int RPM_RAMP_EN;
    int RPM_Flag;
    float KP; // Proportional term
    float KI; // Integral term
    float KD; // Derivative term
    float accum; // Integral accumulation
    float accumMax; // Max I accumulation
    float rpms[10]; // Collection of previous rpms for ROC calcs
    float avgRPMROC; // Average RPM rate of change
    int RPMS_Flag; // Tells when to store RPM value
};

struct AlarmThresholds {
	double RPM_Alarm;
	double Torque_Alarm;
	double Power_Alarm;
};

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
