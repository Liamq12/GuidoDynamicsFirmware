#ifndef INC_ADS124S06_H_
#define INC_ADS124S06_H_

void ADS124S06_WriteRegister(uint8_t reg_addr, uint8_t value);
uint8_t ADS124S06_ReadRegister(uint8_t reg_addr);
void ADS124S06_TestPGA(void);
void ADS124S06_ReadID(void);
void ADS124S06_Init(void);
float ADS124S06_ConvertToVoltage(int32_t raw, float offset);

#endif /* INC_ADS124S06_H_ */
