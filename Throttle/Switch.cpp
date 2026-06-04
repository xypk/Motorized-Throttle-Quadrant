#include <Arduino.h>
#include <Switch.h>

Switch::Switch(uint8_t on_pin, uint8_t off_pin) {
  _on_pin = on_pin;
  _off_pin = off_pin;
}

void Switch::init() {
  pinMode(_on_pin, INPUT_PULLUP);
  pinMode(_off_pin, INPUT_PULLUP);
}

unsigned long Switch::holdTime() {
  if (holding) {
    return millis() - _hold_start;
  } else {
    return 0;
  }
}
void Switch::update() {
  bool current_state;
  bool on_read = !digitalRead(_on_pin);
  bool off_read = !digitalRead(_off_pin);

  if (on_read) {
    state = 1;
  }
  else {
    if (!off_read) {
      holding = false;
    }
    state = 0;
  }

  if ((on_read || off_read) && !holding) {
    _hold_start = millis();
    holding = true;
  }
}