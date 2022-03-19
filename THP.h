uint32_t activeTimeout = 5*60*1000 ; //5мин
struct DigooData{
  uint32_t updated    = 0;
  float   temperature = 0;
  double humidity     = 100;
  bool    isNew[2]    = {true, true};
  float pressure      = 760.00;
  
};
CUSTOM_CHAR(Pressure,  E863F10F-079E-48FF-8F27-9C2605A29F52, PR+EV, FLOAT, 760, 600, 1200, true);

struct THP : Service::TemperatureSensor {         // First we create a derived class from the HomeSpan 
  
  SpanCharacteristic *CurrentTemperature;              // here we create a generic pointer to a SpanCharacteristic 
  SpanCharacteristic *StatusActive;
  DigooData*          Data;
  SpanCharacteristic  *AirPressure;    
    
  THP(DigooData* Data) : Service::TemperatureSensor(){
 
    LOG1("Constructing digoo…\n");
    this->Data = Data;
    CurrentTemperature = new Characteristic::CurrentTemperature(25);
    CurrentTemperature->setRange(-50, 100, 0.1);
    StatusActive       = new Characteristic::StatusActive(true);
    AirPressure        = new Characteristic::Pressure(760);

    LOG1(this->Data->humidity);         
    LOG1("Constructing Digoo successful!\n");

    new THP_h(this->Data);
    
  } // end constructor


  void loop(){                                     // loop() method
    if ( (millis() - Data->updated) > activeTimeout ){
      LOG1("sensor timeout\n");
      if (StatusActive -> getVal()) {StatusActive->setVal(false);}
      return;
    }
    
    if (Data->isNew[0]){
      
      Data->isNew[0] = false;
      AirPressure->setVal(Data->pressure);
      
      CurrentTemperature->setVal(Data->temperature);
      
      if ( !(StatusActive -> getVal()) ) {StatusActive->setVal(true);}
    }
  } // loop 


  struct THP_h : Service::HumiditySensor {         // First we create a derived class from the HomeSpan 
    
    SpanCharacteristic *CurrentRelativeHumidity;              // here we create a generic pointer to a SpanCharacteristic 
    SpanCharacteristic *StatusActive;
    DigooData* DataH;
    
    THP_h(DigooData* Data) : Service::HumiditySensor(){
   
      this->DataH = Data;
      CurrentRelativeHumidity=new Characteristic::CurrentRelativeHumidity(0);// this is where we create the On Characterstic we had previously defined in setup().  Save this in the pointer created above, for use below
      StatusActive=new Characteristic::StatusActive(true);
     
    } // end constructor
  
  
    void loop(){                                     // loop() method
      if ( (millis() - DataH->updated) > activeTimeout ){
        if (StatusActive -> getVal()) {StatusActive->setVal(false);}
        return;
      }
      
      if (DataH->isNew[1]){
        
        DataH->isNew[1] = false;

        //if (CurrentRelativeHumidity->getVal() != DataH->humidity) {CurrentRelativeHumidity->setVal(DataH->humidity);LOG1("sensor H update\n");}
        CurrentRelativeHumidity->setVal(DataH->humidity);
        
        if ( !(StatusActive -> getVal()) ) {StatusActive->setVal(true);}
      }
    } // loop 
  };
};
