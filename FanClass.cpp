#include <nvs.h>
#include <nvs_flash.h>
#include "FanClass.h"

static nvs_handle fanNVS;
size_t nvslen;

struct FanStruct FanState;

void NVS_init(){
  LOG1("FAN storage\n");
  nvs_open("FAN",NVS_READWRITE,&fanNVS);
      
  if(!nvs_get_blob(fanNVS,"FANDATA",NULL,&nvslen)) {                       // if found  data in NVS
    nvs_get_blob(fanNVS,"FANDATA",&FanState,&nvslen); }              // retrieve data  
}


////////////////////////////
   
  RECUP::RECUP() : Service::Fan(){

    LOG1("Constructing Fan…\n");
    Active              = new Characteristic::Active(OFF);
    RotationDirection   = new Characteristic::RotationDirection(OUTTAKE);
    RotationSpeed       = new Characteristic::RotationSpeed(0);
    Name                = new Characteristic::Name("recuperator");
    CurrentFanState     = new Characteristic::CurrentFanState(sIDLE);
    TargetFanState      = new Characteristic::TargetFanState(tAUTO);

    this->pwmPin        = new LedPin(SpeedPin, 68, 25000);

    RotationSpeed->setRange(1000,5300,100); //sets the range to be from a min of 1000 to a max of 5300, in steps of 1000
                                            //speed = 108 * duty - 5300
                                            //duty  = (speed + 5300) / 108
    NVS_init();
    
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
    LOG1("Constructing successful!\n");
  } // end constructor

  void RECUP::PollCurrentState(){
    
  }

  boolean RECUP::update(){            

    if(TargetFanState->getNewVal()==tMANUAL &&
       TargetFanState->getVal() != tMANUAL){ 
                                                              
                                                              
      LOG1("Manual mode\n");
      
      TargetFanState->setVal(tMANUAL);  
          
    
    } else if(TargetFanState->getNewVal()==tAUTO &&
              TargetFanState->getVal() != tAUTO){

      LOG1("Auto mode\n");                                 
      TargetFanState->setVal(tAUTO);
    }   

    return(true);                                   
  
  } // update

  void RECUP::loop(){
    //LOG1("Loop\n");                                     
  }
      


    
 
