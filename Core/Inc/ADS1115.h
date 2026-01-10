#ifndef INC_ADS1115_H_
#define INC_ADS1115_H_

int16_t ADS1115_ReadChannel(uint8_t channel);
double ADS1115_ConvertToVoltage(int16_t raw);
void ADS1115_Diagnostics(void);

#endif /* INC_ADS1115_H_ */
