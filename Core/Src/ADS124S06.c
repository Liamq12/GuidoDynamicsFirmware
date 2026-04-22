#include "stdio.h"
#include "string.h"

#include <main.h>

#include "ADS124S06.h"

#define ADS124S06_REG_ID     0x00
#define ADS124S06_REG_STATUS 0x01
#define ADS124S06_REG_INMUX  0x02
#define ADS124S06_REG_PGA    0x03   // PGA Setting Register (Gain + PGA enable)
#define ADS124S06_REG_RATE   0x04
#define ADS124S06_REG_REFCTL 0x05
#define ADS124S06_REG_EXITE1 0x06   // Excitation register 1, defaults fine for load cells
#define ADS124S06_REG_EXITE2 0x07   // Excitation register 2, defaults fine for load cells
#define ADS124S06_REG_BIAS   0x08   // Bias register, defaults fine for load cells
#define ADS124S06_REG_SYSCTL 0x09

#define ADS124S06_CMD_WREG   0x40   // Write Register command
#define ADS124S06_CMD_RREG   0x20   // Read Register command
#define ADS124S06_CMD_START  0x08   // Start Conversions command
#define ADS124S06_CMD_RDATA  0x12   // Read Data command

extern SPI_HandleTypeDef hspi1;

char id_buffer[64];

/**
 * Write one register
 */
void ADS124S06_WriteRegister(uint8_t reg_addr, uint8_t value)
{
    uint8_t tx[3];

    tx[0] = ADS124S06_CMD_WREG | (reg_addr & 0x1F);  // WREG + address
    tx[1] = 0x00;                                     // Write 1 register (N-1 = 0)
    tx[2] = value;

    HAL_SPI_TransmitReceive(&hspi1, tx, tx, 3, HAL_MAX_DELAY);  // rx buffer can reuse tx since we don't care about reply here
}

/**
 * Read one register
 */
uint8_t ADS124S06_ReadRegister(uint8_t reg_addr)
{
    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};

    tx[0] = ADS124S06_CMD_RREG | (reg_addr & 0x1F);   // RREG + address
    tx[1] = 0x00;                                      // Read 1 register
    tx[2] = 0xFF;                                      // Dummy byte

    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, HAL_MAX_DELAY);

    return rx[2];   // Data comes back in the 3rd byte
}

/**
 * Example: Set PGA gain and read it back
 */
void ADS124S06_TestPGA(void)
{
    uint8_t written_gain = 0x06;   // PGA gain = 32 (see table below)
    uint8_t readback;

    // Optional small delay after power-up
    HAL_Delay(10);

    // Write new PGA setting (PGA enabled + gain)
    ADS124S06_WriteRegister(ADS124S06_REG_PGA, written_gain);

    // Small delay for register to settle
    HAL_Delay(5);

    // Read it back
    readback = ADS124S06_ReadRegister(ADS124S06_REG_PGA);

    // Format result
    snprintf(id_buffer, sizeof(id_buffer),
             "PGA Write: 0x%02X (Gain=32)\r\nPGA Readback: 0x%02X",
             written_gain, readback);
}

void ADS124S06_ReadID(void)
{
    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};
    uint8_t id;

    // RREG command for register 0x00 (ID), read 1 register
    tx[0] = 0x20 | ADS124S06_REG_ID;   // 0x20
    tx[1] = 0x00;                      // Number of registers - 1

    // Use one TransmitReceive call — this is the most reliable way with HAL
    if (HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, HAL_MAX_DELAY) != HAL_OK)
    {
        snprintf(id_buffer, sizeof(id_buffer), "SPI Error");
        return;
    }

    id = rx[2];   // The actual ID byte comes back in the third position

    snprintf(id_buffer, sizeof(id_buffer), "ID: 0x%02X", id);
}

void ADS124S06_Init(void){
	HAL_Delay(10); // Let power rails settle
	uint8_t readback = 0;

    uint8_t writeMuxSel = 0x01;   // AIN0 is MUXP, AIN1 is MUXN
    ADS124S06_WriteRegister(ADS124S06_REG_INMUX, writeMuxSel);

    HAL_Delay(5); // Settle
    readback = ADS124S06_ReadRegister(ADS124S06_REG_INMUX);

    HAL_Delay(5);

    uint8_t writePGASel = 0x0F;
    ADS124S06_WriteRegister(ADS124S06_REG_PGA, writePGASel);

    HAL_Delay(5);
    readback = ADS124S06_ReadRegister(ADS124S06_REG_PGA);

    HAL_Delay(5);

    uint8_t writeDataRate = 0x16; // 60 SPS, Low Latency Filter, 60Hz notch
    ADS124S06_WriteRegister(ADS124S06_REG_RATE, writeDataRate);

    HAL_Delay(5);
    readback = ADS124S06_ReadRegister(ADS124S06_REG_RATE);

    HAL_Delay(5);

    uint8_t writeRefControl = 0x3A; // Internal 2.5V reference, always on, no buffers
    ADS124S06_WriteRegister(ADS124S06_REG_REFCTL, writeRefControl);

    HAL_Delay(5);
    readback = ADS124S06_ReadRegister(ADS124S06_REG_REFCTL);

    HAL_Delay(5);

    uint8_t cmd = ADS124S06_CMD_START;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
}

int32_t ADS124S06_ReadData(void)
{
    uint8_t cmd[1] = {ADS124S06_CMD_RDATA};
    uint8_t rx[3] = {0};
    int32_t raw;

    while (HAL_GPIO_ReadPin(DRDY_GPIO_Port, DRDY_Pin) == GPIO_PIN_SET);

    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);

//    HAL_Delay(30);

    HAL_SPI_Receive(&hspi1, rx, 3, HAL_MAX_DELAY);

    raw = ((int32_t)rx[0] << 16) |
          ((int32_t)rx[1] << 8)  |
           (int32_t)rx[2];

    // sign extend 24-bit
    if (raw & 0x800000)
        raw |= 0xFF000000;

    return raw;
}

float ADS124S06_ConvertToVoltage(int32_t raw, float offset){
	return (((float)raw / 8388607.0f) * ((2.5f * 100 / 128.0f))) + offset;
}
