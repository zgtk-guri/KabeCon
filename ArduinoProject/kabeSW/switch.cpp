#include "switch.h"
#include <Arduino.h>
#include <Servo.h>

Switch::Switch(Servo &servo, int powerPin, int offPosition, int onPosition, int standbyPosition):
    servo(servo),
    powerPin(powerPin),
    offPosition(offPosition),
    onPosition(onPosition),
    standbyPosition(standbyPosition)
{
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
}

void Switch::on(){
  isOn = true;
  move(onPosition);
}

void Switch::off(){
  isOn = false;
  move(offPosition);
}

void Switch::home(){
  digitalWrite(powerPin, HIGH); //サーボを動かすときは電源を入れます
  servo.write(standbyPosition);
  delay(500);
  digitalWrite(powerPin, LOW);  //動かし終わったら電源を切ります
}

void Switch::move(int position){
  digitalWrite(powerPin, HIGH); //サーボを動かすときは電源を入れます
  servo.write(position);
  delay(1000);
  servo.write(standbyPosition);
  delay(500);
  digitalWrite(powerPin, LOW);  //動かし終わったら電源を切ります
}

void Switch::setPositions(int offPosition, int onPosition, int standbyPosition){
  this->offPosition = offPosition;
  this->onPosition = onPosition;
  this->standbyPosition = standbyPosition;
}

