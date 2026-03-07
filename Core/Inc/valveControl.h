#ifndef INC_VALVECONTROL_H_
#define INC_VALVECONTROL_H_

void zeroValve();
void valveControlLoop();
void generatePulses(int pulses, int direction);

void PID_OP_PT();
void ramp();

#endif /* INC_VALVECONTROL_H_ */
