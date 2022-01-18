#include "FanClass.h"
float pwmData;
unsigned long HSCurrentTime     = 0;   // опрос
////////////////////////////
   
RECUP::RECUP() : Service::Fan(){

    LOG1("Constructing Fan…\n");
    Active              = new Characteristic::Active(OFF, true);
    RotationDirection   = new Characteristic::RotationDirection(OUTTAKE, true);
    RotationSpeed       = new Characteristic::RotationSpeed(1000, true);
    Name                = new Characteristic::Name("recuperator");
    CurrentFanState     = new Characteristic::CurrentFanState(sIDLE, true);
    TargetFanState      = new Characteristic::TargetFanState(tAUTO, true);

    this->pwmPin        = new LedPin(SpeedPin, 50, 25000);

    RotationSpeed->setRange(MIN_SPEED,MAX_SPEED,STEP_SPEED); //sets the range to be from a min of 1000 to a max of 5300, in steps of 1000
                                                              //speed = 108 * duty - 5300
                                                              //duty  = (speed + 5300) / 108
   
/*                         
    pinMode(this->OpenPin,OUTPUT); 
    digitalWrite(this->OpenPin,LOW);
    ButtonArray[0] = this->OpenPin;
                                              
    pinMode(this->ClosePin,OUTPUT);
    digitalWrite(this->ClosePin,LOW);
    ButtonArray[1] = this->ClosePin;  
                      
    pinMode(this->StopPin,OUTPUT);
    digitalWrite(this->StopPin,LOW);
    ButtonArray[2] = this->StopPin;
    
    pinMode(this->ClSensorPin.PIN, INPUT_PULLUP);
    pinMode(this->OpSensorPin.PIN, INPUT_PULLUP);
    pinMode(this->ObSensorPin.PIN, INPUT_PULLUP);
    attachInterruptArg(this->ClSensorPin.PIN, isr, &(this->ClSensorPin), CHANGE);
    attachInterruptArg(this->OpSensorPin.PIN, isr, &(this->OpSensorPin), CHANGE);
    attachInterruptArg(this->ObSensorPin.PIN, isr, &(this->ObSensorPin), CHANGE);         
*/    
    setFanState();
    lcdStatus();
    HSCurrentTime = millis();
    LOG1("Constructing successful!\n");
} // end constructor

void RECUP::PollCurrentState(){
    
}

boolean RECUP::update(){            
  LOG1("Updating fan state ...\n");
  if (Active->updated()){
    Active->setVal(Active->getNewVal());
    
  lcdStatus();
  setFanState(); 
  }

  if (RotationSpeed->updated()){
    RotationSpeed->setVal(RotationSpeed->getNewVal());
    setSpeed();
  }

  if (TargetFanState->updated()){
    TargetFanState->setVal(TargetFanState->getNewVal());
  }

  if (RotationDirection->updated() && TargetFanState->getVal() == tMANUAL){
    RotationDirection->setVal(RotationDirection->getNewVal());
    setSpeed();
  } else {
    RotationDirection->setVal(RotationDirection->getVal());
    LOG1("Changing direction is not allowed in AUTO mode\n");
  }


    return(true);                                   
  
} // update

void RECUP::inc(){
  if (RotationSpeed->getVal() < MAX_SPEED){
    RotationSpeed->setVal(RotationSpeed->getVal() + STEP_SPEED);
    setSpeed();
    LCDoutput.Speed = String(RotationSpeed->getVal());
  }
}

void RECUP::dec(){
  if (RotationSpeed->getVal() > MIN_SPEED){
    RotationSpeed->setVal(RotationSpeed->getVal() - STEP_SPEED);
    setSpeed();
    LCDoutput.Speed = String(RotationSpeed->getVal());
  }
}

void RECUP::OnOff(){
  Active->setVal(!Active->getVal());
  setFanState();
  lcdStatus();
}

void RECUP::setFanState(){
  
  if (Active->getVal() == OFF) {
      pwmPin->set(50);
      CurrentFanState->setVal(sIDLE);
    }else {
      CurrentFanState->setVal(sBLOW);
      setSpeed();
    }   
}

void RECUP::setSpeed(){
  //float pwmData = float(RotationSpeed->getVal())/108 + float(5300)/108;
  if (RotationDirection->getVal() == OUTTAKE) {
    pwmData = 0.0078*(RotationSpeed->getVal()) + 56.3;
  } else {
    pwmData = -0.0081*(RotationSpeed->getVal()) + 46.084;
  }
  
  pwmPin->set( pwmData );
  
  LCDoutput.Speed = String(RotationSpeed->getVal());
  LOG1("RotationSpeed ");
  LOG1(LCDoutput.Speed);
  LOG1("\n");
  LOG1("PWM ");
  LOG1(String(pwmData));
  LOG1("\n");
}

void RECUP::lcdStatus(){
  if (Active->getVal() == OFF){
    LCDoutput.Status = "Выкл";
  } else{
    LCDoutput.Status = RotationDirection->getVal()==OUTTAKE? "Вытяжка":"Приток"; //outtake?
  }
}

void RECUP::loop(){
  if ( (millis() - HSCurrentTime) > DUTY_CYCLE * (2*MAX_SPEED - RotationSpeed->getVal())/MAX_SPEED && TargetFanState->getVal() == tAUTO){
    LOG1("Change direction in the cycle of "); LOG1((millis() - HSCurrentTime)/1000); LOG1("s\n");
    RotationDirection->setVal(!RotationDirection->getVal());
    setFanState();
    lcdStatus();
    HSCurrentTime = millis();  
  }
   //LOG1("Loop\n");
                                        
}

    
 
