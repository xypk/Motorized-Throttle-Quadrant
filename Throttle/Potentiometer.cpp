#include <Arduino.h>
#include <Potentiometer.h>
#include <Mapf.h>
  
Potentiometer::Potentiometer(uint8_t pin) {
  _pin = pin;
}

Potentiometer::Potentiometer(uint8_t pin, uint16_t max_val) {
  _pin = pin;
  _max_val = max_val;
}

void Potentiometer::init() {
  pinMode(_pin, INPUT);
}

void Potentiometer::update() {
  current_val = analogRead(_pin);
  scaled_val = mapf(current_val, 0, _max_val, 0, 1);
}