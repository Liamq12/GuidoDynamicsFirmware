#include "stdio.h"
#include "string.h"

#include <main.h>

#include "udpClientRAW.h"

#include "ADS1115.h"

// ADS1115 I2C address (ADDR pin to GND)
#define ADS1115_ADDRESS 0x48 << 1  // Left shift for HAL

// ADS1115 Register addresses
#define ADS1115_REG_CONVERSION  0x00
#define ADS1115_REG_CONFIG      0x01

// Configuration register bits
#define ADS1115_CFG_OS_START    0x8000  // Start single conversion
#define ADS1115_CFG_MUX_AIN0    0x4000  // AINp = AIN0, AINn = GND
#define ADS1115_CFG_PGA_4_096V  0x0200  // ±4.096V range
//#define ADS1115_CFG_PGA_4_096V  0x0000  // ±4.096V range
#define ADS1115_CFG_MODE_SINGLE 0x0100  // Single-shot mode
#define ADS1115_CFG_DR_128SPS   0x0080  // 128 samples per second
#define ADS1115_CFG_COMP_QUE_DIS 0x0003 // Disable comparator

extern I2C_HandleTypeDef hi2c3;
extern struct dataPacket dataPacketNow;

// Single ended conversion
int16_t ADS1115_ReadChannel(uint8_t channel){

	// Init vars
    uint8_t configData[3];
    uint8_t conversionData[2];
    uint16_t config = 0;
    HAL_StatusTypeDef status;

    // Build configuration word
    config = ADS1115_CFG_OS_START;      // Start conversion

    // Set multiplexer based on channel (single-ended)
    switch(channel) {
        case 0: config |= 0x4000; break;  // AIN0 vs GND
        case 1: config |= 0x5000; break;  // AIN1 vs GND
        case 2: config |= 0x6000; break;  // AIN2 vs GND
        case 3: config |= 0x7000; break;  // AIN3 vs GND
        default: config |= 0x4000; break;
    }

    // Set config register params
    config |= ADS1115_CFG_PGA_4_096V;   // ±4.096V range
    config |= ADS1115_CFG_MODE_SINGLE;  // Single-shot mode
    config |= ADS1115_CFG_DR_128SPS;    // 128 SPS
    config |= ADS1115_CFG_COMP_QUE_DIS; // Disable comparator

    // Write configuration to config register
    configData[0] = ADS1115_REG_CONFIG;
    configData[1] = (config >> 8) & 0xFF;  // MSB
    configData[2] = config & 0xFF;         // LSB

    // HAL transmit function with error reporting
    status = HAL_I2C_Master_Transmit(&hi2c3, ADS1115_ADDRESS, configData, 3, 100);
//    if (status != HAL_OK) {
//        snprintf(dataPacketNow.error,
//                 sizeof(dataPacketNow.error),
//                 "I2C Write Error: %d\r\n",
//                 status);
//        return 0;
//    }

    // Wait for 128 samples to complete (TODO: Make this auto-calculated based on SPS register)
    HAL_Delay(15);

    // Point to conversion register
    uint8_t regAddr = ADS1115_REG_CONVERSION;
    HAL_I2C_Master_Transmit(&hi2c3, ADS1115_ADDRESS, &regAddr, 1, HAL_MAX_DELAY);

    // Read conversion result
    HAL_I2C_Master_Receive(&hi2c3, ADS1115_ADDRESS, conversionData, 2, HAL_MAX_DELAY);

    // Combine bytes (MSB first)
    int16_t result = (conversionData[0] << 8) | conversionData[1];

    return result;
}

double ADS1115_ConvertToVoltage(int16_t raw)
{
    // For ±4.096V range, LSB = 0.125 mV
    // Maximum value is ±32767
    double voltage = raw * 0.000125;  // 0.125 mV per bit
    return voltage;
}

void ADS1115_Diagnostics(void){
    uint8_t configData[2];
    HAL_StatusTypeDef status;

    uint8_t devicesFound = 0;
    for (uint8_t i = 1; i < 128; i++) {
        status = HAL_I2C_IsDeviceReady(&hi2c3, i << 1, 1, 10);
        if (status == HAL_OK) {
        	devicesFound++;
        }
    }
//	snprintf(dataPacketNow.error,
//			 sizeof(dataPacketNow.error),
//			 "I2C Write Error: %d\r\n",
//			 devicesFound);
}
