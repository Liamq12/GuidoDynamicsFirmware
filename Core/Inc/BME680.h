#ifndef INC_BME680_H_
#define INC_BME680_H_

void BME680_Init();
float BME680_ReadTemp();
float BME680_ReadPressure();
float BME680_ReadHumidity(float temp_comp);
float BME680_ReadVOC();

struct BME680_Calib {
    // Temperature
    uint16_t par_t1;
    int16_t  par_t2;
    int8_t   par_t3;

    // Pressure
    uint16_t par_p1;
    int16_t  par_p2;
    int8_t   par_p3;
    int16_t  par_p4;
    int16_t  par_p5;
    int8_t   par_p6;
    int8_t   par_p7;
    int16_t  par_p8;
    int16_t  par_p9;
    uint8_t  par_p10;

    // Humidity
    uint16_t par_h1;
    uint16_t par_h2;
    int8_t   par_h3;
    int8_t   par_h4;
    int8_t   par_h5;
    uint8_t  par_h6;
    int8_t   par_h7;
};

#endif /* INC_BME680_H_ */
