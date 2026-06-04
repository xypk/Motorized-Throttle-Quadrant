#include <Arduino.h>
#include <Button.h>

Button::Button() {
  
}

Button::Button(uint8_t pin) {
  _pin = pin;
}

void Button::init() {
  pinMode(_pin, INPUT_PULLUP);
}

unsigned long Button::holdTime() {
  if(holding) {
    return millis() - hold_start;
  } else {
    return 0;
  }
}

void Button::update() {
  bool current_state = !digitalRead(_pin);
  if(current_state) {
    if(!holding) {
      hold_start = millis();
      holding = true;
    }
  } else {
    holding = false;
  }
  state = current_state;
}