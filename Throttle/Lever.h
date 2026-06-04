#ifndef _LEVER_H
#define _LEVER_H

#include <Servo.h>
#include <Button.h>

class Lever {
  public:
    Lever(Servo servo);
    Lever(Servo servo, Button btn);
    void calibrate(int zero_pos, int max_pos, int rev_pos);
    void update();
    void writeSpeed(int16_t speed, uint8_t acceleration);
    void setPosition(float position, uint16_t speed, uint8_t acceleration);
  public:
    float value; // [-100,100] (-) means rev
    bool btn_down;
  private:
    int _rev_pos;
    int _zero_pos;
    int _max_pos;
    Servo _servo;
    Button _btn;
};

#endif