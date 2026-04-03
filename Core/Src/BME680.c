/**
 * Author: Liam Homburger
 * Senior Design: F25-66 Vehicle Dynamics Test Hardware
 *
 * If you are reading this, shit is prob fucked. This code is derived from the Bosch BME680
 * datasheet which describes the specific memory mapping and formulas for calculating values
 * based on the factory calibration values. Reference the chip datasheet for a good explaination
 * on this system.
 */

#include "stdio.h"
#include "string.h"
#include <stdint.h>

#include <main.h>

#include "udpClientRAW.h"

#include "BME680.h"

// BME680 I2C address (ADDR pin to GND)
#define BME680_ADDR 0x76 << 1  // Left shift for HAL

// Register addresses
#define CTRL_MEAS 0x74
#define CTRL_HUM 0x72
#define CONFIG 0x75

// Oversampling settings. Higher is more accurate but slower
#define HUMIDITY_OS 0x3 // x4
#define TEMP_OS 0x3 // x4
#define PRESS_OS 0x3 //x4

// IIR filter coefficient
#define IIR 0x2 // IIR Coeff 3

extern I2C_HandleTypeDef hi2c3;
extern struct dataPacket dataPacketNow;

// Calibration block addresses
#define BME680_CALIB1_ADDR  0x8A
#define BME680_CALIB1_LEN   23

#define BME680_CALIB2_ADDR  0xE1
#define BME680_CALIB2_LEN   10

// ADC register starts
#define TEMP_ADC_ADDR 0x22
#define PRESSURE_ADC_ADDR 0x1F
#define HUMID_ADC_ADDR 0x25

// Calibration parameters, cached on startup
struct BME680_Calib calibData;

uint8_t tempData[3];
uint8_t pressData[3];
uint8_t humidData[2];

void BME680_Init(){
	uint8_t data[6];
	uint8_t indexData = 0x0;

	uint8_t buf1[BME680_CALIB1_LEN];
	uint8_t buf2[BME680_CALIB2_LEN];
	HAL_StatusTypeDef ret;

    HAL_StatusTypeDef status;

	data[0] = CTRL_HUM;
	data[1] = HUMIDITY_OS;
	data[2] = CTRL_MEAS;
	data[3] = (TEMP_OS << 5) | (PRESS_OS << 2);
	data[4] = CONFIG;
	data[5] = (IIR << 2);

	status = HAL_I2C_Master_Transmit(&hi2c3, BME680_ADDR, data, 6, 100);
    HAL_Delay(15);

    // --- Read block 1 (0x8A to 0xA0) ---
    ret = HAL_I2C_Mem_Read(&hi2c3, BME680_ADDR,
						   BME680_CALIB1_ADDR, I2C_MEMADD_SIZE_8BIT,
						   buf1, BME680_CALIB1_LEN, HAL_MAX_DELAY);
//    if (ret != HAL_OK) return ret;

    // --- Read block 2 (0xE1 to 0xF0) ---
    ret = HAL_I2C_Mem_Read(&hi2c3, BME680_ADDR,
						   BME680_CALIB2_ADDR, I2C_MEMADD_SIZE_8BIT,
						   buf2, BME680_CALIB2_LEN, HAL_MAX_DELAY);
//    if (ret != HAL_OK) return ret;

    calibData.par_t1 = (uint16_t)(buf2[9]  << 8) | buf2[8];
    calibData.par_t2 = (int16_t) (buf1[1] << 8) | buf1[0];
    calibData.par_t3 = (int8_t)   buf1[2];

    calibData.par_p1  = (uint16_t)(buf1[5] << 8) | buf1[4];
    calibData.par_p2  = (int16_t) (buf1[7] << 8) | buf1[6];
    calibData.par_p3  = (int8_t)   buf1[8];
    calibData.par_p4  = (int16_t) (buf1[11] << 8) | buf1[10];
    calibData.par_p5  = (int16_t) (buf1[13] << 8) | buf1[12];
    calibData.par_p6  = (int8_t)   buf1[15];
    calibData.par_p7  = (int8_t)   buf1[14];
    calibData.par_p8  = (int16_t) (buf1[19] << 8) | buf1[18];
    calibData.par_p9  = (int16_t) (buf1[21] << 8) | buf1[20];
    calibData.par_p10 = (uint8_t)  buf1[22];

    calibData.par_h1 = (uint16_t)((buf2[2] << 4) | (buf2[1] & 0x0F));
    calibData.par_h2 = (uint16_t)((buf2[0] << 4) | (buf2[1] >> 4));
    calibData.par_h3 = (int8_t)   buf2[3];
    calibData.par_h4 = (int8_t)   buf2[4];
    calibData.par_h5 = (int8_t)   buf2[5];
    calibData.par_h6 = (uint8_t)  buf2[6];
    calibData.par_h7 = (int8_t)   buf2[7];
}
void BME680_ForceTrigger(){
    HAL_I2C_Mem_Read(&hi2c3, BME680_ADDR,
    				 TEMP_ADC_ADDR, I2C_MEMADD_SIZE_8BIT,
    				 tempData, 3, HAL_MAX_DELAY);

    HAL_I2C_Mem_Read(&hi2c3, BME680_ADDR,
    				 PRESSURE_ADC_ADDR, I2C_MEMADD_SIZE_8BIT,
    				 pressData, 3, HAL_MAX_DELAY);

    HAL_I2C_Mem_Read(&hi2c3, BME680_ADDR,
    				 HUMID_ADC_ADDR, I2C_MEMADD_SIZE_8BIT,
    				 humidData, 2, HAL_MAX_DELAY);
    uint8_t data[2];
	HAL_StatusTypeDef ret;

    data[0] = CTRL_MEAS;
    data[1] = (TEMP_OS << 5) | (PRESS_OS << 2) | 0x1;

    ret= HAL_I2C_Master_Transmit(&hi2c3, BME680_ADDR, data, 2, 100);
}
// Bunch of math based on datasheet to apply factory compensation
float BME680_ReadTemp(){
	uint32_t temp_adc = ((uint32_t)tempData[0] << 12) | ((uint32_t)tempData[1] << 4) | ((uint32_t)tempData[2] >> 4);
	float var1 = (((float)temp_adc / 16384.0f) - ((float)calibData.par_t1 / 1024.0f)) * (float)calibData.par_t2;
	float var2 = ((((float)temp_adc / 131072.0f) - ((float)calibData.par_t1 / 8192.0f)) * (((float)temp_adc / 131072.0f) - ((float)calibData.par_t1 / 8192.0f))) * ((float)calibData.par_t3 * 16.0f);
	float t_fine = var1 + var2;
	float temp_comp = t_fine / 5120.0f;
	return (float) temp_comp;
}
float BME680_ReadPressure(){
	uint32_t press_adc = ((uint32_t)pressData[0] << 12) | ((uint32_t)pressData[1] << 4) | ((uint32_t)pressData[2] >> 4);
	uint32_t temp_adc = ((uint32_t)tempData[0] << 12) | ((uint32_t)tempData[1] << 4) | ((uint32_t)tempData[2] >> 4);
	// Recalculate t_fine incase ReadTemp is not called, and FPU ops are computationally cheap
	float var1 = (((float)temp_adc / 16384.0f) - ((float)calibData.par_t1 / 1024.0f)) * (float)calibData.par_t2;
	float var2 = ((((float)temp_adc / 131072.0f) - ((float)calibData.par_t1 / 8192.0f)) * (((float)temp_adc / 131072.0f) - ((float)calibData.par_t1 / 8192.0f))) * ((float)calibData.par_t3 * 16.0f);
	float t_fine = var1 + var2;

	var1 = ((float)t_fine / 2.0f) - 64000.0f;
	var2 = var1 * var1 * ((float)calibData.par_p6 / 131072.0f);
	var2 = var2 + (var1 * (float)calibData.par_p5 * 2.0f);
	var2 = (var2 / 4.0f) + ((float)calibData.par_p4 * 65536.0f);
	var1 = ((((float)calibData.par_p3 * var1 * var1) / 16384.0f) + ((float)calibData.par_p2 * var1)) / 524288.0f;
	var1 = (1.0f + (var1 / 32768.0)) * (float) calibData.par_p1;
	float press_comp = 1048576.0f - (float) press_adc;
	press_comp = ((press_comp - (var2 / 4096.0f)) * 6250.0f) / var1;
	var1 = ((float)calibData.par_p9 * press_comp * press_comp) / 2147483648.0f;
	var2 = press_comp * ((float)calibData.par_p8 / 32768.0f);
	float var3 = (press_comp / 256.0f) * (press_comp / 256.0f) * (press_comp / 256.0f) * (calibData.par_p10 / 131072.0f);
	press_comp = press_comp + (var1 + var2 + var3 + ((float)calibData.par_p7 * 128.0f)) / 16.0f;
	return press_comp;
}
float BME680_ReadHumidity(float temp_comp){
	uint32_t hum_adc = ((uint32_t)humidData[0] << 8) | (uint32_t)humidData[1];
	float var1 = hum_adc - (((float)calibData.par_h1 * 16.0f) + (((float)calibData.par_h3 / 2.0f) * temp_comp));
	float var2 = var1 * (((float)calibData.par_h2 / 262144.0f) * (1.0f + (((float)calibData.par_h4 / 16384.0f) * temp_comp) + (((float)calibData.par_h5 / 1048576.0f) * temp_comp * temp_comp)));
	float var3 = (float)calibData.par_h6 / 16384.0f;
	float var4 = (float)calibData.par_h7 / 2097152.0f;
	float hum_comp = var2 + ((var3 + (var4 * temp_comp)) * var2 * var2);
	return hum_comp;
}
// This requires heating the gas sensor for some period of time, which significantly slows down sample rate and delays the rest of the program. Can be implemented smartly, but not necessary given the EHS sensors in the dyno bay
float BME680_ReadVOC(){
	return -1.0f;
}
