#ifndef _SERVO_H
#define _SERVO_H

class Servo {
  public:
    Servo();
    Servo(uint8_t servo_id);
    void init();
    void update();
    void writeSpeed(int16_t speed, uint8_t acceleration);
    void writePosition(int16_t position, uint16_t speed, uint8_t acceleration);
    void wheelMode();
    void enableTorque(uint8_t enable);
    void calibrationOfs();
  public:
    int pos;
    int speed;
    int torque;
    int voltage;
    int temperature;
    int current;
    bool move;
  private:
    uint8_t _servo_id;
};

void stsInit();

#endif