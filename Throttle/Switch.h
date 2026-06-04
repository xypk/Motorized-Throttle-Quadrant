#ifndef _SWITCH_H
#define _SWITCH_H

class Switch {
  public:
    Switch(uint8_t on_pin, uint8_t off_pin);
    void init();
    unsigned long holdTime();
    void update();
  public:
    bool state;
    bool holding = false;
  private:
    uint8_t _on_pin;
    uint8_t _off_pin;
    unsigned long _hold_start;
};

#endif