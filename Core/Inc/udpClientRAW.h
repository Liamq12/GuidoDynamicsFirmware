#ifndef INC_UDPCLIENTRAW_H_
#define INC_UDPCLIENTRAW_H_

struct dataPacket {
    double force;
    double RPM;
    double temp;
    double humidity;
    double pressure;
    double voc;
    double acceleration;
    char error[100];
};

void udpClient_send(void);
void udpClient_connect(void);


#endif /* INC_UDPCLIENTRAW_H_ */
