#ifndef __SWITCH__
#define __SWITCH__

class Servo;

class Switch{
public:
  Switch(Servo &servo, int powerPin, int offPosition, int onPosition, int standbyPosition);

  void on();
  void off();
  void turn(){ if(isOn) off(); else on(); }
  void home();

  void setPositions(int offPosition, int onPosition, int standbyPosition);

private:
  bool isOn = false;
  Servo &servo;
  const int powerPin;
  int offPosition, onPosition, standbyPosition;

  void move(int position);
};

#endif
