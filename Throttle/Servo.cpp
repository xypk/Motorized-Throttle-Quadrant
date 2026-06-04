#include <Arduino.h>
#include <Servo.h>
#include <SCServo.h>

SMS_STS sts;

Servo::Servo() {
  _servo_id = 1;
}

Servo::Servo(uint8_t servo_id) {
  _servo_id = servo_id;
}

void Servo::init() {
  sts.EnableTorque(_servo_id, 1);
}

void stsInit() {
  sts.pSerial = &Serial1;
}

void Servo::update() {
  Serial1.read();

  if(sts.FeedBack(_servo_id)!=-1){
    pos = sts.ReadPos(_servo_id);
    speed = sts.ReadSpeed(_servo_id);
    torque = sts.ReadLoad(_servo_id);
    voltage = sts.ReadVoltage(_servo_id);
    temperature = sts.ReadTemper(_servo_id);
    move = sts.ReadMove(_servo_id);
    current = sts.ReadCurrent(_servo_id);
  }
}

void Servo::writeSpeed(int16_t speed, uint8_t acceleration) {
  sts.WriteSpe(_servo_id, speed, acceleration);
}

void Servo::writePosition(int16_t position, uint16_t speed, uint8_t acceleration) {
  sts.WritePosEx(_servo_id, position, speed, acceleration);
}

void Servo::wheelMode()  {
  sts.WheelMode(_servo_id);
}

void Servo::enableTorque(uint8_t enable) {
  sts.EnableTorque(_servo_id, enable);
}

void Servo::calibrationOfs() {
  sts.CalibrationOfs(_servo_id);
}