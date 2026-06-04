#ifndef _POTENTIOMETER_H
#define _POTENTIOMETER_H

class Potentiometer {
  public:
    Potentiometer(uint8_t pin);
    Potentiometer(uint8_t pin, uint16_t max_val);
    void init();
    void update();
  public:
    uint16_t current_val;
    float scaled_val;
  private:
    uint8_t _pin;
    int _max_val = 1023;
};

#endif