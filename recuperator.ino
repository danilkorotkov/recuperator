#include "HomeSpan.h" 

#include "FanClass.h"
RECUP *recuperator;

#include "PCF8583.h"
// declare an instance of the library for IC at address 0xA0
// (A0 pin connected to ground)
#define sda  23
#define scl  22
PCF8583 counter(0xA0, sda, scl);
unsigned long CounterTime     = 0;   // 

#define CLK 25
#define DT  26
#define SW  27
#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);  // encoder + button

#include <OneWire.h>
#include <DallasTemperature.h>
// GPIO where the DS18B20 is connected to
const int inoneWireBus = 17; 
const int outoneWireBus = 16;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire inoneWire(inoneWireBus);
OneWire outoneWire(outoneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature insensors(&inoneWire);
DallasTemperature outsensors(&outoneWire);



#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, sda, scl);

unsigned long CurrentTime     = 0;   // опрос
unsigned long LCDTimeout   = 1000;   // опрос
unsigned long RotTimeout   = 5000;   // опрос
unsigned long RotTime      = 0;   // опрос

WordStruct LCDoutput;

void IRAM_ATTR isrENC() {
  enc1.tick();  // encoder int
}

void encloop(){
  enc1.tick();
  
  if (enc1.isTurn()) {     // если был совершён поворот (индикатор поворота в любую сторону)
    // ваш код
  }
  
  if (enc1.isRight()) {
    Serial.println("Right");         // если был поворот
    recuperator->inc();

    drawStatus();
  }
  
  if (enc1.isLeft()) {
    Serial.println("Left");
    recuperator->dec();

    drawStatus();
  }
  
  if (enc1.isRightH()) Serial.println("Right holded"); // если было удержание + поворот
  if (enc1.isLeftH()) Serial.println("Left holded");
  
  //if (enc1.isPress()) Serial.println("Press");         // нажатие на кнопку (+ дебаунс)
  //if (enc1.isRelease()) Serial.println("Release");     // то же самое, что isClick
  
  if (enc1.isClick()) Serial.println("Click");         // одиночный клик
  
  if (enc1.isSingle()){
    Serial.println("Single");       // одиночный клик (с таймаутом для двойного)
     recuperator->OnOff();
  }
  
  
  if (enc1.isDouble()) Serial.println("Double");       // двойной клик
  
  
  if (enc1.isHolded()) {                                // если была удержана и энк не поворачивался
    Serial.println("Holded");
    recuperator->TargetFanState->setVal(!recuperator->TargetFanState->getVal());
  }
  //if (enc1.isHold()) Serial.println("Hold");         // возвращает состояние кнопки

}


void HomeKit(){
  homeSpan.setApSSID("Recup-AP");
  homeSpan.setApPassword("");
  homeSpan.setControlPin(0);
  homeSpan.setStatusPin(2);
  homeSpan.setLogLevel(1);

  homeSpan.setSketchVersion("2.1.1");
  homeSpan.enableOTA();
  
  homeSpan.begin(Category::Fans,"Recuperator");
  
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Recuperator"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("beta"); 
      new Characteristic::FirmwareRevision("1.1"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    recuperator = new RECUP();
}

void drawStatus() {
    display.clear();

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, LCDoutput.inTemp);
    display.drawString(0, 16, "вн");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(128, 0, LCDoutput.outTemp);
    display.drawString(128, 16, "нар");
   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 16, LCDoutput.Speed);
    
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 45, LCDoutput.Status);

    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if (recuperator->TargetFanState->getVal() == tAUTO){
      display.drawString(0, 45, "@");
    } else{
      display.drawString(0, 45, "®");  
    }
    
    display.display();
 
}

void setup() {
  Serial.begin(115200);

  // Start the DS18B20 sensor
  insensors.begin();
  insensors.requestTemperatures();
  outsensors.begin();
  outsensors.requestTemperatures();
  delay(1000);

  HomeKit();

  Serial.println("LCD test");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  Serial.println("Enc test");
  attachInterrupt(CLK, isrENC, CHANGE);    
  attachInterrupt(DT,  isrENC, CHANGE);   
  attachInterrupt(SW,  isrENC, CHANGE);
  enc1.setType(TYPE2);

    // configure PCF8586 to event counter mode and reset counts
  counter.setMode(MODE_EVENT_COUNTER);
  counter.setCount(0);
  CurrentTime = millis();
  Serial.println("Setup complete");  
}

void loop() {
  homeSpan.poll();
  if ( (millis() - CurrentTime) > LCDTimeout ) {

    float temperatureC = insensors.getTempCByIndex(0);
    insensors.requestTemperatures();
    if(temperatureC != DEVICE_DISCONNECTED_C) {
      LCDoutput.inTemp = String(temperatureC, 1) + "ºC";
    }

    temperatureC = outsensors.getTempCByIndex(0);
    outsensors.requestTemperatures(); 
    if(temperatureC != DEVICE_DISCONNECTED_C) {
      LCDoutput.outTemp = String(temperatureC, 1) + "ºC";
    }   
 
    Serial.print("IN  temp:  ");Serial.println(LCDoutput.inTemp);
    Serial.print("OUT temp:  ");Serial.println(LCDoutput.outTemp);

    if ( (millis() - RotTime) > RotTimeout ) {
      uint32_t rotSpeed = 30000*counter.getCount()/(millis() - RotTime);
      counter.setCount(0);
      //LCDoutput.Speed = String(rotSpeed);
      Serial.print("Aver speed ");Serial.println(rotSpeed);
      RotTime = millis();
      
      if (WiFi.status() != WL_CONNECTED) { 
        Serial.println("Couldn't get a wifi connection");
      }else {
        Serial.print("WIFI RSSI: ");Serial.println(WiFi.RSSI());
      }
    }
    
    CurrentTime = millis();

    drawStatus();
  }
  encloop();
}
