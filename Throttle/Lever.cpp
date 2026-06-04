#include <Arduino.h>
#include <Lever.h>
#include <Servo.h>
#include <Button.h>
#include <Mapf.h>

Lever::Lever(Servo servo) {
  _servo = servo;
}

Lever::Lever(Servo servo, Button btn) {
  _servo = servo;
  _btn = btn;
}

void Lever::calibrate(int zero_pos, int max_pos, int rev_pos) {
  _rev_pos = rev_pos;
  _max_pos = max_pos;
  _zero_pos = zero_pos;
}

void Lever::update() {
  float mapped_pos;
  _servo.update();
  int servo_pos = _servo.pos;
  mapped_pos = mapf(servo_pos, _zero_pos, _max_pos, 0, 100);
  if(mapped_pos < -0.5) { // prevent unintentional engaging
    mapped_pos = mapf(servo_pos, _zero_pos, _rev_pos, 0, -100);
    value = constrain(mapped_pos, -100, 0);
  } else {
    value = constrain(mapped_pos, 0, 100);
  }
}

void Lever::writeSpeed(int16_t speed, uint8_t acceleration) {
  _servo.writeSpeed(speed, acceleration);
}

void Lever::setPosition(float position, uint16_t speed, uint8_t acceleration) {
  int16_t target_position = mapf(constrain(position, 0, 100), 0, 100, _zero_pos, _max_pos);
  _servo.writePosition(target_position, speed, acceleration);
}