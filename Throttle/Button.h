#ifndef _BUTTON_H
#define _BUTTON_H

class Button {
  public:
    Button();
    Button(uint8_t pin);
    void init();
    unsigned long holdTime();
    void update();
  public:
    bool state;
    bool holding = false;
    unsigned long hold_start;
  private:
    uint8_t _pin;
};

#endif