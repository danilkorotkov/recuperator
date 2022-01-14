#include "HomeSpan.h" 

#include "FanClass.h"
RECUP *recuperator;

#include "PCF8583.h"
// declare an instance of the library for IC at address 0xA0
// (A0 pin connected to ground)
#define sda  21
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
const int oneWireBus = 17;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);




#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, sda, scl);

unsigned long CurrentTime     = 0;   // опрос
unsigned long LCDTimeout   = 1000;   // опрос
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
  }
  
  if (enc1.isLeft()) {
    Serial.println("Left");
    recuperator->dec();
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
  
  
  if (enc1.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался
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
      new Characteristic::Model("alpha"); 
      new Characteristic::FirmwareRevision("0.1"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    recuperator = new RECUP();
}

void drawStatus() {

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, LCDoutput.inTemp);

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(128, 0, LCDoutput.outTemp);
   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 16, LCDoutput.Speed);
    
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 45, LCDoutput.Status);
 
}

void setup() {
  Serial.begin(115200);

  // Start the DS18B20 sensor
  sensors.begin();
  sensors.requestTemperatures();
  delay(1000);
  sensors.requestTemperatures();
  
  Serial.println("LCD test");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  Serial.println("Enc test");
  attachInterrupt(CLK, isrENC, CHANGE);    
  attachInterrupt(DT,  isrENC, CHANGE);   
  attachInterrupt(SW,  isrENC, CHANGE);
  enc1.setType(TYPE2);

  HomeKit();


  // configure PCF8586 to event counter mode and reset counts
  counter.setMode(MODE_EVENT_COUNTER);
  counter.setCount(0);
  CurrentTime = millis();
  Serial.println("Setup complete");  
}

void loop() {
  uint32_t rotSpeed;
  homeSpan.poll();
  if ( (millis() - CurrentTime) > LCDTimeout ) {

    float temperatureC = sensors.getTempCByIndex(0);
    sensors.requestTemperatures();
    if(temperatureC != DEVICE_DISCONNECTED_C) {
      LCDoutput.inTemp = String(temperatureC, 1) + "ºC";
    }

    
    rotSpeed = 30000*counter.getCount()/(millis() - CurrentTime);
    counter.setCount(0);
    CurrentTime = millis();
    //Serial.println(rotSpeed);
    LCDoutput.Speed = String(rotSpeed);

    display.clear();
    drawStatus();
    display.display();
    //Serial.println("LCD updated");
  }
  encloop();
}
