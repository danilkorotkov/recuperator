     
#ifndef FANCLASS_H
#define FANCLASS_H

#include "HomeSpan.h" 
#include "extras/PwmPin.h"

#define ROTATION_SPEED       0
#define MIN_SPEED            1000
#define MAX_SPEED            5300
#define STEP_SPEED           100

#define ON                   1
#define OFF                  0

#define INTAKE               0
#define OUTTAKE              1

#define sINACTIVE            0
#define sIDLE                1
#define sBLOW                2

#define tMANUAL              0
#define tAUTO                1

#define SpeedPin             13

struct RECUP;

extern struct WordStruct {
    String inTemp     = "21.5ºC";
    String outTemp    = "21.5ºC";
    String Speed      = "0";
    String Status     = "Выкл";
}LCDoutput;


struct RECUP : Service::Fan {         // First we create a derived class from the HomeSpan 

  SpanCharacteristic *Active;              // here we create a generic pointer to a SpanCharacteristic 
  SpanCharacteristic *RotationDirection;
  SpanCharacteristic *RotationSpeed;
  SpanCharacteristic *Name;
  SpanCharacteristic *CurrentFanState;
  SpanCharacteristic *TargetFanState;
  LedPin             *pwmPin;
     
  RECUP();
  boolean update();                                   
  //void loop();
  void PollCurrentState();
  void inc();
  void dec();
  void setSpeed();
  void OnOff();
  void setFanState();
  void lcdStatus();
};

//////

#endif
